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
#include "queue_com.h"
#include "time_handle.h"
#include "ring_buffer.h"
#include "controller_machine.h"
#include "log_machine.h"

#ifdef MIC_PRIOR_MANEGER_ENABLE
#include "terminal_disconnect_connect_manager.h"
#endif /* MIC_PRIOR_MANEGER_ENABLE */

/*$ terminal Recieve Buffer Process */
typedef struct TTlRingMsgPro {
    /*! state switch and interval timer */
    TUserTimer smTimer, itvTimer;
    /* receive message over flag */
    bool recvOver;
    /*! recieve message lenght */
    uint32_t msgLen;
    /*! trace message count */
    uint16_t traceMsgCnt;
}TTlRingMsgPro;

enum TESendState {
    TMNL_SENDING,
    TMNL_WAITASK,
    TMNL_SEND_IDLE
};

/*$ terminal Recieve Buffer Process */
typedef struct TSendMsgPro {
    uint32_t state;/*! send msg state */
    uint32_t ptrMsgAddr; /*! pointer to msg */
}TSendMsgPro;

typedef struct T1722ForTmnlTable {
    uint64_t Id1722;
    uint16_t appAddr;
    bool map;
}T1722ForTmnlTable;

#define TERMINAL_MSG_BUF_SIZE 256
typedef struct TTmnlSendMsgElem {
    uint8_t reNum;
    uint32_t timeOut;
    uint16_t msgLen;
    uint8_t msgBuf[TERMINAL_MSG_BUF_SIZE]; /* including backup */
}TTmnlSendMsgElem;

#define TERMINAL_SEND_QSIZE 180

typedef struct TTerminal_comPro {
    TTerminalRcvMsg recvMsg;
    TCharRingBuf *pRingBuf;
    TTlRingMsgPro ringMsgPro;
    TComQueue *pSendQe; /*! send queue */
    TSendMsgPro sendPro;
}TTerminal_comPro;
/*$ char ring buffer size */
#define TERMINAL_RING_BUF_SIZE 180

/*! local function decralation */
static void Terminal_charMsgPro(void);
static void Terminal_sendMsgPro(void);
static void Terminal_appDataPro(void);

#ifdef ALLOT_IN_NEW_QE
/*$ ALLOT CMD queue size */
#define TALLOTCMD_QSIZE 32
#define ALLOT_CMD_TIMEOUT 50
/*$ current allot command queue element */
static TAllotAddrBuf *l_caQe = (TAllotAddrBuf *)0;
/*$ story pointer */
static uint32_t l_allotCmdElem[TALLOTCMD_QSIZE] = {
    0U
};
/*$ terminal allot command task queue */
static TComQueue l_allotCmdQueue = {
    0U, 0U, 0U, TALLOTCMD_QSIZE, &l_allotCmdElem[0]
};
/*$ lock mutex */
pthread_mutex_t l_allotMutex;
#endif /* ALLOT_IN_NEW_QE */

/*! local terminal recv buffer----------------------------------------------*/
static uint8_t l_recvBuf[TERMINAL_RING_BUF_SIZE];
/*$ local terminal ring buffer */
static TCharRingBuf l_terminalRingBuf = {
    1, &l_recvBuf[0], 0U, 0U, TERMINAL_RING_BUF_SIZE
};

static T1722ForTmnlTable l_table[SYSTEM_TMNL_MAX_NUM] = {
    {0U, 0xffff, 0},
};
/* store pointer */
static uint32_t l_sendQeBuf[TERMINAL_SEND_QSIZE];

static TComQueue l_sendQueue = {
    0U, 0U, 0U, TERMINAL_SEND_QSIZE, &l_sendQeBuf[0]
};

static TTerminal_comPro l_terminalPro;

T1722ForTmnlTable *T_Tab = &l_table[0];

extern tmnl_pdblist dev_terminal_list_guard; // 终端链表表头结点
extern solid_pdblist endpoint_list;			// 系统中终端链表哨兵节点
volatile bool gvregister_recved = false;// 单个终端注册完成的标志

bool T1722ForTmnlTable_updateAppAddr(uint64_t id, uint16_t app) {
    T1722ForTmnlTable *p = &l_table[0];
    int mapIndex;
    int i;
    bool found = false;
    for (i = 0; i < SYSTEM_TMNL_MAX_NUM; i++) {
        if (id == p[i].Id1722) {
            p[i].appAddr = app;
            if (!p[i].map) {
                p[i].map = true;
            }
            return true;
        }

        if ((!p[i].map)
              && (!found))
        {
            mapIndex = i;
            found = true;
        }
    }

    /* not found, add to end of table */
    p[mapIndex].Id1722 = id;
    p[mapIndex].map = true;
    p[mapIndex].appAddr = app;
    
    return true;
}

bool T1722ForTmnlTable_foundById(uint64_t id) {
    T1722ForTmnlTable *p = &l_table[0];
    int i;
    for (i = 0; i < SYSTEM_TMNL_MAX_NUM; i++) {
        if (id == p[i].Id1722) {
            return true;
        }
    }
    return false;
}

void T1722ForTmnlTable_clearMap(void) {
    T1722ForTmnlTable *p = &l_table[0];
    int i;
    for (i = 0; i < SYSTEM_TMNL_MAX_NUM; i++) {
        p[i].map = false;
        p[i].Id1722 = 0U;
        p[i].appAddr = 0xffff;
    }
}

bool T1722ForTmnlTable_foundByAppAddr(uint16_t addr, uint64_t *outId) {
    T1722ForTmnlTable *p = &l_table[0];
    int i;
    for (i = 0; i < SYSTEM_TMNL_MAX_NUM; i++) {
        if ((p[i].map)
               && (addr == p[i].appAddr))
        {
            *outId = p[i].Id1722;
            return true;
        }
    }
    return false;
}

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
		if( (write_num != write_counts) || ferror( fd ) )
		{
			DEBUG_INFO( " write terminal address Err: ret = %d", ret );
			return -1;
		}

		Fflush( fd ); // 写完一定要刷新到文件中，否则会读不成功
		DEBUG_INFO( "Success Write tmnl num total= %d", tmnl_num );
		
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
*	else: the right terminal num of reading from stream( change data 2016-1-23)
*******************************************************************/ 
int terminal_address_list_read_file( FILE* fd,  terminal_address_list* ptmnl_addr )
{
	assert( fd && ptmnl_addr );
	bool is_crc_right = false;
	terminal_address_list temp_addr_list[SYSTEM_TMNL_MAX_NUM];

	memset( temp_addr_list, 0, sizeof(terminal_address_list)*SYSTEM_TMNL_MAX_NUM);
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
		// read the file data and  count the crc of the data
		uint16_t rount_num = 0;
		uint16_t real_num = Fread( fd, temp_addr_list, sizeof(terminal_address_list), tmnl_num );
		if( real_num != tmnl_num )
		{
			DEBUG_INFO( "Err  real terminal num in addess file!" );
			return -1;
		}
		
		uint8_t *p = (uint8_t*)temp_addr_list;
		for( rount_num = 0; rount_num < real_num*sizeof(terminal_address_list); rount_num++ )
		{
			count_crc +=p[rount_num];
		}
		
		if( count_crc == read_crc )
			is_crc_right = true;
	}

	DEBUG_INFO( "read crc = %d --- count_crc = %d ", read_crc, count_crc );
	// if crc is right, read terminal address infomation
	int tmnl_counts = 0;
	if( is_crc_right )
	{
		tmnl_counts = tmnl_num;
		memcpy( ptmnl_addr, temp_addr_list, tmnl_counts*sizeof(terminal_address_list) );
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
#ifndef TERMINAL_COM_PRO_LATER
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

	system_raw_packet_tx( send_frame.dest_address.value, send_frame.payload, send_len, RUNINFLIGHT, TRANSMIT_TYPE_AECP, is_resp_data );
	
	return (uint16_t)send_len;
#else
    int sendLen = -1;
    /* not send system queue */
    TTmnlSendMsgElem *pMsg;
    struct host_to_endstation *data_buf = (struct host_to_endstation*)buf;
    pMsg = (TTmnlSendMsgElem*)malloc(sizeof(TTmnlSendMsgElem));
    if (pMsg != (TTmnlSendMsgElem *)0) {
        if (is_resp_data) {
            pMsg->reNum = 0;
            pMsg->timeOut = 0;
        }
        else {
            pMsg->reNum = 2;
            pMsg->timeOut =
                get_host_endstation_command_timeout(\
                    data_buf->cchdr.command_control & COMMAND_TMN_MASK);
        }
        
        /* send buffer */
        uint8_t *p = &pMsg->msgBuf[0];
        uint8_t crc = 0;
        int i = 0;
        
        memset(p, 0, TERMINAL_MSG_BUF_SIZE);
        p[0] = data_buf->cchdr.byte_guide;
        p[1] = data_buf->cchdr.command_control;
        p[2] = (uint8_t)(data_buf->cchdr.address & 0x00ff);
        p[3] = (uint8_t)((data_buf->cchdr.address & 0xff00) >> 8);
        p[4] = data_buf->data_len;
        if (data_buf->data_len > 0) {
            memcpy(&p[5], data_buf->data, data_buf->data_len);
        }
        
        pMsg->msgLen = 5 + data_buf->data_len;
        for (i = 0; i < pMsg->msgLen; i++) {
            crc ^= p[i];
        }
        
        p[pMsg->msgLen] = crc;
        pMsg->msgLen += 1; /* including crc */
        memcpy(&p[pMsg->msgLen], p, pMsg->msgLen);
        pMsg->msgLen += pMsg->msgLen;
        
        /* push to send message queue */
        if (!QueueCom_postFiFo(l_terminalPro.pSendQe, (void *)pMsg)) {
            free(pMsg);
            pMsg = NULL;
            
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                LOGGING_LEVEL_ERROR,
                "[Can't Post terminal send command to"
                "queue(%d):Queue full]",
                (uint32_t)l_terminalPro.pSendQe);
        }
        else {
            sendLen = pMsg->msgLen;
            
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[Post terminal send command"
                    " to queue(%d) Success]",
                    (uint32_t)l_terminalPro.pSendQe);
        }
    }
    
    return sendLen;
#endif /*TERMINAL_COM_PRO_LATER*/
}
// proccess recv conference deal message from raw network
extern terminal_address_list_pro allot_addr_pro;
void terminal_recv_message_pro( struct terminal_deal_frame *conference_frame )
{
    assert( NULL != conference_frame );
    uint16_t frame_len = conference_frame->payload_len/2;
    uint8_t *pRight = NULL;
    uint8_t data_buf[MAX_FUNC_MSG_LEN] = { 0 };
    tmnl_pdblist tmnl_list_station = NULL;

    // check the crc of the both data backups,if crc is wrong,return directory
    if( check_conferece_deal_data_crc( frame_len, conference_frame->payload, ZERO_OFFSET_IN_PAYLOAD))
    {	
        pRight = conference_frame->payload;
    }
    else
    {
        if( check_conferece_deal_data_crc( frame_len, conference_frame->payload + frame_len, ZERO_OFFSET_IN_PAYLOAD))
            pRight = conference_frame->payload + frame_len;
        else	
            return;
    }
#ifdef TERMINAL_COM_PRO_LATER
    /* save to ring buffer */
    int pos = 0;
    while (pos < frame_len) {
        RingBuffer_saveChar(l_terminalPro.pRingBuf, pRight[pos]);
        pos++;
    }
#endif
    memcpy( data_buf, pRight, frame_len );
    ttmnl_recv_msg recv_data;
    ssize_t ret = 0;
    DEBUG_RECV( data_buf, frame_len, "Recv Right Conference Data====>>>>>" );
    ret = conference_end_to_host_deal_recv_msg_read( &recv_data, data_buf, ZERO_OFFSET_IN_PAYLOAD, sizeof(ttmnl_recv_msg), frame_len);
    if( ret < 0 )
    {
        DEBUG_INFO( "Err recv conference data read" );
        assert( ret >=0 );
    }

    if( !(recv_data.cchdr.command_control & COMMAND_FROM_TMN) ) // is not terminal command
    {
        return; 
    }

    // 查看系统是否存在此实体，若存在继续处理;不存在新建节点后插入链表
    uint64_t target_id;
    uint8_t *ptr;
    ptr = conference_frame->aecpdu_aem_header.aecpdu_header.header.target_entity_id.value;
    target_id = convert_eui64_to_uint64_return(ptr);
    tmnl_list_station = search_terminal_dblist_entity_id_node(target_id, dev_terminal_list_guard);
    if( NULL == tmnl_list_station )
    {
        bool foud_by_addr = false;

        if ((recv_data.cchdr.command_control & COMMAND_TMN_MASK) != ALLOCATION)
            foud_by_addr = true;

        if (foud_by_addr)
        {
            tmnl_list_station = search_terminal_dblist_address_id_node(recv_data.cchdr.address & TMN_ADDR_MASK, dev_terminal_list_guard);
            if (tmnl_list_station != NULL)
            {
            	tmnl_list_station->tmnl_dev.entity_id = target_id;
            	foud_by_addr = true;
            }
            else
            {
                foud_by_addr = false;
            	DEBUG_INFO("No found addr terminal addr = %04x", recv_data.cchdr.address & TMN_ADDR_MASK);
            }
        }

        if (!foud_by_addr)
        {
            DEBUG_INFO( "create new tmnl list node[ 0x%016llx ] ", target_id );
            tmnl_list_station = create_terminal_dblist_node( &tmnl_list_station );
            if (tmnl_list_station == NULL)
            {
            	DEBUG_INFO( "create new terminal dblist node failed!" );
            	return;
            }

            init_terminal_dblist_node_info( tmnl_list_station );
            tmnl_list_station->tmnl_dev.entity_id = target_id; 
            insert_terminal_dblist_trail( dev_terminal_list_guard, tmnl_list_station );
        }
    }
    /* update table */
    if (((recv_data.cchdr.command_control & COMMAND_TMN_MASK) == QUERY_END)
          && (!(recv_data.cchdr.address & BROADCAST_FLAG))) {
        DEBUG_INFO("update addr = 0x%04x",
            recv_data.cchdr.address & TMN_ADDR_MASK);
        T1722ForTmnlTable_updateAppAddr(target_id,
            recv_data.cchdr.address & TMN_ADDR_MASK);
    }
#ifndef TERMINAL_COM_PRO_LATER
    if( NULL != tmnl_list_station )
    {
    	if( recv_data.cchdr.command_control & COMMAND_TMN_REPLY ) // proccess response data
    	{
            if((recv_data.cchdr.command_control & COMMAND_TMN_MASK)== QUERY_END)
            {
            	terminal_register( recv_data.cchdr.address & TMN_ADDR_MASK, recv_data.data[0], tmnl_list_station );
                // 注册完成
            	gvregister_recved = true;
            }
            else if((recv_data.cchdr.command_control & COMMAND_TMN_MASK)== SET_END_STATUS )
            {
            	terminal_type_save( recv_data.cchdr.address & TMN_ADDR_MASK, recv_data.data[0],((recv_data.cchdr.command_control&COMMAND_TMN_CHAIRMAN)?true:false));
            }
            else if( (recv_data.cchdr.command_control & COMMAND_TMN_MASK) == CHECK_END_RESULT )
            {
            	// 主机查询终端响应函数
            	terminal_query_vote_ask( recv_data.cchdr.address, recv_data.data[0] );
            }
    	}

    	if( (recv_data.cchdr.command_control & COMMAND_TMN_MASK) == TRANSIT_END_MSG ) // 特殊命令特殊处理
    	{
            terminal_trasmint_message( recv_data.cchdr.address, recv_data.data, recv_data.data_len );
    	}
#ifdef ALLOT_IN_NEW_QE
        else if ((recv_data.cchdr.command_control & COMMAND_TMN_MASK) == ALLOCATION) {
            pthread_mutex_lock(&l_allotMutex);
            if (!(recv_data.cchdr.command_control & COMMAND_TMN_REPLY)) {
                uint32_t pos, addr;
                TAllotAddrBuf *ptr = (TAllotAddrBuf *)0;
                bool found = false;

                /* if queue node has no allocation command, push to queue, otherwise
                     update lastest time for this allocation command */
                queue_for_each(&l_allotCmdQueue, pos, addr) {
                    ptr = (TAllotAddrBuf *)addr;
                    if (ptr != 0) {
                        ttmnl_recv_msg data;
                        uint16_t addr_;
                        ssize_t ret = 0;
                        
                        ret = conference_end_to_host_deal_recv_msg_read(&data,
                            ptr->buf, 0,
                            sizeof(ttmnl_recv_msg), ptr->len);
                        if (ret < 0) {
                            continue;
                        }
                        
                        addr_ = data.cchdr.address;
                        if (addr_ == recv_data.cchdr.address) {
                            /* address equal? */
                            userTimerStart(ALLOT_CMD_TIMEOUT, &ptr->timer);
                            found = true;
                            
                            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                LOGGING_LEVEL_DEBUG,
                                "[Update allot command to"
                                " Queue(%d) addr = 0x%04x]",
                                (uint32_t)&l_allotCmdQueue,
                                addr_);
                            break;
                        }
                    }
                }
                
                ptr = (TAllotAddrBuf *)0;
                /* get current allot address */
                if (l_caQe != NULL) {
                    ttmnl_recv_msg data_;
                    ssize_t ret = 0;
                    ret = conference_end_to_host_deal_recv_msg_read(&data_,
                        l_caQe->buf, 0,
                        sizeof(ttmnl_recv_msg), l_caQe->len);
                    if (data_.cchdr.address == recv_data.cchdr.address) {
                        found = true;
                    }
                }
                
                if (!found) {
                    if (frame_len > ALLOT_MAX_SIZE) {
                        pthread_mutex_unlock(&l_allotMutex);
                        return;
                    }
                        
                    /* add to queue */
                    ptr = (TAllotAddrBuf *)malloc(sizeof(TAllotAddrBuf));
                    if (ptr != (TAllotAddrBuf *)0) {
                        ptr->vptr = terminal_func_allot_address;
                        ptr->len = frame_len;
                        memcpy(ptr->buf, data_buf, frame_len);
                        /* exist value time */
                        userTimerStart(ALLOT_CMD_TIMEOUT, &ptr->timer);
                        ptr->renew = false;
                        
                        if (QueueCom_postFiFo(&l_allotCmdQueue,
                                                (void *)ptr))
                        {
                            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                LOGGING_LEVEL_DEBUG,
                                "[Post New allot command to"
                                " Queue(%d) Taddr = 0x%04x Success]",
                                (uint32_t)&l_allotCmdQueue,
                                recv_data.cchdr.address);
                       }
                       else {
                            free(ptr);
                            ptr = (TAllotAddrBuf *)0;
                            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                LOGGING_LEVEL_ERROR,
                                "[Can't Post New allot command to Queue(%d) "
                                "Taddr = 0x%04x, No space]",
                                (uint32_t)&l_allotCmdQueue,
                                recv_data.cchdr.address);
                        }
                    }
                }
                else {
                    /* abandon packet */
                }
            }
            else {
                ttmnl_recv_msg data;
                ssize_t ret = -1;
                
                /* no current allot task? */
                if (l_caQe == (TAllotAddrBuf *)0) {
                    pthread_mutex_unlock(&l_allotMutex);
                    return;
                }
                
                ret = conference_end_to_host_deal_recv_msg_read(&data,
                    l_caQe->buf, 0,
                    ALLOT_MAX_SIZE, l_caQe->len);
                if (ret < 0) {
                    pthread_mutex_unlock(&l_allotMutex);
                    return;
                }
                
                if (data.cchdr.address == recv_data.cchdr.address) {
                    /* write new data */
                    l_caQe->len = frame_len;
                    memcpy(l_caQe->buf, data_buf, frame_len);
                    l_caQe->renew = true;
                    /* exist value time */
                    userTimerStart(ALLOT_CMD_TIMEOUT, &l_caQe->timer);
                }
                else {
                    /* do nothing */
                }
            }

            pthread_mutex_unlock(&l_allotMutex);
        }
#endif /* ALLOT_IN_NEW_QE */  
        else // 处理其它命令
    	{
            find_func_command_link( TERMINAL_USE, recv_data.cchdr.command_control & COMMAND_TMN_MASK, 0, data_buf, frame_len );
    	}
    }
#endif    
}

void host_reply_terminal(uint8_t cmd, uint16_t address,
    uint8_t *data_pay, uint16_t data_len)
{
    struct host_to_endstation askbuf; 
    uint16_t  asklen = 0;
    uint64_t  target_zero = 0;

    askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
    askbuf.cchdr.command_control = cmd |COMMAND_TMN_REPLY;
    askbuf.cchdr.address = address; 
    askbuf.data_len = data_len;

    if ((data_pay != NULL)
        && (data_len != 0))
    {
        memcpy( askbuf.data, data_pay, data_len );
    }

    ternminal_send(&askbuf, asklen, target_zero, true);
}

void terminal_common_create_node_by_adp_discover_can_regist(
    const uint64_t  target_id )
{
    tmnl_pdblist tmnl_list_station =
        search_terminal_dblist_entity_id_node(target_id,
            dev_terminal_list_guard);
    if (NULL == tmnl_list_station) {
        DEBUG_INFO( "create new tmnl list node[ 0x%016llx ] ", target_id);
        tmnl_list_station = create_terminal_dblist_node( &tmnl_list_station);
        if (NULL == tmnl_list_station) {
            DEBUG_INFO("create new terminal dblist node failed!");
            return;
        }

        init_terminal_dblist_node_info(tmnl_list_station);
        tmnl_list_station->tmnl_dev.entity_id = target_id;
        host_timer_stop( &tmnl_list_station->tmnl_dev.spk_timeout);
        insert_terminal_dblist_trail(dev_terminal_list_guard,
            tmnl_list_station);
    }
}
#ifdef ALLOT_IN_NEW_QE
extern terminal_address_list tmnl_addr_list[SYSTEM_TMNL_MAX_NUM];
void Terminal_allotPro(void) {
    uint32_t addr = 0;
    TAllotAddrBuf **ppCaQe = &l_caQe; /*! current allot command queue element */
    if (*ppCaQe == (TAllotAddrBuf *)0) {
        if (QueueCom_popFiFo(&l_allotCmdQueue, &addr)) {
            *ppCaQe = (TAllotAddrBuf *)addr;
            bool timeout = userTimerTimeout(&(*ppCaQe)->timer);
            if (!timeout) {
                (*ppCaQe)->vptr(0, (*ppCaQe)->buf, (*ppCaQe)->len);
                userTimerStart(ALLOT_CMD_TIMEOUT, &(*ppCaQe)->timer);

                /* log information */
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[Excute allot command begin]");
            }
            else {
                QueueCom_itemFree((void *)(*ppCaQe));
                *ppCaQe = (TAllotAddrBuf *)0;
            }
        }
    }
    else {
        if (((*ppCaQe)->renew)
            && (!userTimerTimeout(&(*ppCaQe)->timer)))
        {
            (*ppCaQe)->vptr(0, (*ppCaQe)->buf,(*ppCaQe)->len);
            QueueCom_itemFree((void *)(*ppCaQe));
            *ppCaQe = (TAllotAddrBuf *)0;

            /* log information */
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[Excute allot command Finish]");
        }
        else if (userTimerTimeout(&(*ppCaQe)->timer)) {
            /* this timer must start */
            QueueCom_itemFree((void *)(*ppCaQe));
            *ppCaQe = (TAllotAddrBuf *)0;
            /* log allot current address failed */
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_ERROR,
                    "[Excute allot command Falied, Timeout]");
            
            /* release current allotted(but not use) address */
            terminal_address_list_pro* pAllot = &allot_addr_pro;
            if (!pAllot->renew_flag) {
                uint16_t *pA = &tmnl_addr_list[pAllot->index].addr;
                if (!found_terminal_dblist_node_by_addr(*pA)) {
                    *pA = 0xffff;
                }
            }
        }
        else {
            /* do nothing */
        }
    }
}
#endif
static void Terminal_sendMsgPro(void) {
    uint32_t *pSendState = &l_terminalPro.sendPro.state;
    uint32_t addr;
    if ((*pSendState == TMNL_SEND_IDLE)
          && (QueueCom_popFiFo(l_terminalPro.pSendQe, &addr)))
    {  
        /* format data and send */
        struct jdksavdecc_frame frame;
        struct jdksavdecc_aecpdu_aem aemdu;
        TTmnlSendMsgElem *sendMsg = (TTmnlSendMsgElem *)addr;
        uint16_t tmnlAddr = *((uint16_t *)&sendMsg->msgBuf[2]);
        uint64_t target_id = 0U;
        struct jdksavdecc_eui64 uid;
        struct jdksavdecc_eui48 send_dest;
        int sendLen = 0;
        
        l_terminalPro.sendPro.ptrMsgAddr = addr;
        
        if (T1722ForTmnlTable_foundByAppAddr(tmnlAddr, &target_id)) {
            convert_uint64_to_eui64(uid.value, target_id);
            convert_entity_id_to_eui48_mac_address(target_id,
                send_dest.value);
        }
        else {
            memcpy(&send_dest,
                &jdksavdecc_multicast_adp_acmp,
                sizeof(struct jdksavdecc_eui48));
        }

        sendLen = Conference_1722DataFormat(&frame, &aemdu,
                send_dest,
                uid,
                sendMsg->msgLen,
                sendMsg->msgBuf);
        if (sendLen >= 0) {
            uint8_t ethertype[2] = {0x22, 0xf0};
            int frameLen = sendLen + ETHER_HDR_SIZE;
            memcpy(frame.payload, send_dest.value, 6);
            memcpy(frame.payload+6, net.m_my_mac, 6);
            memcpy(frame.payload+12, ethertype, 2);
            controller_machine_1722_network_send(gp_controller_machine,
                frame.payload, frameLen);
            *pSendState = TMNL_SENDING;
            
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[ IDLE Send App Data Success ]");
        }
        else { /* error send len */
            QueueCom_itemFree(
                (void *)l_terminalPro.sendPro.ptrMsgAddr);
            l_terminalPro.sendPro.ptrMsgAddr = 0U;
            gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_ERROR,
                    "[ IDLE Send App Data Failed(Format data error) ]");
        }
    }

    if (*pSendState == TMNL_SENDING) {
        if (l_terminalPro.sendPro.ptrMsgAddr != 0) {
            TTmnlSendMsgElem *sendMsg =
                (TTmnlSendMsgElem *)l_terminalPro.sendPro.ptrMsgAddr;
            if (sendMsg->timeOut) {
                over_time_set(TMNL_WAITASK_TIMEOUT, sendMsg->timeOut);
                *pSendState = TMNL_WAITASK;
                
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[ Send State Machine Change to WaitAsk"
                    "(Timeout = %d) ]", sendMsg->timeOut);
            }
            else {
                QueueCom_itemFree(
                    (void *)l_terminalPro.sendPro.ptrMsgAddr);
                l_terminalPro.sendPro.ptrMsgAddr = 0U;
                *pSendState = TMNL_SEND_IDLE;
            }
        }
    }
    else if ((*pSendState == TMNL_WAITASK)
        && (over_time_listen(TMNL_WAITASK_TIMEOUT)))
    {
        TTmnlSendMsgElem *sendMsg =
            (TTmnlSendMsgElem *)l_terminalPro.sendPro.ptrMsgAddr;
        if (sendMsg->reNum > 0) {   
            /* format data and send */
            struct jdksavdecc_frame frame;
            struct jdksavdecc_aecpdu_aem aemdu;
            uint16_t tmnlAddr = *((uint16_t *)&sendMsg->msgBuf[2]);
            uint64_t target_id = 0U;
            struct jdksavdecc_eui64 uid;
            struct jdksavdecc_eui48 send_dest;
            int sendLen = 0;
            
            if (T1722ForTmnlTable_foundByAppAddr(tmnlAddr, &target_id)) {
                convert_uint64_to_eui64(uid.value, target_id);
                convert_entity_id_to_eui48_mac_address(target_id,
                    send_dest.value);
            }
            else {
                memcpy(&send_dest,
                    &jdksavdecc_multicast_adp_acmp,
                    sizeof(struct jdksavdecc_eui48));
            }
            
            sendLen = Conference_1722DataFormat(&frame, &aemdu,
                    send_dest,
                    uid,
                    sendMsg->msgLen,
                    sendMsg->msgBuf);
            if (sendLen > 0) {
                uint8_t ethertype[2] = {0x22, 0xf0};
                int frameLen = sendLen + ETHER_HDR_SIZE;
                memcpy(frame.payload, send_dest.value, 6);
                memcpy(frame.payload+6, net.m_my_mac, 6);
                memcpy(frame.payload+12, ethertype, 2);
                controller_machine_1722_network_send(
                    gp_controller_machine,
                    frame.payload, frameLen);
                *pSendState = TMNL_SENDING;
                /* send data */
                sendMsg->reNum--;
                
                gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[ ReSend App Data Success ]");
            }
            else { /* error send len */
                QueueCom_itemFree(
                    (void *)l_terminalPro.sendPro.ptrMsgAddr);
                l_terminalPro.sendPro.ptrMsgAddr = 0U;
                *pSendState = TMNL_SEND_IDLE;
            }
        }
        else {
            QueueCom_itemFree(
                (void *)l_terminalPro.sendPro.ptrMsgAddr);
            l_terminalPro.sendPro.ptrMsgAddr = 0U;
            *pSendState = TMNL_SEND_IDLE;
             gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                    LOGGING_LEVEL_DEBUG,
                    "[ ReSend App Data Done ]");
        }
    }
    else {
        /* do nothing */
    }
}

static void Terminal_appDataPro(void) {
    TTerminalRcvMsg *pMsgPro = &l_terminalPro.recvMsg;
    uint16_t tmnlAddr = pMsgPro->addr & TMN_ADDR_MASK;
    uint8_t cmd = pMsgPro->cmd & COMMAND_TMN_MASK; /* only cmd */
    bool isChairMan = (pMsgPro->cmd&COMMAND_TMN_CHAIRMAN)?true:false;
    uint64_t target_id;

    /* not a terminal command */
    if (!(pMsgPro->cmd & COMMAND_FROM_TMN)) {
        DEBUG_INFO("[ Proccess App Data Failed->not"
        "Terminal comand ]");
        return;
    }
    
    if (pMsgPro->cmd & COMMAND_TMN_REPLY) {/* proccess response data */
        if (l_terminalPro.sendPro.state != TMNL_SEND_IDLE) {
            uint16_t preAddr;
            uint16_t preCmd;
            TTmnlSendMsgElem *sendMsg;
            
            sendMsg = (TTmnlSendMsgElem *)l_terminalPro.sendPro.ptrMsgAddr;
            if (sendMsg == (TTmnlSendMsgElem *)0) {
                DEBUG_INFO("[ Proccess App Data Failed->"
                    "no data in current task node ]");
                return;
            }
            
            preAddr = *((uint16_t *)&sendMsg->msgBuf[2]);
            preCmd = *(&sendMsg->msgBuf[1]);

            /* check command */
            if ((preCmd & COMMAND_TMN_MASK) != cmd) {
                DEBUG_INFO("[ Proccess App Data Failed->no"
                    "match Current send Cmd ]");
                return;
            }
            
            /* check address */
            if ((!(pMsgPro->addr & BROADCAST_FLAG))
                  && (tmnlAddr != (preAddr &TMN_ADDR_MASK)))
            {
                DEBUG_INFO("[ Proccess App Data Failed->no"
                    "match Current send Addr ]");
                return;
            }

            /* free node */
            QueueCom_itemFree(
                        (void *)l_terminalPro.sendPro.ptrMsgAddr);
            l_terminalPro.sendPro.ptrMsgAddr = 0U;

            /* change send-state machine state */
            DEBUG_INFO("[Send State Machine Change to IDLE]");
            l_terminalPro.sendPro.state = TMNL_SEND_IDLE;
            
            if (cmd == QUERY_END) {
                /* looking target id */
                if (T1722ForTmnlTable_foundByAppAddr(tmnlAddr, &target_id)) {
                    tmnl_pdblist tmnl_list_station;
                    tmnl_list_station = search_terminal_dblist_entity_id_node(target_id,
                        dev_terminal_list_guard);
                    if (NULL != tmnl_list_station) {
                        DEBUG_INFO("[ Process register command begin ]");
                        terminal_register(tmnlAddr, pMsgPro->buf[0], tmnl_list_station);
                        // 注册完成
                        gvregister_recved = true;
                    }
                }
            }
            else if (cmd == SET_END_STATUS) {
                DEBUG_INFO("[ Process SET_END command begin ]");
                terminal_type_save(tmnlAddr, pMsgPro->buf[0], isChairMan);
            }
            else if (cmd == CHECK_END_RESULT) {
                DEBUG_INFO("[ Process CHECK_END_RESULT command begin ]");
                terminal_query_vote_ask(tmnlAddr, pMsgPro->buf[0]);
            }
        }
    }
        
    if (cmd == TRANSIT_END_MSG) {
        DEBUG_INFO("[ Process TRANSIT_END_MSG command begin ]");
        terminal_trasmint_message(tmnlAddr, pMsgPro->buf, pMsgPro->data);
    }
#ifdef ALLOT_IN_NEW_QE    
    else if (cmd == ALLOCATION) {
        uint32_t frame_len = l_terminalPro.ringMsgPro.msgLen;
        
        pthread_mutex_lock(&l_allotMutex);
                
        if (!(pMsgPro->cmd & COMMAND_TMN_REPLY)) {
            uint32_t pos, addr;
            TAllotAddrBuf *ptr = (TAllotAddrBuf *)0;
            bool found = false;

            /* if queue node has no allocation command, push to queue, otherwise
                 update lastest time for this allocation command */
            queue_for_each(&l_allotCmdQueue, pos, addr) {
                ptr = (TAllotAddrBuf *)addr;
                if (ptr != 0) {
                    ttmnl_recv_msg data;
                    uint16_t addr_;
                    ssize_t ret = 0;
                    
                    ret = conference_end_to_host_deal_recv_msg_read(&data,
                        ptr->buf, 0,
                        sizeof(ttmnl_recv_msg), ptr->len);
                    if (ret < 0) {
                        continue;
                    }
                    
                    addr_ = data.cchdr.address;
                    if (addr_ == pMsgPro->addr) {
                        /* address equal? */
                        userTimerStart(ALLOT_CMD_TIMEOUT, &ptr->timer);
                        found = true;
                        DEBUG_INFO("[Post allot command to"
                            " Queue update(0x%04x)]", addr_);
                        break;
                    }
                }
            }
            
            ptr = (TAllotAddrBuf *)0;
            /* get current allot address */
            if (l_caQe != NULL) {
                ttmnl_recv_msg data_;
                ssize_t ret = 0;
                
                ret = conference_end_to_host_deal_recv_msg_read(&data_,
                    l_caQe->buf, 0,
                    sizeof(ttmnl_recv_msg), l_caQe->len);
                if (data_.cchdr.address == pMsgPro->addr) {
                    found = true;
                }
            }
            
            if (!found) {
                if (frame_len > ALLOT_MAX_SIZE) {
                    pthread_mutex_unlock(&l_allotMutex);
                    return;
                }
                
                /* add to queue */
                ptr = (TAllotAddrBuf *)malloc(sizeof(TAllotAddrBuf));
                if (ptr != (TAllotAddrBuf *)0) {
                    ptr->vptr = terminal_func_allot_address;
                    ptr->len = frame_len;
                    memcpy(ptr->buf, (uint8_t *)pMsgPro, frame_len);
                    /* exist value time */
                    userTimerStart(ALLOT_CMD_TIMEOUT, &ptr->timer);
                    ptr->renew = false;
                    QueueCom_postFiFo(&l_allotCmdQueue, (void *)ptr);
                    DEBUG_INFO("[Post allot command to"
                        " Queue new(0x%04x)]", pMsgPro->addr);
                }
            }
            else {
                /* abandon packet */
            }
        }
        else {
            /* no current allot task? */
            if (l_caQe == (TAllotAddrBuf *)0) {
                pthread_mutex_unlock(&l_allotMutex);
                return;
            }
            
            ttmnl_recv_msg data;
            ssize_t ret = 0;
            ret = conference_end_to_host_deal_recv_msg_read(&data,
                l_caQe->buf, 0,
                ALLOT_MAX_SIZE, l_caQe->len);
            if (ret < 0) {
                pthread_mutex_unlock(&l_allotMutex);
                return;
            }
            
            DEBUG_INFO("[allot command last renew (0x%04x-0x%04x)]",
                data.cchdr.address, pMsgPro->addr);
            if (data.cchdr.address == pMsgPro->addr) {
                /* write new data */
                l_caQe->len = frame_len;
                memcpy(l_caQe->buf, (uint8_t *)pMsgPro, frame_len);
                l_caQe->renew = true;
                /* exist value time */
                userTimerStart(ALLOT_CMD_TIMEOUT, &l_caQe->timer);
            }
            else {
                /* do nothing */
            }
        }

        pthread_mutex_unlock(&l_allotMutex);
    } 
#endif /* ALLOT_IN_NEW_QE */
    else {
        /* not including specical cmd like 'TRANSIT_END_MSG' and include crc*/
        find_func_command_link(TERMINAL_USE, cmd,
            0, (uint8_t *)pMsgPro, 6);
    }
}

static void Terminal_charMsgPro(void) {
    TTerminalRcvMsg *pMsgPro;
    TTlRingMsgPro *pRingPro;
    uint8_t ch; /* char store */
    pRingPro = &l_terminalPro.ringMsgPro;
    if (pRingPro->recvOver) {
        userTimerStart(10, &pRingPro->itvTimer);
        if (userTimerTimeout(&pRingPro->smTimer)) {
            /* process app data  here */
            DEBUG_INFO("[ proccess App Data Begin ]");
            Terminal_appDataPro();
            /* stop sm timer */
            userTimerStop(&pRingPro->smTimer);
            pRingPro->msgLen = 0;
            pRingPro->recvOver = (bool)0;
        }
    }
    /* get ring char in buffer */
    pMsgPro = &l_terminalPro.recvMsg;
    while (RingBuffer_getChar(l_terminalPro.pRingBuf, &ch)) {
        userTimerStart(10, &pRingPro->itvTimer);
        pRingPro->recvOver = (bool)0;
        if ((pRingPro->msgLen == 0)
              && (ch == CONFERENCE_TYPE))
        {
            pMsgPro->head = CONFERENCE_TYPE;
            pRingPro->msgLen = 1;
        }
        else if (pRingPro->msgLen == 1) {
            pMsgPro->cmd = ch;
            pRingPro->msgLen = 2;
        }
        else if (pRingPro->msgLen == 2) {
            /* '=' will clear  data stored last time, must be */
            pMsgPro->addr = (((uint16_t)ch) & 0x00ff);
            pRingPro->msgLen = 3;
        }
        else if (pRingPro->msgLen == 3) {
            /* '|=' must be */
            pMsgPro->addr |= ((((uint16_t)ch) << 8) & 0xff00);
            pRingPro->msgLen = 4;
        }
        else if (pRingPro->msgLen == 4) {
            pMsgPro->data = ch;
            pRingPro->msgLen = 5;
            
            if ((pMsgPro->cmd & COMMAND_TMN_MASK) == TRANSIT_END_MSG) {
                pRingPro->traceMsgCnt = ch + 1; /* contain crc */
                if (pMsgPro->data > TERMINAL_MESSAGE_MAX_LEN) {
                    pRingPro->msgLen = 0;
                }
            }
            else {
                pRingPro->traceMsgCnt = 1;
            }
        }
        else if (pRingPro->msgLen >= 5) {
            if (pRingPro->traceMsgCnt > 0) {
                pMsgPro->buf[pRingPro->msgLen - 5] = ch;
                pRingPro->msgLen++;
                pRingPro->traceMsgCnt--;
            }
            
            if (pRingPro->traceMsgCnt == 0) {
                if (check_conferece_deal_data_crc(pRingPro->msgLen,
                    pMsgPro, 0))
                {
                    pRingPro->recvOver = (bool)1;
                    userTimerStart(2, &pRingPro->smTimer);
                    DEBUG_INFO("[ Parser char Message Done ]");
                    break;
                }
                else {
                    pRingPro->msgLen = 0;
                }
            }
        }
        else {
            /* never come this else */
        }
    }
    
    if (userTimerTimeout(&pRingPro->itvTimer)) {
        pRingPro->msgLen = 0;
    }
}

void Terminal_comPro(uint32_t sysTick) {
    Terminal_charMsgPro();
    Terminal_sendMsgPro();
#ifdef ALLOT_IN_NEW_QE
    pthread_mutex_lock(&l_allotMutex);
    Terminal_allotPro();
    pthread_mutex_unlock(&l_allotMutex);
#endif /* ALLOT_IN_NEW_QE */
    terminal_speakInterposeOffPro();

    /* process firt apply */
    terminal_firstSpkingPro(sysTick);

#ifdef MIC_PRIOR_MANEGER_ENABLE
    Terminal_micManagerTask(sysTick);
#endif /* MIC_PRIOR_MANEGER_ENABLE */
}

void Terminal_comInitial(void) {
    l_terminalPro.pRingBuf = &l_terminalRingBuf;
    userTimerStop(&l_terminalPro.ringMsgPro.smTimer);
    userTimerStop(&l_terminalPro.ringMsgPro.itvTimer);
    l_terminalPro.ringMsgPro.recvOver = (bool)1;
    l_terminalPro.ringMsgPro.msgLen = 0;
    l_terminalPro.ringMsgPro.traceMsgCnt = 0;
    l_terminalPro.pSendQe = &l_sendQueue;
    l_terminalPro.sendPro.state = TMNL_SEND_IDLE;
    l_terminalPro.sendPro.ptrMsgAddr = 0U;
#ifdef ALLOT_IN_NEW_QE
    pthread_mutex_init(&l_allotMutex, NULL);
#endif /* ALLOT_IN_NEW_QE */
}

