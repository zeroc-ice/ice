// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
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
    public const System.String ICE_STRING_VERSION = "1.4.0"; // "A.B.C", with A=major, B=minor, C=patch
    public const int ICE_INT_VERSION = 10400;                // AABBCC, with AA=major, BB=minor, CC=patch
}

}
