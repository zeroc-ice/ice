// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <ClockI.h>

using namespace std;

void
ClockI::tick()
{
    cout << "tick" << endl;
}
