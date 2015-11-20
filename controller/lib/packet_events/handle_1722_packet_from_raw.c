#include "handle_1722_packet_from_raw.h"
#include "util.h"
#include "adp_controller_machine.h"
#include "end_station_prcs.h"

int send_controller_avail_response(const uint8_t *frame, size_t frame_len, const uint8_t dst_mac[6], const uint8_t src_mac[6] )
{
        int send_frame_returned = 0;
        size_t pos = 0;
	uint8_t ether_type[2] = {0x22, 0xf0};
		
        uint8_t * tx_frame = (uint8_t*)malloc( frame_len + ETHER_HDR_SIZE );
        if ( tx_frame == NULL )
        {
            	DEBUG_INFO("LOGGING_LEVEL_ERROR: unable to allocate response frame");
            	return -1;
        }
        memcpy( tx_frame, dst_mac, DEST_MAC_SIZE );
	memcpy( tx_frame + DEST_MAC_SIZE, src_mac, SRC_MAC_SIZE);
	memcpy( tx_frame + DEST_MAC_SIZE + SRC_MAC_SIZE, ether_type, ETHER_PROTOCOL_SIZE);
        memcpy( tx_frame + ETHER_HDR_SIZE, frame, frame_len );
        jdksavdecc_common_control_header_set_control_data( JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_RESPONSE, tx_frame, pos + ETHER_HDR_SIZE );

        send_frame_returned = raw_send(&net, dst_mac, tx_frame, frame_len + ETHER_HDR_SIZE );  
        if(send_frame_returned < 0)
        {
            DEBUG_INFO("LOGGING_LEVEL_ERROR: netif_send_frame error");
            assert(send_frame_returned >= 0);
        }
        
        release_heap_space( &tx_frame );
		
        return 0;
}

int rx_raw_packet_event( const uint8_t dst_mac[6], const uint8_t src_mac[6], bool *is_notification_id_valid, solid_pdblist list_hdr, const uint8_t *frame, size_t frame_len, int *status, uint16_t operation_id, bool is_operation_id_valid )
{
	uint8_t subtype = jdksavdecc_common_control_header_get_subtype( frame, ZERO_OFFSET_IN_PAYLOAD );
	solid_pdblist guard = list_hdr;

	switch( subtype )
	{
		case JDKSAVDECC_SUBTYPE_ADP:
		{
			solid_pdblist end_station = NULL;
			uint64_t current_endpoint_id = 0;	// ��ǰ�������ݰ����ն�ID
			uint64_t end_entity_id = 0; 			// �����������е��ն�ID
			bool found_adp_in_end_station = false;

			struct jdksavdecc_adpdu adpdu;
			memset(&adpdu,0,sizeof(adpdu) );
			jdksavdecc_adpdu_read( &adpdu, frame, 0, frame_len );
			convert_eui64_to_uint64( adpdu.header.entity_id.value, &current_endpoint_id );
			int list_len = get_endpoint_dblist_length( guard );

			if ((adpdu.entity_capabilities & JDKSAVDECC_ADP_ENTITY_CAPABILITY_GENERAL_CONTROLLER_IGNORE) ||
					(adpdu.entity_capabilities & JDKSAVDECC_ADP_ENTITY_CAPABILITY_ENTITY_NOT_READY))
			{
				break;
			}

			if( NULL != guard )
			{
				// �鿴ϵͳ�Ƿ���ڴ�adp
				int i = 0;
				for( i = 0; i < list_len; i++)
				{
					if(get_entity_id_endpoint_dblist( guard, i, &end_entity_id ) == 1 )
					{
						if( end_entity_id == current_endpoint_id )	// �ն��Ѵ�����ϵͳ
						{
							found_adp_in_end_station = true;
							end_station = search_endtity_node_endpoint_dblist(guard, end_entity_id );
							break;
						}
					}
				}
			}
			else
			{
				DEBUG_INFO("invalid system list!");
				break;
			}

			if(jdksavdecc_eui64_convert_to_uint64(&adpdu.header.entity_id) != 0)
			{
				if(!found_adp_in_end_station) // �ն˲���ϵͳ�У�����ϵͳ
				{          
					end_station = adp_proccess_new_entity( guard, &end_station, &adpdu, list_len );
					if( end_station->solid.connect_flag == CONNECT )
						DEBUG_ONINFO("[ END_STATION CONNECT: 0x%llx ] ", end_station->solid.entity_id );

				}
				else// �ն���ϵͳ��
				{
					// �ն�����ϵͳ�У�ֻ��available_index��ͬ
					if ((adpdu.available_index < (get_available_index_endpoint_dblist_node( end_station ))) ||\
							( jdksavdecc_eui64_convert_to_uint64(&adpdu.entity_model_id) != jdksavdecc_eui64_convert_to_uint64(get_entity_model_id_endpoint_dblist_node( end_station ))))
					{
						DEBUG_INFO("LOGGING_LEVEL_DEBUG:Re-enumerating end station with entity_id %llx", end_entity_id);
						// ����ö��
					}

					// ����adp���ݰ�Ϊ���µ�����
					update_entity_adpdu_endpoint_dblist( &adpdu, end_station );

					// �����ն˵�����״̬������ն˵Ŀ���ʱ�䣬����ʱʱ�䡣��Ϊ�ն˴���ϵͳ��ʱ����Զ�����µ�
					// ԭ��:�ն˵��ߵ���ID�Ľṹ��ڵ㲻�ᱻɾ��������ʱֻ�����������״̬������״̬
					if( end_station->solid.connect_flag == DISCONNECT )	
					{
						end_station->solid.connect_flag = CONNECT;
						adp_entity_state_avail( guard, end_station, &adpdu.header );
						DEBUG_ONINFO("[ END_STATION AFLESH CONNECT: 0x%llx ] ", end_station->solid.entity_id );
					}
					// �����ն˵Ŀ���ʱ�䣬����ʱʱ�䡣��Ϊ�ն˴���ϵͳ��ʱ����Զ�����µ�
					else
					{
						adp_entity_state_avail( guard, end_station, &adpdu.header );
					}
				}
			}
			else if (adpdu.header.message_type != JDKSAVDECC_ADP_MESSAGE_TYPE_ENTITY_DISCOVER)
			{
				DEBUG_INFO("LOGGING_LEVEL_ERROR:Invalid ADP packet with an entity ID of 0.");
			}

			//*status = 0;
		}
		break;

		case JDKSAVDECC_SUBTYPE_ACMP:
		{
			bool found_acmp_in_end_station = false;
			struct jdksavdecc_eui64 entity_entity_id;
			uint64_t entity_id = 0;
			bzero( &entity_entity_id, sizeof(entity_entity_id) );

			uint32_t msg_type = jdksavdecc_common_control_header_get_control_data( frame, 0 );
			int list_len = get_endpoint_dblist_length( guard );


			if((msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_STATE_RESPONSE) || 
			(msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_CONNECTION_RESPONSE))
			{
				entity_entity_id = jdksavdecc_acmpdu_get_talker_entity_id( frame, ZERO_OFFSET_IN_PAYLOAD );
			}
			else if((msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_CONNECT_RX_RESPONSE) ||
			(msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_DISCONNECT_RX_RESPONSE) ||
			(msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_RX_STATE_RESPONSE))
			{
				entity_entity_id = jdksavdecc_acmpdu_get_listener_entity_id( frame, ZERO_OFFSET_IN_PAYLOAD );
			}

			// listerner ��ϵͳ��
			if( NULL != guard && (convert_eui64_to_uint64_return(entity_entity_id.value) != 0) )
			{
				int i = 0;
				for( i = 0; i < list_len; i++)
				{
					if(get_entity_id_endpoint_dblist( guard, i, &entity_id ) == 1 )
					{
						if( entity_id == convert_eui64_to_uint64_return(entity_entity_id.value) )	// �ն��Ѵ�����ϵͳ
						{
							found_acmp_in_end_station = true;
							break;
						}
					}
				}
			}

			if( found_acmp_in_end_station )
			{		
				//���յ��Ĵ���ͬ�����ӹ�������
				proc_rcvd_acmp_resp( msg_type, frame, frame_len, status );
				*is_notification_id_valid = true;
			}
			else
			{
				//DEBUG_INFO("LOGGING_LEVEL_DEBUG:Wait for correct ACMP response packet.");
				*status = AVDECC_LIB_STATUS_INVALID;
			}
		}
		break;
			
		case JDKSAVDECC_SUBTYPE_AECP:
		{
			uint32_t msg_type = jdksavdecc_common_control_header_get_control_data( frame, ZERO_OFFSET_IN_PAYLOAD );
			struct jdksavdecc_eui64 entity_entity_id = jdksavdecc_common_control_header_get_stream_id( frame, ZERO_OFFSET_IN_PAYLOAD );
			uint16_t cmd_type = jdksavdecc_aecpdu_aem_get_command_type( frame, ZERO_OFFSET_IN_PAYLOAD );
			int list_len = get_endpoint_dblist_length( guard );
			bool found_aecp_in_end_station = false;
			uint64_t entity_id;

			if( NULL != guard )
			{
				int i = 0;
				for( i = 0; i < list_len; i++)
				{
					if(get_entity_id_endpoint_dblist( guard, i, &entity_id ) == 1 )
					{
						if( entity_id == convert_eui64_to_uint64_return(entity_entity_id.value) )	// �ն��Ѵ�����ϵͳ
						{
							found_aecp_in_end_station = true;
							break;
						}
					}
				}
			}
			else
			{
				DEBUG_INFO("invalid system list!");
				break;
			}
			
			// �������ն˵�Э���,��1722��δ��ʹ��,��ʱ�������ͼ�Ϊ����ϵͳЭ�����ݸ��صĳ��ȣ��������ݵ�Э�����ݳ���
			if( ( msg_type == JDKSAVDECC_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_COMMAND )  )
			{
				if( found_aecp_in_end_station )
					proc_aecp_message_type_vendor_unique_command_conference( frame, frame_len, status );
			}
			else
			{
				/* check dest mac address is ours */
                    		if ( is_mac_equal( dst_mac, net.m_my_mac))
                    		{
                        		if (msg_type == JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_COMMAND &&
                             			cmd_type == JDKSAVDECC_AEM_COMMAND_CONTROLLER_AVAILABLE)//����ֻ�����������������
                        		{
                            			send_controller_avail_response( frame, frame_len, src_mac, dst_mac );
                        		}
                        		else
                        		{
						if ( found_aecp_in_end_station )
						{
						    switch(msg_type)
						    {
						        
						        case JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_RESPONSE:// ����������AEM_RESPONSE
						        {
						        	proc_rcvd_aem_resp( frame, frame_len, status );
								*is_notification_id_valid = true;
						            	break;
						        }
						        case JDKSAVDECC_AECP_MESSAGE_TYPE_ADDRESS_ACCESS_RESPONSE:// ����������ADDRESS_ACCESS_RESPONSE
						        {
								proc_rcvd_aecp_aa_resp( frame, frame_len, status);
								*is_notification_id_valid = true;
								break;
						        }
						    }
						}
						else
						{
							*status = AVDECC_LIB_STATUS_INVALID;
						}
					}
                    		}
			}
		}
		break;
		
		default:
		break;
	}

	return 0;
}

