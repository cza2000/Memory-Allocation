#include <stdio.h> 
#include <signal.h>     
#include <unistd.h>  
#include <sys/types.h>
#include <stdlib.h>

//1 - Set memory size (default=1024) /*设置内存的大小*/
//2 - Select memory allocation algorithm /* 设置当前的分配算法 */
//3 - New process /*创建新的进程，主要是获取内存的申请数量*/
//4 - Terminate a process /*删除进程，归还分配的存储空间，并删除描述该进程内存分配的节点*/
//5 - Display memory usage /* 显示当前内存的使用情况，包括空闲区的情况和已经分配的情况 */
//0 - Exit
#define PROCESS_NAME_LEN 32        /*进程名长度*/
#define MIN_SLICE    10           /*最小碎片的大小*/
#define DEFAULT_MEM_SIZE 1024      /*内存大小*/
#define DEFAULT_MEM_START 0        /*起始位置*/
/* 内存分配算法 */
#define MA_FF 1  //first fit
#define MA_BF 2  //best fit
#define MA_WF 3  //worst fit
int mem_size=DEFAULT_MEM_SIZE;     /*内存大小*/
int ma_algorithm = MA_FF;          /*当前分配算法*/
static int pid = 0;                /*初始pid*/
int flag = 0;                      /*设置内存大小标志*/
//【第一个错误修正】
/*指向内存中空闲块链表的首指针*/
struct free_block_type *free_block;
//【第二个错误修正】
//函数声明
struct free_block_type* init_free_block(int mem_size);

int main(){
    char choice = '1';      pid=0;
    free_block = init_free_block(mem_size); //初始化空闲区
    while(1) {
    
    display_menu();	//显示菜单
    fflush(stdin);
    choice=getchar();	//获取用户输入
    while(choice=='\n')
    	choice = getchar();
    switch(choice){
        case '1': set_mem_size(); break; 	//设置内存大小
        case '2': set_algorithm();flag=1; break;//设置算法
        case '3': new_process(); flag=1; break;//创建新进程
        case '4': kill_process(); flag=1;   break;//删除进程
        case '5': display_mem_usage();    flag=1; break;	//显示内存使用
        case '0': do_exit(); exit(0);	//释放链表并退出
        default: break;      }  } 
	return 0;
}

/* 描述每一个空闲块的数据结构 */
struct free_block_type{
    int size;   
    int start_addr;
    struct free_block_type *next;
};  

/*每个进程分配到的内存块的描述*/
struct allocated_block{
    int pid;    int size;
    int start_addr;
    char process_name[PROCESS_NAME_LEN];
    struct allocated_block *next;
    };
/*进程分配内存块链表的首指针*/
struct allocated_block *allocated_block_head = NULL;

/*初始化空闲块，默认为一块，可以指定大小及起始地址*/
struct free_block_type* init_free_block(int mem_size){
    struct free_block_type *fb;

    fb=(struct free_block_type *)malloc(sizeof(struct free_block_type));
    if(fb==NULL){
        printf("No mem\n");
        return NULL;
        }
    fb->size = mem_size;
    fb->start_addr = DEFAULT_MEM_START;
    fb->next = NULL;
    return fb;
}
/*显示菜单*/
void display_menu(){
    printf("\n");
    printf("1 - Set memory size (default=%d)\n", mem_size);
    printf("2 - Select memory allocation algorithm\n");
    printf("3 - New process \n");
    printf("4 - Terminate a process \n");
    printf("5 - Display memory usage \n");
    printf("0 - Exit\n");
}
/*设置内存的大小*/
int set_mem_size(){
    int size;
    if(flag!=0){  //防止重复设置
        printf("Cannot set memory size again\n");
        return 0;
        }
    printf("Total memory size =");
    scanf("%d", &size);
    if(size>0) {
        mem_size = size;
        free_block->size = mem_size;
        }
    flag=1;  return 1;
    }
/* 设置当前的分配算法 */
void set_algorithm(){
    int algorithm;
    printf("\t1 - First Fit\n");
    printf("\t2 - Best Fit \n");
    printf("\t3 - Worst Fit \n");
    scanf("%d", &algorithm);
    if(algorithm>=1 && algorithm <=3)  
              ma_algorithm=algorithm;
	//按指定算法重新排列空闲区链表
    rearrange(ma_algorithm); 
}
/*按指定的算法整理内存空闲块链表*/
void rearrange(int algorithm){
    switch(algorithm){
        case MA_FF:  rearrange_FF(); break;
        case MA_BF:  rearrange_BF(); break;
        case MA_WF:  rearrange_WF(); break;
        }
}
/*按FF算法重新整理内存空闲块链表*/
void rearrange_FF(){   //按照地址从小到大排序
	struct free_block_type *pb,*pf,temp;
	pf = free_block;
	if(free_block == NULL)  //空闲块链表为空
		return;
	if(free_block->next == NULL)   //链表仅有一个节点
		return;
	while(pf->next!= NULL){
		pb = pf->next;
		while(pb!=NULL){
			if(pf->start_addr > pb->start_addr){
				temp = *pf;
				*pf = *pb;
				*pb = temp;
				temp.next = pf->next;
				pf->next = pb->next;
				pb->next = temp.next;
			}
			pb = pb->next;
		}
		pf = pf->next;
	}
	return;
}
/*按BF算法重新整理内存空闲块链表*/
void rearrange_BF(){   //按照块大小从小到大排序
	struct free_block_type *pb,*pf,temp;
	pf = free_block;
	if(free_block == NULL)  //空闲块链表为空
		return;
	if(free_block->next == NULL)   //链表仅有一个节点
		return;
	while(pf->next!= NULL){
		pb = pf->next;
		while(pb!=NULL){
			if((pf->size) > (pb->size)){
				temp = *pf;
				*pf = *pb;
				*pb = temp;
				temp.next = pf->next;
				pf->next = pb->next;
				pb->next = temp.next;
			}
			pb = pb->next;
		}
		pf =pf->next;
	}
	return;
}
/*按WF算法重新整理内存空闲块链表*/
void rearrange_WF(){   //按照块大小从大到小排序
	struct free_block_type *pb,*pf,temp;
	pf = free_block;
	if(free_block == NULL)  //空闲块链表为空
		return;
	if(free_block->next == NULL)   //链表仅有一个节点
		return;
	while(pf->next!= NULL){
		pb = pf->next;
		while(pb!=NULL){
			if(pf->size < pb->size){
				temp = *pf;
				*pf = *pb;
				*pb = temp;
				temp.next = pf->next;
				pf->next = pb->next;
				pb->next = temp.next;
			}
			pb = pb->next;
		}
		pf =pf->next;
	}
	return;
}
/*创建新的进程，主要是获取内存的申请数量*/
int new_process(){
    struct allocated_block *ab;
    int size;    int ret;
    ab=(struct allocated_block *)malloc(sizeof(struct allocated_block));
    //printf("已经分配空间");
    if(!ab) exit(-5);
    ab->next = NULL;
    pid++;
    sprintf(ab->process_name, "PROCESS-%02d", pid);
    ab->pid = pid;    
    printf("Memory for %s:", ab->process_name);
    scanf("%d", &size);
    if(size>0) ab->size=size;
    ret = allocate_mem(ab);  /* 从空闲区分配内存，ret==1表示分配ok*/
 /*如果此时allocated_block_head尚未赋值，则赋值*/
    if((ret==1) &&(allocated_block_head == NULL)){ 
        allocated_block_head=ab;
        return 1;        }
    /*分配成功，将该已分配块的描述插入已分配链表*/
    else if (ret==1) {
        ab->next=allocated_block_head;
        allocated_block_head=ab;
        return 2;        }
    else if(ret==-1){ /*分配不成功*/
        printf("Allocation fail\n");
        free(ab);
        return(-1);       
     }
    return 3;
    }
/*分配内存模块*/
int allocate_mem(struct allocated_block *ab){
    struct free_block_type *fbt, *pre;
    int request_size=ab->size;
    fbt = pre = free_block;
	if(free_block == NULL)  //没有空闲块，分配失败
		return(-1);
	if(free_block->next == NULL && free_block->size < request_size)   //仅有一个空闲块且大小不满足，分配失败
		return(-1);
	int sum = 0;
	while(fbt != NULL){
		//1.找到可满足空闲分区且分配后剩余空间足够大，则分割
		if((fbt->size)>(ab->size) && (fbt->size)-(ab->size)>=MIN_SLICE){
			ab->start_addr = fbt->start_addr;
			fbt->start_addr = fbt->start_addr + ab->size;
			fbt->size = fbt->size - ab->size;
			rearrange(ma_algorithm);
			return 1;
		}
		//2.找到可满足空闲分区且但分配后剩余空间比较小，则一起分配
		if(fbt->size > ab->size && (fbt->size)-(ab->size)<MIN_SLICE){
			ab->start_addr = fbt->start_addr;
			ab->size = fbt->size;
			if(free_block->next == NULL)
				free_block = NULL;
			//空闲块链表中删除该节点
			if(pre==free_block)
				free_block = free_block->next;
			else pre->next = fbt->next;
			if(fbt->next == NULL)
				fbt = NULL;
			rearrange(ma_algorithm);
			return 1;	
		}
		//该空闲块小于需要的内存
		sum = sum + fbt->size;
		pre = fbt;
		fbt = pre->next;		
	}
	//3.找不可满足需要的空闲分区但空闲分区之和能满足需要，则采用内存紧缩技术，进行空闲分区的合并，然后再分配
	if(sum >= ab->size){
	struct allocated_block *ee,*nn;
	ee = allocated_block_head;
	nn = ee;
	//对已分配分区按照地址排序
	struct allocated_block *pb,*pf,temp;
	pf = allocated_block_head;
	if(allocated_block_head == NULL)  //空闲块链表为空
		return;
	if(allocated_block_head == NULL)   //链表仅有一个节点
		return;
	while(pf->next!= NULL){
		pb = pf->next;
		while(pb!=NULL){
			if(pf->start_addr > pb->start_addr){
				temp = *pf;
				*pf = *pb;
				*pb = temp;
				temp.next = pf->next;
				pf->next = pb->next;
				pb->next = temp.next;
			}
			pb = pb->next;
		}
		pf = pf->next;
	}
	//采用内存紧缩技术，已分配区都向前移动
	while(nn->next != NULL){
		ee = nn;
		nn = nn->next;
		if(nn->start_addr != (ee->start_addr + ee->size))
			nn->start_addr = ee->start_addr + ee->size;
	}
	//重分配空闲区
	free_block->start_addr = nn->start_addr + nn->size;
	free_block->size = sum;
		//找到可满足空闲分区且分配后剩余空间足够大，则分割
		if(sum-(ab->size)>MIN_SLICE){
			ab->start_addr = free_block->start_addr;
			free_block->start_addr = free_block->start_addr + ab->size;
			free_block->size = free_block->size - ab->size;
			rearrange(ma_algorithm);
			return 1;
		}
		//2.找到可满足空闲分区且但分配后剩余空间比较小，则一起分配
		if(sum-(ab->size)<=MIN_SLICE){
			ab->start_addr = free_block->start_addr;
			free_block = NULL;
			//空闲块链表中删除该节点
			rearrange(ma_algorithm);
			return 1;	
		}
    }
	return(-1); //分配失败
}
//函数声明
struct allocated_block * find_process(int request_pid);
/*删除进程，归还分配的存储空间，并删除描述该进程内存分配的节点*/
kill_process(){
    struct allocated_block *ab;
    int pid;
    printf("Kill Process, pid=");
    scanf("%d", &pid);
    ab = find_process(pid);
    if(ab!=NULL){
        free_mem(ab); /*释放ab所表示的分配区*/
        dispose(ab);  /*释放ab数据结构节点*/
        }
}
struct allocated_block * find_process(int request_pid){
	struct allocated_block *tem;
	tem = allocated_block_head;
	while(tem != NULL)
		if(tem->pid == request_pid)
			return tem;
		else tem = tem->next;
}
/*将ab所表示的已分配区归还，并进行可能的合并*/
int free_mem(struct allocated_block *ab){
    int algorithm = ma_algorithm;
    struct free_block_type *fbt, *pre, *work;
    pre = free_block;

   	fbt=(struct free_block_type*) malloc(sizeof(struct free_block_type));
   	if(free_block == NULL)
    {
    	free_block = fbt;
    	fbt->start_addr = ab->start_addr;
    	fbt->size = ab->size;
    	fbt->next = NULL;
    	return 1;
    }
    if(!fbt) return(-1);
    // 进行可能的合并，基本策略如下
    // 1. 将新释放的结点插入到空闲分区队列末尾
    fbt->start_addr = ab->start_addr;
    fbt->size = ab->size;
    fbt->next = NULL;
    
    while(pre->next != NULL) pre = pre->next;
    pre->next = fbt;
    // 2. 对空闲链表按照地址有序排列
    rearrange_FF();
    // 3. 检查并合并相邻的空闲分区
    pre = free_block;
    while(pre != NULL){
    	work = pre->next;
    	if(work == NULL)
    		break;
    	if(work->start_addr <= (pre->start_addr + pre->size) )
    	{
    		pre->size = pre->size + work->size;
    		pre->next = work->next;
    	}
    	else pre = pre->next;
    }
    // 4. 将空闲链表重新按照当前算法排序 
    rearrange(algorithm);
    return 1;
}
/*释放ab数据结构节点*/
int dispose(struct allocated_block *free_ab){
    struct allocated_block *pre, *ab;
   	if(free_ab == allocated_block_head) { /*如果要释放第一个节点*/
     allocated_block_head = allocated_block_head->next;
        free(free_ab);
        return 1;
        }
    pre = allocated_block_head;  
    ab = allocated_block_head->next;
    while(ab!=free_ab){ pre = ab;  ab = ab->next; }
    pre->next = ab->next;
    free(ab);
    return 2;
   }
/* 显示当前内存的使用情况，包括空闲区的情况和已经分配的情况 */
int display_mem_usage(){
    struct free_block_type *fbt=free_block;
    struct allocated_block *ab=allocated_block_head;
    if(fbt==NULL) {
    	printf("当前无空闲区块！\n");
    //return(-1);
    }
    printf("----------------------------------------------------------\n");

    /* 显示空闲区 */
    printf("Free Memory:\n");
    printf("%20s %20s\n", "      start_addr", "       size");
    while(fbt!=NULL){
        printf("%20d %20d\n", fbt->start_addr, fbt->size);
        fbt=fbt->next;
        } 
/* 显示已分配区 */
    printf("\nUsed Memory:\n");
    printf("%10s %20s %10s %10s\n", "PID", "ProcessName", "start_addr", " size");
    while(ab!=NULL){
        printf("%10d %20s %10d %10d\n", ab->pid, ab->process_name, ab->start_addr, ab->size);
        ab=ab->next;
        }
    printf("----------------------------------------------------------\n");
    return 0;
    }   

void do_exit(){
	//释放空闲区列表
	struct free_block_type *fbt,*temp;
	fbt = free_block;
	if(fbt!=NULL){
		temp = fbt->next;
		free(fbt);
		fbt = temp;
	}
}



