#include <linux/ctype.h>
#include <linux/string.h>
#include <linux/delay.h>
#include "rk1000_tve.h"
#include <linux/kernel.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#ifdef CONFIG_AX100
static const struct fb_videomode rk1000_cvbs_mode[] = {	
	{"1280x720p", 60, 1280, 720, 57231360, 6, 5, 6, 5, 5, 5, 0, 0, 0},
	{"1920x1080p", 60, 1920, 1080, 148500000, 148, 88, 36, 4, 44, 5, 0, 0, 0},
};
#endif
#ifdef CONFIG_AX200
static  struct fb_videomode rk1000_cvbs_mode[] = {
	{"1920x1080p", 60, 1920, 1080, 148500000, 148, 88, 36, 4, 44, 5, 0, 0, 0},
	//{"1080x1920p", 60, 1080, 1920, 148500000, 40, 40, 60, 40, 40, 42, 0, 0, 0},
	{"1088x1920p", 60, 1088, 1920, 132652800, 10, 10, 10, 10, 20, 20, 0, 0, 0},
	{"1280x720p", 60, 1280, 720, 57231360, 6, 5, 6, 5, 5, 5, 0, 0, 0},
	//{"1920x1080p", 60, 1920, 1080, 125498160, 2, 2, 2, 2, 2, 2, 0, FB_VMODE_NONINTERLACED, 0},
	//{"1080x1920p", 60, 1080, 1920, 125498160, 2, 2, 2, 2, 2, 2, 0, FB_VMODE_NONINTERLACED, 0},
	{"3072x768p", 60, 3072, 768, 142942320, 2, 2, 2, 2, 2, 2, 0, FB_VMODE_NONINTERLACED, 0},
	{"4096x512p", 60, 4096, 512, 132978720, 30, 30, 14, 3, 10, 3, 0, FB_VMODE_NONINTERLACED, 0},
	/*{"1080x1920p", 60, 1080, 1920, 148500000, 36, 4, 148, 88, 5, 44, 0, 0, 0},
	{"600x800p", 60, 600, 800, 29306160, 2, 2, 2, 2, 2, 2, 0, FB_VMODE_NONINTERLACED, 0},
	{"800x1000p", 60, 800, 1000, 48650160, 2, 2, 2, 2, 2, 2, 0, FB_VMODE_NONINTERLACED, 0},
	{"720x1080p", 60, 720, 1080, 47306160, 2, 2, 2, 2, 2, 2, 0, FB_VMODE_NONINTERLACED, 0},
	{"480x800p", 60, 480, 800, 23502960, 2, 2, 2, 2, 2, 2, 0, FB_VMODE_NONINTERLACED, 0},*/	
	{"2048x1152p", 60, 2048, 1152, 142711920, 2, 2, 2, 2, 2, 2, 0, FB_VMODE_NONINTERLACED, 0 },
	
};
#endif

static struct rk1000_monspecs cvbs_monspecs;
extern int cvbsformat;

static int rk1000_cvbs_set_enable(struct rk_display_device *device, int enable)
{

	if (cvbs_monspecs.suspend)
		return 0;
	if ((cvbs_monspecs.enable != enable) ||
	    (cvbs_monspecs.mode_set != rk1000_tve.mode)) {
		if ((enable == 0) && cvbs_monspecs.enable) {
			cvbs_monspecs.enable = 0;
		} else if (enable == 1) {

		  if(cvbsformat>=0){
		  		rk1000_switch_fb(cvbs_monspecs.mode,
					 cvbs_monspecs.mode_set);
				cvbsformat=-1;
		  	}else{	
				rk1000_switch_fb(cvbs_monspecs.mode,
						 cvbs_monspecs.mode_set);
		  	}		
			cvbs_monspecs.enable = 1;
		}
	}
	return 0;
}

static int rk1000_cvbs_get_enable(struct rk_display_device *device)
{
/***************************/
        int i;
        struct file *fp;
        mm_segment_t fs;
        loff_t pos;
        char x[5]="1920";
        char y[5]="1080";
        struct fb_videomode p = {"1920x1080p", 60, 1920, 1080, 148500000, 2, 2, 2, 2, 2, 2, 0, FB_VMODE_NONINTERLACED, 0 };
        fp = filp_open("/cache/newmode.txt", O_RDWR, 0666);
        if (IS_ERR(fp)){
                printk("No create file error/n");
        }
        else{
                fs = get_fs();
                set_fs(KERNEL_DS);;
                pos = 0;
                if(fp->f_op->read(fp, x, 4, &fp->f_pos)) printk("read x success");
                else printk("read x fail");
                pos = 4;
                if(vfs_read(fp, y, 4, &pos)) printk("read y success");
		else printk("read y fail");
                filp_close(fp,NULL);
                set_fs(fs);
                printk("+++++++++%s+++++++++++", x);
                printk("+++++++++%s+++++++++++", y);
                p.name = y;
                p.xres = (int)simple_strtol(x, NULL, 10);
                p.yres = (int)simple_strtol(y, NULL, 10);;
                p.pixclock = (p.xres+6)*(p.yres+6)*60;
                
		rk1000_cvbs_mode[5] = p;
                INIT_LIST_HEAD(&cvbs_monspecs.modelist);
                for (i = 0; i < ARRAY_SIZE(rk1000_cvbs_mode); i++)
                        display_add_videomode(&rk1000_cvbs_mode[i],
                                      &cvbs_monspecs.modelist);
                if (rk1000_tve.mode < TVOUT_YPBPR_720X480P_60) {
                        cvbs_monspecs.mode = (struct fb_videomode *)
                                      &(rk1000_cvbs_mode[rk1000_tve.mode - 1]);
                        cvbs_monspecs.mode_set = rk1000_tve.mode;
                } else {
                        cvbs_monspecs.mode = (struct fb_videomode *)
                                        &(rk1000_cvbs_mode[0]);
                        cvbs_monspecs.mode_set = TVOUT_CVBS_NTSC;
                }
                rk1000_tve.cvbs = &cvbs_monspecs;

        }
/**************************/
	return cvbs_monspecs.enable;
}

static int rk1000_cvbs_get_status(struct rk_display_device *device)
{
	/*if (rk1000_tve.mode < TVOUT_YPBPR_720X480P_60)
		return 1;
	else
		return 0;*/
	return 1;
}

static int rk1000_cvbs_get_modelist(struct rk_display_device *device,
				    struct list_head **modelist)
{
	*modelist = &(cvbs_monspecs.modelist);
	return 0;
}

static int rk1000_cvbs_set_mode(struct rk_display_device *device,
				struct fb_videomode *mode)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(rk1000_cvbs_mode); i++) {
		if (fb_mode_is_equal(&rk1000_cvbs_mode[i], mode)) {
			if (((i + 1) != rk1000_tve.mode)) {
				cvbs_monspecs.mode_set = i + 1;
				cvbs_monspecs.mode = (struct fb_videomode *)
							&rk1000_cvbs_mode[i];
			}
			return 0;
		}
	}
	return -1;
}

static int rk1000_cvbs_get_mode(struct rk_display_device *device,
				struct fb_videomode *mode)
{
	*mode = *(cvbs_monspecs.mode);
	return 0;
}
/******************************/
static int rk1000_cvbs_add_mode(const char * value)
{
        int i;
        char x[5], y[5];
        int xres, yres;
        struct fb_videomode p = {"2200x666p", 60, 2200, 666, 129656880, 2, 2, 2, 2, 2, 2, 0, FB_VMODE_NONINTERLACED, 0 };
        struct file *fp;
        mm_segment_t fs;
        loff_t pos;

        i = 0;
        while(*value != 'x') 
	{
		x[i] = *value; 
		i++;
                value++;
	}
	x[i] = '\0';
	i = 0;
        value++;
        while(*value != 'p')
	{
		y[i] = *value;
		i++;
		value++;
	}
        y[i] = '\0';
        xres = (int)simple_strtol(x, NULL, 10);
        yres = (int)simple_strtol(y, NULL, 10);
//        printk("%d==", xres);
//        printk("%d==", yres);	
        p.name = value;
	p.xres = xres;
	p.yres = yres;
	p.pixclock = (xres+6)*(yres+6)*60;

	if(p.pixclock > 148500000){
		p.xres = 2048;
		p.yres = 1152;
		p.pixclock = 2054*1158*60;
		strcpy(x, "2048\0");
		strcpy(y, "1152\0");
	}
	if((p.xres + p.yres) < 1024){
		p.xres = 512;
		p.yres = 512;
		p.pixclock = 518*518*60;
		strcpy(x, "512\0");
		strcpy(y, "512\0");
	}

	rk1000_cvbs_mode[5] = p;
        
	fp = filp_open("/cache/newmode.txt", O_RDWR|O_CREAT, 0777);
        if (IS_ERR(fp)){
                printk("create file error add mode fail!/n");
                return -1;
        }
        fs =get_fs();
        set_fs(KERNEL_DS);
        pos = 0;
        vfs_write(fp, x, sizeof(x), &pos);
	pos = 4;
        vfs_write(fp, y, sizeof(y), &pos);
        filp_close(fp,NULL);
        set_fs(fs);


        INIT_LIST_HEAD(&cvbs_monspecs.modelist);
        for (i = 0; i < ARRAY_SIZE(rk1000_cvbs_mode); i++)
                display_add_videomode(&rk1000_cvbs_mode[i],
                                      &cvbs_monspecs.modelist);
        if (rk1000_tve.mode < TVOUT_YPBPR_720X480P_60) {
                cvbs_monspecs.mode = (struct fb_videomode *)
                                      &(rk1000_cvbs_mode[rk1000_tve.mode - 1]);
                cvbs_monspecs.mode_set = rk1000_tve.mode;
        } else {
                cvbs_monspecs.mode = (struct fb_videomode *)
                                        &(rk1000_cvbs_mode[0]);
                cvbs_monspecs.mode_set = TVOUT_CVBS_NTSC;
        }
        rk1000_tve.cvbs = &cvbs_monspecs;
        printk("add new mode success");

	return 0;
}

/****************************/
static struct rk_display_ops rk1000_cvbs_display_ops = {
	.setenable = rk1000_cvbs_set_enable,
	.getenable = rk1000_cvbs_get_enable,
	.getstatus = rk1000_cvbs_get_status,
	.getmodelist = rk1000_cvbs_get_modelist,
	.setmode = rk1000_cvbs_set_mode,
	.getmode = rk1000_cvbs_get_mode,
	.addmode = rk1000_cvbs_add_mode,
};

static int rk1000_display_cvbs_probe(struct rk_display_device *device,
				     void *devdata)
{
	device->owner = THIS_MODULE;
	strcpy(device->type, "TV");
	device->name = "cvbs";
	device->priority = DISPLAY_PRIORITY_TV;
	device->property = 0;
	device->priv_data = devdata;
	device->ops = &rk1000_cvbs_display_ops;
	return 1;
}

static struct rk_display_driver display_rk1000_cvbs = {
	.probe = rk1000_display_cvbs_probe,
};

int rk1000_register_display_cvbs(struct device *parent)
{
	int i;
	
	memset(&cvbs_monspecs, 0, sizeof(struct rk1000_monspecs));
	INIT_LIST_HEAD(&cvbs_monspecs.modelist);
	for (i = 0; i < ARRAY_SIZE(rk1000_cvbs_mode); i++)
		display_add_videomode(&rk1000_cvbs_mode[i],
				      &cvbs_monspecs.modelist);
	if (rk1000_tve.mode < TVOUT_YPBPR_720X480P_60) {
		cvbs_monspecs.mode = (struct fb_videomode *)
				      &(rk1000_cvbs_mode[rk1000_tve.mode - 1]);
		cvbs_monspecs.mode_set = rk1000_tve.mode;
	} else {
		cvbs_monspecs.mode = (struct fb_videomode *)
					&(rk1000_cvbs_mode[0]);
		cvbs_monspecs.mode_set = TVOUT_CVBS_NTSC;
	}
	cvbs_monspecs.ddev = rk_display_device_register(&display_rk1000_cvbs,
							parent, NULL);
	rk1000_tve.cvbs = &cvbs_monspecs;
	return 0;
}
