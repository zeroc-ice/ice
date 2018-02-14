// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// This macro sets the default value only when compiling with slice2py.
//
#ifdef __SLICE2PY__
#   define DEFAULT(X) = X
#else
#   define DEFAULT(X) /**/
#endif

//
// This macro sets the default value only when not compiling with slice2py.
//
#ifndef __SLICE2PY__
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
};

class NoDefault
{
    int x NODEFAULT(10);
    int y NODEFAULT(10);
};

//
// This class is only defined when compiling with slice2py.
//
#ifdef __SLICE2PY__
class PythonOnly
{
    string lang DEFAULT("python");
    int version DEFAULT(ICE_VERSION);
};
#endif

};
