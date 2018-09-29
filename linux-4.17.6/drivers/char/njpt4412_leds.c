#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>





#define LED_CNT		2

static int major;
static struct cdev led_cdev;
static struct class *cls;
static int led1,led2;
static int led3,led4;



static int led_open(struct inode *inode, struct file *file)
{
	printk("%d : %s \n",__LINE__,__func__);

	return 0;
}


static ssize_t led_write(struct file *file, const char __user *user_buf,size_t count, loff_t *ppos)
{

	char buf;
	int minor = iminor(file->f_inode);

//	printk("minor is %d \n",minor);

//	printk("%d : %s \n",__LINE__,__func__);

	if(count != 1){
		printk("count= %d \n",count);
		return 1;
	}

	if(copy_from_user(&buf, user_buf, count)){
//		printk("rcv %d \n",buf);
		return -EFAULT;
	}
	
//	printk("rcv %d \n",buf);

	if(buf == 0x01){
		switch(minor){
		case 0:
			gpio_set_value(led1, 0);
			break;
		case 1:
			gpio_set_value(led2, 0);
 			break;
		default:
			printk("%d: %s rcv minor error \n",__LINE__,__func__);
			break;
		}
	
	}else if (buf == 0x00){
		switch(minor){
		case 0:
			gpio_set_value(led1, 1);
			break;
		case 1:
			gpio_set_value(led2, 1);
			break;
		default:
			printk("%d: %s rcv minor error \n",__LINE__,__func__);
			break;
		}
	
	}
	return 1;
}


static struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.open  = led_open,
	.write = led_write,
};


static int led_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	dev_t devid;
	struct pinctrl *pctrl;
	struct pinctrl_state *pstate;


	pctrl = devm_pinctrl_get(dev);
	if(pctrl == NULL){
		printk("devm_pinctrl_get error \n");
	}
	pstate = pinctrl_lookup_state(pctrl,"led_demo");

	if(pstate == NULL){
		printk("pinctrl_lookup_state error \n");
	}

	pinctrl_select_state(pctrl,pstate);
	printk("enter %s \n",__func__);

	led1 = of_get_named_gpio(dev->of_node,"njpt4412,int_gpio1",0);
	led2 = of_get_named_gpio(dev->of_node,"njpt4412,ini_gpio2",0);
//	led3 = of_get_named_gpio(dev->of_node,"njpt4412,ini_gpio3",0);
//	led4 = of_get_named_gpio(dev->of_node,"njpt4412,ini_gpio4",0);

	if(led1 <= 0){
		printk("%s error \n",__func__);
		return -EINVAL;
	}else{
		printk("led1 %d \n",led1);
		printk("led2 %d \n",led2);
//		printk("led3 %d \n",led3);
//		printk("led4 %d \n",led4);
		devm_gpio_request_one(dev, led1, GPIOF_OUT_INIT_HIGH, "led1");
		devm_gpio_request_one(dev, led2, GPIOF_OUT_INIT_HIGH, "led2");
//		devm_gpio_request_one(dev, led3, GPIOF_OUT_INIT_HIGH, "led3");
//		devm_gpio_request_one(dev, led4, GPIOF_OUT_INIT_HIGH, "led4");
	}

	if(alloc_chrdev_region(&devid, 1, LED_CNT, "led") < 0){
		printk("%s error \n",__func__);
		goto error;
	}

	major = MAJOR(devid);
	cdev_init(&led_cdev, &led_fops);
	cdev_add(&led_cdev, devid, LED_CNT);

	cls = class_create(THIS_MODULE, "led");
	device_create(cls, NULL, MKDEV(major, 1), NULL, "led0");
	device_create(cls, NULL, MKDEV(major, 2), NULL, "led1");
//	device_create(cls, NULL, MKDEV(major, 3), NULL, "led2");
//	device_create(cls, NULL, MKDEV(major, 4), NULL, "led3");


//	gpio_direction_output(led1,1);
//	gpio_direction_output(led2,1);
//	gpio_direction_output(led3,1);
//	gpio_direction_output(led4,1);
	printk("%d : led   [1------------------4] \n",__LINE__);



error:
	unregister_chrdev_region(MKDEV(major, 1), LED_CNT);
	return 0;

}


static int led_remove(struct platform_device *pdev)
{
	printk("%d : %s \n",__LINE__,__func__);
	device_destroy(cls, MKDEV(major, 1));
	device_destroy(cls, MKDEV(major, 2));
//	device_destroy(cls, MKDEV(major, 3));
//	device_destroy(cls, MKDEV(major, 4));

	class_destroy(cls);
	unregister_chrdev_region(MKDEV(major, 1), LED_CNT);
	printk("%d : %s \n",__LINE__,__func__);


	return 0;
}


static const struct of_device_id led_dt_ids[] = {
	{ .compatible = "njpt4412,led_demo",},
	{},
};
MODULE_DEVICE_TABLE(of,led_dt_ids);

static struct platform_driver led_driver = {
	.driver = {
		.name = "led_demo",
		.of_match_table = of_match_ptr(led_dt_ids),
	},
	.probe = led_probe,
	.remove = led_remove,
};



static int led_init(void)
{
	int ret;
	printk("enter %s \n",__func__);
	ret = platform_driver_register(&led_driver);
	if(ret){
		printk(KERN_ERR "led demo :probe failed: %d \n",ret);
	}

	return ret;
}


static void led_exit(void)
{

	printk("exit %s \n",__func__);
	platform_driver_unregister(&led_driver);
}



module_init(led_init);
module_exit(led_exit);

MODULE_AUTHOR("zhuchengzhi");
MODULE_DESCRIPTION("njpt4412 led driver");
MODULE_LICENSE("GPL");
