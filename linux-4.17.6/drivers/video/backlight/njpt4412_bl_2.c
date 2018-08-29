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
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/uaccess.h>  /**/

/*
    PWM 时钟频率 100M
    100M / 250 / 4 = 100000
    1/100000 = 10us
*/

static int              		major;
static struct   cdev    		backlight_cdev;
static struct   class   		*cls;
static struct 	pinctrl   		*pctrl;
static struct 	pinctrl_state 	*pstate_out;
static int      lcd_gpd0_1;


static struct device 	*dev;
static struct clk 		*base_clk;
static struct resource 	*res = NULL, *irq = NULL;


struct TIMER_BASE
{
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

volatile static struct TIMER_BASE *timer = NULL;

static volatile unsigned int io_bit_count;
static volatile unsigned int io_data;




static irqreturn_t timer_for_1wire_interrupt(int irq, void *dev_id)
{
    printk("%d : %s \n",__LINE__,__func__);

	timer->TINT_CSTAT |= (0x1 << 6);  //clears timer 1 interrupt status




	return IRQ_HANDLED;
}



static ssize_t backlight_write(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
    unsigned char reg, ret;
    ret = copy_from_user(&reg, buf, 1);

    if (ret < 0){
        printk("%d copy_from_user error\n", __LINE__);
    }

    if (reg > 127){
		reg = 127;
	}

  
    return 1;
}


static int backlight_open(struct inode *inode, struct file *file)
{
    printk("%d : %s \n",__LINE__,__func__);
    return 0;
}

static int backlight_release(struct inode *inode, struct file *file)
{
    printk("%d : %s \n",__LINE__,__func__);
    return 0;
}

static struct file_operations backlight_fops =
{
    .owner              = THIS_MODULE,
    .open               = backlight_open,
    .release            = backlight_release,
    .write              = backlight_write,
};


static void start_timer1(unsigned char reg)
{
    unsigned int tcon;

//    timer->TCNTB1 = 650;
    //init tranfer and start timer
//    tcon = timer->TCON;
//    tcon &= ~(0xF << 16);
//    tcon |= (1 << 17);
//    timer->TCON = tcon;
//    tcon |= (1 << 16);
//    tcon |= (1 << 19);
//    tcon &= ~(1 << 17);
//    timer->TCON = tcon;
//    timer->TINT_CSTAT |= 0x08;

	timer->TINT_CSTAT |= (0x1 << 1);  //timer 1 interrupt enable 
	timer->TCON |= (0x1 << 8);        //start timer 1

	printk("%d : %s \n",__LINE__,__func__);
}

static void stop_timer1(void)
{
	timer->TCON &= ~(0x1 << 8);        //stop timer 1
	printk("%d : %s \n",__LINE__,__func__);
}


static int backlight_probe(struct platform_device *pdev)
{
    int ret;
    dev_t devid;
    dev = &pdev->dev;
    pctrl = devm_pinctrl_get(dev);

    printk("%d: %s pctrl = %#x \n", __LINE__,__func__,pctrl);

    if (pctrl == NULL){
        printk("devm_pinctrl_get error\n");
        return -EINVAL;
    }


    pstate_out = pinctrl_lookup_state(pctrl, "backlight_out");
	printk("%d : pstate_out = %#x \n",__LINE__,pstate_out);

    if (pstate_out == NULL) {
        printk("pinctrl_lookup_state error\n");
        return -EINVAL;
    }


    lcd_gpd0_1 = of_get_named_gpio(dev->of_node, "njpt4412,backlight", 0);
	printk("%d : lcd_gpd0_1 = %#x \n",__LINE__,lcd_gpd0_1);

    if (!lcd_gpd0_1){
        printk("of_get_named_gpio() error\n");
        return -EINVAL;
    }

    devm_gpio_request_one(dev, lcd_gpd0_1, GPIOF_OUT_INIT_HIGH, "lcd_gpd0_1");

	pinctrl_select_state(pctrl,pstate_out);

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	printk("%d : res = %#x \n",__LINE__,res);

    if (res == NULL){
        printk("platform_get_resource error\n");
        return -EINVAL;
    }

    base_clk = devm_clk_get(&pdev->dev, "timers");
	printk("%d : base_clk = %#x \n",__LINE__,base_clk);

    if (IS_ERR(base_clk)){
        dev_err(dev, "failed to get timer base clk\n");
        return PTR_ERR(base_clk);
    }


    ret = clk_prepare_enable(base_clk);

	printk("%d : ret = %#x \n",__LINE__,ret);

    if (ret < 0){
        dev_err(dev, "failed to enable base clock\n");
        return ret;
    }

    timer = devm_ioremap_resource(&pdev->dev, res);
  	printk("%d : timer: %x\n",__LINE__, (unsigned int)timer);

    if (timer == NULL){
        printk("devm_ioremap_resource error\n");
        return -EINVAL;
    }
	
#if 1
 	timer->TCFG0  = ((timer->TCFG0 & ~(0xff << 0)) | (0xfa << 0)) ;
 	timer->TCFG1  = ((timer->TCFG1 & ~(0xff << 0)) | (0x02 << 0)) ;
	timer->TCNTB1 = 100000;
	timer->TCMPB1 =  90000;
 	timer->TCON   = ((timer->TCON & ~(0x0f << 0)) | (0x06 << 8)) ;


    irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	printk("%d : irq = %d \n",__LINE__,irq);

    if (irq == NULL){
        printk("platform_get_resource irq error\n");
        return -EINVAL;
    }


    ret = devm_request_irq(dev, irq->start, timer_for_1wire_interrupt , IRQF_TIMER, "backlight", NULL);
	printk("%d : ret = %d \n",__LINE__,ret);

    if (ret){
        dev_err(dev, "unable to request irq\n");
        return -EINVAL;
    }


	start_timer1(0x60);

#endif


 	if (alloc_chrdev_region(&devid, 0, 1, "my_backlight") < 0){
        printk("%d: alloc_chrdev_region \n",__LINE__);
        return -EINVAL;
    }

    major = MAJOR(devid);
    cdev_init(&backlight_cdev, &backlight_fops);
    cdev_add(&backlight_cdev, devid, 1);
    cls = class_create(THIS_MODULE, "backlight_demo");
    device_create(cls, NULL, MKDEV(major, 0), NULL, "backlight");
	printk("%d : backlight_cdev %d \n",__LINE__,major);

    return 0;
}

static int backlight_remove(struct platform_device *pdev)
{
    printk("%d: %s\n",__LINE__, __func__);

    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);
    cdev_del(&backlight_cdev);
    unregister_chrdev_region(MKDEV(major, 0), 1);
    devm_pinctrl_put(pctrl);
    devm_free_irq(dev, irq->start, NULL);
    clk_disable_unprepare(base_clk);
    devm_gpio_free(dev, lcd_gpd0_1);
    return 0;
}

static const struct of_device_id backlight_dt_ids[] =
{
    { .compatible = "njpt4412,backlight_demo", },
    {},
};

MODULE_DEVICE_TABLE(of, backlight_dt_ids);

static struct platform_driver backlight_driver =
{
    .driver        = {
        .name      = "backlight_demo",
        .of_match_table    = of_match_ptr(backlight_dt_ids),
    },
    .probe         = backlight_probe,
    .remove        = backlight_remove,
};

static int backlight_init(void)
{
    int ret;
    printk("%d : %s ****** \n",__LINE__, __func__);
    ret = platform_driver_register(&backlight_driver);

    if (ret){
        printk(KERN_ERR "backlight demo: probe faid backlight: %d\n", ret);
    }

    return ret;
}

static void backlight_exit(void)
{
    printk("%d : %s\n",__LINE__, __func__);
    platform_driver_unregister(&backlight_driver);
}

module_init(backlight_init);
module_exit(backlight_exit);


MODULE_AUTHOR("zhuchengzhi");
MODULE_DESCRIPTION("njpt4412 lcd backlight driver");
MODULE_LICENSE("GPL");


