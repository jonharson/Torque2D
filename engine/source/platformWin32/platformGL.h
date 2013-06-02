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

#ifndef _PLATFORMGL_H_
#define _PLATFORMGL_H_

//put this here so the GUI can get to it
//<--%PUAP% -Mat add #defines for min window/resolution size
//these can be set as low as 1 1, but we picked 320
//because the iPhone can init to 320x480 or 480x320
#define MIN_RESOLUTION_X			320
#define MIN_RESOLUTION_Y			320//for 320 x 480 or 480 x 320
#define MIN_RESOLUTION_BIT_DEPTH	16
#define MIN_RESOLUTION_XY_STRING	"320 320"
//%PUAP%-->

#include "torqueConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "platformWin32/gl_types.h"

#define GLAPI extern
#define GLAPIENTRY __stdcall

#include "platformWin32/gl_types.h"

#define GL_FUNCTION(fn_type,fn_name,fn_args, fn_value) extern fn_type (__stdcall *fn_name)fn_args;
#include "platform/GLCoreFunc.h"
#include "platform/GLExtFunc.h"
#include "platform/GLUFunc.h"
#undef GL_FUNCTION

/* EXT_vertex_buffer */
#define GL_V12MTVFMT_EXT                     0x8702
#define GL_V12MTNVFMT_EXT                    0x8703
#define GL_V12FTVFMT_EXT                     0x8704
#define GL_V12FMTVFMT_EXT                    0x8705

struct DLLEXPORTS GLState
{
   bool suppARBMultitexture;
   bool suppEXTblendcolor;
   bool suppEXTblendminmax;
   bool suppPackedPixels;
   bool suppTexEnvAdd;
   bool suppLockedArrays;
   bool suppTextureEnvCombine;
   bool suppVertexArrayRange;
   bool suppFogCoord;
   bool suppEdgeClamp;
   bool suppTextureCompression;
   bool suppS3TC;
   bool suppFXT1;
   bool suppTexAnisotropic;
   bool suppPalettedTexture;
   bool suppVertexBuffer;
   bool suppSwapInterval;

   unsigned int triCount[4];
   unsigned int primCount[4];
   unsigned int primMode; // 0-3

   GLfloat maxAnisotropy;
   GLint   maxTextureUnits;
};

extern DLLEXPORTS GLState gGLState;
#define UNSIGNED_SHORT_5_6_5 0x8363
#define UNSIGNED_SHORT_5_6_5_REV 0x8364

extern DLLEXPORTS bool gOpenGLDisablePT;
extern DLLEXPORTS bool gOpenGLDisableCVA;
extern DLLEXPORTS bool gOpenGLDisableTEC;
extern DLLEXPORTS bool gOpenGLDisableARBMT;
extern DLLEXPORTS bool gOpenGLDisableFC;
extern DLLEXPORTS bool gOpenGLDisableTCompress;
extern DLLEXPORTS bool gOpenGLNoEnvColor;
extern DLLEXPORTS float gOpenGLGammaCorrection;
extern DLLEXPORTS bool gOpenGLNoDrawArraysAlpha;

inline DLLEXPORTS void dglSetRenderPrimType(unsigned int type)
{
   gGLState.primMode = type;
}

inline DLLEXPORTS void dglClearPrimMetrics()
{
   for(int i = 0; i < 4; i++)
      gGLState.triCount[i] = gGLState.primCount[i] = 0;
}

inline DLLEXPORTS bool dglDoesSupportPalettedTexture()
{
   return gGLState.suppPalettedTexture && (gOpenGLDisablePT == false);
}

inline DLLEXPORTS bool dglDoesSupportCompiledVertexArray()
{
   return gGLState.suppLockedArrays && (gOpenGLDisableCVA == false);
}

inline DLLEXPORTS bool dglDoesSupportTextureEnvCombine()
{
   return gGLState.suppTextureEnvCombine && (gOpenGLDisableTEC == false);
}

inline DLLEXPORTS bool dglDoesSupportARBMultitexture()
{
   return gGLState.suppARBMultitexture && (gOpenGLDisableARBMT == false);
}

inline DLLEXPORTS bool dglDoesSupportEXTBlendColor()
{
   return gGLState.suppEXTblendcolor;
}

inline DLLEXPORTS bool dglDoesSupportEXTBlendMinMax()
{
   return gGLState.suppEXTblendminmax;
}

inline DLLEXPORTS bool dglDoesSupportVertexArrayRange()
{
   return gGLState.suppVertexArrayRange;
}

inline DLLEXPORTS bool dglDoesSupportFogCoord()
{
   return gGLState.suppFogCoord && (gOpenGLDisableFC == false);
}

inline DLLEXPORTS bool dglDoesSupportEdgeClamp()
{
   return gGLState.suppEdgeClamp;
}

inline DLLEXPORTS bool dglDoesSupportTextureCompression()
{
   return gGLState.suppTextureCompression && (gOpenGLDisableTCompress == false);
}

inline DLLEXPORTS bool dglDoesSupportS3TC()
{
   return gGLState.suppS3TC;
}

inline DLLEXPORTS bool dglDoesSupportFXT1()
{
   return gGLState.suppFXT1;
}

inline DLLEXPORTS bool dglDoesSupportTexEnvAdd()
{
   return gGLState.suppTexEnvAdd;
}

inline DLLEXPORTS bool dglDoesSupportTexAnisotropy()
{
   return gGLState.suppTexAnisotropic;
}

inline DLLEXPORTS bool dglDoesSupportVertexBuffer()
{
   return gGLState.suppVertexBuffer;
}

inline DLLEXPORTS GLfloat dglGetMaxAnisotropy()
{
   return gGLState.maxAnisotropy;
}

inline DLLEXPORTS GLint dglGetMaxTextureUnits()
{
   if (dglDoesSupportARBMultitexture())
      return gGLState.maxTextureUnits;
   else
      return 1;
}


#ifdef __cplusplus
}
#endif

#endif
