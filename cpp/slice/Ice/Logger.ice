// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_LOGGER_ICE
#define ICE_LOGGER_ICE

module Ice
{

local class Logger
{
    void trace(string category, string message);
    void warning(string message);
    void error(string message);
};

};

#endif
