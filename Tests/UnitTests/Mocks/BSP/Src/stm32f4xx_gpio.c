/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_gpio.h"

DEFINE_FAKE_VOID_FUNC(GPIO_DeInit, GPIO_TypeDef*);

DEFINE_FAKE_VOID_FUNC(GPIO_Init, GPIO_TypeDef*, GPIO_InitTypeDef*);

DEFINE_FAKE_VOID_FUNC(GPIO_StructInit, GPIO_InitTypeDef*);

DEFINE_FAKE_VOID_FUNC(GPIO_PinLockConfig, GPIO_TypeDef*, uint16_t);

DEFINE_FAKE_VALUE_FUNC(uint8_t, GPIO_ReadInputDataBit, GPIO_TypeDef*, uint16_t);

DEFINE_FAKE_VALUE_FUNC(uint16_t, GPIO_ReadInputData, GPIO_TypeDef*);

DEFINE_FAKE_VALUE_FUNC(uint8_t, GPIO_ReadOutputDataBit, GPIO_TypeDef*, uint16_t);

DEFINE_FAKE_VALUE_FUNC(uint16_t, GPIO_ReadOutputData, GPIO_TypeDef*);

DEFINE_FAKE_VOID_FUNC(GPIO_SetBits, GPIO_TypeDef*, uint16_t);

DEFINE_FAKE_VOID_FUNC(GPIO_ResetBits, GPIO_TypeDef*, uint16_t);

DEFINE_FAKE_VOID_FUNC(GPIO_WriteBit, GPIO_TypeDef*, uint16_t, BitAction);

DEFINE_FAKE_VOID_FUNC(GPIO_Write, GPIO_TypeDef*, uint16_t);

DEFINE_FAKE_VOID_FUNC(GPIO_ToggleBits, GPIO_TypeDef*, uint16_t);

DEFINE_FAKE_VOID_FUNC(GPIO_PinAFConfig, GPIO_TypeDef*, uint16_t, uint8_t);