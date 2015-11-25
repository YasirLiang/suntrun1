/*send_interval.h
**date: 2015/11/25
**
**
*/

#ifndef __SEND_INTERVAL_H__
#define __SEND_INTERVAL_H__

#include "jdksavdecc_world.h"
#include "host_time.h"

/*{@系统发送数据的间隔时间(用于系统对响应数据的发送)*/
#define SEND_INTERVAL_TIME 100 // 100MS
/*@}*/

enum send_interval_enum
{
	INTERVAL_IDLE,
	INTERVAL_PRIMED,
	INTERVAL_WAIT
};

typedef struct _type_send_next_interval_time
{
	bool running;					// 运行
	bool elapsed;					// 超时标志
	uint32_t start_time;			// 开始时间
	uint32_t count_time;			// 超时时间
}tsend_next_interval_time;

typedef struct _type_send_interval_native
{
	enum send_interval_enum send_interval_flag;
	tsend_next_interval_time timeout;
}tsend_interval_native;

void send_interval_timer_state_avail( timetype timeout, tsend_interval_native* target );
void send_interval_timer_start( timetype timeout, tsend_interval_native* target );
void send_interval_timer_stop(  tsend_interval_native* target );
void send_interval_timer_update( timetype timeout, tsend_interval_native* target );
bool send_interval_timer_timeout( tsend_interval_native* target );
bool is_send_interval_timer_timeout( void );
void uart_resp_send_interval_timer_start( void );
void send_interval_init( void );
int set_send_interval_idle_state( void );
int set_send_interval_primed_state( void );
int set_send_interval_wait_state( void );
bool is_send_interval_idle_state( void );
bool is_send_interval_primed_state( void );
bool is_send_interval_wait_state( void );


#endif
