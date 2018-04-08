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
/*                     Internal CAN Mode Definitions                          */
/******************************************************************************/
#define DEBUG_MODE				6
#define LOOPBACK_SILENT_MODE	5
#define SILENT_MODE		    	4
#define LOOPBACK_MODE       	3
#define SLEEP_MODE				2
#define NORMAL_MODE         	1
#define INIT_MODE				0
/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/*                     Transmit FIFO States                                    */
/******************************************************************************/

//From Page 1359 of the Reference Manual:

	//"In order to transmit a message, the application must select one empty 
	//transmit mailbox, setup the identifier, the data length code (DLC) and 
	//the data before requesting the transmission by setting the corresponding 
	//TXRQ bit in the CAN_TIxR register. Once the mailbox has left empty state,
	//the software no longer has write access to the mailbox registers. 
	//Immediately after the TXRQ bit has been set, the mailbox enters pending 
	//state and waits to become the highest priority mailbox"


#define EMPTY				0
#define PENDING				1
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

#define EMPTY				0
#define PENDING_1			1
#define PENDING_2			2
#define PENDING_3			3
/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/*                      Module SIDs  (http://bit.ly/2iQw5cs)                  */
/******************************************************************************/



/******************************************************************************/
/******************************************************************************/

//CAN message struct as defined in Receive FIFO description in proc/stm32______.h
struct CAN_MESSAGE{

	uint32_t id;
	uint32_t dlc_time_stamp;
	uint32_t data_high;
	uint32_t data_low;
	
} CAN_MESSAGE;

/******************************************************************************/
/*                                 Functions                                  */
/******************************************************************************/
void CAN_init(void);
int CAN_check_error(void);
void setupBroadcast(void);
void setupMessage(uint16_t SID);

inline void handleCANbco(void);
inline void handleCANmo(void);

bool CAN_send(void);
bool CAN_broadcast(void);

bool CAN_receive_broadcast(void);
bool CAN_receive_specific(void);

void CAN_send_fault(void);
void CAN_send_heartbeat(bool fake);

void CAN_message_dump(CAN_MESSAGE *message, bool outgoing);
void CAN_print_errors(void);
bool CAN_ping(uint16_t SID, bool initiator);
void CAN_print_errors(void);
/******************************************************************************/
/******************************************************************************/
#endif


