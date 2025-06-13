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
 * | 	DT		 	| 	GPIO19		|
 * |:--------------:|:--------------|
 * | 	sck		 	| 	GPIO18		|
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
#include "switch.h"

/*==================[macros and definitions]=================================*/
#define PERIODO_MUESTREO 12.5 * 10000 // 80Hz

/*==================[internal data definition]===============================*/

TaskHandle_t medir_task_handle = NULL;
TaskHandle_t temporal_task_handle = NULL;
float medicion;
char sMed[10];
char sTiempo[10];
bool medicionContinua = false;
bool medicionTemporal = false;
int auxContador = 0;

/*==================[internal functions declaration]=========================*/
void FuncTimerTareas(void *param)
{
	vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(temporal_task_handle, pdFAIL);
}

void prenderLeds()
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

void ModoMedicionContinua()
{

	medicionContinua = !medicionContinua;
}

void ModoMedicionTemporal()
{
	medicionTemporal = true;
}

static void TareaMedir(void *pvParameter)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (medicionContinua == true)
		{
			medicion = HX711_get_units(1); // leo el dato que me manda la placa
			// función para prender los leds
			prenderLeds();
			// mandar los datos por puerto serie
			sprintf(sMed, "%.2f\n", medicion);
			UartSendString(UART_PC, "$");
			UartSendString(UART_PC, sMed);
			UartSendString(UART_PC, ";");
		}
	}
}

static void TareaTemporal(void *pvParameter)
{
	float umbral = 2;
	bool inicio_medicion = false;
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (medicionTemporal)
		{
			// tengo que calcular cuantas veces se mantuvo la medición y multiplicarlo por el periodo de muestreo
			if (!inicio_medicion)
			{
				medicion = HX711_get_units(1);
				prenderLeds();

				if (medicion > umbral)
					inicio_medicion = true;

				sprintf(sMed, "%.2f\n", medicion);
				UartSendString(UART_PC, "$");
				UartSendString(UART_PC, sMed);
				UartSendString(UART_PC, ";");
			}
			else
			{
				if (umbral < medicion)
				{
					medicion = HX711_get_units(1);
					prenderLeds();

					sprintf(sMed, "%.2f\n", medicion);
					UartSendString(UART_PC, "$");
					UartSendString(UART_PC, sMed);
					UartSendString(UART_PC, ";");
					auxContador++;
				}
				else
				{
					float tiempoSostenido = 0.125 * auxContador;
					sprintf(sTiempo, "%.2f\n", tiempoSostenido);
					UartSendString(UART_PC, "$");
					UartSendString(UART_PC, "Se sostuvo durante ");
					UartSendString(UART_PC, sTiempo);
					UartSendString(UART_PC, "s");
					UartSendString(UART_PC, ";");
					medicionTemporal = false;
					auxContador = 0;
					inicio_medicion = false;
				}
			}
		}
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	// inits
	LedsInit();
	HX711_Init(64, GPIO_18, GPIO_19);
	HX711_setScale(2182.0);
	HX711_tare(10);

	SwitchesInit();
	SwitchActivInt(SWITCH_1, ModoMedicionContinua, NULL);
	SwitchActivInt(SWITCH_2, ModoMedicionTemporal, NULL);

	timer_config_t timer_medicion = {// configuración del timer para tomar la medición
									 .timer = TIMER_A,
									 .period = PERIODO_MUESTREO,
									 .func_p = FuncTimerTareas,
									 .param_p = NULL};
	TimerInit(&timer_medicion);

	serial_config_t my_uart = {// configuración de la uart
							   .port = UART_PC,
							   .baud_rate = 9600,
							   .func_p = NULL, // función que se activa cuando hay una interrupción
							   .param_p = NULL};
	UartInit(&my_uart);
	// creo que acá tengo que configurar la medición

	// puedo hacer un umbral a partil del cual grafique la fuerza

	xTaskCreate(&TareaMedir, "toma la medición y la procesa", 4096, NULL, 5, &medir_task_handle);
	xTaskCreate(&TareaTemporal, "mide e informa el tiempo que se sostuvo", 4096, NULL, 5, &temporal_task_handle);

	TimerStart(timer_medicion.timer);
}
/*==================[end of file]============================================*/