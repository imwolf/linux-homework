#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/proc_fs.h>
#include <linux/jiffies.h>
#include <asm/uaccess.h>
#include <linux/moduleparam.h>

#include <linux/sched.h> //task_struct等
#include <linux/pid_namespace.h>
#include <linux/fs.h>
#include <linux/time.h>

#include <linux/delay.h>

#define LOAD_INT(x) ((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1-1)) * 100)

#define MODULE_NAME "Myproc"
#define MYDATA_LEN 16

#define PMFILE "pm3file"
#define PMDIR "pm3dir"

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

//读文件myfile的读驱动函数
static int proc_read(char *page,char **start,off_t off,int count,int *eof,void *data)
{
	int len = 0;
	struct my_proc_data * mydatap = (struct my_proc_data*)data;

	if (param == 0){ //获取loadavg

		len += sprintf(page + len, "%lu.%02lu %lu.%02lu %lu.%02lu\n"/* %ld/%d %d\n*/,
				LOAD_INT(avenrun[0]), LOAD_FRAC(avenrun[0]),
				LOAD_INT(avenrun[1]), LOAD_FRAC(avenrun[1]),
				LOAD_INT(avenrun[2]), LOAD_FRAC(avenrun[2])
				/*,nr_running(), nr_threads,*/
				/*task_active_pid_ns(current)->last_pid*/);
	}else if (param == 1){
		len += sprintf(page + len,"loops_per_jiffy:%lu\n",loops_per_jiffy);
		len += sprintf(page + len,"bogomips=lpj/(500000/HZ):\n%lu.%02lu\n",loops_per_jiffy/(500000/HZ),loops_per_jiffy/(5000/HZ)%100);
	}

	return len;
}

//写文件myfile的写驱动函数
static int proc_write(struct file *file,const char* buffer,unsigned long count,void *data)
{
	/*
	int len;
	struct my_proc_data *mydatap = (struct my_proc_data *) data;
	if (count > MYDATA_LEN)
		len = MYDATA_LEN;
	else
		len = count;
	if (copy_from_user(mydatap->value,buffer,len)){//buffer 在user mode
		return -EFAULT;//?Bad address,why minus?
	}
	mydatap->value[len-1]='\0';*/
	int len;
	char buf[16];
	if (count > MYDATA_LEN)
		len = MYDATA_LEN;
	else
		len = count;
	copy_from_user(buf,buffer,len);
	loops_per_jiffy = simple_strtol(buf,NULL,10); 
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
	strcpy(mydata.value,"Ticks=");
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

