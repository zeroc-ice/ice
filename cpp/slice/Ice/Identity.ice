// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_IDENTITY_ICE
#define ICE_IDENTITY_ICE

module Ice
{

struct Identity
{
    string name;
    int group;
    string category;
};

dictionary<Identity, Object> ObjectDict;

};

#endif
