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

class ContactI extends Contact
{
    //
    // No read/write mutexes in Java - hence use native
    // synchronization.
    //

    synchronized public String
    getName(Ice.Current current)
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException();
        }

	return name;
    }

    synchronized public void
    setName(String name, Ice.Current current)
	throws DatabaseException
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException();
        }

	assert(_identity.name.length() != 0);
	_phonebook.move(_identity, this.name, name);
	this.name = name;
    }

    synchronized public String
    getAddress(Ice.Current current)
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException();
        }

	return address;
    }

    synchronized public void
    setAddress(String address, Ice.Current current)
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException();
        }

	this.address = address;
    }

    synchronized public String
    getPhone(Ice.Current current)
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException();
        }

	return phone;
    }

    synchronized public void
    setPhone(String phone, Ice.Current current)
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException();
        }

	this.phone = phone;
    }

    synchronized public void
    destroy(Ice.Current current)
	throws DatabaseException
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException();
        }

        _destroyed = true;

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
	catch(Freeze.NotFoundException ex)
	{
	    //
	    // Raised by remove. Ignore.
	    //
	}
	catch(Freeze.DatabaseException ex)
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
        _destroyed = false;

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
    private boolean _destroyed;
}
