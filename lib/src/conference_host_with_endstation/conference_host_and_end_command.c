#include "conference_host_and_end_handle.h"

// ������ѯ�ն�
int host_to_endstation_command_type_query_end_excute()
{
	
}

// �ն�Ҫ������ַ
int host_to_endstation_command_type_allocation_excute()
{
	
}

// �����ն�״̬
int host_to_endstation_command_type_set_end_status_excute()
{
	
}

// ���·����ַ
int host_to_endstation_command_type_realloaction_excute()
{
	
}

// �ն˰�������
int host_to_endstation_command_type_keypad_action_excute()
{
	
}

// �����ն�ָʾ��
int host_to_endstation_command_type_set_endlight_excute()
{
	
}

// �����ն˷����ַ
int host_to_endstation_command_type_new_allocation_excute()
{
	
}

//�ն˱���
int host_to_endstation_command_type_end_assign_excute()
{
	
}

// �����ն�LCD��ʾ��ʽ
int host_to_endstation_command_type_set_end_lcd_excute()
{
	
}

// �����ն�LCD��ʾ����,�����ǹ�������
int host_to_endstation_command_type_common_send_end_lcd_excute()
{
	
}

// �����ն�LED��ʾ��ʽ
int host_to_endstation_command_type_set_end_led_excute()
{
	
}

// �����ն�LED��ʾ����,�����ǹ�������
int host_to_endstation_command_type_common_send_end_led_excute()
{
	
}

// ��ϯ�����ƻ���
int host_to_endstation_command_type_president_control_excute()
{
	
}

// ���ͱ�����
int host_to_endstation_command_type_send_vote_result_excute()
{
	
}

// ����ʱ��
int host_to_endstation_command_type_talktime_len_excute()
{
	
}

// ��������״̬
int host_to_endstation_command_type_host_send_status_excute()
{
	
}

// �����ն�LCD��ʾ����
int host_to_endstation_command_type_send_end_lcd_display_excute()
{
	
}

// �����ն�
int host_to_endstation_command_type_optition_end_excute()
{
	
}

// ���û�Ͳ״̬
int host_to_endstation_command_type_set_mis_status_excute()
{
	
}

// �ն������¼�
int host_to_endstation_command_type_end_spething_excute()
{
	
}

// ��ѯ�ն˱�����
int host_to_endstation_command_type_check_end_result_excute()
{
	
}

// ת����λ������Ϣ���ݵ���󳤶ȣ���ʱ��data_len��Ҫ����ʵ�ʵ����ȷ��
int host_to_endstation_command_type_transit_host_msg_excute()
{
	
}

// ת���ն˶���Ϣ
int host_to_endstation_command_type_transit_end_msg_excute()
{
	
}

// ���Դ������е��������ն˵�����
void handle_end_to_host_command_type( uint8_t order, const void* packet )
{
	if( order == HOST_TO_ENDSTATION_COMMAND_TYPE_TRANSIT_END_MSG )
	{
		struct endstation_to_host_special* p_packet = (struct endstation_to_host_special *)packet;

		host_to_endstation_command_type_transit_end_msg_excute();
	}
	else
	{
		struct endstation_to_host* nml_pacet = (struct endstation_to_host*)packet;
		switch( order )
		{
			case HOST_TO_ENDSTATION_COMMAND_TYPE_QUERY_END:
			{
				host_to_endstation_command_type_query_end_excute();
			}
			break;
			case HOST_TO_ENDSTATION_COMMAND_TYPE_ALLOCATION:
			{
				host_to_endstation_command_type_allocation_excute();
			}
			break;
			case HOST_TO_ENDSTATION_COMMAND_TYPE_SET_END_STATUS:
			{
				host_to_endstation_command_type_set_end_status_excute();
			}
			break;
			case HOST_TO_ENDSTATION_COMMAND_TYPE_REALLOCATION:
			{
				host_to_endstation_command_type_realloaction_excute();
			}
			break;
			case HOST_TO_ENDSTATION_COMMAND_TYPE_KEYPAD_ACTION:
			{
				host_to_endstation_command_type_keypad_action_excute();
			}
			break;
			case HOST_TO_ENDSTATION_COMMAND_TYPE_SET_ENDLIGHT:
			{
				host_to_endstation_command_type_set_endlight_excute();
			}
			break;
			case HOST_TO_ENDSTATION_COMMAND_TYPE_NEW_ALLOCATION:
			{
				host_to_endstation_command_type_new_allocation_excute();
			}
			break;
			case HOST_TO_ENDSTATION_COMMAND_TYPE_END_ASSIGN:
			{
				host_to_endstation_command_type_end_assign_excute();
			}
			break;
			case HOST_TO_ENDSTATION_COMMAND_TYPE_SET_END_LCD:
			{
				host_to_endstation_command_type_set_end_lcd_excute();
			}
			break;
			case HOST_TO_ENDSTATION_COMMAND_TYPE_COMMON_SEND_END_LCD:
			{
				host_to_endstation_command_type_common_send_end_lcd_excute();
			}
			break;
			case HOST_TO_ENDSTATION_COMMAND_TYPE_SET_END_LED:
			{
				host_to_endstation_command_type_set_end_led_excute();
			}
			break;	
			case HOST_TO_ENDSTATION_COMMAND_TYPE_COMMON_SEND_END_LED:
			{
				host_to_endstation_command_type_common_send_end_led_excute();
			}
			break;
			case HOST_TO_ENDSTATION_COMMAND_TYPE_PRESIDENT_CONTROL:
			{
				host_to_endstation_command_type_president_control_excute();
			}
			break;
			case HOST_TO_ENDSTATION_COMMAND_TYPE_SEND_VOTE_RESULT:
			{
				host_to_endstation_command_type_send_vote_result_excute();
			}
			break;
			case HOST_TO_ENDSTATION_COMMAND_TYPE_TALKTIME_LEN:
			{
				host_to_endstation_command_type_talktime_len_excute();
			}
			break;
			case HOST_TO_ENDSTATION_COMMAND_TYPE_HOST_SEND_STATUS:
			{
				host_to_endstation_command_type_host_send_status_excute();
			}
			break;
			case HOST_TO_ENDSTATION_COMMAND_TYPE_SEND_END_LCD_DISPLAY:
			{
				host_to_endstation_command_type_send_end_lcd_display_excute();
			}
			break;
			case HOST_TO_ENDSTATION_COMMAND_TYPE_OPTITION_END:
			{
				host_to_endstation_command_type_optition_end_excute();
			}
			break;
			case HOST_TO_ENDSTATION_COMMAND_TYPE_SET_MIS_STATUS:
			{
				host_to_endstation_command_type_set_mis_status_excute();
			}
			break;
			case HOST_TO_ENDSTATION_COMMAND_TYPE_END_SPETHING:
			{
				host_to_endstation_command_type_end_spething_excute();
			}
			break;
			case HOST_TO_ENDSTATION_COMMAND_TYPE_CHECK_END_RESULT:
			{
				host_to_endstation_command_type_check_end_result_excute();
			}
			break;
			case HOST_TO_ENDSTATION_COMMAND_TYPE_TRANSIT_HOST_MSG:
			{
				host_to_endstation_command_type_transit_host_msg_excute();
			}
			break;
			default:
				break;
		}
	}
}

void handle_end_to_host_normal_packet( const struct endstation_to_host* nml_pacet )
{
	uint8_t byte_guide = nml_pacet->cchdr.byte_guide;			// �����ֽ�	
	uint8_t cmd_type = nml_pacet->cchdr.command_control;		// ��������
	uint16_t address = nml_pacet->cchdr.address;				// Ӧ�õ�ַ

	if( byte_guide == CONFERENCE_TYPE )
	{
		handle_end_to_host_command_type( cmd_type, nml_pacet );
	}
}

void handle_end_to_host_special_packet(const struct endstation_to_host_special* spl_paceket )
{
	uint8_t byte_guide = spl_paceket->cchdr.byte_guide;			// �����ֽ�	
	uint8_t cmd_type = spl_paceket->cchdr.command_control;		// ��������
	uint16_t address = spl_paceket->cchdr.address;				// Ӧ�õ�ַ

	// ������ת���ն˶���Ϣ
	if( (byte_guide == CONFERENCE_TYPE) && (cmd_type == HOST_TO_ENDSTATION_COMMAND_TYPE_TRANSIT_END_MSG) )
	{
		handle_end_to_host_command_type( cmd_type, spl_paceket );
	}
}

int handle_end_to_host_packet_deal_func(int param, const struct endstation_to_host* normal_pacet, const struct endstation_to_host_special* special_paceket )
{
	int r = param;
	
	// ����������ʽ��Э������
	if( r == 1)
	{
		handle_end_to_host_normal_packet( normal_pacet );					
	}
	// ���������ʽ��Э������,��ʱ������������Ҫ�����⴦��
	else if( r == 0 )
	{
		handle_end_to_host_special_packet( special_paceket );						
	}
	else if( r == -1)
	{
		fprintf(stdout, "bad parameter:\terrorread conference frame data!\n" );
		return -1;
	}
}


