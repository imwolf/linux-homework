//#include <linux/config.h>  2.6.35 and 3.0.24 does not have this
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h>  /* everything... */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <asm/uaccess.h> /* copy_*_user */
#include "scull.h" /* local definitions */

/*
 * Our parameters which can be set at load time.
 */
int scull_major = SCULL_MAJOR;
int scull_minor = 0;
int scull_nr_devs = SCULL_NR_DEVS; /* number of bare scull devices */
int scull_quantum = SCULL_QUANTUM; //ADD:defined at scull.h;4000
int scull_qset =    SCULL_QSET;    //1000
module_param(scull_major, int, S_IRUGO);
module_param(scull_minor, int, S_IRUGO);
module_param(scull_nr_devs, int, S_IRUGO);
module_param(scull_quantum,int,S_IRUGO);//ADD:
module_param(scull_qset,int,S_IRUGO);//
MODULE_AUTHOR("Alessandro Rubini, Jonathan Corbet");
MODULE_LICENSE("Dual BSD/GPL");
struct scull_dev *scull_devices; /* allocated in scull_init_module */

/*
 * Empty out the scull device; must be called with the device
 * semaphore held.
 */
int scull_trim(struct scull_dev *dev)
{
	//memset(dev->data, 0, SCULL_DATA_SIZE);
	struct scull_qset *next,*dptr;
	int qset = dev->qset;
	int i;

	for (dptr = dev->data;dptr;dptr = next){
		if (dptr->data){
			for (i = 0;i<qset;i++)
				kfree(dptr->data[i]);//释放每一个量子，默认共1000个
			kfree(dptr->data);//將整个量子集释放，4000大小的指针数组	   
			dptr->data = NULL;
		}
		next = dptr->next;
		kfree(dptr);
	}

	dev->size = 0;
	dev->quantum = scull_quantum; //ADD
	dev->qset = scull_qset;
	dev->data = NULL;
	return 0;
}


/*
 * Open and close
 */
int scull_open(struct inode *inode, struct file *filp)
{
	struct scull_dev *dev; /* device information */
	dev = container_of(inode->i_cdev, struct scull_dev, cdev);
	filp->private_data = dev; /* for other methods */

	/* now trim to 0 the length of the device if open was write-only */
	if ( (filp->f_flags & O_ACCMODE) == O_WRONLY) {
		if (down_interruptible(&dev->sem))
			return -ERESTARTSYS;
		scull_trim(dev); /* ignore errors */
		up(&dev->sem);
	}
	return 0; /* success */
}


int scull_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/*
 *遍历链表,遍历到第n项；必要的时候，分配主动分配链表项
 *也就是说该方法基本上一直能成功(除非内存不够)
 *量子集和量子的分配推迟到向设备文件写入时才有必要分配
 */
struct scull_qset *scull_follow(struct scull_dev *dev,int n)
{
	struct scull_qset *qs = dev->data;
	if (!qs){//在该项尚未分配qset时分配一项
		qs = dev->data = kmalloc(sizeof(struct scull_qset),GFP_KERNEL);
		if (qs == NULL)
			return NULL;
		memset(qs,0,sizeof(struct scull_qset));
	}

	while(n--){
		if (!qs->next){//后项没有时主动分配一项
			qs->next = kmalloc(sizeof(struct scull_qset),GFP_KERNEL);
			if (qs->next == NULL)
				return NULL;
			memset(qs->next,0,sizeof(struct scull_qset));
		}
		qs = qs->next;
		continue;
	}
	return qs;
}

/*
 * Data management: read and write
 */
ssize_t scull_read(struct file *filp, char __user *buf, size_t count,
		loff_t *f_pos)
{
	struct scull_dev *dev = filp->private_data;
	struct scull_qset *dptr; /*ADD:链表中的第一项*/
	int quantum = dev->quantum,qset = dev->qset;
	int itemsize = quantum * qset;

	int item,s_pos,q_pos,rest;
	ssize_t retval = 0;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	if (*f_pos >= dev->size)
		goto out;
	if (*f_pos + count > dev->size)
		count = dev->size - *f_pos;

	//ADD:计算位置
	item = (long)*f_pos /itemsize; //链表中的第几项
	rest = (long)*f_pos % itemsize;
	s_pos = rest / quantum;  //该项中的第几个量子
	q_pos = rest % quantum;	 //量子(char 数组)中的第几个单元

	dptr = scull_follow(dev,item);  //遍历到正确的位置

	if (dptr == NULL || !dptr->data || !dptr->data[s_pos])
		goto out;   //各种空，什么时候发生?

	if (count > quantum -q_pos)  //量子化处理，每次最多读到量子尾（即最多4000，默认时）
		count = quantum -q_pos;
	if (copy_to_user(buf, dev->data + *f_pos, count)) {
		retval = -EFAULT;
		goto out;
	}
	*f_pos += count;
	retval = count;
out:
	up(&dev->sem);
	return retval;
}
ssize_t scull_write(struct file *filp, const char __user *buf, size_t count,
		loff_t *f_pos)
{
	struct scull_dev *dev = filp->private_data;
	struct scull_qset *dptr;
	int quantum = dev->quantum; //ADD
	int qset = dev->qset;
	int itemsize = quantum * qset; //链表项大小 1000 * 4000
	int item,s_pos,q_pos,rest;
	ssize_t retval = -ENOMEM; /* value used in "goto out" statements */
	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	/*	if (*f_pos >= SCULL_DATA_SIZE) {
		retval = -ENOSPC;
		goto out;
		}*/
	//计算正确位置
	item = (long) * f_pos /itemsize;
	rest = (long)*f_pos % itemsize;
	s_pos = rest/quantum;
	q_pos = rest % quantum;

	dptr = scull_follow(dev,item);
	if (dptr == NULL)  //至少应该找到一个不空的项
		goto out;

	if (!dptr->data) {  //赋上一个1000长度的指针数组
		dptr->data = kmalloc(qset * sizeof(char *),GFP_KERNEL);
		if (!dptr->data)
			goto out;
		memset(dptr->data,0,qset *sizeof (char *));
	}

	if (!dptr->data[s_pos]){//赋上4000长度的char数组（一个量子）
		dptr->data[s_pos] = kmalloc(quantum,GFP_KERNEL); 
		if (!dptr->data[s_pos])
			goto out;
	}

	if (count > quantum -q_pos)  //最多到量子尾
		count = quantum - q_pos;

	if (*f_pos + count > SCULL_DATA_SIZE)
		count = SCULL_DATA_SIZE - *f_pos;
	if (copy_from_user(dev->data + *f_pos, buf, count)) {
		retval = -EFAULT;
		goto out;
	}
	*f_pos += count;
	retval = count;
	if (dev->size < *f_pos)//只应该在写后位置超过大小时size增长；在只读打开时变为0
		dev->size = *f_pos;
out:
	up(&dev->sem);
	return retval;
}
loff_t scull_llseek(struct file *filp, loff_t off, int whence)
{
	struct scull_dev *dev = filp->private_data;
	loff_t newpos;
	switch(whence) {
		case 0: /* SEEK_SET */
			newpos = off;
			break;
		case 1: /* SEEK_CUR */
			newpos = filp->f_pos + off;
			break;
		case 2: /* SEEK_END */
			newpos = dev->size + off;
			break;
		default: /* can't happen */
			return -EINVAL;
	}
	if (newpos < 0)
		return -EINVAL;
	filp->f_pos = newpos;
	return newpos;
}
struct file_operations scull_fops = {
	.owner = THIS_MODULE,
	.llseek = scull_llseek,
	.read = scull_read,
	.write = scull_write,
	.open = scull_open,
	.release = scull_release,
};

/*
 * Finally, the module stuff
 */

/*
 * The cleanup function is used to handle initialization failures as well.
 * Thefore, it must be careful to work correctly even if some of the items
 * have not been initialized
 */
void scull_cleanup_module(void)
{
	int i;
	dev_t devno = MKDEV(scull_major, scull_minor);
	/* Get rid of our char dev entries */
	if (scull_devices) {
		for (i = 0; i < scull_nr_devs; i++) {
			scull_trim(scull_devices + i);
			cdev_del(&scull_devices[i].cdev);
		}
		kfree(scull_devices);
	}
	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, scull_nr_devs);
}
/*
 * Set up the char_dev structure for this device.
 */
static void scull_setup_cdev(struct scull_dev *dev, int index)
{
	int err, devno = MKDEV(scull_major, scull_minor + index);
	cdev_init(&dev->cdev, &scull_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &scull_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		printk(KERN_NOTICE "Error %d adding scull%d", err, index);
}
int scull_init_module(void)
{
	int result, i;
	dev_t dev = 0;
	/*
	 * Get a range of minor numbers to work with, asking for a dynamic
	 * major unless directed otherwise at load time.
	 */
	if (scull_major) {
		dev = MKDEV(scull_major, scull_minor);
		result = register_chrdev_region(dev, scull_nr_devs, "scull");
	} else {
		result = alloc_chrdev_region(&dev, scull_minor, scull_nr_devs,
				"scull");
		scull_major = MAJOR(dev);
	}
	if (result < 0) {
		printk(KERN_WARNING "scull: can't get major %d\n", scull_major);
		return result;
	}
	/*
	 * allocate the devices -- we can't have them static, as the number
	 * can be specified at load time
	 */
	scull_devices = kmalloc(scull_nr_devs * sizeof(struct scull_dev),
			GFP_KERNEL);
	if (!scull_devices) {
		result = -ENOMEM;
		goto fail; /* Make this more graceful */
	}
	memset(scull_devices, 0, scull_nr_devs * sizeof(struct scull_dev));
	/* Initialize each device. */
	for (i = 0; i < scull_nr_devs; i++) {
		scull_devices[i].quantum = scull_quantum; //ADD:
		scull_devices[i].qset = scull_qset;
		//init_MUTEX(&scull_devices[i].sem);
		sema_init(&scull_devices[i].sem,1);//since about 2.6.36? been removed
		scull_setup_cdev(&scull_devices[i], i);
	}
	return 0; /* succeed */
fail:
	scull_cleanup_module();
	return result;
}
module_init(scull_init_module);
module_exit(scull_cleanup_module);
