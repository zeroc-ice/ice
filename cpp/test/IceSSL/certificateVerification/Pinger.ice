// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef PINGER_ICE
#define PINGER_ICE

#include <Ice/BuiltinSequences.ice>

module Test
{

class KeyManager
{
    void getServerCerts(out Ice::ByteSeq trusted, out Ice::ByteSeq untrusted);
    void getTrustedClientKeys(out Ice::ByteSeq key, out Ice::ByteSeq cert);
    void getUntrustedClientKeys(out Ice::ByteSeq key, out Ice::ByteSeq cert);
    void shutdown();
};

class Pinger
{
};

};

#endif
