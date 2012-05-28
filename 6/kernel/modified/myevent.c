#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include <asm/atomic.h>

/////////////////////////////////////////////////////////////////////////
//myevent
//TODO:结构体中增加信号量的资源数  int value DONE
typedef struct __myevent{
	int eventNum; // 事件号
	atomic_t value;	
	wait_queue_head_t p; // 系统等待队列首指针
	struct __myevent *next; // 队列链指针
}myevent_t;


myevent_t * lpmyevent_head = NULL ; // 链头指针
myevent_t * lpmyevent_end = NULL ; // 链尾指针

/*
 *查找事件，失败返回NULL
 */
myevent_t * scheventNum(int eventNum, myevent_t **prev)
{
	myevent_t *tmp = lpmyevent_head;
	*prev = NULL;
	while(tmp){
		if(tmp->eventNum == eventNum)
			return tmp;
		*prev = tmp;
		tmp = tmp->next;
	}
	return NULL;
}

//TODO:系统调用得增加一个参数，信号量的资源数，只在新建的时候给新的结构体赋值  DONE
/*
 *建立或查找事件的系统调用，它返回一个新建或已建事件的编号
 */
asmlinkage int sys_myevent_open(int eventNum,int value)
	/*
	 * 约定 eventNum 为 0 时建立一个以偶数号递增的新事件
	 */
{
	myevent_t *new_event = 0;
	myevent_t *prev = 0;
	int result ;
	if(eventNum) //事件号非 0,非新建
		if(!scheventNum( eventNum, &prev)) //该号事件不存在
			result = 0;
		else //返回查到的事件号
			result = eventNum;
	else{ //按递增的偶数号新建一事件,并追加到事件队列中
		new_event = (myevent_t *) kmalloc(sizeof(myevent_t),GFP_KERNEL);
		init_waitqueue_head(&new_event->p);
		new_event->next = NULL;
		new_event->p.task_list.next = &new_event->p.task_list;
		new_event->p.task_list.prev = &new_event->p.task_list;
		if(!lpmyevent_head){ //首次加入
			new_event->eventNum = 2;
			printk("[open]:first open-->eventNum:<%u> == 2?",new_event->eventNum);
			lpmyevent_head = lpmyevent_end = new_event;
		}
		else{ //追加到事件队列
			new_event->eventNum = lpmyevent_end->eventNum + 2;
			lpmyevent_end->next = new_event;
			lpmyevent_end = new_event;
		}
		atomic_set(&new_event->value,value);  //YJ:注意这里没有对value进行检验,约定大于等于0；0的话与原来的功能一样
		printk("[open]:eventNum:%u result:%u value:%u \n",new_event->eventNum,result,atomic_read(&new_event->value));
		result = new_event->eventNum;
	}
	//返回操作的结果
	return result ;
}

//TODO:如果资源计数已经为0，那么进程只能挂到等待队列上等待；
//	如果资源计数大于0，那么就减少1个计数，调用立即返回 DONE
/*
 * 让调用进程阻塞在指定事件上的系统调用
 */
asmlinkage int sys_myevent_wait(int eventNum)
{
	myevent_t *tmp;
	myevent_t *prev = NULL;

	//取出指定事件的等待队列头指针?只是在事件队列上找到对应的事件吧，然后把该事件上的等待队列头指针取出来用
	if((tmp = scheventNum( eventNum, &prev)) != NULL){
		printk("[wait]:value is %u",atomic_read(&tmp->value));
		if (atomic_read(&tmp->value) > 0){   //YJ:有可用资源，减1并立即返回；不然等待
			atomic_dec(&tmp->value);
			printk("[wait]:i've dec value to <%u>",atomic_read(&tmp->value));
			return eventNum;
		}
		printk("[wait]:value should be 0 to sleep-->value:%u\n",atomic_read(&tmp->value));

		DEFINE_WAIT(wait); //初始化等待队列入口
		//使调用进程进入阻塞状态
		//prepare_to_wait(&tmp>p,&wait,TASK_INTERRUPTIBLE);
		set_current_state(TASK_INTERRUPTIBLE);
		add_wait_queue_exclusive(&tmp->p,&wait);  //独占等待,放到队尾并置标志
		schedule(); //引发系统重新调度
		finish_wait(&tmp->p,&wait); // 设置当前进程状态为RUNNING,并且从队列中删除之（如果队列非空）
		printk("[wait]:now i'm back and value is :%u\n",atomic_read(&tmp->value));
		return eventNum;
	}
	return 0;
}

//TODO:V操作，将该事件的资源计数+1，然后如果等待队列上有进程在等待，就唤醒它，否则为NULL，就什么也不用做了
//具体 DONE
/*
 * 唤醒在指定事件上等待的进程的系统调用
 */
asmlinkage int sys_myevent_signal(int eventNum)
{
	myevent_t *tmp = NULL;
	myevent_t *prev = NULL;
	//取出指定事件的等待队列头指针
	if((tmp = scheventNum(eventNum,&prev)) != NULL) {
		if (list_empty(&(tmp->p.task_list))) {  //没有进程在队列上
			atomic_inc(&tmp->value);
			printk("[signal]:so list is empty and value now is(added):%u\n",atomic_read(&tmp->value));
			return eventNum;		
		}
		
		//唤醒操作,由于独占等待，只会唤醒一个进程，而且DEFINE_WAIT时挂上了autoremove_wake_up方法，进程会自动从队列上删除，wake_up和add_wait_queue这些都自动加spinlock了
		printk("[signal]:so i'm going to wake up one exclusive process\n");
		wake_up(&tmp->p); 
		return eventNum;
	}
	return 0;
}

/*
 * 撤销指定事件的系统调用
 */
asmlinkage int sys_myevent_close(int eventNum)
{
	myevent_t *prev=NULL;
	myevent_t *releaseItem;
	if((releaseItem = scheventNum(eventNum,&prev)) != NULL){
		//找到指定事件
		if( releaseItem == lpmyevent_end) //在队尾
			lpmyevent_end = prev;
		else if(releaseItem == lpmyevent_head) //在队首
			lpmyevent_head = lpmyevent_head->next;
		else //在队中
			prev->next = releaseItem->next;
		wake_up_all(&releaseItem->p);  //YJ:唤醒阻塞到该事件上的所有进程
		kfree(releaseItem); //释放事件节点
		printk("[close]:event %u should have been closed\n",eventNum);
		return eventNum;
	}
	return 0;
}



