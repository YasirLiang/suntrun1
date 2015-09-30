#include "conference_data_parser.h"

//从数据帧中获取会议系统的命令,普通与特殊的命令
int conference_parser_raw_data_get(struct jdksavdecc_frame *frame, 
									uint8_t *conference_data,
									ssize_t *conference_data_len,
									struct endstation_to_host *phost,
									struct endstation_to_host_special *spephost)
{
	printf("============================================\n");
	*conference_data_len = frame->length - CONFERENCE_DATA_IN_CONTROLDATA_LEN;
	ssize_t seclen = 0; 
	
	if( frame->payload && (*conference_data >= END_TO_HOST_CMD_LEN) )
	{
		if( (frame->payload[0] == JDKSAVDECC_1722A_SUBTYPE_AECP)  &&\
			( frame->payload[CONFERENCE_DATA_IN_CONTROLDATA_OFFSET - 1] == CONFERENCE_TYPE ) )
		{
			memcpy( conference_data, frame->payload + CONFERENCE_DATA_IN_CONTROLDATA_OFFSET, *conference_data_len);

			if( phost && spephost)
				seclen = conference_end_to_host_frame_read(frame, phost, spephost, 0, sizeof(frame->payload));
			
			if( seclen == *conference_data_len)
			{
				*conference_data_len = seclen;
				return 0;//正确数据长度
			}
				
		}
	}

	fprintf(stdout, "Raw data get failed:len?actlen:%d?%d\n", *conference_data_len, seclen);
	return -1;
}


