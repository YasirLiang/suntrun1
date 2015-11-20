#include "configuration_descptor.h"
#include "host_controller_debug.h"
#include "jdksavdecc_aem_descriptor.h"

void desc_type_list_node_init( desc_pdblist  desc_node, const uint8_t *frame, size_t pos, uint16_t descriptor_counts_count, uint16_t descriptor_counts_offset )
{
	uint16_t offset = 0;
	uint32_t i = 0;

	for( i = 0; i < descriptor_counts_count; i++)
	{
		uint16_t desc_type = jdksavdecc_uint16_get(frame, descriptor_counts_offset + pos + offset);
		desc_node->endpoint_desc.conf_desc.desc_type[i] = desc_type;
		offset += 0x4;
	}
}

void desc_count_list_node_init( desc_pdblist  desc_node, const uint8_t *frame, size_t pos, uint16_t descriptor_counts_count, uint16_t descriptor_counts_offset )
{
	uint16_t offset = 0x2;
	uint32_t i = 0;

	for( i = 0; i < descriptor_counts_count; i++)
	{
		desc_node->endpoint_desc.conf_desc.desc_count[i] = jdksavdecc_uint16_get(frame, descriptor_counts_offset + pos + offset);
		offset += 0x4;
	}
}


void store_config_desc( const uint8_t *frame, int pos, size_t frame_len, desc_pdblist  desc_info_node )
{
	struct jdksavdecc_descriptor_configuration config_desc;
	ssize_t ret = jdksavdecc_descriptor_configuration_read(&config_desc, frame, pos, frame_len);

	if(ret < 0)
	{
		DEBUG_INFO("LOGGING_LEVEL_ERROR: 0x%016llx, config_desc_read error", desc_info_node->endpoint_desc.entity_id);
		assert(ret >= 0);
	}
	
	assert( desc_info_node && config_desc.descriptor_counts_count <= MAX_DESC_COUNTS_NUM && config_desc.descriptor_counts_offset == 74 );
	desc_info_node->endpoint_desc.conf_desc.descriptor_counts_count = config_desc.descriptor_counts_count;
	desc_type_list_node_init( desc_info_node, frame, pos, config_desc.descriptor_counts_count, config_desc.descriptor_counts_offset );
	desc_count_list_node_init(desc_info_node, frame, pos, config_desc.descriptor_counts_count, config_desc.descriptor_counts_offset  );
	desc_info_node->endpoint_desc.is_configuration_desc_exist = true;
}

