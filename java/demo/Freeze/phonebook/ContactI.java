// **********************************************************************
//
// Copyright (c) 2002
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

class ContactI extends Contact
{
    //
    // No read/write mutexes in Java - hence use native
    // synchronization.
    //

    synchronized public String
    getName(Ice.Current current)
    {
	return name;
    }

    synchronized public void
    setName(String name, Ice.Current current)
	throws DatabaseException
    {
	assert(_identity.name.length() != 0);
	_phonebook.move(_identity, this.name, name);
	this.name = name;
    }

    synchronized public String
    getAddress(Ice.Current current)
    {
	return address;
    }

    synchronized public void
    setAddress(String address, Ice.Current current)
    {
	this.address = address;
    }

    synchronized public String
    getPhone(Ice.Current current)
    {
	return phone;
    }

    synchronized public void
    setPhone(String phone, Ice.Current current)
    {
	this.phone = phone;
    }

    synchronized public void
    destroy(Ice.Current current)
	throws DatabaseException
    {
	try
	{
	    assert(_identity.name.length() != 0);
	    _phonebook.remove(_identity, name);

	    //
	    // This can throw EvictorDeactivatedException (which
	    // indicates an internal error). The exception is
	    // currently ignored.
	    //
	    _evictor.destroyObject(_identity);
	}
	catch(Freeze.DBNotFoundException ex)
	{
	    //
	    // Raised by remove. Ignore.
	    //
	}
	catch(Freeze.DBException ex)
	{
	    DatabaseException e = new DatabaseException();
	    e.message = ex.message;
	    throw e;
	}
    }

    ContactI(PhoneBookI phonebook, Freeze.Evictor evictor)
    {
	_phonebook = phonebook;
	_evictor = evictor;

	//
	// It's possible to avoid this if there were two constructors
	// - one for original creation of the Contact and one for
	// loading of an existing Contact.
	//
	name = new String();
	address = new String();
	phone = new String();
    }

    protected void
    setIdentity(Ice.Identity identity)
    {
	_identity = identity;
    }

    private PhoneBookI _phonebook;
    private Freeze.Evictor _evictor;
    private Ice.Identity _identity;
	
}
