#ifndef DBG_H
#define DBG_H

int reset_debugger(void);
int set_debugger_vis(bool vis);

void dbg_log_instruction(void);

void dbg_kbdinput_cb(bool keystate, int keycode);
#endif
