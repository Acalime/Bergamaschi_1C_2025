/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Utiliza un sensor ultrasònico para medir la distancia a la que se encuentra un objeto.
 * Muestra la distancia medida en cm por un display.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO	 	| 	GPIO_3		|
 * |:--------------:|:--------------|
 * | 	TRIGGER	 	| 	GPIO_2		|
 * |:--------------:|:--------------|
 * | 	+5V		 	| 	+5V			|
 * |:--------------:|:--------------|
 * | 	GND		 	| 	GND			|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/04/2025 | Document creation		                         |
 *
 * @author Micaela Bergamaschi (micaela.bergamaschi@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <led.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "hc_sr04.h"
#include "switch.h"
#include "lcditse0803.h"
/*==================[macros and definitions]=================================*/
TaskHandle_t medir_task_handle = NULL;
TaskHandle_t mostrar_task_handle = NULL;
TaskHandle_t DetectarSwitch_task_handle = NULL;
bool conmutarMedicion = false;
bool handle = false; 
uint16_t medicionGuardada;
uint16_t medicion;

#define ON 0
#define OFF 1

#define DELAY_TAREA 1000
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

static void TareaMedir(void *pvParameter)
{
	while (true)
	{
		
		if (conmutarMedicion)
		{
			medicion = HcSr04ReadDistanceInCentimeters();

			if (medicion < 10)
			{
				LedOff(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
			}
			else if (medicion < 20)
			{
				LedOn(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
			}
			else if (medicion < 30)
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOff(LED_3);
			}
			else if (medicion >= 30)
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOn(LED_3);
			}
			else
			{
				LedOff(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
			}
		}else {
			LedOff(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
		}

		vTaskDelay(DELAY_TAREA / portTICK_PERIOD_MS);
	}
}

static void TareaDetectarSwitch(void *pvParameter)
{
	uint8_t teclas;
	while(true)    {
    	teclas  = SwitchesRead();
    	switch(teclas){
    		case SWITCH_1:
    			conmutarMedicion = !conmutarMedicion;
    		break;
			case SWITCH_2:
    			handle = !handle; 
				medicionGuardada = medicion; 
    		break;
    	}
		vTaskDelay(DELAY_TAREA / portTICK_PERIOD_MS);
	}
}

static void TareaMostrar(void *pvParameter)
{
	while (true)
	{
		if(handle)
		{
			LcdItsE0803Write(medicionGuardada);
		}else if(!handle){
			LcdItsE0803Write(medicion);
		}
		
		vTaskDelay(DELAY_TAREA / portTICK_PERIOD_MS);	
	}
	
}

/*==================[external functions definition]==========================*/
void app_main(void)
{

	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2);
	SwitchesInit(); 
	LcdItsE0803Init(); 


	xTaskCreate(&TareaMedir, "Medir", 512, NULL, 5, &medir_task_handle);
	xTaskCreate(&TareaDetectarSwitch, "Detectar Switch", 512, NULL, 5, &DetectarSwitch_task_handle);
	xTaskCreate(&TareaMostrar, "Mostrar", 512, NULL, 5, &mostrar_task_handle);
}
/*==================[end of file]============================================*/