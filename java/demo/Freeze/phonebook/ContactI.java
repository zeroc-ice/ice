// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
	return _name;
    }

    synchronized public void
    setName(String name, Ice.Current current)
	throws DatabaseException
    {
	assert(_identity.name.length() != 0);
	_phonebook.move(_identity, _name, name);
	_name = name;
    }

    synchronized public String
    getAddress(Ice.Current current)
    {
	return _address;
    }

    synchronized public void
    setAddress(String address, Ice.Current current)
    {
	_address = address;
    }

    synchronized public String
    getPhone(Ice.Current current)
    {
	return _phone;
    }

    synchronized public void
    setPhone(String phone, Ice.Current current)
    {
	_phone = phone;
    }

    synchronized public void
    destroy(Ice.Current current)
	throws DatabaseException
    {
	try
	{
	    assert(_identity.name.length() != 0);
	    _phonebook.remove(_identity, _name);

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
	_name = new String();
	_address = new String();
	_phone = new String();
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
