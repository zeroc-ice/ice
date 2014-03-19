// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class PhoneBookI extends _PhoneBookDisp
{
    public ContactPrx
    createContact(Ice.Current current)
        throws DatabaseException
    {
        //
        // Generate a new unique identity.
        //
        Ice.Identity ident = new Ice.Identity();
        ident.name = java.util.UUID.randomUUID().toString();
        ident.category = "contact";

        //
        // Create a new Contact Servant.
        //
        ContactI contact = new ContactI(_contactFactory);
    
        //
        // Create a new Ice Object in the evictor, using the new
        // identity and the new Servant.
        //
        _evictor.add(contact, ident);

        return ContactPrxHelper.uncheckedCast(current.adapter.createProxy(ident));
    }

    public ContactPrx[]
    findContacts(String name, Ice.Current current)
        throws DatabaseException
    {
        try
        {
            Ice.Identity[] identities = _index.find(name);

            ContactPrx[] contacts = new ContactPrx[identities.length];
            for(int i = 0; i < identities.length; ++i)
            {
                contacts[i] = ContactPrxHelper.uncheckedCast
                    (current.adapter.createProxy(identities[i]));
            }
            return contacts;
        }
        catch(Freeze.DatabaseException ex)
        {
            DatabaseException e = new DatabaseException();
            e.message = ex.message;
            throw e;
        }
    }

    public void
    setEvictorSize(int size, Ice.Current current)
        throws DatabaseException
    {
        //
        // No synchronization necessary, _evictor is immutable.
        //
        _evictor.setSize(size);
    }

    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    PhoneBookI(Freeze.Evictor evictor, ContactFactory contactFactory, 
               NameIndex index)
    {
        _evictor = evictor;
        _contactFactory = contactFactory;
        _index = index;
    }
    
    private Freeze.Evictor _evictor;
    private ContactFactory _contactFactory;
    private NameIndex _index;
}
