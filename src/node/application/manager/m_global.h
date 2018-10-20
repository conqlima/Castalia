#ifndef _GLOBAL_H_
#define _GLOBAL_H_

extern "C" {
#include "communication/context.h"
#include <time.h>
#include "api/api_definitions.h"
#include "communication/plugin/plugin.h"
#include "communication/service.h"
}

/**
 * plugin used by agent to send network data
 */
extern ContextId m_CONTEXT_ID;

/**
 * PLugin definition
 */
extern CommunicationPlugin m_comm_plugin[11];

/**
 * Variable used by the stack
 */
extern unsigned long long m_port;

/**
 * Struct to represent the messages exchanged
 */
extern Tmsg m_st_msg[11];

void m_timer_reset_timeout(Context *ctx);
int m_timer_count_timeout(Context *ctx);
void device_associated(Context *ctx, DataList *list);
void m_device_unavailable(Context *ctx);
void device_reqmdsattr();
void new_data_received(Context *ctx, DataList *list);
void print_device_attributes(Context *ctx, Request *r, DATA_apdu *response_apdu);
void m_castalia_mode(unsigned int my_plugin_number);

#endif
