/*
  ==============================================================================

	VCF_24DB.h
    Author:  Jan Janssen & Jarno Verheesen

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
			template <class T > class VoltageControlledFilter24dB
			{
				public:
					VoltageControlledFilter24dB(uint32_t samplerate) : _samplerate(static_cast<T>(samplerate))
																	 , _cutoff(static_cast<T>(0))
												  					 , _resonance(static_cast<T>(0))
												  					 , _p0(static_cast<T>(0))
												  					 , _p1(static_cast<T>(0))
												  					 , _p2(static_cast<T>(0))
												  					 , _p3(static_cast<T>(0))
												  					 , _p32(static_cast<T>(0))
												  					 , _p33(static_cast<T>(0))
												  					 , _p34(static_cast<T>(0))
					{
						SetCoefficient(static_cast<T>(1000), static_cast<T>(0.10));
					}

					~VoltageControlledFilter24dB() {}

					T Process(T sample)
					{
						try
						{
							T k = _resonance * 4;

							// Coefficients optimized using differential evolution
							// to make feedback gain 4.0 correspond closely to the
							// border of instability, for all values of omega.
							T out = static_cast<T>(_p3 * 0.360891 + _p32 * 0.417290 + _p33 * 0.177896 + _p34 * 0.0439725);

							_p34 = _p33;
							_p33 = _p32;
							_p32 = _p3;

							_p0 += (EfficientTanh(sample - k * out) - EfficientTanh(_p0)) * _cutoff;
							_p1 += (EfficientTanh(_p0) - EfficientTanh(_p1)) * _cutoff;
							_p2 += (EfficientTanh(_p1) - EfficientTanh(_p2)) * _cutoff;
							_p3 += (EfficientTanh(_p2) - EfficientTanh(_p3)) * _cutoff;

							return out;
						}
						catch(const std::exception& e)
						{
							std::cerr << e.what() << '\n';
							throw std::runtime_error(e.what());
						}
					}

					void SetCoefficient(T coeffienct, T resonance)
					{
						try
						{
							_cutoff = static_cast<T>(coeffienct * 2.0 * (PI / _samplerate));
							_cutoff = static_cast<T>(1.0 - _cutoff);
							_cutoff += static_cast<T>(fabs(_cutoff));
							_cutoff *= static_cast<T>(0.5);
							_cutoff = static_cast<T>(1.0 - _cutoff);

							_resonance = resonance;
						}
						catch(const std::exception& e)
						{
							std::cerr << e.what() << '\n';
							throw std::runtime_error(e.what());
						}
					}

				private:

					T EfficientTanh(T input)
					{
						//See comment below for explaination.

						auto x = input * input;
						auto a = (((x + 378) * x + 17325) * x + 135135) * input;
						auto b = ((28 * x + 3150) * x + 62370) * x + 135135;

						auto output = a / b;
						output = Clamp(output, static_cast<T>(-1), static_cast<T>(1));
						return output;
					}

					T Clamp(T value, T low, T high )
					{
					    if (high < low)
						{
							throw std::runtime_error("upper bound is greater than lower bound");
						}
					    return (value < low) ? low : (high < value) ? high : value;
					}

					uint32_t _samplerate;
					T _cutoff;
					T _resonance;
					T _p0 ,_p1 ,_p2 ,_p3;
					T _p32, _p33, _p34;
			};
		}
	}
}

/*
	1.) Lambert’s continued fraction for tanh approximation
				x
	f(x):= -----------------------------
					 x²
			1 + ------------------------
						 x²
				3 + --------------------
							 x²
					5 + ----------------
								x²
						7 + ------------
								   x²
							9 + --------
									  x²
								11 + ---
									  13

	2.) lambert's series with 7 devisions after expression simplification

			 x⁷ + 378x⁵ + 17325x³ + 135135x
	f(x):= ----------------------------------
			28x⁶ + 3150x⁴ + 62370x² + 135135

	3.) optimised denominator en divisor and precomputing x²

			a(x)
	f(x):= -----
			b(x)

	a(x):=(((x² + 378)x² + 17325)x² + 135135)x

	b(x):=((28X² + 3150)x² + 62370)x² + 135135

	4.) since an approximation like this is likely to "blow up" a min max hard clamp
	    of [ -1 ... 1] is required.
*/
