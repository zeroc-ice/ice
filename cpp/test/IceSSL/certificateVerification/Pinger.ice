// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
