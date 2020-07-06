/*
  ==============================================================================

	VCF.h
    Author:  Jan Janssen
    Source:  https://cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf

  ==============================================================================
*/

#ifndef VCF_H_INCLUDED
#define VCF_H_INCLUDED

#define PI 3.14159265358979323846264338327950288
#define SAMPLERATE 48000

class VCF {

	public:

		VCF();
		~VCF();

		void ClearState();
		void SetCoeff(float frequency, float resonance);
		float Process(int type, float sample);

	enum filter_type {
        low_pass = 0,
        band_pass,
        high_pass,
        notch,
        peak,
        all
    };

	private:

		float ic1eq;
		float ic2eq;
		float g;
		float k;
		float a1;
		float a2;
		float a3;

		float output;
};

#endif //VCF_H_INCLUDED
