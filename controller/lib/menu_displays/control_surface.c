
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h> 
#include <unistd.h> 
#include <time.h>

#include "lcd192x64.h"
#include "menu_f.h"

void *thread_control_surface(void *arg)
{
	int ret=0;
	int fd;  
  unsigned char key_val; 
	int key_down_flag=0;	
	struct timeval turn_tv0,turn_tv1,click_tv0,click_tv1;
	double interval=0;
	
	ret = lcd192x64_init();
	if (ret)
	{
	  printf("lcd192x64_init fail\n");
	  exit(1);
	}

	MenuInit();
 
  fd = open("/dev/buttons",O_RDWR);  
  if (fd < 0)  
  {  
		printf("open(/dev/buttons) failed.\n"); 
  }  
	memset((void *)&turn_tv0,0,sizeof(turn_tv0));
	while(1)
	{
		read(fd,&key_val,1);  
		switch(key_val&0x0F)
		{
			case 1://旋转
				gettimeofday(&turn_tv1,0);
				interval=(turn_tv1.tv_sec-turn_tv0.tv_sec)*1000+(turn_tv1.tv_usec-turn_tv0.tv_usec)/1000;
				if((interval>100)||(interval<0))
				{
					memcpy((void *)&turn_tv0,(void *)&turn_tv1,sizeof(turn_tv0));
					if(key_val&0x80)
					{
						MenuScroll(gsnCurMGrp,-1);
						printf("knob turn left!\n");  
					}
					else
					{
						MenuScroll(gsnCurMGrp,1);
						printf("knob turn right!\n");  
					}
				}
				break;
			case 2://按键
				if(!(key_val&0x10))//按下
				{
					if(!key_down_flag)
					{
						gettimeofday(&click_tv0,0);
						key_down_flag=1;
					}
				}
				else
				{
					gettimeofday(&click_tv1,0);
					interval=(click_tv1.tv_sec-click_tv0.tv_sec)*1000+(click_tv1.tv_usec-click_tv0.tv_usec)/1000;
					if(key_down_flag&&(interval>100))
					{
						ItemSelected(gsnCurMGrp);
						key_down_flag=0;
						printf("key click!\n");  
					}
				}
				break;
		}
	}
}

