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

#ifndef SERVERPRIVATEAMD_ICE
#define SERVERPRIVATEAMD_ICE

#include <TestAMD.ice>

exception UnknownDerived extends Base
{
    string ud;
};

exception UnknownIntermediate extends Base
{
   string ui;
};

exception UnknownMostDerived1 extends KnownIntermediate
{
   string umd1;
};

exception UnknownMostDerived2 extends UnknownIntermediate
{
   string umd2;
};

#endif
