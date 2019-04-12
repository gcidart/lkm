/*
 *  chardev.c: Creates a read-only char device that says how many times
 *  you've read from the dev file
 *  Based on the code in http://www.tldp.org/LDP/lkmpg/2.6/html/x569.html
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <asm/uaccess.h>	/* for put_user */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sid");
MODULE_DESCRIPTION("Character Driver - First Attempt");
/*  
 *  Prototypes - this would normally go in a .h file
 */
static int char_init(void);
static void char_exit(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "chardev"	/* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80		/* Max length of the message from the device */

/* 
 * Global variables are declared as static, so are global within the file. 
 */

static int Major;		/* Major number assigned to our device driver */
static int Device_Open = 0;	/* Is device open?  
				 * Used to prevent multiple access to device */
static char msg[BUF_LEN];	/* The msg the device will give when asked */
static char *msg_Ptr;
struct class *pclass;

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

/*
 * This function is called when the module is loaded
 */
static int __init char_init(void)
{
        Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0) {
		printk(KERN_ALERT "Registering char device failed with %d\n", Major);
	  	return Major;
	}
	/* Create Device File https://stackoverflow.com/questions/49350553/can-i-call-mknod-from-my-kernel-module */
        pclass = class_create(THIS_MODULE, DEVICE_NAME);
	if(pclass < 0)
	{
		printk(KERN_ALERT "Cant create class\n");
		unregister_chrdev(Major, DEVICE_NAME);
		return -1;
	}
	if (device_create(pclass, NULL,MKDEV(Major, 0), NULL, DEVICE_NAME) < 0){
	  	printk(KERN_ALERT "Device File not created\n");
		unregister_chrdev(Major, DEVICE_NAME);
		return -1;
	}	  

	printk(KERN_INFO "/dev/%s created.\n", DEVICE_NAME);

	return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
static void __exit char_exit(void)
{
	/* Destroy Device and Class */
	device_destroy(pclass, MKDEV(Major, 0));
	class_destroy(pclass);

	/* 
	 * Unregister the device 
	 */
	unregister_chrdev(Major, DEVICE_NAME);
}

/*
 * Methods
 */

/* 
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *file)
{
	static int counter = 0;

	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	sprintf(msg, "I already told you %d times Hello world!\n", counter++);
	msg_Ptr = msg;
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

/* 
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;		/* We're now ready for our next caller */

	/* 
	 * Decrement the usage count, or else once you opened the file, you'll
	 * never get get rid of the module. 
	 */
	module_put(THIS_MODULE);

	return 0;
}

/* 
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
	/*
	 * Number of bytes actually written to the buffer 
	 */
	int bytes_read = 0;

	/*
	 * If we're at the end of the message, 
	 * return 0 signifying end of file 
	 */
	if (*msg_Ptr == 0)
		return 0;

	/* 
	 * Actually put the data into the buffer 
	 */
	while (length && *msg_Ptr) {

		/* 
		 * The buffer is in the user data segment, not the kernel 
		 * segment so "*" assignment won't work.  We have to use 
		 * put_user which copies data from the kernel data segment to
		 * the user data segment. 
		 */
		put_user(*(msg_Ptr++), buffer++);

		length--;
		bytes_read++;
	}

	/* 
	 * Most read functions return the number of bytes put into the buffer
	 */
	return bytes_read;
}

/*  
 * Called when a process writes to dev file: echo "hi" > /dev/hello 
 */
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
	return -EINVAL;
}


module_init(char_init);
module_exit(char_exit);