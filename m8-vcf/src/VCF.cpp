/*
  ==============================================================================

	VCF.h
    Author:  Jan Janssen
    Source:  https://cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf

  ==============================================================================
*/

#include "VCF.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

VCF::VCF()
{
	ic1eq = 0;
	ic2eq = 0;
	g = 0;
	k = 0;
	a1 = 0;
	a2 = 0;
	a3 = 0;
	output = 0;
}

VCF::~VCF()
{

}

float VCF::Process(int type, float sample)
{
	float v3 = sample - ic2eq;
	float v1 = a1 * ic1eq + a2 * v3;
	float v2 = ic2eq + a2 * ic1eq + a3 * v3;

	ic1eq = 2 * v1 - ic1eq;
	ic2eq = 2 * v2 - ic2eq;

	switch (type)
	{
		case low_pass:
			output = v2;
		break;
		case band_pass:
			output = v1;
		break;
		case high_pass:
			output = sample - k * v1 - v2;
		break;
		case notch:
			output = sample - k * v1;
		break;
		case peak:
			output = 2 * v2 - sample + k * v1;
		break;
		case all:
			output = sample - 2 * k * v1;
		break;
	}

	return output;
}

void VCF::ClearState()
{
	ic1eq = 0;
	ic2eq = 0;

	return; 
}

void VCF::SetCoeff(float frequency, float resonance)
{
	g = tan((PI * frequency) / SAMPLERATE);
	k = 2 - 2 * resonance;
	a1 = 1 / (1 + g * (g + k));
	a2 = g * a1;
	a3 = g * a2;

	return;
}