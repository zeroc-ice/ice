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

void ::IceInternal::incRef(::SecureUdp::MessageAuthenticator* p) { p->__incRef(); }
void ::IceInternal::decRef(::SecureUdp::MessageAuthenticator* p) { p->__decRef(); }

SecureUdp::MessageAuthenticator::MessageAuthenticator()
{
    // TODO: Should generate a random MAC key here

    // Bogus MAC - gotta fix this.
    _macKeyBytes.push_back(2);
    _macKeyBytes.push_back(0);
    _macKeyBytes.push_back(0);
    _macKeyBytes.push_back(2);
    _macKeyBytes.push_back(0);
    _macKeyBytes.push_back(1);
    _macKeyBytes.push_back(1);
    _macKeyBytes.push_back(7);
    _macKeyBytes.push_back(1);
    _macKeyBytes.push_back(0);
    _macKeyBytes.push_back(2);
    _macKeyBytes.push_back(2);
}

SecureUdp::MessageAuthenticator::MessageAuthenticator(const ByteSeq& macKey)
{
    _macKeyBytes = macKey;
}

SecureUdp::MessageAuthenticator::~MessageAuthenticator()
{
}

ByteSeq
SecureUdp::MessageAuthenticator::computeMAC(const ByteSeq& message) const
{
    // TODO: Should generate a REAL MAC here.
    ByteSeq bytes;

    // Bogus MAC - gotta fix this.
    bytes.push_back(2);
    bytes.push_back(0);
    bytes.push_back(0);
    bytes.push_back(2);
    bytes.push_back(0);
    bytes.push_back(1);
    bytes.push_back(1);
    bytes.push_back(7);
    bytes.push_back(1);
    bytes.push_back(0);
    bytes.push_back(2);
    bytes.push_back(2);

    return bytes;
}

bool
SecureUdp::MessageAuthenticator::authenticate(const ByteSeq& message, const ByteSeq& macCode)
{
    ByteSeq targetMAC = computeMAC(message);
    return targetMAC == macCode;
}

const ByteSeq&
SecureUdp::MessageAuthenticator::getMACKey() const
{
    return _macKeyBytes;
}

