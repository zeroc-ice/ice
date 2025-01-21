// Copyright (c) ZeroC, Inc.

#include "Ice/UserException.h"
#include "Ice/Demangle.h"
#include "Ice/InputStream.h"
#include "Ice/OutputStream.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

const char*
Ice::UserException::what() const noexcept
{
    return ice_id();
}

void
Ice::UserException::ice_print(ostream& os) const
{
    string className{demangle(typeid(*this).name())};
    // On Windows, the class name is prefixed with "class "; we removed it.
    if (className.compare(0, 6, "class ") == 0)
    {
        className.erase(0, 6);
    }

    os << className << '{';
    ice_printFields(os);
    os << '}';
}

void
Ice::UserException::ice_printFields(ostream&) const
{
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
