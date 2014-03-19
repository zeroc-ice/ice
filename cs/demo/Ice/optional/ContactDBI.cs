// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;
using System.Collections.Generic;

public class ContactDBI : ContactDBDisp_
{
    Dictionary<string,Contact> _contacts = new Dictionary<string,Contact>();

    public override void addContact(string name, Ice.Optional<Demo.NumberType> type, Ice.Optional<string> number, Ice.Optional<int> dialGroup,
            Ice.Current current)
    {
        Contact contact = new Contact();
        contact.name = name;
        if(type.HasValue)
        {
            contact.type = type;
        }
        if(number.HasValue)
        {
            contact.number = number;
        }
        if(dialGroup.HasValue)
        {
            contact.dialGroup = dialGroup;
        }
        _contacts[name] = contact;
    }

    public override void updateContact(string name, Ice.Optional<Demo.NumberType> type, Ice.Optional<string> number,
            Ice.Optional<int> dialGroup, Ice.Current current)
    {
        Contact c;
        if(_contacts.TryGetValue(name, out c))
        {
            if(type.HasValue)
            {
                c.type = type;
            }
            if(number.HasValue)
            {
                c.number = number;
            }
            if(dialGroup.HasValue)
            {
                c.dialGroup = dialGroup;
            }
        }
    }

    public override Demo.Contact query(string name, Ice.Current current)
    {
        Contact c;
        if(_contacts.TryGetValue(name, out c))
        {
            return c;
        }
        return null;
    }

    public override Ice.Optional<string> queryNumber(string name, Ice.Current current)
    {
        Contact c;
        if(_contacts.TryGetValue(name, out c))
        {
            return c.number;
        }
        return Ice.Util.None;
    }

    public override void queryDialgroup(string name, out Ice.Optional<int> dialGroup, Ice.Current current)
    {
        Contact c;
        if(_contacts.TryGetValue(name, out c))
        {
            dialGroup = c.dialGroup;
        }
        else
        {
            dialGroup = Ice.Util.None;
        }
    }
    
    public override void shutdown(Ice.Current current)
    {
        System.Console.Out.WriteLine("Shutting down...");
        current.adapter.getCommunicator().shutdown();
    }
}
