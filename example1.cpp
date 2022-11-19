#include <iostream>
#include <stdint.h>

extern "C" {
#include "NeonUart/neon_uart.h"
}

extern void print_debug(uint8_t* buf, uint16_t len);
void test_parse_buffer(neon_parser_t* parser, uint8_t* buffer, uint16_t maxlen);

/* 0. Define a message/struct */
#define MSG1_ID 0x03
typedef struct {
    float f1;
    int i2;
    char c3;
} myMsg1_t;

#define MSG2_ID 0x09
typedef struct {
    uint16_t u1;
    uint16_t u2;
    uint32_t u3;
    uint32_t u4;
    char c1;
    char c2;
    char c3;
    float f1;
    float f2;
} myMsg2_t;

/* 1. The callback will handle the received data */
void callback_msg1(void* pv)
{
    myMsg1_t msg = { 0 };
    memcpy(&msg, pv, sizeof(myMsg1_t)); // Can copy data here or to a global. After going out of the callback scope, the data will be freed
    std::cout << "We are in the msg 1 callback ! values: " << std::dec << (float)msg.f1 << " , " << (int)msg.i2 << " , " << (int)msg.c3 << std::endl;
}

void callback_msg2(void* pv)
{
    myMsg2_t msg = { 0 };
    memcpy(&msg, pv, sizeof(myMsg2_t)); // Can copy data here or to a global. After going out of the callback scope, the data will be freed
    std::cout << "We are in the msg 2 callback! Too much data to unpack. " << std::endl;
    std::cout << "f1: " << msg.f1 << "f, f2: " << msg.f2 << "f, " << std::hex << "u4: " << msg.u4 << std::endl;
}

/* 2. Callback for unhandled inbound msg types, optional */
void print_unhandled(uint8_t type)
{
    std::cout << "Unhandled type " << std::hex << "0x" << (int)type << std::endl;
}

void example1()
{
    /* 3. create Parser object. */
    neon_parser_t parser1 = neon_parser_t(); // Usually in plain C we actually initialize with = {0}

    /* 4. set optional callback for printing an unhandled Msg ID when received (if needed) */
    neon_parser_set_unhandled_cb(&parser1, print_unhandled);

    /* 5. define messages dynamically:
        Every Msg ID is linked to a struct and to an application-defined callback. 
        The Parser callback array are allocated on the first call to this function */

    neon_define_message(&parser1, MSG1_ID, sizeof(myMsg1_t), callback_msg1);

    neon_define_message(&parser1, MSG2_ID, sizeof(myMsg2_t), callback_msg2);

    /* 6. Initialize object */
    neon_parser_init(&parser1);

    std::cout << "Footprint of the parser: " << std::dec << (int)sizeof(parser1) << " bytes" << std::endl;

    /* 7. Setup is done, enjoy! */

    /* ------------------------ */
    
    /* 8. Test 1 : send and receive msg1 */
    uint8_t buffer_out[N_MAX_PAYLOAD] = { 0 };

    myMsg1_t message_one = myMsg1_t();
    message_one.f1 = 666.6f;
    message_one.i2 = 999;
    message_one.c3 = 22;

    uint8_t len_out = neon_build_message(&parser1, (uint8_t*)&message_one, sizeof(myMsg1_t), 0x03, buffer_out);
    
    /* In a real application, you will likely call (Arduino-style)
    *   Serial.write(buffer_out, len_out); 
    */
    print_debug(buffer_out, len_out);
   
    // parse the built buffer to get the initial data
    test_parse_buffer(&parser1, buffer_out, N_MAX_PAYLOAD);

    /* ------------------------ */

    /* 9. Test 2 : send a message with wrong ID (0x05 instead of 0x03) */
    // Let's test the unhandled callback
    memset(buffer_out, 0, N_MAX_PAYLOAD);
    len_out = neon_build_message(&parser1, (uint8_t*)&message_one, sizeof(myMsg1_t), 0x05, buffer_out);

    // We expect the unhandled callback to be triggered now
    test_parse_buffer(&parser1, buffer_out, N_MAX_PAYLOAD);

    /* ------------------------ */

    /* 10. Test 3 : send and receive msg2 */
    memset(buffer_out, 0, N_MAX_PAYLOAD);

    myMsg2_t message_two = myMsg2_t(); // Usually in plain C we actually initialize with = {0}
    message_two.c1 = 1;
    message_two.c2 = 2;
    message_two.c3 = 3;
    message_two.f1 = 1234.56f;
    message_two.f2 = 789.0123f;
    message_two.u1 = 0xAABB;
    message_two.u2 = 0xDEAD;
    message_two.u3 = 0xBEEF;
    message_two.u4 = 0xCAFE;

    len_out = neon_build_message(&parser1, (uint8_t*)&message_two, sizeof(myMsg2_t), MSG2_ID, buffer_out);

    // Write the buffer out
    print_debug(buffer_out, len_out);

    // See if we can get msg2!
    test_parse_buffer(&parser1, buffer_out, N_MAX_PAYLOAD);

    /* BYE BYE NEON! */
    neon_parser_deinit(&parser1);
}

void test_parse_buffer(neon_parser_t* parser, uint8_t* buffer, uint16_t maxlen)
{
    /* In a real application, the for loop below will be more like this: 
    *   (Arduino style)
    * 
    *   while(Serial.available()) 
    *   { 
    *       uint8_t ch = Serial.read();
    *       neon_parse_char(parser, ch);
    *   }
    */
    for (uint16_t i = 0; i < maxlen; i++)
    {
        // The function also returns 1 when successfully parsed.
        // However it's optional, since we already have the callback.
        if (neon_parse_char(parser, buffer[i]))
        {
            break;
        }
    }
}
