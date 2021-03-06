#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/jiffies.h>
#include <asm/uaccess.h>
#include <linux/moduleparam.h>

#include <linux/sched.h> //task_struct等
/*#include <linux/list.h>*/

#define MODULE_NAME "Myproc"
#define MYDATA_LEN 16
#define PMDIR "pm2dir"
#define PMFILE "pm2file"

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
	struct task_struct *task = current;
	struct my_proc_data * mydatap = (struct my_proc_data*)data;
	len += sprintf(page + len,"%s%ld HZ=%d\n",mydatap->value,jiffies,HZ);

	len += sprintf(page+len,"遍历父进程\npid\tppid\tcomm\n");
	while (task != &init_task){
		len += sprintf(page + len,"%d\t%d\t[%s]\n",task->pid,task->parent->pid,task->comm);
		task = task->parent;
	}
	/* task now points to init*/
	len += sprintf(page + len,"%d\t%d\t[%s]\n",task->pid,task->parent->pid,task->comm);
	/*len += sprintf(page + len,"init:%d\t[%s]\n",(&init_task)->pid,(&init_task)->comm);*/

	len += sprintf(page + len,"遍历任务队列\ncomm\tpid\n");
	for_each_process(task){
		/* this poinlessly prints the name and pid of each task*/
		/*printk("%s[%d]",task->comm,task->pid);*/
		len += sprintf(page + len,"%d\t[%s]\n",task->pid,task->comm);
	}
	return len;
}

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
		remove_proc_entry("myfile",0);
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

