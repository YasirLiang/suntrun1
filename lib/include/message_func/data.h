#ifndef __DATA_H__
#define __DATA_H__

#include "jdksavdecc_world.h"
#include "func_proccess.h"

#define MAX_FUNC_LINK_ITEMS 46
#define MAX_PROCCESS_FUNC 46

#define TERMINAL_USE   (1<<0)
#define MENU_USE  (1<<1)
#define SYSTEM_USE   (1<<2)
#define COMPUTER_USE   (1<<3)

extern const proccess_func_items proccess_func_link_tables[MAX_FUNC_LINK_ITEMS];
extern const func_link_items func_link_tables[MAX_FUNC_LINK_ITEMS];

#endif

