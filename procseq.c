/**
 *  procseq.c -  create a "file" in /proc using Seq File
 * 	This program uses the seq_file library to manage the /proc file.
 *
 * 	Test using cat /proc/seqproc or dd if=/proc/seqproc bs=9400
 *
 */

#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/module.h>	/* Specifically, a module */
#include <linux/proc_fs.h>	/* Necessary because we use proc fs */
#include <linux/seq_file.h>	/* for seq_file */
#include<linux/list.h>          /* for linked list */
#include <linux/slab.h>		/* For kmalloc */

#define procf_name  "seqproc"
struct proc_dir_entry *mypde;

MODULE_AUTHOR("Sid");
MODULE_LICENSE("GPL");

struct k_list {
struct list_head list_ptr;
long long val;
};

struct list_head test_head;
struct list_head *ptr;
struct k_list *temp;

static loff_t l_size = 0;

/**
 * This function is called at the beginning of a sequence.
 * ie, when:
 *	- the /proc file is read (first time)
 *	- after the function stop (end of sequence)
 *
 */
static void *my_seq_start(struct seq_file *s, loff_t *pos)
{
	printk(KERN_ALERT "position in start: %Ld", *pos);
	if(*pos==0)
	{
		(*pos)++;
		return list_entry(test_head.next, struct k_list, list_ptr);
	}
	/*Doing this as seq_start() is called everytime after 4K bytes are written*/
	else if(*pos < l_size)
	{
		//(*pos)++;      
		/*Need to reread the last ptr returned by seq_next() incase 4KB page limit is hit */
		return list_entry(ptr, struct k_list, list_ptr);
	}
	else 
		return NULL;
	
}

/**
 * This function is called after the beginning of a sequence.
 * It's called untill the return is NULL (this ends the sequence).
 *
 */
static void *my_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	ptr = ((struct k_list* ) v)->list_ptr.next;
	temp=list_entry(ptr,struct k_list,list_ptr);
	printk(KERN_ALERT "position in next: %Ld out of %Ld with value %Ld", *pos, l_size, temp->val);

	/* Return NULL and go to seq_stop if traversed through the entire list */
	if((*pos)++ >=l_size) 	
		return NULL;
	else
		return temp;
	
}

/**
 * This function is called at the end of a sequence
 * 
 */
static void my_seq_stop(struct seq_file *s, void *v)
{
	/* nothing to do, we use a static value in start() */
	printk(KERN_ALERT "inside seq_stop()");
}

/**
 * This function is called for each "step" of a sequence after seq_start() or seq_next()
 * TODO : Figure out why values at 4K boundary do not getting printed without the hack in seq_start()
 */
static int my_seq_show(struct seq_file *s, void *v)
{
	temp = (struct k_list*) v;
	printk(KERN_ALERT "inside seq_show() with value %Ld", temp->val);
	seq_printf(s, "%Ld\n", temp->val);
	return 0;
}

/**
 * This structure gathers "function" to manage the sequence
 *
 */
static struct seq_operations my_seq_ops = {
	.start = my_seq_start,
	.next  = my_seq_next,
	.stop  = my_seq_stop,
	.show  = my_seq_show
};

/**
 * This function is called when the /proc file is open.
 *
 */
static int my_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &my_seq_ops);
};

/**
 * This structure gather "function" that manage the /proc file
 *
 */
static struct file_operations my_file_ops = {
	.owner   = THIS_MODULE,
	.open    = my_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release
};
	
	
/**
 * This function is called when the module is loaded
 *
 */
static int __init proc_init(void)
{
	int i;
	mypde = proc_create(procf_name, 0666, NULL, &my_file_ops);
        if(mypde==NULL)
        {
                remove_proc_entry(procf_name, NULL); // empty do while define in proc_fs.h
                printk(KERN_ALERT "Could not create /proc/%s\n", procf_name);
                return -ENOMEM;
        }
	INIT_LIST_HEAD(&test_head);
	for(i =0; i < 2100; i++)
	{
		temp = kmalloc(sizeof(struct k_list ),GFP_KERNEL);
		temp->val = i;
		list_add_tail(&temp->list_ptr,&test_head);
		l_size+=1;
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
module_init(proc_init);
module_exit(proc_exit);
