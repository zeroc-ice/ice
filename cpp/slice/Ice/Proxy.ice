// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PROXY_ICE
#define ICE_PROXY_ICE

module IceProxy
{

module Ice
{

/**
 *
 * The Ice Proxy object. 
 * Used whenever a remote method call is invoked.
 *
 **/
local interface _guard_Object
{
    /**
     *
     *
     **/
    string _getIdentity();

    Object * _newIdentity(string identity);

    Object * _twoway();

    Object * _oneway();

    Object * _batchOneway();

    Object * _datagram();

    Object * _batchDatagram();

    Object * _secure(bool secure);

    Object * _timeout(int timeout);

    void _flush(); 

};

};

};

#endif
