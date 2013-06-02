#ifndef _PERLIN_NOISE_H_
#define _PERLIN_NOISE_H_

#ifndef _TORQUE_TYPES_H_  
#include "platform/types.h"  
#endif  

#ifndef _VECTOR_H_
#include "collection/vector.h"
#endif

#ifndef _VECTOR2D_H
#include "collection/vector2d.h"
#endif

#ifndef _SIMBASE_H_  
#include "sim/simBase.h"  
#endif  

class RandomGeneratorBase;

extern EnumTable InterpolationTypeTable;

class PerlinNoise : public SimObject
{
public:
	enum InterpolationType
	{
		Linear = 0,
		Cosine = 1,
		Cubic = 2
	};

	struct Octave
	{
		U32 frequency;
		U32 amplitude;
	};

    PerlinNoise(void);
	PerlinNoise(const S32 seed, const F32 persistance = .25f, const InterpolationType interpolation = Cosine);

    virtual ~PerlinNoise(void);

    DECLARE_CONOBJECT(PerlinNoise);

	static void initPersistFields(void);

    //virtual void CopyTo(SimObject* object);

	Vector<F32> PerlinNoise1D(const U32 dim);
	Vector2d<F32> PerlinNoise2D(const U32 dimX, const U32 dimY);

	void SetInterpolationType(const InterpolationType interpolation) { mInterpolation = interpolation; }
	//Note that this reset the current status of the smoothing phase
	void SetUseSmoothing(const bool smooth) { mUseSmoothing = smooth; }
	//RNG or Prime based noise
	void SetUsePrime(const bool prime) { mUsePrime = prime; }
	//Only usefull in RNG mode
	void SetSeed(const S32 seed) { mSeed = seed; }

	//Only usefull in prime noise mode
	void SetPrimes(const U32 prime1, const U32 prime2, const U32 prime3, const U32 prime4);
	void SetPrime1(const U32 prime1) { mPrime1 = prime1; }
	void SetPrime2(const U32 prime2) { mPrime2 = prime2; }
	void SetPrime3(const U32 prime3) { mPrime3 = prime3; }
	void SetPrime4(const U32 prime4) { mPrime4 = prime4; }

	//Only usefull when not using manual octave
	void SetPersistance(const F32 persistance) { mPersistance = persistance; }
	
	void SetScaling(const F32 maxValue) { mMaxValue = maxValue; }

	//This overide any manual octave pass and add a defined number of octave defined by the persistance
	void SetOctaveCount(const U32 octave);

	void AddManualOctave(const Octave octave);
	void AddManualOctave(const U32 frequency, const U32 amplitude);

	//Getters
	const bool GetUseSmoothing(void) const { return mUseSmoothing; }
	const bool GetUsePrime(void) const { return mUsePrime; }
	const S32 GetSeed(void) const { return mSeed; }
	const InterpolationType GetInterpolationType(void) const { return mInterpolation; }
	const F32 GetPersistance(void) const { return mPersistance; }
	const F32 GetScaling(void) const { return mMaxValue; }
	const U32 GetOctaveCount(void) const { return mOctaveCount; }
	const U32 GetPrime1(void) const { return mPrime1; }
	const U32 GetPrime2(void) const { return mPrime2; }
	const U32 GetPrime3(void) const { return mPrime3; }
	const U32 GetPrime4(void) const { return mPrime4; }

	static PerlinNoise::InterpolationType GetInterpolationTypeEnum(const char* label);
    static const char* GetInterpolationTypeDescription(const InterpolationType interpolation);

private:
    typedef SimObject Parent; 

	//Noise phase
	const F32 Noise1D(const U32 x) const;
	const F32 Noise2D(const U32 x, const U32 y) const;

	//Smoothing phase
	const F32 SmoothedNoise1D(const U32 x) const;
	const F32 SmoothedNoise2D(const U32 x, const U32 y) const;

	//Interpolation phase
	const F32 InterpolatedNoise1D(const F32 x) const;
	const F32 InterpolatedNoise2D(const F32 x, const F32 y) const;

	//Interpolation mode
	const F32 LinearInterpolate(const F32 v1, const F32 v2, const F32 value) const;
	const F32 CosineInterpolate(const F32 v1, const F32 v2, const F32 value) const;
	const F32 CubicInterpolate(const F32 previous, const F32 v1, const F32 v2, const F32 next, const F32 value) const;

	const F32 ScaleResult(const F32 value) const;

	RandomGeneratorBase* mRand;

protected:
	S32 mSeed;

	F32 mPersistance;
	U32 mOctaveCount;
	Vector<Octave> mOctavePass;

	bool mUsePrime;
	bool mUseSmoothing;

	InterpolationType mInterpolation;

	U32 mPrime1;
	U32 mPrime2;
	U32 mPrime3;
	U32 mPrime4;

	F32 mMaxValue;

	//Torquescript boillerplate crap, lots of it...
	static bool SetSeed(void* obj, const char* data) { static_cast<PerlinNoise*>(obj)->SetSeed(dAtoi(data)); return false; }
	static bool SetUseSmoothing(void* obj, const char* data) { static_cast<PerlinNoise*>(obj)->SetUseSmoothing(dAtob(data)); return false; }
	static bool SetUsePrime(void* obj, const char* data) { static_cast<PerlinNoise*>(obj)->SetUsePrime(dAtob(data)); return false; }
	static bool SetOctaveCount(void* obj, const char* data) { static_cast<PerlinNoise*>(obj)->SetOctaveCount(dAtoi(data)); return false; }
	static bool SetPersistance(void* obj, const char* data) { static_cast<PerlinNoise*>(obj)->SetPersistance(dAtof(data)); return false; }
	static bool SetScaling(void* obj, const char* data) { static_cast<PerlinNoise*>(obj)->SetScaling(dAtof(data)); return false; }
	static bool SetPrime1(void* obj, const char* data) { static_cast<PerlinNoise*>(obj)->SetPrime1(dAtoi(data)); return false; }
	static bool SetPrime2(void* obj, const char* data) { static_cast<PerlinNoise*>(obj)->SetPrime2(dAtoi(data)); return false; }
	static bool SetPrime3(void* obj, const char* data) { static_cast<PerlinNoise*>(obj)->SetPrime3(dAtoi(data)); return false; }
	static bool SetPrime4(void* obj, const char* data) { static_cast<PerlinNoise*>(obj)->SetPrime4(dAtoi(data)); return false; }

	static const char* GetSeed(void* obj, const char* data) { return Con::getIntArg( static_cast<PerlinNoise*>(obj)->GetSeed()); }
	static const char* GetUseSmoothing(void* obj, const char* data) { return Con::getBoolArg( static_cast<PerlinNoise*>(obj)->GetUseSmoothing()); }
	static const char* GetUsePrime(void* obj, const char* data) { return Con::getBoolArg( static_cast<PerlinNoise*>(obj)->GetUsePrime()); }
	static const char* GetOctaveCount(void* obj, const char* data) { return Con::getIntArg( static_cast<PerlinNoise*>(obj)->GetOctaveCount()); }
	static const char* GetPersistance(void* obj, const char* data) { return Con::getFloatArg( static_cast<PerlinNoise*>(obj)->GetPersistance()); }
	static const char* GetScaling(void* obj, const char* data) { return Con::getFloatArg( static_cast<PerlinNoise*>(obj)->GetScaling()); }
	static const char* GetPrime1(void* obj, const char* data) { return Con::getIntArg( static_cast<PerlinNoise*>(obj)->GetPrime1()); }
	static const char* GetPrime2(void* obj, const char* data) { return Con::getIntArg( static_cast<PerlinNoise*>(obj)->GetPrime2()); }
	static const char* GetPrime3(void* obj, const char* data) { return Con::getIntArg( static_cast<PerlinNoise*>(obj)->GetPrime3()); }
	static const char* GetPrime4(void* obj, const char* data) { return Con::getIntArg( static_cast<PerlinNoise*>(obj)->GetPrime4()); }

    static bool WriteSeed(void* obj, StringTableEntry pFieldName) { return true; }
	static bool WriteUseSmoothing(void* obj, StringTableEntry pFieldName) { return static_cast<PerlinNoise*>(obj)->GetUseSmoothing() != true; }
	static bool WriteUsePrime(void* obj, StringTableEntry pFieldName) { return static_cast<PerlinNoise*>(obj)->GetUsePrime() != false; }
	static bool WriteOctaveCount(void* obj, StringTableEntry pFieldName) { return static_cast<PerlinNoise*>(obj)->GetOctaveCount() != 5; }
	static bool WritePersistance(void* obj, StringTableEntry pFieldName) { return static_cast<PerlinNoise*>(obj)->GetPersistance() != .25f; }
	static bool WriteScaling(void* obj, StringTableEntry pFieldName) { return static_cast<PerlinNoise*>(obj)->GetScaling() != 255; }
	static bool WritePrime1(void* obj, StringTableEntry pFieldName) { return static_cast<PerlinNoise*>(obj)->GetPrime1() != 15731; }
	static bool WritePrime2(void* obj, StringTableEntry pFieldName) { return static_cast<PerlinNoise*>(obj)->GetPrime2() != 789221; }
	static bool WritePrime3(void* obj, StringTableEntry pFieldName) { return static_cast<PerlinNoise*>(obj)->GetPrime3() != 1376312589; }
	static bool WritePrime4(void* obj, StringTableEntry pFieldName) { return static_cast<PerlinNoise*>(obj)->GetPrime4() != 1073741824; }

    static bool	SetInterpolationType(void* obj, const char* data)
    {
		const InterpolationType type = GetInterpolationTypeEnum(data);

		if(type != Cosine && type != Linear && type != Cubic)
            return false;

		static_cast<PerlinNoise*>(obj)->SetInterpolationType(type);
        return false;
    }
    static const char* GetInterpolationType(void* obj, const char* data) { return GetInterpolationTypeDescription( static_cast<PerlinNoise*>(obj)->GetInterpolationType()); }
	static bool WriteInterpolationType(void* obj, StringTableEntry pFieldName) { return static_cast<PerlinNoise*>(obj)->GetInterpolationType() != Cosine; }

	//virtual void onTamlCustomWrite(TamlCustomNodes& customNodes);
    //virtual void onTamlCustomRead(const TamlCustomNodes& customNodes);
};

#endif
