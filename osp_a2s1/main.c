#include "main.h"

// Put file_ops in this file as Ecplise CDT is too stupid.
static struct file_operations file_ops =
{
		.read = device_read,
		.write = device_write,
		.open = device_open,
		.release = device_release
};

static int __init osp_a2s1_init(void)
{
	printk(KERN_ALERT "OSP_A2S1: Loading kernel module...\n");

	// Register char device
	// 0 here means asking OS give me a major device number
	major_number = register_chrdev(0, CHAR_DEVICE_NAME, &file_ops);

	// major number cannot be less than zero,
	// It must be wrong if it's -1 (or something else whatever)
	if(major_number < 0)
	{
		printk(KERN_ERR "OSP_A2S1: registering char device failed, kernel returned %d\n", major_number);
		return major_number;
	}
	else
	{
		printk(KERN_ALERT "OSP_A2S1: char device registered with major device number: %d\n", major_number);
	}

	// Register device class
	osp_device_class = class_create(THIS_MODULE, DEVICE_CLASS_NAME);

	// Detect if something went wrong
	// IS_ERR() macro detects the pointer is assigned to a valid value
	if(IS_ERR(osp_device_class))
	{
		// If things goes wrong, unregister char device
		printk(KERN_ERR "OSP_A2S1: Failed to create device class, error code %d\n", PTR_ERROR(osp_device_class));
		unregister_chardev(major_number, CHAR_DEVICE_NAME);
		return PTR_ERROR(osp_device_class);
	}
	else
	{
		printk(KERN_ALERT "OSP_A2S1: device class registered successfully.\n");
	}

	// Register device, create something at "/dev"
	// device_create(class, parent, dev_t, drvdata, fmt)
	osp_device = device_create(osp_device_class, NULL, MKDEV(major_number, 0), NULL, CHAR_DEVICE_NAME);
	if(IS_ERR(osp_device))
	{
		class_destroy(osp_device_class);
		unregister_chardev(major_number, CHAR_DEVICE_NAME);
		printk(KERN_ERR "OSP_A2S1: Failed to create the char device, error code %d", PTR_ERROR(osp_device_class));
		return PTR_ERROR(osp_device);
	}
	else
	{
		mutex_init(&osp_mutex);
		printk(KERN_ALERT "OSP_A2S1: Initialization finished successfully, winner winner, chicken dinner!\n");
		return 0;
	}

}


static int __exit osp_a2s1_exit(void)
{
	// Free mutex
	mutex_destroy(&osp_mutex);

	// Unload char device
	device_destroy(osp_device_class, MKDEV(major_number, 0));

	// Unregister device class and unload it
	class_unregister(osp_device_class);
	class_destroy(osp_device_class);

	// Unregister char device
	unregister_chardev(major_number, CHAR_DEVICE_NAME);
	printk(KERN_ALERT "OSP_A2S1: Device unloaded.\n");

	return 0;
}

static ssize_t device_read(struct file *, char *, size_t, loff_t *)
{

}

static ssize_t device_write(struct file *, const char *, size_t, loff_t *)
{

}

static int device_open(struct inode *, struct file *)
{
	if(mutex_trylock(&osp_mutex) != 1)
	{
		printk(KERN_ERR "OSP_A2S1: Device is busy, failed to open.\n");
		return -EBUSY;
	}

	printk(KERN_ALERT "OSP_A2S1: Device opened.\n");
	return 0;
}

static int device_release(struct inode *, struct file *)
{
	mutex_unlock(&osp_mutex);
	printk(KERN_ALERT "OSP_A2S1: Device released.\n");
	return 0;
}

// Some macros


module_init(osp_a2s1_init);
module_exit(osp_a2s1_exit);
