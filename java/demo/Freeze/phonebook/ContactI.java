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
	return name;
    }

    synchronized public void
    setName(String name, Ice.Current current)
	throws DatabaseException
    {
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
	    _evictor.destroyObject(current.id);
	}
	catch(Freeze.DatabaseException ex)
	{
	    DatabaseException e = new DatabaseException();
	    e.message = ex.message;
	    throw e;
	}
    }

    ContactI(Freeze.Evictor evictor)
    {
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

    private Freeze.Evictor _evictor;
 
}
