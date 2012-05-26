#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/slab.h>

typedef struct __myevent{
	int eventNum; // 事件号
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

/*
 *建立或查找事件的系统调用，它返回一个新建或已建事件的编号
 */
asmlinkage int sys_myevent_open(int eventNum)
	/*
	 * 约定 eventNum 为 0 时建立一个以偶数号递增的新事件
	 */
{
	myevent_t *new;
	myevent_t *prev;
	int result ;
	if(eventNum) //事件号非 0,非新建
		if(!scheventNum( eventNum, &prev)) //该号事件不存在
			result = 0;
		else //返回查到的事件号
			result = eventNum;
	else{ //按递增的偶数号新建一事件,并追加到事件队列中
		new = (myevent_t *) kmalloc(sizeof(myevent_t),GFP_KERNEL);
		init_waitqueue_head(&new->p);
		new->next = NULL;
		new->p.task_list.next = &new->p.task_list;
		new->p.task_list.prev = &new->p.task_list;
		if(!lpmyevent_head){ //首次加入
			new->eventNum = 2;
			lpmyevent_head = lpmyevent_end = new;
		}
		else{ //追加到事件队列
			new->eventNum = lpmyevent_end->eventNum + 2;
			lpmyevent_end->next = new;
			lpmyevent_end = new;
		}
		result = new->eventNum;
	}
	//返回操作的结果
	return result ;
}

/*
 * 让调用进程阻塞在指定事件上的系统调用
 */
asmlinkage int sys_myevent_wait(int eventNum)
{
	myevent_t *tmp;
	myevent_t *prev = NULL;
	//取出指定事件的等待队列头指针
	if((tmp = scheventNum( eventNum, &prev)) != NULL){
		DEFINE_WAIT(wait); //初始化等待队列入口
		//使调用进程进入阻塞状态
		prepare_to_wait(&tmp->p,&wait,TASK_INTERRUPTIBLE);
		schedule(); //引发系统重新调度
		finish_wait(&tmp->p,&wait); //等待进程被唤醒,结束阻塞状态
		return eventNum;
	}
	return 0;
}

/*
 * 唤醒在指定事件上等待的进程的系统调用
 */
asmlinkage int sys_myevent_signal(int eventNum)
{
	myevent_t *tmp = NULL;
	myevent_t *prev = NULL;
	//取出指定事件的等待队列头指针
	if((tmp = scheventNum(eventNum,&prev)) != NULL) {
		wake_up(&tmp->p); //唤醒它
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
		sys_myevent_signal(eventNum); //如果阻塞则唤醒它
		kfree(releaseItem); //释放事件节点
		return eventNum;
	}
	return 0;
}


