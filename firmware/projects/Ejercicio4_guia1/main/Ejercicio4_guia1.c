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
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/


typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;



/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{
	int unidad, decena, centena; 

	/*unidad = data % 10; //428/10 = 42 => 428%10 = 8
	decena = (data/10)%10; // 428/10 = 42 => 42%10 = 2
	centena = (data/100)%10; //428/100=4 => 4%10 = 4

	bcd_number[0] = centena; 
	bcd_number[1] = decena; 
	bcd_number[2] = unidad; */

	for(int i = 0; i < digits; i++)
	{
		bcd_number[i] = data % 10; 
		data = data / 10; 
	}

	return 0; 
}

int8_t cambiarEstado(uint8_t digitoBCD, gpioConf_t * vectorGPIO){ //1 digito 4 bits

	// 9 = 1001
	//     1000 
	for(int i = 0; i<4; i++)
	{
		if(digitoBCD & 1<<i)
		{
			GPIOOn(vectorGPIO[i].pin);
		}else GPIOOff(vectorGPIO[i].pin);  
	}

return 0; 


}
/*==================[external functions definition]==========================*/
void app_main(void){
	
	uint32_t dato = 42856;
	uint8_t digitos = 5; 
	uint8_t arregloBCD[digitos];

	uint8_t devuelve = convertToBcdArray(dato, digitos, &arregloBCD);

	for(int i = digitos-1; i > -1; i--){
		printf("%d", arregloBCD[i]);
	}

	gpioConf_t vectorGPIO[4] = {
		{GPIO_20, GPIO_OUTPUT},
		{GPIO_21, GPIO_OUTPUT},
		{GPIO_22, GPIO_OUTPUT},
		{GPIO_23, GPIO_OUTPUT}
	};

	GPIOInit(vectorGPIO[0].pin, vectorGPIO[0].dir);
	GPIOInit(vectorGPIO[1].pin, vectorGPIO[1].dir);
	GPIOInit(vectorGPIO[2].pin, vectorGPIO[2].dir);
	GPIOInit(vectorGPIO[3].pin, vectorGPIO[3].dir);

	uint8_t digito = 9; 
	int8_t devuelve = cambiarEstado(digito, &vectorGPIO); 


}
/*==================[end of file]============================================*/