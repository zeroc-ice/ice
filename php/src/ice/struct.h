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

#ifndef ICE_PHP_STRUCT_H
#define ICE_PHP_STRUCT_H

#include "marshal.h"

bool Struct_register(const Slice::StructPtr& TSRMLS_DC);
MarshalerPtr Struct_create_marshaler(const Slice::StructPtr& TSRMLS_DC);

#endif
