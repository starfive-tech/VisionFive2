/*
 * Copyright (c) 2019, Chips&Media
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WIN32) || defined(__MINGW32__)
#	define PLATFORM_WIN32
#elif defined(linux) || defined(__linux) || defined(ANDROID)
#	define PLATFORM_LINUX
#elif defined(unix) || defined(__unix)
#   define PLATFORM_QNX
#else
#	define PLATFORM_NON_OS
#endif

#if defined(_MSC_VER)
#	include <windows.h>
#	define inline _inline
#elif defined(__GNUC__)
#elif defined(__ARMCC__)
#else
#  error "Unknown compiler."
#endif

#define API_VERSION_MAJOR       5
#define API_VERSION_MINOR       5
#define API_VERSION_PATCH       60
#define API_VERSION             ((API_VERSION_MAJOR<<16) | (API_VERSION_MINOR<<8) | API_VERSION_PATCH)

#if defined(PLATFORM_NON_OS) || defined (ANDROID) || defined(MFHMFT_EXPORTS) || defined(PLATFORM_QNX) || defined(CNM_SIM_PLATFORM)
//#define SUPPORT_FFMPEG_DEMUX
#else
//#define SUPPORT_FFMPEG_DEMUX
#endif

//------------------------------------------------------------------------------
// COMMON
//------------------------------------------------------------------------------
//#if defined(linux) || defined(__linux) || defined(ANDROID) || defined(CNM_FPGA_HAPS_INTERFACE)
#define SUPPORT_MULTI_INST_INTR
//#endif

#ifndef PTHREAD_MUTEX_ROBUST_NP
#define PTHREAD_MUTEX_ROBUST_NP     1
#endif


// do not define BIT_CODE_FILE_PATH in case of multiple product support. because wave410 and coda980 has different firmware binary format.
#define CORE_0_BIT_CODE_FILE_PATH   "coda960.out"     // for coda960
#define CORE_1_BIT_CODE_FILE_PATH   "coda980.out"     // for coda980
#define CORE_2_BIT_CODE_FILE_PATH   "picasso.bin"     // for wave510
#define CORE_4_BIT_CODE_FILE_PATH   "millet.bin"      // for wave520
#define CORE_5_BIT_CODE_FILE_PATH   "kepler.bin"      // for wave515
#define CORE_6_BIT_CODE_FILE_PATH   "chagall.bin"     // for wave521
#define CORE_7_BIT_CODE_FILE_PATH "vincent.bin" // for wave517

//------------------------------------------------------------------------------
// OMX
//------------------------------------------------------------------------------




//------------------------------------------------------------------------------
// WAVE521C
//------------------------------------------------------------------------------


//#define SUPPORT_LOOK_AHEAD_RC




//------------------------------------------------------------------------------
// WAVE511
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// WAVE517
//------------------------------------------------------------------------------


//#define SUPPORT_SW_UART
//#define SUPPORT_SW_UART_V2	// WAVE511 or WAVE521C
#endif /* __CONFIG_H__ */


