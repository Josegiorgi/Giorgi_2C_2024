/*! @mainpage Template
 *
 * @brief Este programa controla un sistema de medición de distancia utilizando un ESP32, un sensor ultrasónico HC-SR04 y una pantalla LCD. 
 * También utiliza LEDs para representar visualmente la distancia medida y una comunicación UART para enviar los valores de distancia a una PC.
 * 
 * El programa mide distancias, controla los LEDs en función de la distancia medida, muestra la distancia en una pantalla LCD y 
 * se comunica con una PC a través de un puerto serial. El usuario puede interactuar con el sistema mediante botones o comandos UART 
 * para encender/apagar el sistema o activar/desactivar la función de "hold", que mantiene el último valor medido en la pantalla.
 *
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO	 	| 	GPIO_3		|
 * | 	TRIGGER	 	| 	GPIO_2		|
 * | 	 +5V 	 	| 	 +5V		|
 * | 	 GND 	 	| 	 GND		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 18/09/2024 | Guia 2 ejercicio 3		                     |
 *
 * @author Josefina Giorgi (josefina.giorgi@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
/**
 * @brief Inclusión de las librerías estándar y personalizadas necesarias para el funcionamiento del programa.
 */
#include <stdio.h>
#include <stdint.h>
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
/*==================[macros and definitions]=================================*/

/**
 * @brief Periodo en microsegundos para la tarea de mostrar medida.
 *
 * Define el intervalo de tiempo para la tarea encargada de mostrar la medida
 * en la pantalla LCD.
 */
#define CONFIG_BLINK_PERIOD_MOSTRAR_MEDIDA 500000

/**
 * @brief Periodo en microsegundos para la tarea de tomar medida.
 *
 * Define el intervalo de tiempo para la tarea encargada de tomar la medida
 * utilizando el sensor de distancia.
 */
#define CONFIG_BLINK_PERIOD_TOMAR_MEDIDA 1000000

/*==================[internal data definition]===============================*/

/**
 * @brief Handle para la tarea de tomar medidas.
 *
 * Se utiliza para referirse a la tarea que toma las medidas del sensor de
 * distancia. Este handle es necesario para gestionar y manipular la tarea
 * desde el sistema operativo FreeRTOS.
 */
TaskHandle_t tomarMedida_task_handle = NULL;

/**
 * @brief Handle para la tarea de mostrar medidas.
 *
 * Se utiliza para referirse a la tarea que muestra las medidas en la pantalla
 * LCD y controla los LEDs. Este handle es necesario para gestionar y
 * manipular la tarea desde el sistema operativo FreeRTOS.
 */
TaskHandle_t mostrarMedida_task_handle = NULL;

/**
 * @brief Variable que almacena la distancia medida en centímetros.
 *
 * Almacena el valor de la distancia medida por el sensor HcSr04, expresado en
 * centímetros.
 */
uint16_t distancia;

/**
 * @brief Variable que almacena el estado de las teclas.
 *
 * Almacena el valor leído de las teclas, utilizado para determinar qué acción
 * realizar en función de la tecla presionada.
 */
uint8_t teclas;

/**
 * @brief Indica el estado de encendido del sistema.
 *
 * Indica si el sistema está encendido (true) o apagado (false). Este valor se
 * utiliza para controlar la ejecución de las tareas relacionadas con el
 * sistema.
 */
bool encendido = false;

/**
 * @brief Indica el estado del modo hold.
 *
 * Indica si el modo hold está activo (true) o inactivo (false). Este valor
 * controla si la distancia medida debe ser mostrada en la pantalla LCD.
 */
bool hold = true;

/*==================[internal functions declaration]=========================*/

/**
 * @brief Controla el encendido de los LEDs según la distancia medida.
 *
 * Enciende o apaga los LEDs en función de la distancia:
 * - Apaga todos los LEDs si la distancia es menor a 10.
 * - Enciende el LED 1 si la distancia está entre 10 y 20.
 * - Enciende los LEDs 1 y 2 si la distancia está entre 20 y 30.
 * - Enciende los tres LEDs si la distancia es mayor a 30.
 * 
 * @param distancia Distancia medida en centímetros.
 */
static void SeleccionLED(uint16_t distancia) {
    if (distancia < 10) {
        LedOff(LED_1);
        LedOff(LED_2);
        LedOff(LED_3);
    } else if ((distancia >= 10) && (distancia < 20)) {
        LedOn(LED_1);
        LedOff(LED_2);
        LedOff(LED_3);
    } else if ((distancia >= 20) && (distancia < 30)) {
        LedOn(LED_1);
        LedOn(LED_2);
        LedOff(LED_3);
    } else if (distancia >= 30) {
        LedOn(LED_1);
        LedOn(LED_2);
        LedOn(LED_3);
    }
}
/**
 * @brief Tarea encargada de tomar medidas de distancia.
 *
 * Esta tarea se ejecuta continuamente. Cuando el sistema está encendido,
 * mide la distancia utilizando el sensor HcSr04. La tarea se desbloquea
 * mediante notificaciones para realizar la medición.
 * 
 * @param pvParameter Parámetro pasado a la tarea (no utilizado en este caso).
 */
static void TomarMedida(void *pvParameter) {
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (encendido) {
            distancia = HcSr04ReadDistanceInCentimeters();
        }
    }
}

/**
 * @brief Tarea encargada de mostrar la medida de distancia y controlar los LEDs. Ademas se agrego el envio del string por puerto PC.
 *
 * Esta tarea actualiza los LEDs según la distancia medida y muestra la
 * distancia en una pantalla LCD si el sistema está encendido y el modo hold
 * está activo. La tarea se desbloquea mediante notificaciones para actualizar
 * la pantalla y los LEDs.
 * 
 * @param pvParameter Parámetro pasado a la tarea (no utilizado en este caso).
 */
static void MostrarMedida(void *pvParameter) {
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (encendido) {
            SeleccionLED(distancia);
			UartSendString(UART_PC,(const char*)UartItoa(distancia,10));
			UartSendString(UART_PC," cm\r\n");
            if (hold) {
                LcdItsE0803Write(distancia);
            }
        } else {
            LedsOffAll();
            LcdItsE0803Off();
        }
    }
}

/**
 * @brief Función de interrupción para el botón 1.
 *
 * Alterna el estado de encendido del sistema cuando se presiona el botón 1.
 */
void intFuncionTecla1() {
    encendido = !encendido;
}

/**
 * @brief Función de interrupción para el botón 2.
 *
 * Alterna el estado del modo hold cuando se presiona el botón 2.
 */
void intFuncionTecla2() {
    hold = !hold;
}

/**
 * @brief Función para notificar la tarea de tomar medidas desde la ISR.
 *
 * Envía una notificación a la tarea de tomar medidas para indicar que debe
 * realizar una medición. Se llama desde la ISR del timer.
 * 
 * @param param Parámetro pasado a la función (no utilizado en este caso).
 */
void funcTomarMedida(void* param) {
    vTaskNotifyGiveFromISR(tomarMedida_task_handle, pdFALSE);
}

/**
 * @brief Función para notificar la tarea de mostrar medidas desde la ISR.
 *
 * Envía una notificación a la tarea de mostrar medidas para indicar que debe
 * actualizar los LEDs y la pantalla. Se llama desde la ISR del timer.
 * 
 * @param param Parámetro pasado a la función (no utilizado en este caso).
 */
void funcMostrarMedida(void* param) {
    vTaskNotifyGiveFromISR(mostrarMedida_task_handle, pdFALSE);
}

/**
 * @brief Procesa las entradas de la UART.
 *
 * Esta función procesa las entradas desde la UART, permitiendo el control del encendido y del estado 
 * de hold mediante comandos recibidos.
 */
void funcionPuertoSerie(){
	uint8_t entradaTeclado;
	UartReadByte(UART_PC,&entradaTeclado);
	switch (entradaTeclado)
	{
	case'O':
		encendido=!encendido;
		break;
	case 'H':
		hold=!hold;
		break;
	default:
		break;
	}		
}

/*==================[external functions definition]==========================*/
/**
 * @brief Función principal de la aplicación.
 *
 * Inicializa todos los periféricos, configura las tareas y timers, y prepara el sistema para funcionar.
 */
void app_main(void) {
    LedsInit();
    SwitchesInit();
    HcSr04Init(GPIO_3, GPIO_2);
    LcdItsE0803Init();

    // Inicialización de timers
    timer_config_t timer_tomarMedida = {
        .timer = TIMER_A,
        .period = CONFIG_BLINK_PERIOD_TOMAR_MEDIDA,
        .func_p = funcTomarMedida,
        .param_p = NULL
    };
    TimerInit(&timer_tomarMedida);

    timer_config_t timer_mostrarMedida = {
        .timer = TIMER_B,
        .period = CONFIG_BLINK_PERIOD_MOSTRAR_MEDIDA,
        .func_p = funcMostrarMedida,
        .param_p = NULL
    };
    TimerInit(&timer_mostrarMedida);

    xTaskCreate(&TomarMedida, "Tomar Medida", 2048, NULL, 5, &tomarMedida_task_handle);
    xTaskCreate(&MostrarMedida, "Mostrar Medida", 512, NULL, 5, &mostrarMedida_task_handle);

    SwitchActivInt(SWITCH_1, intFuncionTecla1, NULL);
    SwitchActivInt(SWITCH_2, intFuncionTecla2, NULL);

    // Inicialización del conteo de timers
    TimerStart(timer_tomarMedida.timer);
    TimerStart(timer_mostrarMedida.timer);

	//Defino estructura
	serial_config_t configSerie = {
		.port = UART_PC,
        .baud_rate = 115200,
        .func_p = funcionPuertoSerie,
        .param_p = NULL,
	};

	UartInit(&configSerie);

}
