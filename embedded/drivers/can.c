#include "../include/can.h"

/******************************************************************************/
/*                                Local Variables                             */
/******************************************************************************/
static volatile unsigned int numOverflows = 0;
static volatile unsigned int timeOfLastMessage = 0;
static unsigned int *receivePointer;

static unsigned int lastSentTimeout = 0, currTime = 0;

/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/*                              Initialization                                */
/******************************************************************************/


void CAN_fifo_init(void) {
//    memset((void *) fifos, 0, sizeof(CAN_MESSAGE) * FIFO_SIZE);
     

////////////////////Transfer Mailboxes (3) CAN_TIxR {x = 0,1,2}//////////////////////////////////////////////////
//BITS 31-21 are Standard ID bits
//Bit 2 is the standard id/ext  (0)
//Bit 1 is remote frame/data frame (0)
//Bit 0 is the transmit request set by software, cleared by hardware when sent

//Data Registers: 
	//High Byte: CAN_TDHxR {x = 0,1,2}
	//Low Byte: CAN_TDLxR {x = 0,1,2}


///////////////////Receive Mailboxes (2) CAN_RIxR {x = 0,1}/////////////////////////
//Bits 31-21 are standard id
//Bit 2 is standard id/ext id (0)
//Bit 1 is remote frame/data frame (0)
//Bit 0 reserved

//Data Registers: 
	//High Byte: CAN_RDHxR {x = 0,1}
	//Low Byte: CAN_RDLxR {x = 0,1}







 
    // do not use extended identifier
    
    // filter 0 used for FIFO 0
    // filter 0 uses mask 0
    
    // filter 1 used for FIFO 1
    // filter 1 uses mask 1
   
    // enable filter 0
    // enable filter 1 
   
}

void CAN_set_up_interrupts(void) {
    CAN_SFR(INTbits, CAN_MAIN).RBIE = 1;
    
    // Disable Interrupts for now
    CAN_SFR(FIFOINT0bits, CAN_MAIN).RXFULLIE = 1;
    CAN_SFR(FIFOINT1bits, CAN_MAIN).RXFULLIE = 1;
    //GLOBAL_RECEIVE_ENABLE = 1;                  // interrupt when not empty
    //ADDRESSED_RECEIVE_ENABLE = 1;               // interrupt when not empty        
    
    // enable interrupts globally
#if CAN_MAIN == 1
    IEC1bits.CAN1IE = 1;
    IPC11bits.CAN1IP = 3;
    IPC11bits.CAN1IS = 0;
#elif CAN_MAIN == 2
    IEC1bits.CAN2IE = 1;
    IPC11bits.CAN2IP = 3;
    IPC11bits.CAN2IS = 0;
#endif
    MAIN_CAN_FLAG = 0;
    ALT_CAN_FLAG = 0;
}

// See http://ww1.microchip.com/downloads/en/DeviceDoc/61154C.pdf Bit Timing section
void CAN_set_timings(void) {
    CAN_SFR(CFGbits, CAN_MAIN).SAM = 1;                  // Sample 3 times between SEG1PH and SEG2PH
    CAN_SFR(CFGbits, CAN_MAIN).SEG2PHTS = 1;             // SEG2PH set manually

// Want 16 Tq every 1/250kHz, so FTq = 4MHz
#if defined(BAUD_250K)
    CAN_SFR(CFGbits, CAN_MAIN).SEG1PH = 4;              // + 5 Tq
    CAN_SFR(CFGbits, CAN_MAIN).SEG2PH = 4;              // + 5 Tq
    CAN_SFR(CFGbits, CAN_MAIN).PRSEG = 4;               // + 5 Tq (+ 1 Tq from sync segment) = 16 Tq
    CAN_SFR(CFGbits, CAN_MAIN).SJW = 1;                 // +/- 2 Tq
    CAN_SFR(CFGbits, CAN_MAIN).BRP = 7;                 // = (64 000 000 / (2 * FTq) - 1 = 7

// Want 8 Tq every 1/1MHz, so FTq = 8 MHz
#elif defined(BAUD_1M)
    CAN_SFR(CFGbits, CAN_MAIN).SEG1PH = 2;               // +   3 Tq
    CAN_SFR(CFGbits, CAN_MAIN).SEG2PH = 2;               // +   3 Tq
    CAN_SFR(CFGbits, CAN_MAIN).PRSEG = 0;                // +   1 Tq (+ 1 Tq from sync segment) = 8 Tq
    CAN_SFR(CFGbits, CAN_MAIN).SJW = 1;                  // +/- 2 Tq
    CAN_SFR(CFGbits, CAN_MAIN).BRP = 3;                  // = (64 000 000 / (2 * FTq) - 1 = 3
#endif
}

int CAN_set_mode(int mode) {
    if (mode < 0 || mode > 7) return -1;
    CAN_SFR(CONbits, CAN_MAIN).REQOP = mode;
    while (CAN_SFR(CONbits, CAN_MAIN).OPMOD != mode);
    return 0;
}

void CAN_init(void) {
	
	//TODO Actually write these values to the CAN_MCR Register correctly
	CAN_MCR |= 1<<15; //Master Software Reset
    
	CAN_MCR |= 1; //Set CAN to Initialization Mode

	while( !CAN_MSR->INAK  ) {} //Waits until Hardware signals we are in
								//Initialization mode

	CAN_MCR |= 1<< 6; //Automatic Bus-Off Management
	CAN_MCR |= 1<<5; //Automatic Wakeup Mode
	CAN_MCR |= 1<<4; //Non-Automatic Retransmission ?????
	CAN_MCR |= 1<<2; //Transmit Priority

	CAN_MCR &= (~0x1); //Sets CAN to Normal Mode

	//TODO finish rest of configuration
	/*
	CAN_set_mode(CONFIG_MODE);
    CAN_set_timings();
    CAN_fifo_init();
    CAN_set_up_interrupts();
    CAN_set_mode(NORMAL_MODE);
	*/
}
/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/*                                Utility                                     */
/******************************************************************************/
inline void setupBroadcast(void) {
    sending = BROADCAST_SEND_ADDR;
    sending->SID = ALL;
    sending->from = ourRole;
}

inline void setupMessage(uint16_t SID) {
    sending = ADDRESSED_SEND_ADDR;
    sending->SID = SID;
    sending->from = ourRole;
}

int CAN_check_error(void) { return CAN_SFR(TREC, CAN_MAIN); }

uint16_t ROLEtoSID(ROLE r) {
    switch (r) {
        case VNM: return VNM_SID;
        case WCM: return WCM_SID;
        case BCM: return BCM_SID;
        case VSM: return VSM_SID;
        case MCM: return MCM_SID;
        case BMS: return BMS_SID;
        default: return 0;
    }
}

void check_bus_integrity(void) {
    if (CAN_check_error()) {
        fault = CAN_BUS_ERROR;
        next_state = FAULT_STATE;
    }
    else if (IS_CAN_FAULT) {
        prev_fault = fault;
        fault = HEALTHY;
    }
}
/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/*                         Sending Messages                                   */
/******************************************************************************/

/*
Bits 25:24 CODE[1:0]: Mailbox code
In case at least one transmit mailbox is free, the code value is equal to the number of the
next transmit mailbox free.
In case all transmit mailboxes are pending, the code value is equal to the number of the
transmit mailbox with the lowest priority
*/

//TODO Add checking of status values after sending (page 1376 of ref. manual)

// Returns true/false based on whether or not it's possible to send the message currently
bool CAN_send(void) {
    if (  ) {
        fault = CAN_OUT_FULL_ERROR;
        next_state = fault;
        return false;   // wait until FIFO is not full
    }
    if (debuggingOn) CAN_message_dump(sending, true);
    CAN_SFR(FIFOCON2SET, CAN_MAIN) = 0x2000;     // increment pointer for fifo
    CAN_SFR(FIFOCON2bits, CAN_MAIN).TXREQ = 1;   // tell CAN to send message
    return true;
}

/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/*                        Receiving Messages                                  */
/******************************************************************************/

bool CAN_receive_broadcast(void) {
    if (!GLOBAL_RECEIVE_FLAG) return false;
    receivePointer = BROADCAST_REC_ADDR;
    receiving.raw[0] = receivePointer[0];
    receiving.raw[1] = receivePointer[1];
    receiving.raw[2] = receivePointer[2];
    receiving.raw[3] = receivePointer[3];
    if (debuggingOn) CAN_message_dump(&receiving, false);
    CAN_SFR(FIFOCON0SET, CAN_MAIN) = 0x2000;
    return true;
}

bool CAN_receive_specific(void) {
    if (!ADDRESSED_RECEIVE_FLAG) return false;
    receivePointer = ADDRESSED_REC_ADDR;
    receiving.raw[0] = receivePointer[0];
    receiving.raw[1] = receivePointer[1];
    receiving.raw[2] = receivePointer[2];
    receiving.raw[3] = receivePointer[3];
    if (debuggingOn) CAN_message_dump(&receiving, false);
    CAN_SFR(FIFOCON1SET, CAN_MAIN) = 0x2000;
    return true;
}
/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/*                          Heartbeat Related                                 */
/******************************************************************************/
inline void handleCANbco(void) {
    if (!CAN_broadcast()) fault = CAN_OUT_FULL_ERROR;
}

inline void handleCANmo(void) {
    if (!CAN_send()) fault = CAN_OUT_FULL_ERROR;
}

inline void load_state(void) {
    sending->byte0 = prev_fault;
    sending->byte1 = fault;
    sending->byte2 = prev_state;
    sending->byte3 = state;
    sending->byte4 = next_state;
}

void CAN_send_fault(void) {
    currTime = TMR45;
    if (CHECK_SEND_CAN_TO) {
        lastSentTimeout = currTime;
        setupBroadcast();
        sending->SIZE = 6;
        sending->message_num = FAULT;
        load_state();
        handleCANbco();
    }
}

void CAN_send_heartbeat(bool fake) {
    sending = BROADCAST_SEND_ADDR;
    sending->SID = ALL;
    sending->from = fake ? WCM : ourRole;
    sending->SIZE = 6;
    sending->message_num = HEARTBEAT;
    load_state();
    if (fake) heartbeatsReceived = 2; // since we bypassed WCM
    handleCANbco();
}
/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/*                                    ISRs                                    */
/******************************************************************************/
void __ISR (MAIN_CAN_VECTOR, IPL1SOFT) MAIN_CAN_Interrupt(void) {
    if (CAN_MAIN_VECTOR_BITS.ICODE == 0) {
        checkBroadcasts();
        fault = CAN_IN_FULL_ERROR;
    }
    else if (CAN_MAIN_VECTOR_BITS.ICODE == 1) {
        checkMessages();
        fault = CAN_IN_FULL_ERROR;
    }
    CAN_SFR(INTbits, CAN_MAIN).RBIF = 0;
    MAIN_CAN_FLAG = 0;
}

void __ISR (ALT_CAN_VECTOR, IPL1SOFT) ALT_CAN_Interrupt (void) {
    ALT_CAN_FLAG = 0;
}
/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/*        Compiled depending on availablity of Serial Debug                   */
/******************************************************************************/
void CAN_print_errors(void) {
    if (CAN_SFR(TREC, CAN_MAIN)) printf("CAN ERROR: ");
    else printf("No errors ");
    if (CAN_SFR(TRECbits, CAN_MAIN).TXBO) printf("TXBO ");
    else if (CAN_SFR(TRECbits, CAN_MAIN).TXBP) printf("TXBP ");
    else if (CAN_SFR(TRECbits, CAN_MAIN).TXWARN) printf("TXW ");
    if (CAN_SFR(TRECbits, CAN_MAIN).RXBP) printf("RXBP ");
    else if (CAN_SFR(TRECbits, CAN_MAIN).RXWARN) printf("RXW ");
    printf("(TX:%3d RX:%3d)", CAN_SFR(TRECbits, CAN_MAIN).TERRCNT, CAN_SFR(TRECbits, CAN_MAIN).RERRCNT);
    printf("\r\n");
}

void CAN_message_dump(CAN_MESSAGE *message, bool outgoing) {
    int i;
    if (outgoing && (message->SID & ALL))   printf("BO: ");
    else if (outgoing)                      printf("MO: ");
    else if (message->SID & ALL)            printf("BI: ");
    else                                    printf("MI: ");                                    
    printf("0x%3x from %3s ", message->SID, roleStr[message->from]);
    printf(" %2d (%u bytes): %s ", message->message_num, message->SIZE - 1, messageStr[message->message_num]);
    if ((message->message_num == FAULT || message->message_num == HEARTBEAT) && message->SIZE == 6) 
        printf("[%.5s][%.5s] [%.5s][%.5s][%.5s]", faultStr[message->byte0], faultStr[message->byte1], 
                stateStr[message->byte2], stateStr[message->byte3], stateStr[message->byte4]);
    else if (message->message_num == PING_BACK || message->message_num == SOFTWARE_VER)
        printf("\t[%s]", message->bytes);
    else for (i = 0; i < message->SIZE - 1; i++) printf("[0x%2x] ", message->bytes[i]);
    printf("\r\n");
}

bool CAN_ping(uint16_t SID, bool initiator) {
    bool result = true;
    if (SID == ROLEtoSID(ourRole)) {
        if (debuggingOn) printf("can't ping yourself! (you are %s)\r\n", roleStr[ourRole]);
        return;
    }
    (SID == ALL) ? setupBroadcast() : setupMessage(SID);
    sending->SIZE = initiator ? 1 : 8;
    sending->message_num = initiator ?  PING_TO : PING_BACK;
    if (initiator) return (SID == ALL) ? CAN_broadcast() : CAN_send();
    sending->byte0 = timestamp[11];
    sending->byte1 = timestamp[12];
    sending->byte2 = timestamp[14];
    sending->byte3 = timestamp[15];
    sending->byte4 = timestamp[17];
    sending->byte5 = timestamp[18];
    sending->byte6 = '\0';
    result = (SID == ALL) ? CAN_broadcast() : CAN_send();
    setupMessage(SID);
    sending->SIZE = 8;
    sending->message_num = SOFTWARE_VER;
    strcpy(sending->bytes, &timestamp[26]);
    sending->byte6 = '\0';
    return result && ((SID == ALL) ? CAN_broadcast() : CAN_send());
}
/******************************************************************************/
/******************************************************************************/

