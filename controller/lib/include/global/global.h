/**
*File:global.h
*Date:2016/5/17
*
*/

/********************************/ 
// 建立的全局信息管理文件
// 包括软件的版本、各个模块
// 名字
/********************************/

#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#define central_control_unit_max_num 3 			// 中央传输单元的个数
#define central_control_unit_name "AVB 4in/4out"	// 中央控制单元名字定义
#define central_control_unit_transmit_name "DCS8000"	// 中央控制单元名字定义
#define central_control_transmit_uint_output 0		// 中央控制传输单元输出端口定义

//#define conference_uint_name "DCS6000-UNITE"	// 会议单元模块名字定义
#define conference_uint_name "AVB_R 4in/4out"		// chang in time 20160624
#define conference_uint_recieve_uint_input 0		// 会议接收单元输入端口定义
#define conference_uint_transmit_uint_ouput 0		// 会议传输单元输出端口

#define software_version "DCS6000_HC_V1.0.0" 		// 软件版本号

#endif
