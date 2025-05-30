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
 * | 	VCC		 	| 	VCC			|
 * |:--------------:|:--------------|
 * | 	DT		 	| 	GPIO17		|
 * |:--------------:|:--------------|
 * | 	sck		 	| 	GPIO15		|
 * |:--------------:|:--------------|
 * | 	GND		 	| 	GND			|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 23/05/2025 | Document creation		                         |
 * |:----------:|:-----------------------------------------------|
 * | 23/05/2025 | Creación de la tarea mostrar                   |
 * |:----------:|:-----------------------------------------------|
 * | 23/05/2025 | función prender leds        			         |
 *
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
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/
#define PERIODO_MUESTREO 12.5 * 100000 // 80Hz

/*==================[internal data definition]===============================*/

TaskHandle_t medir_task_handle = NULL;
uint32_t medicion;
bool conmutarMedicion = true;

/*==================[internal functions declaration]=========================*/
void FuncTimerTareas(void *param)
{
	vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);
}

void prenderLeds()
{
	if (conmutarMedicion)
		{

			if (medicion < 2)
			{
				LedOff(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
			}
			else if (medicion < 3)
			{
				LedOn(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
			}
			else if (medicion < 4)
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOff(LED_3);
			}
			else if (medicion >= 4)
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
		}
		else
		{
			LedOff(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
		}
}

static void TareaMedir(void *pvParameter)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		medicion = 5000000; //HX711_read(); // leo el dato que me manda la placa
		LedToggle(LED_3); 
		// función para prender los leds
		// prenderLeds(); 
		// mandar los datos por puerto serie
		UartSendString(UART_PC, (char *)UartItoa(medicion, 10));
		// UartSendString(UART_PC, (char*)medicion);
		UartSendString(UART_PC, "\n");
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	// inits
	LedsInit();
	HX711_Init(64, GPIO_15, GPIO_17);
	//HX711_read();
	LedToggle(LED_1); 
	timer_config_t timer_medicion = {// configuración del timer para tomar la medición
									 .timer = TIMER_A,
									 .period = PERIODO_MUESTREO,
									 .func_p = FuncTimerTareas,
									 .param_p = NULL};
	TimerInit(&timer_medicion);
	LedToggle(LED_2); 
	serial_config_t my_uart = {// configuración de la uart
							   .port = UART_PC,
							   .baud_rate = 115200,
							   .func_p = NULL, // función que se activa cuando hay una interrupción
							   .param_p = NULL};
	UartInit(&my_uart);
	// creo que acá tengo que configurar la medición

	// puedo hacer un umbral a partil del cual grafique la fuerza
	
	xTaskCreate(&TareaMedir, "toma la medición y la procesa", 4096, NULL, 5, &medir_task_handle);

	TimerStart(timer_medicion.timer);
}
/*==================[end of file]============================================*/