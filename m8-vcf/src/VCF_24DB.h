/*
  ==============================================================================

	VCF_24DB.h
    Author:  Jan Janssen
    Source:  http://www.musicdsp.org/showone.php?id=24

  ==============================================================================
*/

#ifndef VCF_24DB_H_INCLUDED
#define VCF_24DB_H_INCLUDED

#define PI 3.14159265358979323846264338327950288
#define SAMPLERATE 48000

class VCF_24DB {

public:

	VCF_24DB()
	{
		p0 = p1 = p2 = p3 = p32 = p33 = p34 = 0.0;
		SetCoeff(1000.0f, 0.10f);
	}

	~VCF_24DB() {}

	float Process(float sample)
	{
		double k = resonance * 4;

		// Coefficients optimized using differential evolution
		// to make feedback gain 4.0 correspond closely to the
		// border of instability, for all values of omega.
		double out = p3 * 0.360891 + p32 * 0.417290 + p33 * 0.177896 + p34 * 0.0439725;

		p34 = p33;
		p33 = p32;
		p32 = p3;

		p0 += (tanh(sample - k * out) - tanh(p0)) * cutoff;
		p1 += (tanh(p0) - tanh(p1)) * cutoff;
		p2 += (tanh(p1) - tanh(p2)) * cutoff;
		p3 += (tanh(p2) - tanh(p3)) * cutoff;

		return out;
	}

	void SetCoeff(float c, float r)
	{
		cutoff = c * 2 * PI / SAMPLERATE;
		cutoff = 1 - cutoff;
		cutoff += fabs(cutoff);
		cutoff *= 0.5f;
		cutoff = 1 - cutoff;

		resonance = r;
	}

private:

	double cutoff;
	double resonance;
	double p0;
	double p1;
	double p2;
	double p3;
	double p32;
	double p33;
	double p34;
};

#endif //VCF_H_INCLUDED
