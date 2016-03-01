// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Stream.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* Ice::upCast(InputStream* p) { return p; }
IceUtil::Shared* Ice::upCast(OutputStream* p) { return p; }

void
Ice::UserExceptionReader::__writeImpl(::IceInternal::BasicStream*) const
{
    assert(false); // Should never be called.
}

void
Ice::UserExceptionReader::__readImpl(::IceInternal::BasicStream*)
{
    assert(false); // Should never be called.
}

bool
Ice::UserExceptionReader::__usesClasses() const
{
    return usesClasses();
}

void
Ice::UserExceptionWriter::__writeImpl(::IceInternal::BasicStream*) const
{
    assert(false); // Should never be called.
}

void
Ice::UserExceptionWriter::__readImpl(::IceInternal::BasicStream*)
{
    assert(false); // Should never be called.
}

bool
Ice::UserExceptionWriter::__usesClasses() const
{
    return usesClasses();
}
