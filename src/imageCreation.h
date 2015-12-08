#ifndef _IMAGECREATION_
#define _IMAGECREATION_
#include"configurations.h"

/******************************
 *   FUNCTIONS
 ******************************/
int startAquisition(sParameterStruct * sParameters_A,
	sParameterStruct * sParameters_B, sConfigStruct * config);
int aquire(sParameterStruct * sParameters_A,
	sParameterStruct * sParameters_B, sConfigStruct * config);
#endif
