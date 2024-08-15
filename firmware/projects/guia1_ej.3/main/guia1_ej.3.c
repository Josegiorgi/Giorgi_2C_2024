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
 * | 8/08/2024 | Ejercicio 3 de Guia 1		                         |
 *
 * @author Josefina Giorgi\ (josefina.giorgi@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
#define ON  1
#define OFF  2
#define TOGGLE  3
/*==================[internal data definition]===============================*/
struct leds
{
    uint8_t mode;       //ON, OFF, TOGGLE
	uint8_t n_led;      //indica el número de led a controlar
	uint8_t n_ciclos;   //indica la cantidad de ciclos de ncendido/apagado
	uint16_t periodo;   //indica el tiempo de cada ciclo
} my_leds;
/*==================[internal functions declaration]=========================*/
void controladorLEDs(struct leds *myLeds){
switch(myLeds->mode){
    		case(ON):
				switch (myLeds->n_led)
				{
					case(1):
						LedOn(LED_1);
					break;

					case(2):
						LedOn(LED_2);
					break;

					case(3):
						LedOn(LED_3);
					break;
				}
			break;

			case(OFF) :
				switch (myLeds->n_led)
				{
					case (1):
						LedOff(LED_1);
					break;

					case(2):
						LedOff(LED_2);
					break;

					case (3):
						LedOff(LED_3);
					break;
				}
    		break;

			case(TOGGLE):	
    			for(int i=0; i<myLeds->n_ciclos; i++)
				{
					switch (myLeds->n_led)
				{
					case (1):
						LedToggle(LED_1);
					break;

					case(2):
						LedToggle(LED_2);
					break;

					case (3):
						LedToggle(LED_3);
					break;
				}

					int retardoAux = myLeds->periodo/100;
					for(int j=0;j<retardoAux;j++)
					{
						vTaskDelay(100/ portTICK_PERIOD_MS);
					}	
				}
    		break;

    	}
}
/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit();
	struct leds LEDS;

	LEDS.mode = 3;
	LEDS.n_led = 3;
	LEDS.n_ciclos = 2000;
	LEDS.n_ciclos = LEDS.n_ciclos*2; //La función TOGGLE realiza cambios, no ciclos completos. Por lo que si no agrego esta línea, en vez de realizarse 10 ciclos encendido/apagado, se realizan la mitad.
	LEDS.periodo = 500;
	controladorLEDs(&LEDS);
}
/*==================[end of file]============================================*/