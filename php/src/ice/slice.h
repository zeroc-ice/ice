// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_PHP_SLICE_H
#define ICE_PHP_SLICE_H

#include "common.h"
#include <Slice/Parser.h>

bool Slice_init(TSRMLS_D);
bool Slice_shutdown(TSRMLS_D);
bool Slice_createClasses(int TSRMLS_DC);
bool Slice_destroyClasses(TSRMLS_D);
Slice::UnitPtr Slice_getUnit();
bool Slice_isNativeKey(const Slice::TypePtr&);

#endif
