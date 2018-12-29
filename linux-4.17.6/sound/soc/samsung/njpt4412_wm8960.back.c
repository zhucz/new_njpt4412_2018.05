/*
 *  njpt4412-wm8960.c
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */

#include "../codecs/wm8960.h"
#include "./i2s.h"
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <sound/pcm.h>


#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/clk/clk-conf.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/pm_runtime.h>
#include <linux/sched.h>
#include <linux/clkdev.h>
#include <linux/stringify.h>

#define ITOP4412_WM8960 1
#define DEBUG	1

#ifdef	DEBUG
#define	dprintk( argc, argv... )		printk( argc, ##argv )
#else
#define	dprintk( argc, argv... )		
#endif


#define SAMSUNG_I2S_DIV_PRESCALER 2


/*------------2018-11-16 add by zhuchengzhi--------------------*/
#include <linux/platform_device.h>

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

struct clk *base_clk_iis0;
struct clk *base_clk_iis0_opclk0;
struct clk *base_clk_iis0_opclk1;
struct clk *base_clk_iis0_cdclk0;
static struct resource *res1;
static volatile void __iomem *iis0_regs_base;

/*-------------------------------------------------------------*/


 /*
  * Default CFG switch settings to use this driver:
  *	SMDKV310: CFG5-1000, CFG7-111111
  */

 /*
  * Configure audio route as :-
  * $ amixer sset 'DAC1' on,on
  * $ amixer sset 'Right Headphone Mux' 'DAC'
  * $ amixer sset 'Left Headphone Mux' 'DAC'
  * $ amixer sset 'DAC1R Mixer AIF1.1' on
  * $ amixer sset 'DAC1L Mixer AIF1.1' on
  * $ amixer sset 'IN2L' on
  * $ amixer sset 'IN2L PGA IN2LN' on
  * $ amixer sset 'MIXINL IN2L' on
  * $ amixer sset 'AIF1ADC1L Mixer ADC/DMIC' on
  * $ amixer sset 'IN2R' on
  * $ amixer sset 'IN2R PGA IN2RN' on
  * $ amixer sset 'MIXINR IN2R' on
  * $ amixer sset 'AIF1ADC1R Mixer ADC/DMIC' on
  */

/* SMDK has a 16.934MHZ crystal attached to WM8960 */
//#define SMDK_WM8960_FREQ 16934000
#define SMDK_WM8960_FREQ 112896000

struct smdk_wm8960_data {
	int mclk1_rate;
};

/* Default SMDKs */
static struct smdk_wm8960_data smdk_board_data = {
	.mclk1_rate = SMDK_WM8960_FREQ,
};


#if 1 //copy from web 

static int smdk_hw_params(struct snd_pcm_substream *substream,struct snd_pcm_hw_params *params)
{
#if 0
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct snd_soc_card *soc_card = rtd->card;
	unsigned int pll_out;
	int ret;

	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S
								| SND_SOC_DAIFMT_NB_NF
								| SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		return ret;

	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S
								| SND_SOC_DAIFMT_NB_NF
								| SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		return ret;

/* set the CPU system clock */  
	ret = snd_soc_dai_set_sysclk(cpu_dai, 1, 256, SND_SOC_CLOCK_OUT);  
	if (ret < 0){
		printk( "%s: AP I2SCLK RCLK setting error, %d\n", cpu_dai->name, ret );
		return ret;
	}

	ret = snd_soc_dai_set_sysclk(cpu_dai, 2, 256, SND_SOC_CLOCK_OUT);  
	if (ret < 0){
		printk( "%s: AP I2SCLK RCLK setting error, %d\n", cpu_dai->name, ret );
		return ret;
	}

	ret = snd_soc_dai_set_clkdiv(codec_dai,1, 16);
	if( ret < 0 ){
		printk( "%s: Codec DACDIV setting error, %d\n", codec_dai->name, ret );
		return ret;
	}

	ret = snd_soc_dai_set_clkdiv(codec_dai, WM8960_DACDIV, 1<<3);
	if( ret < 0 ){
		printk( "%s: Codec DACDIV setting error, %d\n", codec_dai->name, ret );
		return ret;
	}
#endif
	return 0;
}

#endif

#if 0  //copy from  smdk_wm8960.c  

int set_epll_rate(unsigned long rate )
{
        struct clk *fout_epll;

        fout_epll = clk_get(NULL, "fout_epll");
//      fout_epll = devm_clk_get(&pdev->dev, "fout_epll");

        if (IS_ERR(fout_epll)) {
                printk(KERN_ERR "%s: failed to get fout_epll\n", __func__);
                return PTR_ERR(fout_epll);
        }

        if (rate == clk_get_rate(fout_epll))
                goto out;

        clk_set_rate(fout_epll, rate);
		
out:
        clk_put(fout_epll);

        return 0;
}

static int smdk_hw_params(struct snd_pcm_substream *substream,struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	unsigned int rate = params_rate(params);
	snd_pcm_format_t fmt = params_format( params );

	int bclk_div = 0, bfs, psr = 0, rfs = 0, rclk_div = 0, ret = 0;

	unsigned int mul = 0;

	unsigned long rclk = 0;

	dprintk("+%s()\n", __FUNCTION__);

	switch (fmt){
		case SNDRV_PCM_FORMAT_S16:
			bclk_div = 32;
			break;
			
		case SNDRV_PCM_FORMAT_S20_3LE:
		case SNDRV_PCM_FORMAT_S24:
			bclk_div = 48;
			break;
			
		default:
			dprintk("-%s(): PCM FMT ERROR\n", __FUNCTION__);
			return -EINVAL;
	}

	switch (params_format(params)){
		case SNDRV_PCM_FORMAT_U24:
		case SNDRV_PCM_FORMAT_S24:
			bfs = 48;
			break;

		case SNDRV_PCM_FORMAT_U16_LE:
		case SNDRV_PCM_FORMAT_S16_LE:
			bfs = 32;
			break;
		default:
			return -EINVAL;
        }
	
	switch (params_rate(params)) {
        case 16000:
        case 22050:
        case 24000:
        case 32000:
        case 44100:
        case 48000:
        case 88200:
        case 96000:
                if (bfs == 48)
                        rfs = 384;
                else
                        rfs = 256;
                break;
        case 64000:
                rfs = 384;
                break;
        case 8000:
        case 11025:
        case 12000:
                if (bfs == 48)
                        rfs = 768;
                else
                        rfs = 512;
                break;
        default:
                return -EINVAL;
        }

	switch (rate){
		case 8000:
		case 11025:
		case 12000:
			if(bclk_div == 48)
				rclk_div = 768;	//0x300
			else
				rclk_div = 512;	//0x200
			break;
			
		case 16000:
		case 22050:
		case 24000:
		case 32000:
		case 44100:	//AC44
		case 48000:
		case 88200:
		case 96000:
			if(bclk_div == 48)
				rclk_div = 384;	//0x180
			else
				rclk_div = 256;	//0x100
			break;
		
		case 64000:
			rclk_div = 384;		//0x180
			break;
		
		default:
			dprintk("-%s(): SND RATE ERROR\n", __FUNCTION__);
			return -EINVAL;
	}

	rclk = params_rate(params) * rfs;

	switch (rclk){
		case 4096000:
		case 5644800:
		case 6144000:
		case 8467200:
		case 9216000:
		        psr = 8;
		        break;

		case 8192000:
		case 11289600:
		case 12288000:
		case 16934400:
		case 18432000:
		        psr = 4;
		        break;

		case 22579200:
		case 24576000:
		case 33868800:
		case 36864000:
		        psr = 2;
		        break;

		case 67737600:
		case 73728000:
		        psr = 1;
		        break;

		default:
		        printk("Not yet supported!\n");
		        return -EINVAL;
	}

	set_epll_rate(rclk * psr);
	dprintk("[zsb] rclk = %ld, psr = %d, bfs = %d\n", rclk, psr, bfs);
	
	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S
										| SND_SOC_DAIFMT_NB_NF
										| SND_SOC_DAIFMT_CBS_CFS);
	if(ret < 0)
	{
		dprintk("-%s(): Codec DAI configuration error, %d\n", __FUNCTION__, ret);
		return ret;
	}

	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S
										| SND_SOC_DAIFMT_NB_NF
										| SND_SOC_DAIFMT_CBS_CFS);
	if( ret < 0 )
	{
		dprintk("-%s(): AP DAI configuration error, %d\n", __FUNCTION__, ret);
		return ret;
	}

	ret = snd_soc_dai_set_sysclk(cpu_dai, SAMSUNG_I2S_CDCLK, 0, SND_SOC_CLOCK_OUT);
	if(ret < 0)
	{
		dprintk("-%s(): AP sycclk CDCLK setting error, %d\n", __FUNCTION__, ret);
		return ret;
	}

	ret = snd_soc_dai_set_clkdiv(codec_dai, WM8960_SYSCLKDIV, WM8960_SYSCLK_DIV_1);
	if( ret < 0 )
	{
		dprintk("-%s(): Codec SYSCLKDIV setting error, %d\n", __FUNCTION__, ret);
		return ret;
	}

	ret = snd_soc_dai_set_clkdiv(codec_dai, WM8960_DACDIV, WM8960_DAC_DIV_1);
	if(ret < 0)
	{
		dprintk("-%s(): Codec DACDIV setting error, %d\n", __FUNCTION__, ret);
		return ret;
	}

	mul = rate * rclk_div;
	
	ret = snd_soc_dai_set_clkdiv(codec_dai, WM8960_DCLKDIV, mul);
	if( ret < 0 )
	{
		dprintk( "-%s(): Codec DCLKDIV setting error, %d\n", __FUNCTION__, ret );
		return ret;
	}
	
	ret = snd_soc_dai_set_clkdiv(cpu_dai, SAMSUNG_I2S_DIV_BCLK, bfs);
	if(ret < 0)
	{
		dprintk("-%s(): AP prescalar setting error, %d\n", __FUNCTION__, ret);
		return ret;
	}

	ret = snd_soc_dai_set_clkdiv(cpu_dai, SAMSUNG_I2S_DIV_PRESCALER, psr);
	if(ret < 0)
	{
		dprintk("-%s(): AP RFS setting error, %d\n", __FUNCTION__, ret);
		return ret;
	}
	
	dprintk("-%s()\n", __FUNCTION__);
	
	return 0;

}
#endif

/*
 * SMDK WM8960 DAI operations.
 */
static struct snd_soc_ops smdk_ops = {
	.hw_params = smdk_hw_params,
};

#if 1
static const struct snd_soc_dapm_widget smdk4x12_dapm_capture_widgets[] = {
	SND_SOC_DAPM_MIC("Mic Jack",NULL ),
	SND_SOC_DAPM_LINE("Line Input 3 (FM)",NULL ),
};

static const struct snd_soc_dapm_widget smdk4x12_dapm_playback_widgets[] = {
	SND_SOC_DAPM_HP("Headphone Jack", NULL),
	SND_SOC_DAPM_SPK("Speaker_L", NULL),
	SND_SOC_DAPM_SPK("Speaker_R", NULL),
};

static const struct snd_soc_dapm_route smdk4x12_audio_map[] = {
	{"Headphone Jack", NULL, "HP_L"},
	{"Headphone Jack", NULL, "HP_R"},
	{"Speaker_L", NULL, "SPK_LP"},
	{"Speaker_L", NULL, "SPK_LN"},
	{"Speaker_R", NULL, "SPK_RP"},
	{"Speaker_R", NULL, "SPK_RN"},
	{"LINPUT1", NULL, "MICB"},
	{"MICB", NULL, "Mic Jack"},
};
#endif


static int smdk_wm8960_init_paiftx(struct snd_soc_pcm_runtime *rtd)
{
	/* Other pins NC */
#if 0 

//	struct snd_soc_dapm_context *dapm = &rtd->card->dapm;
//	snd_soc_dapm_enable_pin(dapm, "Headphone Jack");
//	snd_soc_dapm_enable_pin(dapm, "Speaker");
	snd_soc_dapm_enable_pin(dapm, "SPK_LP");
	snd_soc_dapm_enable_pin(dapm, "SPK_LN");
	snd_soc_dapm_enable_pin(dapm, "SPK_RP");
	snd_soc_dapm_enable_pin(dapm, "SPK_RN");
	snd_soc_dapm_enable_pin(dapm, "LINPUT1");
	snd_soc_dapm_enable_pin(dapm, "LINPUT3");
	snd_soc_dapm_enable_pin(dapm, "RINPUT1");
	snd_soc_dapm_enable_pin(dapm, "RINPUT2");


#else 

#if 1 //copy from web 
	struct snd_soc_dapm_context *dapm = &rtd->card->dapm;
	int err;  
	struct snd_soc_codec *codec = rtd->codec;  
	struct clk *fout_epll;
	struct clk *mout_audss;
	fout_epll = __clk_lookup("fout_epll");
	mout_audss = __clk_lookup("mout_audss");
//	clk_set_rate(fout_epll,67737602);
//	clk_set_rate(fout_epll,112896000); 
	clk_set_rate(fout_epll,1411000); 
	clk_set_parent(mout_audss,fout_epll);

	snd_soc_dapm_new_controls( dapm, smdk4x12_dapm_playback_widgets, ARRAY_SIZE( smdk4x12_dapm_playback_widgets ) );
	snd_soc_dapm_add_routes( dapm, smdk4x12_audio_map, ARRAY_SIZE( smdk4x12_audio_map ) );
	snd_soc_dapm_enable_pin(dapm, "Headphone Jack");
	snd_soc_dapm_sync( dapm );

#else //copy from  smdk_wm8960.c
	struct snd_soc_codec *codec = rtd->codec;
//	struct snd_soc_dapm_context *dapm = &codec->dapm;
	struct snd_soc_dapm_context *dapm = &rtd->card->dapm;
	
	dprintk("+%s()\n", __FUNCTION__);

	snd_soc_dapm_nc_pin(dapm, "RINPUT1");
	snd_soc_dapm_nc_pin(dapm, "LINPUT2");
	snd_soc_dapm_nc_pin(dapm, "RINPUT2");
	snd_soc_dapm_nc_pin(dapm, "OUT3");
	
	snd_soc_dapm_new_controls(dapm, smdk4x12_dapm_capture_widgets, ARRAY_SIZE(smdk4x12_dapm_capture_widgets));
	snd_soc_dapm_new_controls(dapm, smdk4x12_dapm_playback_widgets, ARRAY_SIZE(smdk4x12_dapm_playback_widgets));

	snd_soc_dapm_add_routes(dapm, smdk4x12_audio_map, ARRAY_SIZE(smdk4x12_audio_map));

	snd_soc_dapm_enable_pin(dapm, "Headphone Jack");
	snd_soc_dapm_enable_pin(dapm, "Mic Jack");
	snd_soc_dapm_enable_pin(dapm, "Speaker_L");
	snd_soc_dapm_enable_pin(dapm, "Speaker_R");
	
	snd_soc_dapm_disable_pin(dapm, "Line Input 3 (FM)");

	dprintk("*%s(): dapm sync start\n", __FUNCTION__);
	snd_soc_dapm_sync( dapm );
	dprintk("*%s(): dapm sync end\n", __FUNCTION__);

	dprintk("-%s()\n", __FUNCTION__);
	
	return 0;
#endif
#endif


	return 0;
}

static struct snd_soc_dai_link smdk_dai[] = {

#if 1	
//	{ /* Primary DAI i/f */
//		.name = "WM8960 AIF1",
//		.stream_name = "Pri_Dai",
//		.cpu_dai_name = "samsung-i2s",
//		.codec_dai_name = "wm8960-hifi",
//		.platform_name = "samsung-idma",
//		.codec_name = "wm8960",
//		.init = smdk_wm8960_init_paiftx,
//		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBM_CFM,
//		.ops = &smdk_ops,
//	},

	{ /* Primary DAI i/f */
		.name		= "wm8960",
		.stream_name	= "Playback",
		.codec_dai_name	= "wm8960-hifi",
//		.cpu_dai_name = "samsung-i2s",
//		.platform_name = "samsung-audio",
//		.codec_name = "wm8960",
		.init = smdk_wm8960_init_paiftx,
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBM_CFM,
		.ops = &smdk_ops,
	},

#else
	{ /* Primary DAI i/f */
		.name		= "wm8960",
		.stream_name	= "Playback",
		.codec_dai_name	= "wm8960-hifi",
//		.cpu_dai_name = "samsung-i2s",
//		.platform_name = "samsung-audio",
//		.codec_name = "wm8960",
		.init = smdk_wm8960_init_paiftx,
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBM_CFM,
		.ops = &smdk_ops,
	},

#endif





};

static struct snd_soc_card smdk = {
//	.name = "SMDK-I2S",
	.name = "WM-8960A",
	.owner = THIS_MODULE,
	.dai_link = smdk_dai,
	.num_links = ARRAY_SIZE(smdk_dai),
};

static const struct of_device_id samsung_wm8960_of_match[] = {
	{ .compatible = "samsung,njpt4412-wm8960", .data = &smdk_board_data },
	{},
};
MODULE_DEVICE_TABLE(of, samsung_wm8960_of_match);


static int smdk_audio_probe(struct platform_device *pdev)
{
	int ret;
	struct device_node *np = pdev->dev.of_node;
	struct snd_soc_card *card = &smdk;
	struct smdk_wm8960_data *board;
	const struct of_device_id *id;

/*------------2018-11-16 add by zhuchengzhi--------------------*/
    unsigned int temp;

 	struct device *dev = &pdev->dev;
/*-------------------------------------------------------------*/

	card->dev = &pdev->dev;

	board = devm_kzalloc(&pdev->dev, sizeof(*board), GFP_KERNEL);
	if (!board)
		return -ENOMEM;

	if (np) {
		smdk_dai[0].cpu_dai_name = NULL;
		smdk_dai[0].cpu_of_node = of_parse_phandle(np,"samsung,i2s-controller", 0);
		if (!smdk_dai[0].cpu_of_node) {
			dev_err(&pdev->dev,
			   "Property 'samsung,i2s-controller' missing or invalid\n");
			ret = -EINVAL;
		}

		smdk_dai[0].platform_name = NULL;
		smdk_dai[0].platform_of_node = smdk_dai[0].cpu_of_node;
		smdk_dai[0].codec_of_node = of_parse_phandle(np,"samsung,audio-codec",0) ;

		if (!smdk_dai[0].codec_of_node) {
			dev_err(&pdev->dev,
			   "Property 'samsung,audio-codec' missing or invalid\n");
			ret = -EINVAL;
		}


		printk("%d : %s \n",__LINE__,__func__);
	}

	id = of_match_device(of_match_ptr(samsung_wm8960_of_match), &pdev->dev);
	if (id){
		*board = *((struct smdk_wm8960_data *)id->data);
		printk("%d : %s \n",__LINE__,__func__);
	}

	printk("%d : %s \n",__LINE__,__func__);



/*------------2018-11-16 add by zhuchengzhi TODO--------------------*/
//I2S时钟 IIS
    base_clk_iis0 = devm_clk_get(&pdev->dev, "iis");
    if (IS_ERR(base_clk_iis0)) {
        dev_err(dev, "failed to get timer base clk\n");
        return PTR_ERR(base_clk_iis0);
    }

    ret = clk_prepare_enable(base_clk_iis0);
    if (ret < 0) {
        dev_err(dev, "failed to enable base clock\n");
        return ret;
    }
	printk("iis0_rate: = %ld \n",clk_get_rate(base_clk_iis0));


//I2S时钟 I2S_OPCLK0
    base_clk_iis0_opclk0 = devm_clk_get(&pdev->dev, "i2s_opclk0");
    if (IS_ERR(base_clk_iis0_opclk0)) {
        dev_err(dev, "failed to get timer base clk\n");
        return PTR_ERR(base_clk_iis0_opclk0);
    }

    ret = clk_prepare_enable(base_clk_iis0_opclk0);
    if (ret < 0) {
        dev_err(dev, "failed to enable base clock\n");
        return ret;
    }
	printk("iis0_opclk0_rate: = %ld \n",clk_get_rate(base_clk_iis0_opclk0));

//I2S时钟 I2S_OPCLK1
    base_clk_iis0_opclk1 = devm_clk_get(&pdev->dev, "i2s_opclk1");
    if (IS_ERR(base_clk_iis0_opclk1)) {
        dev_err(dev, "failed to get timer base clk\n");
        return PTR_ERR(base_clk_iis0_opclk1);
    }

    ret = clk_prepare_enable(base_clk_iis0_opclk1);
    if (ret < 0) {
        dev_err(dev, "failed to enable base clock\n");
        return ret;
    }
	printk("iis0_opclk1_rate: = %ld \n",clk_get_rate(base_clk_iis0_opclk1));

//I2S时钟 I2S_CDCLK0
    base_clk_iis0_cdclk0 = devm_clk_get(&pdev->dev, "i2s_cdclk0");
    if (IS_ERR(base_clk_iis0_cdclk0)) {
        dev_err(dev, "failed to get timer base clk\n");
        return PTR_ERR(base_clk_iis0_cdclk0);
    }

    ret = clk_prepare_enable(base_clk_iis0_cdclk0);
    if (ret < 0) {
        dev_err(dev, "failed to enable base clock\n");
        return ret;
    }
	printk("iis0_cdclk0_rate: = %ld \n",clk_get_rate(base_clk_iis0_cdclk0));


	//寄存器映射 0x03830000 0x100
    res1 = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (res1 == NULL){
        printk("platform_get_resource error\n");
        return -EINVAL;
    }
    printk("res1: %x\n",(unsigned int)res1->start);


    iis0_regs_base = devm_ioremap_resource(&pdev->dev, res1);
    if (iis0_regs_base == NULL){
        printk("devm_ioremap_resource error\n");
        return -EINVAL;
    } 


    temp = readl(iis0_regs_base);
//    temp |= (1 << 9) | (0 << 7) | (0 << 6) | (0 << 5) | (0 << 4);
    writel(temp, iis0_regs_base);
    temp = 0x00;
    temp = readl(iis0_regs_base);
	printk("%d [VIDCON1 = %#x] \n",__LINE__,temp);

/*-------------------------------------------------------------*/



	platform_set_drvdata(pdev, board);

	ret = devm_snd_soc_register_card(&pdev->dev, card);

	if (ret)
		dev_err(&pdev->dev, "snd_soc_register_card() failed:%d\n", ret);

	return ret;
}

static struct platform_driver smdk_audio_driver = {
	.driver		= {
		.name	= "wm8960",
		.of_match_table = of_match_ptr(samsung_wm8960_of_match),
		.pm	= &snd_soc_pm_ops,
	},
	.probe		= smdk_audio_probe,
};

module_platform_driver(smdk_audio_driver);

MODULE_DESCRIPTION("ALSA SoC SMDK WM8960");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:njpt4412-audio-wm8960");
