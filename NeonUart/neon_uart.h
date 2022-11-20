/**================================ (c) Andrea Raffin 2022 =========================================
**-------------------------------------------------------------------------------------------------
**  File Description: Header for the simple Uart Parser
**===============================================================================================*/
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
/**=================================================================================================
 **                                 Includes / Defines / Macros
 **===============================================================================================*/
#include <stdint.h>
#ifndef ARDUINO
#include <malloc.h>
#endif
/**=================================================================================================
 **                             Public Typedefs / Constants / Macros
 **===============================================================================================*/
#define MAGIC1 0xEC
#define MAGIC2 0x9D

// These macros are a trick for defining and referencing both MSG ID and MSG Struct Typedef
#define NEON_DEF(NAME,TYPE) NAME ##_t; typedef enum { NAME ##_ID = TYPE } NAME ##_id_t
#define NEON_MSG(NAME) NAME ##_ID, sizeof( NAME ##_t)

// Tweak this to reduce footprint, for small apps you will get away with 16
#define N_MAX_MSG_TYPES 16
// Tweak this to reduce footprint, for small apps you will get away with 64
#define N_MAX_PAYLOAD 64
// For example:
// with 64 types and 256 byte messages, footprint is 288 bytes
// with 16 types and 64 byte messages, footprint is 96 bytes

// TODO: hash table or linked list to avoid allocating the full array for msg callbacks

  typedef enum {
    GOT_NONE = 0,
    GOT_SYNC1 = 1,
    GOT_SYNC2 = 2,
    GOT_FAMILY = 3,
    GOT_ID = 4,
    GOT_LENGTH1 = 5,
    GOT_LENGTH2 = 6,
    GOT_PAYLOAD = 7,
    GOT_CHKA = 8
  } parser_state_t;

  typedef enum {
    NOT_PARSED = 0,
    PARSED = 1
  } parser_flag_t;

  typedef enum {
    HANDLER_CB_RX = 0,
    HANDLER_CB_TX = 1
  } parser_handler_direction_t;

  typedef void (*unhandled_callback_t)(uint8_t type);

  typedef struct
  {
    /* Message structure: 0xEC 0x9D <family> <type> <lenL> <lenH> <...payload...> <crc1><crc2> */
    parser_state_t state;
    uint8_t msg_family;
    uint8_t msg_id;
    uint16_t msg_len;
    uint8_t chka;
    uint8_t chkb;
    uint16_t count;
    uint16_t error_count;
    uint8_t payload[N_MAX_PAYLOAD];
    void *cbs;                          // cb register for Handlers and App callbacks, to be cast to parser_callbacks_t*
    unhandled_callback_t unhandled_cb;  // cb for unhandled function
  } neon_parser_t;

  typedef void (*handler_callback_t)(neon_parser_t *pParser, uint8_t txrx, uint8_t type, uint8_t *payload, uint8_t *output);
  typedef void (*app_callback_t)(void *pv);

  typedef struct
  {
    handler_callback_t h;
    app_callback_t a;
    uint16_t msg_size;
  } parser_callback_t;

  typedef struct
  {
    parser_callback_t cb[N_MAX_MSG_TYPES];
  } parser_callbacks_t;

  /**=================================================================================================
 **                                  Public Functions
 **===============================================================================================*/
  void neon_parser_init(neon_parser_t *pParser);
  void neon_parser_deinit(neon_parser_t *pParser);
  void neon_parser_reset(neon_parser_t *pParser);

  // Application Callbacks
  void neon_parser_set_cb(neon_parser_t *pParser, uint8_t msg_type, app_callback_t cb);
  void neon_parser_set_unhandled_cb(neon_parser_t *pParser, unhandled_callback_t cb);

  // TX
  void neon_define_message(neon_parser_t *pParser, uint8_t msg_type, uint16_t length, app_callback_t cb);
  uint8_t neon_build_message(neon_parser_t *pParser, uint8_t *pay, uint8_t type, uint16_t paylen, uint8_t *msg_out);

  // RX
  parser_flag_t neon_parse_char(neon_parser_t *pParser, uint8_t b);

#ifdef __cplusplus
}  // extern "C"
#endif