/* This is the user level program */

/* 	*******************************************************************
	PART 1 implements 50 read/write operations for  2 device rb trees
	In order to do this, user needs to input the rbtree number and the 
	IOCTL command(0-ASCENDING and 1-DESCENDING). The kprobe can be inserted 
	in the read/write with either of the devices using the following options:
	MENU: 
	rbt530_dev1 - ./main 1  0  probe_write/probe_read    
                  ./main 1  1  probe_write/probe_read           

    rbt530_dev2 - ./main 2  0  probe_write/probe_read  
                  ./main 2  1  probe_write/probe_read             		

********************************************************************* */


#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "rbt530.h"
#define WR_VALUE _IOW('k',0,int)
#define MAX       20


struct data {
			long long Time;
			pid_t Pid;
			unsigned long Address;

};


struct probe{
	int flag;
	unsigned long offset;
	char function_name[25];

};






void kprobe_write_func(char c[]){

	int fd1;
	
	/*open devices */
	fd1 = open("/dev/KPROBES", O_RDWR);

	if(fd1<0){

		printf("Can not open device file KPROBES.\n");
		return ;
	}

	else 
		printf("Opened the device file KPROBES.\n");

	struct probe p;

	
	p.flag = 1;
	p.offset = 0;
	
	if(strcmp(c,"probe_write")==0)
		strcpy(p.function_name,"rbt530_driver_write");   // write

	else
		strcpy(p.function_name,"rbt530_driver_read");

	printf("Function name:%s\n", p.function_name);

	
	int res1;
	res1 = write(fd1, &p, sizeof(struct probe));
	 if(res1<0){
 		printf("Write function failed\n");
 		//return;
 	}

 	close(fd1);
 	printf("Closing kprobe write\n");

 	return ;


}








void *kprobe_read_func(){

	int fd1,res1,i;
	struct data d;
	//struct probe kprobe;

	/*open devices */

	i=0;

	while(i<10){
	fd1 = open("/dev/KPROBES", O_RDWR);

	if(fd1<0){

		printf("Can not open device file KPROBES.\n");
		return NULL;
	}

	else 
		printf("Opened the device file KPROBES.\n");

		
	res1 = read(fd1, &d, sizeof(struct data));
	 if(res1<0)
 	printf("Write function failed\n");


    printf("kprobe values: %lld  %d %lx\n", d.Time, d.Pid, d.Address);

    close(fd1);
    i++;
}
    printf("Closing kprobe read\n");
 	



 //}
 	return NULL ;


}












/* Write function for tree1 */
void *write_func1(){


/* For Kprobes */
int fd2,i,res;
int KEY=0, DATA=40;

for(i=0;i<MAX;i++){
/*open devices */
fd2 = open("/dev/RBT5301", O_RDWR);

if(fd2<0){

	printf("Can not open device file rbt530.\n");
	return NULL;

	}



	
prb_obj_t new_obj;
new_obj = (prb_obj_t) malloc(sizeof(rb_obj_t));
memset(new_obj,0,sizeof(rb_obj_t));
	

new_obj->key = KEY++;
new_obj->data = DATA++;
res = write(fd2, new_obj, sizeof(rb_obj_t));
if(res<0)
	printf("Write function failed\n");
printf("Performing write operation\n");
	


free(new_obj);
close(fd2);
}
printf("Closing rbt530\n");
return NULL;


}




/* Write function for tree2 */

void *write_func2(){

/* For Kprobes */
int fd2,i,res;
int KEY=0, DATA= 10; 
for(i=0;i<MAX;i++){
/*open devices */
fd2 = open("/dev/RBT5302", O_RDWR);

if(fd2<0){

	printf("Can not open device file rbt530.\n");
	return NULL;

	}



	
prb_obj_t new_obj;
new_obj = (prb_obj_t) malloc(sizeof(rb_obj_t));
memset(new_obj,0,sizeof(rb_obj_t));
	
new_obj->key = KEY++;
new_obj->data = DATA++;
res = write(fd2, new_obj, sizeof(rb_obj_t));
if(res<0)
	printf("Write function failed\n");
printf("Performing write operation\n");
	

free(new_obj);
close(fd2);
}
printf("Closing rbt530\n");
return NULL;

}



void IOcontrol1(int num){

int fd,res1;
fd = open("/dev/RBT5301", O_RDWR);

if(fd<0){

	printf("Can not open device file RBT530.\n");
	return;
	}

res1 = ioctl(fd, WR_VALUE, &num);
if(res1<0){
	printf("IOCTL function failed\n");
	//return;
}


close(fd);
return;

}





/* Read function for tree1 */
void *read_func1(){

int fd,res1,i;
for(i=0;i<MAX;i++){
/*open devices */
fd = open("/dev/RBT5301", O_RDWR);

if(fd<0){

	printf("Can not open device file RBT530.\n");
	return NULL;
	}



prb_obj_t new_obj;
new_obj = (prb_obj_t) malloc(sizeof(rb_obj_t));
memset(new_obj,0,sizeof(rb_obj_t));

res1 = read(fd, new_obj, sizeof(rb_obj_t));
if(res1<0)
 	printf("Read function failed\n");
printf("Performing read operation\n");
 

free(new_obj);
close(fd);
}
printf("Closed the device file RBT5301.\n");
return NULL;

}




void IOcontrol2(int num){

int fd,res1;
fd = open("/dev/RBT5302", O_RDWR);

if(fd<0){

	printf("Can not open device file RBT530.\n");
	return;
	}

res1 = ioctl(fd, WR_VALUE, &num);
if(res1<0){
	printf("IOCTL function failed\n");
	//return;
}


close(fd);
return;

}




/* Read function for tree2 */
void *read_func2(){

int fd,res1,i;
for(i=0;i<MAX;i++){
/*open devices */
fd = open("/dev/RBT5302", O_RDWR);

if(fd<0){

	printf("Can not open device file RBT530.\n");
	return NULL;
	}

prb_obj_t new_obj;
new_obj = (prb_obj_t) malloc(sizeof(rb_obj_t));
memset(new_obj,0,sizeof(rb_obj_t));

res1 = read(fd, new_obj, sizeof(rb_obj_t));
if(res1<0)
 	printf("Read function failed\n");
printf("Performing read operation\n");
 
free(new_obj);
close(fd);
}
printf("Closed the device file RBT5302.\n");
return NULL;

}



int main(int argc, char **argv){

	int a,b;
	pthread_t tid1, tid2, tid3,tid4,tid5;
	

if(argc <=3)
	{	printf("3 arguments need to be passed. Please try again\n");
		return 0;
	}

a= atoi(argv[1]);
b= atoi(argv[2]);
//char c[]= argv[3]; 

int ret1=-1,ret2=-1;
ret1=strcmp(argv[3],"probe_write");
ret2=strcmp(argv[3],"probe_read");

if((ret1<0)&&(ret2<0)){
	printf("Invalid command for kprobe. Please try again\n");
 	return 0;
 }


 /*Read and write operations for device 1*/   
if(a==1){
	kprobe_write_func(argv[3]);
 	IOcontrol1(b);
 	pthread_create(&tid3,NULL, write_func1, NULL );
   	pthread_create(&tid4,NULL, read_func1, NULL );
   	pthread_create(&tid5,NULL,kprobe_read_func,NULL);

   }



 /*Read and write operations for device 2*/ 
if(a==2){

	kprobe_write_func(argv[3]);
	IOcontrol2(b);
 	pthread_create(&tid1,NULL, write_func2, NULL );
   	pthread_create(&tid2,NULL, read_func2, NULL );
   	pthread_create(&tid5,NULL,kprobe_read_func,NULL);


   }


 

 if(a==1){	
 pthread_join(tid3,NULL);
 pthread_join(tid4,NULL);

}


 if(a==2){	
 pthread_join(tid1,NULL);
 pthread_join(tid2,NULL);

}
 
 pthread_join(tid5,NULL);




return 0;

}







