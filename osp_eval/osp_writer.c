#include "osp_writer.h"

// Put file_ops in this file as Ecplise CDT is too stupid.
static struct file_operations file_ops =
{
		.owner = THIS_MODULE,
		.read = device_read,
		.write = device_write,
		.open = device_open,
		.release = device_release
};

static int __init osp_a2s1_init(void)
{
	printk(KERN_INFO "OSP_A2S1: Loading kernel module...\n");

	// Register char device
	// 0 here means asking OS give me a major device number
	major_number = register_chrdev(0, CHAR_DEVICE_NAME, &file_ops);

	// major number cannot be less than zero,
	// It must be wrong if it's -1 (or something else whatever)
	if(major_number < 0)
	{
		printk(KERN_ERR "OSP_A2S1: Registering char device failed, kernel returned %d .\n", major_number);
		return major_number;
	}
	else
	{
		printk(KERN_INFO "OSP_A2S1: Char device registered with major device number: %d .\n", major_number);
	}

	// Register device class
	osp_device_class = class_create(THIS_MODULE, DEVICE_CLASS_NAME);

	// Detect if something went wrong
	// IS_ERR() macro detects the pointer is assigned to a valid value
	if(IS_ERR(osp_device_class))
	{
		// If things goes wrong, unregister char device
		printk(KERN_ERR "OSP_A2S1: Failed to create device class, error code %ld .\n", PTR_ERR(osp_device_class));
		unregister_chrdev(major_number, CHAR_DEVICE_NAME);
		return PTR_ERR(osp_device_class);
	}
	else
	{
		printk(KERN_INFO "OSP_A2S1: Device class registered successfully.\n");
	}

	// Register device, create something at "/dev"
	// device_create(class, parent, dev_t, drvdata, fmt)
	osp_device = device_create(osp_device_class, NULL, MKDEV(major_number, 0), NULL, CHAR_DEVICE_NAME);
	if(IS_ERR(osp_device))
	{
		class_destroy(osp_device_class);
		unregister_chrdev(major_number, CHAR_DEVICE_NAME);
		printk(KERN_ERR "OSP_A2S1: Failed to create the char device, error code %ld .\n", PTR_ERR(osp_device_class));
		return PTR_ERR(osp_device);
	}
	else
	{
		mutex_init(&osp_mutex);
		printk(KERN_INFO "OSP_A2S1: Initialization finished successfully, winner winner, chicken dinner!\n");
		return 0;
	}

}


static void __exit osp_a2s1_exit(void)
{
	// Free mutex
	mutex_destroy(&osp_mutex);

	// Unload char device
	device_destroy(osp_device_class, MKDEV(major_number, 0));

	// Unregister device class and unload it
	class_unregister(osp_device_class);
	class_destroy(osp_device_class);

	// Unregister char device
	unregister_chrdev(major_number, CHAR_DEVICE_NAME);
	printk(KERN_INFO "OSP_A2S1: Device unloaded.\n");
}

static ssize_t device_read(struct file * file_pointer, char __user * str_buffer, size_t str_length, loff_t * offset)
{
	int copy_result;
	printk(KERN_INFO "OSP_A2S1: Device_read started...");
	copy_result = copy_to_user(
			str_buffer,
			osp_message_payload,
			message_size);

	// Result must be 0, or something goes wrong again.
	if(copy_result != 0)
	{
		printk(KERN_ERR "OSP_A2S1: Copy buffer to user space failed, function returned %d\n", copy_result);
		return -EFAULT;
	}
	else
	{
		// Print length
		printk(KERN_INFO "OSP_A2S1: Copy successful, length: %d\n", message_size);
		printk(KERN_INFO "OSP_A2S1: Message: \"%s\"", osp_message_payload);


		vfree(osp_message_payload);
		message_size = 0;
		return 0;
	}
}

static ssize_t device_write(struct file * file_pointer, const char __user * str_buffer, size_t str_length, loff_t * offset)
{
	int copy_result;
	printk(KERN_INFO "OSP_A2S1: Device_write started...");

	// Initialize and wipe the message memory area first before using
	osp_message_payload = vmalloc(str_length);
	if(osp_message_payload != NULL)
	{
		// Wipe the memory area before using...
		memset(osp_message_payload, '\0', (size_t)message_size);
	}
	else
	{
		printk(KERN_ERR "OSP_A2S1: Message payload initialization failed!");
		return -EFAULT;
	}

	// Copy string buffer
	copy_result = copy_from_user(osp_message_payload, str_buffer, str_length);
	if(copy_result != 0)
	{
		printk(KERN_ERR "OSP_A2S1: Copy buffer from user space failed, function returned %d\n", copy_result);
		return -EFAULT;
	}

	log_key("/root/test.log", osp_message_payload);
	printk(KERN_INFO "OSP_A2S1: Copy from user space finished.\n");

	// Set the length for device_read function
	message_size = strlen(osp_message_payload);
	return str_length;
}

static int device_open(struct inode * inode_pointer, struct file * file_pointer)
{
	if(mutex_trylock(&osp_mutex) != 1)
	{
		printk(KERN_ERR "OSP_A2S1: Device is busy, failed to open.\n");
		return -EBUSY;
	}

	printk(KERN_INFO "OSP_A2S1: Device opened.\n");
	return 0;
}

static int device_release(struct inode * inode_pointer, struct file * file_pointer)
{
	mutex_unlock(&osp_mutex);
	printk(KERN_INFO "OSP_A2S1: Device released.\n");
	return 0;
}

static void log_key(char * path, char * str_to_write)
{
	// Declare file pointer and segment stuff
	struct file * file_pointer;
	mm_segment_t origin_file_segment;

	// Backup file segment and set to kernel space operations
	origin_file_segment = get_fs();

	set_fs(KERNEL_DS);

	printk(KERN_INFO "USBKBD_Keylogger: Kernel mode entered for file pointer.");


	// Try open it
	// File open mode: r/w, append; Permission: 666
	file_pointer = filp_open(path, O_RDWR | O_APPEND | O_CREAT, 0666);

	// If something went wrong, print debug info and stop
	if(IS_ERR(file_pointer))
	{
		printk(KERN_ERR "USBKBD_Keylogger: Failed to open file, returned error %ld", PTR_ERR(file_pointer));
		return;
	}
	else
	{
		printk(KERN_INFO "USBKBD_Keylogger: File opened/created.");
	}


	printk(KERN_INFO "USBKBD_Keylogger: Kernel mode entered.");

	printk(KERN_INFO "USBKBD_Keylogger: Message to write: %s", str_to_write);
	printk(KERN_INFO "USBKBD_Keylogger: File position value: %lld, address :%p", file_pointer->f_pos, &file_pointer->f_pos);

	if(file_pointer->f_op->write)
	{
		printk(KERN_INFO "USBKBD_Keylogger: f_op->write is not null, just use it.");

		// Write to file
		file_pointer->f_op->write(
				file_pointer,
				str_to_write,
				sizeof(str_to_write),
				&file_pointer->f_pos);
	}
	else
	{
		printk(KERN_INFO "USBKBD_Keylogger: f_op->write somehow does not exist, use something else instead.");

		vfs_write(
				file_pointer,
				str_to_write,
				sizeof(str_to_write),
				&file_pointer->f_pos);

		vfs_fsync(file_pointer, 0);
	}



	printk(KERN_INFO "USBKBD_Keylogger: Written to file.");
	// Restore file segment back to original one
	set_fs(origin_file_segment);

	printk(KERN_INFO "USBKBD_Keylogger: Return user mode.");

	// Close the file pointer (also flush)
	filp_close(file_pointer, NULL);

	printk(KERN_INFO "USBKBD_Keylogger: Log finished.");
}


// Some macros


module_init(osp_a2s1_init);
module_exit(osp_a2s1_exit);
