// **********************************************************************
//
// Copyright (c) 2004
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

#include <FreezeScript/Exception.h>

using namespace std;

//
// FailureException
//
FreezeScript::FailureException::FailureException(const char* file, int line, const string& reason) :
    IceUtil::Exception(file, line), _reason(reason)
{
}

string FreezeScript::FailureException::_name = "FreezeScript::FailureException";

const string&
FreezeScript::FailureException::ice_name() const
{
    return _name;
}

void
FreezeScript::FailureException::ice_print(ostream& out) const
{
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Exception::ice_print(out);
#else
    ::IceUtil::Exception::ice_print(out);
#endif
    out << ":\nerror occurred during transformation"; // TODO
    if(!_reason.empty())
    {
        out << ":\n" << _reason;
    }
}

IceUtil::Exception*
FreezeScript::FailureException::ice_clone() const
{
    return new FailureException(ice_file(), ice_line(), _reason);
}

void
FreezeScript::FailureException::ice_throw() const
{
    throw *this;
}

string
FreezeScript::FailureException::reason() const
{
    return _reason;
}
