#ifndef _PERLIN_NOISE_SCRIPTBINDING_H_
#define _PERLIN_NOISE_SCRIPTBINDING_H_

#include "console/console.h"

ConsoleMethod(PerlinNoise, SetUseSmoothing, void, 3, 3, "(bool smooth) Wheter to use smoothed noise generation."
														"@return No return Value.")
{
	object->SetUseSmoothing(dAtob(argv[2]));
}

ConsoleMethod(PerlinNoise, SetUsePrime, void, 3, 3, "(bool prime) Wheter to use prime based noise generation."
														"@return No return Value.")
{
	object->SetUsePrime(dAtob(argv[2]));
}

ConsoleMethod(PerlinNoise, SetSeed, void, 3, 3, "(integer seed) Seed to use for RNG based noise generation."
													"@return No return Value.")
{
	object->SetSeed(dAtoi(argv[2]));
}

ConsoleMethod(PerlinNoise, SetPrime1, void, 3, 3, "(integer prime) First prime to use for the prime based noise generation."
													"@return No return Value.")
{
	object->SetPrime1(dAtoi(argv[2]));
}

ConsoleMethod(PerlinNoise, SetPrime2, void, 3, 3, "(integer prime) Second prime to use for the prime based noise generation."
													"@return No return Value.")
{
	object->SetPrime2(dAtoi(argv[2]));
}

ConsoleMethod(PerlinNoise, SetPrime3, void, 3, 3, "(integer prime) Third prime to use for the prime based noise generation."
													"@return No return Value.")
{
	object->SetPrime3(dAtoi(argv[2]));
}

ConsoleMethod(PerlinNoise, SetPrime4, void, 3, 3, "(integer prime) Fourth prime to use for the prime based noise generation."
													"@return No return Value.")
{
	object->SetPrime4(dAtoi(argv[2]));
}

ConsoleMethod(PerlinNoise, SetPersistance, void, 3, 3, "(float persistance) Divergence between octave pass when using set number of pass."
														"@return No return Value.")
{
	object->SetPersistance(dAtof(argv[2]));
}

ConsoleMethod(PerlinNoise, SetScaling, void, 3, 3, "(float maxvalue) Maximum value for the returned data to be scaled by."
													"@return No return Value.")
{
	object->SetScaling(dAtof(argv[2]));
}

ConsoleMethod(PerlinNoise, SetOctaveCount, void, 3, 3, "(integer octave) Number of octave pass. Divergence between pass will be determined by persistance.."
														"@return No return Value.")
{
	object->SetOctaveCount(dAtoi(argv[2]));
}

ConsoleMethod(PerlinNoise, GetUseSmoothing, bool, 2, 2, "() Get whether smoother noise generation is enable or not."
														"@return Whether smoothed noise generation is enabled or not.")
{
	return object->GetUseSmoothing();
}

ConsoleMethod(PerlinNoise, GetUsePrime, bool, 2, 2, "() Get whether prime noise generation is used."
														"@return Whether prime based noise generation is enabled (RNG based if not).")
{
	return object->GetUsePrime();
}

ConsoleMethod(PerlinNoise, GetPersistance, F32, 2, 2, "() Get the persistance."
														"@return persistance.")
{
	return object->GetPersistance();
}

ConsoleMethod(PerlinNoise, GetOctaveCount, S32, 2, 2, "() Get the octave count."
														"@return octave count.")
{
	return object->GetOctaveCount();
}

ConsoleMethod(PerlinNoise, GetPrime1, S32, 2, 2, "() Get prime 1."
													"@return .")
{
	return object->GetPrime1();
}

ConsoleMethod(PerlinNoise, GetPrime2, S32, 2, 2, "() Get prime 2."
													"@return .")
{
	return object->GetPrime2();
}

ConsoleMethod(PerlinNoise, GetPrime3, S32, 2, 2, "() Get prime 3."
													"@return .")
{
	return object->GetPrime3();
}

ConsoleMethod(PerlinNoise, GetPrime4, S32, 2, 2, "() Get prime 4."
													"@return .")
{
	return object->GetPrime4();
}

ConsoleMethod(PerlinNoise, GetInterpolationType, const char*, 2, 2,   "() Get the used interpolation type."
																		"@return ." )
{
    return PerlinNoise::GetInterpolationTypeDescription(object->GetInterpolationType());
}

ConsoleMethod(PerlinNoise, SetInterpolationType, void, 3, 3, "(interpolationtype type) Sets the interpolation type.\n"
															"@return No return value.")
{
    const PerlinNoise::InterpolationType type = PerlinNoise::GetInterpolationTypeEnum(argv[2]);

    if(type != PerlinNoise::Linear && type != PerlinNoise::Cubic && type != PerlinNoise::Cosine)
        return;

    object->SetInterpolationType(type);
}

/*
	Vector<F32> PerlinNoise1D(const U32 dim);
	Vector2d<F32> PerlinNoise2D(const U32 dimX, const U32 dimY);
	void AddManualOctave(const Octave octave);
	void AddManualOctave(const U32 frequency, const U32 amplitude);
*/

#endif