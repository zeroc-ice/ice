// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef PINGER_ICE
#define PINGER_ICE

#include <Ice/BuiltinSequences.ice>

class KeyManager
{
    void getServerCerts(; Ice::ByteSeq trusted, Ice::ByteSeq untrusted);
    void getTrustedClientKeys(; Ice::ByteSeq key, Ice::ByteSeq cert);
    void getUntrustedClientKeys(; Ice::ByteSeq key, Ice::ByteSeq cert);
    void shutdown();
};

class Pinger
{
    void ping();
};

#endif
