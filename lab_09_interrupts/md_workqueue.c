#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/ktime.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Workqueue kernel module");
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

struct my_work_struct {
    struct work_struct work;
    int code;
    ktime_t start_time;
};
static struct workqueue_struct *my_workqueue;
static struct my_work_struct *work1, *work2;

static void work_function_1(struct work_struct *work) {
    ktime_t end_time, diff;
    struct my_work_struct *my_work = container_of(work, struct my_work_struct, work);
    int code = my_work->code;
    int press_code = code & 0x7F;
    if (press_code < len_ascii) {
        end_time = ktime_get();
        diff = ktime_sub(end_time, my_work->start_time);
        printk(KERN_INFO "md_workqueue: fast Key: %s Code: %d Time: %lld\n", ascii[press_code], code, ktime_to_ns(diff));
    }
}

static void work_function_2(struct work_struct *work) {
    msleep(1);
    ktime_t end_time, diff;
    struct my_work_struct *my_work = container_of(work, struct my_work_struct, work);
    int code = my_work->code;
    int press_code = code & 0x7F;
    if (press_code < len_ascii) {
        end_time = ktime_get();
        diff = ktime_sub(end_time, my_work->start_time);
        printk(KERN_INFO "md_workqueue: slow Key: %s Code: %d Time: %lld\n", ascii[press_code], code, ktime_to_ns(diff));
    }
}

static irqreturn_t irq_handler_workqueue(int irq, void *dev_id) {
    int code;
    if (IRQ_NUM == irq) {
        code = inb(0x60);
        if (code & 0x80) {
            work1->start_time = ktime_get();
            work1->code = code;
            work2->start_time = ktime_get();
            work2->code = code;
            queue_work(my_workqueue, &work1->work);
            queue_work(my_workqueue, &work2->work);
            return IRQ_HANDLED;
        }
    }
    return IRQ_NONE;
}

static int __init my_init(void) {
    if (request_irq(IRQ_NUM, irq_handler_workqueue, IRQF_SHARED, "irq_handler_workqueue", (void *)(irq_handler_workqueue)) < 0) {
        printk(KERN_ERR "md_workqueue: Error request_irq\n");
        return -ENOMEM;
    }
    my_workqueue = alloc_workqueue("my_workqueue", WQ_MEM_RECLAIM, 1);
    if (my_workqueue == NULL) {
        free_irq(IRQ_NUM, (void *)(irq_handler_workqueue));
        printk(KERN_ERR "md_workqueue: Error create_workqueue\n");
        return -ENOMEM;
    }
    work1 = kmalloc(sizeof(struct my_work_struct), GFP_KERNEL);
    if (work1 == NULL) {
        free_irq(IRQ_NUM, (void *)(irq_handler_workqueue));
        destroy_workqueue(my_workqueue);
        printk(KERN_ERR "md_workqueue: Error kmalloc\n");
        return -ENOMEM;
    }
    work2 = kmalloc(sizeof(struct my_work_struct), GFP_KERNEL);
    if (work2 == NULL) {
        free_irq(IRQ_NUM, (void *)(irq_handler_workqueue));
        destroy_workqueue(my_workqueue);
        kfree(work1);
        printk(KERN_ERR "md_workqueue: Error kmalloc\n");
        return -ENOMEM;
    }
    INIT_WORK(&work1->work, work_function_1);
    INIT_WORK(&work2->work, work_function_2);
    printk(KERN_INFO "md_workqueue: module load\n");
    return 0;
}

static void __exit my_exit(void) {
    free_irq(IRQ_NUM, (void *)(irq_handler_workqueue));
    flush_workqueue(my_workqueue);
    destroy_workqueue(my_workqueue);
    kfree(work1);
    kfree(work2);
    printk(KERN_INFO "md_workqueue: module unload\n");
}

module_init(my_init);
module_exit(my_exit);
