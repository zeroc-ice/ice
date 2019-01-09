// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <IceBox/IceBox.h>

using namespace std;

void
IceBox::FailureException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nservice failure exception: " << reason;
}
