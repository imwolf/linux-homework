#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/proc_fs.h>
#include <linux/jiffies.h>
#include <asm/uaccess.h>
#include <linux/moduleparam.h>

#include <linux/time.h>  //do_gettimeofday

/*#include <linux/sched.h>*/
/*#include <linux/interrupt.h>*/
/*#include <linux/types.h>*/
/*#include <linux/spinlock.h>*/
/*#include <linux/timer.h>*/

/*#include <asm/hardirq.h>*/

#define MODULE_NAME "Myproc"
#define MYDATA_LEN 16
#define PMDIR "pm11dir"
#define PMFILE "pm11file"

//放用户空间传入的数据
struct my_proc_data{
	char value[MYDATA_LEN];
};

struct my_proc_data mydata;

//proc结构变量
static struct proc_dir_entry *example_dir;
static struct proc_dir_entry *data_file;
static int param;
module_param(param,int,0644);

/*******************another version*******************************
static int proc_read(char *buf,char **start,off_t off,int count,int *eof,void *data)
{
	int len = 0 ;

	struct timeval  tv;
	struct timeval tv1;
	struct timespec tv2;
	unsigned long j1;
	u64 j2;

	j1 = jiffies; 
	j2 = get_jiffies_64();
	do_gettimeofday(&tv1);
	tv2 = current_kernel_time(); 
	do_gettimeofday(&tv);

	len += sprintf(buf,"0x%08lx 0x%016Lx %10i.%06i\n"
		       "%40i.%09i\n",
		       j1, j2,
		       (int) tv1.tv_sec, (int) tv1.tv_usec,
		       (int) tv2.tv_sec, (int) tv2.tv_nsec);
	*start = buf;
	return len;

}**********************************/


//读文件myfile的读驱动函数
static int proc_read(char *buf,char **start,off_t off,int count,int *eof,void *data)
{
	int len = 0 ;

	struct timeval  tv;
	unsigned long j1;
	u64 j2;

	do_gettimeofday(&tv);
	j1 = jiffies;
	j2 = get_jiffies_64();

	len += sprintf(buf + len,"%10i.%06i\n",tv.tv_sec,tv.tv_usec);
	len += sprintf(buf + len,"0x%08lx 0x%016Lx\n",j1,j2);
	*start = buf;
	return len;
}

/*************************************************
static int proc_read(char *buf,char **start,off_t off,int count,int *eof,void *data)
{
	int len = 0 ;
	unsigned long j1;
	j1 = jiffies;

	len += sprintf(buf + len,"0x%08lx\n",jiffies);
	return len;
}***************************************************/

//写文件myfile的写驱动函数
static int proc_write(struct file *file,const char* buffer,unsigned long count,void *data)
{
	int len;
	struct my_proc_data *mydatap = (struct my_proc_data *) data;
	if (count > MYDATA_LEN)
		len = MYDATA_LEN;
	else
		len = count;
	if (copy_from_user(mydatap->value,buffer,len)){//buffer 在user mode
		return -EFAULT;//?Bad address,why minus?
	}
	mydatap->value[len-1]='\0';
	return len;
}

//装入模块
int init_module(void)
{
	//创建proc/myfile目录
	example_dir = (struct proc_dir_enry *)proc_mkdir(PMDIR,NULL);
	if (example_dir == 0){
		printk ("mkdir fail\n");
		return -1;
	}
	
	data_file = (struct proc_dir_entry *)create_proc_entry(PMFILE,0666,example_dir);
	if (data_file == 0){
		remove_proc_entry(PMFILE,0);
		printk("mkfile fale\n");
		return -ENOMEM;
	}
	
	data_file->data = &mydata;
	data_file->read_proc = &proc_read;
	data_file->write_proc=&proc_write;
	//data_file->owner=THIS_MODULE; 在我的内核中该结构已经没有owner成员了
	return 0;
}

//卸载模块
void cleanup_module(void)
{
	remove_proc_entry(PMFILE,example_dir);
	remove_proc_entry(PMDIR,NULL);
	printk("Goodbye.\n");
}
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TEST");
MODULE_AUTHOR("xxx");

