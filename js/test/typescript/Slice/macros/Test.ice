//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

//
// This macro sets the default value only when compiling with slice2js.
//
#ifdef __SLICE2JS__
#   define DEFAULT(X) = X
#else
#   define DEFAULT(X) /**/
#endif

//
// This macro sets the default value only when not compiling with slice2js.
//
#ifndef __SLICE2JS__
#   define NODEFAULT(X) = X
#else
#   define NODEFAULT(X) /**/
#endif

[["js:es6-module"]]

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
// This class is only defined when compiling with slice2js.
//
#ifdef __SLICE2JS__
class JsOnly
{
    string lang DEFAULT("js");
    int version DEFAULT(ICE_VERSION);
}
#endif

}
