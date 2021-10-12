#ifndef _SBI_H_
#define _SBI_H_

#define SBI_CONSOLE_PUTCHAR 1
#define SBI_CONSOLE_GETCHAR 2
#define SBI_SHUTDOWN 8

void console_putchar(char c);
void console_getchar();
void shutdown();

#endif // _SBI_H_