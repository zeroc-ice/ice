// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/StreamI.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

Ice::StreamI::StreamI(const InstancePtr& instance) :
    _stream(instance)
{
}
