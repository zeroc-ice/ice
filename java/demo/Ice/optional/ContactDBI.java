// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class ContactDBI extends _ContactDBDisp
{
    private java.util.Map<String, Contact> _contacts = new java.util.HashMap<String, Contact>();

    public final void
    addContact(String name, Ice.Optional<NumberType> type, Ice.Optional<String> number, Ice.IntOptional dialGroup,
               Ice.Current current)
    {
        Contact contact = new Contact();
        contact.name = name;
        if(type.isSet())
        {
            contact.setType(type.get());
        }
        if(number.isSet())
        {
            contact.setNumber(number.get());
        }
        if(dialGroup.isSet())
        {
            contact.setDialGroup(dialGroup.get());
        }
        _contacts.put(name, contact);
    }

    public final void
    updateContact(String name, Ice.Optional<NumberType> type, Ice.Optional<String> number, Ice.IntOptional dialGroup,
                  Ice.Current current)
    {
        Contact c = _contacts.get(name);
        if(c != null)
        {
            if(type.isSet())
            {
                c.setType(type.get());
            }
            if(number.isSet())
            {
                c.setNumber(number.get());
            }
            if(dialGroup.isSet())
            {
                c.setDialGroup(dialGroup.get());
            }
        }
    }

    public final Contact
    query(String name, Ice.Current current)
    {
        return _contacts.get(name);
    }

    public final void
    queryDialgroup(String name, Ice.IntOptional dialGroup, Ice.Current current)
    {
        Contact c = _contacts.get(name);
        if(c != null)
        {
            dialGroup.set(c.optionalDialGroup());
        }
    }

    public final Ice.Optional<String>
    queryNumber(String name, Ice.Current current)
    {
        Ice.Optional<String> ret = null;
        Contact c = _contacts.get(name);
        if(c != null)
        {
            ret = c.optionalNumber();
        }
        return ret;
    }

    public void
    shutdown(Ice.Current current)
    {
        System.out.println("Shutting down...");
        current.adapter.getCommunicator().shutdown();
    }
}
