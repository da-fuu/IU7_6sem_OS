#include <linux/module.h>
#include <asm/io.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Tasklet kernel module");
MODULE_AUTHOR("Zharinov");

#define IRQ_NUM 1
static const char *ascii[] = {  " ", "Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "Backspace",
                                "Tab", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "Enter", "Ctrl",
                                "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "`", "Shift (left)", "\\",
                                "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "Shift (right)",
                                "*", "Alt", "Space", "CapsLock",
                                "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10",
                                "NumLock", "ScrollLock", "Home", "Up", "Page-Up", "-", "Left",
                                " ", "Right", "+", "End", "Down", "Page-Down", "Insert", "Delete" };
static const int len_ascii = ARRAY_SIZE(ascii);
static struct tasklet_struct *tasklet = NULL;
static ktime_t irq_start_time;

static void my_tasklet_fun(unsigned long data) {
    char key_name[20];
    ktime_t end_time, diff;
    int code = inb(0x60);
    int press_code;
    if (code & 0x80) {
        press_code = code & 0x7F;
        if (press_code < len_ascii)
            strncpy(key_name, ascii[press_code], sizeof(key_name));
        else
            strncpy(key_name, "Unknown", sizeof(key_name));
        end_time = ktime_get();
        diff = ktime_sub(end_time, irq_start_time);
        printk(KERN_INFO "mytasklet: Key: %s Code: %d Time: %lld\n", key_name, code, ktime_to_ns(diff));
    }
}

static irqreturn_t irq_handler_tasklet(int irq, void *dev_id) {
    if (irq == IRQ_NUM) {
        irq_start_time = ktime_get();
        tasklet_schedule(tasklet);
        return IRQ_HANDLED;
    }
    return IRQ_NONE;
}

static int __init my_init(void) {
    if (request_irq(IRQ_NUM, irq_handler_tasklet, IRQF_SHARED, "irq_handler_tasklet", (void *)(irq_handler_tasklet))) {
        printk(KERN_ERR "mytasklet: Error request_irq\n");
        return -ENOMEM;
    }
    tasklet = kmalloc(sizeof(struct tasklet_struct), GFP_KERNEL);
    if (tasklet == NULL) {
        printk(KERN_ERR "mytasklet: Error kmalloc tasklet\n");
        free_irq(IRQ_NUM, (void *)(irq_handler_tasklet));
        return -ENOMEM;
    }
    tasklet_init(tasklet, my_tasklet_fun, 0);
    printk(KERN_INFO "mytasklet: module load\n");
    return 0;
}

static void __exit my_exit(void) {
    tasklet_kill(tasklet);
    kfree(tasklet);
    free_irq(IRQ_NUM, (void *)(irq_handler_tasklet));
    printk(KERN_INFO "mytasklet: module unload\n");
}

module_init(my_init);
module_exit(my_exit);
