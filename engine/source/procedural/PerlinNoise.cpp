
#include "PerlinNoise.h"
#include "PerlinNoise_ScriptBinding.h"

#ifndef _MRANDOM_H_
#include "math/mRandom.h"
#endif

#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif


#include "string/stringBuffer.h"


IMPLEMENT_CONOBJECT(PerlinNoise);
//IMPLEMENT_CONOBJECT(PerlinNoise, WriteCustomTamlSchema);

PerlinNoise::PerlinNoise(void)
{
	PerlinNoise(Platform::getRealMilliseconds(), .25f, Cosine);
}

PerlinNoise::PerlinNoise(const S32 seed, const F32 persistance, const InterpolationType interpolation)
{
	mSeed = seed;
	mRand = NULL;

	mUsePrime = false;
	mUseSmoothing = true;

	mPersistance = persistance;
	mOctaveCount = 5;
	mInterpolation = interpolation;

	mPrime1 = 15731;
	mPrime2 = 789221;
	mPrime3 = 1376312589;
	mPrime4 = 1073741824;

	mMaxValue = 255;
}

PerlinNoise::~PerlinNoise(void)
{
	if(mRand != NULL)
		delete mRand;
}

void PerlinNoise::initPersistFields(void)  
{  
    Parent::initPersistFields();

    addProtectedField("Seed", TypeS32, NULL, &SetSeed, &GetSeed, &WriteSeed, "Perlin noise initial seed, only usefull if using the RNG based noise.");
	addProtectedField("UsePrime", TypeBool, NULL, &SetUsePrime, &GetUsePrime, &WriteUsePrime, "Wheter to use the RNG or prime based implementation.");
	addProtectedField("UseSmoothing", TypeBool, NULL, &SetUseSmoothing, &GetUseSmoothing, &WriteUseSmoothing, "Wheter to use a smoothing phase or not.");
	addProtectedField("OctaveCount", TypeS32, NULL, &SetOctaveCount, &GetOctaveCount, &WriteOctaveCount, "Number of octave pass. Divergence between pass will be determined by persistance.");
	addProtectedField("Persistance", TypeF32, NULL, &SetPersistance, &GetPersistance, &WritePersistance, "Divergence between octave pass when using set number of pass.");
	addProtectedField("MaxValue", TypeF32, NULL, &SetScaling, &GetScaling, &WriteScaling, "Maximum result value.");
	addProtectedField("Prime1", TypeS32, NULL, &SetPrime1, &GetPrime1, &WritePrime1, "Prime based noise first prime.");
	addProtectedField("Prime2", TypeS32, NULL, &SetPrime2, &GetPrime2, &WritePrime2, "Prime based noise second prime.");
	addProtectedField("Prime3", TypeS32, NULL, &SetPrime3, &GetPrime3, &WritePrime3, "Prime based noise third prime.");
	addProtectedField("Prime4", TypeS32, NULL, &SetPrime4, &GetPrime4, &WritePrime4, "Prime based noise forth prime.");
	addProtectedField("Interpolation", TypeEnum, NULL, &SetInterpolationType, &GetInterpolationType, &WriteInterpolationType, 1, &InterpolationTypeTable, "Interpolation mode to be used.");
}

Vector<F32> PerlinNoise::PerlinNoise1D(const U32 dim)
{
	if(mRand != NULL)
		delete mRand;

	if(!mUsePrime)
		mRand = new RandomLCG(mSeed);

	Vector<F32> result;
	result.reserve(dim);

	for(U32 x = 0; x < dim; x++)
	{
		F32 total = 0;
		if(mOctavePass.size() > 0)
		{
			for(U32 pass = 0; pass < (U32)mOctavePass.size(); pass++)
			{
				total += InterpolatedNoise1D((F32)x * mOctavePass[pass].frequency) * mOctavePass[pass].amplitude;
			}
		}
		else
		{
			for(U32 pass = 0; pass < mOctaveCount; pass++)
			{
				const U32 frequency = (U32)mPow(2.f, (F32)pass);
				const F32 amplitude = mPow(mPersistance, (F32)pass);
				total += InterpolatedNoise1D((F32)x * frequency) * amplitude;
			}
		}

		result.push_back(ScaleResult(total));
	}

	return result;
}

Vector2d<F32> PerlinNoise::PerlinNoise2D(const U32 dimX, const U32 dimY)
{
	if(mRand != NULL)
		delete mRand;

	if(!mUsePrime)
		mRand = new RandomLCG(mSeed);

	Vector2d<F32> result;
	result.resize(dimX, dimY);

	for(U32 x = 0; x < dimX; x++)
	{
		for(U32 y = 0; y < dimY; y++)
		{
			F32 total = 0;
			if(mOctavePass.size() > 0)
			{
				for(U32 pass = 0; pass < (U32)mOctavePass.size(); pass++)
				{
					total += InterpolatedNoise2D((F32)x * mOctavePass[pass].frequency, (F32)y * mOctavePass[pass].frequency) * mOctavePass[pass].amplitude;
				}
			}
			else
			{
				for(U32 pass = 0; pass < mOctaveCount; pass++)
				{
					const U32 frequency = (U32)mPow(2.f, (F32)pass);
					const F32 amplitude = mPow(mPersistance, (F32)pass);
					total += InterpolatedNoise2D((F32)x * frequency, (F32)y * frequency) * amplitude;
				}
			}

			result.push_back(ScaleResult(total));
		}
	}

	return result;
}

const F32 PerlinNoise::ScaleResult(const F32 value) const
{
	//.... 
	return (F32)(U32)(value * (mMaxValue - FLT_EPSILON));
}

const F32 PerlinNoise::InterpolatedNoise1D(const F32 x) const
{
	const U32 integral = (U32)x;
    const F32 fractional = x - integral;

	F32 v1, v2;

	if(mUseSmoothing)
	{
		v1 = SmoothedNoise1D(integral);
		v2 = SmoothedNoise1D(integral + 1);
	}
	else
	{
		v1 = Noise1D(integral);
		v2 = Noise1D(integral + 1);
	}

	switch(mInterpolation)
	{
		case(Linear):
			return LinearInterpolate(v1, v2, fractional);

		case(Cosine):
			return CosineInterpolate(v1, v2, fractional);

		case(Cubic):
			{
				F32 v0, v3;

				if(mUseSmoothing)
				{
					v0 = SmoothedNoise1D((U32)x - 1);
					v3 = SmoothedNoise1D((U32)x + 2);
				}
				else
				{
					v0 = Noise1D((U32)x - 1);
					v3 = Noise1D((U32)x + 2);
				}

				return CubicInterpolate(v0, v1, v2, v3, fractional);
			}

		default:
			Con::errorf("Invalid interpolation format!");
			return 0.f;
	}
}

const F32 PerlinNoise::InterpolatedNoise2D(const F32 x, const F32 y) const
{
	const U32 integralX = (U32)x;
    const F32 fractionalX = x - integralX;

	const U32 integralY = (U32)y;
    const F32 fractionalY = y - integralY;

	F32 v1, v2, v3, v4;

	if(mUseSmoothing)
	{
		v1 = SmoothedNoise2D(integralX, integralY);
		v2 = SmoothedNoise2D(integralX + 1, integralY);
		v3 = SmoothedNoise2D(integralX, integralY + 1);
		v4 = SmoothedNoise2D(integralX + 1, integralY + 1);
	}
	else
	{
		v1 = Noise2D(integralX, integralY);
		v2 = Noise2D(integralX + 1, integralY);
		v3 = Noise2D(integralX, integralY + 1);
		v4 = Noise2D(integralX + 1, integralY + 1);
	}

	switch(mInterpolation)
	{
		case(Linear):
			{
				const F32 i1 = LinearInterpolate(v1 , v2 , fractionalX);
				const F32 i2 = LinearInterpolate(v3 , v4 , fractionalX);
				return LinearInterpolate(i1 , i2 , fractionalY);
			}

		case(Cosine):
			{
				const F32 i1 = CosineInterpolate(v1 , v2 , fractionalX);
				const F32 i2 = CosineInterpolate(v3 , v4 , fractionalX);
				return CosineInterpolate(i1 , i2 , fractionalY);
			}

		case(Cubic):
			{
				F32 v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;

				if(mUseSmoothing)
				{
					v5 = SmoothedNoise2D(integralX -1, integralY -1);
					v6 = SmoothedNoise2D(integralX, integralY -1);
					v7 = SmoothedNoise2D(integralX + 1, integralY - 1);
					v8 = SmoothedNoise2D(integralX + 2, integralY - 1);

					v9 = SmoothedNoise2D(integralX - 1, integralY);
					v10 = SmoothedNoise2D(integralX - 1, integralY + 1);
					v11 = SmoothedNoise2D(integralX + 2, integralY);
					v12 = SmoothedNoise2D(integralX + 2, integralY + 1);

					v13 = SmoothedNoise2D(integralX - 1, integralY + 2);
					v14 = SmoothedNoise2D(integralX, integralY + 2);
					v15 = SmoothedNoise2D(integralX + 1, integralY + 2);
					v16 = SmoothedNoise2D(integralX + 2, integralY + 2);
				}
				else
				{
					v5 = Noise2D(integralX -1, integralY -1);
					v6 = Noise2D(integralX, integralY -1);
					v7 = Noise2D(integralX + 1, integralY - 1);
					v8 = Noise2D(integralX + 2, integralY - 1);

					v9 = Noise2D(integralX - 1, integralY);
					v10 = Noise2D(integralX - 1, integralY + 1);
					v11 = Noise2D(integralX + 2, integralY);
					v12 = Noise2D(integralX + 2, integralY + 1);

					v13 = Noise2D(integralX - 1, integralY + 2);
					v14 = Noise2D(integralX, integralY + 2);
					v15 = Noise2D(integralX + 1, integralY + 2);
					v16 = Noise2D(integralX + 2, integralY + 2);
				}

				const F32 i1 = CubicInterpolate(v5, v6, v7, v8, fractionalX);
				const F32 i2 = CubicInterpolate(v9, v1, v2, v11, fractionalX);
				const F32 i3 = CubicInterpolate(v10, v3, v4, v12, fractionalX);
				const F32 i4 = CubicInterpolate(v13, v14, v15, v16, fractionalX);

				return CubicInterpolate(i1, i2, i3, i4, fractionalY);
			}

		default:
			Con::errorf("Invalid interpolation format!");
			return 0.f;
	}
}

void PerlinNoise::SetOctaveCount(const U32 octave)
{
	mOctaveCount = octave;
	mOctavePass.clear();
}

//Do not validate that they are actually primes or valid in any way
void PerlinNoise::SetPrimes(const U32 prime1, const U32 prime2, const U32 prime3, const U32 prime4)
{
	mPrime1 = prime1;
	mPrime2 = prime2;
	mPrime3 = prime3;
	mPrime4 = prime4;
}

void PerlinNoise::AddManualOctave(const Octave octave)
{
	mOctavePass.push_back(octave);
	mOctaveCount = mOctavePass.size();
}

void PerlinNoise::AddManualOctave(const U32 frequency, const U32 amplitude)
{
	Octave lOctave;
	lOctave.frequency = frequency;
	lOctave.amplitude = amplitude;
	mOctavePass.push_back(lOctave);
	mOctaveCount = mOctavePass.size();
}

const F32 PerlinNoise::LinearInterpolate(const F32 v1, const F32 v2, const F32 value) const
{
	return  v1 * (1 - value) + v2 * value;
}

const F32 PerlinNoise::CosineInterpolate(const F32 v1, const F32 v2, const F32 value) const
{
	const F32 angle = value * (F32)M_PI;
	const F32 f = (1 - mCos(angle)) * .5f;
	return  v1 * (1 - f) + v2 * value;
}

//Om nom nom CPU cycle!
const F32 PerlinNoise::CubicInterpolate(const F32 previous, const F32 v1, const F32 v2, const F32 next, const F32 value) const
{
	const F32 P = (next - v2) - (previous - v1);
	const F32 Q = (previous - v1) - P;
	const F32 R = v2 - previous;

	return P * mPow(value, 3) + Q * mPow(value, 2) + R * value + v1;
}

const F32 PerlinNoise::Noise1D(const U32 x) const
{
	if(mUsePrime)
	{
		const U32 r = (U32)mPow((F32)(x << 13), (F32)x);
		return (1.f - ((r * (r * r * mPrime1 + mPrime2) + mPrime3) & 0x7fffffff) / ((F32)mPrime4));
	}

	return mRand->randRangeF(-1.f, 1.f);
}

const F32 PerlinNoise::Noise2D(const U32 x, const U32 y) const
{
	if(mUsePrime)
	{
		const U32 n = x + y * 57;
		const U32 r = (U32)mPow((F32)(n << 13), (F32)n);
		return (1.f - ((r * (r * r * mPrime1 + mPrime2) + mPrime3) & 0x7fffffff) / ((F32)mPrime4));
	}

	return mRand->randRangeF(-1.f, 1.f);
}

const F32 PerlinNoise::SmoothedNoise1D(const U32 x) const
{
	return Noise1D(x) / 2 + Noise1D(x - 1) / 4 + Noise1D(x + 1) / 4;
}

const F32 PerlinNoise::SmoothedNoise2D(const U32 x, const U32 y) const
{
    const F32 corners = (Noise2D(x - 1, y - 1) + Noise2D(x + 1, y - 1) + Noise2D(x - 1, y + 1) + Noise2D(x + 1, y + 1)) / 16;
    const F32 sides = (Noise2D(x - 1, y) + Noise2D(x + 1, y) + Noise2D(x, y - 1) + Noise2D(x, y + 1)) / 8;
    const F32 center = Noise2D(x, y) / 4;

    return corners + sides + center;
}

//Torquescript boillerplate
static EnumTable::Enums InterpolationTypeLookup[] =
{
	{ PerlinNoise::Linear, "Linear" },
	{ PerlinNoise::Cosine, "Cosine" },
	{ PerlinNoise::Cubic, "Cubic" },
};

EnumTable InterpolationTypeTable(sizeof(InterpolationTypeLookup) / sizeof(EnumTable::Enums), &InterpolationTypeLookup[0]);

PerlinNoise::InterpolationType PerlinNoise::GetInterpolationTypeEnum(const char* label)
{
    for(U32 i = 0; i < (sizeof(InterpolationTypeLookup) / sizeof(EnumTable::Enums)); i++)
    {
        if(dStricmp(InterpolationTypeLookup[i].label, label) == 0)
            return (InterpolationType)InterpolationTypeLookup[i].index;
    }

    Con::warnf("PerlinNoise::GetInterpolationTypeEnum() - Invalid interpolation type of '%s'", label );
    return (InterpolationType) - 1;
}

const char* PerlinNoise::GetInterpolationTypeDescription(const InterpolationType interpolation)
{
    for(U32 i = 0; i < (sizeof(InterpolationTypeLookup) / sizeof(EnumTable::Enums)); i++)
    {
        if( InterpolationTypeLookup[i].index == interpolation )
            return InterpolationTypeLookup[i].label;
    }

    Con::warnf( "PerlinNoise::GetInterpolationTypeDescription() - Invalid interpolation type." );
    return StringTable->EmptyString;
}