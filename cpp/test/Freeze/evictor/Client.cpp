// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    string ref = "factory:default -p 12345 -t 2000";
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    Test::RemoteEvictorFactoryPrx factory = Test::RemoteEvictorFactoryPrx::checkedCast(base);

    //
    // Test SaveUponEviction mode
    //
    {
        cout << "testing SaveUponEviction mode... " << flush;

        const Ice::Int size = 5;
        Ice::Int i;

        Test::RemoteEvictorPrx evictor = factory->createEvictor("saveUponEviction", Test::SaveUponEviction);
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
        evictor = factory->createEvictor("saveUponEviction", Test::SaveUponEviction);
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
        // Create new servant - should cause eviction.
        //
        servants.push_back(evictor->createServant(size));
        test(evictor->getLastSavedValue() == 0);

        //
        // Restore the evicted servant, which evicts another
        // servant, and so on.
        //
        for(i = 0; i <= size; i++)
        {
            test(servants[i]->getValue() == i);
            test(evictor->getLastSavedValue() == (i + 1) % (size + 1));
        }

        //
        // Destroy new servant and verify eviction no longer occurs.
        //
        servants[size]->destroy();
        evictor->clearLastSavedValue();
        for(i = 0; i < size; i++)
        {
            test(servants[i]->getValue() == i);
        }
        test(evictor->getLastSavedValue() == -1);

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
    // Test SaveAfterMutatingOperation mode
    //
    {
        cout << "testing SaveAfterMutatingOperation mode... " << flush;

        const Ice::Int size = 5;
        Ice::Int i;

        Test::RemoteEvictorPrx evictor =
            factory->createEvictor("saveAfterMutatingOperation", Test::SaveAfterMutatingOperation);
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
        evictor = factory->createEvictor("saveAfterMutatingOperation", Test::SaveAfterMutatingOperation);
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

        //
        // Restore the evicted servant, which evicts another
        // servant, and so on.
        //
        for(i = 0; i <= size; i++)
        {
            test(servants[i]->getValue() == i);
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
