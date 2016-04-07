/*
*file:log.h
*build date:2016-4-6
*
*/

/**
//����:��Ϣ�ɼ�ϵͳ���ͷ�ļ�
//
//
*/

#ifndef __LOG_H__
#define __LOG_H__

#include "jdksavdecc_world.h"

#define LOG_MSG_BUF_SIZE 512
#define LOG_BUF_COUNT 32

typedef struct _type_log_data
{
	int32_t level;
	char msg[LOG_MSG_BUF_SIZE];
	int32_t time_stamp_ms;
}log_data;

typedef struct _type_log_base tstrlog_base;
struct _type_log_base
{
	int32_t log_level; // The base log level for messages to be logged
	uint32_t read_index;
        uint32_t write_index;
        int32_t missed_log_event_cnt; // The number of missed log that exceeds the log buffer count.
        log_data log_buf[LOG_BUF_COUNT];
        void (*callback_func)(void *, int32_t, const char *, int32_t);
	void *user_obj;

	void (*log_init)( tstrlog_base * );// ��ʼ��log base �ṹ��
	void (*set_log_level)( int32_t , tstrlog_base * );// ����log log_level�ĺ���
	void (*post_log_msg)( tstrlog_base *, int32_t , const char *, ... );// ������Ҫlog��buf
	void (*post_log_event)(void);// ʹ�ܲɼ���Ϣ��������������
	void (*set_log_callback)(void (*)(void *, int32_t, const char *, int32_t), void *, tstrlog_base * );// post event ��Ļص�����
	uint32_t (*missed_log_event_count)( tstrlog_base * );// ����û�м�¼����Ϣ����Ŀ
};

void log_init( tstrlog_base *base_log );
void set_log_level( int32_t new_log_level, tstrlog_base *base_log );
void post_log_msg( tstrlog_base *base_log, int32_t level, const char *fmt, ... );
void set_log_callback(void (*new_log_callback) (void *, int32_t, const char *, int32_t), void *p, tstrlog_base *base_log );
uint32_t missed_log_event_count( tstrlog_base *base_log );


#endif
