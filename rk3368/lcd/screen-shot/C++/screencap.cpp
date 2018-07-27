#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>  
#include <sys/mman.h>  
#include <linux/kd.h>  
#include <memory.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <binder/ProcessState.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>

#include <ui/PixelFormat.h>
#include <sys/time.h>
#include <SkImageEncoder.h>
#include <SkBitmap.h>
#include <SkData.h>
#include <SkStream.h>

using namespace android;
typedef struct bmp_header 
{
	short twobyte;
	//14B
	char bfType[2];
	unsigned int bfSize;
	unsigned int bfReserved1;
	unsigned int bfOffBits;
}BMPHEADER;
 
typedef struct bmp_info
{
	//40B
	unsigned int biSize;
	int biWidth;
	int biHeight;
	unsigned short biPlanes	;
	unsigned short biBitCount;
	unsigned int biCompression;
#define BI_RGB        0L	
#define BI_RLE8       1L	
#define BI_RLE4       2L	
#define BI_BITFIELDS  3L	
	unsigned int biSizeImage;
	int biXPelsPerMeter;
	int biYPelsPerMeter;
	unsigned int biClrUsed;
	unsigned int biClrImportant;
}BMPINFO; 
 
typedef struct tagRGBQUAD {
	unsigned char rgbBlue;
	unsigned char rgbGreen;
	unsigned char rgbRed;
	unsigned char rgbReserved;
} RGBQUAD;
 
typedef struct tagBITMAPINFO {
    BMPINFO    bmiHeader;
    //RGBQUAD    bmiColors[1];
	unsigned int rgb[3];
} BITMAPINFO;
 
static int get_rgb888_header(int w, int h, BMPHEADER * head, BMPINFO * info)
{
	int size = 0;
	if (head && info) {
		size = w * h * 3;
		memset(head, 0, sizeof(* head));
		memset(info, 0, sizeof(* info));
		head->bfType[0] = 'B';
		head->bfType[1] = 'M';
		head->bfOffBits = 14 + sizeof(* info);
		head->bfSize = head->bfOffBits + size;
		head->bfSize = (head->bfSize + 3) & ~3;
		size = head->bfSize - head->bfOffBits;
		head->bfReserved1 = 0; 

		info->biSize = sizeof(BMPINFO);
		info->biWidth = w;
		info->biHeight = -h;
		info->biPlanes = 1;
		info->biBitCount = 32;
		info->biCompression = 0;
		info->biSizeImage = size;
                info->biXPelsPerMeter = 4000;
                info->biYPelsPerMeter = 4000;
                info->biClrUsed = 0;
                info->biClrImportant = 0;

		printf("rgb888:%dbit,%d*%d,%d\n", info->biBitCount, w, h, head->bfSize);
	}
	return size;
}
 
static int get_rgb565_header(int w, int h, BMPHEADER * head, BITMAPINFO * info)
{
	int size = 0;
	if (head && info) {
		size = w * h * 2;
		memset(head, 0, sizeof(* head));
		memset(info, 0, sizeof(* info));
		head->bfType[0] = 'B';
		head->bfType[1] = 'M';
		head->bfOffBits = 14 + sizeof(* info);
		head->bfSize = head->bfOffBits + size;
		head->bfSize = (head->bfSize + 3) & ~3;
		size = head->bfSize - head->bfOffBits;
		
		info->bmiHeader.biSize = sizeof(info->bmiHeader);
		info->bmiHeader.biWidth = w;
		info->bmiHeader.biHeight = -h;
		info->bmiHeader.biPlanes = 1;
		info->bmiHeader.biBitCount = 16;
		info->bmiHeader.biCompression = BI_BITFIELDS;
		info->bmiHeader.biSizeImage = size;
 
		info->rgb[0] = 0xF800;
		info->rgb[1] = 0x07E0;
		info->rgb[2] = 0x001F;
 
		printf("rgb565:%dbit,%d*%d,%d\n", info->bmiHeader.biBitCount, w, h, head->bfSize);
	}
	return size;
}
/* 
static int save_bmp_rgb565(FILE * hfile, int w, int h, void * pdata)
{
	int success = 0;
	int size = 0;
	BMPHEADER head;
	BITMAPINFO info;
	
	size = get_rgb565_header(w, h, &head, &info);
	if (size > 0) {
		fwrite(head.bfType, 1, 14, hfile);
		fwrite(&info, 1, sizeof(info), hfile);
		fwrite(pdata, 1, size, hfile);
		success = 1;
	}
 
	return success;
}
 
static int save_bmp_rgb888(FILE * hfile, int w, int h, void * pdata)
{
	int success = 0;
	int size = 0;
	BMPHEADER head;
	BMPINFO info;
	
	size = get_rgb888_header(w, h, &head, &info);
	if (size > 0) {
		fwrite(head.bfType, 1, 14, hfile);
		fwrite(&info, 1, sizeof(info), hfile);
		fwrite(pdata, 1, size, hfile);
		success = 1;
	}
	
	return success;
}

int RGB565_to_RGB24(unsigned char *rgb565,unsigned char *rgb24,int width,int height)
{
    int i;
    int whole = width*height;
    unsigned char r,g,b;
    unsigned short  *pix565;

    pix565 = (unsigned short *)rgb565;
    for(i = 0;i < whole;i++)
    {    
        r = (unsigned char)( ((*pix565)>>11)&0x001f);
        *rgb24 = (r<<3) | (r>>2);
        rgb24++;
        g = (unsigned char)(((*pix565)>>5)&0x003f);
        *rgb24 = (g<<2) | (g>>4);
        rgb24++;
        b = (unsigned char)((*pix565)&0x001f);
        *rgb24 = (b<<3) | (b>>2);
        rgb24++;
        pix565++;             
    }
    return 0;
}
int BGRtoRGB(unsigned char *bgr, unsigned char *rgb, int width,int height)
{
	unsigned int whole = width * height;
	unsigned int i;
	unsigned short *bgr2;
	unsigned short *rgb2;
	bgr2 = (unsigned short *)bgr;
	rgb2 = (unsigned short *)rgb;

	for(i=0; i< whole; i++)
	{
		*rgb2 = ((*bgr2<<11)&0xf800) | ((*bgr2>>11)&0x001f) | ((*bgr2)&0x07e0);
		rgb2++;
		bgr2++;
	}
	return 0;
}
*/
//======================================================================================
using namespace android;
static uint32_t DEFAULT_DISPLAY_ID = ISurfaceComposer::eDisplayIdMain;
static SkColorType flinger2skia(PixelFormat f)
{
    switch (f) {
        case PIXEL_FORMAT_RGB_565:
            return kRGB_565_SkColorType;
        default:
            return kN32_SkColorType;
    }
}

static status_t vinfoToPixelFormat(const fb_var_screeninfo& vinfo,
        uint32_t* bytespp, uint32_t* f)
{

    switch (vinfo.bits_per_pixel) {
        case 16:
            *f = PIXEL_FORMAT_RGB_565;
            *bytespp = 2;
            break;
        case 24:
            *f = PIXEL_FORMAT_RGB_888;
            *bytespp = 3;
            break;
        case 32:
            // TODO: do better decoding of vinfo here
            *f = PIXEL_FORMAT_RGBX_8888;
            *bytespp = 4;
            break;
        default:
            return BAD_VALUE;
    }
    return NO_ERROR;
}

int main()
{
    ProcessState::self()->startThreadPool();
    struct timeval start,end;//记录开始时间
    gettimeofday(&start, NULL);

    int32_t displayId = DEFAULT_DISPLAY_ID;
    int fd = -1;

    const char* fn = "out.BMP";
    fd = open(fn, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if (fd == -1) {
        fprintf(stderr, "Error opening file: %s (%s)\n", fn, strerror(errno));
        return 1;
    }
    
    void const* mapbase = MAP_FAILED;
    ssize_t mapsize = -1;
    void const* base = 0;
    uint32_t w, s, h, f;
    size_t size = 0;

    ScreenshotClient screenshot;
    sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(displayId);
    if (display != NULL && screenshot.update(display, Rect(), false) == NO_ERROR) {
        base = screenshot.getPixels();
        w = screenshot.getWidth();
        h = screenshot.getHeight();
        s = screenshot.getStride();
        f = screenshot.getFormat();
//        size = screenshot.getSize();
    } else {
        fprintf(stderr, "Error: Data acquisition failure");
        return 1;
    }

    if (base) {
        if (0) {//保存为JPEG图片250ms+
            const SkImageInfo info = SkImageInfo::Make(w, h, flinger2skia(f),
                                                       kPremul_SkAlphaType);
            SkBitmap b;
            b.installPixels(info, const_cast<void*>(base), s*bytesPerPixel(f));
            SkDynamicMemoryWStream stream;
            SkImageEncoder::EncodeStream(&stream, b,
                    SkImageEncoder::kJPEG_Type, SkImageEncoder::kDefaultQuality);
            SkData* streamData = stream.copyToData();
            write(fd, streamData->data(), streamData->size());
            streamData->unref();
        } else {//保存为BMP图片80ms+
            BMPHEADER head;
            BMPINFO info;
            get_rgb888_header(w, h, &head, &info);
            write(fd, head.bfType, 14);
            write(fd, &info, sizeof(info));
            size_t Bpp = bytesPerPixel(f);//获得每个像素大小,目前为4字节，BGRA
            for (size_t y=0 ; y<h ; y++) {
                write(fd, base, w*Bpp);
                base = (void *)((char *)base + s*Bpp);
            }
        }
    }
    gettimeofday(&end, NULL);
    long timeuse =1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
    printf("time=%f\n",timeuse /1000.0); //显示整个运行花的时间ms
    close(fd);
    if (mapbase != MAP_FAILED) {
        munmap((void *)mapbase, mapsize);
    }

    return 0;
}
