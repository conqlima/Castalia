#ifndef SAMPLE_AGENT_COMMON_H_
#define SAMPLE_AGENT_COMMON_H_

//#include "agent.h"

struct mds_system_data;
extern intu8 AGENT_SYSTEM_ID_VALUE[];

//extern int alarms;
//extern ContextId CONTEXT_ID;
//extern CommunicationPlugin comm_plugin;


void *oximeter_event_report_cb();
void *blood_pressure_event_report_cb();
void *weightscale_event_report_cb();
void *glucometer_event_report_cb();
void *thermometer_event_report_cb();

//void timer_reset_timeout(Context *ctx);
//int timer_count_timeout(Context *ctx);
//void device_connected(Context *ctx, const char *addr);
//void device_unavailable(Context *ctx);
//void device_associated(Context *ctx);
//void sigalrm(int dummy);
//void tcp_mode();

struct mds_system_data *mds_data_cb();

#endif
