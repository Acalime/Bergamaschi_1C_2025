/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Este proyecto se trata de un dinamómetro digital capaz de medir la fuerza realizada con una
 * persona con la mano. 
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 * 
 * Referencias
 * FM media en hombres 27,5 kg
 * FM media en mujeres 16,7 kg 
 * https://scielo.isciii.es/scielo.php?script=sci_arttext&pid=S0212-16112008000100006
 * 
 * 80 hz -> hoja de datos. para el timer
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
 * | 23/05/2025 | Document creation		                         |
 * |:----------:|:-----------------------------------------------|
 * | 23/05/2025 | Creación de la tarea mostrar                   |
 *
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "led.h"
#include "hx711.h"
#include "timer_mcu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/*==================[macros and definitions]=================================*/
#define PERIODO_MUESTREO 12.5*1000 //80Hz

/*==================[internal data definition]===============================*/

TaskHandle_t medir_task_handle = NULL;


/*==================[internal functions declaration]=========================*/
static void TareaMedir(void *pvParameter){

	while(true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 
		HX711_read();

	}

}


/*==================[external functions definition]==========================*/
void app_main(void){
	//inits
	LedsInit(); 
	HX711_Init(2, CH0, CH1); 


	timer_config_t timer_medicion = { //configuración del timer para tomar la medición
        .timer = TIMER_A,
        .period = PERIODO_MUESTREO,
        .func_p = TaskNotifyGiveFromISR(medir_task_handle, pdFALSE),
        .param_p = NULL
    };
    TimerInit(&timer_medicion);
	//creo que acá tengo que configurar la medición
	
	// puedo hacer un umbral a partil del cual grafique la fuerza  

	xTaskCreate(&TareaMedir, "toma la medición y la procesa", 512, NULL, 5, &medir_task_handle);



}
/*==================[end of file]============================================*/