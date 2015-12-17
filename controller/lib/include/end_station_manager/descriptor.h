#ifndef __DESCRIPTOR_H__
#define __DESCRIPTOR_H__

#include "jdksavdecc_world.h"
#include "jdksavdecc_util.h"

#define MAX_STREAM_NUM 8
#define MAX_DESC_COUNTS_NUM 20	// THE MAX NUMBER IS 108 IN AVDECC-LIB

struct out_and_in_descptor
{
	uint32_t connect_num;						// 流的连接数量
	uint64_t stream_id;							// 输入输出流的流ID
	uint16_t descriptor_type;
    	uint16_t descriptor_index;
	struct jdksavdecc_eui64 current_format;			
};



typedef struct out_and_in_stream_descptor		// 输入输出流描述符
{
	uint8_t num;								// 输入流或输出流的数量
	struct out_and_in_descptor desc[MAX_STREAM_NUM];
}o_a_i_str_desc;

typedef struct _entity_decriptor
{
	uint64_t entity_entity_id;
	uint16_t descriptor_type;
	uint16_t descriptor_index;
	uint16_t configurations_count;
    	uint16_t current_configuration;
}entity_decriptor;

typedef struct _configuration_descriptor
{
	uint16_t descriptor_type;
	uint16_t descriptor_index;
	uint16_t descriptor_counts_count;
    	uint16_t descriptor_counts_offset;
	uint16_t desc_type[MAX_DESC_COUNTS_NUM];
	uint16_t desc_count[MAX_DESC_COUNTS_NUM];
}configuration_descriptor;

struct endpoint_decriptor 						// 实体的描述符，可在后面添加其它的描述符类型
{
	uint64_t entity_id;							// 实体ID
	struct jdksavdecc_string entity_name;			// 实体名字
	struct jdksavdecc_string firmware_version;		// 实体的固件版本
	bool is_entity_desc_exist;
	bool is_configuration_desc_exist;
	bool is_input_stream_desc_exist;
	bool is_output_stream_desc_exist;
	entity_decriptor entity_desc;
	configuration_descriptor conf_desc;
	o_a_i_str_desc input_stream;
	o_a_i_str_desc output_stream;
};

typedef struct endpoint_decriptor_node
{
	struct endpoint_decriptor endpoint_desc;
	struct endpoint_decriptor_node *prior, *next;
}desc_dblist, *desc_pdblist;

void descriptor_looking_for_connections( desc_pdblist desc_head );

#endif
