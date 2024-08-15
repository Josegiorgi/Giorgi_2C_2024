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
 * | 15/08/2024 | Ejercicio 6 de guia 1		                         |
 *
 * @author Giorgi Josefina (josefina.giorgi@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <gpio_mcu.h>

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;
/*==================[internal functions declaration]=========================*/
int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{
	for(int i=2; i<digits; i--){
		bcd_number[i]=data%10;
		data=data/10;
	}
return 1;
}

void BCDtoPin (uint8_t digitBCD, gpioConf_t *gpio){   //Funcion que incializa y compara bit a bit
	uint8_t mask=1;
	for(int i=0;i<4;i++)
	{
		GPIOInit(gpio[i].pin,gpio[i].dir);
	}
	for(int j=0;j<4;j++){
		if((mask & digitBCD) != 0){
			GPIOOn(gpio[j].pin);
			}
			else{
				GPIOOff(gpio[j].pin);
			}
		mask = mask<<1;
	}

}
funcion6 ()
{
	convertToBcdArray()

}
/*==================[external functions definition]==========================*/
void app_main(void){

	
	uint8_t valor = 5;
	gpioConf_t pines[4]={{GPIO_20,GPIO_OUTPUT},{GPIO_21,GPIO_OUTPUT},{GPIO_22,GPIO_OUTPUT},{GPIO_23,GPIO_OUTPUT}}; //Vector de struct para incializar GPIO
	

}
/*==================[end of file]============================================*/