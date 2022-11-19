// TODO: THIS MUST BE A GENERATED FILE!
#pragma once
#include <stdint.h>
#include <string.h>
#include "m_uart_macros.h"

//#define MAP_CB(X) X ##_MSG_TYPE , X ##_cb
//
//#define MESSAGE_CB(X, Z) void X ##_cb(void* pv) { \
//                            X ##_t* msg = (X ##_t*)pv; \
//                            if(Z != NULL) memcpy(Z, msg, sizeof(X ##_t)); \
//
//#define END_MESSAGE_CB } \
//
//#define COPY_TO(X) memcpy(X, msg, sizeof())

//#define DEF_MSG_H(X, Z) typedef struct { Z } X ##_t; \
//void X ##_handler(uint8_t type, uint8_t* payload)    \
//
//DEF_MSG_H(msg_override_setpoint, 
//    float lat;
//    float lon;
//    );
#if 0
#define XMESSAGE_MSG_TYPE 0x25
typedef struct
{
    float f1;
    int i2;
} XMESSAGE_t;
void XMESSAGE_handler(parser_ctx_t* pParser, uint8_t tx, uint8_t type, uint8_t* payload, uint8_t* output);

#define ZMESSAGE_MSG_TYPE 0x16
typedef struct
{
    char c1;
    float f2;
    int i3;
} ZMESSAGE_t;
void ZMESSAGE_handler(parser_ctx_t* pParser, uint8_t tx, uint8_t type, uint8_t* payload, uint8_t* output);
#endif