#ifndef __CONFERENCE_H__
#define __CONFERENCE_H__

#include "jdksavdecc_world.h"
#include "host_controller_debug.h"
#include "jdksavdecc_aecp_aem.h"

#define CONFERENCE_TYPE 0xAB
#define DATAMAXLENGTH 64

//Э��ͷ������
#define CONFERENCE_COMMON_HEADER_LEN 4
//У�����ݳ���
#define CONFERENCE_CRC_LEN 1
//�ն���Ӧ����λ����
#define END_TO_HOST_DATA_IN_CMD_LEN 1
//������Ӧ����λ����
#define HOST_TO_END_DATA_IN_CMD_LEN 1

/*{@����ϵͳ�����ڸ����е�ƫ��@}*/
#define CONFERENCE_DATA_IN_CONTROLDATA_OFFSET 24
#define CONFERENCE_DATA_IN_CONTROLDATA_LEN 24

//�ն���ͨ��Ӧ����(��������λ)
#define END_TO_HOST_CMD_LEN (CONFERENCE_COMMON_HEADER_LEN +  CONFERENCE_CRC_LEN +\
	END_TO_HOST_DATA_IN_CMD_LEN)
//�ն�������Ӧ����(����������λ)
#define END_TO_HOST_CMD_SPECIAL_LEN (END_TO_HOST_CMD_LEN + END_TO_HOST_DATA_IN_CMD_LEN)

//������Ӧ���·�����ͷ������(�������ݳ��� (�����������ݳ���λ��ֵȷ��))
#define HOST_COMMON_TO_END_EXDATA_LEN (CONFERENCE_COMMON_HEADER_LEN + HOST_TO_END_DATA_IN_CMD_LEN +\
	CONFERENCE_CRC_LEN)
//��ͨ�ն���Ӧ���Ϸ������ͷ�����ȼ�У�����ݳ���(�������ݳ��� 1)
#define END_COMMON_TO_HOST_LEN (CONFERENCE_COMMON_HEADER_LEN  + CONFERENCE_CRC_LEN)
//�����ն���Ӧ���Ϸ������ͷ����У�����ݳ���(�������ݳ��� (�����������ݳ���λ��ֵȷ��))
#define END_COMMON_TO_HOST_SPECIAL_LEN (CONFERENCE_COMMON_HEADER_LEN + END_TO_HOST_DATA_IN_CMD_LEN +\
	CONFERENCE_CRC_LEN)


/*{@��������*/
#define HOST_TO_ENDSTATION_COMMAND_TYPE_QUERY_END 0x01//������ѯ�ն�
#define HOST_TO_ENDSTATION_COMMAND_TYPE_ALLOCATION 0x02//�ն�Ҫ������ַ
#define HOST_TO_ENDSTATION_COMMAND_TYPE_SET_END_STATUS 0x03//�����ն�״̬
#define HOST_TO_ENDSTATION_COMMAND_TYPE_REALLOCATION 0x04//���·����ַ
/*{@�ն˰�������*/
#define HOST_TO_ENDSTATION_COMMAND_TYPE_KEYPAD_ACTION 0x05//�ն˰�������
/*@�ն˰�������}*/
#define HOST_TO_ENDSTATION_COMMAND_TYPE_SET_ENDLIGHT 0x06//�����ն�ָʾ��
#define HOST_TO_ENDSTATION_COMMAND_TYPE_NEW_ALLOCATION 0x07//�����ն˷����ַ
#define HOST_TO_ENDSTATION_COMMAND_TYPE_END_ASSIGN 0x08 //�ն˱���
#define HOST_TO_ENDSTATION_COMMAND_TYPE_SET_END_LCD  0x09 //�����ն�LCD��ʾ��ʽ
#define HOST_TO_ENDSTATION_COMMAND_TYPE_COMMON_SEND_END_LCD 0x0a//�����ն�LCD��ʾ����,�����ǹ�������

#define HOST_TO_ENDSTATION_COMMAND_TYPE_SET_END_LED 0x0b//�����ն�LED��ʾ��ʽ
#define HOST_TO_ENDSTATION_COMMAND_TYPE_COMMON_SEND_END_LED 0x0c//�����ն�LED��ʾ����,�����ǹ�������
#define HOST_TO_ENDSTATION_COMMAND_TYPE_PRESIDENT_CONTROL  0x0d//��ϯ�����ƻ���
#define HOST_TO_ENDSTATION_COMMAND_TYPE_SEND_VOTE_RESULT 0x0e //���ͱ�����
#define HOST_TO_ENDSTATION_COMMAND_TYPE_TALKTIME_LEN 0x0f //����ʱ��
#define HOST_TO_ENDSTATION_COMMAND_TYPE_HOST_SEND_STATUS 0x10 //��������״̬
#define HOST_TO_ENDSTATION_COMMAND_TYPE_SEND_END_LCD_DISPLAY 0x11//�����ն�LCD��ʾ����
#define HOST_TO_ENDSTATION_COMMAND_TYPE_OPTITION_END 0x12 //�����ն�

#define HOST_TO_ENDSTATION_COMMAND_TYPE_SET_MIS_STATUS 0x13 //���û�Ͳ״̬

#define HOST_TO_ENDSTATION_COMMAND_TYPE_END_SPETHING 0x14 //�ն������¼�

#define HOST_TO_ENDSTATION_COMMAND_TYPE_CHECK_END_RESULT 0x16 //��ѯ�ն˱�����

#define HOST_TO_ENDSTATION_COMMAND_TYPE_TRANSIT_HOST_MSG 0x1e//ת����λ������Ϣ���ݵ���󳤶ȣ���ʱ��data_len��Ҫ����ʵ�ʵ����ȷ��
#define HOST_TO_ENDSTATION_COMMAND_TYPE_TRANSIT_END_MSG 0x1f //ת���ն˶���Ϣ
/*@}��������*/ 

/*@{�������λ�ĺ���*/
#define CONFERENCE_COMMAND_HIGH3BIT_BIT7_END_TO_HOST 1
#define CONFERENCE_COMMAND_HIGH3BIT_BIT7_HOST_TO_END 0
#define CONFERENCE_COMMAND_HIGH3BIT_BIT6_REPONSE 1
#define CONFERENCE_COMMAND_HIGH3BIT_BIT6_INITIATIVE_SEND 0
#define CONFERENCE_COMMAND_HIGH3BIT_BIT5_PRESIDENT_SEND 1
#define CONFERENCE_COMMAND_HIGH3BIT_BIT5_COMMON_SEND 0
//000(�����·�, ���������ı���,��ͨ����������ı���)
#define CONFERENCE_COMMAND_HIGH3BIT_MEANING_0 \
	(CONFERENCE_COMMAND_HIGH3BIT_BIT7_HOST_TO_END << 2) | \
	(CONFERENCE_COMMAND_HIGH3BIT_BIT6_INITIATIVE_SEND << 1) |\
	(CONFERENCE_COMMAND_HIGH3BIT_BIT5_COMMON_SEND << 0)
//001(�����·�,���������ı���,��ϯ��Ԫ�����ı���)	
#define CONFERENCE_COMMAND_HIGH3BIT_MEANING_1 \
		(CONFERENCE_COMMAND_HIGH3BIT_BIT7_HOST_TO_END << 2) | \
	(CONFERENCE_COMMAND_HIGH3BIT_BIT6_INITIATIVE_SEND << 1) |\
	(CONFERENCE_COMMAND_HIGH3BIT_BIT5_PRESIDENT_SEND << 0)
//010(�����·�,��Ӧ�ı���,��ͨ����������ı���)		
#define CONFERENCE_COMMAND_HIGH3BIT_MEANING_2 \
		(CONFERENCE_COMMAND_HIGH3BIT_BIT7_HOST_TO_END << 2) | \
	(CONFERENCE_COMMAND_HIGH3BIT_BIT6_REPONSE << 1) |\
	(CONFERENCE_COMMAND_HIGH3BIT_BIT5_COMMON_SEND << 0)
//011(�����·�,��Ӧ�ı���,��ϯ��Ԫ�����ı���)		
#define CONFERENCE_COMMAND_HIGH3BIT_MEANING_3 \
		(CONFERENCE_COMMAND_HIGH3BIT_BIT7_HOST_TO_END << 2) | \
	(CONFERENCE_COMMAND_HIGH3BIT_BIT6_REPONSE << 1)|\
	(CONFERENCE_COMMAND_HIGH3BIT_BIT5_PRESIDENT_SEND << 0)
//100(�ն��Ϸ�,���������ı���, ��ͨ����������ı���)		
#define CONFERENCE_COMMAND_HIGH3BIT_MEANING_4 \
		(CONFERENCE_COMMAND_HIGH3BIT_BIT7_END_TO_HOST << 2) | \
	(CONFERENCE_COMMAND_HIGH3BIT_BIT6_INITIATIVE_SEND << 1)|\
	(CONFERENCE_COMMAND_HIGH3BIT_BIT5_COMMON_SEND << 0)
//101(�ն��Ϸ�,���������ı���, ��ϯ��Ԫ�����ı���)		
#define CONFERENCE_COMMAND_HIGH3BIT_MEANING_5 \
		(CONFERENCE_COMMAND_HIGH3BIT_BIT7_END_TO_HOST << 2) | \
	(CONFERENCE_COMMAND_HIGH3BIT_BIT6_INITIATIVE_SEND << 1) |\
	(CONFERENCE_COMMAND_HIGH3BIT_BIT5_PRESIDENT_SEND << 0)
//110(�ն��Ϸ�,��Ӧ�ı���, ��ͨ����������ı���)		
#define CONFERENCE_COMMAND_HIGH3BIT_MEANING_6 \
		(CONFERENCE_COMMAND_HIGH3BIT_BIT7_END_TO_HOST << 2) | \
	(CONFERENCE_COMMAND_HIGH3BIT_BIT6_REPONSE << 1) |\
	(CONFERENCE_COMMAND_HIGH3BIT_BIT5_COMMON_SEND << 0)
//111(�ն��Ϸ�,��Ӧ�ı���, ��ϯ��Ԫ�����ı���)		
#define CONFERENCE_COMMAND_HIGH3BIT_MEANING_7 \
		(CONFERENCE_COMMAND_HIGH3BIT_BIT7_END_TO_HOST << 2) | \
	(CONFERENCE_COMMAND_HIGH3BIT_BIT6_REPONSE << 1) |\
	(CONFERENCE_COMMAND_HIGH3BIT_BIT5_PRESIDENT_SEND << 0)
/*@�������λ�ĺ���}*/


/*{@�����·����ݳ���*/
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_QUERY_END 0//������ѯ�ն�
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_ALLOCATION_STATUS1 0x02//�ն�Ҫ������ַ��Ӧ1
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_ALLOCATION_STATUS2 0x0//�ն�Ҫ������ַ��Ӧ2
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_REALLOCATION 0x0//���·����ַ
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_SET_END_STATUS 0x04//�����ն�״̬
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_SET_MIS_STATUS 0x01 //���û�Ͳ״̬
/*{@�ն˰�������*/
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_KEYPAD_COMMONE 0x01//��ͨ��Ӧ
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_KEYPAD_SPECIAL1 0x03//������Ӧ1
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_KEYPAD_SPECIAL2 0x06//������Ӧ2
/*@�ն˰�������}*/
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_SET_ENDLIGHT 0x02//�����ն�ָʾ��
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_NEW_ALLOCATION 0x00//�����ն˷����ַ
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_END_ASSIGN 0x00 //�ն˱���
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_SET_END_LCD  0x1 //�����ն�LCD��ʾ��ʽ
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_COMMON_SEND_END_LCD 0x05//�����ն�LCD��ʾ����,�����ǹ�������

#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_SET_END_LED 0x02//�����ն�LED��ʾ��ʽ
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_COMMON_SEND_END_LED 0x01//�����ն�LED��ʾ����,�����ǹ�������
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_PRESIDENT_CONTROL  0x01//��ϯ�����ƻ���
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_SEND_VOTE_RESULT 0x08 //���ͱ�����
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_TALKTIME_LEN 0x01 //����ʱ��
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_HOST_SEND_STATUS 0x07 //��������״̬
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_SEND_END_LCD_DISPLAY 0x02//�����ն�LCD��ʾ����
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_OPTITON_END 0x01 //�����ն�
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_END_SPETHING 0x00 //�ն������¼�
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_TRANSIT_HOST_MSG_MAX 120//ת����λ������Ϣ���ݵ���󳤶ȣ���ʱ��data_len��Ҫ����ʵ�ʵ����ȷ��
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_TRANSIT_END_MSG 0x00 //ת���ն˶���Ϣ
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_CHECK_END_RESULT 0x00 //��ѯ�ն˱�����
/*@}�����·����ݳ���*/

/*@{�ն˵�ַ*/
#define CONFERENCE_ENDSTATION_ADDRESS_APPLICATION (0x0 & 0xf) << 12//Ӧ�õ�ַ��ʮ��λ��־
#define CONFERENCE_ENDSTATION_ADDRESS_DEVICE (0x1 & 0xf) << 12//�豸��ַ��ʮ��λ��־

#define CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_ALL ( 0x40 & 0x7f ) //ȫ�㲥��7λ��־
#define CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_COMMON ( 0x41 & 0x7f ) //�㲥��ͨ�����7λ��־
#define CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_VIP ( 0x42 & 0x7f ) //�㲥VIP��7λ��־
#define CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_COMMON_PRESIDENT ( 0x44 & 0x7f ) //�㲥��ͨ��ϯ��7λ��־
#define CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_EXECUTE_PRESIDENT (0x48 & 0x7f ) //�㲥ִ����ϯ��7λ��־

//�㲥�Ƿ���Ҫ��Ӧ��ʱ�䳤��
#define CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_RESPOND ( (0x01 & 0x1) << 8 )
#define CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_NORESPOND ( (0x00 & 0x1) << 8 )
#define CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_WIND7F ( ( 0x00 & 0x3 ) << 6 )
#define CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_WINDFF ( ( 0x01 & 0x3 ) <<6 )
#define CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_WIND1FF ( ( 0x02 & 0x3 ) << 6 )
#define CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_WIND3FF ( (0x03 & 0x3 ) << 6 )
/*@�ն˵�ַ}*/

struct conference_end_address_euint16
{
	uint8_t vales[2];
};

// ����ϵͳЭ�鹫��ͷ����ʽ
struct conference_common_header
{
	uint8_t byte_guide;			// �����ֽ�
	uint8_t command_control;	// ��������
	uint16_t address;			// �ն˵�ַ
};

struct host_to_endstation
{
	struct conference_common_header cchdr;	// ����ͷ��
	uint8_t data_len;						// ���ݳ���
	uint8_t data[DATAMAXLENGTH];			// ������
	uint8_t crc;							// У����(ֵΪǰ�����ݵ����)
	uint8_t deal_backups[DATAMAXLENGTH + HOST_COMMON_TO_END_EXDATA_LEN];	// Э��ı���
};

struct endstation_to_host					
{
	struct conference_common_header cchdr;	// ����ͷ��
	uint8_t data;							// ������
	uint8_t crc;							// У����(ֵΪǰ�����ݵ����)
	uint8_t deal_backups[DATAMAXLENGTH + HOST_COMMON_TO_END_EXDATA_LEN];	// Э��ı���
};

struct endstation_to_host_special			// �ն����������ʽ
{
	struct conference_common_header cchdr;
	uint8_t data_len;
	uint8_t data[DATAMAXLENGTH];
	uint8_t crc;
	uint8_t deal_backups[DATAMAXLENGTH + HOST_COMMON_TO_END_EXDATA_LEN];	// Э��ı���
};

struct terminal_deal_frame // aecp data conference data frame
{
	struct jdksavdecc_aecpdu_aem aecpdu_aem_header;
	uint16_t payload_len;
	uint8_t payload[ (DATAMAXLENGTH + HOST_COMMON_TO_END_EXDATA_LEN)*2]; // Э�鱸��
};

#endif




