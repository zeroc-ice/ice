// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PICKLER_ICE
#define ICE_PICKLER_ICE

module Ice
{

native InputStream;
native OutputStream;

local class Pickler
{
    void pickle(Object object, OutputStream out);
    Object unpickle(string type, InputStream in);
};

};

#endif

