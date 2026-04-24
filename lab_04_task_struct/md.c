#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zharinov");
MODULE_DESCRIPTION("Print process info");
MODULE_VERSION("1.0");

static char *state_str(unsigned int state) {
    int pos = 0;
    static char buf[128];
    size_t size = sizeof(buf);
    buf[0] = '\0';
    if (state == TASK_RUNNING) {
        snprintf(buf, size, "TASK_RUNNING");
        return buf;
    }
    if (state & TASK_INTERRUPTIBLE) pos += snprintf(buf + pos, size - pos, "TASK_INTERRUPTIBLE | ");
    if (state & TASK_UNINTERRUPTIBLE) pos += snprintf(buf + pos, size - pos, "TASK_UNINTERRUPTIBLE | ");
    if (state & __TASK_STOPPED) pos += snprintf(buf + pos, size - pos, "__TASK_STOPPED | ");
    if (state & __TASK_TRACED) pos += snprintf(buf + pos, size - pos, "__TASK_TRACED | ");
    if (state & EXIT_DEAD) pos += snprintf(buf + pos, size - pos, "EXIT_DEAD | ");
    if (state & EXIT_ZOMBIE) pos += snprintf(buf + pos, size - pos, "EXIT_ZOMBIE | ");
    if (state & TASK_PARKED) pos += snprintf(buf + pos, size - pos, "TASK_PARKED | ");
    if (state & TASK_DEAD) pos += snprintf(buf + pos, size - pos, "TASK_DEAD | ");
    if (state & TASK_WAKEKILL) pos += snprintf(buf + pos, size - pos, "TASK_WAKEKILL | ");
    if (state & TASK_WAKING) pos += snprintf(buf + pos, size - pos, "TASK_WAKING | ");
    if (state & TASK_NOLOAD) pos += snprintf(buf + pos, size - pos, "TASK_NOLOAD | ");
    if (state & TASK_NEW) pos += snprintf(buf + pos, size - pos, "TASK_NEW | ");
    if (state & TASK_RTLOCK_WAIT) pos += snprintf(buf + pos, size - pos, "TASK_RTLOCK_WAIT | ");
    if (state & TASK_FREEZABLE) pos += snprintf(buf + pos, size - pos, "TASK_FREEZABLE | ");
    if (state & __TASK_FREEZABLE_UNSAFE) pos += snprintf(buf + pos, size - pos, "__TASK_FREEZABLE_UNSAFE | ");
    if (state & TASK_FROZEN) pos += snprintf(buf + pos, size - pos, "TASK_FROZEN | ");
    if (pos >= 3) {
        buf[pos - 3] = '\0';
    }
    return buf;
}

static const char *policy_str(unsigned int policy) {
    switch (policy) {
        case SCHED_NORMAL: return "SCHED_NORMAL";
        case SCHED_FIFO: return "SCHED_FIFO";
        case SCHED_RR: return "SCHED_RR";
        case SCHED_BATCH: return "SCHED_BATCH";
        case SCHED_IDLE: return "SCHED_IDLE";
        case SCHED_DEADLINE: return "SCHED_DEADLINE";
        case SCHED_EXT: return "SCHED_EXT";
        default: return "UNKNOWN";
    }
}

static int __init md_init(void) {
    struct task_struct *task = &init_task;
    do {
        printk(KERN_INFO "*** comm - %s, pid - %d, parent - %s, ppid - %d, tgid - %d\n"
               "    cpu - %u, state - %s, flags - 0x%x, on_cpu - %d\n"
               "    prio - %d, static_prio - %d, normal_prio - %d, policy - %s\n"
               "    migration_disabled - %hu, migration_flags - %hu, exit_state - %d, exit_code - %d, exit_signal - %d\n"
               "    utime - %llu, stime - %llu, start_time - %llu, sessionid - %u\n"
               "    last_switch_count - %lu, last_switch_time - %lu, nvcsw - %lu, nivcsw - %lu\n",
               task->comm, task->pid, task->parent->comm, task->parent->pid, task->tgid,
               task->thread_info.cpu, state_str(task->__state), task->flags, task->on_cpu,
               task->prio, task->static_prio, task->normal_prio, policy_str(task->policy),
               task->migration_disabled, task->migration_flags, task->exit_state, task->exit_code, task->exit_signal,
               task->utime, task->stime, task->start_time, task->sessionid,
               task->last_switch_count, task->last_switch_time, task->nvcsw, task->nivcsw);
    } while ((task = next_task(task)) != &init_task);

    printk(KERN_INFO "*** current: comm - %s, pid - %d, parent - %s, ppid - %d, tgid - %d\n"
           "    cpu - %u, state - %s, flags - 0x%x, on_cpu - %d\n"
           "    prio - %d, static_prio - %d, normal_prio - %d, policy - %s\n"
           "    migration_disabled - %hu, migration_flags - %hu, exit_state - %d, exit_code - %d, exit_signal - %d\n"
           "    utime - %llu, stime - %llu, start_time - %llu, sessionid - %u\n"
           "    last_switch_count - %lu, last_switch_time - %lu, nvcsw - %lu, nivcsw - %lu\n",
           current->comm, current->pid, current->parent->comm, current->parent->pid, current->tgid,
           current->thread_info.cpu, state_str(current->__state), current->flags, current->on_cpu,
           current->prio, current->static_prio, current->normal_prio, policy_str(current->policy),
           current->migration_disabled, current->migration_flags, current->exit_state, current->exit_code, current->exit_signal,
           current->utime, current->stime, current->start_time, current->sessionid,
           current->last_switch_count, current->last_switch_time, current->nvcsw, current->nivcsw);
    return 0;
}

static void __exit md_exit(void) {
    printk(KERN_INFO "*** Module unload\n");
}

module_init(md_init);
module_exit(md_exit);
