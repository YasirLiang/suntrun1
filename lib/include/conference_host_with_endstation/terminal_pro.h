#ifndef __TERMINAL_PRO_H__
#define __TERMINAL_PRO_H__

#include "jdksavdecc_world.h"
#include "conference.h"
#include "terminal_common.h"

#define ADDRESS_FILE "address.dat"	// �ն˵�ַ��Ϣ��ŵ��ļ�
//#define ADDRESS_FILE "profile/address.dat"	// �ն˵�ַ��Ϣ��ŵ��ļ�

#define MAX_LIMIT_SPK_NUM 4
#define MAX_LIMIT_APPLY_NUM 10

extern tmnl_pdblist dev_terminal_list_guard; // �ն������ͷ���

typedef enum
{
	RGST_IDLE=0,
	RGST_WAIT,
	RGST_QUERY,
}register_state;	// ����״̬

typedef struct _type_tmnl_register_pro
{
  register_state rgs_state;
  uint16_t tmn_total;
  uint16_t TmnRgsted;
}ttmnl_register_proccess;

enum
{
	REFUSE_APPLY=0,
	NEXT_APPLY,
	APPROVE_APPLY
};

typedef struct _tsystem_discuccess
{
	ttmnl_discuss_mode edis_mode;	// ����ģʽ
	uint8_t    limit_num;     			//���޵ķ���������
	uint8_t    speak_limit_num;		//���޵ķ�������
	uint8_t    currect_first_index;		//���������ȷ��Ե�����
	uint8_t    apply_limit; 			// ���뷢�Ե���������
	uint8_t    apply_num;			// ��������
	uint16_t   speak_addr_list[MAX_LIMIT_SPK_NUM];	// ���Ե�ַ��
	uint16_t   apply_addr_list[MAX_LIMIT_APPLY_NUM];	 // �����ַ��
}tsys_discuss_pro;

typedef enum
{
	NO_VOTE=0,
	VOTE_SET,
	VOTE_SET_OVER,
}evote_flag;

typedef struct _type_vote_proccess
{
  evote_flag flag;
  uint16_t index;
}tvote_pro;

typedef struct
{
	bool is_int;
	uint16_t 	chmaddr;
}tchairman_control_in;

int init_terminal_address_list_from_file( void );
void init_terminal_address_list( void );
inline void init_terminal_allot_address( void );
inline void init_terminal_device_double_list();
void 	test_interface_terminal_address_list_write_file( FILE** fd );
void print_out_terminal_addr_infomation( terminal_address_list* p, int num);
void init_terminal_proccess_system( void );
bool terminal_register( uint16_t address, uint8_t dev_type, tmnl_pdblist p_tmnl_station );
uint16_t find_new_apply_addr( terminal_address_list_pro* p_gallot, terminal_address_list* p_gaddr_list, uint16_t* new_index);
/*{@*/
tmnl_pdblist terminal_system_dblist_except_free( void );
void terminal_open_addr_file_wt_wb( void );
/*@}*/

/*{@*/
int terminal_func_allot_address( uint16_t cmd, void *data, uint32_t data_len );
int terminal_func_key_action( uint16_t cmd, void *data, uint32_t data_len );
int terminal_func_chairman_control( uint16_t cmd, void *data, uint32_t data_len );
int terminal_func_send_main_state( uint16_t cmd, void *data, uint32_t data_len );
int terminal_func_cmd_event( uint16_t cmd, void *data, uint32_t data_len );

/*@}*/

#endif

