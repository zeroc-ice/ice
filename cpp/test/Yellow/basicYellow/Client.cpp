// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <Yellow/Yellow.h>
#include <TestCommon.h>

using namespace std;
using namespace Ice;
using namespace Yellow;

class StressThread : public IceUtil::Thread
{
public:

    StressThread(const QueryPrx& query) :
	_query(query)
    {
    }

    virtual void
    run()
    {
	string intf = "::Test";
	for (int i = 0; i < 100; ++i)
	{
	    Ice::ObjectPrx result = _query->lookup(intf);
	}
    }

private:

    QueryPrx _query;
};

static int
run(int argc, char* argv[], const CommunicatorPtr& communicator)
{
    QueryPrx query = QueryPrx::checkedCast(
	communicator->stringToProxy("Yellow/Query:default -p 12346"));

    AdminPrx admin = AdminPrx::checkedCast(
	communicator->stringToProxy("Yellow/Admin:default -p 12347"));

    cout << "testing add... ";

    ObjectAdapterPtr adapter = communicator->createObjectAdapter("dummy");
    ObjectPrx test1 = adapter->createProxy(stringToIdentity("test"));
    ObjectPrx test2 = adapter->createProxy(stringToIdentity("test2"));

    ObjectPrx test3 = adapter->createProxy(stringToIdentity("test3"));
    ObjectPrx test4 = adapter->createProxy(stringToIdentity("test4"));

    admin->add("::Test", test1);
    admin->add("::Test", test2);

    ObjectProxySeq seq = query->lookupAll("::Test");
    test(seq.size() == 2);
    Identity ident = seq[0]->ice_getIdentity();
    test(ident == stringToIdentity("test") || ident == stringToIdentity("test2"));
    if(ident == stringToIdentity("test"))
    {
	test(seq[1]->ice_getIdentity() == stringToIdentity("test2"));
    }
    else
    {
	test(seq[1]->ice_getIdentity() == stringToIdentity("test"));
    }

    admin->add("::Test2", test3);
    admin->add("::Test2", test4);

    seq = query->lookupAll("::Test2");
    test(seq.size() == 2);
    ident = seq[0]->ice_getIdentity();
    test(ident == stringToIdentity("test3") || ident == stringToIdentity("test4"));
    if(ident == stringToIdentity("test3"))
    {
	test(seq[1]->ice_getIdentity() == stringToIdentity("test4"));
    }
    else
    {
	test(seq[1]->ice_getIdentity() == stringToIdentity("test3"));
    }

    cout << "ok" << endl;

    cout << "testing lookup... ";
    try
    {
	query->lookup("::Foo");
	test(false);
    }
    catch(const NoSuchOfferException&)
    {
	// Expected
    }
    ObjectPrx obj = query->lookup("::Test");
    ident = obj->ice_getIdentity();
    test(ident == stringToIdentity("test") || ident == stringToIdentity("test2"));

    obj = query->lookup("::Test2");
    ident = obj->ice_getIdentity();
    test(ident == stringToIdentity("test3") || ident == stringToIdentity("test4"));

    cout << "ok" << endl;

    cout << "testing remove... ";
    try
    {
	admin->remove("::Test2", test1);
	test(false);
    }
    catch(const NoSuchOfferException&)
    {
	// Expected
    }

    admin->remove("::Test2", test3);

    obj = query->lookup("::Test2");
    ident = obj->ice_getIdentity();
    test(ident == stringToIdentity("test4"));

    admin->remove("::Test2", test4);

    try
    {
	query->lookup("::Test2");
	test(false);
    }
    catch(const NoSuchOfferException&)
    {
	// Expected
    }
    cout << "ok" << endl;

    cout << "testing concurrent queries... " << flush;
    vector<IceUtil::ThreadControl> controls;
    for(int i = 0; i < 10; ++i)
    {
	IceUtil::ThreadPtr t = new StressThread(query);
	controls.push_back(t->start());
    }
    for(vector<IceUtil::ThreadControl>::iterator p = controls.begin(); p != controls.end(); ++p)
    {
	p->join();
    }
    cout << "ok" << endl;

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
	communicator = Ice::initialize(argc, argv);
	status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    try
    {
	communicator->destroy();
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    return status;
}
