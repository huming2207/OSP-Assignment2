#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <sys/types.h>

static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);

static struct file_operations file_ops =
{
		.read = device_read,
		.write = device_write,
		.open = device_open,
		.release = device_release
};

static int major_number;
static struct osp_message_container
{
	char message_payload[100] = {'\0'};
	struct semaphore message_mutex;
};
