// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class ContactFactory implements Ice.ObjectFactory
{
    public Ice.Object
    create(String type)
    {
        assert(type.equals("::Demo::Contact"));
        return new ContactI(this);
    }

    public void
    destroy()
    {
    }

    ContactFactory()
    {
    }

    void
    setEvictor(Freeze.Evictor evictor)
    {
        _evictor = evictor;
    }

    Freeze.Evictor
    getEvictor()
    {
        return _evictor;
    }

    private Freeze.Evictor _evictor;
}
