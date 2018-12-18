// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/IconvStringConverter.h>

#ifndef _WIN32

using namespace std;
using namespace Ice;
using namespace IceUtil;

IconvInitializationException::IconvInitializationException(const char* file, int line, const string& reason) :
    ExceptionHelper<IconvInitializationException>(file, line),
    _reason(reason)
{
}

#ifndef ICE_CPP11_COMPILER
IconvInitializationException::~IconvInitializationException() throw()
{
}
#endif

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

#ifndef ICE_CPP11_MAPPING
IconvInitializationException*
IconvInitializationException::ice_clone() const
{
    return new IconvInitializationException(*this);
}
#endif

string
IconvInitializationException::reason() const
{
    return _reason;
}
#endif
