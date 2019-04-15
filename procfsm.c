/*
 *  procfsm.c: Module for creating /proc file
 *  Based on the code in http://www.tldp.org/LDP/lkmpg/2.6/html/x710.html
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sid");
MODULE_DESCRIPTION("/proc file module - First Attempt");


#define procf_name  "helloproc"
struct proc_dir_entry *mypde;
static int proc_init(void);
static void proc_exit(void);
static ssize_t procfile_read(struct file* , char *,size_t ,loff_t *);
static ssize_t procfile_write(struct file* ,const char *,size_t ,loff_t *);
static struct file_operations fops = {
	.read = procfile_read,
	.write = procfile_write

};
static int code = 0;
/*
 * This function is called when the module is loaded
 */
static int __init proc_init(void)
{
        mypde = proc_create(procf_name, 0666, NULL, &fops);
	if(mypde==NULL)
	{
		remove_proc_entry(procf_name, NULL); // empty do while define in proc_fs.h
		printk(KERN_ALERT "Could not create /proc/%s\n", procf_name);
		return -ENOMEM;
	}
	  

	printk(KERN_INFO "/proc/%s created.\n", procf_name);

	return 0;
}

/*
 * This function is called when the module is unloaded
 */
static void __exit proc_exit(void)
{
	proc_remove(mypde);
	printk(KERN_INFO "/proc/%s removed.\n", procf_name);
}

/*
 * Methods
 */
static ssize_t procfile_read(struct file *filp, char *buffer, size_t length, loff_t * offset)

{
	//Check https://devarea.com/linux-kernel-development-creating-a-proc-file-and-interfacing-with-user-space/#.XLD6fEPhVwE
	printk(KERN_INFO "/proc/%s read\n", procf_name);
	if(*offset> 0)
	{
		return 0;
	}
	else
	{
		int len =   sprintf(buffer, "Code: %i\n", code);
		*offset = len;
		return len;
	}
	
}

static ssize_t procfile_write(struct file *filp, const char *buffer, size_t length, loff_t *offset)
{
	int num, c;
	printk(KERN_INFO "/proc/%s write\n", procf_name);
	if(*offset > 0)
		return -EFAULT;
	num = sscanf(buffer, "%i", &code);
	c = strlen(buffer);
	*offset = c;
	return c;
}



module_init(proc_init);
module_exit(proc_exit);
