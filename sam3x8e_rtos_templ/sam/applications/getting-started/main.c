#include "asf.h"
#include "stdio_serial.h"
#include "conf_board.h"
#include "conf_clock.h"
#include "FreeRTOS.h"
#include "task.h"
#include "smc.h"
#include "ili93xx.h"



#define ILI93XX_LCD_CS 1
#define IRQ_PRIOR_PIO    0

#define BLINK_PERIOD     1000

#define STRING_EOL    "\r"
#define STRING_HEADER "-- Getting Started Example --\r\n" \
"-- "BOARD_NAME" --\r\n" \
    "-- Compiled: "__DATE__" "__TIME__" --"STRING_EOL


        struct ili93xx_opt_t g_ili93xx_display_opt;


static void configure_console(void)
{
    const usart_serial_options_t uart_serial_options = {
        .baudrate = CONF_UART_BAUDRATE,
#ifdef CONF_UART_CHAR_LENGTH
        .charlength = CONF_UART_CHAR_LENGTH,
#endif
        .paritytype = CONF_UART_PARITY,
#ifdef CONF_UART_STOP_BITS
        .stopbits = CONF_UART_STOP_BITS,
#endif
    };

    /* Configure console UART. */
    sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
    stdio_serial_init(CONF_UART, &uart_serial_options);
}

void loop(void)
{
    for(;;)
    {
	vTaskDelay(20);
	ioport_toggle_pin_level(LED0_GPIO);
    }

}
int i = 200;
void task1(void)
{

    for(;;)
    {
        vTaskDelay(i);
        ioport_toggle_pin_level(PIO_PC23_IDX);
    }
}

void TRNG_Task(void)
{
    TaskHandle_t handle=xTaskGetCurrentTaskHandle();
    pmc_enable_periph_clk(ID_TRNG);
    /* Enable TRNG */
    trng_enable(TRNG);
    /* Enable TRNG interrupt */
    NVIC_DisableIRQ(TRNG_IRQn);
    NVIC_ClearPendingIRQ(TRNG_IRQn);
    NVIC_SetPriority(TRNG_IRQn, 0);
    NVIC_EnableIRQ(TRNG_IRQn);
    trng_enable_interrupt(TRNG);

    vTaskDelay(1000);
    trng_disable_interrupt(TRNG);
    vTaskDelete(handle);
}

void TRNG_Handler(void)
{
    uint32_t status;

    status = trng_get_interrupt_status(TRNG);

    if ((status & TRNG_ISR_DATRDY) == TRNG_ISR_DATRDY) {
        printf("-- Random Value: %x --\n\r", trng_read_output_data(TRNG));
    }
    trng_disable_interrupt(TRNG);
}

void lcdtask(void)
{

    /** Initialize display parameter */
    g_ili93xx_display_opt.ul_width = ILI93XX_LCD_WIDTH;
    g_ili93xx_display_opt.ul_height = ILI93XX_LCD_HEIGHT;
    g_ili93xx_display_opt.foreground_color = COLOR_BLACK;
    g_ili93xx_display_opt.background_color = COLOR_WHITE;

    /** Switch off backlight */
    //aat31xx_disable_backlight();

    /** Initialize LCD */
    ili93xx_init(&g_ili93xx_display_opt);

    /** Set backlight level */
    //aat31xx_set_backlight(AAT31XX_AVG_BACKLIGHT_LEVEL);

    ili93xx_set_foreground_color(COLOR_WHITE);
    ili93xx_draw_filled_rectangle(0, 0, ILI93XX_LCD_WIDTH,
                                  ILI93XX_LCD_HEIGHT);
    /** Turn on LCD */
    ili93xx_display_on();
    ili93xx_set_cursor_position(0, 0);

    /** Draw text, image and basic shapes on the LCD */
    ili93xx_set_foreground_color(COLOR_BLACK);
    ili93xx_draw_string(10, 20, (uint8_t *)"ili93xx_lcd example");

    ili93xx_set_foreground_color(COLOR_RED);
    ili93xx_draw_circle(60, 160, 40);
    ili93xx_set_foreground_color(COLOR_GREEN);
    ili93xx_draw_circle(120, 160, 40);
    ili93xx_set_foreground_color(COLOR_BLUE);
    ili93xx_draw_circle(180, 160, 40);

    ili93xx_set_foreground_color(COLOR_VIOLET);
    ili93xx_draw_line(0, 0, 240, 320);
    for(;;);
}

int main(void)
{
    sysclk_init();
    board_init();
    configure_console();

    if (SysTick_Config(sysclk_get_cpu_hz() / 1000)) {
        puts("-F- Systick configuration error\r");
        while (1);
    }

    /* Configure LCD EBI pins */
#define ioport_set_pin_peripheral_mode(pin, mode) \
    do {\
        ioport_set_pin_mode(pin, mode);\
           ioport_disable_pin(pin);\
    } while (0)

    //pmc_enable_periph_clk(ID_PIOC);
    ioport_set_pin_peripheral_mode(PIN_EBI_DATA_BUS_D0,PIN_EBI_DATA_BUS_FLAGS);
    ioport_set_pin_peripheral_mode(PIN_EBI_DATA_BUS_D1,PIN_EBI_DATA_BUS_FLAGS);
    ioport_set_pin_peripheral_mode(PIN_EBI_DATA_BUS_D2,PIN_EBI_DATA_BUS_FLAGS);
    ioport_set_pin_peripheral_mode(PIN_EBI_DATA_BUS_D3,PIN_EBI_DATA_BUS_FLAGS);
    ioport_set_pin_peripheral_mode(PIN_EBI_DATA_BUS_D4,PIN_EBI_DATA_BUS_FLAGS);
    ioport_set_pin_peripheral_mode(PIN_EBI_DATA_BUS_D5,PIN_EBI_DATA_BUS_FLAGS);
    ioport_set_pin_peripheral_mode(PIN_EBI_DATA_BUS_D6,PIN_EBI_DATA_BUS_FLAGS);
    ioport_set_pin_peripheral_mode(PIN_EBI_DATA_BUS_D7,PIN_EBI_DATA_BUS_FLAGS);

    ioport_set_pin_peripheral_mode(PIN_EBI_NRD,PIN_EBI_NRD_FLAGS);
    ioport_set_pin_peripheral_mode(PIN_EBI_NWE,PIN_EBI_NWE_FLAGS);
    ioport_set_pin_peripheral_mode(PIN_EBI_NCS1,PIN_EBI_NCS1_FLAGS);
    ioport_set_pin_peripheral_mode(PIN_EBI_LCD_RS,PIN_EBI_LCD_RS_FLAGS);

    /** Enable peripheral clock */
    pmc_enable_periph_clk(ID_SMC);

    /** Configure SMC interface for Lcd */
    smc_set_setup_timing(SMC, ILI93XX_LCD_CS, SMC_SETUP_NWE_SETUP(2)
                         | SMC_SETUP_NCS_WR_SETUP(2)
                             | SMC_SETUP_NRD_SETUP(2)
                                 | SMC_SETUP_NCS_RD_SETUP(2));
    smc_set_pulse_timing(SMC, ILI93XX_LCD_CS, SMC_PULSE_NWE_PULSE(4)
                         | SMC_PULSE_NCS_WR_PULSE(4)
                             | SMC_PULSE_NRD_PULSE(10)
                                 | SMC_PULSE_NCS_RD_PULSE(10));
    smc_set_cycle_timing(SMC, ILI93XX_LCD_CS, SMC_CYCLE_NWE_CYCLE(10)
                         | SMC_CYCLE_NRD_CYCLE(22));

    smc_set_mode(SMC, ILI93XX_LCD_CS, SMC_MODE_READ_MODE
                 | SMC_MODE_WRITE_MODE|SMC_MODE_DBW_BIT_8);

    *((volatile uint8_t *)0x61000000)=0x00;

  //  gpio_configure_pin(PIO_PC23_IDX,(PIO_TYPE_PIO_OUTPUT_1 | PIO_DEFAULT));

   // xTaskCreate((TaskFunction_t)loop,"loop",1024,NULL,1,NULL);
    //xTaskCreate((TaskFunction_t)task1,"task1",1024,NULL,1,NULL);
    //xTaskCreate((TaskFunction_t)TRNG_Task,"TRNG",100,NULL,1,NULL);
    xTaskCreate((TaskFunction_t)lcdtask,"lcdtask",1024,NULL,1,NULL);
    vTaskStartScheduler();

    asm("label:nop\n"
        "b label");
}
