#include <linux/module.h>
#include <asm/io.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Tasklet kernel module");
MODULE_AUTHOR("Zharinov");

#define IRQ_NUM 1

char* ascii[84] = { " ", "Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "Backspace",
                    "Tab", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "Enter", "Ctrl",
                    "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "`", "Shift (left)", "\\",
                    "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "Shift (right)",
                    "*", "Alt", "Space", "CapsLock",
                    "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10",
                    "NumLock", "ScrollLock", "Home", "Up", "Page-Up", "-", "Left",
                    " ", "Right", "+", "End", "Down", "Page-Down", "Insert", "Delete"};

static struct tasklet_struct *tasklet = NULL;
static ktime_t irq_start_time;

void my_tasklet_fun(unsigned long data) {
    char last_key_name[20];
    int last_key_code = -1;
    ktime_t end_time, diff;
    int code = inb(0x60);
    int is_release = (code & 0x80) ? 1 : 0;
    int press_code = code & 0x7F;
    if (is_release && press_code < 84) {
        last_key_code = code;
        strncpy(last_key_name, ascii[press_code], sizeof(last_key_name));
        end_time = ktime_get();
        diff = ktime_sub(end_time, irq_start_time);
        printk(KERN_INFO "mytasklet: Key: %s Code: %d Execution time: %lld\n", last_key_name, last_key_code, ktime_to_ns(diff));
    }
}

static irqreturn_t my_irq_handler(int irq, void *dev_id) {
    if (irq == IRQ_NUM) {
        irq_start_time = ktime_get();
        tasklet_schedule(tasklet);
        return IRQ_HANDLED;
    }
    return IRQ_NONE;
}

static int __init my_init(void) {
    if (request_irq(IRQ_NUM, my_irq_handler, IRQF_SHARED, "my_irq_handler", (void *)(my_irq_handler))) {
        printk(KERN_ERR "mytasklet: Error request_irq\n");
        return -ENOMEM;
    }
    tasklet = kmalloc(sizeof(struct tasklet_struct), GFP_KERNEL);
    if (tasklet == NULL) {
        printk(KERN_ERR "mytasklet: Error kmalloc tasklet\n");
        free_irq(IRQ_NUM, (void *)(my_irq_handler));
        return -ENOMEM;
    }
    tasklet_init(tasklet, my_tasklet_fun, 0);
    printk(KERN_INFO "mytasklet: module load\n");
    return 0;
}

static void __exit my_exit(void) {
    tasklet_kill(tasklet);
    kfree(tasklet);
    free_irq(IRQ_NUM, (void *)(my_irq_handler));
    printk(KERN_INFO "mytasklet: module unload\n");
}

module_init(my_init);
module_exit(my_exit);
