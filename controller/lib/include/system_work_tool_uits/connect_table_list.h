/*connect_table_list.h
**data: 2015/11/3
**
**
*/


#ifndef __CONNECT_TABLE_LIST_H__
#define __CONNECT_TABLE_LIST_H__

#include "endstation_connection.h"

extern connect_tbl_pdblist connect_table_dblist_node_create( connect_tbl_pdblist* new_node );
extern void connect_table_double_list_init( connect_tbl_pdblist* guard_node );
extern void connect_table_double_list_insert_node( connect_tbl_pdblist new_node, connect_tbl_pdblist guard );
extern void connect_table_double_list_move_node_to_tail( connect_tbl_pdblist move_node, connect_tbl_pdblist guard );
extern int connect_table_double_list_length_get( connect_tbl_pdblist guard );
extern void connect_table_double_list_show_connected( connect_tbl_pdblist guard );
extern void connect_table_double_list_show_all( connect_tbl_pdblist guard );


#endif

