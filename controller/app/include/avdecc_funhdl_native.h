/*
* @file
* @brief avdecc funhdl native
* @ingroup avdecc funhdl native
* @cond
******************************************************************************
* Last updated for version 1.0.0
* Last updated on  2016-09-21
*
*                    Moltanisk Liang
*                    ---------------------------
*                    avb auto control system
*
* Copyright (C) Moltanisk Liang, GuangZhou Suntron. All rights reserved.
******************************************************************************
* @endcond
*/
#ifndef __AVDECC_FUNHDL_NATIVE_H__
#define __AVDECC_FUNHDL_NATIVE_H__

/*! handle command funtion thread create*/
extern int pthread_handle_cmd_func(pthread_t *pid,
                                          const proccess_func_items *p_items);
/*! proccess recieve data and a little funtion proccess thread create */
extern int pthread_proccess_recv_data_create(pthread_t *pid, void * pgm);

#endif /*__AVDECC_FUNHDL_NATIVE_H__*/

/*****************************************************************************
*NOTE01:
*pthread.h must included before including 'avdecc_funhdl_native.h', 
*because of pthread_t not declaring in 'avdecc_funhdl_native.h'.
*
*NOTE02:
*func_proccess.h must included before including 'avdecc_funhdl_native.h', 
*because of pthread_t not declaring in 'avdecc_funhdl_native.h'.
*/

