#ifndef _CAN__H__
#define _CAN__H__

#include <stdint.h>
#include <stdbool.h>
#include "system.h"

/*
 * CAN1: Pin , Pin
 * CAN2: Pin , Pin
 */

/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/*                     Receive FIFO States                                    */
/******************************************************************************/
//State value checked against the FPM[1:0] bits in the CAN_RFR Register

//Overflow occurs when another message arrives while Receive FIFO is in 
//PENDING_3 state. If Overflow occurs, the FOVR bit in the CAN_RFR register 
//is set.

//After software reads out the received message, it must release the message by
//setting the RFOM bit in the CAN_RFR register.

//There are 2 Receive-related interrupts on pg. 1362 of the Ref. Manual

#define EMPTY			 0
#define PENDING_1			1
#define PENDING_2			2
#define PENDING_3			3
/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/******************************************************************************/

//Definition of CAN_MESSAGE struct
typedef struct  {
  unsigned int   id;                 // 11 bit standard identifier
  unsigned char  data[8];            // Data field
  unsigned char  len;                // Length of data field in bytes
} CAN_MESSAGE;


/******************************************************************************/
/*                                 Functions                                  */
/******************************************************************************/
void CAN_init(void);
void CAN_setInitMode(void);
void CAN_setBitTiming(void);
void CAN_mailboxInit(void);
void CAN_MCRInit(void);
void CAN_filterInit(void);
//void CAN_interruptInit(void);
void CAN_waitReady(void);
void CAN_start(void);

void CAN_WriteMsg(CAN_MESSAGE *msg);
void CAN_ReadMsg0(CAN_MESSAGE *msg);
void CAN_ReadMsg1(CAN_MESSAGE *msg);

void disable_CAN_interrupts(void);

/******************************************************************************/
/******************************************************************************/
#endif


