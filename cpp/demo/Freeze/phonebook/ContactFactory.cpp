// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#include <ContactFactory.h>
#include <PhoneBookI.h>

using namespace std;
using namespace Ice;
using namespace Freeze;

ContactFactory::ContactFactory()
{
}

ObjectPtr
ContactFactory::create(const string& type)
{
    assert(type == "::Contact");
    return new ContactI(this);
}

void
ContactFactory::destroy()
{
    //
    // Break cyclic object dependencies
    //
    _evictor = 0;
}

void
ContactFactory::setEvictor(const Freeze::EvictorPtr& evictor)
{
    _evictor = evictor;
}
    
const Freeze::EvictorPtr&
ContactFactory::getEvictor() const
{
    return _evictor;
}

