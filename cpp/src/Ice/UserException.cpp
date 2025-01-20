//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/UserException.h"
#include "Ice/InputStream.h"
#include "Ice/OutputStream.h"

using namespace std;
using namespace Ice;

const char*
Ice::UserException::what() const noexcept
{
    return ice_id();
}

void
Ice::UserException::ice_print(ostream& os) const
{
    os << ice_id();
}

void
Ice::UserException::_write(OutputStream* os) const
{
    os->startException();
    _writeImpl(os);
    os->endException();
}

void
Ice::UserException::_read(InputStream* is)
{
    is->startException();
    _readImpl(is);
    is->endException();
}

bool
Ice::UserException::_usesClasses() const
{
    return false;
}
