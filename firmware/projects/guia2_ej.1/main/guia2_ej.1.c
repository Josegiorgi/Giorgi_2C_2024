/*! @mainpage Template
 *
 * @brief Este archivo contiene la aplicación principal para controlar LEDs basados en las mediciones de un sensor ultrasónico,
 * utilizando un ESP32 e interactuando con una pantalla LCD y pulsadores.
 *
 * El programa toma continuamente mediciones de distancia usando un sensor ultrasónico, muestra las mediciones
 * en una pantalla LCD y enciende o apaga los LEDs según la distancia medida. También permite al usuario
 * controlar si el sistema está activo o en pausa mediante pulsadores.
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
 * | 5/09/2024  |     Ejercicio guia 2, ejercicio 1              |
 *
 * @author Giorgi Josefina (josefina.giorgi@ingenieria.uner.edu.ar)
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
#include"switch.h"
/*==================[macros and definitions]=================================*/

/*==================[macros y definiciones]===================================*/
/**
 * @brief Periodo de retardo en milisegundos para la tarea de mostrar la medida.
 */
#define CONFIG_BLINK_PERIOD_MOSTRAR_MEDIDA 500  
/**
 * @brief Periodo de retardo en milisegundos para la tarea de tomar la medida.
 */
#define CONFIG_BLINK_PERIOD_TOMAR_MEDIDA 1000  
/**
 * @brief Periodo de retardo en milisegundos para la tarea de lectura de botones.
 */
#define CONFIG_BLINK_PERIOD_TECLAS 100  

/*==================[internal data definition]===============================*/

/** 
 * @brief Handle para la tarea de tomar medidas.
 * 
 * Este handle se usa para controlar y gestionar la tarea que se encarga de 
 * tomar las medidas de distancia.
 */
TaskHandle_t tomarMedida_task_handle = NULL;

/** 
 * @brief Handle para la tarea de mostrar medidas.
 * 
 * Este handle se utiliza para gestionar la tarea que muestra las medidas 
 * obtenidas.
 */
TaskHandle_t mostrarMedida_task_handle = NULL;

/** 
 * @brief Handle para la tarea que gestiona las teclas.
 * 
 * Este handle controla la tarea encargada de leer e interpretar las 
 * entradas de las teclas.
 */
TaskHandle_t teclas_task_handle = NULL;

/** 
 * @brief Almacena la distancia medida.
 * 
 * Esta variable de 16 bits se usa para almacenar la distancia calculada por 
 * el sensor.
 */
uint16_t distancia;

/** 
 * @brief Almacena el estado de las teclas.
 * 
 * Esta variable de 8 bits contiene el valor de las teclas presionadas. 
 */
uint8_t teclas;

/** 
 * @brief Indica el estado de encendido.
 * 
 * Esta variable booleana se utiliza para indicar si el sistema está 
 * encendido o apagado.
 */
bool encendido = false;

/** 
 * @brief Indica si el sistema está en modo hold.
 * 
 * Esta variable booleana indica si el sistema está en un estado de 
 * retención (hold), en el que no se actualizan las medidas.
 */
bool hold = true;

/*==================[internal functions declaration]=========================*/
/**
 * @brief Controla el estado de los LEDs en función de la distancia medida.
 *
 * Esta función enciende o apaga los LEDs según el valor de la distancia proporcionada.
 * Si la distancia es menor a 10, apaga todos los LEDs.
 * Si está entre 10 y 20, enciende el LED 1.
 * Si está entre 20 y 30, enciende el LED 1 y el LED 2.
 * Si la distancia es mayor a 30, enciende todos los LEDs.
 * 
 * @param distancia Distancia medida en centímetros.
 */
static void SeleccionLED(uint16_t distancia) {
    if (distancia < 10) {
        LedOff(LED_1);
        LedOff(LED_2);
        LedOff(LED_3);
    }
    if ((distancia > 10) && (distancia < 20)) {
        LedOn(LED_1);
        LedOff(LED_2);
        LedOff(LED_3);
    }
    if ((distancia > 20) && (distancia < 30)) {
        LedOn(LED_1);
        LedOn(LED_2);
        LedOff(LED_3);
    }
    if (distancia > 30) {
        LedOn(LED_1);
        LedOn(LED_2);
        LedOn(LED_3);
    }
}

/**
 * @brief Tarea encargada de tomar medidas de distancia.
 *
 * Esta tarea se ejecuta de manera continua mientras el sistema esté encendido,
 * midiendo la distancia utilizando el sensor HcSr04. Los datos se actualizan
 * con una periodicidad configurada.
 * 
 * @param pvParameter Parámetro pasado a la tarea (no utilizado en este caso).
 */
static void TomarMedida(void *pvParameter) {
    while (1) {
        if (encendido) {
            distancia = HcSr04ReadDistanceInCentimeters();
        }
        vTaskDelay(CONFIG_BLINK_PERIOD_TOMAR_MEDIDA / portTICK_PERIOD_MS);
    }
}

/**
 * @brief Tarea encargada de mostrar la medida de distancia y controlar los LEDs.
 *
 * Esta tarea se encarga de actualizar los LEDs en función de la distancia medida
 * y de mostrarla en una pantalla LCD. Si el sistema está en modo "hold", la medida
 * se muestra en la pantalla; de lo contrario, se apagan tanto los LEDs como la pantalla.
 * 
 * @param pvParameter Parámetro pasado a la tarea (no utilizado en este caso).
 */
static void MostrarMedida(void *pvParameter) {
    while (1) {
        if (encendido) {
            SeleccionLED(distancia);
            if (hold) {
                LcdItsE0803Write(distancia);
            }
        } else {
            LedsOffAll();
            LcdItsE0803Off();
        }
        vTaskDelay(CONFIG_BLINK_PERIOD_MOSTRAR_MEDIDA / portTICK_PERIOD_MS);
    }
}

/**
 * @brief Tarea encargada de gestionar la entrada de teclas.
 *
 * Esta tarea lee el estado de las teclas y, según la tecla presionada, alterna
 * el estado de encendido/apagado del sistema o activa/desactiva el modo "hold".
 * 
 * @param pvParameter Parámetro pasado a la tarea (no utilizado en este caso).
 */
static void Teclas(void *pvParameter) {
    while (1) {
        teclas = SwitchesRead();
        switch (teclas) {
            case SWITCH_1:
                encendido = !encendido;
                vTaskDelay(CONFIG_BLINK_PERIOD_TECLAS / portTICK_PERIOD_MS);
                break;
            case SWITCH_2:
                hold = !hold;
                vTaskDelay(CONFIG_BLINK_PERIOD_TECLAS / portTICK_PERIOD_MS);
                break;
        }
    }
}

/*==================[external functions definition]==========================*/
/**
 * @brief Función principal de la aplicación.
 *
 * Inicializa los periféricos necesarios (LEDs, teclas, sensor de distancia, pantalla LCD)
 * y crea las tareas de medición, visualización y gestión de teclas.
 */
void app_main(void) {
    LedsInit();
    SwitchesInit();
    HcSr04Init(GPIO_3, GPIO_2);
    LcdItsE0803Init();

    xTaskCreate(&TomarMedida, "Tomar Medida", 2048, NULL, 5, &tomarMedida_task_handle);
    xTaskCreate(&MostrarMedida, "Mostrar Medida", 512, NULL, 5, &mostrarMedida_task_handle);
    xTaskCreate(&Teclas, "Teclas", 512, NULL, 5, &teclas_task_handle);
}

/*==================[end of file]============================================*/