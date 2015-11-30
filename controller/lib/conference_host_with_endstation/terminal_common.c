#include "terminal_common.h"
#include "linked_list_unit.h"
#include "host_controller_debug.h"
#include "conference_host_to_end.h"
#include "system_packet_tx.h"
#include "inflight.h"
#include "conference_end_to_host.h"
#include "aecp_controller_machine.h"
#include "file_util.h"
#include "linked_list_unit.h"
#include "terminal_pro.h"

extern tmnl_pdblist dev_terminal_list_guard; // 终端链表表头结点

/*****************************************************************
*Writer:YasirLiang
*Date:2015/10/21
*Name:terminal_address_list_write_file
*Func: 从某个中内存中往输出流中写入一定个数的终端地址信息
*param:
*	fd: output stream fd
*	ptmnl_addr: the buf of writing  to stream
*	write_counts: the num of of the type of "terminal_address_list" need to write
*return value:
*	-1:error
*	else: the right num of writing for stream or 0
*说明:注意:这里写入文件的数据时有一定的格式的，即按已经定义好的文件格式写入,从头开始读取
*******************************************************************/ 
int terminal_address_list_write_file( FILE* fd,  terminal_address_list* ptmnl_addr, const uint16_t write_counts )
{	
	assert( ptmnl_addr && fd );
	uint16_t tmnl_num = 0;
	uint16_t crc = 0; // 存储的crc
	int ret = 0;
	int i = 0;

	/* 1 read the num and the crc of terminal */
	if( (ret = Fseek(fd, 0, SEEK_SET)) == -1 ) // 从文件开始的地方读
		return -1;
	
	read_addr_file_terminal_num( fd, &tmnl_num );
	if( ferror(fd) )
	{
		DEBUG_ERR( "read addr err: Read file Err occur!" );
		return -1;
	}
	
	read_addr_file_terminal_crc( fd, &crc );
	if( ferror(fd) )
	{
		DEBUG_ERR( "read check err: Read file Err occur!" );
		return -1;
	}

	/* 2.  写入地址信息*/
	if( (ret = Fseek(fd, 0, SEEK_SET)) == -1 ) // 从文件开始的地方读
		return -1;
	
	tmnl_num += write_counts;
	uint8_t* pbuf = ( uint8_t* )ptmnl_addr;
	for( i = 0; i < write_counts * (sizeof(terminal_address_list)); i++ )
	{
		crc += pbuf[i];
	}

	ret = Fwrite( fd, &tmnl_num, sizeof(uint16_t), 1 ); // new num
	if( ferror( fd ) )
	{
		DEBUG_ERR( "write tmnl num err: ret = %d", ret );
		return -1;
	}

	ret = Fwrite( fd, &crc, sizeof(uint16_t), 1 ); // new crc
	if( ferror( fd ) )
	{
		DEBUG_ERR( "write crc err: ret = %d", ret );
		return -1;
	}

	if( (ret = Fseek( fd, 0, SEEK_END)) == -1 ) // 从文件结尾的地方开始写
		return -1;

	if( write_counts <= SYSTEM_TMNL_MAX_NUM )
	{
		int write_num = Fwrite( fd, ptmnl_addr, sizeof(terminal_address_list), write_counts );
		if( (write_num < write_counts) || ferror( fd ) )
		{
			DEBUG_INFO( " write terminal address Err: ret = %d", ret );
			return -1;
		}

		Fflush( fd ); // 写完一定要刷新到文件中，否则会读不成功
		return write_num;
	}
	
	Fflush( fd ); // 刷新到文件中
	return 0; // 从这里返回会正确的写入的数据
}

/*****************************************************************
*Writer:YasirLiang
*Date:2015/10/21
*Name:terminal_address_list_read_file
*Func:读取整个地址信息文件到相应的地址列表中
*param:
*	fd: input stream fd
*	ptmnl_addr: the buf of reading  from stream
*return value:
*	-1:error
*	else: the right terminal num of reading from stream
*******************************************************************/ 
int terminal_address_list_read_file( FILE* fd,  terminal_address_list* ptmnl_addr )
{
	assert( fd && ptmnl_addr );
	bool is_crc_right = false;

	if( Fseek( fd, 0, SEEK_SET ) == -1 ) // 从文件开始的地方读
		return -1;
	
	// read the num of terminal and the crc
	uint16_t tmnl_num = 0;
	if( (read_addr_file_terminal_num( fd, &tmnl_num ) == 0) && ( !ferror(fd) ) )
	{
		DEBUG_INFO( "read the num of terminal: end of file !" );
		return -1;
	}
	else if( ferror(fd) )
	{
		DEBUG_ERR( "read addr err: Read file Err occur!" );
		return -1;
	}

	// read the crc
	uint16_t read_crc = 0; // 存储的crc
	if( (read_addr_file_terminal_crc( fd, &read_crc ) <= 0) && ( !ferror(fd) ))
	{
		DEBUG_INFO( "read check err: end of file!" );
		return -1;
	}
	else if( ferror(fd) )
	{
		DEBUG_ERR( "read check err: Read file Err occur!" );
		return -1;
	}

	if( Fseek( fd, 4, SEEK_SET ) == -1 ) // 从文件的终端地址信息开始的地方读
		return -1;
	
	uint16_t count_crc = 0; // 计算的crc
	if( (tmnl_num > 0) && (tmnl_num <= SYSTEM_TMNL_MAX_NUM ))
	{
		// count the crc of the data
		while( !feof( fd ) )
		{
			uint8_t byte = 0;
			int ret = 0;
			ret = Fread( fd, &byte, sizeof(byte), 1 );
			if( ferror( fd ) ) // 读取错误
			{	
				return -1;
			}

			if( ret != 0 ) // 不是文件末尾
			{
				count_crc += byte;
			}
		}

		if( count_crc == read_crc )
			is_crc_right = true;
	}

	DEBUG_INFO( "read crc = %d --- count_crc = %d ", read_crc, count_crc );
	// if crc is right, read terminal address infomation
	int tmnl_counts = 0;
	if( is_crc_right )
	{
		if( Fseek( fd, 4, SEEK_SET ) == -1 ) // 从文件的终端地址信息开始的地方读
			return -1;
		
		while( !feof( fd ) )
		{
			terminal_address_list addr_tmp;
			int ret = Fread( fd, &addr_tmp, sizeof(addr_tmp), 1 ); // read one terminal address info every time
			if( ferror( fd ) ) // 读取错误,return 
			{
				return -1;
			}
			
			if( ret != 0 ) // not end of file
			{
				ptmnl_addr[tmnl_counts].addr = addr_tmp.addr;
				ptmnl_addr[tmnl_counts].tmn_type = addr_tmp.tmn_type;
				tmnl_counts++;
			}
		}
	}
	else
	{
		DEBUG_INFO( "read addr file err: check crc is wrong!" );
		return -1;
	}

	return tmnl_counts;
}

// send terminal conference deal message in 1722 frame payload by pipe
uint16_t ternminal_send( void *buf, uint16_t length, uint64_t uint64_target_id, bool is_resp_data )
{
	int send_len = 0;
	int cnf_data_len = 0;
	struct host_to_endstation *data_buf = (struct host_to_endstation*)buf;
	struct host_to_endstation fill_send_buf;
	struct jdksavdecc_frame send_frame;
	struct jdksavdecc_aecpdu_aem aemdu;
	struct jdksavdecc_eui64 target_id;
	convert_uint64_to_eui64( target_id.value, uint64_target_id );

	struct jdksavdecc_eui48 send_dest;
	if( uint64_target_id == BRDCST_1722_ALL )
	{
		memcpy( &send_dest, &jdksavdecc_multicast_adp_acmp, sizeof(struct jdksavdecc_eui48) );
	}
	else
	{
		convert_entity_id_to_eui48_mac_address( uint64_target_id, send_dest.value );
	}
	
	memcpy( send_frame.src_address.value, net.m_my_mac, 6 );
	cnf_data_len = conference_host_to_end_form_msg( &send_frame, &fill_send_buf, data_buf->cchdr.command_control, data_buf->data_len, data_buf->cchdr.address, data_buf->data );
	send_len = conference_1722_control_form_info( &send_frame, &aemdu, send_dest, target_id, cnf_data_len );
	if( send_len < 0 )
	{
		DEBUG_INFO( "send len is bad! send_len = %d", send_len );
		assert( send_len >= 0 );
	}

	DEBUG_INFO( "conf_data_len = %d send len = %d, send dest = %02x-%02x-%02x-%02x-%02x-%02x", cnf_data_len, send_len, \
		send_frame.dest_address.value[0], send_frame.dest_address.value[1], \
		send_frame.dest_address.value[2], send_frame.dest_address.value[3], \
		send_frame.dest_address.value[4], send_frame.dest_address.value[5]);
	system_raw_packet_tx( send_frame.dest_address.value, send_frame.payload, send_len, RUNINFLIGHT, TRANSMIT_TYPE_AECP, is_resp_data );
	
	return (uint16_t)send_len;
}

// proccess recv conference deal message from raw network
void terminal_recv_message_pro( struct terminal_deal_frame *conference_frame )
{
	assert( NULL != conference_frame );
	uint16_t frame_len = conference_frame->payload_len/2;
	uint8_t *p_right_data = NULL;
	uint8_t data_buf[MAX_FUNC_MSG_LEN] = { 0 };
	tmnl_pdblist tmnl_list_station = NULL;
	bool is_new_tmnl_list_station = false;
	
	// check the crc of the both data backups,if crc is wrong,return directory
	if( check_conferece_deal_data_crc( frame_len, conference_frame->payload, ZERO_OFFSET_IN_PAYLOAD))
	{	
		p_right_data = conference_frame->payload;
	}
	else
	{
		if( check_conferece_deal_data_crc( frame_len, conference_frame->payload + frame_len, ZERO_OFFSET_IN_PAYLOAD))
			p_right_data = conference_frame->payload + frame_len;
		else	
			return;
	}
	
	// 查看系统是否存在此实体，若存在继续处理;不存在新建节点后插入链表
	uint64_t target_id = convert_eui64_to_uint64_return(conference_frame->aecpdu_aem_header.aecpdu_header.header.target_entity_id.value);
	if( target_id == 0)
		return;

	tmnl_list_station = search_terminal_dblist_entity_id_node( target_id, dev_terminal_list_guard );
	if( NULL == tmnl_list_station )
	{
		is_new_tmnl_list_station = true;
		tmnl_list_station = create_terminal_dblist_node( &tmnl_list_station );
		if( NULL == tmnl_list_station )
		{
			DEBUG_INFO( "create new terminal dblist node failed!" );
			return;
		}

		init_terminal_dblist_node_info( tmnl_list_station );
		tmnl_list_station->tmnl_dev.entity_id = target_id; 
		insert_terminal_dblist_trail( dev_terminal_list_guard, tmnl_list_station );
	}
	
	if( NULL != tmnl_list_station )
	{
		DEBUG_RECV( p_right_data, frame_len, "Recv Right Conference Data" );
		memcpy( data_buf, p_right_data, frame_len );
		ttmnl_recv_msg recv_data;
		ssize_t ret = 0;
		ret = conference_end_to_host_deal_recv_msg_read( &recv_data, p_right_data, ZERO_OFFSET_IN_PAYLOAD, (TERMINAL_MESSAGE_MAX_LEN + HOST_COMMON_TO_END_EXDATA_LEN)*2, frame_len);
		if( ret < 0 )
		{
			DEBUG_INFO( "Err recv conference data read" );
			assert( ret >=0 );
		}

		if( !(recv_data.cchdr.command_control & COMMAND_FROM_TMN) ) // is not terminal command
		{
			return; 
		}
		
		if( recv_data.cchdr.command_control & COMMAND_TMN_REPLY ) // proccess response data
		{
			if((recv_data.cchdr.command_control & COMMAND_TMN_MASK)== QUERY_END)
			{
				terminal_register( recv_data.cchdr.address & TMN_ADDR_MASK, recv_data.data[0], tmnl_list_station );
			}
			else if((recv_data.cchdr.command_control & COMMAND_TMN_MASK)== SET_END_STATUS )
			{
				terminal_type_save( recv_data.cchdr.address & TMN_ADDR_MASK, recv_data.data[0],((recv_data.cchdr.command_control&COMMAND_TMN_CHAIRMAN)?true:false));
			}
			else if( (recv_data.cchdr.command_control & COMMAND_TMN_MASK) == CHECK_END_RESULT )
			{
				//query_vote_ask( recv_data.cchdr.address, recv_data.data[0]);
			}
		}

		if( (recv_data.cchdr.command_control & COMMAND_TMN_MASK) == TRANSIT_END_MSG ) // 特殊命令特殊处理
		{
			terminal_trasmint_message( recv_data.cchdr.address, recv_data.data, recv_data.data_len );
		}
		else // 处理其它命令
		{
			find_func_command_link( TERMINAL_USE, recv_data.cchdr.command_control & COMMAND_TMN_MASK, 0, data_buf, frame_len );
		}

	}
}

void host_reply_terminal( uint8_t cmd, uint16_t address, uint8_t *data_pay, uint16_t data_len )
{
	struct host_to_endstation askbuf; 
	uint16_t  asklen = 0;
	uint64_t  target_zero = 0;
	
	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = cmd |COMMAND_TMN_REPLY;
	askbuf.cchdr.address = address; 
	askbuf.data_len = data_len;

	if( data_pay != NULL && data_len != 0 )
	{
		memcpy( askbuf.data, data_pay, data_len );
	}

	ternminal_send( &askbuf, asklen, target_zero, true );
}



