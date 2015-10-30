#ifndef __TERMINAL_PRO_H__
#define __TERMINAL_PRO_H__

#include "jdksavdecc_world.h"
#include "conference.h"
#include "terminal_common.h"

#define ADDRESS_FILE "address.dat"	// 终端地址信息存放的文件
//#define ADDRESS_FILE "profile/address.dat"	// 终端地址信息存放的文件

extern tmnl_pdblist dev_terminal_list_guard; // 终端链表表头结点

enum
{
  REFUSE_APPLY=0,
  NEXT_APPLY,
  APPROVE_APPLY
};

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

