// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PROPERTIES_ICE
#define ICE_PROPERTIES_ICE

module Ice
{

local class Properties
{
    string getProperty(string key);
    void setProperty(string key, string value);
};

};

#endif
