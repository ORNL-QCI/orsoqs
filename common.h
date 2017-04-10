#ifndef ORSOQS_COMMON_H
#define ORSOQS_COMMON_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if __x86_64__ || \
	_M_X64 || \
	_WIN64 || \
	__powerpc64__ || \
	__ppc64__ || \
	__PPC64__ || \
	__ARCH_PPC64 || \
	__aarch64__ || \
	__sparc64__
#	define ORSOQS_64BIT
#else
#	define ORSOQS_32BIT
#endif

// Sanity check
#if !defined(ORSOQS_64BIT) && !defined(ORSOQS_32BIT)
#	error "Neither 32 or 64 bit macro defined"
#endif

#ifndef UNUSED
#	define UNUSED(x) (void)(x)
#endif

#endif
