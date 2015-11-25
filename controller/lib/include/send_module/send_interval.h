/*send_interval.h
**date: 2015/11/25
**
**
*/

#ifndef __SEND_INTERVAL_H__
#define __SEND_INTERVAL_H__

#include "jdksavdecc_world.h"
#include "host_time.h"

/*{@ϵͳ�������ݵļ��ʱ��(����ϵͳ����Ӧ���ݵķ���)*/
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
	bool running;					// ����
	bool elapsed;					// ��ʱ��־
	uint32_t start_time;			// ��ʼʱ��
	uint32_t count_time;			// ��ʱʱ��
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
