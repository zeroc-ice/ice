// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
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
    void getServerCerts(out Ice::ByteSeq trusted, out Ice::ByteSeq untrusted);
    void getTrustedClientKeys(out Ice::ByteSeq key, out Ice::ByteSeq cert);
    void getUntrustedClientKeys(out Ice::ByteSeq key, out Ice::ByteSeq cert);
    void shutdown();
};

class Pinger
{
    void ping();
};

#endif
