// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Time.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

class AMI_Servant_setValueAsyncI : public Test::AMI_Servant_setValueAsync
{
public:
    void ice_response() {}
    void ice_exception(const Ice::Exception&) {}
};

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    string ref = "factory:default -p 12345 -t 2000";
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    Test::RemoteEvictorFactoryPrx factory = Test::RemoteEvictorFactoryPrx::checkedCast(base);

    cout << "testing Freeze Evictor... " << flush;
    
    const Ice::Int size = 5;
    Ice::Int i;
    
    Test::RemoteEvictorPrx evictor = factory->createEvictor("Test");
    evictor->setSize(size);
    
    //
    // Create some servants 
    //
    vector<Test::ServantPrx> servants;
    for(i = 0; i < size; i++)
    {
	servants.push_back(evictor->createServant(i, i));
	servants[i]->addFacet("facet1", "data");
	Test::FacetPrx facet1 = Test::FacetPrx::checkedCast(servants[i], "facet1");
	test(facet1);
	facet1->setValue(10 * i);
	facet1->addFacet("facet2", "moreData");
	Test::FacetPrx facet2 = Test::FacetPrx::checkedCast(facet1, "facet2");
	test(facet2);
	facet2->setValue(100 * i);
    }
   
    //
    // Evict and verify values.
    //
    evictor->setSize(0);
    evictor->setSize(size);
    for(i = 0; i < size; i++)
    {
	test(servants[i]->getValue() == i);
	Test::FacetPrx facet1 = Test::FacetPrx::checkedCast(servants[i], "facet1");
	test(facet1);
	test(facet1->getValue() == 10 * i);
	test(facet1->getData() == "data");
	Test::FacetPrx facet2 = Test::FacetPrx::checkedCast(facet1, "facet2");
	test(facet2);
	test(facet2->getData() == "moreData");
    }
    
    //
    // Mutate servants.
    //
    for(i = 0; i < size; i++)
    {
	servants[i]->setValue(i + 100);
	Test::FacetPrx facet1 = Test::FacetPrx::checkedCast(servants[i], "facet1");
	test(facet1);
	facet1->setValue(10 * i + 100);
	Test::FacetPrx facet2 = Test::FacetPrx::checkedCast(facet1, "facet2");
	test(facet2);
	facet2->setValue(100 * i + 100);
    }
    
    //
    // Evict and verify values.
    //
    evictor->setSize(0);
    evictor->setSize(size);
    for(i = 0; i < size; i++)
    {
	test(servants[i]->getValue() == i + 100);
	Test::FacetPrx facet1 = Test::FacetPrx::checkedCast(servants[i], "facet1");
	test(facet1);
	test(facet1->getValue() == 10 * i + 100);
	Test::FacetPrx facet2 = Test::FacetPrx::checkedCast(facet1, "facet2");
	test(facet2);
	test(facet2->getValue() == 100 * i + 100);
    }
    // 
    // Test saving while busy
    //
    Test::AMI_Servant_setValueAsyncPtr setCB = new AMI_Servant_setValueAsyncI;
    for(i = 0; i < size; i++)
    {
	//
	// Start a mutating operation so that the object is not idle.
	//
	servants[i]->setValueAsync_async(setCB, i + 300);
	//
	// Wait for setValueAsync to be dispatched.
	//
	IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(100));

	test(servants[i]->getValue() == i + 100);
	//
	// This operation modifies the object state but is not saved
	// because the setValueAsync operation is still pending.
	//
	servants[i]->setValue(i + 200);
	test(servants[i]->getValue() == i + 200);

	//
	// Force the response to setValueAsync
	//
	servants[i]->releaseAsync();
	test(servants[i]->getValue() == i + 300);
    }

    //
    // Add duplicate facet and catch corresponding exception
    // 
    for(i = 0; i < size; i++)
    {
	try
	{
	    servants[i]->addFacet("facet1", "foobar");
	    test(false);
	}
	catch(const Test::AlreadyRegisteredException&)
	{
	}
    }
    
    //
    // Remove a facet that does not exist
    // 
    try
    {
	servants[0]->removeFacet("facet3");
	test(false);
    }
    catch(const Test::NotRegisteredException&)
    {
    }

    //
    // Remove a facet that does exist
    //
    servants[0]->removeFacet("facet1");
    Test::FacetPrx facet1 = Test::FacetPrx::checkedCast(servants[0], "facet1");
    test(facet1 == 0);
    
    //
    // Remove all facets
    //
    for(i = 0; i < size; i++)
    {
	servants[i]->removeAllFacets();
    }

    evictor->setSize(0);
    evictor->setSize(size);

    for(i = 0; i < size; i++)
    {
	test(servants[i]->getValue() == i + 300);

	facet1 = Test::FacetPrx::checkedCast(servants[i], "facet1");
	test(facet1 == 0);
    }
    
    //
    // Destroy servants and verify ObjectNotExistException.
    //
    for(i = 0; i < size; i++)
    {
	servants[i]->destroy();
	try
	{
	    servants[i]->getValue();
	    test(false);
	}
	catch(const Ice::ObjectNotExistException&)
	{
	    // Expected
	}
    }
    
    //
    // Allocate space for size+1 servants.
    //
    servants.clear();
    
    //
    // Recreate servants.
    //
    for(i = 0; i < size; i++)
    {
	servants.push_back(evictor->createServant(i, i));
    }
    
    //
    // Deactivate and recreate evictor, to ensure that servants
    // are restored properly after database close and reopen.
    //
    evictor->deactivate();
    evictor = factory->createEvictor("Test");
    evictor->setSize(size);
    for(i = 0; i < size; i++)
    {
	servants[i] = evictor->getServant(i);
	test(servants[i]->getValue() == i);
    }
    
    //
    // Clean up.
    //
    evictor->destroyAllServants();
    for(i = 0; i < size; i++)
    {
	try
	{
	    servants[i]->getValue();
	    test(false);
	}
	catch(const Ice::ObjectNotExistException&)
	{
	    // Expected
	}
    }

    evictor->deactivate();
    cout << "ok" << endl;

    factory->shutdown();
    
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

    if(communicator)
    {
        try
        {
            communicator->destroy();
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            status = EXIT_FAILURE;
        }
    }

    return status;
}
