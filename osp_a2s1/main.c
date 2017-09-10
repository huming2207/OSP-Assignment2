#include "main.h"

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
		printk(KERN_ERR "OSP_A2S1: Failed to create device class, kernel returned %d\n", PTR_ERROR(osp_device_class));
		unregister_chardev(major_number, CHAR_DEVICE_NAME);
		return PTR_ERROR(osp_device_class);
	}
	else
	{
		printk(KERN_ALERT "OSP_A2S1: device class registered successfully.\n");
	}

	// Register device, create something at "/dev"
	osp_device = device_create(osp_device_class, NULL, MKDEV(major_number), NULL, CHAR_DEVICE_NAME);

}


static int __exit osp_a2s1_exit(void)
{

}

// Some macros
module_init(osp_a2s1_init);
module_exit(osp_a2s1_exit);
