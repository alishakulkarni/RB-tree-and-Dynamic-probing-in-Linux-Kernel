/* This is the user level program */

/* 	*******************************************************************
	PART 1 implements 50 read/write operations for  2 device rb trees
	In order to do this, user needs to input the rbtree number and the 
	IOCTL command(0-ASCENDING and 1-DESCENDING)
	MENU: 
	rbt530_dev1  - ./main 1  0      (ASCENDING)
                       ./main 1  1      (DESCENDING)

       rbt530_dev2  - ./main 2  0      (ASCENDING)
                     ./main 2  1      (DESCENDING)           		

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
#define MAX       50




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
	
}


close(fd);
return;

}





/* Read function for tree1 */
void *read_func1(){

int fd,res1,i;
for( i=0;i<MAX;i++){
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
	pthread_t tid1, tid2, tid3,tid4;
	

if(argc <=1)
	{	printf("3 arguments need to be passed. Please try again\n");
		return 0;
	}

a= atoi(argv[1]);
b= atoi(argv[2]);
    

 /*Read and write operations for device 1*/   
if(a==1){

 	IOcontrol1(b);
 	pthread_create(&tid3,NULL, write_func1, NULL );
   	pthread_create(&tid4,NULL, read_func1, NULL );
   }


 /*Read and write operations for device 2*/ 
if(a==2){


	IOcontrol2(b);
 	pthread_create(&tid1,NULL, write_func2, NULL );
   	pthread_create(&tid2,NULL, read_func2, NULL );
   }


 

 if(a==1){	
 pthread_join(tid3,NULL);
 pthread_join(tid4,NULL);

}


 if(a==2){	
 pthread_join(tid1,NULL);
 pthread_join(tid2,NULL);

}
 



return 0;

}







