//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

//
// This macro sets the default value only when compiling with slice2cs.
//
#ifdef __SLICE2CS__
#   define DEFAULT(X) = X
#else
#   define DEFAULT(X) /**/
#endif

//
// This macro sets the default value only when not compiling with slice2cs.
//
#ifndef __SLICE2CS__
#   define NODEFAULT(X) = X
#else
#   define NODEFAULT(X) /**/
#endif

module ZeroC::Slice::Test::Macros
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
// This class is only defined when compiling with slice2cs.
//
#ifdef __SLICE2CS__
class CsOnly
{
    string lang DEFAULT("cs");
    int version DEFAULT(ICE_VERSION);
}
#endif

}
