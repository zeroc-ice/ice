// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

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

    public void
    destroy(Ice.Current current)
        throws DatabaseException
    {
        try
        {
            _factory.getEvictor().remove(current.id);
        }
        catch(Freeze.DatabaseException ex)
        {
            DatabaseException e = new DatabaseException();
            e.message = ex.message;
            throw e;
        }
    }

    ContactI(ContactFactory factory)
    {
        _factory = factory;
        //
        // It's possible to avoid this if there were two constructors
        // - one for original creation of the Contact and one for
        // loading of an existing Contact.
        //
        name = new String();
        address = new String();
        phone = new String();
    }

    private ContactFactory _factory;
 
}
