// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Shared.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

//static int simpleDebugCounter = 0;
//static int debugCounter = 0;
//static JTCMutex debugCounterMutex;

IceInternal::SimpleShared::SimpleShared() :
    _ref(0)
{
//    cout << "new SimpleShared: " << ++simpleDebugCounter << endl;
}

IceInternal::SimpleShared::~SimpleShared()
{
//    cout << "delete SimpleShared: " << --simpleDebugCounter << endl;
}

IceInternal::Shared::Shared() :
    _ref(0)
{
//    JTCSyncT<JTCMutex> sync(debugCounterMutex);
//    cout << "new Shared: " << ++debugCounter << endl;
}

IceInternal::Shared::~Shared()
{
//    JTCSyncT<JTCMutex> sync(debugCounterMutex);
//    cout << "delete Shared: " << --debugCounter << endl;
}
