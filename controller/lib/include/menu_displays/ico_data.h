#ifndef _ICO_DATA_H_
#define _ICO_DATA_H_

typedef enum
{
  ICO_CMR=0,
	ICO_CMR_LOCK,
  ICO_APPLY,
  ICO_APPLY_LIMIT,
  ICO_LOCK,
  ICO_UNLOCK,
  ICO_MIC,
  ICO_SPK,
  ICO_SPKER,
  ICO_SPKER_LIMIT,
  ICO_PROHIBIT,
  ICO_NUM,
}EIcoFlag;

extern const unsigned char ico_data[][32];

#endif
