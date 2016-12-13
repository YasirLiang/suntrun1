/*
* @file protocal_qt.h
* @brief protocal communicate with qt application
* @ingroup protocal with qt and server
* @cond
******************************************************************************
* Build Date on  2016-11-11
* Last updated for version 1.0.0
* Last updated on  2016-11-30
*
*                    Moltanisk Liang
*                    ---------------------------
*                    Avdecc Reception Control System
*
* Copyright (C) Moltanisk Liang, GuangZhou Suntron. All rights reserved.
******************************************************************************
* @endcond
*/
/*Including-----------------------------------------------------------------*/
#ifndef __PROTOCAL_QT_H__
#define __PROTOCAL_QT_H__
/*! make a destinction between c and c++ file */
#ifdef __cplusplus /* only define in .cpp file */
    extern "C" { /*! for c++ file transfer c function */
#endif /* __cplusplus */
/*! the head TYPE of qt protocal with server */
#define PROTOCAL_QT_TYPE 0xAA
/*! protocal response mask */
#define PRO_RESP_MASK 0x80
/*! protocal query type */
#define PRO_QUERY_TYPE 0x00
/*! protocal set type */
#define PRO_SET_TYPE 0x20
/*! protocal report type from server */
#define PRO_REPORT_TYPE 0x40
/*! protocal handling mask */
#define PRO_HANDLING_MASK 0x02
/*! protocal error mask */
#define PRO_ERR_MASK 0x1f
/*! protocal commom lenght */
#define PRO_COMMON_LEN 0x07
/*! protocal data offset */
#define PRO_DATA_POS 0x07
/*! data max lenght define */
#define PRO_QT_MAX 256 
/*!{@ protocal command bewteen qt and server define */
/*! qt queue terminal command */
#define QT_QUEUE_ID 0x01
/*! qt switch matrix command */
#define QT_SWITCH_MATRIX 0x02
/*! qt operation terminal command */
#define QT_OPT_TMNL 0x03
/*! qt system setting command */
#define QT_SYS_SET 0x04
/*! qt camera control command */
#define QT_CMR_CTL 0x05
/*!end of command define @} */
/*! each cmd max error number */
#define MAX_ERR_CODE 0x32
#define MAX_CMD_NUM 0x05

/*! {@Error code define-----------------------------------------------------*/
enum TEArcsErrorCode {
    QR_SUCCESS = 0x00, /*! NORMAL */
    NO_ID = 0x01, /*! QEURY NO SUCH ID */
    HOST_HANDING = 0x02, /*! HANDLING */
    MAX_QR_ER_PUB = MAX_ERR_CODE, /*! QUERY ID MAX PUB */
    SM_SUCCESS = 0x00, /*! NORMAL */
    MAX_SM_ER_PUB = MAX_ERR_CODE, /*! QT_SWITCH_MATRIX MAX PUB */
    OPT_SUCCESS = 0x00, /*! NORMAL */
    NO_SIGN = 0x01, /*! SYSTEM NOT SIGN */
    NO_SUCH_ID = 0x02,
    PRESET_STATE = 0x03,
    MAX_OPT_ER_PUB = MAX_ERR_CODE, /*! QT_OPT_TMNL MAX PUB */
    SSET_SUCCESS = 0x00, /*! NORMAL */
    UPDATE_WRITE_ERR = 0x01,
    DATA_FORMAT_ERR = 0x02,
    START_SYS_ERR = 0x03,
    STOP_SYS_ERR = 0x04,
    NO_SPACE = 0x05,
    TRANSMIT_ERR = 0x06,
    MAX_SYSSET_ER_PUB = MAX_ERR_CODE, /*! QT_SYS_SET MAX PUB */
    CMR_CTL_SUCCESS = 0x00, /*! NORMAL */
    MAX_CMRCTL_ER_PUB = MAX_ERR_CODE, /*! QT_CMR_CTL MAX PUB */
};
/*! Error code define-----------------------------------------------------@}*/
/*!struct of qt protocal with service define */
typedef struct TProtocalQt {
    uint8_t head; /*! protocal head */
    uint8_t type;/*! protocal type */
    uint16_t seq;/*! protocal seqId */
    uint8_t cmd;/*! protocal command */
    uint16_t dataLen;/*! protocal data lenght */
    uint8_t dataBuf[PRO_QT_MAX];/*! protocal databuf */
}__attribute__((packed)) TProtocalQt;
/*! struct of qt protocal with query id data */
typedef struct TProtocalQtQueryData {
    uint16_t id;
    uint8_t rgst:1; /*! register flags */
    uint8_t sign:1; /*! sign flags */
    uint8_t vote:1; /*! vote flag */
    uint8_t select:1; /*! select flag */
    uint8_t grade:1; /*! grade flag */
    uint8_t online:1; /*! online flag */
    uint8_t avbIdentity:2; /*! avb device identify */
    uint8_t permision:4; /*! permision of terminal */
    uint8_t micStatus:4; /*! micphone status */
    uint16_t cnntNum; /*! connect num */
    uint64_t id_1722; /*! 1722 id */
    uint8_t name[64]; /*! name of avb device */
}__attribute__((packed)) TProtocalQtQueryData;
/*$ */
static inline int ProtocalQt_checkCrc(TProtocalQt *pIn, uint8_t rCrc) {
    int ret = -1, i;
    uint8_t crc = 0;
    if ( pIn == (TProtocalQt *)0) {
        return -1;
    }
    
    crc ^= pIn->head;
    crc ^= pIn->type;
    crc ^= (uint8_t)(pIn->seq & 0x00ff);
    crc ^= (uint8_t)((pIn->seq & 0xff00) >> 8);
    crc ^= pIn->cmd;
    crc ^= (uint8_t)(pIn->dataLen & 0x00ff);
    crc ^= (uint8_t)((pIn->dataLen & 0xff00) >> 8);
    for (i = 0; i < pIn->dataLen; i++) {
        crc ^= pIn->dataBuf[i];
    }

    if (rCrc == crc) {
        ret = 0;
    }

    return ret;
}
/*$ */
static inline int ProtocalQt_readCmd(uint8_t const * const pBuf,
    int bufLen, int pos, uint8_t * const cmd)
{    
    if ((bufLen < pos)
          && (cmd != (uint8_t * const)0))
    {
        return -1;
    }
    *cmd = pBuf[pos];
    return 0;
}
/*$ */
static inline int ProtocalQt_Fill(TProtocalQt *pIn,
    int inLen, uint8_t *pBuf, int bufLen)
{
    int i;
    
    uint8_t countCrc;
    if (((inLen + 1) > bufLen) /* include crc */
          || (inLen < PRO_COMMON_LEN)
          || (pIn == (TProtocalQt *)0)
          || (pBuf == (uint8_t *)0)
          || (pIn->dataLen > PRO_QT_MAX))
    { /* out off range */
        return -1;
    }
    memset(pBuf, 0, bufLen);
    pBuf[0] = pIn->head;
    pBuf[1] = pIn->type;
    /* low ahead */
    pBuf[2] = (uint8_t)((pIn->seq & 0x00ff) >> 0U);
    pBuf[3] = (uint8_t)((pIn->seq & 0xff00) >> 8U);
    pBuf[4] = pIn->cmd;
    pBuf[5] = (uint8_t)((pIn->dataLen & 0x00ff) >> 0);
    pBuf[6] = (uint8_t)((pIn->dataLen & 0xff00) >> 8);
    for (i = 0; i < pIn->dataLen; i++) {/* fill data */
        pBuf[i+7] = pIn->dataBuf[i];/* fill data */
    }
    /* count crc */
    countCrc = 0;
    for (i = 0; i < inLen; i++) {
        countCrc ^= pBuf[i]; /* count crc */
    }
    /* fill crc */
    pBuf[inLen] = countCrc;
    /* return include crc data len */
    return (inLen + 1);
}

#ifdef __cplusplus
} /* end extern "C" */
#endif /* __cplusplus */

#endif /* __PROTOCAL_QT_H__ */

