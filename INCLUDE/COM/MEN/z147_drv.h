/***********************  I n c l u d e  -  F i l e  ***********************/
/*!
 *        \file  z147_drv.h
 *
 *      \author  APatil
 *        $Date: 2009/08/03 16:43:29 $
 *    $Revision: 2.4 $
 *
 *       \brief  Header file for Z147 driver containing
 *               Z147 specific status codes and
 *               Z147 function prototypes
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

#ifndef _Z147_DRV_H
#define _Z147_DRV_H

#ifdef __cplusplus
      extern "C" {
#endif


/*-----------------------------------------+
|  TYPEDEFS                                |
+-----------------------------------------*/


/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/
/** \name Z147 specific Getstat/Setstat standard codes
 *  \anchor rx_getstat_setstat_codes
 */
/**@{*/
#define Z147_SET_SIGNAL          M_DEV_OF+0x01    /**<   S: Set signal sent on IRQ  */
#define Z147_CLR_SIGNAL          M_DEV_OF+0x02    /**<   S: Uninstall signal        */
#define Z147_RX_DATA_LEN         M_DEV_OF+0x03    /**< G  : Received data length.  */
#define Z147_RX_RXC_IRQ_STAT     M_DEV_OF+0x04    /**< G  : Get RXC IRQ status. */
#define Z147_RX_ERR_IRQ_STAT     M_DEV_OF+0x05    /**< G  : Get RLS (error) IRQ status. */
#define Z147_RX_DATA_RATE        M_DEV_OF+0x06    /**< G,S: Get/Set RX_LCR RX_SPEED. */
#define Z147_ERR_WE              M_DEV_OF+0x07    /**< G,S: Get/Set RX_LCR RX error write enable. */
#define Z147_RX_THR_LEV          M_DEV_OF+0x08    /**< G,S: Get/Set RX_FCR RX threshold level. */
#define Z147_RX_IN_SYNC			 M_DEV_OF+0x09    /**< G  : Get IS driver in sync. */
#define Z147_DISABLE_RX			 M_DEV_OF+0x0A	  /**<   S: Set Disable reception. */
#define Z147_SET_ERR_SIGNAL		 M_DEV_OF+0x0B	  /**<   S: Set RX error signal. */
#define Z147_CLR_ERR_SIGNAL		 M_DEV_OF+0x0C	  /**<   S: Clear RX error signal. */
#define Z147_RX_SYNC_CFG		 M_DEV_OF+0x0D	  /**< G,S: Configure synchronization mode. */
#define Z147_RX_MODE_CFG		 M_DEV_OF+0x0E	  /**< G,S: Configure Receive mode. */

/**@}*/
#define _Z147_GLOBNAME(var,name) var##_##name

#ifndef _ONE_NAMESPACE_PER_DRIVER_
  #define Z147_GLOBNAME(var,name)    _Z147_GLOBNAME(var,name)
#else
  #define Z147_GLOBNAME(var,name)    _Z147_GLOBNAME(Z147,name)
#endif

#define __Z147_GetEntry    Z147_GLOBNAME(Z147_VARIANT, GetEntry)

/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
#ifdef _LL_DRV_
#ifndef _ONE_NAMESPACE_PER_DRIVER_
	extern void __Z147_GetEntry(LL_ENTRY* drvP);
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

#endif /* _Z147_DRV_H */

