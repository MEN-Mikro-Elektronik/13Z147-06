/***********************  I n c l u d e  -  F i l e  ***********************/
/*!
 *        \file  z247_drv.h
 *
 *      \author  APatil
 *        $Date: 2009/08/03 16:43:29 $
 *    $Revision: 2.4 $
 *
 *       \brief  Header file for Z247 driver containing
 *               Z247 specific status codes and
 *               Z247 function prototypes
 *
 *    \switches  _ONE_NAMESPACE_PER_DRIVER_
 *               _LL_DRV_
 */
 /*-------------------------------[ History ]--------------------------------
 *
 * $Log: z17_drv.h,v $
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2004 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

#ifndef _Z247_DRV_H
#define _Z247_DRV_H

#ifdef __cplusplus
      extern "C" {
#endif


/*-----------------------------------------+
|  TYPEDEFS                                |
+-----------------------------------------*/
/* structure for the Z247_BLK_IRQLAT_X setstat/getstat */
typedef struct {
	/* in */
	u_int32 outPort;    /* output port (0..31) */
	u_int32 inPort;     /* input port (0..31) */
	u_int32 irqs2fire;  /* number of interrupts to fire */
	/* out */
	u_int32 tickRate;   /* tick rate from OSS_TickRateGet */
	u_int32 startTick;  /* start tick count */
	u_int32 stopTick;   /* stop tick count */	
	u_int32 irqsRcved;  /* received IRQs */
} Z247_BLK_IRQLAT;

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/
/** \name Z247 specific Getstat/Setstat standard codes
 *  \anchor tx_getstat_setstat_codes
 */
/**@{*/
#define Z247_SET_PORTS           M_DEV_OF+0x00    /**<   S: Set IO ports without affecting others - 8 bit mask - bit 0 for gpio[0] */
#define Z247_CLR_PORTS           M_DEV_OF+0x01    /**<   S: Clear IO ports without affecting others - 8 bit mask - bit 0 for gpio[0] */
#define Z247_DIRECTION           M_DEV_OF+0x02    /**< G,S: Get/Set direction of all ports - 8bit - bit 0 for gpio[0] - value 0 is in - 1 is out  */
#define Z247_IRQ_SENSE           M_DEV_OF+0x03    /**< G,S: Get/set IRQ sense mode for models Z34/Z37 - 16 bit - and for model Z127 - 32 bit - bit 0 and 1 for gpio[0] - 0 no, 1 rising, 2 falling, 3 both edges */
#define Z247_OPEN_DRAIN          M_DEV_OF+0x04    /**< G,S: Get/set open drain mode - 8bit - bit 0 for gpio[0] - value 0 is dis - 1 is enable */
#define Z247_DEBOUNCE            M_DEV_OF+0x05    /**< G,S: Get/set debouncing of all ports - 8bit - bit 0 for gpio[0] - value 0 is dis - 1 is enable */
#define Z247_SET_SIGNAL          M_DEV_OF+0x06    /**<   S: Set signal sent on IRQ  */
#define Z247_CLR_SIGNAL          M_DEV_OF+0x07    /**<   S: Uninstall signal        */
#define Z247_IRQ_LAST_REQUEST    M_DEV_OF+0x08    /**< G  : Get last IRQ request - 8bit - bit 0 for gpio[0] - value 1 is requested - clears the last request  */
#define Z247_TX_TXCIEN_STAT      M_DEV_OF+0x09    /**< G  : Get TX IER IRQ status. */
#define Z247_LOOPBACK		     M_DEV_OF+0x0A    /**< G,S: Get/Set Loop back mode. */
#define Z247_TX_DATA_RATE	     M_DEV_OF+0x0B    /**< G,S: Get/Set data rate of TX. */

#define Z247_TX_MODE_HARDVARD    M_DEV_OF+0x0C    /**< G,S: Get/Set TX_LCR TX_SPEED. */
#define Z247_TX_MODE_BIPOLAR     M_DEV_OF+0x0D    /**< G,S: Get/Set TX_LCR TX_SPEED. */
#define Z247_TX_THR_LEV          M_DEV_OF+0x0E    /**< G,S: Get/Set TX_FCR TX threshold level. */
#define Z247_DISABLE_TX          M_DEV_OF+0x0F	  /**< G,S: Set disable transmission. */
#define Z247_SET_ERR_SIGNAL      M_DEV_OF+0x10    /**<   S: Set signal for TX error */
#define Z247_CLR_ERR_SIGNAL      M_DEV_OF+0x11    /**<   S: Clear signal for TX error */


/* Z17 specific Getstat/Setstat block codes (for test purposes) */
#define Z247_BLK_IRQLAT_START    M_DEV_BLK_OF+0x00    /*   S: Initialize IRQ latency test */
#define Z247_BLK_IRQLAT_RESULT   M_DEV_BLK_OF+0x01    /* G  : Get result of IRQ latency test */

/**@}*/

#ifndef  Z247_VARIANT
  #define Z247_VARIANT    Z17
#endif

#define _Z247_GLOBNAME(var,name) var##_##name

#ifndef _ONE_NAMESPACE_PER_DRIVER_
  #define Z247_GLOBNAME(var,name)    _Z247_GLOBNAME(var,name)
#else
  #define Z247_GLOBNAME(var,name)    _Z247_GLOBNAME(Z17,name)
#endif

#define __Z247_GetEntry    Z247_GLOBNAME(Z247_VARIANT, GetEntry)

/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
#ifdef _LL_DRV_
#ifndef _ONE_NAMESPACE_PER_DRIVER_
	extern void __Z247_GetEntry(LL_ENTRY* drvP);
#endif
#endif /* _LL_DRV_ */

/*-----------------------------------------+
|  BACKWARD COMPATIBILITY TO MDIS4         |
+-----------------------------------------*/
#ifndef U_INT32_OR_64
  /* we have an MDIS4 men_types.h and mdis_api.h included */
  /* only 32bit compatibility needed!                     */
  #define INT32_OR_64    int32
  #define U_INT32_OR_64  u_int32
  typedef INT32_OR_64    MDIS_PATH;
#endif /* U_INT32_OR_64 */

#ifdef __cplusplus
      }
#endif

#endif /* _Z247_DRV_H */

