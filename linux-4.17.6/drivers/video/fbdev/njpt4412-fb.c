/*
 *  1. 一定要注意在LCD驱动中，必须先开启背光灯。否则看不到小企鹅.
 *
 *  2. 2018-09-05 内核启动后能后显示小企鹅控制台了.
 *
 *
 * */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>

#include <asm/mach/map.h>
#include <linux/fb.h>
#include <asm/types.h>


#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>



#define         VIDCON0                 0x00
#define         VIDCON1                 0x04
#define         VIDCON2                 0x08
#define         VIDCON3                 0x0C
#define         VIDTCON0                0x10
#define         VIDTCON1                0x14
#define         VIDTCON2                0x18
#define         VIDTCON3                0x1C
#define         WINCON0                 0x20
#define         SHADOWCON               0x34
#define         WINCHMAP2               0x3c
#define         VIDOSD0A                0x40
#define         VIDOSD0B                0x44
#define         VIDOSD0C                0x48
#define         VIDW00ADD0B0    		0xA0
#define         VIDW00ADD1B0    		0xD0

#define         CLK_SRC_LCD0            0x234
#define         CLK_SRC_MASK_LCD        0x334
#define         CLK_DIV_LCD             0x534

#define 		CLKDIV2_RATIO			0x580
#define			CLK_DIV_STAT_LCD		0x634
#define			CLKDIV2_STAT			0x680

#define         CLK_GATE_IP_LCD         0x934

#define			CLK_GATE_BLOCK			0x970
#define			CLKOUT_CMU_TOP			0xA00

#define         LCDBLK_CFG              0x00
#define         LCDBLK_CFG2             0x04

#define         LCD_LENTH               480
#define         LCD_WIDTH               272
#define         BITS_PER_PIXEL          32




static int s3c_lcdfb_setcolreg(unsigned int regno, unsigned int red,
                               unsigned int green, unsigned int blue,
                               unsigned int transp, struct fb_info *info);


static struct fb_ops s3c_lcdfb_ops =
{
    .owner              = THIS_MODULE,
    .fb_setcolreg       = s3c_lcdfb_setcolreg,
    .fb_fillrect        = cfb_fillrect,
    .fb_copyarea        = cfb_copyarea,
    .fb_imageblit       = cfb_imageblit,
};




static struct fb_info *s3c_lcd;
static volatile void __iomem *lcd_regs_base;
static volatile void __iomem *clk_regs_base;
static volatile void __iomem *lcdblk_regs_base;
//static volatile void __iomem *lcd0_configuration;
static u32 pseudo_palette[16];
static struct resource *res1, *res2, *res3, *res4;
static struct resource *res5, *res6, *res7, *res8;

static struct resource *res9, *res10;



static volatile void __iomem *regs_gpf0con;
static volatile void __iomem *regs_gpf1con;
static volatile void __iomem *regs_gpf2con;
static volatile void __iomem *regs_gpf3con;

static volatile void __iomem *regs_gpc0con;
static volatile void __iomem *regs_gpl1con;

/* from pxafb.c */
static inline unsigned int chan_to_field(unsigned int chan, struct fb_bitfield *bf)
{
    chan &= 0xffff;
    chan >>= 16 - bf->length;
    return (chan << bf->offset);
}


static int s3c_lcdfb_setcolreg(unsigned int regno, unsigned int red,
                               unsigned int green, unsigned int blue,
                               unsigned int transp, struct fb_info *info)
{
    unsigned int color = 0;
    uint32_t *p;

	if(regno > 16){
		printk("%d regno = %#x \n",__LINE__,regno);
		return 1;
	}

    color  = chan_to_field(red,     &info->var.red);
    color |= chan_to_field(green, 	&info->var.green);
    color |= chan_to_field(blue, 	&info->var.blue);

    p = info->pseudo_palette;  
    p[regno] = color;

//	printk("%d : %s \n",__LINE__,__func__);
    return 0;
}

static int lcd_probe(struct platform_device *pdev)
{
    int ret;
    unsigned int temp;

    struct clk *base_clk_fimd0;
    struct clk *base_clk_aclk160;
    struct clk *lcd_43;
    struct clk *sclk_clk_fimd0;

    struct device *dev = &pdev->dev;

	volatile unsigned int *lcd0_configuration;
#if 0
	struct device *dev = &pdev->dev;
	struct pinctrl *pctrl;
	struct pinctrl_state *pstate;


	pctrl = devm_pinctrl_get(dev);
	if(pctrl == NULL){
		printk("devm_pinctrl_get error \n");
	}
	pstate = pinctrl_lookup_state(pctrl,"lcd_demo");

	if(pstate == NULL){
		printk("pinctrl_lookup_state error \n");
	}

	pinctrl_select_state(pctrl,pstate);

	printk("%d lcd gpio configure \n",__LINE__);

#endif


    /* 1. 分配一个fb_info */
    s3c_lcd = framebuffer_alloc(0, NULL);
	if(s3c_lcd == NULL){
		printk("s3c_lcd alloc failed \n");
	}
	printk("s3c_lcd alloc success \n");

    /* 2. 设置 */
    /* 2.1 设置 fix 固定的参数 */
    strcpy(s3c_lcd->fix.id, "mylcd");
    s3c_lcd->fix.smem_len = LCD_WIDTH * LCD_LENTH * BITS_PER_PIXEL / 8;     //显存的长度
    s3c_lcd->fix.type     = FB_TYPE_PACKED_PIXELS;                                                      //类型
    s3c_lcd->fix.visual   = FB_VISUAL_TRUECOLOR;                                                        //TFT 真彩色
    s3c_lcd->fix.line_length = LCD_LENTH * BITS_PER_PIXEL /8 ;                          //一行的长度
  
	/* 2.2 设置 var 可变的参数 */
    s3c_lcd->var.xres           = LCD_LENTH;                    //x方向分辨率
    s3c_lcd->var.yres           = LCD_WIDTH;                    //y方向分辨率
    s3c_lcd->var.xres_virtual   = LCD_LENTH;                    //x方向虚拟分辨率
    s3c_lcd->var.yres_virtual   = LCD_WIDTH;                    //y方向虚拟分辨率
    s3c_lcd->var.bits_per_pixel = BITS_PER_PIXEL;               //每个像素占的bit


//	s3c_lcd->var.pixclock = 60;	    /* pixel clock in ps (pico seconds) */
//	s3c_lcd->var.left_margin;		/* time from sync to picture	*/
//	s3c_lcd->var.right_margin;		/* time from picture to sync	*/
//	s3c_lcd->var.upper_margin;		/* time from sync to picture	*/
//	s3c_lcd->var.lower_margin;
//	s3c_lcd->var.hsync_len;			/* length of horizontal sync	*/
//	s3c_lcd->var.vsync_len;			/* length of vertical sync	*/
//	s3c_lcd->var.vsync;				/* see FB_SYNC_*		*/
//	s3c_lcd->var.vmode;				/* see FB_VMODE_*		*/
//	s3c_lcd->var.rotate;			/* angle we rotate counter clockwise */
//	s3c_lcd->var.colorspace;		/* colorspace for FOURCC-based modes */


#if 1
	/* RGB:888 */
    s3c_lcd->var.red.length     = 8;
    s3c_lcd->var.red.offset     = 16;   //红
    s3c_lcd->var.green.length   = 8;
    s3c_lcd->var.green.offset   = 8;    //绿
    s3c_lcd->var.blue.length    = 8;
    s3c_lcd->var.blue.offset    = 0;    //蓝
#else
    /* RGB:565 */
    s3c_lcd->var.red.offset     = 11;   //红
    s3c_lcd->var.red.length     = 5;
    s3c_lcd->var.green.offset   = 5;    //绿
    s3c_lcd->var.green.length   = 6;
    s3c_lcd->var.blue.offset    = 0;    //蓝
    s3c_lcd->var.blue.length    = 5;
#endif

    s3c_lcd->var.activate       = FB_ACTIVATE_NOW;
    
	/* 2.3 设置操作函数 */
    s3c_lcd->fbops              = &s3c_lcdfb_ops;

    /* 2.4 其他的设置 */
    s3c_lcd->pseudo_palette     = pseudo_palette;               //调色板
    s3c_lcd->screen_size        = LCD_WIDTH * LCD_LENTH * BITS_PER_PIXEL /8;   //显存大小

    /* 3. 硬件相关的操作 */
    /* 3.1 配置GPIO用于LCD */
    //设备树中使用"default"


	/* 3.2 根据LCD手册设置LCD控制器, 比如VCLK的频率等 */
    //寄存器映射
    res1 = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (res1 == NULL){
        printk("platform_get_resource error\n");
        return -EINVAL;
    }
    printk("res1: %x\n",(unsigned int)res1->start);


    lcd_regs_base = devm_ioremap_resource(&pdev->dev, res1);
    if (lcd_regs_base == NULL){
        printk("devm_ioremap_resource error\n");
        return -EINVAL;
    }

    res2 = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    if (res2 == NULL){
        printk("platform_get_resource error\n");
        return -EINVAL;
    }
    printk("res2: %x\n",(unsigned int)res2->start);


    lcdblk_regs_base = devm_ioremap_resource(&pdev->dev, res2);
    if (lcdblk_regs_base == NULL){
        printk("devm_ioremap_resource error\n");
        return -EINVAL;
    }

    res3 = platform_get_resource(pdev, IORESOURCE_MEM, 2);
    if (res3 == NULL){
        printk("platform_get_resource error\n");
        return -EINVAL;
    }
    printk("res3: %x\n",(unsigned int)res3->start);


  lcd0_configuration = ioremap(res3->start, 0x04);    
//lcd0_configuration = devm_ioremap_resource(&pdev->dev, res3);
    if (lcd0_configuration == NULL){
        printk("devm_ioremap_resource error\n");
        return -EINVAL;
    }

////  *(unsigned long *)lcd0_configuration = 7;

//	temp = readl(lcd0_configuration);
//	temp |= 0x07; // power On
//	writel(temp,lcd0_configuration);
//	temp = 0x00;
//	temp = readl(lcd0_configuration);

	*lcd0_configuration = 0x07;
	printk("lcd0_configuration = %#x lcd0 power on\n",temp);


    res4 = platform_get_resource(pdev, IORESOURCE_MEM, 3);
    if (res3 == NULL){
        printk("platform_get_resource error\n");
        return -EINVAL;
    }
    printk("res4: %x\n",(unsigned int)res4->start);

//  clk_regs_base = ioremap(res4->start, 0x1000);
   	clk_regs_base = devm_ioremap_resource(&pdev->dev, res4);
    if (clk_regs_base == NULL){
        printk("devm_ioremap_resource error\n");
        return -EINVAL;
    }

#if 1 //Check LCD GPIO Configure
    res5 = platform_get_resource(pdev, IORESOURCE_MEM, 4);
    if (res5 == NULL){
        printk("platform_get_resource error\n");
        return -EINVAL;
    }
    printk("res5: %x\n",(unsigned int)res5->start);


//  regs_gpf0con = ioremap(res5->start, 0x20);
   	regs_gpf0con = devm_ioremap_resource(&pdev->dev, res5);
    if (regs_gpf0con == NULL){
        printk("devm_ioremap_resource error\n");
        return -EINVAL;
    }

	temp = readl(regs_gpf0con);
	printk("%d [gpf0con temp = %#x] \n",__LINE__,temp);


    res6 = platform_get_resource(pdev, IORESOURCE_MEM, 5);
    if (res6 == NULL){
        printk("platform_get_resource error\n");
        return -EINVAL;
    }
    printk("res6: %x\n",(unsigned int)res6->start);


//  regs_gpf1con = ioremap(res6->start, 0x20);
   	regs_gpf1con = devm_ioremap_resource(&pdev->dev, res6);
    if (regs_gpf1con == NULL){
        printk("devm_ioremap_resource error\n");
        return -EINVAL;
    }

	temp = readl(regs_gpf1con);
	printk("%d [gpf1con temp = %#x] \n",__LINE__,temp);



    res7 = platform_get_resource(pdev, IORESOURCE_MEM, 6);
    if (res7 == NULL){
        printk("platform_get_resource error\n");
        return -EINVAL;
    }
    printk("res7: %x\n",(unsigned int)res7->start);


//  regs_gpf2con = ioremap(res7->start, 0x20);
   	regs_gpf2con = devm_ioremap_resource(&pdev->dev, res7);
    if (regs_gpf2con == NULL){
        printk("devm_ioremap_resource error\n");
        return -EINVAL;
    }

	temp = readl(regs_gpf2con);
	printk("%d [gpf2con temp = %#x] \n",__LINE__,temp);



    res8 = platform_get_resource(pdev, IORESOURCE_MEM, 7);
    if (res8 == NULL){
        printk("platform_get_resource error\n");
        return -EINVAL;
    }
    printk("res8: %x\n",(unsigned int)res8->start);


//  regs_gpf3con = ioremap(res8->start, 0x20);
   	regs_gpf3con = devm_ioremap_resource(&pdev->dev, res8);
    if (regs_gpf3con == NULL){
        printk("devm_ioremap_resource error\n");
        return -EINVAL;
    }

	temp = readl(regs_gpf3con);
	printk("%d [gpf3con temp = %#x] \n",__LINE__,temp);



    res9 = platform_get_resource(pdev, IORESOURCE_MEM, 8);
    if (res9 == NULL){
        printk("platform_get_resource error\n");
        return -EINVAL;
    }
    printk("res9: %x\n",(unsigned int)res9->start);


//  regs_gpc0con = ioremap(res9->start, 0x20);
   	regs_gpc0con = devm_ioremap_resource(&pdev->dev, res9);
    if (regs_gpc0con == NULL){
        printk("devm_ioremap_resource error\n");
        return -EINVAL;
    }

	temp = readl(regs_gpc0con);
	temp |= (0x1 << 8);
	writel(temp,regs_gpc0con);
	temp = 0x00;
	temp = readl(regs_gpc0con);
	printk("%d [gpc0con temp = %#x] \n",__LINE__,temp);


	temp = readl(regs_gpc0con + 0x08);//gpc0dat
	temp |= (3 << 4);//GPC0PUD[2]
	writel(temp,regs_gpc0con + 0x08);
	temp = 0x00;
	temp = readl(regs_gpc0con + 0x08);
	printk("%d [gpc0PUD[2] temp = %#x] \n",__LINE__,temp);

	temp = readl(regs_gpc0con + 0x0C);//gpc0dat
	temp |= (3 << 4);//GPC0DRV[2]
	writel(temp,regs_gpc0con + 0x0C);
	temp = 0x00;
	temp = readl(regs_gpc0con + 0x0C);
	printk("%d [gpc0DRV[2] temp = %#x] \n",__LINE__,temp);


	temp = readl(regs_gpc0con + 0x04);//gpc0dat
	temp &= ~(1 << 2);//GPC0DAT[2]
	writel(temp,regs_gpc0con + 0x04);
	temp = 0x00;
	temp = readl(regs_gpc0con + 0x04);
	printk("%d [gpc0dat[2] temp = %#x] \n",__LINE__,temp);

	msleep(260);
	temp = readl(regs_gpc0con + 0x04);//gpc0dat
	temp |= (1 << 2);//GPC0DAT[2]
	writel(temp,regs_gpc0con + 0x04);
	temp = 0x00;
	temp = readl(regs_gpc0con + 0x04);
	printk("%d [gpc0dat[2] temp = %#x] \n",__LINE__,temp);



    res10 = platform_get_resource(pdev, IORESOURCE_MEM, 9);
    if (res10 == NULL){
        printk("platform_get_resource error\n");
        return -EINVAL;
    }
    printk("res10: %x\n",(unsigned int)res10->start);


//  regs_gpl1con = ioremap(res10->start, 0x20);
   	regs_gpl1con = devm_ioremap_resource(&pdev->dev, res10);
    if (regs_gpl1con == NULL){
        printk("devm_ioremap_resource error\n");
        return -EINVAL;
    }

	temp = readl(regs_gpl1con);
	temp |= (0x1 << 0);
	writel(temp,regs_gpl1con);
	temp = 0x00;
	temp = readl(regs_gpl1con);
	printk("%d [gpfl1on temp = %#x] \n",__LINE__,temp);



	temp = readl(regs_gpl1con + 0x04);//gpL1DAT
	temp |= (1 << 0);//GPL1DAT[0]
	writel(temp,regs_gpl1con + 0x04);
	temp = 0x00;
	temp = readl(regs_gpl1con + 0x04);
	printk("%d [gpL1DAT[0] temp = %#x] \n",__LINE__,temp);


#endif


    //使能时钟
#if 1 //get lcd clock
    base_clk_fimd0 = devm_clk_get(&pdev->dev, "fimd0");
    if (IS_ERR(base_clk_fimd0)) {
        dev_err(dev, "failed to get timer base clk\n");
        return PTR_ERR(base_clk_fimd0);
    }

    ret = clk_prepare_enable(base_clk_fimd0);
    if (ret < 0) {
        dev_err(dev, "failed to enable base clock\n");
        return ret;
    }
	printk("fimd0_rate: = %ld \n",clk_get_rate(base_clk_fimd0));


    base_clk_aclk160 = devm_clk_get(&pdev->dev, "aclk160");
    if (IS_ERR(base_clk_aclk160)) {
        dev_err(dev, "failed to get timer base clk\n");
        return PTR_ERR(base_clk_aclk160);
    }
	printk("aclk160_rate: = %ld \n",clk_get_rate(base_clk_aclk160));

//    ret = clk_prepare_enable(base_clk_aclk160);
//    if (ret < 0) {
//        dev_err(dev, "failed to enable base clock\n");
//        return ret;
//    }


    lcd_43 = devm_clk_get(&pdev->dev, "lcd_user_t");
    if (IS_ERR(lcd_43)) {
        dev_err(dev, "failed to get timer base clk\n");
        return PTR_ERR(lcd_43);
    }
	printk("lcd_43_rate: = %ld \n",clk_get_rate(lcd_43));

    ret = clk_prepare_enable(lcd_43);
    if (ret < 0) {
        dev_err(dev, "failed to enable base clock\n");
        return ret;
    }


    sclk_clk_fimd0 = devm_clk_get(&pdev->dev, "sclk_clk_fimd0");
    if (IS_ERR(sclk_clk_fimd0)) {
        dev_err(dev, "failed to get timer base clk\n");
        return PTR_ERR(sclk_clk_fimd0);
    }
	printk("sclk_clk_fimd0_rate: = %ld \n",clk_get_rate(sclk_clk_fimd0));



//	ret = clk_set_rate(sclk_clk_fimd0,9000000);
//    if (ret < 0) {
//        dev_err(dev, "failed to set sclk_clk_fimd0 base clock\n");
//        return ret;
//    }


//    ret = clk_prepare_enable(sclk_clk_fimd0);
//    if (ret < 0) {
//        dev_err(dev, "failed to enable base clock\n");
//        return ret;
//    }

#endif

    //时钟源选择 0110  SCLKMPLL_USER_T 800M
    temp = readl(clk_regs_base + CLK_SRC_LCD0);
    temp &= ~0x0f;
    temp |= 0x06;
    writel(temp, clk_regs_base + CLK_SRC_LCD0);
   	temp = 0x00;
    temp = readl(clk_regs_base + CLK_SRC_LCD0);
	printk("%d [CLK_SRC_LCD0 = %#x] \n",__LINE__,temp);

	//FIMD0_MASK
    temp = readl(clk_regs_base + CLK_SRC_MASK_LCD);
    temp |= 0x01;
    writel(temp, clk_regs_base + CLK_SRC_MASK_LCD);
    temp = 0x00;
    temp = readl(clk_regs_base + CLK_SRC_MASK_LCD);
	printk("%d [CLK_SRC_MASK_LCD = %#x] \n",__LINE__,temp);


	//SCLK_FIMD0 = MOUTFIMD0/(FIMD0_RATIO + 1),分频 1/1
    temp = readl(clk_regs_base + CLK_DIV_LCD);
    temp &= ~0x0f;
//  temp |=  0x13;
    writel(temp, clk_regs_base + CLK_DIV_LCD);
    temp = 0x00;
    temp = readl(clk_regs_base + CLK_DIV_LCD);
	printk("%d [CLK_DIV_LCD = %#x] \n",__LINE__,temp);


	//CLK_FIMD0 Pass
    temp = readl(clk_regs_base + CLK_GATE_IP_LCD);
    temp |= 0x01;
	temp |= (0x01 << 4);
    writel(temp, clk_regs_base + CLK_GATE_IP_LCD);
    temp = 0x00;
    temp = readl(clk_regs_base + CLK_GATE_IP_LCD);
	printk("%d [CLK_GATE_IP_LCD = %#x] \n",__LINE__,temp);


	//FIMDBYPASS_LBLK0 FIMD Bypass
    temp = readl(lcdblk_regs_base + LCDBLK_CFG);
	temp &= ~(3 << 0);
    temp |=  (1 << 1); //FIMD Bypass
	temp &= ~(3 << 10);//RGB Interface
    writel(temp, lcdblk_regs_base + LCDBLK_CFG);
	temp = 0x00;
    temp = readl(lcdblk_regs_base + LCDBLK_CFG);
	printk("%d [LCDBLK_CFG = %#x] \n",__LINE__,temp);


    temp = readl(lcdblk_regs_base + LCDBLK_CFG2);
    temp |= 1 << 0;
    writel(temp, lcdblk_regs_base + LCDBLK_CFG2);
    temp = 0x00;
    temp = readl(lcdblk_regs_base + LCDBLK_CFG2);
	printk("%d [LCDBLK_CFG2 = %#x] \n",__LINE__,temp);
	mdelay(1000);
    
	//分频      800/(23 +1 ) == 33.3M
	//分频      800/(88 +1 ) == 8.9M
    temp = readl(lcd_regs_base + VIDCON0);
	temp &= ~(0xff << 6);
    temp |= (88 << 6);   //8.9Mhz
    writel(temp, lcd_regs_base + VIDCON0);
    temp = 0x00;
    temp = readl(lcd_regs_base + VIDCON0);
	printk("%d [VIDCON0 = %#x] \n",__LINE__,temp);




#if 1
	printk("%d fimd0_rate: = %ld \n",__LINE__,clk_get_rate(base_clk_fimd0));
	printk("%d aclk160_rate: = %ld \n",__LINE__,clk_get_rate(base_clk_aclk160));
	printk("%d lcd_43_rate: = %ld \n",__LINE__,clk_get_rate(lcd_43));
	printk("%d sclk_clk_fimd0_rate: = %ld \n",__LINE__,clk_get_rate(sclk_clk_fimd0));
#endif


	/*
     * VIDCON1:
     * [5]:IVSYNC  ===> 1 : Inverted(反转)
     * [6]:IHSYNC  ===> 1 : Inverted(反转)
     * [7]:IVCLK   ===> 1 : Fetches video data at VCLK rising edge (下降沿触发)
     * [10:9]:FIXVCLK  ====> 01 : VCLK running
     */
    temp = readl(lcd_regs_base + VIDCON1);
    temp |= (1 << 9) | (0 << 7) | (0 << 6) | (0 << 5) | (0 << 4);
    writel(temp, lcd_regs_base + VIDCON1);
    temp = 0x00;
    temp = readl(lcd_regs_base + VIDCON1);
	printk("%d [VIDCON1 = %#x] \n",__LINE__,temp);


	/*
	 * VIDCON2
	 * [15:14] RSVD ===> 1
	 *
 	 */
    temp = readl(lcd_regs_base + VIDCON2);
    temp |= (1 << 14);
    writel(temp, lcd_regs_base + VIDCON2);
    temp = 0x00;
    temp = readl(lcd_regs_base + VIDCON2);
	printk("%d [VIDCON2 = %#x] \n",__LINE__,temp);



	/*
     * VIDTCON0:
     * [23:16]:  VBPD + 1  <------> tvpw (1 - 20)  13
     * [15:8] :  VFPD + 1  <------> tvfp 22
     * [7:0]  :  VSPW  + 1 <------> tvb - tvpw = 23 - 13 = 10
     */
    temp = readl(lcd_regs_base + VIDTCON0);
//  temp |= (12 << 16) | (21 << 8) | (9);
    temp |= (1 << 16) | (1 << 8) | (9 << 0);
//  temp |= (9 << 16) | (9 << 8) | (9 << 0);
    writel(temp, lcd_regs_base + VIDTCON0);
    temp = 0x00;
    temp = readl(lcd_regs_base + VIDTCON0);
	printk("%d [VIDTCON0 = %#x] \n",__LINE__,temp);

	/*
     * VIDTCON1:
     * [23:16]:  HBPD + 1  <------> thpw (1 - 40)  36
     * [15:8] :  HFPD + 1  <------> thfp 210
     * [7:0]  :  HSPW  + 1 <------> thb - thpw = 46 - 36 = 10
     */
    temp = readl(lcd_regs_base + VIDTCON1);
//  temp |= (35 << 16) | (209 << 8)  | (9);
    temp |= (1 << 16) | (1 << 8)  | (40 << 0);
//  temp |= (29 << 16) | (9 << 8)  | (40);
    writel(temp, lcd_regs_base + VIDTCON1);
    temp = 0x00;
    temp = readl(lcd_regs_base + VIDTCON1);
	printk("%d [VIDTCON1 = %#x] \n",__LINE__,temp);

	/*
     * HOZVAL = (Horizontal display size) - 1 and LINEVAL = (Vertical display size) - 1.
     * Horizontal(水平) display size : 480
     * Vertical(垂直) display size : 272
     */
    temp = readl(lcd_regs_base + VIDTCON2);
    temp = ((LCD_WIDTH-1) << 11) | LCD_LENTH;
    writel(temp, lcd_regs_base + VIDTCON2);
    temp = 0x00;
    temp = readl(lcd_regs_base + VIDTCON2);
	printk("%d [VIDTCON2 = %#x] \n",__LINE__,temp);


	/*
	 * VIDTCON3
	 * [31] =======> 1 Enables VSYNC Signal Output
	 * [29] =======> 1 Enables FRM Signal Output
	 *
 	 */
    temp = readl(lcd_regs_base + VIDTCON3);
    temp |= (1 << 31) | (1 << 29);
    writel(temp, lcd_regs_base + VIDTCON3);
    temp = 0x00;
    temp = readl(lcd_regs_base + VIDTCON3);
	printk("%d [VIDTCON3 = %#x] \n",__LINE__,temp);



	/*
     * WINCON0:
     * [16]:Specifies Half-Word swap control bit.  1 = Enables swap P1779 低位像素存放在低字节
     * [5:2]: Selects Bits Per Pixel (BPP) mode for Window image : 0101 ===> 16BPP RGB565
     * [1]:Enables/disables video output   1 = Enables
     */
    temp = readl(lcd_regs_base + WINCON0);
    temp &= ~(0xf << 2);
    temp |= (1 << 15) | (0xd << 2) | 1; // 8  8  8
//    temp |= (1 << 15) | (0x5 << 2) | 1;   // 5  6  5
    writel(temp, lcd_regs_base + WINCON0);
    temp = 0x00;
    temp = readl(lcd_regs_base + WINCON0);
	printk("%d [WINCON0 = %#x] \n",__LINE__,temp);


	//Window Size For example, Height ? Width (number of word)
    temp = readl(lcd_regs_base + VIDOSD0C);
	temp = ((LCD_LENTH * LCD_WIDTH) >> 1);
    writel(temp, lcd_regs_base + VIDOSD0C);
	temp = 0x00;
    temp = readl(lcd_regs_base + VIDOSD0C);
	printk("%d [VIDOSD0C = %#x] \n",__LINE__,temp);



    temp = readl(lcd_regs_base + SHADOWCON);
	temp |= (1 << 0);
    writel(temp, lcd_regs_base + SHADOWCON);
    temp = 0x00;
    temp = readl(lcd_regs_base + SHADOWCON);
	printk("%d [SHADOWCON = %#x] \n",__LINE__,temp);

	//p1769
    temp = readl(lcd_regs_base + WINCHMAP2);
    temp &= ~(0x7 << 16);
    temp |= (1 << 16);

    temp &= ~(0x7 << 0);
    temp |= 1;
    writel(temp, lcd_regs_base + WINCHMAP2);
    temp = 0x00;
    temp = readl(lcd_regs_base + WINCHMAP2);
	printk("%d [WINCHMAP2 = %#x] \n",__LINE__,temp);

	/*
     * bit0-10 : 指定OSD图像左上像素的垂直屏幕坐标
     * bit11-21: 指定OSD图像左上像素的水平屏幕坐标
     */
    writel(0, lcd_regs_base + VIDOSD0A);
    
	/*
     * bit0-10 : 指定OSD图像右下像素的垂直屏幕坐标
     * bit11-21: 指定OSD图像右下像素的水平屏幕坐标
     */
    writel(((LCD_LENTH-1) << 11) | (LCD_WIDTH-1), lcd_regs_base + VIDOSD0B);
    

    /* 3.3 分配显存(framebuffer), 并把地址告诉LCD控制器 */
    // s3c_lcd->screen_base         显存虚拟地址
    // s3c_lcd->fix.smem_len        显存大小，前面计算的
    // s3c_lcd->fix.smem_start      显存物理地址
    s3c_lcd->screen_base = dma_alloc_writecombine(NULL, s3c_lcd->fix.smem_len, (dma_addr_t *)&s3c_lcd->fix.smem_start, GFP_KERNEL);
	printk("%d s3c_lcd->screen_base = %p \n",__LINE__,&(s3c_lcd->screen_base));

    //显存起始地址
    writel(s3c_lcd->fix.smem_start, lcd_regs_base + VIDW00ADD0B0);
    
	//显存结束地址
    writel(s3c_lcd->fix.smem_start + s3c_lcd->fix.smem_len, lcd_regs_base + VIDW00ADD1B0);


	//Enables video output and logic immediately
    temp = readl(lcd_regs_base + VIDCON0);
	temp |= 0x03;
    writel(temp, lcd_regs_base + VIDCON0);
	temp = 0x00;
    temp = readl(lcd_regs_base + VIDCON0);
	printk("%d [VIDCON0 = %#x] \n",__LINE__,temp);


    /* 4. 注册 */
    ret = register_framebuffer(s3c_lcd);
	if(ret < 0){
        printk("register_framebuffer error\n");
		return ret;
	}

    printk("platform_get_resource ok..........[done] \n");
	printk("======================================================= \n");
    return ret;
}

static int lcd_remove(struct platform_device *pdev)
{
    unregister_framebuffer(s3c_lcd);
    dma_free_writecombine(NULL, s3c_lcd->fix.smem_len, s3c_lcd->screen_base, s3c_lcd->fix.smem_start);
    framebuffer_release(s3c_lcd);
    return 0;
}

static const struct of_device_id lcd_dt_ids[] =
{
    { .compatible = "njpt4412,lcd_demo", },
    {},
};

MODULE_DEVICE_TABLE(of, lcd_dt_ids);

static struct platform_driver lcd_driver =
{
    .driver        = {
        .name      = "lcd_demo",
        .of_match_table    = of_match_ptr(lcd_dt_ids),
    },
    .probe         = lcd_probe,
    .remove        = lcd_remove,
};

static int lcd_init(void)
{
    int ret;

	printk("======================================================= \n");

    ret = platform_driver_register(&lcd_driver);

    if (ret){
        printk(KERN_ERR "lcd: probe fail: %d\n", ret);
    }

    return ret;
}

static void lcd_exit(void)
{
    printk("enter %s\n", __func__);
    platform_driver_unregister(&lcd_driver);
}

module_init(lcd_init);
module_exit(lcd_exit);


MODULE_AUTHOR("zhuchengzhi");
MODULE_DESCRIPTION("njpt4412 lcd driver");
MODULE_LICENSE("GPL");


