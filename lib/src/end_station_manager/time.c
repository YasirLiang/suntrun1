#include "host_time.h"

uint32_t get_current_time( void )
{
 	struct timespec tp;
        timetype time;

	//��ȡָ������ʱ�ӵľ�ȷ��,CLOCK_MONOTONIC:��ϵͳ������һ�̿�ʼ��ʱ����ʹϵͳʱ�䱻�û��ı䣬Ҳ����Ӱ�졣ϵͳ����ʱ�����ʱ����adjtime��NTPӰ�졣
        if ( clock_getres( CLOCK_MONOTONIC, &tp ) != 0 ) 
        {
            DEBUG_INFO("Timer not supported in asios_Clock_monotonic(), asios.c");
        }

	// clock_gettime��clock_settimeȡ�ú�����clk_idָ����ʱ�䡣
    	clock_gettime( CLOCK_MONOTONIC, &tp );
	//ms
    	time = (timetype)(tp.tv_sec * 1000) + (timetype)(tp.tv_nsec/1000000);
        
        return time;
}

