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
 * | 5/09/2024  |     Ejercicio guia 2, ejercicio 1              |
 *
 * @author Giorgi Josefina (josefina.giorgi@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include"switch.h"
/*==================[macros and definitions]=================================*/

#define CONFIG_BLINK_PERIOD_MOSTRAR_MEDIDA 500
#define CONFIG_BLINK_PERIOD_TOMAR_MEDIDA 1000
#define CONFIG_BLINK_PERIOD_TECLAS 100
/*==================[internal data definition]===============================*/
TaskHandle_t tomarMedida_task_handle = NULL;
TaskHandle_t mostrarMedida_task_handle = NULL;
TaskHandle_t teclas_task_handle = NULL;
uint16_t distancia;
uint8_t teclas;
bool encendido = false;
bool hold = true;
/*==================[internal functions declaration]=========================*/
static void SeleccionLED(uint16_t distancia ){
    if(distancia<10){
		LedOff(LED_1); 
		LedOff(LED_2); 
		LedOff(LED_3); 
	}
    if((distancia>10) && (distancia<20)){
		LedOn(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);

	}
	if((distancia>20) && (distancia<30)){
		LedOn(LED_1);
		LedOn(LED_2);
		LedOff(LED_3);
	}
	if(distancia>30){
		LedOn(LED_1);
		LedOn(LED_2);
		LedOn(LED_3);
	}
}
static void TomarMedida(void *pvParameter){
	while(1){
	if(encendido){
		distancia = HcSr04ReadDistanceInCentimeters();
	}
	vTaskDelay(CONFIG_BLINK_PERIOD_TOMAR_MEDIDA / portTICK_PERIOD_MS);
}
}
static void MostrarMedida(void *pvParameter){
    while(1){
        if(encendido){
			SeleccionLED(distancia);
			if(hold){
			LcdItsE0803Write(distancia);
			}
		}
		else {
		LedsOffAll();
		LcdItsE0803Off();
     	}
	 vTaskDelay(CONFIG_BLINK_PERIOD_MOSTRAR_MEDIDA / portTICK_PERIOD_MS);
	}
}	 
static void Teclas(void *pvParameter){
	while(1) {
    	teclas = SwitchesRead();
    	switch(teclas){
    		case SWITCH_1:
	        encendido=!encendido;
		    vTaskDelay(CONFIG_BLINK_PERIOD_TECLAS / portTICK_PERIOD_MS);
    		break;
    		case SWITCH_2:
			hold=!hold;
			vTaskDelay(CONFIG_BLINK_PERIOD_TECLAS / portTICK_PERIOD_MS);	
    		break;
	}
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){
LedsInit();
SwitchesInit();
HcSr04Init(GPIO_3,GPIO_2);
LcdItsE0803Init();

xTaskCreate(&TomarMedida, "Tomar Medida", 2048, NULL, 5, &tomarMedida_task_handle);
xTaskCreate(&MostrarMedida, "Mostrar Medida", 512, NULL, 5, &mostrarMedida_task_handle);
xTaskCreate(&Teclas, "Teclas", 512, NULL, 5, &teclas_task_handle);
}

/*==================[end of file]============================================*/