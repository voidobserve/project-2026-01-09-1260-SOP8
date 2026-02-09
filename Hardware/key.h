#ifndef __KEY_H__
#define __KEY_H__

#define KEY_0_PIN P16 // SW1 检测脚
#define KEY_1_PIN P02 // SW2 检测脚

void key_init(void);
void key0_scan_handle(void);
void key1_scan_handle(void);

#endif
