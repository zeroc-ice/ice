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
// Exception
//
FreezeScript::Exception::Exception(const char* file, int line, const string& reason) :
    IceUtil::Exception(file, line), _reason(reason)
{
}

string FreezeScript::Exception::_name = "FreezeScript::Exception";

const string&
FreezeScript::Exception::ice_name() const
{
    return _name;
}

void
FreezeScript::Exception::ice_print(ostream& out) const
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
FreezeScript::Exception::ice_clone() const
{
    return new Exception(ice_file(), ice_line(), _reason);
}

void
FreezeScript::Exception::ice_throw() const
{
    throw *this;
}

string
FreezeScript::Exception::reason() const
{
    return _reason;
}
