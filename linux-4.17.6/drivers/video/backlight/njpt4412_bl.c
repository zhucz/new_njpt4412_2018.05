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
#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/export.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/pwm.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/time.h>

/*
    PWM 时钟频率 100M
    100M / 250 / 4 = 100000
    1/100000 = 10us
*/

#define  MAGIC_NUMBER    'k'  
#define  BEEP_ON     _IO(MAGIC_NUMBER    ,0)  
#define  BEEP_OFF    _IO(MAGIC_NUMBER    ,1)  
#define  BEEP_FREQ   _IO(MAGIC_NUMBER    ,2) 

static int      major;
static struct   cdev    pwm_cdev;   
static struct   class   *cls;

struct TIMER_BASE{
    unsigned int TCFG0;
    unsigned int TCFG1;
    unsigned int TCON;
    unsigned int TCNTB0;
    unsigned int TCMPB0;
    unsigned int TCNTO0;
    unsigned int TCNTB1;
    unsigned int TCMPB1;
    unsigned int TCNTO1;
    unsigned int TCNTB2;
    unsigned int TCMPB2;
    unsigned int TCNTO2;
    unsigned int TCNTB3;
    unsigned int TCMPB3;
    unsigned int TCNTO3;
    unsigned int TCNTB4;
    unsigned int TCBTO4;
    unsigned int TINT_CSTAT;      
};

volatile static struct TIMER_BASE * timer = NULL;

#define BEPP_IN_FREQ 100000  

static void beep_freq(unsigned long arg)  
{   
    printk("ioctl %d\n",(unsigned int)arg);
    timer->TCNTB1 = BEPP_IN_FREQ;
    timer->TCMPB1 = BEPP_IN_FREQ / arg;
    timer->TCON   = (timer->TCON  &~(0x0f << 8)) | (0x06 << 8);
    timer->TCON = (timer->TCON &~(0x0f << 8))| (0x0d << 8);
}  

static void beep_on(void)
{
    printk("beep on\n");
    timer->TCON = (timer->TCON &~(0x0f << 8))| (0x0d << 8);
    printk("%x\n",timer->TCON);
}


static void beep_off(void)
{
    timer->TCON = timer->TCON & ~(0x01 << 8);
}

static long pwm_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)  
{  
	printk("%d cmd = %d \n",__LINE__,cmd);
    switch(cmd)  
    {  
		case 0:  //BEEP_ON: 
            beep_on();  
            break;  
        case BEEP_OFF:  
            beep_off();  
            break;  
        case BEEP_FREQ:  
            beep_freq(arg);  
            break;  
        default :  
            return -EINVAL;  
			break;
    }; 
    return 0;
}  

static int pwm_open(struct inode *inode, struct file *file)
{
    printk("pwm_open\n");
    return 0;
}

static int pwm_release(struct inode *inode, struct file *file)
{
    printk("pwm_exit\n");
    return 0;
}

static struct file_operations pwm_fops = {
    .owner      = THIS_MODULE,
    .open       = pwm_open,
    .release        = pwm_release,  
    .unlocked_ioctl  = pwm_ioctl, 
};

static int pwm_probe(struct platform_device *pdev) {

    dev_t devid;
    struct device *dev = &pdev->dev;
    struct resource *res = NULL;
    struct pinctrl *pctrl;
    struct pinctrl_state *pstate;
    struct clk *base_clk;
    int ret;
    printk("enter %s\n",__func__);

    pctrl = devm_pinctrl_get(dev);
    if(pctrl == NULL)
    {
        printk("devm_pinctrl_get error\n");
        return -EINVAL;
    }

    pstate = pinctrl_lookup_state(pctrl, "pwm_pin");

    if(pstate == NULL)
    {
        printk("pinctrl_lookup_state error\n");
        return -EINVAL;
    }
    //设置引脚功能    
    pinctrl_select_state(pctrl, pstate);    
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if(res == NULL)
    {
        printk("platform_get_resource error\n");
        return -EINVAL;
    }
    printk("res: %x\n",(unsigned int)res->start);



    base_clk = devm_clk_get(&pdev->dev, "timers");
    if (IS_ERR(base_clk)) {
        dev_err(dev, "failed to get timer base clk\n");
        return PTR_ERR(base_clk);
    }

	printk("rate: = %ld \n",clk_get_rate(base_clk));

    ret = clk_prepare_enable(base_clk);
    if (ret < 0) {
        dev_err(dev, "failed to enable base clock\n");
        return ret;
    }

    timer = devm_ioremap_resource(&pdev->dev, res);
    if(timer == NULL)
    {
        printk("devm_ioremap_resource error\n");
        return -EINVAL;
    }

    printk("timer: %x\n",(unsigned int)timer);

    timer->TCFG0  = (timer->TCFG0 &~(0xff << 0)) | (0xfa << 0);
    timer->TCFG1  = (timer->TCFG1 &~(0x0f << 4)) | (0x02 << 4);
    timer->TCNTB1 = 127;//8700000;
    timer->TCMPB1 = 120;//7700000;
    timer->TCON   = (timer->TCON  &~(0x0f << 8)) | (0x06 << 8);
    printk("%x %x %x %x %x\n",timer->TCFG0,timer->TCFG1,timer->TCNTB1,timer->TCMPB1,timer->TCON);

    if(alloc_chrdev_region(&devid, 0, 1, "pwm") < 0)
    {
        printk("%s ERROR\n",__func__);
        goto error;
    }
    major = MAJOR(devid);
    cdev_init(&pwm_cdev, &pwm_fops);
    cdev_add(&pwm_cdev, devid, 1);  
    cls = class_create(THIS_MODULE, "mypwm");
    device_create(cls, NULL, MKDEV(major, 0), NULL, "mypwm1"); 

    beep_on();  

error:
    unregister_chrdev_region(MKDEV(major, 0), 1);
    return 0;
}

static int pwm_remove(struct platform_device *pdev){    

    printk("enter %s\n",__func__);
    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);
    cdev_del(&pwm_cdev);
    unregister_chrdev_region(MKDEV(major, 0), 1);
       printk("%s enter.\n", __func__);
       return 0;
}

static const struct of_device_id pwm_dt_ids[] = {
    { .compatible = "tiny4412,pwm_demo", },
    {},
};

MODULE_DEVICE_TABLE(of, pwm_dt_ids);

static struct platform_driver pwm_driver = {
    .driver        = {
        .name      = "pwm_demo",
        .of_match_table    = of_match_ptr(pwm_dt_ids),
    },
    .probe         = pwm_probe,
    .remove        = pwm_remove,
};

static int pwm_init(void){
    int ret;
    printk("enter %s\n",__func__);
    ret = platform_driver_register(&pwm_driver);
    if (ret)
        printk(KERN_ERR "pwm demo: probe faipwm: %d\n", ret);
    return ret; 
}

static void pwm_exit(void)
{
    printk("enter %s\n",__func__);
    platform_driver_unregister(&pwm_driver);
}

module_init(pwm_init);
module_exit(pwm_exit);
MODULE_AUTHOR("zhuchengzhi");
MODULE_DESCRIPTION("njpt4412 led driver");
MODULE_LICENSE("GPL");


/*============================================*/
#if 0

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

#define LED_CNT		1

static int major;
static struct cdev led_cdev;
static struct class *cls;
static int led1,led2;



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

	if(led1 <= 0){
		printk("%s error \n",__func__);
		return -EINVAL;
	}else{
		printk("led1 %d \n",led1);
		devm_gpio_request_one(dev, led1, GPIOF_OUT_INIT_HIGH, "led1");
	}

	if(alloc_chrdev_region(&devid, 0, LED_CNT, "lcd_backlight") < 0){
		printk("%s error \n",__func__);
		goto error;
	}

	major = MAJOR(devid);
	cdev_init(&led_cdev, &led_fops);
	cdev_add(&led_cdev, devid, LED_CNT);

	cls = class_create(THIS_MODULE, "led");
	device_create(cls, NULL, MKDEV(major, 0), NULL, "lcd_backlight0");

error:
	unregister_chrdev_region(MKDEV(major, 0), LED_CNT);
	return 0;

}


static int led_remove(struct platform_device *pdev)
{
	printk("%d : %s \n",__LINE__,__func__);
	device_destroy(cls, MKDEV(major, 0));
	device_destroy(cls, MKDEV(major, 1));

	class_destroy(cls);
	unregister_chrdev_region(MKDEV(major, 0), LED_CNT);
	printk("%d : %s \n",__LINE__,__func__);


	return 0;
}


static const struct of_device_id led_dt_ids[] = {
	{ .compatible = "njpt4412,backlight_demo",},
	{},
};
MODULE_DEVICE_TABLE(of,led_dt_ids);

static struct platform_driver led_driver = {
	.driver = {
		.name = "backlight_demo",
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
#endif
