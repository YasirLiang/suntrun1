/*
* @file protocol_structure.h
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
#ifndef __PROTOCOL_STRUCTURE_H__
#define __PROTOCOL_STRUCTURE_H__

/*! c function decleration */
#ifdef __cplusplus /* for c++ transfer */
    extern "C" {
#endif /* __cplusplus */

/*! the virtual table of structure protocol */
typedef struct TProtocolStrVtbl {
    /*! create protocol packet */
    int (*create)(void const * const, int const, void * const, int const);
    
    /*! protocol parser(basing on state-machine or not) */
    int (*parser)(void const * const, int const, void * const, int const);
    
    /*! set protocal type(version of protocol) for parser and creation */
    int (*setType)(int const);
}TProtocolStrVtbl;

/*$ ProtocolStruct_create().................................................*/
int ProtocolStruct_create(TProtocolStrVtbl const * const vptr,
    void const * const strElem, int const inLen,
    void * const strResult, int const limitLen);

/*$ ProtocolStruct_parser().................................................*/
int ProtocolStruct_parser(TProtocolStrVtbl const * const vptr,
    void const * const parserElem, int const inLen,
    void * const parserResult, int const limitLen);

/*$ ProtocolStruct_setType()................................................*/
int ProtocolStruct_setType(TProtocolStrVtbl const * const vptr,
    int const pVersion);

#ifdef __cplusplus
} /* end extern "C" */
#endif /* __cplusplus */

#endif /* __PROTOCOL_STRUCTURE_H__ */

