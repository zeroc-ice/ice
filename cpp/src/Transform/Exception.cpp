// **********************************************************************
//
// Copyright (c) 2003
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

#include <Transform/Exception.h>

using namespace std;

//
// TransformException
//
Transform::TransformException::TransformException(const char* file, int line, const string& reason) :
    IceUtil::Exception(file, line), _reason(reason)
{
}

string Transform::TransformException::_name = "Transform:TransformException";

const string&
Transform::TransformException::ice_name() const
{
    return _name;
}

void
Transform::TransformException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nerror occurred during transformation";
    if(!_reason.empty())
    {
        out << ":\n" << _reason;
    }
}

IceUtil::Exception*
Transform::TransformException::ice_clone() const
{
    return new TransformException(ice_file(), ice_line(), _reason);
}

void
Transform::TransformException::ice_throw() const
{
    throw *this;
}

string
Transform::TransformException::reason() const
{
    return _reason;
}
