// Copyright (c) ZeroC, Inc.

//
// This macro sets the default value only when compiling with slice2cpp.
//
#ifdef __SLICE2CPP__
#   define DEFAULT(X) = X
#else
#   define DEFAULT(X) /**/
#endif

//
// This macro sets the default value only when not compiling with slice2cpp.
//
#ifndef __SLICE2CPP__
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

    //
    // This class is only defined when compiling with slice2cpp.
    //
    #ifdef __SLICE2CPP__
    class CppOnly
    {
        string lang DEFAULT("cpp");
        int version DEFAULT(ICE_VERSION);
    }
    #endif
}
