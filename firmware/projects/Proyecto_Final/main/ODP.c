/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Proyecto Final		                         |
 *
 * @author Josefina Giorgi (josefina.giorgi@ingenieria.uner.edu.ar)
 * @author Irina E. Lauritto (irina.lauritto@ingenria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
#define RETARDO_ODP 20000  
/*==================[internal data definition]===============================*/
TaskHandle_t adquirirODP_task_handle = NULL;
/*==================[internal functions declaration]=========================*/

static void adquirirODP(void *pvParameter) {

}


void funcTimerODP(void *pvParameter) {
    vTaskNotifyGiveFromISR(adquirirODP_task_handle, pdFALSE);
}

/*==================[external functions definition]==========================*/
void app_main(void){

// Inicializar timers
	timer_config_t timer_ODP = {
        .timer = TIMER_B,
        .period = adquirirODP_task_handle,
        .func_p = funcTimerODP,
        .param_p = NULL
    };
    TimerInit(&timer_ODP);
    TimerStart(timer_ODP.timer);
	

	//Creacion de tareas
	xTaskCreate(&adquirirODP, "Adquirir ODP", 2048, NULL, 5, &adquirirODP_task_handle);
	

	//Inicializar UART para comunicación con la PC
    serial_config_t configPC = {
        .port = UART_PC,
        .baud_rate = 115200,
        .func_p = NULL,
        .param_p = NULL,
    };
    UartInit(&configPC);
}
/*==================[end of file]============================================*/