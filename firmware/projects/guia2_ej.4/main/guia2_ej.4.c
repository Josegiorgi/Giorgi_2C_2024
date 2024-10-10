/*! @mainpage Ejercicio 4, de proyecto 2.
 *
 * @brief Descripción General: Esta sección describe cómo funciona el programa.
 * El programa está diseñado para simular una señal ECG y leer una señal analógica utilizando el ESP32. 
 * Los datos se transmiten vía UART a una PC para su visualización.
 * Este archivo implementa dos tareas:
 * - Tarea 1: Lee una señal analógica de CH1 y la envía por UART.
 * - Tarea 2: Genera una señal ECG para simular latidos cardíacos.
 *
 *
 * <a href="https://drive.google.com/...">Ejemplo de Operación</a>
 *
 * @section hardConn Conexiones de Hardware
 *
 * |    Periférico  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 * @section changelog Registro de Cambios
 *
 * |   Fecha	    | Descripción                                    |
 * |:----------:|:-----------------------------------------------|
 * | 03/10/2024 | Guía 2, ejercicio 4.        	                 |
 *
 * @autor Josefina Giorgi (josefina.giorgi@ingenieria.uner.edu.ar)
 */

/*==================[inclusiones]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"

/*==================[macros y definiciones]=================================*/
/** @brief Periodo de tiempo para la tarea de conversión ADC en microsegundos. */
#define CONFIG_BLINK_PERIOD_AD 20000  
/** @brief Periodo de tiempo para la tarea de generación de señal ECG en microsegundos. */
#define CONFIG_BLINK_PERIOD_GENERARECG 40000 
/** @brief Tamaño del buffer para la señal ECG. */
#define BUFFER_SIZE 231

/*==================[definición de datos internos]===============================*/
/**
 * @brief Handle de la tarea de conversión ADC.
 */
TaskHandle_t ConversionAD_task_handle = NULL;

/**
 * @brief Handle de la tarea de generación de señal ECG.
 */
TaskHandle_t generarECG_task_handle = NULL;

/** @brief Variable para almacenar el valor del ADC. */
uint16_t numero;

/** @brief Contador para la generación de señal ECG. */
uint16_t i = 0;

/** @brief Buffer que contiene los valores predefinidos de la señal ECG. */
const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};

/*==================[declaración de funciones internas]=========================*/

/**
 * @brief Tarea que lee el valor ADC de CH1 y lo envía por UART.
 * 
 * Esta tarea se activa periódicamente por un temporizador y envía el valor del ADC
 * convertido a una cadena de caracteres por UART.
 * 
 * @param pvParameter Parámetro de la tarea (no utilizado).
 */
static void ConversionAD(void *pvParameter) {
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        AnalogInputReadSingle(CH1, &numero);
        UartSendString(UART_PC, (const char*)UartItoa(numero, 10));
        UartSendString(UART_PC, "\r");
    }
}

/**
 * @brief Tarea que genera una señal ECG emitiendo valores a través del DAC.
 * 
 * Esta tarea emite los valores del array `ecg` para simular una señal ECG.
 * Recorre el buffer y emite cada valor periódicamente. Esta tarea no funcionaba y por esa razon el ecg se recorre mas abajo.
 * 
 * @param pvParameter Parámetro de la tarea (no utilizado).
 */
static void generarECG(void *pvParameter) {
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (i < BUFFER_SIZE) {
            AnalogOutputWrite(i);
        } else {
            i = 0;
        }
        i++;
    }
}

/**
 * @brief Función de callback del temporizador que activa la tarea de conversión ADC.
 *
 * @param pvParameter Parámetro del temporizador (no utilizado).
 */
static void funcConversionAD(void *pvParameter) {
    vTaskNotifyGiveFromISR(ConversionAD_task_handle, pdFALSE);
}

/**
 * @brief Función de callback del temporizador que activa la tarea de generación de ECG. La tarea fue activada aca
 * porque no me dejaba crearla a parte y el programa no ingresaba en el llamado de la tarea creada.
 *
 * @param pvParameter Parámetro del temporizador (no utilizado).
 */
static void funcgenerarECG(void *pvParameter) {
    if (i < BUFFER_SIZE) {
        AnalogOutputWrite(ecg[i]);
    } else {
        i = 0;
    }
    i++;
}

/*==================[definición de funciones externas]==========================*/

/**
 * @brief Función principal que inicializa periféricos y crea las tareas.
 * 
 * Esta función inicializa temporizadores, UART, ADC y DAC, y lanza
 * dos tareas para la generación de ECG y la lectura ADC.
 */
void app_main(void) {
    // Inicializar temporizador para conversión ADC
    timer_config_t timer_conversionAD = {
        .timer = TIMER_A,
        .period = CONFIG_BLINK_PERIOD_AD,
        .func_p = funcConversionAD,
        .param_p = NULL
    };
    TimerInit(&timer_conversionAD);
    TimerStart(timer_conversionAD.timer);

    xTaskCreate(&ConversionAD, "Conversión AD", 2048, NULL, 5, &ConversionAD_task_handle);

    // Inicializar temporizador para generación de ECG
    timer_config_t timer_generarECG = {
        .timer = TIMER_B,
        .period = CONFIG_BLINK_PERIOD_GENERARECG,
        .func_p = funcgenerarECG,
        .param_p = NULL
    };
    TimerInit(&timer_generarECG);
    TimerStart(timer_generarECG.timer);

    xTaskCreate(&generarECG, "Generar ECG", 2048, NULL, 5, &generarECG_task_handle);

    // Inicializar entrada ADC
    analog_input_config_t conversorAD = {
        .input = CH1,
        .mode = ADC_SINGLE,
    };
    AnalogInputInit(&conversorAD);

    // Inicializar salida DAC
    AnalogOutputInit();

    // Inicializar UART para comunicación con la PC
    serial_config_t configPC = {
        .port = UART_PC,
        .baud_rate = 115200,
        .func_p = NULL,
        .param_p = NULL,
    };
    UartInit(&configPC);
}

/*==================[fin del archivo]============================================*/
