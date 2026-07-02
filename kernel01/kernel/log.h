#ifndef HNM_LOG_H
#define HNM_LOG_H

#include "types.h"

void hnm_log_init(void);
void hnm_log_write(const char *text);
void hnm_log_write_line(const char *text);
void hnm_log_write_hex32(u32 value);

#endif
