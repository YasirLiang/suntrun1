#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "jdksavdecc_adp.h"
#include "host_time.h"

enum entity_connect
{
	DISCONNECT,
	CONNECT
};

struct entity_timeout
{
	bool running;					// 运行
	bool elapsed;					// 超时标志
	uint32_t count_time;			// 超时时间
	uint32_t start_time;			// 开始时间
};

struct entity
{
	struct jdksavdecc_eui64 entity_model_id;
	uint8_t keep_value1;			// 保留值1，用于字节对齐
	bool connect_flag;				// 实体连接属性，
	struct entity_timeout time;		// 实体的超时信息
	uint32_t keep_value2;			// 保留值2，用于字节对齐
	uint32_t entity_index;			// 实体索引(这里由系统分配, 且从 0 开始)
	uint32_t available_index;
	uint64_t entity_id;				// 实体ID
	//struct jdksavdecc_adpdu adpdu;
};

typedef struct entities				// 实体节点，存在于系统中
{
	struct entity solid;
	struct entities *prior, *next;
}solid_dblist,*solid_pdblist;

#endif

