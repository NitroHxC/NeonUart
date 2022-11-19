#include "m_uart_parser.h"
#include "m_uart_handlers.h"
#include <malloc.h>
#include <string.h>
//parser_ctx_t ctx;
//parser_ctx_t* pCtx = &ctx;

static void uart_parser_dispatch(parser_ctx_t* pParser);

void uart_parser_reset(parser_ctx_t* pParser)
{
    pParser->state = GOT_NONE;
    pParser->msgclass = -1;
    pParser->msgid = -1;
    pParser->msglen = -1;
    pParser->chka = -1;
    pParser->chkb = -1;
    pParser->count = 0;
}

void uart_parser_init(parser_ctx_t* pParser)
{
    //uart_handlers_init(pParser);
    uart_parser_reset(pParser);
    // pParser->cb = malloc(sizeof(parser_callbacks_t));
}

parser_callbacks_t* uart_parser_get_cbs(parser_ctx_t* pParser)
{
    return &(pParser->cbs);
}

handler_callback_t uart_parser_get_h_cb(parser_ctx_t* pParser, uint8_t msg_type)
{
    if(msg_type >= N_MAX_MSG_TYPES) return 0;
    return ((parser_callbacks_t*)(pParser->cbs))->cb[msg_type].h;
}

app_callback_t uart_parser_get_app_cb(parser_ctx_t* pParser, uint8_t msg_type)
{
    if(msg_type >= N_MAX_MSG_TYPES) return 0;
    return ((parser_callbacks_t*)(pParser->cbs))->cb[msg_type].a;
}

void uart_parser_set_hcb(parser_ctx_t* pParser, uint8_t msg_type, handler_callback_t cb)
{
    if (msg_type >= N_MAX_MSG_TYPES) return;
    parser_callbacks_t* pCb = ((parser_callbacks_t*)(pParser->cbs));
    if (pCb == NULL) return; // if it doesn't exist, msgs have yet to be linked dinamically at startup
    pCb->cb[msg_type].h = cb; // set Handler callback
}

void uart_parser_set_cb(parser_ctx_t* pParser, uint8_t msg_type, app_callback_t cb)
{
    if(msg_type >= N_MAX_MSG_TYPES) return;
    parser_callbacks_t* pCb = ((parser_callbacks_t*)(pParser->cbs));
    if (pCb == NULL) return; // if it doesn't exist, msgs have yet to be linked dinamically at startup
    pCb->cb[msg_type].a = cb; // set App callback
}

void uart_parser_set_unhandled_cb(parser_ctx_t* pParser, unhandled_callback_t cb)
{
    pParser->unhandled_cb = cb; // set unhandled callback
}

void uart_parser_free(parser_ctx_t* pParser)
{
    // free(pParser->cb);
}

void uart_create_message(parser_ctx_t* pParser, uint8_t msg_type, uint16_t length, app_callback_t cb)
{
    parser_callbacks_t* pCb = ((parser_callbacks_t*)(pParser->cbs));
    if (pCb == NULL)
    {
        // Create instance for msg defs
        pParser->cbs = malloc(sizeof(parser_callbacks_t));
        memset(pParser->cbs, 0, sizeof(parser_callbacks_t));
        pCb = ((parser_callbacks_t*)(pParser->cbs));
    }
    pCb->cb[msg_type].msg_size = length;
    pCb->cb[msg_type].a = cb;
}

void uart_message_handler(parser_ctx_t* pParser, uint8_t tx, uint8_t type, uint8_t* payload, uint8_t* output)
{
    parser_callbacks_t* pCb = ((parser_callbacks_t*)(pParser->cbs));
    if (pCb == NULL) return; // uninitialized, we shouldn't be here

    if (tx)
    {
        // Serializer / "packer" 
        build_message((uint8_t*)payload, pCb->cb[type].msg_size, type, output);
    }
    else
    {
        // Parse function / "unpacker"
        app_callback_t cb = pCb->cb[type].a;
        if (cb == NULL) return;

        void* msg = malloc(pCb->cb[type].msg_size);
        memcpy(msg, payload, pCb->cb[type].msg_size);
        cb((void*)msg);  // then call the App cb  
        free(msg);
    }
}

void uart_parser_addchk(parser_ctx_t* pParser, uint8_t b) 
{
    pParser->chka = (pParser->chka + b) & 0xFF;
    pParser->chkb = (pParser->chkb + pParser->chka) & 0xFF;
}

void uart_parse_char(parser_ctx_t* pParser, uint8_t b)
	{
		if (b == MAGIC1) {

			pParser->state = GOT_SYNC1;
		}

		else if (b == MAGIC2 && pParser->state == GOT_SYNC1) {

			pParser->state = GOT_SYNC2;
			pParser->chka = 0;
			pParser->chkb = 0;
		}

		else if (pParser->state == GOT_SYNC2) {

			pParser->state = GOT_CLASS;
			pParser->msgclass = b;
			uart_parser_addchk(pParser, b);
		}

		else if (pParser->state == GOT_CLASS) {

			pParser->state = GOT_ID;
			pParser->msgid = b;
			uart_parser_addchk(pParser, b);
		}

		else if (pParser->state == GOT_ID) {

			pParser->state = GOT_LENGTH1;
			pParser->msglen = b;
			uart_parser_addchk(pParser, b);
		}

		else if (pParser->state == GOT_LENGTH1) {

			pParser->state = GOT_LENGTH2;
			pParser->msglen += (b << 8);
			pParser->count = 0;
			uart_parser_addchk(pParser, b);
		}

		else if (pParser->state == GOT_LENGTH2) {

			uart_parser_addchk(pParser, b);
			pParser->payload[pParser->count] = b;
			pParser->count += 1;

			if (pParser->count == pParser->msglen) {

				pParser->state = GOT_PAYLOAD;
			}
		}

		else if (pParser->state == GOT_PAYLOAD) {

			pParser->state = (b == pParser->chka) ? GOT_CHKA : GOT_NONE;
		}

		else if (pParser->state == GOT_CHKA) {

			if (b == pParser->chkb) {
				uart_parser_dispatch(pParser);
			}

			else {
				pParser->state = GOT_NONE;
			}
		}
	}

void uart_report_unhandled(parser_ctx_t* pParser, uint8_t msgid)
{
    if (pParser->unhandled_cb != NULL)
    {
        pParser->unhandled_cb(msgid);
    }
}

static void uart_parser_dispatch(parser_ctx_t* pParser) 
{
    // Dispatch TO HANDLER to unpack data
    parser_callbacks_t* pCb = ((parser_callbacks_t*)(pParser->cbs));
    if (pCb == NULL) return; // uninitialized, we shouldn't be here

    if (pCb->cb[pParser->msgid].msg_size == 0)
    {
        uart_report_unhandled(pParser, pParser->msgid);
        return;
    }

    uart_message_handler(pParser, CBH_RX, pParser->msgid, pParser->payload, NULL);
#if 0
    // Get Handler callback ptr
    handler_callback_t pCallback = uart_parser_get_cbs(pParser)->cb[pParser->msgid].h;

    if(pCallback != NULL)
    {
        pCallback(pParser, CBH_RX, pParser->msgid, pParser->payload, NULL);
    }
    else
    {
        uart_report_unhandled(pParser, pParser->msgid);
    }
#endif
}


void uart_parser_checksum(uint8_t* packet, uint16_t size, uint16_t offset, uint8_t* ans)
{
    uint8_t a = 0x00;
    uint8_t b = 0x00;
    uint16_t i = offset;
    while (i < size)
    {
        a += packet[i++];
        b += a;
    }

    ans[0] = (uint8_t)(a & 0xFF);
    ans[1] = (uint8_t)(b & 0xFF);
}

uint8_t build_message(uint8_t* pay, uint16_t paylen, uint8_t type, uint8_t* msg_out)
{
    memset(msg_out, 0, paylen+7);
    uint8_t poos = 0;
    msg_out[poos++] = MAGIC1;
    msg_out[poos++] = MAGIC2;
    msg_out[poos++] = 0x80; // Class, dummy
    msg_out[poos++] = type; // MSG/CMD ID
    msg_out[poos++] = paylen; // Len
    msg_out[poos++] = paylen >> 8;

    uint8_t jh = 0;
    while (jh < paylen)
    {
        msg_out[poos++] = pay[jh];
        jh++;
    }

    uint8_t answ[2];
    uart_parser_checksum(msg_out, poos, 2, answ);
    msg_out[poos++] = answ[0];
    msg_out[poos++] = answ[1];

    return poos;
}

uint8_t uart_build_message(parser_ctx_t* pParser, uint8_t* pay, uint16_t paylen, uint8_t type, uint8_t* msg_out)
{
    // Dispatch TO HANDLER to unpack data
    uart_message_handler(pParser, CBH_TX, type, pay, msg_out);
#if 0
    // Get callback ptr
    handler_callback_t pCallback = uart_parser_get_cbs(pParser)->cb[pParser->msgid].h;

    if (pCallback != NULL)
    {
        pCallback(pParser, CBH_TX, pParser->msgid, pParser->payload, msg_out);
    }
    else
    {
        uart_report_unhandled(pParser, pParser->msgid);
    }
#endif
}
