/*
 * rk1000_tv.c
 *
 * Driver for rockchip rk1000 tv control
 *  Copyright (C) 2009
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *
 */
#include <linux/module.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/fb.h>
#include <linux/rk_fb.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <dt-bindings/rkfb/rk_fb.h>
#endif
#include "rk1000_tve.h"

struct rk1000_tve rk1000_tve;
int cvbsformat = -1;

/*static int __init bootloader_rk1000_setup(char *str){	
	static int ret;	
	if (str) {		
		printk("cvbs init tve.format is %s\n", str);		
		ret = sscanf(str, "%d", &cvbsformat);
	}	
	return 0;
}
early_param("tve.format", bootloader_rk1000_setup);*/

int rk1000_switch_fb(const struct fb_videomode *modedb, int tv_mode)
{
	struct rk_screen *screen;

	if (modedb == NULL)
		return -1;
	screen =  kzalloc(sizeof(*screen), GFP_KERNEL);
	if (screen == NULL)
		return -1;
	memset(screen, 0, sizeof(*screen));
	/* screen type & face */
	screen->type = SCREEN_RGB;
	screen->face = OUT_P888;
	screen->mode = *modedb;
	screen->mode.vmode = 0;
	/* Pin polarity */
	if (FB_SYNC_HOR_HIGH_ACT & modedb->sync)
		screen->pin_hsync = 1;
	else
		screen->pin_hsync = 0;
	if (FB_SYNC_VERT_HIGH_ACT & modedb->sync)
		screen->pin_vsync = 1;
	else
		screen->pin_vsync = 0;
	screen->pin_den = 0;
	screen->pin_dclk = 0;
	//screen->pin_dclk = 1;
	/* Swap rule */
	screen->swap_rb = 0;
	screen->swap_rg = 0;
	screen->swap_gb = 0;
	screen->swap_delta = 0;
	screen->swap_dumy = 0;
	/*screen->overscan.left = 95;
	screen->overscan.top = 95;
	screen->overscan.right = 95;
	screen->overscan.bottom = 95;*/
	screen->overscan.left = 100;
	screen->overscan.top = 100;
	screen->overscan.right = 100;
	screen->overscan.bottom = 100;
	/* Operation function*/
	screen->init = NULL;
	screen->standby = NULL;
	rk_fb_switch_screen(screen, 1 , rk1000_tve.video_source);
	rk1000_tve.mode = tv_mode;
	kfree(screen);
	return 0;
}


static int rk1000_tve_initial(void)
{
	struct rk_screen screen;

	/* RK1000 tvencoder i2c reg need dclk, so we open lcdc.*/
	memset(&screen, 0, sizeof(struct rk_screen));
	/* screen type & face */
	screen.type = SCREEN_RGB;
	screen.face = OUT_P888;
	/* Screen size */
	/*screen.mode.xres = 720;
	screen.mode.yres = 480;*/
	/* Timing */
	/*screen.mode.pixclock = 27000000;
	screen.mode.refresh = 60;
	screen.mode.left_margin = 116;
	screen.mode.right_margin = 16;
	screen.mode.hsync_len = 6;
	screen.mode.upper_margin = 25;
	screen.mode.lower_margin = 14;
	screen.mode.vsync_len = 6;*/
	/* Screen size */
	screen.mode.xres = 1920;
	screen.mode.yres = 1080;
	/* Timing */
	screen.mode.pixclock = 148500000;
	screen.mode.refresh = 60;
	screen.mode.left_margin = 148;
	screen.mode.right_margin = 88;
	screen.mode.hsync_len = 36;
	screen.mode.upper_margin = 4;
	screen.mode.lower_margin = 44;
	screen.mode.vsync_len = 5;
	rk_fb_switch_screen(&screen, 2 , 0);
	/* Power down RK1000 output DAC. */
	return 0;
}

static int rk1000_tve_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "rk3288 hdmi driver removed.\n");
	return 0;
}


static int rk1000_tve_probe(struct platform_device *pdev)
{
	//struct device_node *tve_np;
	int rc;
	rk1000_tve.video_source = 0;
	rk1000_tve.property = 0;
	if (cvbsformat >= 0)		
		rk1000_tve.mode = cvbsformat + 1;	
	else
		rk1000_tve.mode = 1; //1280x710P		
		//rk1000_tve.mode = RK1000_TVOUT_DEAULT;
	//printk("+++++++++++ rk1000_tve.mode is: %d\n", rk1000_tve.mode);
	rc = rk1000_tve_initial();
	if (rc) {
		return -EINVAL;
	}
	rk1000_register_display_cvbs(NULL);
	return 0;
}

static const struct of_device_id rk1000_tve_ids[] = {
	{.compatible = "rockchip,rk1000_tve",},
	{}
};

static struct platform_driver rk1000_tve_driver = {
	.driver		= {
		.name	= "rk1000_tve",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(rk1000_tve_ids),
	},
	.probe = rk1000_tve_probe,
	.remove = rk1000_tve_remove,
};

static int __init rk1000_tve_init(void)
{
	return platform_driver_register(&rk1000_tve_driver);
}

static void __exit rk1000_tve_exit(void)
{
	return platform_driver_unregister(&rk1000_tve_driver);
}

module_init(rk1000_tve_init);
module_exit(rk1000_tve_exit);

/* Module information */
MODULE_DESCRIPTION("ROCKCHIP rk1000 TV Encoder ");
MODULE_LICENSE("GPL");
