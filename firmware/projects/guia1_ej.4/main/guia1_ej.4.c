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
 * | 15/08/2024 | Ejercicio 4 de guia 1	                         |
 *
 * @author Josefina Giorgi (josefina.giorgi@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{
	for(int i=0; i<digits; i++){
		bcd_number[i]=data%10;
		data=data/10;
	}
return 1;
}


/*==================[external functions definition]==========================*/
void app_main(void){
	uint8_t mi_vector[3];
	uint32_t numero = 187;
	uint8_t digitos = 3;
	convertToBcdArray(numero,digitos,mi_vector);

 printf("El tecer numero:\n");
	printf ("%d", mi_vector[0]);
 printf("\nEl segundo numero:\n");
	printf ("%d", mi_vector[1]);
 printf("\nEl primer numero:\n");
	printf ("%d", mi_vector[2]);
}
/*==================[end of file]============================================*/