//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef _PLATFORMAUDIO_H_
#define _PLATFORMAUDIO_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

#ifndef NO_AUDIO_SUPPORT

#ifndef _PLATFORMAL_H_
#include "platform/platformAL.h"
#endif

#ifndef _MMATH_H_
#include "math/mMath.h"
#endif

#ifndef _BITSET_H_
#include "collection/bitSet.h"
#endif

typedef U32 AUDIOHANDLE;
#define NULL_AUDIOHANDLE 0

//--------------------------------------------------------------------------

namespace Audio
{
   enum Constants {
    
      AudioVolumeChannels = 32
   };

   //--------------------------------------
   // sound property description
   struct DLLEXPORTS Description
   {
      F32  mVolume;    // 0-1    1=loudest volume
      S32  mVolumeChannel;
      bool mIsLooping;
      bool mIsStreaming;

      bool mIs3D;
      F32  mReferenceDistance;
      F32  mMaxDistance;
      U32  mConeInsideAngle;
      U32  mConeOutsideAngle;
      F32  mConeOutsideVolume;
      Point3F mConeVector;

      // environment info
      F32 mEnvironmentLevel;
   };

   void initOpenAL();
   void shutdownOpenAL();
   void destroy();
}   

class AudioDescription;
class AudioAsset;
class AudioEnvironment;
class AudioSampleEnvironment;
class AudioStreamSource;

DLLEXPORTS AUDIOHANDLE alxCreateSource(const Audio::Description *desc, const char *filename, const MatrixF *transform=NULL, AudioSampleEnvironment * sampleEnvironment = 0);
DLLEXPORTS AUDIOHANDLE alxCreateSource(AudioDescription *descObject, const char *filename, const MatrixF *transform=NULL, AudioSampleEnvironment * sampleEnvironment = 0);
DLLEXPORTS AUDIOHANDLE alxCreateSource(const AudioAsset *profile, const MatrixF *transform=NULL);
DLLEXPORTS AudioStreamSource* alxFindAudioStreamSource(AUDIOHANDLE handle);

DLLEXPORTS AUDIOHANDLE alxPlay(AUDIOHANDLE handle);
DLLEXPORTS bool alxPause(AUDIOHANDLE handle);
DLLEXPORTS void alxPauseAll();
DLLEXPORTS void alxUnPause(AUDIOHANDLE handle);
DLLEXPORTS void alxUnPauseAll();
DLLEXPORTS void alxStop(AUDIOHANDLE handle);
DLLEXPORTS void alxStopAll();

// one-shot helper alxPlay functions, create and play in one call
DLLEXPORTS AUDIOHANDLE alxPlay(const AudioAsset *profile, const MatrixF *transform=NULL, const Point3F *velocity=NULL);

// Source
DLLEXPORTS void alxSourcef(AUDIOHANDLE handle, ALenum pname, ALfloat value);
DLLEXPORTS void alxSourcefv(AUDIOHANDLE handle, ALenum pname, ALfloat *values);
DLLEXPORTS void alxSource3f(AUDIOHANDLE handle, ALenum pname, ALfloat value1, ALfloat value2, ALfloat value3);
DLLEXPORTS void alxSourcei(AUDIOHANDLE handle, ALenum pname, ALint value);
DLLEXPORTS void alxSourceMatrixF(AUDIOHANDLE handle, const MatrixF *transform);

DLLEXPORTS void alxGetSourcef(AUDIOHANDLE handle, ALenum pname, ALfloat *value);
DLLEXPORTS void alxGetSourcefv(AUDIOHANDLE handle, ALenum pname, ALfloat *values);
DLLEXPORTS void alxGetSource3f(AUDIOHANDLE handle, ALenum pname, ALfloat *value1, ALfloat *value2, ALfloat *value3);
DLLEXPORTS void alxGetSourcei(AUDIOHANDLE handle, ALenum pname, ALint *value);

/**   alSource3f access extension for use with Point3F's
*/
inline DLLEXPORTS void alxSourcePoint3F(AUDIOHANDLE handle, ALenum pname, const Point3F *value)
{
   alxSource3f(handle, pname, value->x, value->y, value->z);
}

/**   alGetSource3f access extension for use with Point3F's
*/
inline DLLEXPORTS void alxSourceGetPoint3F(AUDIOHANDLE handle, ALenum pname, Point3F * value)
{
   alxGetSource3f(handle, pname, &value->x, &value->y, &value->z);
}

// Listener
DLLEXPORTS void alxListenerMatrixF(const MatrixF *transform);
DLLEXPORTS void alxListenerf(ALenum param, ALfloat value);
DLLEXPORTS void alxGetListenerf(ALenum param, ALfloat *value);


/**   alListener3f access extension for use with Point3F's
*/
inline DLLEXPORTS void alxListenerPoint3F(ALenum pname, const Point3F *value)
{
   alListener3f(pname, value->x, value->y, value->z);   
}

/**   alGetListener3f access extension for use with Point3F's
*/
inline DLLEXPORTS void alxGetListenerPoint3F(ALenum pname, Point3F *value)
{
   alGetListener3f(pname, &value->x, &value->y, &value->z);
}

// Environment
DLLEXPORTS void alxEnvironmenti(ALenum pname, ALint value);
DLLEXPORTS void alxEnvironmentf(ALenum pname, ALfloat value);
DLLEXPORTS void alxGetEnvironmenti(ALenum pname, ALint * value);
DLLEXPORTS void alxGetEnvironmentf(ALenum pname, ALfloat * value);

DLLEXPORTS void alxSetEnvironment(const AudioEnvironment * environment);
DLLEXPORTS const AudioEnvironment * alxGetEnvironment();

// misc
DLLEXPORTS void alxUpdateTypeGain(U32 type);
DLLEXPORTS bool alxIsValidHandle(AUDIOHANDLE handle);
DLLEXPORTS bool alxIsPlaying(AUDIOHANDLE handle);
DLLEXPORTS void alxUpdate();
DLLEXPORTS F32 alxGetStreamPosition( AUDIOHANDLE handle );
DLLEXPORTS F32 alxGetStreamDuration( AUDIOHANDLE handle );

#endif // NO_AUDIO_SUPPORT

#endif  // _H_PLATFORMAUDIO_
