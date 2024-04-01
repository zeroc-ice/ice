//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_SCANNER_CONFIG_H
#define ICE_UTIL_SCANNER_CONFIG_H

#include "Config.h" // Required by generated Scanners.

#ifdef _MSC_VER
#    define YY_NO_UNISTD_H
// Warning C4244: conversion from `int` to `_Elem`, possible loss of data
#    pragma warning(disable : 4244)
// warning C4267: conversion from 'size_t' to 'int', possible loss of data
// The result of fread() is a size_t and gets inserted into an int
#    pragma warning(disable : 4267)
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
