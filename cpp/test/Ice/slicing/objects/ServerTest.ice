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

#ifndef SERVERTEST_ICE
#define SERVERTEST_ICE

#include <ClientTest.ice>

class SBSUnknownDerived extends SBase
{
    string sbsud;
};

class SUnknown
{
    string su;
};

class D2 extends B
{
    string sd2;
    B pd2;
};

class D4 extends B
{
    B p1;
    B p2;
};

exception UnknownDerivedException extends BaseException
{
    string sude;
    D2 pd2;
};

#endif
