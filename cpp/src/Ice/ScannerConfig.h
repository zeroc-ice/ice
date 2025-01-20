// Copyright (c) ZeroC, Inc.

#ifndef ICE_SCANNER_CONFIG_H
#define ICE_SCANNER_CONFIG_H

#include "Ice/Config.h" // Required by generated Scanners.

#ifdef _MSC_VER
// Suppress the scanner's inclusion of the non-ANSI header file 'unistd.h'.
#    define YY_NO_UNISTD_H

// Warning C4018: signed/unsigned mismatch
#    pragma warning(disable : 4018)
// Warning C4244: conversion from `int` to `_Elem`, possible loss of data
#    pragma warning(disable : 4244)
// warning C4267: conversion from 'size_t' to 'int', possible loss of data
// The result of fread() is a size_t and gets inserted into an int
#    pragma warning(disable : 4267)
// Warning C4505: unreferenced function with internal linkage 'yy_fatal_error'
#    pragma warning(disable : 4505)
#endif

#ifdef __clang__
#    pragma clang diagnostic ignored "-Wconversion"
#    pragma clang diagnostic ignored "-Wsign-compare"
#    pragma clang diagnostic ignored "-Wunused-but-set-variable"
#endif

#ifdef __GNUC__
#    pragma GCC diagnostic ignored "-Wunused-function"
#    pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#    pragma GCC diagnostic ignored "-Wsign-compare"
#endif

#endif
