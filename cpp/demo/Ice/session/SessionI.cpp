// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <SessionFactoryI.h>
#include <SessionI.h>

using namespace std;
using namespace Demo;

class HelloI : public Hello
{
public:

    HelloI(int id) :
	_id(id)
    {
    }

    virtual ~HelloI()
    {
	cout << "Hello object #" << _id << " destroyed" << endl;
    }

    void
    sayHello(const Ice::Current&) const
    {
	cout << "Hello object #" << _id << " says 'Hello World!'" << endl;
    }

private:

    const int _id;
};

HelloPrx
SessionI::createHello(const Ice::Current& c)
{
    Lock sync(*this);
    if(_destroy)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    HelloPrx hello = HelloPrx::uncheckedCast(c.adapter->addWithUUID(new HelloI(_nextId++)));
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

void
SessionI::destroy(const Ice::Current& c)
{
    Lock sync(*this);
    if(_destroy)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    _destroy = true;

    cout << "The session #" << Ice::identityToString(c.id) << " is now destroyed." << endl;
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

SessionI::SessionI() :
    _timestamp(IceUtil::Time::now()),
    _nextId(0),
    _destroy(false)
{
}
