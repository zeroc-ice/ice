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

class ContactFactory extends Ice.LocalObjectImpl implements Ice.ObjectFactory
{
    public Ice.Object
    create(String type)
    {
	assert(type.equals("::Contact"));
	return new ContactI(this);
    }

    public void
    destroy()
    {
    }

    ContactFactory()
    {
    }

    void
    setEvictor(Freeze.Evictor evictor)
    {
	_evictor = evictor;
    }

    Freeze.Evictor
    getEvictor()
    {
	return _evictor;
    }

    private Freeze.Evictor _evictor;
}
