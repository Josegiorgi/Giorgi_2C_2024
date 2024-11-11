/*! @mainpage Examen Promocional
 *
 * @section genDesc Recuperatorio del examen promocional
 *
 * |    Barrera    |    ESP32   	|
 * |:-------------:|:---------- ----|
 * | 	PIN        |   	GPIO_1		|
 * 
 * |   Balanza     |    ESP32   	|
 * |:-------------:|:---------- ----|
 * | 	galga 1    |     CH1 	    |
 * | 	galga 2    |   	 CH2		|
 * 
 * |    HC-SR04     |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO	 	| 	GPIO_3		|
 * | 	TRIGGER	 	| 	GPIO_2		|
 * | 	 +5V 	 	| 	 +5V		|
 * | 	 GND 	 	| 	 GND		|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Examen Promocional Recuperatorio               |
 *
 * @author Giorgi Josefina (josefina.giorgi@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hc_sr04.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
#include "led.h"
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/
/** @brief Periodo de tiempo para la tarea de tomar distancia en microsegundos. */
#define CONFIG_BLINK_PERIOD_TOMAR_DISTANCIA 100000 //son 0,1 seg
/** @brief Periodo de tiempo para la tarea de medir en la balanza en microsegundos. */
#define CONFIG_BLINK_PERIOD_BALANZA 5000 //son 0,005 seg

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
 * @brief Handle de la tarea de tomar distancia.
 */
TaskHandle_t tomarDistancia_task_handle = NULL;
/**
 * @brief Handle de la tarea de medir en la balanza.
 */
TaskHandle_t medidaBalanza_task_handle = NULL;
/**
 * @brief Variable que almacena la velocidad medida en m/s.
 */
float velocidad;
/**
 * @brief Variable que almacena la velocidad maxima medida en m/s.
 */
float velocidadmax;
/**
 * @brief Variable que almacena el peso medido en kg.
 */
float peso;
/*==================[internal functions declaration]=========================*/
/**
 * @brief Tarea encargada de tomar medidas de distancia.
 *
 * Esta tarea se ejecuta continuamente. Mide la distancia utilizando el sensor HcSr04. La tarea se desbloquea
 * mediante notificaciones para realizar la medición. Calcula la velocidad instantanea del camion en cada ejecucion.
 * Ademas llama a la funcion Seleccion de Leds cuando la distancia es menor a 10m.
 * Tambien obtiene la velocidad maxima y la almacena en una variable global.
 * Por ultimo si es 0 la velocidad, el vehicuo esta quieto y notifica a la funcion medir balanza.
 * 
 * @param pvParameter Parámetro pasado a la tarea (no utilizado en este caso).
 */
static void TomarDistancia(void *pvParameter) {
    uint16_t distancia;
	uint16_t distanciaaux=0;
	float aux=0;
	while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    	 distancia = HcSr04ReadDistanceInCentimeters();
		 distanciaaux = distancia;
		
		if(distancia<1000){
		
		 	velocidad = (distancia*100)-(distanciaaux*100)/(0,1); // Velocidad en m/s
		 	
			SeleccionLED(velocidad);
		 	
			if(velocidad>aux)
			{
				velocidadmax=velocidad;
				velocidadmax=aux;
			}
		
		}

		if(velocidad==0){
			vTaskNotifyGiveFromISR(medidaBalanza_task_handle,pdFALSE);
		}
		
		else{
			LedsOffAll();
		}
	
    }
}

/**
 * @brief Controla el encendido de los LEDs según la velocidad.
 *
 * - Prende el LED 3 si la velocidad es mayor a 8m/s.
 * - Prende el LED 2 si la velocidad es menor a 8m/s y mayor a 0m/s.
 * - Prende el LED 1 si la velocidad es 0m/s.
 * 
 * @param velocidad velocidad calculada en m/s.
 */
static void SeleccionLED(float velocidad) {
    if (velocidad > 8) {
        LedOn(LED_3);
    } else if ((velocidad > 0) && (velocidad < 8)) {
        LedOn(LED_2);     
    } else if (velocidad == 0) {
        LedOn(LED_1);
    }
}

/**
 * @brief  Tarea encargada de medir el el peso en cada galga.
 * Esta medida obtenida analogicamente, se promedia y se suma, luego se convierte a kg.
 * Una vez convertida se llama a la funcion que envia infromacion a la PC.
 * 
 * @param pvParameter Parámetro pasado a la tarea (no utilizado en este caso).
 */
static void MedidaBalanza(void *pvParameter){
float medida1[50];
float medida2[50];
float promedio1 = 0;
float promedio2 = 0 ;
	while (1) {
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        for(uint8_t i=0; i<50; i++){
		AnalogInputReadSingle(CH1, &medida1[i]);
		AnalogInputReadSingle(CH2, &medida2[i]);	
		}
		float total1 = 0;
		float total2 = 0;

		for(uint8_t i=0; i<50; i++){
			total1 = total1 + medida1[i];
			total2 = total2 + medida2[i];
		}
		promedio1=total1/50;
		promedio2=total2/50;

		float medidaGalga1;
		float medidaGalga2;
	
		medidaGalga1=(promedio1*1000*20000)/3,3; //convierto de mV a V y luego de V a Kg
		medidaGalga1=(promedio2*1000*20000)/3,3; //convierto de mV a V y luego de V a Kg

		peso = medidaGalga1+medidaGalga2;

		EnviarNotificacion(peso);

	}
}

/**
 * @brief  Funcion encargada de enviar una notificacion por la UART, especificada como el examen lo solicitaba.
 * 
 * @param peso peso medido por la balanza
 */
static void EnviarNotificacion(float peso) {
   	{
        UartSendString(UART_PC,"Peso: ");
		UartSendString(UART_PC,(const char*)UartItoa(peso,10));
		UartSendString(UART_PC," kg\n");
		UartSendString(UART_PC,"Velocidad maxima: ");
		UartSendString(UART_PC,(const char*)UartItoa(velocidadmax,10));
		UartSendString(UART_PC," m/s\n");
    }
}

/**
 * @brief  Funcion encargada controlar desde la PC la apertura y cierre de una
barrera. Ante el envío de un carácter 'o' se deberá abrir la barrera y con una 'c', cerrarla
 * @param param no utilizado
 */
void funcBarrera(){
	uint8_t entradaTeclado;
	UartReadByte(UART_PC,&entradaTeclado);
	switch (entradaTeclado)
	{
	case'o':
		GPIOOn(GPIO_1);
		break;
	case 'c':
		GPIOOff(GPIO_1);
		break;
	default:
		break;
	}		
}

/**
 * @brief Función para notificar la tarea de tomar distancia.
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
 * @brief Función para notificar la tarea de medir con la balanza.
 *
 * Envía una notificación a la tarea de sensar distancia para indicar que debe
 * realizar una medición. Se llama desde la ISR del timer.
 * 
 * @param param Parámetro pasado a la función (no utilizado en este caso).
 */
void funcMedidaBalanza(void* param) {
    vTaskNotifyGiveFromISR(medidaBalanza_task_handle, pdFALSE);
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

// Inicialización de timer distancia
    timer_config_t timer_tomarDistancia = {
        .timer = TIMER_A,
        .period = CONFIG_BLINK_PERIOD_TOMAR_DISTANCIA,
        .func_p = funcTomarDistancia,
        .param_p = NULL
    };
    TimerInit(&timer_tomarDistancia);
	TimerStart(&timer_tomarDistancia);
xTaskCreate(&TomarDistancia, "Tomar distancia", 2048, NULL, 5, &tomarDistancia_task_handle);

//Inicializacion timer balanza
 timer_config_t timerBalanza = {
        .timer = TIMER_B,
        .period = CONFIG_BLINK_PERIOD_BALANZA,
        .func_p = funcMedidaBalanza,
        .param_p = NULL
    };
    TimerInit(&timerBalanza);
    TimerStart(&timerBalanza);
xTaskCreate(&MedidaBalanza,"Balanza", 2048, NULL, 5, &medidaBalanza_task_handle);

//Defino conversion AD CH1 y CH2 correspondiente a cada galga.
analog_input_config_t conversorAD = {
        .input = CH1,
        .mode = ADC_SINGLE,
    };
    AnalogInputInit(&conversorAD);

analog_input_config_t conversorAD = {
        .input = CH2,
        .mode = ADC_SINGLE,
    };
    AnalogInputInit(&conversorAD);

serial_config_t configSerie = {
		.port = UART_PC,
        .baud_rate = 115200,
        .func_p = funcBarrera,
        .param_p = NULL,
	};
	UartInit(&configSerie);

gpioConf_t pin={GPIO_1,GPIO_OUTPUT};
GPIOInit(GPIO_1,GPIO_OUTPUT);

}
/*==================[end of file]============================================*/