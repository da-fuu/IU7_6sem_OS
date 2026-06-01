#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/fs_context.h>
#include <linux/mnt_idmapping.h>
#include <linux/atomic.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A simple VFS kernel module");

#define MYVFS_MAGIC_NUMBER 0x61946878
#define SLAB_NAME "MyVFSCache"

#define STRUCT_SIZE 32
#define ELEM_NUM 128

static void **struct_arr = NULL;
struct kmem_cache * cache = NULL;

static struct my_vfs_inode {
    int i_mode;
    unsigned long i_ino;
} my_vfs_inode;

static struct inode *my_vfs_make_inode(struct super_block *sb, int mode) {
    struct inode *ret = new_inode(sb);
    if (ret) {
        struct timespec64 ts;
        inode_init_owner(&nop_mnt_idmap, ret, NULL, mode);
        ret->i_size = PAGE_SIZE;
        ts = current_time(ret);
        inode_set_atime_to_ts(ret, ts);
        inode_set_mtime_to_ts(ret, ts);
        inode_set_ctime_to_ts(ret, ts);
        ret->i_private = &my_vfs_inode;
        ret->i_ino = 1;
    }
    printk(KERN_INFO "MyVFS: struct inode created\n");
    return ret;
}

static void my_vfs_put_super(struct super_block *sb) {
    printk(KERN_INFO "MyVFS: super block destroyed\n");
}

static struct super_operations const my_vfs_sup_ops = {
    .put_super = my_vfs_put_super,
    .statfs = simple_statfs,
};

static int my_vfs_fill_sb(struct super_block *sb, struct fs_context *fc) {
    struct inode *root_inode;
    struct timespec64 ts;
    sb->s_blocksize = PAGE_SIZE;
    sb->s_blocksize_bits = PAGE_SHIFT;
    sb->s_magic = MYVFS_MAGIC_NUMBER;
    sb->s_op = &my_vfs_sup_ops;
    root_inode = my_vfs_make_inode(sb, S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO);
    if (!root_inode) {
        printk(KERN_ERR "MyVFS: my_vfs_make_inode error\n");
        return -ENOMEM;
    }
    ts = current_time(root_inode);
    inode_set_atime_to_ts(root_inode, ts);
    inode_set_mtime_to_ts(root_inode, ts);
    inode_set_ctime_to_ts(root_inode, ts);
    root_inode->i_op = &simple_dir_inode_operations;
    root_inode->i_fop = &simple_dir_operations;
    sb->s_root = d_make_root(root_inode);
    if (!sb->s_root) {
        printk(KERN_ERR "MyVFS: d_make_root error\n");
        iput(root_inode);
        return -ENOMEM;
    } else {
        printk(KERN_INFO "MyVFS: VFS root created\n");
    }
    printk(KERN_INFO "MyVFS: super_block init");
    return 0;
}

static int my_vfs_get_tree(struct fs_context *fc) {
    return get_tree_nodev(fc, my_vfs_fill_sb);
}

static const struct fs_context_operations my_vfs_context_ops = {
    .get_tree = my_vfs_get_tree,
};

static int my_vfs_init_fs_context(struct fs_context *fc) {
    fc->ops = &my_vfs_context_ops;
    return 0;
}

static void my_kill_litter_super(struct super_block *sb) {
    printk(KERN_INFO "MyVFS: my_kill_litter_super");
    kill_anon_super(sb);
}

static struct file_system_type my_vfs_type = {
    .owner   = THIS_MODULE,
    .name    = "MyVFS",
    .init_fs_context = my_vfs_init_fs_context,
    .kill_sb = my_kill_litter_super
};

static void my_vfs_free_memory(void)
{
    int i;
    if (struct_arr) {
        for (i = 0; i < ELEM_NUM; i++) {
            if (struct_arr[i])
                kmem_cache_free(cache, struct_arr[i]);
        }
        kfree(struct_arr);
    }
    kmem_cache_destroy(cache);
    printk(KERN_INFO "MyVFS: slab destroyed\n");
}

static int __init my_vfs_init(void) {
    int i, ret;
    ret = register_filesystem(&my_vfs_type);
    if (ret != 0) {
        printk(KERN_ERR "MyVFS: register_filesystem error\n");
        return ret;
    }

    cache = kmem_cache_create(SLAB_NAME, STRUCT_SIZE, 0, SLAB_HWCACHE_ALIGN | SLAB_NO_MERGE, NULL);
    if (!cache) {
        printk(KERN_ERR "MyVFS: kmem_cache_create error\n");
        unregister_filesystem(&my_vfs_type);
        return -ENOMEM;
    }

    struct_arr = kcalloc(ELEM_NUM, sizeof(void *), GFP_KERNEL);
    if (!struct_arr) {
        printk(KERN_ERR "MyVFS: kcalloc error\n");
        my_vfs_free_memory();
        unregister_filesystem(&my_vfs_type);
        return -ENOMEM;
    }

    for (i = 0; i < ELEM_NUM; i++) {
        struct_arr[i] = kmem_cache_alloc(cache, GFP_KERNEL);
        if (!struct_arr[i]) {
            printk(KERN_ERR "MyVFS: kmem_cache_alloc error for entry %d\n", i);
            my_vfs_free_memory();
            unregister_filesystem(&my_vfs_type);
            return -ENOMEM;
        }
    }

    printk(KERN_INFO "MyVFS: loaded\n");
    return 0;
}

static void __exit my_vfs_exit(void) {
    my_vfs_free_memory();
    if (unregister_filesystem(&my_vfs_type) != 0) {
        printk(KERN_ERR "MyVFS: unregister_filesystem error\n");
    }
    printk(KERN_INFO "MyVFS: exit\n");
}

module_init(my_vfs_init);
module_exit(my_vfs_exit);
