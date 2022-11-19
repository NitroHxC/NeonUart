// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <stdint.h>

extern "C" {
#include "m_uart_parser.h"
}
uint8_t test_buffer[64] = { 0 };

void print_unhandled(uint8_t type)
{
    std::cout << "Unhandled type " << std::dec << (int)type << std::endl;
    //uint8_t dummy;
}

//MESSAGE_CB(XMESSAGE, test_buffer)
//// Do stuff here
//END_MESSAGE_CB

//
//void cb_xmessage(void* pv)
//{
//    MESSAGE_CALLBACK(XMESSAGE, pv);
// 
//    //XMESSAGE_t* msg = (XMESSAGE_t*)pv;
//    // DO STUFF with unpacked data
//    std::cout << "Override setpoint! " << std::endl;
//
//    // At the end of this scope data is going to be destroyed
//}

typedef struct {
    float f1;
    int i2;
    char c3;
} msg1_t;

void callback03(void* pv)
{
    //msg1_t* msg = (msg1_t*)pv;
    msg1_t msg = { 0 };
    memcpy(&msg, pv, sizeof(msg1_t));
    std::cout << "We are in the callback! values: " << std::dec << (float)msg.f1 << " , " << (int)msg.i2 << " , " << (int)msg.c3 << std::endl;
}

int main()
{
    std::cout << "Hello World!\n";

    parser_ctx_t parser1 = parser_ctx_t();

    uart_parser_set_unhandled_cb(&parser1, print_unhandled);

    uart_create_message(&parser1, 0x03, sizeof(msg1_t), callback03);

    //uart_parser_set_cb(&parser1, MAP_CB(XMESSAGE));

    uart_parser_init(&parser1);

    // Test: build a message
    uint8_t buffer_out[N_MAX_PAYLOAD] = {0};

    msg1_t out = msg1_t();
    out.f1 = 666.6f;
    out.i2 = 999; 
    out.c3 = 22;

    uart_build_message(&parser1, (uint8_t*)&out, sizeof(msg1_t), 0x03, buffer_out);

    for (uint8_t i = 0; i < sizeof(msg1_t); i++)
    {
        std::cout << "0x" << std::hex << (int)buffer_out[i] << ",";
    }

    std::cout << std::endl;
    
    // parse the built buffer to get the initial data
    uart_parser_reset(&parser1);
    uint8_t has_chk = 0;
    for (uint16_t i = 0; i < N_MAX_PAYLOAD; i++)
    {
        uart_parse_char(&parser1, buffer_out[i]);

        std::cout << std::dec << "Char " <<(int)i << " " << std::hex << "0x" << (int)buffer_out[i] << " \t --> status : " << parser1.state << ", msgid " << parser1.msgid << " msgLEN " << parser1.msglen << std::endl;

        if (has_chk)
        {
            if (buffer_out[i] == parser1.chkb)
            {
                std::cout << " PARSED!! " << std::endl;
            }
            break;
        }

        if (parser1.state == GOT_CHKA)
        {
            has_chk = 1;
            
        }
    }

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
