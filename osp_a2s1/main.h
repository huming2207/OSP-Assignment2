#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <sys/types.h>

MODULE_AUTHOR("Ming Hu s3554025");
MODULE_LICENSE("GPL and additional rights");
MODULE_DESCRIPTION("OSP A2s1 kernel module");
MODULE_VERSION("1.0");

#define CHAR_DEVICE_NAME "S3554025Device" // Will be "/dev/S3554025Device"
#define DEVICE_CLASS_NAME "osp_a2" // Will be "/sys/class/osp_a2/S3554025Device"

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
struct osp_message_container
{
	char message_payload[100];
	int message_size;
	struct semaphore message_mutex;
};

static struct class * osp_device_class;
static struct device * osp_device;
static int open_times;
