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
 * |:--------------:|:--------------|
 * |   Display      |   EDU-CIAA	|
 * |:--------------:|:-------------:|
 * | 	Vcc 	    |	5V      	|
 * | 	BCD1		| 	GPIO_20		|
 * | 	BCD2	 	| 	GPIO_21		|
 * | 	BCD3	 	| 	GPIO_22		|
 * | 	BCD4	 	| 	GPIO_23		|
 * | 	SEL1	 	| 	GPIO_19		|
 * | 	SEL2	 	| 	GPIO_18		|
 * | 	SEL3	 	| 	GPIO_9		|
 * | 	Gnd 	    | 	GND     	|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 09/05/2025 | Document creation.	                         |
 * | 25/04/2025 | Se modifica el ejercicio 2                     |
 * | 09/05/2025 | Se agregan los timers                          |
 *
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
#include "uart_mcu.h"
/*==================[macros and definitions]=================================*/
TaskHandle_t medir_task_handle = NULL;
TaskHandle_t mostrar_task_handle = NULL;
TaskHandle_t uart_task_handle = NULL;
TaskHandle_t DetectarSwitch_task_handle = NULL;
bool conmutarMedicion = false;
bool handle = false; 
uint16_t medicionGuardada;
uint16_t medicion;

#define ON 0
#define OFF 1

#define DELAY_TAREA 1000000 //el timer necesita microsegundos
/*==================[internal data definition]===============================*/
void FuncTimerTareas(void* param){
	vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE); 
	vTaskNotifyGiveFromISR(mostrar_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(uart_task_handle, pdFALSE); 
	
}

/*void FuncUart(void* param) {
	uint8_t caracter;
	UartReadByte (UART_PC, &caracter);
	UartSendByte (UART_PC, (char *) &caracter);
	}*/


/*==================[internal functions declaration]=========================*/

static void TareaMedir(void *pvParameter)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
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

	}
}

void CambiarEstado(){
	conmutarMedicion = !conmutarMedicion;
}

void GuardarMedicion(){
	handle = !handle; 
	medicionGuardada = medicion; 
}

static void TareaMostrar(void *pvParameter)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(handle)
		{
			LcdItsE0803Write(medicionGuardada);
		}else if(!handle){
			LcdItsE0803Write(medicion);
		}

	}
	
}

void UartTask(void* param) {
	
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		UartSendString(UART_PC, (char*)UartItoa(medicion, 10)); 
		UartSendString(UART_PC, " cm\n"); 

	}
	
}

/*==================[external functions definition]==========================*/
void app_main(void)
{

	LedsInit();
	HcSr04Init(GPIO_3, GPIO_2);
	SwitchesInit(); 
	SwitchActivInt(SWITCH_1, CambiarEstado, NULL); 
	SwitchActivInt(SWITCH_2, GuardarMedicion, NULL); 
	LcdItsE0803Init(); 

timer_config_t timer_Tareas = {
        .timer = TIMER_A,
        .period = DELAY_TAREA,
        .func_p = FuncTimerTareas,
        .param_p = NULL
    };
    TimerInit(&timer_Tareas);

serial_config_t my_uart = {
		.port = UART_PC,
		.baud_rate = 9600,
		.func_p = NULL,
		.param_p = NULL
		}; 
		UartInit(&my_uart); 
    
	xTaskCreate(&TareaMedir, "Medir", 512, NULL, 5, &medir_task_handle);
	xTaskCreate(&TareaMostrar, "Mostrar", 512, NULL, 5, &mostrar_task_handle);
	xTaskCreate(&UartTask, "UART", 512, &my_uart, 5, &uart_task_handle);

	TimerStart(timer_Tareas.timer); 
}
/*==================[end of file]============================================*/