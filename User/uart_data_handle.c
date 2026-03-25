#include "uart_data_handle.h"
#include "motor_handle.h"
#include "user_config.h"

static volatile u8 cmd_buff[10] = {0};    // 存放接收到的一条指令
static volatile u8 cur_cmd_buff_len = 0;  // 指示当前接收到的指令的索引（之后会在程序中更新，不用清零）
static volatile u8 dest_cmd_buff_len = 0; // 存放最终要接收的指令长度（之后会在程序中更新，不用清零）

static volatile u8 timeout_enable = 0; // 超时计数使能
static volatile u16 timeout_cnt = 0;   // 超时计数

// 由定时中断调用，累计超时计数
void uart_data_recv_timeout_add(void)
{
    if (timeout_enable)
    {
        timeout_cnt += 10; // 有计数溢出的风险，要注意在溢出前进行处理
    }
}

// void uart_data_recv_timeout_clear(void)
// {
//     timeout_cnt = 0;
// }

void uart_data_handle(void)
{
    static volatile u8 uart_data_handle_status = UART_DATA_HANDLE_STATUS_IDLE; // 状态机

    u8 recv_byte;
    u8 check_sum = 0; // 存放计算之后的校验和
    u8 i;             // 循环计数值

    if (0 == uart0_rxbuffer_get_count())
    {
        // 未收到数据，累加超时计时
        if (timeout_cnt >= UART_DATA_HANDLE_TIMEOUT)
        {
            // 接收超时
            timeout_cnt = 0;
            timeout_enable = 0;                                     // 不使能超时计数
            uart_data_handle_status = UART_DATA_HANDLE_STATUS_IDLE; // 重新开始接收

#if USER_DEBUG_ENABLE
#if 0
            // 打印超时之后，缓冲区内的数据
            printf("=================================>\n");
            printf("uart recv timeout\n");
            for (i = 0; i < ARRAY_SIZE(cmd_buff); i++)
            {
                printf("%02x", (u16)cmd_buff[i]);
            }
            printf("=================================^\n");
#endif
#endif
        }

        return; // 串口缓冲区的数据为空，直接返回
    }

    timeout_enable = 1; // 使能超时计数
    timeout_cnt = 0;    // 收到数据，清空倒计时
    recv_byte = uart0_rxbuffer_get_byte();

    switch (uart_data_handle_status)
    {
    case UART_DATA_HANDLE_STATUS_IDLE:
        if (UART_DATA_HANDLE_FORMAT_HEAD == recv_byte)
        {
            cmd_buff[0] = recv_byte;
            cur_cmd_buff_len = 1;

            uart_data_handle_status = UART_DATA_HANDLE_STATUS_FORMAT_HEAD;
        }
        break;
        // ===============================================================
    case UART_DATA_HANDLE_STATUS_FORMAT_HEAD:
        cmd_buff[cur_cmd_buff_len++] = recv_byte;
        dest_cmd_buff_len = recv_byte;                         // 存放要接收的数据长度
        uart_data_handle_status = UART_DATA_HANDLE_STATUS_LEN; // 表示接收到了数据帧长度
        // printf("len == %bu\n", dest_cmd_buff_len);
        break;
        // ===============================================================
    case UART_DATA_HANDLE_STATUS_LEN:
        cmd_buff[cur_cmd_buff_len++] = recv_byte;
        if (cur_cmd_buff_len >= dest_cmd_buff_len) // 如果接收完所有的数据
        {
            for (i = 0; i < dest_cmd_buff_len - 1; i++)
            {
                check_sum += cmd_buff[i];
            }

            if (check_sum != cmd_buff[dest_cmd_buff_len - 1])
            {
// 校验和错误
#if USER_DEBUG_ENABLE
                // printf("check sum error\n");
#endif
                timeout_cnt = 0;
                timeout_enable = 0;                                     // 不使能超时计数
                uart_data_handle_status = UART_DATA_HANDLE_STATUS_IDLE; // 重新接收数据
            }
            else
            {
// 校验和正确
#if USER_DEBUG_ENABLE
                // printf("check sum ok\n");
#endif
                uart_data_handle_status = UART_DATA_HANDLE_STATUS_END;
            }
        }
        break;
    // ===============================================================
    default:

        break;
    }

    if (UART_DATA_HANDLE_STATUS_END != uart_data_handle_status)
    {
        return; // 未接收完数据，不进入下面的处理操作，函数直接返回
    }

#if USER_DEBUG_ENABLE
#if 0
    // 打印接收到的一帧数据
    for (i = 0; i < dest_cmd_buff_len; i++)
    {
        printf("0x%02x ", (u16)cmd_buff[i]);
    }
    printf("\n");
#endif
#endif

    switch (cmd_buff[2])
    {
        // 判断要操作哪个电机
    case 0x01:
        // printf("obj == motor 0 \n");
        switch (cmd_buff[3])
        {
            // 判断要执行什么操作
        case UART_CMD_STOP: // 停止
            break;
        case UART_CMD_FORWARD: // 正转
            motor_set_dir(&motor_handle_0, MOTOR_DIR_FORWARD);
#if USER_DEBUG_ENABLE
            // printf("motor 0 forward\n");
#endif
            break;
        case UART_CMD_REVERSE: // 反转
            motor_set_dir(&motor_handle_0, MOTOR_DIR_REVERSE);
#if USER_DEBUG_ENABLE
            // printf("motor 0 reverse\n");
#endif
            break;
        default:
            break;
        }
        break;
        // =======================================
    case 0x02:
        // printf("motor 1 \n");
        switch (cmd_buff[3])
        {
            // 判断要执行什么操作
        case UART_CMD_STOP: // 停止
            break;
        case UART_CMD_FORWARD: // 正转
            motor_set_dir(&motor_handle_1, MOTOR_DIR_FORWARD);
#if USER_DEBUG_ENABLE
            // printf("motor 1 forward\n");
#endif
            break;
        case UART_CMD_REVERSE: // 反转
            motor_set_dir(&motor_handle_1, MOTOR_DIR_REVERSE);
#if USER_DEBUG_ENABLE
            // printf("motor 1 reverse\n");
#endif
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }

    // 处理完成后，重新接收数据
    timeout_cnt = 0;
    timeout_enable = 0; // 不使能超时计数
    uart_data_handle_status = UART_DATA_HANDLE_STATUS_IDLE;
}
