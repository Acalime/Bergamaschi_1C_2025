/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Luego de la misteriosa nevada mortal que cubrió Buenos Aires, un pequeño grupo de sobrevivientes 
 * intenta construir una estación de monitoreo ambiental para detectar indicios de una nueva nevada tóxica 
 * y la presencia de radiación. 
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  				|   ESP32   	|
 * |:------------------------------:|:--------------|
 * | 	sensor de radiación 	 	| 	CH1			|
 * |:------------------------------:|:--------------|
 * | 	VCC     					|	3V3     	|
 * | 	DATA temperatura y humedad  | 	GPIO_9  	|
 * | 	GND		 					| 	GND 		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Micaela Bergamaschi (micaela.bergamaschi@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "dht11.h"
#include "timer_mcu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "uart_mcu.h"
#include "switch.h"
#include "gpio_mcu.h"
#include "led.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
#define PERIODO_RIESGO 1 * 1000 * 1000
#define PERIODO_RADIACION 5 * 1000 * 1000

/*==================[internal data definition]===============================*/
TaskHandle_t riesgo_task_handle = NULL;
TaskHandle_t radiacion_task_handle = NULL;
float humedad; 
float temperatura; 
uint16_t radiacion; 
bool ledVerde = false; 
bool ledAmarillo = false; 
bool ledRojo = false; 
char sTemp[4]; 
char sHum[4]; 
bool encender = false; 


/*==================[internal functions declaration]=========================*/

/**
 * @brief función que notifica a la tarea TareaRiesgo
 */
void FuncTimerRiesgo(void *param)
{
	vTaskNotifyGiveFromISR(riesgo_task_handle, pdFALSE);
}

/**
 * @brief función que notifica a la tarea TareaRadiacion
 */
void FuncTimerRadiacion(void *param)
{
	vTaskNotifyGiveFromISR(radiacion_task_handle, pdFALSE);
}

/**
 * @brief función que se encarga de encender o apagar los leds. 
 */
void PrenderLeds(){
	if(ledVerde){
		LedOn(LED_1); 
		LedOff(LED_2); 
		LedOff(LED_3); 
	}else if (ledAmarillo){
		LedOn(LED_2); 
		LedOff(LED_1); 
		LedOff(LED_3); 
	}else if (ledRojo){
		LedOn(LED_3); 
		LedOff(LED_2); 
		LedOff(LED_1); 
	}
}
/** @brief Tarea que se encarga de medir la temperatura y la humedad del ambniente y decide si hay 
 *         riesgo de nevada */

static void TareaRiesgo(void *pvParameter){
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(encender){
			dht11Read(&humedad, &temperatura); 

		if(humedad >= 85 ){
			if(0<= temperatura && temperatura <= 2){

				ledRojo = true;  
				ledVerde = false; 

				sprintf(sTemp, "%.2f\n", temperatura); //convierte a string los float
				sprintf(sHum, "%.2f\n", humedad);

				UartSendString(UART_PC, "Temperatura: "); 
				UartSendString(UART_PC, sTemp);
				UartSendString(UART_PC, " °C- Humedad: "); 
				UartSendString(UART_PC, sHum); 
				UartSendString(UART_PC, "- RIESGO DE NEVADA\n"); 
			}
		}else {
			ledRojo = false; 
			ledVerde = true; 
			UartSendString(UART_PC, "Temperatura: "); 
			UartSendString(UART_PC, (char*)UartItoa(temperatura, 10));
			UartSendString(UART_PC, " °C- Humedad: "); 
			UartSendString(UART_PC, (char*)UartItoa(humedad, 10));
			UartSendString(UART_PC, "\n");
			
		}

		PrenderLeds(); 
		}
		
	}
	
}
/**
 * @brief Tarea que se encarga de tomar el dato de radiación de la entrada analógica y decide 
 * 		  si hay un nivel demasiado alto de radiación. 
 */
static void TareaRadiacion(void *pvParameter){

	while(true){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(encender){
			AnalogInputReadSingle(CH1, radiacion); //radiacion es un valor entre 0 y 3300

		//convierto a mR/h

		radiacion = (radiacion*100)/3300; 

		//comparo y envío el mensaje

		if(radiacion < 40){
			ledVerde = true; 
			ledAmarillo = false; 
			UartSendString(UART_PC, "Radiación "); 
			UartSendString(UART_PC, (char*)UartItoa(radiacion, 10));
			UartSendString(UART_PC, "mR/h\n"); 

		}else{
			ledVerde = false; 
			ledAmarillo = true; 
			UartSendString(UART_PC, "Radiación "); 
			UartSendString(UART_PC, (char*)UartItoa(radiacion, 10));
			UartSendString(UART_PC, " mR/h- Radiación elevada\n");

		}

		prenderLeds(); 
		}
	}
}

void EncenderDispositivo(){
	encender = true; 
}

void ApagarDispositivo(){
	encender = false; 
}


/*==================[external functions definition]==========================*/
void app_main(void){

	GPIOInit(GPIO_9, GPIO_INPUT); 
	dht11Init(GPIO_9);
	LedsInit(); 
	

	timer_config_t timer_riesgo = { //configuración del timer para la tarea riesgo
        .timer = TIMER_A,
        .period = PERIODO_RIESGO,
        .func_p = FuncTimerRiesgo,
        .param_p = NULL
    };
    TimerInit(&timer_riesgo);

	timer_config_t timer_radiacion = { //configuracion del timer para la tarea radiacion
        .timer = TIMER_A,
        .period = PERIODO_RADIACION,
        .func_p = FuncTimerRadiacion,
        .param_p = NULL
    };
    TimerInit(&timer_radiacion);

	serial_config_t my_uart = {// configuración de la uart
		.port = UART_PC,
		.baud_rate = 9600,
		.func_p = NULL, // función que se activa cuando hay una interrupción
		.param_p = NULL};
	UartInit(&my_uart);

	analog_input_config_t Canal_1 = {	//configuración del CH1
	.input = CH1,			
	.mode = ADC_SINGLE,
	.func_p = NULL,			
	.param_p = NULL,			
	.sample_frec = 0
	};
	AnalogInputInit(&Canal_1); 

	SwitchesInit();
	SwitchActivInt(SWITCH_1, EncenderDispositivo, NULL); 
	SwitchActivInt(SWITCH_2, ApagarDispositivo, NULL);

	xTaskCreate(&TareaRiesgo, "mide y decide si hay riesgo de nevada", 500, NULL, 5, &riesgo_task_handle);
	xTaskCreate(&TareaRadiacion, "mide y decide si el nivel de radiacion es elevado", 500, NULL, 5, &radiacion_task_handle);



	TimerStart(timer_riesgo.timer);
	TimerStart(timer_radiacion.timer);  

}
/*==================[end of file]============================================*/