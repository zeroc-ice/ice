// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
    public const System.String ICE_STRING_VERSION = "1.3.0.1"; // "A.B.C", with A=major, B=minor, C=patch
    public const int ICE_INT_VERSION = 010300; // AABBCC, with AA=major, BB=minor, CC=patch
}

}