#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include<linux/init.h>
#include<linux/moduleparam.h>
//#include "rbtree_drv.h"

#define DEVICE_NAME     "KPROBES"


struct probe{
	int flag;
	unsigned long offset;
	char function_name[25];

};

/* Kprobe implementation */

struct rbprobe_dev{
	struct cdev cdev;
	char name[20];
	struct kprobe kp;
	struct probe p;
	
}*rbprobe_devp ;



struct data {
			ktime_t Time;
			pid_t Pid;
			unsigned long Address;

};

struct probe p;
struct data d;




static dev_t rbprobe_dev_number;    /*Allocate device number*/ 
struct class *rbprobe_dev_class;    /*Tie with the device model*/
static struct device *rbprobe_dev_device;
static char *user_name = "Dear John"; /* the default user name, for insmod command */
module_param(user_name, charp,0000);    //to get parameter from load.sh script to greet the user




/* Open the rbprobe driver */

int rbprobe_driver_open(struct inode *inode, struct file *file){


	struct rbprobe_dev *rbprobe_devp;



	/*Get the per-device structure that contains this cdev */
	rbprobe_devp = container_of(inode->i_cdev, struct rbprobe_dev, cdev);


	/* Easy access to cmos_devp from rest of the entry points */
	file->private_data = rbprobe_devp;
	printk("\n%s is opening \n", rbprobe_devp->name);

	return 0;

}


int rbprobe_driver_release(struct inode *inode, struct file *file){

struct rbprobe_dev *rbprobe_devp = file->private_data;

printk("\n%s is closing \n", rbprobe_devp->name);

return 0;

}

static int Fault_Handler(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
	printk("In the Fault handler function \n");
	printk(KERN_INFO "fault_handler: p->addr = 0x%p, trap #%dn",
		p->addr, trapnr);
	/* Return 0 because we don't handle the fault. */
	return 0;
}






int Pre_Handler(struct kprobe *p, struct pt_regs *regs){
 	
	ktime_t Time;
	pid_t Pid;
	//struct data d;
 	printk(KERN_ALERT"PROBE HIT \n");

 	



 	printk(KERN_INFO "pre_handler: p->addr = 0x%p, ip = %lx,"
			" flags = 0x%lx\n",
		p->addr, regs->ip, regs->flags);


 	Time= ktime_get();
 	Pid = task_pid_nr(current);
 	//value = *((int *)rbprobe_devp.kp.addr);

 	d.Time = Time;
 	d.Pid = Pid;
 	d.Address = (unsigned long)p->addr;

 	printk("kprobe values: %lld  %d %ld\n", d.Time, d.Pid, d.Address);




 	return 0;

 }


 static void Post_Handler(struct kprobe *p, struct pt_regs *regs,
				unsigned long flags)
{
	printk("In the Post handler function \n");
	printk(KERN_INFO "<%s> post_handler: p->addr = 0x%p, flags = 0x%lx\n",
		p->symbol_name, p->addr, regs->flags);
}




/* Write Function */
 static ssize_t rbprobe_driver_write(struct file *file, const char *buf,
           size_t count, loff_t *ppos){

 	struct rbprobe_dev *rbprobe_devp = file->private_data;
 	struct probe p;
 	int ret=0;

 	printk("Write function for KPROBE being called"); 


 	
	//p = kmalloc(sizeof(struct probe), GFP_KERNEL);
	//memset(p,0,sizeof(struct probe));


	 if(copy_from_user(&p,(struct probe *)buf,sizeof(struct probe))){
		 		return -EFAULT;
		 }

		 //rbprobe_devp->p.flag = p.flag;
		 //rbprobe_devp->p.offset = p.offset;
		 //strcpy(rbprobe_devp->p.function_name , p.function_name);

		 printk("Flag value: %d\n", p.flag);
		// printk("Offset value: %d\n", p.offset);
		  printk("Function name: %s\n", p.function_name);


		 	
	// if(p.flag == 0) {
		unregister_kprobe(&(rbprobe_devp->kp));
		// return -1;
		memset(&(rbprobe_devp->kp),0, sizeof(struct kprobe));	
	// }
	if(p.flag==1){

		rbprobe_devp->kp.pre_handler = Pre_Handler;
	 	rbprobe_devp->kp.post_handler = Post_Handler;
	 	rbprobe_devp->kp.fault_handler = Fault_Handler;
	 	rbprobe_devp->kp.addr = (kprobe_opcode_t*)kallsyms_lookup_name(p.function_name);
	 	//rbprobe_devp->kp.offset = rbprobe_devp->p.offset;
	 	//rbprobe_devp->kp.symbol_name = ptr;

		//printk("Function address is %x", &rbt530_driver_write); 

	 	ret = register_kprobe(&(rbprobe_devp->kp));
	}

	
 	printk("return value for registering %d",ret);
	if (ret < 0) {
		printk(KERN_INFO "register_kprobe failed, returned %d\n", ret);  
		return ret;
		}
	printk(KERN_INFO "Planted kprobe\n");   
	
	return 0;


 }



/* Write Function */
 ssize_t rbprobe_driver_read(struct file *file,  char *buf,
           size_t count, loff_t *ppos){


 	printk(KERN_ALERT"In the KProbe read\n");
 	copy_to_user((char*)buf,&d, sizeof(struct data));
 	return 0;


}





/* File Operations Structure. Defined in linux/fs.h */
static struct file_operations rbprobe_fops = {

	.owner           =THIS_MODULE,
	.open            =rbprobe_driver_open,
	.release         =rbprobe_driver_release,
	.write           =rbprobe_driver_write,
	.read            =rbprobe_driver_read,
	

};


 



static int rbprobe_driver_init(void){

	int ret;
	

	/*Request dynamic allocation of a device major number */
	if(alloc_chrdev_region(&rbprobe_dev_number,0,1, DEVICE_NAME)<0){
			printk( "Can't register device\n");
			return -1;
	}
	else
		//printk( "DEVICE DRIVER FILE CREATION SUCCESS\n\n\n\n");




	/*Populate sysfs entries*/
	rbprobe_dev_class = class_create(THIS_MODULE, DEVICE_NAME);

	

	/*Allocate memory for the per-device structure */
	rbprobe_devp = kmalloc(sizeof(struct rbprobe_dev), GFP_KERNEL);


	if(!rbprobe_devp){

			printk("Bad kmalloc\n");
			return -ENOMEM;
	}

	//rbprobe_devp->the_root = RB_ROOT;


	/* Request I/O region */
	sprintf(rbprobe_devp->name, DEVICE_NAME);


	/* Connect the file operations with cdev */
	cdev_init(&rbprobe_devp->cdev, &rbprobe_fops);
	rbprobe_devp->cdev.owner = THIS_MODULE;

	/*Connect the major/minor number to the cdev*/

	ret = cdev_add(&rbprobe_devp->cdev, (rbprobe_dev_number),1);

	if(ret){
		printk("Bad cdev\n");
		return ret;
	}


	/* Send uevents to udev, so it'll create /dev nodes */

	rbprobe_dev_device = device_create(rbprobe_dev_class, NULL, MKDEV(MAJOR(rbprobe_dev_number),0), NULL, DEVICE_NAME);




	////time_since_boot = (jiffies-INITIAL_JIFFIES)/HZ;


	printk("rbprobe driver initialised.\n");

	return 0;

}









/* Driver Exit */

static void rbprobe_driver_exit(void){

	
   unregister_kprobe(&(rbprobe_devp->kp));

	/* Release the major number */
	unregister_chrdev_region((rbprobe_dev_number),1);


	/*Destroy Device */
	device_destroy(rbprobe_dev_class, MKDEV(MAJOR(rbprobe_dev_number),0));
	cdev_del(&rbprobe_devp->cdev);
	kfree(rbprobe_devp);

	/* Destroy driver_class */
	class_destroy(rbprobe_dev_class);

	printk("rbprobe driver removed.\n");

	//printk("Goodbye\n\n\n\n");
}

module_init(rbprobe_driver_init);
module_exit(rbprobe_driver_exit);
MODULE_LICENSE("GPL v2");













