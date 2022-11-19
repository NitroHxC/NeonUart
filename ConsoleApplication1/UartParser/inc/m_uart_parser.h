#pragma once

#include <stdint.h>
#include "m_uart_config.h"
#include "m_uart_handlers.h"
#include "m_uart_msgs.h"

typedef enum {
    GOT_NONE,
    GOT_SYNC1,
    GOT_SYNC2,
    GOT_CLASS,
    GOT_ID,
    GOT_LENGTH1,
    GOT_LENGTH2,
    GOT_PAYLOAD,
    GOT_CHKA
} parser_state_t;

typedef void (*unhandled_callback_t)(uint8_t type);

typedef struct {
/* Message structure: 0xEC 0x9F 0x80 <type> <lenL> <lenH> <...payload...> <crc1><crc2> */
	parser_state_t state;
	uint8_t msgclass;
	uint8_t msgid;
	uint16_t msglen;
	uint8_t chka;
	uint8_t chkb;
	uint16_t count;
	uint8_t payload[64];
    void* cbs; // cb register for Handlers and App callbacks, to be cast to parser_callbacks_t*
    unhandled_callback_t unhandled_cb; // cb for unhandled function
} parser_ctx_t;

typedef void (*handler_callback_t)(parser_ctx_t* pParser, uint8_t txrx, uint8_t type, uint8_t* payload, uint8_t* output);
//typedef void (*handler_callback_t)(uint8_t, uint8_t, uint8_t*, uint8_t*);
//typedef void (*builder_callback_t)(parser_ctx_t* pParser, uint8_t type, uint8_t* payload, uint8_t* output);
typedef void (*app_callback_t)(void* pv);

typedef enum {
    CBH_RX = 0,
    CBH_TX = 1
} parser_handler_direction_t;

typedef struct {
    handler_callback_t h;
    app_callback_t a;
    uint16_t msg_size;
} parser_callback_t;

typedef struct {
    parser_callback_t cb[N_MAX_MSG_TYPES];
} parser_callbacks_t;

//app_callbacks_t* uart_parser_get_app_cbs(parser_ctx_t* pParser);
parser_callbacks_t* uart_parser_get_cbs(parser_ctx_t* pParser);

/* -------- API --------------- */
void uart_parser_init(parser_ctx_t* pParser);
void uart_parser_reset(parser_ctx_t* pParser);

void uart_create_message(parser_ctx_t* pParser, uint8_t msg_type, uint16_t length, app_callback_t cb);

void uart_parser_set_cb(parser_ctx_t* pParser, uint8_t msg_type, app_callback_t cb);
void uart_parser_set_unhandled_cb(parser_ctx_t* pParser, unhandled_callback_t cb);

void uart_parse_char(parser_ctx_t* pParser, uint8_t b);

uint8_t uart_build_message(parser_ctx_t* pParser, uint8_t* pay, uint16_t paylen, uint8_t type, uint8_t* msg_out);
uint8_t build_message(uint8_t* pay, uint16_t paylen, uint8_t type, uint8_t* msg_out);

