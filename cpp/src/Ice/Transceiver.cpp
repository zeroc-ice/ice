// **********************************************************************
//
// Copyright (c) 2002
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

#include <Ice/Transceiver.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(Transceiver* p) { p->__incRef(); }
void IceInternal::decRef(Transceiver* p) { p->__decRef(); }
