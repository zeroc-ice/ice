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
#include <Factory.h>
#include <Pickle.h>

using namespace std;

Ice::ObjectPtr
Factory::create(const string& id)
{
    if (id == "::Persistent")
	return new Persistent;

    assert(false);
    return 0;
}
