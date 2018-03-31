#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"

void LedInit(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void task1(void)
{
    LedInit();
    for(;;)
    {
        GPIO_SetBits(GPIOC,GPIO_Pin_13);
        vTaskDelay(500);
        GPIO_ResetBits(GPIOC,GPIO_Pin_13);
        vTaskDelay(500);
    }
}

void main(void)
{
    SysTick_Config(72000);

    xTaskCreate((TaskFunction_t)task1,"task1",1024,NULL,1,NULL);

    vTaskStartScheduler();

    for(;;);
}