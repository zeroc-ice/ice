// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <ContactFactory.h>
#include <PhoneBookI.h>

using namespace std;

ContactFactory::ContactFactory()
{
}

Ice::ObjectPtr
#ifndef NDEBUG
ContactFactory::create(const string& type)
#else
ContactFactory::create(const string&)
#endif
{
    assert(_evictor);
    assert(type == "::Demo::Contact");
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
    
Freeze::EvictorPtr
ContactFactory::getEvictor() const
{
    return _evictor;
}

