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

    //
    // Test EvictionStrategy
    //
    {
        cout << "testing EvictionStrategy... " << flush;

        const Ice::Int size = 5;
        Ice::Int i;

        Test::RemoteEvictorPrx evictor = factory->createEvictor("EvictionStrategy", Test::Eviction);
        evictor->setSize(size);

        //
        // Create the same number of servants as the evictor size
        // (i.e., don't exceed queue size). Servants should be
        // saved immediately.
        //
        vector<Test::ServantPrx> servants;
        for(i = 0; i < size; i++)
        {
            servants.push_back(evictor->createServant(i));
            test(evictor->getLastSavedValue() == i);
        }

        //
        // Evict and verify values.
        //
        evictor->setSize(0);
        evictor->setSize(size);
        evictor->clearLastSavedValue();
        for(i = 0; i < size; i++)
        {
            test(servants[i]->getValue() == i);
        }

        //
        // Mutate servants.
        //
        for(i = 0; i < size; i++)
        {
            servants[i]->setValue(i + 100);
        }

        //
        // Servants should not be saved yet.
        //
        test(evictor->getLastSavedValue() == -1);
        for(i = 0; i < size; i++)
        {
            test(servants[i]->getValue() == i + 100);
        }

        //
        // Evict and verify values.
        //
        evictor->setSize(0);
        evictor->setSize(size);
        for(i = 0; i < size; i++)
        {
            test(servants[i]->getValue() == i + 100);
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
            servants.push_back(evictor->createServant(i));
        }

        //
        // Deactivate and recreate evictor, to ensure that servants
        // are restored properly after database close and reopen.
        //
        evictor->deactivate();
        evictor = factory->createEvictor("EvictionStrategy", Test::Eviction);
        evictor->setSize(size);
        for(i = 0; i < size; i++)
        {
            test(servants[i]->getValue() == i);
        }

        //
        // No servants should have been evicted yet.
        //
        test(evictor->getLastEvictedValue() == -1);

        //
        // Create new servant - should cause eviction.
        //
        servants.push_back(evictor->createServant(size));
        test(evictor->getLastEvictedValue() == 0);

        //
        // Restore the evicted servant, which evicts another
        // servant, and so on.
        //
        for(i = 0; i <= size; i++)
        {
            test(servants[i]->getValue() == i);
            test(evictor->getLastEvictedValue() == (i + 1) % (size + 1));
        }

        //
        // Destroy new servant and verify eviction no longer occurs.
        //
        servants[size]->destroy();
        evictor->clearLastEvictedValue();
        for(i = 0; i < size; i++)
        {
            test(servants[i]->getValue() == i);
        }
        test(evictor->getLastEvictedValue() == -1);


	// 
	// Test explicit saves
	//
	for(i = 0; i < size; i++)
        {
	    servants[i]->saveValue(i + 1);
	    test(evictor->getLastSavedValue() == i + 1);
        }

        //
        // Clean up.
        //
        for(i = 0; i < size; i++)
        {
            servants[i]->destroy();
        }

        cout << "ok" << endl;
    }

    //
    // Test IdleStrategy
    //
    {
        cout << "testing IdleStrategy... " << flush;

        const Ice::Int size = 5;
        Ice::Int i;

        Test::RemoteEvictorPrx evictor = factory->createEvictor("IdleStrategy", Test::Idle);
        evictor->setSize(size);

        //
        // Create the same number of servants as the evictor size
        // (i.e., don't exceed queue size). Servants should be
        // saved immediately.
        //
        vector<Test::ServantPrx> servants;
        for(i = 0; i < size; i++)
        {
            servants.push_back(evictor->createServant(i));
            test(evictor->getLastSavedValue() == i);
        }

        //
        // Evict and verify values.
        //
        evictor->setSize(0);
        evictor->setSize(size);
        for(i = 0; i < size; i++)
        {
            test(servants[i]->getValue() == i);
        }

        //
        // Mutate servants and verify they have been saved.
        //
        for(i = 0; i < size; i++)
        {
            servants[i]->setValue(i + 100);
            test(evictor->getLastSavedValue() == i + 100);
        }

        //
        // Evict and verify values.
        //
        evictor->setSize(0);
        evictor->setSize(size);
        for(i = 0; i < size; i++)
        {
            test(servants[i]->getValue() == i + 100);
        }

        //
        // No servants should have been saved yet.
        //
        test(evictor->getLastSavedValue() == -1);

        //
        // Test idle behavior.
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
            //
            // Object should not have been modified or saved yet.
            //
            test(servants[i]->getValue() == i + 100);
            test(evictor->getLastSavedValue() == -1);
            //
            // This operation modifies the object state but is not saved
            // because the setValueAsync operation is still pending.
            //
            servants[i]->setValue(i + 200);
            test(servants[i]->getValue() == i + 200);
            test(evictor->getLastSavedValue() == -1);
            //
            // Force the response to setValueAsync, which should cause
            // the object to be saved.
            //
            servants[i]->releaseAsync();
            test(servants[i]->getValue() == i + 300);
            test(evictor->getLastSavedValue() == i + 300);
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
            servants.push_back(evictor->createServant(i));
        }

        //
        // Deactivate and recreate evictor, to ensure that servants
        // are restored properly after database close and reopen.
        //
        evictor->deactivate();
        evictor = factory->createEvictor("IdleStrategy", Test::Idle);
        evictor->setSize(size);
        for(i = 0; i < size; i++)
        {
            test(servants[i]->getValue() == i);
        }

        //
        // No servants should have been saved yet.
        //
        test(evictor->getLastSavedValue() == -1);

        //
        // Create new servant - should cause eviction but no
        // servants should be saved.
        //
        servants.push_back(evictor->createServant(size));
        test(evictor->getLastSavedValue() == size);
        test(evictor->getLastEvictedValue() != -1);

        //
        // Restore the evicted servant, which evicts another
        // servant, and so on.
        //
        for(i = 0; i <= size; i++)
        {
            test(servants[i]->getValue() == i);
            test(evictor->getLastEvictedValue() == (i + 1) % (size + 1));
        }
        test(evictor->getLastSavedValue() == -1);

        //
        // Clean up.
        //
        for(i = 0; i <= size; i++)
        {
            servants[i]->destroy();
        }

        cout << "ok" << endl;
    }

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
