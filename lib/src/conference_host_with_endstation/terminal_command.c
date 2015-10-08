#include "terminal_command.h"

static bool fill_ask_chk_sum( struct host_to_endstation* pask_buf, uint16_t* pask_len)
{
	uint8_t *p;
	uint8_t Chk;
	uint16_t i = 0;
	
	if((NULL == pask_buf)||(NULL == pask_len))
	{
		return false;
	}
	
	*pask_len = sizeof(struct conference_common_header) + 1 + pask_buf->data_len;// 负载长度，除了校验
	p = (uint8_t *)pask_buf;
	Chk = 0;
	for( i = 0; i < *pask_len; i++)
	{
		Chk ^= (*p);
		p++;
	}
	
	*p = Chk;
	(*pask_len) += 1;
	
	return true;
}


void terminal_query_endstation( void *pdata )
{
	 

	ternminal_send();
}

void terminal_reallot_address( void )
{
	struct host_to_endstation askbuf; 
	uint16_t  asklen;

	allot_addr_pro.AddrStart = 0;
	allot_addr_pro.Index = 0;
	allot_addr_pro.RenewFlag = 0;
	init_terminal_address_list();
	
	askbuf.cchdr.byte_guide = CONFERENCE_TYPE;
	askbuf.cchdr.command_control = HOST_TO_ENDSTATION_COMMAND_TYPE_REALLOCATION;
	askbuf.cchdr.address = 0x8000; // 全广播
	askbuf.data_len = 0;

	fill_ask_chk_sum(&askbuf,&asklen);
	ternminal_send( &askbuf, askbuf);
}

