#include "host_time.h"

uint32_t get_current_time( void )
{
 	struct timespec tp;
        timetype time;

	//获取指定类型时钟的精确度,CLOCK_MONOTONIC:从系统启动这一刻开始计时，即使系统时间被用户改变，也不受影响。系统休眠时不会计时。受adjtime和NTP影响。
        if ( clock_getres( CLOCK_MONOTONIC, &tp ) != 0 ) 
        {
            DEBUG_INFO("Timer not supported in asios_Clock_monotonic(), asios.c");
        }

	// clock_gettime和clock_settime取得和设置clk_id指定的时间。
    	clock_gettime( CLOCK_MONOTONIC, &tp );
	//ms
    	time = (timetype)(tp.tv_sec * 1000) + (timetype)(tp.tv_nsec/1000000);
        
        return time;
}

