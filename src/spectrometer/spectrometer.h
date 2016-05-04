#if !defined(spectrometerh)
#define spectrometerh 1

#include "configurations.h"

int spectrometer_init(sSpectrometerStruct * config);
int spectrometer_uninit(sSpectrometerStruct * config);
int spectrometer_get(double * wavelengths, double * spectra, int length);
int spectrometer_trigger(sSpectrometerStruct * config, void (*callback) (sSpectrometerStruct * config));

#endif