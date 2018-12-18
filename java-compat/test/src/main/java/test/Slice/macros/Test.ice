// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

[["java:package:test.Slice.macros"]]

//
// This macro sets the default value only when compiling with slice2java.
//
#ifdef __SLICE2JAVA__
#   define DEFAULT(X) = X
#else
#   define DEFAULT(X) /**/
#endif

//
// This macro sets the default value only when not compiling with slice2java.
//
#ifndef __SLICE2JAVA__
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
// This class is only defined when compiling with slice2java.
//
#ifdef __SLICE2JAVA__
class JavaOnly
{
    string lang DEFAULT("java");
    int version DEFAULT(ICE_VERSION);
}
#endif

}
