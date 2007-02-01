// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <SessionI.h>

using namespace std;
using namespace Demo;

class HelloI : public Hello
{
public:

    HelloI(const string& name, int id) :
        _name(name),
        _id(id)
    {
    }

    virtual ~HelloI()
    {
        cout << "Hello object #" << _id << " for session `" << _name << "' destroyed" << endl;
    }

    void
    sayHello(const Ice::Current&) const
    {
        cout << "Hello object #" << _id << " for session `" << _name << "' says:\n"
             << "Hello " << _name << "!" << endl;
    }

private:

    const string _name;
    const int _id;
};

SessionI::SessionI(const string& name) :
    _name(name),
    _timestamp(IceUtil::Time::now()),
    _nextId(0),
    _destroy(false)
{
    cout << "The session " << _name << " is now created." << endl;
}

HelloPrx
SessionI::createHello(const Ice::Current& c)
{
    Lock sync(*this);
    if(_destroy)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    HelloPrx hello = HelloPrx::uncheckedCast(c.adapter->addWithUUID(new HelloI(_name, _nextId++)));
    _objs.push_back(hello);
    return hello;
}

void
SessionI::refresh(const Ice::Current& c)
{
    Lock sync(*this);
    if(_destroy)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    _timestamp = IceUtil::Time::now();
}

string
SessionI::getName(const Ice::Current&) const
{
    Lock sync(*this);
    if(_destroy)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    return _name;
}

void
SessionI::destroy(const Ice::Current& c)
{
    Lock sync(*this);
    if(_destroy)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    _destroy = true;

    cout << "The session " << _name << " is now destroyed." << endl;
    try
    {
        c.adapter->remove(c.id);
        for(list<HelloPrx>::const_iterator p = _objs.begin(); p != _objs.end(); ++p)
        {
            c.adapter->remove((*p)->ice_getIdentity());
        }
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
        // This method is called on shutdown of the server, in which
        // case this exception is expected.
    }

    _objs.clear();
}

IceUtil::Time
SessionI::timestamp() const
{
    Lock sync(*this);
    if(_destroy)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
    return _timestamp;
}
