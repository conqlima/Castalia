#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "communication/context.h"

/**
 * Plugin used by agent to send network data
 */
extern ContextId CONTEXT_ID;

/**
 * PLugin definition
 */
extern CommunicationPlugin comm_plugin;

/**
 * Struct to represent the messages exchanged
 */
extern Tmsg st_msg[11];

void timer_reset_timeout(Context *ctx);
int timer_count_timeout(Context *ctx);
void device_connected(Context *ctx, const char *addr);
void device_unavailable(Context *ctx);
void device_associated(Context *ctx);
void sigalrm(int dummy);
void castalia_mode();

#endif
