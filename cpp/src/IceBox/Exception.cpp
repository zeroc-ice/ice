// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceBox/IceBox.h>

using namespace std;

void
IceBox::FailureException::ice_print(ostream& out) const
{
#ifdef __BCPLUSPLUS__
    Ice::Exception::ice_print(out);
#else
    Exception::ice_print(out);
#endif
    out << ":\nservice failure exception: " << reason;
}

