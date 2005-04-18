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

using namespace std;
using namespace Demo;

// XXX Add using namespace Ice.

class HelloI : public Hello
{
public:

    HelloI(int id) :
	_id(id)
    {
    }

    ~HelloI()
    {
	// XXX Use real output, something like "`Hello' object #xxx
	// destroyed.", not programming language style output.
	cout << _id << ": ~Hello" << endl;
    }

    void
    sayHello(const Ice::Current&) const
    {
	cout << _id << ": Hello World!" << endl;
    }

private:

    const int _id;
};

SessionI::SessionI(const Ice::ObjectAdapterPtr& adapter, const IceUtil::Time& timeout) :
    _adapter(adapter),
    _timeout(timeout),
    _destroy(false),
    _refreshTime(IceUtil::Time::now()),
    _nextId(0)
{
}

SessionI::~SessionI()
{
    // XXX Remove, or add user-readable comment.
    cout << "~SessionI" << endl;
}

HelloPrx
SessionI::createHello(const Ice::Current&)
{
    Lock sync(*this);
    // XXX Check for destruction w/ ObjectNotExistException?
    HelloPrx hello = HelloPrx::uncheckedCast(_adapter->addWithUUID(new HelloI(_nextId++)));
    _objs.push_back(hello);
    return hello;
}

void
SessionI::refresh(const Ice::Current& c)
{
    Lock sync(*this);
    // XXX Check for destruction w/ ObjectNotExistException?
    _refreshTime = IceUtil::Time::now();
}

void
SessionI::destroy(const Ice::Current& c)
{
    Lock sync(*this);
    // XXX Check for destruction w/ ObjectNotExistException?
    _destroy = true;
    // XXX Add cleanup from the so-called "destroyCallback" here.
}

bool
SessionI::destroyed() const
{
    // XXX This should only check for _destroy. A reaper thread should
    // call destroy() if there was a timeout.
    Lock sync(*this);
    return _destroy || (IceUtil::Time::now() - _refreshTime) > _timeout;
}

// XXX Get rid of this function, remove the hello objects from the
// object adapter in destroy().
void
SessionI::destroyCallback()
{
    Lock sync(*this);
    // XXX Real output please, that is appropriate for a demo.
    cout << "SessionI::destroyCallback: _destroy=" << _destroy << " timeout="
	 << ((IceUtil::Time::now()-_refreshTime) > _timeout) << endl;
    for(list<HelloPrx>::const_iterator p = _objs.begin(); p != _objs.end(); ++p)
    {
	try
	{
	    _adapter->remove((*p)->ice_getIdentity());
	}
	catch(const Ice::ObjectAdapterDeactivatedException&)
	{
	    // This method is called on shutdown of the server, in
	    // which case this exception is expected.
	}
    }
    _objs.clear();
}
