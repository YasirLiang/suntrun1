#include "descriptor.h"
#include "host_controller_debug.h"

void descriptor_looking_for_connections( desc_pdblist desc_head )
{
	assert( desc_head );
	desc_pdblist desc = desc_head->next;
	
	MSGINFO( "\n----------------------------------connections list------------------------------------\n");
	
	for( ; desc != desc_head; desc = desc->next )
	{
		// 一个实体中的所有输入流与所有输出流比较
		uint8_t stream_input_desc_count = desc->endpoint_desc.input_stream.num;
		int in_stream_index = 0;
		for( in_stream_index = 0; in_stream_index < stream_input_desc_count; in_stream_index++ )
		{
			if( desc->endpoint_desc.input_stream.desc[in_stream_index].connect_num == 0)// 当前实体的输入流连接数为0
			{
				continue;
			}

			// 一个实体中的一个输入流与所有实体中的所有输出流比较, 一个输出流可以连多个输入流，而输入流只能被一个输出流占用
			desc_pdblist out_desc = desc_head->next;
			for( ; out_desc != desc_head; out_desc = out_desc->next )
			{
				// 与一个实体中的所有输出流比较
				uint8_t stream_output_desc_count = out_desc->endpoint_desc.output_stream.num;
				int out_stream_index = 0;
				for( out_stream_index = 0; out_stream_index < stream_output_desc_count; out_stream_index++)
				{
					if( (out_desc->endpoint_desc.output_stream.desc[out_stream_index].connect_num > 0)\
						&& (out_desc->endpoint_desc.output_stream.desc[out_stream_index].stream_id \
						== desc->endpoint_desc.input_stream.desc[in_stream_index].stream_id ) )
					{
						MSGINFO( "[ 0x%016llx(%d) -> 0x%016llx(%d) Stream ID = 0x%016llx ]\n",\
							out_desc->endpoint_desc.entity_id,\
							out_desc->endpoint_desc.output_stream.desc[out_stream_index].descriptor_index,\
							desc->endpoint_desc.entity_id,\
							desc->endpoint_desc.input_stream.desc[in_stream_index].descriptor_index,\
							desc->endpoint_desc.input_stream.desc[in_stream_index].stream_id);
					}
				}
			}
		}
	}
}

