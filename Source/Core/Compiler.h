/*
MicroBuild
Copyright (C) 2016 TwinDrills

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

// One of the configurations defines needs to be passed so we know what
// we are actually building!
#if !defined(MB_SHIPPING_BUILD) && \
	!defined(MB_RELEASE_BUILD) && \
	!defined(MB_DEBUG_BUILD)

#error "No compile configuration defined. One of the following should always" \
		"be defined: LD_SHIPPING_BUILD, LD_RELEASE_BUILD, LD_DEBUG_BUILD"

#endif

// Try and figure out what platform we are compiling for, though in practice
// we should have minimal to no platform specific code.
#ifdef _WIN32
	#define MB_PLATFORM_WINDOWS

#elif defined(__linux__)
	#define MB_PLATFORM_LINUX

#elif defined(__APPLE__)
	#include "TargetConditionals.h"

	#if defined(TARGET_OS_MAC)	
		#define MB_PLATFORM_MACOS	
	#elif defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)	
		#error "Unable to determine platform."	
	#else	
		#error "Unable to determine platform."	
	#endif

#else
	#error "Unable to determine platform."

#endif

// Disable secure function warnings, we won't be using any of them for obvious
// portability reasons.
#if defined(_MSC_VER)
	#ifndef _CRT_SECURE_NO_WARNINGS
		#define _CRT_SECURE_NO_WARNINGS (1)

	#endif
	#ifndef _CRT_SECURE_NO_DEPRECATE
		#define _CRT_SECURE_NO_DEPRECATE (1)

	#endif

	// "This function or variable may be unsafe ..."
	#pragma warning(disable : 4996) 

	// function': name was marked as #pragma deprecated
	#pragma warning(disable : 4995)

#endif

// min/max macros if they don't already exists.
#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif

#ifndef max
#define max(x, y) ((x) > (y) ? (x) : (y))
#endif