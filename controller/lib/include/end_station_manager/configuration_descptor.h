#ifndef __CONFIGURATION_DESCPTOR_H__
#define __CONFIGURATION_DESCPTOR_H__

#include "jdksavdecc_world.h"
#include "jdksavdecc_util.h"
#include "descriptor.h"

void store_config_desc( const uint8_t *frame, int pos, size_t frame_len, desc_pdblist  desc_info_node );
void desc_count_list_node_init( desc_pdblist  desc_node, const uint8_t *frame, size_t pos, uint16_t descriptor_counts_count, uint16_t descriptor_counts_offset );
void desc_type_list_node_init( desc_pdblist  desc_node, const uint8_t *frame, size_t pos, uint16_t descriptor_counts_count, uint16_t descriptor_counts_offset );


#endif
