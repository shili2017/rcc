#ifndef _SBI_H_
#define _SBI_H_

#include <stdint.h>

#define SBI_SET_TIMER 0
#define SBI_CONSOLE_PUTCHAR 1
#define SBI_CONSOLE_GETCHAR 2
#define SBI_CLEAR_IPI 3
#define SBI_SEND_IPI 4
#define SBI_REMOTE_FENCE_I 5
#define SBI_REMOTE_SFENCE_VMA 6
#define SBI_REMOTE_SFENCE_VMA_ASID 7
#define SBI_SHUTDOWN 8

void set_timer(uint64_t timer);
void console_putchar(char c);
uint64_t console_getchar();
void shutdown();

#endif // _SBI_H_
