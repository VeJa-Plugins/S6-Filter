/*
  ==============================================================================

	VCF.h
    Author:  Jan Janssen & Jarno Verheesen
    Source:  https://cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf

  ==============================================================================
*/

#pragma once

#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <exception>
#include <iostream>

//M_PI definition of pi
#define PI 3.14159265358979323846

namespace VeJa
{
	namespace Plugins
	{
		namespace Filters
		{
			template <class T > class VoltageControlledFilter12dB
			{
				public:

					VoltageControlledFilter12dB(uint32_t samplerate) : _samplerate(samplerate)
																	 , _ic1eq(static_cast<T>(0))
																	 , _ic2eq(static_cast<T>(0))
																	 , _g(static_cast<T>(0))
																	 , _k(static_cast<T>(0))
																	 , _a1(static_cast<T>(0))
																	 , _a2(static_cast<T>(0))
																	 , _a3(static_cast<T>(0))

					{
					}

					enum filter_type {
    				    low_pass = 0,
    				    band_pass,
    				    high_pass,
    				    notch,
    				    peak,
    				    all
    				};

					~VoltageControlledFilter12dB()
					{
					}

					void SetCoefficient(T frequency, T resonance)
					{
						try
						{
							_g = static_cast<T>(std::tan((PI * frequency) / _samplerate));
							_k = static_cast<T>(2.0 - 2 * resonance);
							_a1 = 1 / (1 + _g * (_g + _k));
							_a2 = _g * _a1;
							_a3 = _g * _a2;
						}
						catch(const std::exception& e)
						{
							std::cerr << e.what() << '\n';
							throw std::runtime_error(e.what());
						}
					}

					T Process(T sample, int type)
					{
						try
						{
						auto v3 = sample - _ic2eq;
						auto v1 = _a1 * _ic1eq + _a2 * v3;
						auto v2 = _ic2eq + _a2 * _ic1eq + _a3 * v3;

						_ic1eq = static_cast<T>(2) * v1 - _ic1eq;
						_ic2eq = static_cast<T>(2) * v2 - _ic2eq;

						T output = 0;

						switch (type)
						{
							case low_pass:
								output = v2;
							break;
							case band_pass:
								output = v1;
							break;
							case high_pass:
								output = sample - _k * v1 - v2;
							break;
							case notch:
								output = sample - _k * v1;
							break;
							case peak:
								output = 2 * v2 - sample + _k * v1;
							break;
							case all:
								output = sample - 2 * _k * v1;
							break;
						}

						return output;
						}
						catch(const std::exception& e)
						{
							std::cerr << e.what() << '\n';
							throw std::runtime_error(e.what());
						}
					}

				private:
				
					uint32_t _samplerate;
					T _ic1eq, _ic2eq, _g, _k, _a1, _a2, _a3;
			};
		}
	}
}
