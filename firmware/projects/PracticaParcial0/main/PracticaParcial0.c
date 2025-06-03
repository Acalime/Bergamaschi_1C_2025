/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Se pretende diseñar un dispositivo basado en la ESP-EDU que permita controlar el
 *	riego y el pH de una plantera.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral              |   ESP32   	|
 * |:--------------------------:|:--------------|
 * | 	Sensor de humedad	 	| 	GPIO_16		|
 * |:--------------------------:|:--------------|
 * | 	Bomba de agua		 	| 	GPIO_17		|
 * |:--------------------------:|:--------------|
 * | 	Sensor de pH		 	| 	   CH1		|
 * |:--------------------------:|:--------------|
 * | 	Bomba sol básica	 	| 	GPIO_15		|
 * |:--------------------------:|:--------------|
 * | 	Bomba sol ácida		 	| 	GPIO_13		|
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
#include "timer_mcu.h"
#include "analog_io_mcu.h"
#include "uart_mcu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mcu.h"
#include "string.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
// el período de muestreo debe ser de 3s
#define PERIODO_MUESTREO 3000 * 1000 // 3s = 0,000003us.
#define PERIODO_ENVIO 5000 * 1000	 // 5s = 0,000005us

uint16_t valorPH;
bool humedad, bombaAgua, pHA, pHB;
bool MedicionActiva = false;

/*==================[internal data definition]===============================*/
TaskHandle_t agua_task_handle = NULL;
TaskHandle_t pH_task_handle = NULL;
TaskHandle_t enviar_task_handle = NULL;

typedef struct
{
	gpio_t pin; /*!< GPIO pin number */ // estructura para gpios
	io_t dir;							/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;
/*==================[internal functions declaration]=========================*/
/** @brief Función que notifica a las tareas que controlan la humedad y el ph de que deben cumplir su función. */
void FuncTimerTareas(void *param) // esto tmb hay que documentarlo con doxygen?
{
	vTaskNotifyGiveFromISR(agua_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(pH_task_handle, pdFALSE);
}

/** @brief Función que notifica a la tarea que envía datos por uart que debe cumplir su función. */
void FuncTimerEnviar(void *param) // esto tmb hay que documentarlo con doxygen?
{
	vTaskNotifyGiveFromISR(enviar_task_handle, pdFALSE);
}

/** @brief tarea que controla la humedad de la tierra y en caso necesario, enciende la bomba de agua */

static void TareaAgua(void *pvParameter)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (MedicionActiva)
		{
			if (GPIORead(GPIO_16))
			{
				// si es true tengo que poner agua -> prender la bomba
				GPIOOn(GPIO_17);
				bombaAgua = true;
				humedad = false;
			}
			else if (!GPIORead(GPIO_16))
			{
				GPIOOff(GPIO_17); // si está bien, apaga la bomba que estaba prendida
				// mensaje de que ta bien
				humedad = true;
				bombaAgua = false;
			}
		}
	}
}

/** @brief tarea que controla el ph de la tierra y en caso necesario enciende las bombas de solución acida o básica. */

static void TareaPH(void *pvParameter)
{
	while (true)
	{
		// tengo una fracción de 3V que me indica cual es el valor de ph.
		//  6 en la escala de ph son 1,28V
		//  6,7 en la escala de ph son 1,43V
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (MedicionActiva)
		{
			AnalogInputReadSingle(CH1, &valorPH);

			if (valorPH <= 1.28)
			{
				// prender el gpio 15
				GPIOOn(GPIO_15);
				GPIOOff(GPIO_13);
				pHB = true;
				pHA = false;
			}
			else if (valorPH >= 1.43)
			{
				// prender el gpio 13
				GPIOOff(GPIO_15);
				GPIOOn(GPIO_13);
				pHA = true;
				pHB = false;
			}
			else
			{
				GPIOOff(GPIO_15);
				GPIOOff(GPIO_13);
			}
		}
	}
}

/** @brief tarea que se encarga de enviar mensajes hacia la pc por uart. Informa el estado de la tierra */

static void TareaEnviarUart(void *pvParameter)
{ // tarea para mandar un mensaje a la pc
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if(MedicionActiva){
			UartSendString(UART_PC, "pH: ");
		UartSendString(UART_PC, (char *)UartItoa(valorPH, 10));
		UartSendString(UART_PC, "\n");

		if (humedad)
		{
			UartSendString(UART_PC, "Humedad correcta");
			UartSendString(UART_PC, "\n");
		}
		else if (bombaAgua)
		{
			UartSendString(UART_PC, "Bomba de agua encendida");
			UartSendString(UART_PC, "\n");
		}
		else if (pHA)
		{
			UartSendString(UART_PC, "Bomba PHA encendida");
			UartSendString(UART_PC, "\n");
		}
		else if (pHB)
		{
			UartSendString(UART_PC, "Bomba PHB encendida");
			UartSendString(UART_PC, "\n");
		}
		}
	}
}

/** @brief función que se encarga de iniciar la medición */

void IniciarMedicion()
{
	MedicionActiva = true;
}

/** @brief función que se encarga de detener la medición */

void DetenerMedicion()
{
	MedicionActiva = false;
}

/*==================[external functions definition]==========================*/
void app_main(void)
{

	// inits
	gpioConf_t vectorGPIO[4] = {// hago esto para tener organizados los gpios.
								{GPIO_16, GPIO_INPUT},
								{GPIO_17, GPIO_OUTPUT},
								{GPIO_15, GPIO_OUTPUT},
								{GPIO_13, GPIO_OUTPUT}};

	GPIOInit(vectorGPIO[0].pin, vectorGPIO[0].dir); // inicializo todos los gpios juntos
	GPIOInit(vectorGPIO[1].pin, vectorGPIO[1].dir);
	GPIOInit(vectorGPIO[2].pin, vectorGPIO[2].dir);
	GPIOInit(vectorGPIO[3].pin, vectorGPIO[3].dir);

	timer_config_t timer_medicion = {// configuración del timer para tomar la medición
									 .timer = TIMER_A,
									 .period = PERIODO_MUESTREO,
									 .func_p = FuncTimerTareas,
									 .param_p = NULL};
	TimerInit(&timer_medicion);

	timer_config_t timer_enviar = {// configuración del timer para tomar la medición
								   .timer = TIMER_B,
								   .period = PERIODO_ENVIO,
								   .func_p = FuncTimerEnviar,
								   .param_p = NULL};
	TimerInit(&timer_enviar);

	analog_input_config_t Canal_1 = {
		// configuración del CH1
		.input = CH1,
		.mode = ADC_SINGLE, // pasa cuando hay interrupciones
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0 // solo para continuo
	};
	AnalogInputInit(&Canal_1);

	serial_config_t my_uart = {// configuración de la uart
							   .port = UART_PC,
							   .baud_rate = 9200,
							   .func_p = NULL, // función que se activa cuando hay una interrupción
							   .param_p = NULL};
	UartInit(&my_uart);

	// interrupciones con switch
	SwitchesInit();
	SwitchActivInt(SWITCH_1, IniciarMedicion, NULL); 
	SwitchActivInt(SWITCH_2, DetenerMedicion, NULL);

	// suministro de agua GPIO 16 = 1 si la humedad es poca

	xTaskCreate(&TareaAgua, "registra el grado de humedad de la tierra", 500, NULL, 5, &agua_task_handle);

	// control de ph. 6<= pH <= 6,7.
	xTaskCreate(&TareaPH, "registra el pH de la tierra", 500, NULL, 5, &pH_task_handle);

	// UART para mostrar mensajes.
	xTaskCreate(&TareaEnviarUart, "Envía un mensaje a la PC", 512, NULL, 5, &enviar_task_handle);

	TimerStart(timer_medicion.timer);
	TimerStart(timer_enviar.timer);
}
/*==================[end of file]============================================*/