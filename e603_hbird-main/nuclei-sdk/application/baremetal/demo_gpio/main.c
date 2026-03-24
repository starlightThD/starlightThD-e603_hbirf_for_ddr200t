// See LICENSE for license details.
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "nuclei_sdk_soc.h"

// GPIO寄存器定义
#define GPIO_BASE_ADDR          0x10010000UL
#define GPIO_LED_REG_OFFSET     0x0000      // LED寄存器偏移
#define GPIO_SW_REG_OFFSET      0x0004      // SW寄存器偏移

// 内存映射访问宏
#define GPIO_LED_REG            (*((volatile uint32_t *)(GPIO_BASE_ADDR + GPIO_LED_REG_OFFSET)))
#define GPIO_SW_REG             (*((volatile uint32_t *)(GPIO_BASE_ADDR + GPIO_SW_REG_OFFSET)))

// 简单的延时函数
void simple_delay(uint32_t count) {
    for (volatile uint32_t i = 0; i < count; i++) {
        __asm__ volatile ("nop");
    }
}

// 读取SW开关状态
uint8_t read_switches(void) {
    return GPIO_SW_REG & 0xFF;
}

// 设置LED状态
void set_leds(uint8_t pattern) {
    GPIO_LED_REG = pattern;
}

// 显示SW状态到UART
void display_sw_status(uint8_t sw_value) {
    printf("SW状态: ");
    for (int i = 7; i >= 0; i--) {
        printf("%c", (sw_value >> i) & 1 ? '1' : '0');
    }
    printf(" (0x%02X)\r\n", sw_value);
}

// 模式1: 基本流水灯
void mode_flowing_led(uint32_t speed) {
    uint8_t patterns[] = {
        0b00000001,  // LED0亮
        0b00000010,  // LED1亮
        0b00000100,  // LED2亮
        0b00001000,  // LED3亮
        0b00010000,  // LED4亮
        0b00100000,  // LED5亮
        0b01000000,  // LED6亮
        0b10000000,  // LED7亮
    };
    
    static int pattern_index = 0;
    
    set_leds(patterns[pattern_index]);
    pattern_index = (pattern_index + 1) % 8;
    simple_delay(speed);
}

// 模式2: 呼吸灯效果
void mode_breathing_led(uint32_t speed) {
    static uint8_t brightness = 0;
    static int direction = 1;  // 1: 增加亮度, -1: 减少亮度
    static uint32_t pwm_counter = 0;
    
    // 简单的PWM实现：根据亮度值控制LED
    // 当PWM计数器小于亮度值时，LED亮；否则灭
    if (pwm_counter < brightness) {
        set_leds(0xFF);  // 所有LED亮
    } else {
        set_leds(0x00);  // 所有LED灭
    }
    
    // 更新PWM计数器
    pwm_counter = (pwm_counter + 16) % 256;
    
    // 更新亮度（呼吸效果）
    brightness += direction * 2;
    if (brightness >= 250) {
        brightness = 250;
        direction = -1;
    } else if (brightness <= 5) {
        brightness = 5;
        direction = 1;
    }
    
    simple_delay(speed / 50);
}

// 模式3: 二进制计数器
void mode_binary_counter(uint32_t speed) {
    static uint8_t counter = 0;
    
    set_leds(counter);
    counter++;
    simple_delay(speed);
}

// 模式4: 随机闪烁
void mode_random_blink(uint32_t speed) {
    static uint32_t seed = 0x12345678;
    
    // 简单的伪随机数生成
    seed = seed * 1103515245 + 12345;
    set_leds((seed >> 16) & 0xFF);
    simple_delay(speed);
}

// 模式5: 追逐效果
void mode_chase_effect(uint32_t speed) {
    static uint8_t pattern = 0b00000001;
    static int direction = 0;  // 0: 向左, 1: 向右
    
    set_leds(pattern);
    
    if (direction == 0) {
        pattern = (pattern << 1) | (pattern >> 7);
        if (pattern == 0b10000000) direction = 1;
    } else {
        pattern = (pattern >> 1) | (pattern << 7);
        if (pattern == 0b00000001) direction = 0;
    }
    
    simple_delay(speed);
}

// 根据SW选择模式和速度
void process_sw_control(void) {
    uint8_t sw_value = read_switches();
    
    // SW[2:0] 选择模式 (0-7)
    uint8_t mode = sw_value & 0x07;
    
    // SW[7:3] 控制速度 (0-31)
    uint8_t speed_level = (sw_value >> 3) & 0x1F;
    
    // 速度映射：级别越高，速度越慢
    uint32_t speed = 500000 + (speed_level * 100000);
    
    // 只在模式变化时显示
    static uint8_t last_mode = 0xFF;
    static uint8_t last_speed_level = 0xFF;
    
    if (mode != last_mode || speed_level != last_speed_level) {
        printf("\r\n");
        display_sw_status(sw_value);
        printf("模式: %d, 速度级别: %d\r\n", mode, speed_level);
        last_mode = mode;
        last_speed_level = speed_level;
    }
    
    // 根据模式执行相应的LED效果
    switch (mode) {
        case 0:
            // 模式0: 关闭所有LED
            set_leds(0x00);
            break;
        case 1:
            // 模式1: 基本流水灯
            mode_flowing_led(speed);
            break;
        case 2:
            // 模式2: 呼吸灯效果
            mode_breathing_led(speed);
            break;
        case 3:
            // 模式3: 二进制计数器
            mode_binary_counter(speed);
            break;
        case 4:
            // 模式4: 随机闪烁
            mode_random_blink(speed);
            break;
        case 5:
            // 模式5: 追逐效果
            mode_chase_effect(speed);
            break;
        case 6:
            // 模式6: 显示SW状态
            set_leds(sw_value);
            break;
        case 7:
            // 模式7: 交替闪烁
            static uint8_t alt_state = 0;
            set_leds(alt_state ? 0xAA : 0x55);
            alt_state = !alt_state;
            simple_delay(speed);
            break;
        default:
            set_leds(0x00);
            break;
    }
}

int main(void)
{
    uint32_t rval, seed;
    unsigned long hartid, clusterid;
    rv_csr_t misa;

    // 获取当前集群的hart id
    hartid = __get_hart_id();
    clusterid = __get_cluster_id();
    misa = __RV_CSR_READ(CSR_MISA);

    printf("========================================\r\n");
    printf("SW控制LED流水灯应用\r\n");
    printf("Cluster %lu, Hart %lu, MISA: 0x%lx\r\n", clusterid, hartid, misa);
    
    printf("\r\nGPIO配置信息:\r\n");
    printf("GPIO基地址: 0x%08lX\r\n", GPIO_BASE_ADDR);
    printf("LED寄存器地址: 0x%08lX\r\n", GPIO_BASE_ADDR + GPIO_LED_REG_OFFSET);
    printf("SW寄存器地址: 0x%08lX\r\n", GPIO_BASE_ADDR + GPIO_SW_REG_OFFSET);
    
    printf("\r\n使用说明:\r\n");
    printf("1. SW[2:0] 选择LED显示模式 (0-7)\r\n");
    printf("2. SW[7:3] 控制流水灯速度 (0-31)\r\n");
    printf("3. 模式说明:\r\n");
    printf("   0: 关闭所有LED\r\n");
    printf("   1: 基本流水灯\r\n");
    printf("   2: 呼吸灯效果\r\n");
    printf("   3: 二进制计数器\r\n");
    printf("   4: 随机闪烁\r\n");
    printf("   5: 追逐效果\r\n");
    printf("   6: 显示SW状态\r\n");
    printf("   7: 交替闪烁\r\n");
    printf("========================================\r\n");
    
    // 初始显示SW状态
    uint8_t initial_sw = read_switches();
    printf("\r\n初始SW状态: ");
    display_sw_status(initial_sw);
    
    // 主循环：处理SW控制
    while (1) {
        process_sw_control();
    }

    return 0;
}
