// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
