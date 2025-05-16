/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Programa que digitaliza una señal analógica y la transmite a un graficador de puerto serie de la PC.
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
 * | 16/05/2025 | Document creation		                         |
 * |:----------:|:-----------------------------------------------|
 * | 16/05/2025 | Document creation		                         |
 * 
 *
 * @author Micaela Bergamaschi (micaela.bergamaschi@ingenieria.uner.edu.ar)
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "timer_mcu.h"
#include "analog_io_mcu.h"
#include "uart_mcu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*==================[macros and definitions]=================================*/
#define PERIODO_MUESTREO 2*1000 //2 ms * 1000 me da 2x10^-6
#define BUFFER_SIZE 231


/*==================[internal data definition]===============================*/
TaskHandle_t leerEnviar_task_handle = NULL;
TaskHandle_t generarECG_task_handle = NULL; 

uint16_t datosConvertidos; //variable que almacena todos los datos del conversor (12)


const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};

/*==================[internal functions declaration]=========================*/
void FuncTimerTareas(void* param){
	vTaskNotifyGiveFromISR(leerEnviar_task_handle, pdFALSE); 
	vTaskNotifyGiveFromISR(generarECG_task_handle, pdFALSE); 
}
/*@brief esta tarea lee y envía la señal generada por un potenciómetro*/
static void TareaLeerEnviar(void *pvParameter){
	while(true){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 
		AnalogInputReadSingle(CH1, &datosConvertidos);
		

		UartSendString(UART_PC,"$"); 
		UartSendString(UART_PC, (char *)UartItoa(datosConvertidos, 10)); 
		UartSendString(UART_PC, ";"); 
	}

}
/*@brief esta tarea analogiza un ecg definido anteriormente en un arreglo*/
static void TareaGenerarECG(void *pvParameter){
	int i = 0; 
	while(true){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		//hacer analógico ecg en i
		AnalogOutputWrite(ecg[i]); //escribo el valor y lo convierto en analógico
		i++; 

		if(i == BUFFER_SIZE)
		{
			i = 0; 
		}
		
	}
}
/*==================[external functions definition]==========================*/


void app_main(void){
	timer_config_t timer_interrupciones = { //configuración del timer
        .timer = TIMER_A,
        .period = PERIODO_MUESTREO,
        .func_p = FuncTimerTareas,
        .param_p = NULL
    };
    TimerInit(&timer_interrupciones);

	analog_input_config_t Canal_1 = {	//configuración del CH1
	.input = CH1,			
	.mode = ADC_SINGLE,		//pasa cuando hay interrupciones
	.func_p = NULL,			
	.param_p = NULL,			
	.sample_frec = 0	//solo para continuo
	};
	AnalogInputInit(&Canal_1); 
	AnalogOutputInit(); //como solo tengo 1 opción hago directo esa línea

	serial_config_t my_uart = { //configuración de la uart
		.port = UART_PC,
		.baud_rate = 115200, 
		.func_p = NULL, //función que se activa cuando hay una interrupción 
		.param_p = NULL
		}; 
		UartInit(&my_uart); 

	xTaskCreate(&TareaLeerEnviar, "Leer y Enviar", 512, NULL, 5, &leerEnviar_task_handle);
	xTaskCreate(&TareaGenerarECG, "Genera un ECG", 512, NULL, 5, &generarECG_task_handle);



	TimerStart(timer_interrupciones.timer); 

}
/*==================[end of file]============================================*/