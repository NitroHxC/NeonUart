#include "m_uart_handlers.h"
#include "m_uart_msgs.h"
#include "m_uart_parser.h"
#include <malloc.h>

// TODO : automatic code generation

#if 0
void uart_handlers_init(parser_ctx_t* pParser)
{
    // Create instance for msg defs
    pParser->cbs = malloc(sizeof(parser_callbacks_t));
    
    // Link all handlers to msg type

    // handlers that are defined in pParser .c file must be linked to each msg type
    for(int i = 0; i < N_MAX_MSG_TYPES; i++)
    {
        if(uart_parser_get_cbs(pParser)->cb[i].a != NULL)
        {
            uart_parser_get_cbs(pParser)->cb[i].a = 
        }
    }
}

#endif