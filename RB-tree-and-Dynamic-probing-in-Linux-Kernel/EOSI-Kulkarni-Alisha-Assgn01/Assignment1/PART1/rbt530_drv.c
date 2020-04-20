/**************RBT530 Driver Code********************/
/*This driver performs read and write operations for 2 device RB trees based 
on the requirements in the assignment. */


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/errno.h>
#include<linux/init.h>
#include<linux/moduleparam.h>
#include <linux/rbtree.h>
#include <linux/mutex.h>
#include <linux/ioctl.h>
#include "rbt530.h"




#define DEVICE_NAME            "RBT530"
#define DEVICE_NAME1           "RBT5301"
#define DEVICE_NAME2           "RBT5302"

#define ASCENDING       0
#define DESCENDING      1

#define WR_VALUE _IOW('k', 0, int)  



/*struct for the RB tree node*/
typedef struct rb530_node{

rb_obj_t rb_obj;
struct rb_node rb_node_new;
}rb530_node,*rb530_obj_node;



/* per device structure */
struct rbt530_dev{
	struct cdev cdev;
	char name[20];
	rb530_obj_node rb530_obj_node_new;
	struct rb_root the_root;
	struct rb_node *index;
	int val;
	struct mutex lock;
	
}*rbt530_devp1,*rbt530_devp2;


static dev_t rbt530_dev_number1,rbt530_dev_number2;    /*Allocate device number*/ 
struct class *rbt530_dev_class;    /*Tie with the device model*/
static struct device *rbt530_dev_device1,*rbt530_dev_device2;
static char *user_name = "Dear John"; /* the default user name, for insmod command */
module_param(user_name, charp,0000);    //to get parameter from load.sh script to greet the user



/* Open the rbt530 driver */

int rbt530_driver_open(struct inode *inode, struct file *file){



	struct rbt530_dev *rbt530_devp;

	
 	/*Get the per-device structure that contains this cdev */
	rbt530_devp = container_of(inode->i_cdev, struct rbt530_dev, cdev);


	/* Easy access to cmos_devp from rest of the entry points */
	file->private_data = rbt530_devp;
	printk("\n%s is opening \n", rbt530_devp->name);
	return 0;

}



/* Close the device file */
int rbt530_driver_release(struct inode *inode, struct file *file){

struct rbt530_dev *rbt530_devp = file->private_data;

printk("\n%s is closing \n", rbt530_devp->name);

return 0;

}





 /* Insert helper function for the write file operation */ 
 void my_rb_insert(struct rb_root *root, rb530_obj_node rb530_obj)
    {
        struct rb_node **link = &root->rb_node, *parent=NULL;
	int value = rb530_obj->rb_obj.key;
	rb530_obj_node objs;

	// /* Go to the bottom of the tree */
	while (*link)
	 {	
	    parent = *link;
	    objs = container_of(parent, struct rb530_node, rb_node_new);

	    if (objs->rb_obj.key > value)
	 	link = &(*link)->rb_left;
	     else
	 	link = &(*link)->rb_right;
	 }

	// /* Put the new node there */
	 rb_link_node(&rb530_obj->rb_node_new, parent, link);
	 rb_insert_color(&rb530_obj->rb_node_new, root);
    }





    /* Search helper function for the write file operation */ 
    rb530_obj_node my_rb_search(struct rb_root *root, rb_obj_t key_data)
    {
        struct rb_node *node = root->rb_node;  /* top of the tree */
        rb530_obj_node stuff;

        while (node)
	{
	    stuff = container_of(node, struct rb530_node, rb_node_new);
	   

	     if (stuff->rb_obj.key > key_data.key)
		 node = node->rb_left;
	    else if (stuff->rb_obj.key < key_data.key)
		 node = node->rb_right;
	    else
		 return stuff;  /* Found it */
  	}
	return NULL;
    }


/* Write nodes into the device tree with data-value */
static ssize_t rbt530_driver_write(struct file *file, const char *buf,
           size_t count, loff_t *ppos){

		int flag2 =0, flag1=0;
		/*Create an object of device */
		struct rbt530_dev *rbt530_devp; 
		rb530_obj_node pnew_rbt530_obj_node = NULL;
		rb530_obj_node search;


		rbt530_devp= file->private_data;
		mutex_lock(&rbt530_devp->lock);

		 if(!(pnew_rbt530_obj_node=kmalloc(sizeof(struct rb530_node), GFP_KERNEL))){
		 	printk("Bad Malloc\n");
		 	mutex_unlock(&rbt530_devp->lock);
		 	return -ENOMEM;
		 }



		/*Initialise new node to 0 */

		  if(copy_from_user(&(pnew_rbt530_obj_node->rb_obj),(char *)buf,count)){

		 		mutex_unlock(&rbt530_devp->lock);	
		 		return -EFAULT;
		 }
		 	


	

		 printk("Performing write: Key value = %d, Data value = %d\n", pnew_rbt530_obj_node->rb_obj.key, pnew_rbt530_obj_node->rb_obj.data);
		 //mutex_unlock(&rbt530_devp->mutex);

		   //if(RB_EMPTY_ROOT(&(rbt530_devp->the_root))){
		 	search =  my_rb_search(&(rbt530_devp->the_root),pnew_rbt530_obj_node->rb_obj); //non zero value 
		 	//mutex_lock(&rbt530_devp->mutex);

			if((pnew_rbt530_obj_node->rb_obj.data!=0) && (search!=NULL))	
			{ 	
		   		   	//my_rb_insert(&(rbt530_devp->the_root), pnew_rbt530_obj_node);
				    search->rb_obj.data = pnew_rbt530_obj_node->rb_obj.data;
				    flag1 =1;
				    printk("Data is being replaced\n");
				   // kfree(pnew_rbt530_obj_node);

		   }


		   	else if ((pnew_rbt530_obj_node->rb_obj.data==0) && (search!=NULL))
		   	{
		   		rb_erase(&(search->rb_node_new),&(rbt530_devp->the_root));
		   		kfree(search);

		   		printk("Data is being deleted\n");
		   		flag2 = 1;

		   		kfree(pnew_rbt530_obj_node);
				mutex_unlock(&rbt530_devp->lock);
				return -4;


		   	}


		   	if((pnew_rbt530_obj_node->rb_obj.data!=0)&&(flag1==0)&&(flag2==0))
		   		printk("Data is being inserted\n");
			my_rb_insert(&(rbt530_devp->the_root), pnew_rbt530_obj_node);


			mutex_unlock(&rbt530_devp->lock);
			
			
		 

		   	
		   return 0;

}

EXPORT_SYMBOL(rbt530_driver_write);






/* Read nodes into the device tree with data-value */
 ssize_t rbt530_driver_read(struct file *file,  char *buf,
           size_t count, loff_t *ppos){

		
// 		Create an object of device 
 		struct rbt530_dev *rbt530_devp = file->private_data;
 		rb530_obj_node new= NULL;
 		struct rb_node *temp = NULL;
 		mutex_lock(&rbt530_devp->lock);
 		printk("\n%s Name input from read\n", rbt530_devp->name);


 		if(rbt530_devp->index == NULL){
		printk("Next element is NULL\n");
		printk(" %d \n", rbt530_devp->val);
		mutex_unlock(&rbt530_devp->lock);
		return -EINVAL; 
	}

		new = container_of((rbt530_devp->index), struct rb530_node, rb_node_new );
 		printk(" %d %d \n", new->rb_obj.data,  new->rb_obj.key);
 		copy_to_user((char*)buf, &(new->rb_obj), count);

 		if(rbt530_devp->val==ASCENDING){
 			temp = rb_next(rbt530_devp->index);
 			if(temp!=NULL){
 				rbt530_devp->index = temp;
 			}
		}
 		else if(rbt530_devp->val==DESCENDING){
 			temp = rb_prev(rbt530_devp->index);
 			if(temp!=NULL) rbt530_devp->index = temp;

 		}
 		else{
 			printk("Invalid command\n ");
 		}
 		 //rbt530_devp->the_root = RB_ROOT;
 		

 		//kfree(new);
 		mutex_unlock(&rbt530_devp->lock);



 		return 0;
 }


EXPORT_SYMBOL(rbt530_driver_read);


/* IOCTL for user to choose ascending or descending order */
long rbt530_driver_ioctl(struct file *file, unsigned int cmd, unsigned long arg ){

	struct rbt530_dev *rbt530_devp = file->private_data;
	int new;

	printk("IOCTL\n");

	mutex_lock(&rbt530_devp->lock);
	
	switch(cmd){

		case WR_VALUE:			/* READ_ORDER */
		copy_from_user((int*)&new,(int*)arg, sizeof(new));
		if((new != ASCENDING)&&(new!= DESCENDING)){
				mutex_unlock(&rbt530_devp->lock);
				return -EINVAL;
		}		
		else{

			if(rbt530_devp->the_root.rb_node == NULL){
					printk(" NO ELEMENTS IN ARRAY\n");
					printk(" %d \n", rbt530_devp->val);
					mutex_unlock(&rbt530_devp->lock);
					return -EINVAL; 

		}


		rbt530_devp->val =new;

		if(rbt530_devp->val==ASCENDING){
			printk("ASCENDING\n");
			rbt530_devp->index = rb_first(&rbt530_devp->the_root);

		}


		else if(rbt530_devp->val==DESCENDING){
			printk("DESCENDING\n");
			rbt530_devp->index = rb_last(&rbt530_devp->the_root);

		}


		printk(" Command change %d \n", rbt530_devp->val);


		}

		break;
		default:
		mutex_unlock(&rbt530_devp->lock);
		return -ENOIOCTLCMD;
		break;


	}


mutex_unlock(&rbt530_devp->lock);
return 0;

}




/* File Operations Structure. Defined in linux/fs.h */
static struct file_operations rbt530_fops = {

	.owner           =THIS_MODULE,
	.open            =rbt530_driver_open,
	.release         =rbt530_driver_release,
	.write           =rbt530_driver_write,
	.read            =rbt530_driver_read,
	.unlocked_ioctl  =rbt530_driver_ioctl,

};






static int rbt530_driver_init(void){

	int ret;
	

	/*Request dynamic allocation of a device major number */
	if(alloc_chrdev_region(&rbt530_dev_number1,0,1, DEVICE_NAME1)<0){
			printk("Can't register device\n");
			return -1;
	}

/*Request dynamic allocation of a device major number */
	if(alloc_chrdev_region(&rbt530_dev_number2,0,2, DEVICE_NAME2)<0){
			printk("Can't register device\n");
			return -1;
	}

		/*Populate sysfs entries*/
	rbt530_dev_class = class_create(THIS_MODULE, DEVICE_NAME);



	

	/*Allocate memory for the per-device structure */
	rbt530_devp1 = kmalloc(sizeof(struct rbt530_dev), GFP_KERNEL);


	if(!rbt530_devp1){

			printk("Bad kmalloc\n");
			return -ENOMEM;
	}

	/*Allocate memory for the per-device structure */
	rbt530_devp2 = kmalloc(sizeof(struct rbt530_dev), GFP_KERNEL);


	if(!rbt530_devp2){

			printk("Bad kmalloc\n");
			return -ENOMEM;
	}


	rbt530_devp1->the_root = RB_ROOT;
	rbt530_devp2->the_root = RB_ROOT;

	/* Request I/O region */
	sprintf(rbt530_devp1->name, DEVICE_NAME1);

	sprintf(rbt530_devp2->name, DEVICE_NAME2);


	/* Connect the file operations with cdev */
	cdev_init(&rbt530_devp1->cdev, &rbt530_fops);
	rbt530_devp1->cdev.owner = THIS_MODULE;

	cdev_init(&rbt530_devp2->cdev, &rbt530_fops);
	rbt530_devp2->cdev.owner = THIS_MODULE;


	/* Initialise mutexes */
	mutex_init(&rbt530_devp1->lock);
	mutex_init(&rbt530_devp2->lock);

	rbt530_devp1->val = -1; 
	rbt530_devp2->val = -1; 


	/*Connect the major/minor number to the cdev*/

	ret = cdev_add(&rbt530_devp1->cdev, (rbt530_dev_number1),1);

	if(ret){
		printk("Bad cdev\n");
		return ret;
	}


		ret = cdev_add(&rbt530_devp2->cdev, (rbt530_dev_number2),1);

	if(ret){
		printk("Bad cdev\n");
		return ret;
	}


	/* Send uevents to udev, so it'll create /dev nodes */

	rbt530_dev_device1 = device_create(rbt530_dev_class, NULL, MKDEV(MAJOR(rbt530_dev_number1),0), NULL, DEVICE_NAME1);
	rbt530_dev_device2 = device_create(rbt530_dev_class, NULL, MKDEV(MAJOR(rbt530_dev_number2),0), NULL, DEVICE_NAME2);

	////time_since_boot = (jiffies-INITIAL_JIFFIES)/HZ;
	//sema_init(&rbt530_devp->sem,1);
	//mutex_init(&rbt530_devp->mutex);
	printk("rbt530 1 driver initialised.\n");
	printk("rbt530 2 driver initialised.\n" );

	
	return 0;
	
	
}



/* Driver Exit */

static void rbt530_driver_exit(void){

	/* Release the major number */
	unregister_chrdev_region((rbt530_dev_number1),2);
	unregister_chrdev_region((rbt530_dev_number2),2);


	/*Destroy Device */
	device_destroy(rbt530_dev_class, MKDEV(MAJOR(rbt530_dev_number1),0));
	device_destroy(rbt530_dev_class, MKDEV(MAJOR(rbt530_dev_number2),0));
	cdev_del(&rbt530_devp1->cdev);
	cdev_del(&rbt530_devp2->cdev);
	
	kfree(rbt530_devp1);
	kfree(rbt530_devp2);

	/* Destroy driver_class */
	class_destroy(rbt530_dev_class);

	printk("rbt530 driver removed.\n");

	
}

module_init(rbt530_driver_init);
module_exit(rbt530_driver_exit);
MODULE_LICENSE("GPL v2");




/*******************************************End Of Driver Code********************************************/





























































