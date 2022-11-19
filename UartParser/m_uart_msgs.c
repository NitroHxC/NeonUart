// TODO: THIS MUST BE A GENERATED FILE!

#include "m_uart_msgs.h"
#include "m_uart_parser.h"
#include "m_uart_config.h"



//
//#define DEF_MSG(I, X) void X ##_handler(uint8_t type, uint8_t* payload)    \
//{                                                   \
//    parser_callback_t cb = cb_map[type];            \
//    if(cb_map[type] == NULL) return;                \
//    X ##_t msg = {0};                               \
//    memcpy(&msg, payload, sizeof(X ##_t));          \
//    cb(type, &msg);                                 \
//}                                                   \
//
//
//DEF_MSG(msg_override_setpoint);



// All that is needed for each Message is
//  - Struct definition
//  - Type id 
//  - the handler function


//#define XMESSAGE_MSG_TYPE 0x45
//typedef struct
//{
//    float f1;
//    int i2;
//} XMESSAGE_t;
#if 0
void XMESSAGE_handler(parser_ctx_t* pParser, uint8_t tx, uint8_t type, uint8_t* payload, uint8_t* output)
{
    if (tx)
    {
        // Serializer / "packer" 
        build_message((uint8_t*)payload, sizeof(XMESSAGE_t), type, output);
    }
    else
    {
        // Parse function / "unpacker"
        app_callback_t cb = uart_parser_get_cbs(pParser)->cb[type].a;
        if (cb == NULL) return;

        XMESSAGE_t* msg = malloc(sizeof(XMESSAGE_t));
        memcpy(msg, payload, sizeof(XMESSAGE_t));
        cb((void*)msg);  // then call the App cb  
        free(msg);
    }
}

void ZMESSAGE_handler(parser_ctx_t* pParser, uint8_t tx, uint8_t type, uint8_t* payload, uint8_t* output)
{
    if (tx)
    {
        // Serializer / "packer" 
        build_message((uint8_t*)payload, sizeof(ZMESSAGE_t), type, output);
    }
    else
    {
        // Parse function / "unpacker"
        app_callback_t cb = uart_parser_get_cbs(pParser)->cb[type].a;
        if (cb == NULL) return;

        ZMESSAGE_t* msg = malloc(sizeof(ZMESSAGE_t));
        memcpy(msg, payload, sizeof(ZMESSAGE_t));
        cb((void*)msg);  // then call the App cb  
        free(msg);
    }
}

#endif
void uart_msgs_init()
{

}
