#ifndef __POWER_H__
#define __POWER_H__

#ifdef __cplusplus
extern "C" {
#endif

void create_power_dialog(void);
void start_auto_poweroff_timer(void);
void stop_auto_poweroff_timer(void);
void update_last_activity_time(void);

#ifdef __cplusplus
}
#endif
#endif