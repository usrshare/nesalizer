#ifndef DBG_H
#define DBG_H

int reset_debugger(void);
int set_debugger_vis(bool vis);

//returns 1 if execution may resume and 0 otherwise.
int dbg_log_instruction(void);
#endif
