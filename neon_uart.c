/**================================ (c) Andrea Raffin 2022 =========================================
**-------------------------------------------------------------------------------------------------
**  File Description: simple uart parser
**===============================================================================================*/
/**=================================================================================================
 **                                           Includes
 **===============================================================================================*/
#include "neon_uart.h"
#include <string.h>
/**=================================================================================================
 **                                 Private Function Prototypes
 **===============================================================================================*/
static void neon_parser_addchk(neon_parser_t* pParser, uint8_t b);
static void neon_parser_checksum(uint8_t* packet, uint16_t size, uint16_t offset, uint8_t* ans);

static parser_callbacks_t* neon_parser_get_cbs(neon_parser_t* pParser);
static void neon_parser_free(neon_parser_t* pParser);

static void neon_parser_dispatch(neon_parser_t *pParser);
static void neon_report_unhandled(neon_parser_t *pParser, uint8_t msg_id);

static uint8_t neon_parser_build_message(uint8_t *pay, uint16_t paylen, uint8_t type, uint8_t *msg_out);
/**=================================================================================================
 **                                      Private Functions
 **===============================================================================================*/

/***************************************************************************************************
 ** Description    : description for a function
 **************************************************************************************************/
static parser_callbacks_t *neon_parser_get_cbs(neon_parser_t *pParser) {
  return (parser_callbacks_t *)(pParser->cbs);
}

static void neon_parser_free(neon_parser_t *pParser) {
  free(pParser->cbs);
}

uint8_t neon_message_handler(neon_parser_t *pParser, uint8_t tx, uint8_t type, uint8_t *payload, uint8_t *output) {
  parser_callbacks_t *pCb = ((parser_callbacks_t *)(pParser->cbs));
  if (pCb == NULL)
    return 0;  // uninitialized, we shouldn't be here

  if (tx) {
    // Outbound TX Serializer / "packer"
    return neon_parser_build_message((uint8_t *)payload, pCb->cb[type].msg_size, type, output);
  } else {
    // Inbound RX Deserializer function / "unpacker"
    app_callback_t cb = pCb->cb[type].a;
    if (cb == NULL)
      return 0;

    void *msg = malloc(pCb->cb[type].msg_size);
    memcpy(msg, payload, pCb->cb[type].msg_size);
    cb((void *)msg);  // then call the App cb
    free(msg);
    return 0;
  }
}

parser_flag_t neon_parse_char(neon_parser_t *pParser, uint8_t b) {
  if (b == MAGIC1 && pParser->state == GOT_NONE) {
    pParser->state = GOT_SYNC1;
  }

  else if (b == MAGIC2 && pParser->state == GOT_SYNC1) {

    pParser->state = GOT_SYNC2;
    pParser->chka = 0;
    pParser->chkb = 0;
  }

  else if (pParser->state == GOT_SYNC2) {

    pParser->state = GOT_FAMILY;
    pParser->msg_family = b;
    neon_parser_addchk(pParser, b);
  }

  else if (pParser->state == GOT_FAMILY) {

    pParser->state = GOT_ID;
    pParser->msg_id = b;
    neon_parser_addchk(pParser, b);
  }

  else if (pParser->state == GOT_ID) {

    pParser->state = GOT_LENGTH1;
    pParser->msg_len = b;
    neon_parser_addchk(pParser, b);
  }

  else if (pParser->state == GOT_LENGTH1) {

    pParser->state = GOT_LENGTH2;
    pParser->msg_len += (b << 8);
    pParser->count = 0;
    neon_parser_addchk(pParser, b);
  }

  else if (pParser->state == GOT_LENGTH2) {
    // Validate length first
    if (pParser->msg_len >= N_MAX_PAYLOAD) {
      pParser->error_count++;
      pParser->state = GOT_NONE;
    } else if (pParser->msg_len == 0) {
      neon_report_unhandled(pParser, pParser->msg_id);
      neon_parser_reset(pParser);
    }
    neon_parser_addchk(pParser, b);
    pParser->payload[pParser->count] = b;
    pParser->count += 1;

    if (pParser->count == pParser->msg_len) {

      pParser->state = GOT_PAYLOAD;
    }
  }

  else if (pParser->state == GOT_PAYLOAD) {

    pParser->state = (b == pParser->chka) ? GOT_CHKA : GOT_NONE;
  }

  else if (pParser->state == GOT_CHKA) {

    if (b == pParser->chkb) {
      neon_parser_dispatch(pParser);
      neon_parser_reset(pParser);
      return PARSED;
    } else {
      pParser->state = GOT_NONE;
      pParser->error_count++;
      neon_parser_reset(pParser);
    }
  }

  return NOT_PARSED;
}

static void neon_report_unhandled(neon_parser_t *pParser, uint8_t msg_id) {
  if (pParser->unhandled_cb != NULL) {
    pParser->unhandled_cb(msg_id);
  }
}

static void neon_parser_dispatch(neon_parser_t *pParser) {
  // Dispatch TO HANDLER to unpack data
  parser_callbacks_t *pCb = ((parser_callbacks_t *)(pParser->cbs));
  if (pCb == NULL)
    return;  // uninitialized, we shouldn't be here

  if (pCb->cb[pParser->msg_id].msg_size == 0) {
    neon_report_unhandled(pParser, pParser->msg_id);
    return;
  }

  neon_message_handler(pParser, HANDLER_CB_RX, pParser->msg_id, pParser->payload, NULL);
}

void neon_parser_addchk(neon_parser_t *pParser, uint8_t b) {
  pParser->chka = (pParser->chka + b) & 0xFF;
  pParser->chkb = (pParser->chkb + pParser->chka) & 0xFF;
}

static void neon_parser_checksum(uint8_t *packet, uint16_t size, uint16_t offset, uint8_t *ans) {
  uint8_t a = 0x00;
  uint8_t b = 0x00;
  uint16_t i = offset;
  while (i < size) {
    a += packet[i++];
    b += a;
  }

  ans[0] = (uint8_t)(a & 0xFF);
  ans[1] = (uint8_t)(b & 0xFF);
}

static uint8_t neon_parser_build_message(uint8_t *pay, uint16_t paylen, uint8_t type, uint8_t *msg_out) {
  memset(msg_out, 0, paylen + 7);
  uint8_t pos = 0;
  msg_out[pos++] = MAGIC1;
  msg_out[pos++] = MAGIC2;
  msg_out[pos++] = 0x80;    // Family ID, dummy
  msg_out[pos++] = type;    // MSG/CMD ID
  msg_out[pos++] = paylen & 0xFF;  // Len
  msg_out[pos++] = (paylen >> 8) & 0xFF;

  uint8_t j = 0;
  while (j < paylen) {
    msg_out[pos++] = pay[j];
    j++;
  }

  uint8_t answ[2];
  neon_parser_checksum(msg_out, pos, 2, answ);
  msg_out[pos++] = answ[0];
  msg_out[pos++] = answ[1];

  return pos;
}

/**=================================================================================================
 **                                       Public Functions
 **===============================================================================================*/
/***************************************************************************************************
 ** Description    : description for a function
 **************************************************************************************************/
void neon_parser_reset(neon_parser_t *pParser) {
  pParser->state = GOT_NONE;
  pParser->msg_family = -1;
  pParser->msg_id = -1;
  pParser->msg_len = -1;
  pParser->chka = -1;
  pParser->chkb = -1;
  pParser->count = 0;
}

void neon_parser_init(neon_parser_t *pParser) {
  neon_parser_reset(pParser);
  pParser->error_count = 0;
}

void neon_parser_deinit(neon_parser_t *pParser) {
  neon_parser_free(pParser);
}

void neon_define_message(neon_parser_t *pParser, uint8_t msg_type, uint16_t length, app_callback_t cb) {
  parser_callbacks_t *pCb = ((parser_callbacks_t *)(pParser->cbs));
  if (pCb == NULL) {
    // First time here, create instance for msg defs
    pParser->cbs = malloc(sizeof(parser_callbacks_t));
    memset((void *)pParser->cbs, 0, sizeof(parser_callbacks_t));
    pCb = ((parser_callbacks_t *)(pParser->cbs));

    // Reset parser here so we don't need to do it in App
    neon_parser_init(pParser);

    if (pCb == NULL)
      return;  // couldn't allocate
  }
  pCb->cb[msg_type].msg_size = length;
  pCb->cb[msg_type].a = cb;
}

uint8_t neon_build_message(neon_parser_t *pParser, uint8_t *pay, uint8_t type, uint16_t paylen, uint8_t *msg_out) {
  // Dispatch TO HANDLER to unpack data
  return neon_message_handler(pParser, HANDLER_CB_TX, type, pay, msg_out);
}

void neon_parser_set_cb(neon_parser_t *pParser, uint8_t msg_type, app_callback_t cb) {
  if (msg_type >= N_MAX_MSG_TYPES)
    return;
  parser_callbacks_t *pCb = ((parser_callbacks_t *)(pParser->cbs));
  if (pCb == NULL)
    return;                  // if it doesn't exist, msgs have yet to be linked dinamically at startup
  pCb->cb[msg_type].a = cb;  // set App callback
}

void neon_parser_set_unhandled_cb(neon_parser_t *pParser, unhandled_callback_t cb) {
  pParser->unhandled_cb = cb;  // set unhandled callback
}