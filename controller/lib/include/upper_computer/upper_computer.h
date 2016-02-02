#ifndef __UPPER_COMPUTER_H__
#define __UPPER_COMPUTER_H__

#include "jdksavdecc_world.h"

#define DATA_PAYLOAD_LEN_MAX 256
#define OTHER_DATA_LENGHT 6
#define UPPER_PAYLOAD_DATA_MAX_LEN ( DATA_PAYLOAD_LEN_MAX + OTHER_DATA_LENGHT)
#define UPPER_COMPUTER_DATA_LOADER 0xac

#define UPPER_RECV_BUF_MAX 2048

#define HOST_UPPER_COMPUTER_COMMON_HEAD_LENGTH 5
#define CMPT_HEAD_OFFSET 0
#define CMPT_PROTOCOL_OFFSET 1
#define CMPT_CMD_OFFSET 2
#define CMPT_DATA_LEN_OFFSET 3
#define CMPT_DATA_OFFSET 5
#define CMPT_COMMON_LEN 5

/*{@��������λ��Э��ľ������� 27 ��@}*/
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_CONFERENCE_DISCUSSION_PARAMETER 0x01	// ���ֲ���			 
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_MISCROPHONE_SWITCH 0x02 				// ��˷翪��
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_MISCROPHONE_STATUS 0x03 					// ��˷�״̬
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_SELECT_PROPOSER 0x05 					// ѡ��������
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_EXAMINE_APPLICATION 0x06 				// ��������
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_CONFERENCE_PERMISSION 0x07 				// ����Ȩ��
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_SENDDOWN_MESSAGE 0x08 					// �·�����Ϣ
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_TABLE_TABLET_STANDS_MANAGER 0x09 		// ���ƹ���
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_BEGIN_SIGN 0x0a 							// ��ʼǩ��
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_SIGN_SITUATION 0x0b 						// ǩ�����
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_END_OF_SIGN 0x0c 						// ǩ������
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_ENDSTATION_ALLOCATION_APPLICATION_ADDRESS 0x0e	// �ն˷���Ӧ�õ�ַ 	
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_ENDSTATION_REGISTER_STATUS 0x0f 			// �ն˱������
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_CURRENT_VIDICON 0x10 					// ��ǰ�����
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_ENDSTATION_ADDRESS_UNDETERMINED_ALLOCATION 0x11	// ����λ�ն�Ӧ�õ�ַ	 
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_VIDICON_CONTROL 0x12 					// ���������
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_VIDICON_PRERATION_SET 0x13 				// �����Ԥ�ò���
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_VIDICON_LOCK 0x14 						// ���������
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_VIDICON_OUTPUT 0x15 						// ��������
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_BEGIN_VOTE 0x17 							// ��ʼ���
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_PAUSE_VOTE 0x18 							// �ݶ����
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_REGAIN_VOTE 0x19 						// �ָ����
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_END_VOTE 0x20							// �������
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_RESULT_VOTE 0x21 						// ������
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_TRANSMIT_TO_ENDSTATION 0x22 				// ת�����ն�
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_REPORT_ENDSTATION_MESSAGE 0x23 			// �ϱ��ն˶���Ϣ
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_HIGH_DEFINITION_SWITCH_SET 0x24  		// ���ø�������ͷ�л�����
/*{@��������λ��Э��ľ�������@}*/

#define HOST_UPPER_COMPUTER_COMMON_LEN 5


struct host_upper_cmpt_common	// ��������λ��Э�鹫��ͷ
{
	uint8_t state_loader;			// �����ֽڣ��̶�����Ϊ0xAC
	uint8_t deal_type;				// ��������
	uint8_t command;				// ����	
	uint16_t data_len;				// ���ݳ��ȸ��ֽڸ����е�λ
};

struct host_upper_cmpt
{
	uint8_t deal_crc;
	struct host_upper_cmpt_common common_header;
	uint8_t data_payload[DATA_PAYLOAD_LEN_MAX];
};

typedef struct _type_host_upper_cmpt_msg
{
	uint8_t state_loader;			// �����ֽڣ��̶�����Ϊ0xAC
	uint8_t deal_type;				// ��������
	uint8_t command;				// ����	
	uint16_t data_len;				// ���ݳ��ȸ��ֽڸ����е�λ
	uint8_t data_payload[DATA_PAYLOAD_LEN_MAX];
}__attribute__((packed)) thost_upper_cmpt_msg;

struct host_upper_cmpt_frame		// ����udp���ݵ���Ϣ�ṹ��
{
	uint8_t dest_address[32];	 	// Ŀ��ip��ַ
	int dest_port;					// Ŀ�Ķ˿ں�
	int payload_len;
	uint8_t payload[UPPER_RECV_BUF_MAX ]; //���ո���
};

extern bool  is_upper_udp_client_connect;

#endif




