// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

FreezeScript::FailureException::~FailureException() throw()
{
}

const char* FreezeScript::FailureException::_name = "FreezeScript::FailureException";

string
FreezeScript::FailureException::ice_name() const
{
    return _name;
}

void
FreezeScript::FailureException::ice_print(ostream& out) const
{
    ::IceUtil::Exception::ice_print(out);
    out << ":\nerror occurred during transformation"; // TODO
    if(!_reason.empty())
    {
        out << ":\n" << _reason;
    }
}

FreezeScript::FailureException*
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
