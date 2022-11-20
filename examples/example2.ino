#include <Arduino.h>
#include "neon_uart.h"

neon_parser_t g_parser;

uint32_t g_last_tx_time = 0;

/* 1. define the MSG contents and its msg ID using the Macro */
typedef struct {
    float f1;
    int i2;
    char c3;
} NEON_DEF(myMsg1_t, 0x03);

/* 2. define what happens when we receive the message */
void callback_msg1(void* pv)
{
    myMsg1_t msg = { 0 };
    memcpy(&msg, pv, sizeof(myMsg1_t)); // Last chance to copy data out
    Serial.println("Got Msg 1! Data: f1: " + String(msg.f1) + ", i2: " + String(msg.i2) + ", c3: " + String(msg.c3));
    
    // At this point we are going out of scope and the data in `pv` is going to be freed
}

void setup()
{
    Serial.begin(115200); // Debug Serial (to PC's Serial Monitor)
    Serial1.begin(115200); // Serial used for Neon, attached to some other Arduino device

    /* 3. What we defined in steps 1 and 2 is being linked/registered together */
    neon_define_message(&g_parser, NEON_MSG(myMsg1_t), callback_msg1);
}

void loop()
{
    // RX
    while(Serial1.available())
    {
        uint8_t ch = Serial1.read();

        /* 5. Feed the parser with incoming bytes! */
        neon_parse_char(&g_parser, ch);
    }

    // TX every 1 sec
    if(millis() - g_last_tx_time >= 1000)
    {
        g_last_tx_time = millis();

        /* 6. Populate your msg as you like */
        myMsg1_t message_one = {0};
        message_one.f1 = 666.6f;
        message_one.i2 = 999;
        message_one.c3 = 22;

        /* 7. Build the message in an output buffer */
        uint8_t buffer_out[32] = {0};
        uint8_t len_out = neon_build_message(&g_parser, (uint8_t*)&message_one, NEON_MSG(myMsg1_t), buffer_out);

        /* 8. Send it out! */
        Serial1.write(buffer_out, len_out);
    }
}