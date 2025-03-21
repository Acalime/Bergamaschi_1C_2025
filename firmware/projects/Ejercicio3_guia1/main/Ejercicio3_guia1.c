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
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#define ON 0
#define OFF 1
#define TOGGLE 2
/*==================[macros and definitions]=================================*/
	struct leds
{
    uint8_t mode;       //ON, OFF, TOGGLE
	uint8_t n_led;        //indica el número de led a controlar
	uint8_t n_ciclos;  //indica la cantidad de ciclos de ncendido/apagado
	uint16_t periodo;    //indica el tiempo de cada ciclo
} my_leds; 

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
void funcion(struct leds* estructura){
switch (estructura->mode)
{
case ON:
{
	switch (estructura->n_led)
		{
		case 1:
			LedOn(LED_1);
			break;
		case 2:
			LedOn(LED_2);
			break;
		case 3:
			LedOn(LED_3);
			break;
		
		default:
			break;
		}
		break;
}
	
case OFF:
	{
	switch (estructura->n_led)
		{
		case 1:
			LedOff(LED_1);
			break;
		case 2:
			LedOff(LED_2);
			break;
		case 3:
			LedOff(LED_3);
			break;
		
		default:
			break;
		}
		break;
}
	break;
case TOGGLE:
	{
	for(int i = 0; i<(2*estructura->n_ciclos); i++){
		switch (estructura->n_led)
		{
			case 1:
				LedToggle(LED_1);
				break;
			case 2:
				LedToggle(LED_2);
				break;
			case 3:
				LedToggle(LED_3);
				break;
			
			default:
				break;
		}
		vTaskDelay(estructura->periodo / portTICK_PERIOD_MS);
		}
	
}
	break;

default:
	break;
}
}
/*==================[external functions definition]==========================*/

void app_main(void){

LedsInit(); 

struct leds ledsito; 
ledsito.mode = OFF; 
ledsito.n_led = 3; 
ledsito.n_ciclos = 10; 
ledsito.periodo = 1000; 
funcion(&ledsito);

struct leds ledsito1; 
ledsito1.mode = OFF; 
ledsito1.n_led = 1;  
funcion(&ledsito1);

struct leds ledsito2; 
ledsito2.mode = TOGGLE; 
ledsito2.n_led = 2; 
ledsito2.n_ciclos = 10; 
ledsito2.periodo = 500;
funcion(&ledsito2);


}
/*==================[end of file]============================================*/