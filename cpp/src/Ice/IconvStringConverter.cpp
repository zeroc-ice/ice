//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/IconvStringConverter.h>

#ifndef _WIN32

using namespace std;
using namespace Ice;
using namespace IceUtil;

IconvInitializationException::IconvInitializationException(const char* file, int line, string reason) noexcept
    : Exception(file, line),
      _reason(std::move(reason))
{
}

void
IconvInitializationException::ice_print(ostream& out) const
{
    IceUtil::Exception::ice_print(out);
    out << ": " << _reason;
}

string
IconvInitializationException::ice_id() const
{
    return "::Ice::IconvInitializationException";
}

#endif
