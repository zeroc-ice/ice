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

