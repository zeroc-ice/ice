// **********************************************************************
//
// Copyright (c) 2001
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

class ContactFactory extends Ice.LocalObjectImpl implements Ice.ObjectFactory, Freeze.ServantInitializer
{
    public Ice.Object
    create(String type)
    {
	assert(type.equals("::Contact"));
	return new ContactI(_phoneBook, _evictor);
    }

    public void
    destroy()
    {
    }

    public void
    initialize(Ice.ObjectAdapter adapter, Ice.Identity ident, Ice.Object servant)
    {
	ContactI contact = (ContactI)servant;
	contact.setIdentity(ident);
    }

    ContactFactory(PhoneBookI phoneBook, Freeze.Evictor evictor)
    {
	_phoneBook = phoneBook;
	_evictor = evictor;
    }

    private PhoneBookI _phoneBook;
    private Freeze.Evictor _evictor;
}
