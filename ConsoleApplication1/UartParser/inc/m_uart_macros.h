#pragma once

#define MAP_CB(X) X ##_MSG_TYPE , X ##_cb

#define MESSAGE_CB(X, Z) void X ##_cb(void* pv) { \
                            X ##_t* msg = (X ##_t*)pv; \
                            if(Z != NULL) memcpy(Z, msg, sizeof(X ##_t)); \

#define END_MESSAGE_CB } \

#define COPY_TO(X) memcpy(X, msg, sizeof())