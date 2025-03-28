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
/*==================[macros and definitions]=================================*/
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

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
	
	uint32_t dato = 428;
	uint8_t digitos = 3; 
	uint8_t arregloBCD[digitos];

	uint8_t devuelve = convertToBcdArray(dato, digitos, &arregloBCD);

	for(int i = digitos-1; i > -1; i--){
		printf("%d", arregloBCD[i]);
	}

}
/*==================[end of file]============================================*/