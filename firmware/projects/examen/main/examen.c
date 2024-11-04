/*! @mainpage Examen Final
 *
 * @section genDesc General Description
 * Examen 4/11/2024
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    BUZZER     |    ESP32   	|
 * |:-------------:|:---------- ----|
 * | 	PIN        |   	GPIO_1		|
 * 
 * | Acelerometro  |    ESP32   	|
 * |:-------------:|:---------- ----|
 * | 	canal x    |     CH1 	    |
 * | 	canal Y    |   	 CH2		|
 * | 	canal Z    |   	 CH3		|
 * 
 * |    HC-SR04     |   ESP32   	|
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
 * | 12/09/2023 | Examen 4/11/24     	                         |
 *
 * @author Giorgi Joseifuna (josefina.giorgi@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hc_sr04.h"
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
#include "led.h"
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/
/** @brief Periodo de tiempo para la tarea de tomar distancia en microsegundos. */
#define CONFIG_BLINK_PERIOD_TOMAR_DISTANCIA 500000 //son 0,5 seg
/** @brief Periodo de tiempo para la tarea de medir acelerometro en microsegundos. */
#define CONFIG_BLINK_PERIOD_ACELEROMETRO 10000 // son 100hz
/** @brief Periodo de tiempo para la frecuencia del buzzer en milisegundos. */
#define CONFIG_BLINK_PERIOD_GPIO_1 500

/**
 * @typedef gpioConf_t
 * @brief Estructura que define la configuración de un pin GPIO.
 *
 * Esta estructura contiene la configuración de un pin GPIO, incluyendo su número de pin
 * y la dirección (entrada o salida).
 */
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;
/*==================[internal data definition]===============================*/
/**
 * @brief Handle de la tarea de sensar distancia.
 */
TaskHandle_t tomarDistancia_task_handle = NULL;
/**
 * @brief Handle de la tarea enviar notifiacion.
 */
TaskHandle_t enviarNotificacion_task_handle = NULL;
/**
 * @brief Handle de la tarea de medir acelerometro.
 */
TaskHandle_t medidaAcelerometro_task_handle = NULL;

/**
 * @brief Variable que almacena la distancia medida en centímetros.
 *
 * Almacena el valor de la distancia medida por el sensor HcSr04, expresado en
 * centímetros.
 */
uint16_t distancia;
/*==================[internal functions declaration]=========================*/
/**
 * @brief Controla el encendido de los LEDs según la distancia medida.
 *
 * Enciende o apaga los LEDs en función de la distancia:
 * - Prende todos los LEDs si la distancia es menor a 300cm.
 * - Prende el LED verde y LED amarillo si la distancia está entre 300cm y 500cm.
 * - Apaga los tres LEDs si la distancia es mayor a 500cm.
 * 
 * @param distancia Distancia medida en centímetros.
 */
static void SeleccionLED(uint16_t distancia) {
    if (distancia < 300) {
        LedOn(LED_1);
        LedOn(LED_2);
        LedOn(LED_3);
    } else if ((distancia >= 300) && (distancia < 500)) {
        LedOn(LED_1);
        LedOn(LED_2);
        LedOff(LED_3);
    } else if (distancia >= 500) {
        LedOn(LED_1);
        LedOff(LED_2);
        LedOff(LED_3);
    }
}

/**
 * @brief Tarea encargada de tomar medidas de distancia.
 *
 * Esta tarea se ejecuta continuamente. Mide la distancia utilizando el sensor HcSr04. La tarea se desbloquea
 * mediante notificaciones para realizar la medición.
 * Ademas llama a la funcion seleccion de leds y a la funcion activar alarma.
 * Tambien envia la notificacion a una Aplicación corriendo en un Smartphone. (segun la consigna)
 * 
 * @param pvParameter Parámetro pasado a la tarea (no utilizado en este caso).
 */
static void TomarDistancia(void *pvParameter) {
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    	 distancia = HcSr04ReadDistanceInCentimeters();
		 SeleccionLED(distancia);
		 ActivaAlarma(distancia);
		 vTaskNotifyGiveFromISR(enviarNotificacion_task_handle, pdFALSE);
    }
}

/**
 * @brief Tarea encargada de envíar utilizando un módulo bluetooth conectado al segundo puerto serie de la placa ESP-EDU
 * La tarea envia la frase "Peligro vehiculo cerca" cuando la distancia es menor a 3m.
 * La tarea envia la frase "Precaucion vehiculo cerca" cuando la distancia es entre 3m y 5m.
 *
 * @param pvParameter Parámetro pasado a la tarea (no utilizado en este caso).
 */
static void EnviarNotificacion(void *pvParameter) {
   	if (distancia < 300) {
        UartSendString(UART_CONNECTOR,"Peligro, vehiculo cerca\n");
    } else if ((distancia >= 300) && (distancia < 500)) {
		UartSendString(UART_CONNECTOR,"Precaucion, vehiculo cerca\n");
    }
}

/**
 * @brief Funcion encargada de activar una alarma a distintas frecuencias dependiendo de la distancia a la cual se encuentren los vehiculos traseros.
 * El buzzer suena a una frecuencia de 1 segundo cuando esta entre 3m y 5m.
 * El buzzer suena a una frecuencia de 0.5 segundo cuando esta menor a 3m.
 * 
 * @param pvParameter Parámetro pasado a la tarea (no utilizado en este caso).
 */
static void ActivaAlarma(uint16_t distancia) {
	GPIOInit(GPIO_1,GPIO_OUTPUT);
	if ((distancia >= 300) && (distancia < 500)) {
        GPIOOn(GPIO_1);
		vTaskDelay(CONFIG_BLINK_PERIOD_GPIO_1 / portTICK_PERIOD_MS);
		GPIOOn(GPIO_1);
		vTaskDelay(CONFIG_BLINK_PERIOD_GPIO_1 / portTICK_PERIOD_MS);

    } else if (distancia < 300) {
        GPIOOn(GPIO_1);
		vTaskDelay((CONFIG_BLINK_PERIOD_GPIO_1)/2 / portTICK_PERIOD_MS);
		GPIOOn(GPIO_1);
		vTaskDelay((CONFIG_BLINK_PERIOD_GPIO_1)/2 / portTICK_PERIOD_MS);
    }
}

/**
 * @brief  Tarea encargada de medir el acelerometro en cada uno de sus ejes.
 * Esta medida se convierte mediante una conversion proporcionada por la catedra.
 * Una vez convertida se compara si es mayo que 4 y envia un mensaje a la aplicacion advirtiendo "Caida detectada".
 * 
 * @param pvParameter Parámetro pasado a la tarea (no utilizado en este caso).
 */
static void MedidaAcelerometro(void *pvParameter){
	float medidaX;
	float medidaY;
	float medidaZ;
	while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        AnalogInputReadSingle(CH1, &medidaX);
		AnalogInputReadSingle(CH2, &medidaY);
		AnalogInputReadSingle(CH3, &medidaZ);

		float medidaXenG = (medidaX-1.56)/(0,3);
		float medidaYenG = (medidaX-1.56)/(0,3);
		float medidaZenG = (medidaX-1.56)/(0,3);

		float sumatoria = (medidaXenG+medidaYenG+medidaYenG);

		if(sumatoria>4)
		{
			UartSendString(UART_CONNECTOR,"CAIDA DETECTADA");
		}

    }

}

/**
 * @brief Función para notificar la tarea de sensar distancia.
 *
 * Envía una notificación a la tarea de sensar distancia para indicar que debe
 * realizar una medición. Se llama desde la ISR del timer.
 * 
 * @param param Parámetro pasado a la función (no utilizado en este caso).
 */
void funcTomarDistancia(void* param) {
    vTaskNotifyGiveFromISR(tomarDistancia_task_handle, pdFALSE);
}

/**
 * @brief Función para notificar la tarea de tomar medida del acelerometro.
 *
 * Envía una notificación a la tarea medida acelerometro para indicar que debe
 * realizar una medición. Se llama desde la ISR del timer.
 * 
 * @param param Parámetro pasado a la función (no utilizado en este caso).
 */
void funcMedidaAcelerometro(void* param) {
    vTaskNotifyGiveFromISR(medidaAcelerometro_task_handle, pdFALSE);
}
/*==================[external functions definition]==========================*/
/**
 * @brief Función principal de la aplicación.
 *
 * Inicializa todos los periféricos, configura las tareas y timers, y prepara el sistema para funcionar.
 */
void app_main(void){

HcSr04Init(GPIO_3, GPIO_2);
LedsInit();
LcdItsE0803Init();

// Inicialización de timer distancia
    timer_config_t timer_tomarDistancia = {
        .timer = TIMER_A,
        .period = CONFIG_BLINK_PERIOD_TOMAR_DISTANCIA,
        .func_p = funcTomarDistancia,
        .param_p = NULL
    };
    TimerInit(&timer_tomarDistancia);
	TimerStart(&timer_tomarDistancia);
xTaskCreate(&TomarDistancia, "Sensar distancia", 2048, NULL, 5, &tomarDistancia_task_handle);
xTaskCreate(&EnviarNotificacion, "Sensar distancia", 2048, NULL, 5, &enviarNotificacion_task_handle);

gpioConf_t pin={GPIO_1,GPIO_OUTPUT};

//Notifiacion bluethoot al segundo UART de la placa
serial_config_t configSerie = {
		.port = UART_CONNECTOR,
        .baud_rate = 115200,
        .func_p = NULL,
        .param_p = NULL,
	};

	UartInit(&configSerie);

//Inicializacion timer acelerometro
 timer_config_t timerAcelerometro = {
        .timer = TIMER_B,
        .period = CONFIG_BLINK_PERIOD_ACELEROMETRO,
        .func_p = funcMedidaAcelerometro,
        .param_p = NULL
    };
    TimerInit(&timerAcelerometro);
    TimerStart(&timerAcelerometro);

 //Defino converion AD, imagino que el velocimetro esta en CH1
 analog_input_config_t conversorAD = {
        .input = CH1,CH2,CH3,
        .mode = ADC_SINGLE,
    };
    AnalogInputInit(&conversorAD);

xTaskCreate(&MedidaAcelerometro, "Medida Acelerometro", 2048, NULL, 5, &medidaAcelerometro_task_handle);
}
/*==================[end of file]============================================*/