/////////////////////////////////////////////
//////              MOCK               //////
/////////////////////////////////////////////

#pragma once
#include_next "stm32f4xx_gpio.h"
#include "fff.h"

#ifdef __cplusplus
 extern "C" {
#endif

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/*  Function used to set the GPIO configuration to the default reset state ****/
DECLARE_FAKE_VOID_FUNC(GPIO_DeInit, GPIO_TypeDef*);

/* Initialization and Configuration functions *********************************/
DECLARE_FAKE_VOID_FUNC(GPIO_Init, GPIO_TypeDef*, GPIO_InitTypeDef*);

DECLARE_FAKE_VOID_FUNC(GPIO_StructInit, GPIO_InitTypeDef*);

DECLARE_FAKE_VOID_FUNC(GPIO_PinLockConfig, GPIO_TypeDef*, uint16_t);

/* GPIO Read and Write functions **********************************************/
DECLARE_FAKE_VALUE_FUNC(uint8_t, GPIO_ReadInputDataBit, GPIO_TypeDef*, uint16_t);

DECLARE_FAKE_VALUE_FUNC(uint16_t, GPIO_ReadInputData, GPIO_TypeDef*);

DECLARE_FAKE_VALUE_FUNC(uint8_t, GPIO_ReadOutputDataBit, GPIO_TypeDef*, uint16_t);

DECLARE_FAKE_VALUE_FUNC(uint16_t, GPIO_ReadOutputData, GPIO_TypeDef*);

DECLARE_FAKE_VOID_FUNC(GPIO_SetBits, GPIO_TypeDef*, uint16_t);

DECLARE_FAKE_VOID_FUNC(GPIO_ResetBits, GPIO_TypeDef*, uint16_t);

DECLARE_FAKE_VOID_FUNC(GPIO_WriteBit, GPIO_TypeDef*, uint16_t, BitAction);

DECLARE_FAKE_VOID_FUNC(GPIO_Write, GPIO_TypeDef*, uint16_t);

DECLARE_FAKE_VOID_FUNC(GPIO_ToggleBits, GPIO_TypeDef*, uint16_t);

/* GPIO Alternate functions configuration function ****************************/
DECLARE_FAKE_VOID_FUNC(GPIO_PinAFConfig, GPIO_TypeDef*, uint16_t, uint8_t);

#ifdef __cplusplus
}
#endif
/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
