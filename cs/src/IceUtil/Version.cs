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

namespace IceUtil
{

using System;

public sealed class Version
{
    //
    // The Ice version.
    //
    public const System.String ICE_STRING_VERSION = "1.3.0alpha1"; // "A.B.C", with A=major, B=minor, C=patch
    public const int ICE_INT_VERSION = 010300; // AABBCC, with AA=major, BB=minor, CC=patch
}

}