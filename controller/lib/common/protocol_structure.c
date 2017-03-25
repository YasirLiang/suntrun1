/*
* @file protocol_structure.c
* @brief protocol construction supper class
* @ingroup protocol construction supper class
* @cond
******************************************************************************
* Build Date on  2017-03-24
* Last updated for version 1.0.0
* Last updated on  2017-03-24
*
*                    Moltanisk Liang
*                    ---------------------------
*                    avb auto control system
*
* Copyright (C) Moltanisk Liang, GuangZhou Suntron. All rights reserved.
******************************************************************************
* @endcond
*/

/*$ Including file----------------------------------------------------------*/
#include "protocol_structure.h"

/*$ ProtocolStruct_create().................................................*/
int ProtocolStruct_create(TProtocolStrVtbl const * const vptr,
    void const * const strElem, int const inLen,
    void * const strResult, int const limitLen)
{
    return vptr->create(strElem, inLen, strResult, limitLen);
}

/*$ ProtocolStruct_parser().................................................*/
int ProtocolStruct_parser(TProtocolStrVtbl const * const vptr,
    void const * const parserElem, int const inLen,
    void * const parserResult, int const limitLen)
{
    return vptr->parser(parserElem, inLen, parserResult, limitLen);
}

/*$ ProtocolStruct_setType()................................................*/
int ProtocolStruct_setType(TProtocolStrVtbl const * const vptr,
    int const pVersion)
{
    return vptr->setType(pVersion);
}

