// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceBox/IceBox.h>
#include <Ice/Stream.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
IceBox::ServiceFailureException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nservice failure exception";
}
