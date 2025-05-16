/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Utiliza un sensor ultrasònico para medir la distancia a la que se encuentra un objeto.
 * Muestra la distancia medida en cm por un display. Además, se pueden usar las teclas de 
 * la placa, o las teclas "o" para empezar y detener la medición, y la tecla "H" para mantener el 
 * valor en el display. 
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
#include "ctype.h"
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

static void TareaMedir(void *pvParameter) //tarea que se encarga de medir
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

void CambiarEstado(){ //función que se encarga de empezar o detener la medición 
	conmutarMedicion = !conmutarMedicion;
}

void GuardarMedicion(){ //función que se encarga de guardar la medición tomada 
	handle = !handle; 
	medicionGuardada = medicion; 
}

void LeerTeclado(){

	uint8_t tecla; 
	UartReadByte(UART_PC, &tecla);

	if(toupper(tecla) == 'O')
	{
		CambiarEstado(); 
		UartSendString(UART_PC, "Medir/Detener\n");
	}else if(toupper(tecla) == 'H'){
		GuardarMedicion(); 
		UartSendString(UART_PC, "Hold\n");

	}else UartSendString(UART_PC, "caracter no valido\n");

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

	LedsInit(); //inicializa los leds
	HcSr04Init(GPIO_3, GPIO_2); //inicializa el medidior de ultrasonido
	SwitchesInit(); //inicializa los switches
	SwitchActivInt(SWITCH_1, CambiarEstado, NULL); //interrupción switch 1
	SwitchActivInt(SWITCH_2, GuardarMedicion, NULL); //interrupción switch 2
	LcdItsE0803Init();  //inicializa el display

timer_config_t timer_Tareas = { //configuración del timer
        .timer = TIMER_A,
        .period = DELAY_TAREA,
        .func_p = FuncTimerTareas,
        .param_p = NULL
    };
    TimerInit(&timer_Tareas);

serial_config_t my_uart = { //configuración de la uart
		.port = UART_PC,
		.baud_rate = 9600,
		.func_p = LeerTeclado, //función que se activa cuando hay una interrupción 
		.param_p = NULL
		}; 
		UartInit(&my_uart); 
    
	xTaskCreate(&TareaMedir, "Medir", 512, NULL, 5, &medir_task_handle); //tarea que se encarga de medir
	xTaskCreate(&TareaMostrar, "Mostrar", 512, NULL, 5, &mostrar_task_handle); //tarea que se encarga de mostrar la medición 
	xTaskCreate(&UartTask, "UART", 512, &my_uart, 5, &uart_task_handle); //tarea que se encarga de la comunicación serie

	TimerStart(timer_Tareas.timer); 
}
/*==================[end of file]============================================*/