// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/MessageAuthenticator.h>

using Ice::ByteSeq;

void ::IceInternal::incRef(::IceSecurity::SecureUdp::MessageAuthenticator* p) { p->__incRef(); }
void ::IceInternal::decRef(::IceSecurity::SecureUdp::MessageAuthenticator* p) { p->__decRef(); }

IceSecurity::SecureUdp::MessageAuthenticator::MessageAuthenticator()
{
    // TODO: Should generate a random MAC key here
    int i = 0;

    // Bogus MAC - gotta fix this.
    _macKeyBytes[i++] = 2;
    _macKeyBytes[i++] = 0;
    _macKeyBytes[i++] = 0;
    _macKeyBytes[i++] = 2;
    _macKeyBytes[i++] = 0;
    _macKeyBytes[i++] = 1;
    _macKeyBytes[i++] = 1;
    _macKeyBytes[i++] = 7;
    _macKeyBytes[i++] = 1;
    _macKeyBytes[i++] = 0;
    _macKeyBytes[i++] = 2;
    _macKeyBytes[i++] = 2;
}

IceSecurity::SecureUdp::MessageAuthenticator::MessageAuthenticator(const ByteSeq& macKey)
{
    _macKeyBytes = macKey;
}

IceSecurity::SecureUdp::MessageAuthenticator::~MessageAuthenticator()
{
}

ByteSeq
IceSecurity::SecureUdp::MessageAuthenticator::computeMAC(const ByteSeq& message) const
{
    // TODO: Should generate a REAL MAC here.
    ByteSeq bytes;
    int i = 0;

    // Bogus MAC - gotta fix this.
    bytes[i++] = 2;
    bytes[i++] = 0;
    bytes[i++] = 0;
    bytes[i++] = 2;
    bytes[i++] = 0;
    bytes[i++] = 1;
    bytes[i++] = 1;
    bytes[i++] = 7;
    bytes[i++] = 1;
    bytes[i++] = 0;
    bytes[i++] = 2;
    bytes[i++] = 2;

    return bytes;
}

bool
IceSecurity::SecureUdp::MessageAuthenticator::authenticate(const ByteSeq& message, const ByteSeq& macCode)
{
    ByteSeq targetMAC = computeMAC(message);
    return targetMAC == macCode;
}

const ByteSeq&
IceSecurity::SecureUdp::MessageAuthenticator::getMACKey() const
{
    return _macKeyBytes;
}

