/*! @mainpage Ejercicio 6 del proyecto 1.
 *
 * @section genDesc General Description: El programa permite ingresar un numero de 3 cifras y este mismo numero
 * es mostrado en el display LCD de manera secuencial, para lograr esto utilizamos las funciones de los ejercicio anteriores
 * que permiten pasar a BCD y al display de manera ordenada. 
 *
 *
 * @section hardConn Hardware Connection
 *
 * |    EDU-ESP     |   PERIFERICO 	|
 * |:--------------:|:--------------|
 * |:   GPIO_20    :|:      D1      |
 * |:   GPIO_21    :|:      D2      |
 * |:   GPIO_22    :|:      D3      |
 * |:   GPIO_23    :|:      D4      |
 * |:   GPIO_19    :|:     SEL_1    |
 * |:   GPIO_18    :|:     SEL_2    |
 * |:   GPIO_9     :|:     SEL_3    |
 * |:    +5V       :|:     +5V      |
 * |:    GND       :|:     GND      |
 * 
 * *
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
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
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

/*==================[internal functions declaration]=========================*/
/**
 * @brief Convierte un número en su representación en BCD.
 *
 * Esta función convierte un número de tipo uint32_t a un array de dígitos en BCD.
 *
 * @param[in] data Número a convertir.
 * @param[in] digits Cantidad de dígitos del número.
 * @param[out] bcd_number Array donde se almacenará la representación BCD del número.
 * @return int8_t Devuelve 1 si la conversión fue exitosa.
 */
int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{
	for(int i=0; i<digits; i++){
		bcd_number[digits-1-i]=data%10;
		data=data/10;
	}
return 1;
}

/**
 * @brief Configura y establece el valor de pines GPIO en función de un dígito BCD.
 *
 * Esta función inicializa los pines GPIO según la configuración pasada como argumento y establece su estado
 * en función del valor del dígito BCD.
 *
 * @param[in] digitBCD Dígito en BCD que se quiere representar.
 * @param[in] gpio Array de estructuras gpioConf_t que define la configuración de los pines GPIO.
 */
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

/**
 * @brief Convierte un valor numérico a BCD y lo muestra en un dispositivo utilizando pines GPIO.
 *
 * Esta función convierte un valor numérico a su representación en BCD, luego inicializa y maneja
 * los pines GPIO para mostrar cada dígito en el dispositivo correspondiente.
 *
 * @param[in] value Valor numérico a convertir y mostrar.
 * @param[in] digit Cantidad de dígitos del número.
 * @param[in] pinBCD Array de estructuras gpioConf_t para los pines BCD.
 * @param[in] pinsMUX Array de estructuras gpioConf_t para los pines MUX.
 */
void funBCDtoLCD (uint32_t value, uint8_t digit, gpioConf_t *pinBCD, gpioConf_t *pinsMUX)
{
	uint8_t vector[3];
	convertToBcdArray(value,digit,vector);
	for (uint8_t i=0; i<3; i++){
		GPIOInit(pinsMUX[i].pin,pinsMUX[i].dir);
	}
	for(uint8_t i=0;i<digit;i++){
		BCDtoPin(vector[i],pinBCD);
		GPIOOn(pinsMUX[i].pin);
		GPIOOff(pinsMUX[i].pin);
		printf ("Sacando numero %d\n",vector[i]);
		vTaskDelay(1000 / portTICK_PERIOD_MS);

	}

}
/*==================[external functions definition]==========================*/
/**
 * @brief Convierte un valor numérico a BCD y lo muestra en un dispositivo utilizando pines GPIO.
 *
 * Esta función convierte un valor numérico a su representación en BCD, luego inicializa y maneja
 * los pines GPIO para mostrar cada dígito en el dispositivo correspondiente.
 *
 * @param[in] value Valor numérico a convertir y mostrar.
 * @param[in] digit Cantidad de dígitos del número.
 * @param[in] pinBCD Array de estructuras gpioConf_t para los pines BCD.
 * @param[in] pinsMUX Array de estructuras gpioConf_t para los pines MUX.
 */
void app_main(void){

	gpioConf_t pines[4]={{GPIO_20,GPIO_OUTPUT},{GPIO_21,GPIO_OUTPUT},{GPIO_22,GPIO_OUTPUT},{GPIO_23,GPIO_OUTPUT}}; //Vector de struct para incializar GPIO
    uint32_t value = 123;
	gpioConf_t pinMUX[3]={{GPIO_19, GPIO_OUTPUT},{GPIO_18, GPIO_OUTPUT},{GPIO_9, GPIO_OUTPUT}};
	printf("Arranca programa\n");
	funBCDtoLCD(value,3,pines,pinMUX);
}
/*==================[end of file]============================================*/