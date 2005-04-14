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

class HelloI : public Hello
{
public:

    HelloI(int id) :
	_id(id)
    {
    }

    ~HelloI()
    {
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
    cout << "~SessionI" << endl;
}

HelloPrx
SessionI::createHello(const Ice::Current&)
{
    Lock sync(*this);
    HelloPrx hello = HelloPrx::uncheckedCast(_adapter->addWithUUID(new HelloI(_nextId++)));
    _objs.push_back(hello);
    return hello;
}

void
SessionI::refresh(const Ice::Current& c)
{
    Lock sync(*this);
    _refreshTime = IceUtil::Time::now();
}

void
SessionI::destroy(const Ice::Current& c)
{
    Lock sync(*this);
    _destroy = true;
}

bool
SessionI::destroyed() const
{
    Lock sync(*this);
    return _destroy || (IceUtil::Time::now() - _refreshTime) > _timeout;
}

void
SessionI::destroyCallback()
{
    Lock sync(*this);
    cout << "SessionI::destroyCallback: _destroy=" << _destroy << " timeout="
	 << ((IceUtil::Time::now()-_refreshTime) > _timeout) << endl;
    for(list<HelloPrx>::const_iterator p = _objs.begin(); p != _objs.end(); ++p)
    {
	_adapter->remove((*p)->ice_getIdentity());
    }
    _objs.clear();
}
