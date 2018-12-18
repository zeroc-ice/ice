// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// This macro sets the default value only when compiling with slice2php.
//
#ifdef __SLICE2PHP__
#   define DEFAULT(X) = X
#else
#   define DEFAULT(X) /**/
#endif

//
// This macro sets the default value only when not compiling with slice2php.
//
#ifndef __SLICE2PHP__
#   define NODEFAULT(X) = X
#else
#   define NODEFAULT(X) /**/
#endif

module Test
{

class Default
{
    int x DEFAULT(10);
    int y DEFAULT(10);
}

class NoDefault
{
    int x NODEFAULT(10);
    int y NODEFAULT(10);
}

//
// This class is only defined when compiling with slice2php.
//
#ifdef __SLICE2PHP__
class PhpOnly
{
    string lang DEFAULT("php");
    int version DEFAULT(ICE_VERSION);
}
#endif

}
