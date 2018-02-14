// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <IceUtil/Random.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace IceUtil;

class ReadThread : public Thread
{
public:

    ReadThread(vector<Test::ServantPrx>& servants) :
        _servants(servants)
    {
    }
    
    virtual void
    run()
    {
        int loops = 10;
        while(loops-- > 0)
        {
            try
            {
                _servants[0]->getValue();
                test(false);
            }
            catch(const Ice::ObjectNotExistException&)
            {
                // Expected
            }
            catch(...)
            {
                test(false);
            }
            
            for(int i = 1; i < static_cast<int>(_servants.size()); ++i)
            {
                test(_servants[i]->getValue() == i);
            }
        }
    }

private:
    vector<Test::ServantPrx>& _servants;
};

class ReadForeverThread : public Thread, public IceUtil::Monitor<IceUtil::RecMutex>
{
public:

    enum State { StateRunning, StateDeactivating, StateDeactivated };

    ReadForeverThread(vector<Test::ServantPrx>& servants) :
        _servants(servants),
        _state(StateRunning)
    {
    }
    
    virtual void
    run()
    {
        for(;;)
        {
            try
            {
                for(int i = 0; i < static_cast<int>(_servants.size()); ++i)
                {
                    if(getState() == StateDeactivated)
                    {
                        _servants[i]->slowGetValue();
                        test(false);
                    }
                    else
                    {
                        test(_servants[i]->slowGetValue() == i);
                    }
                }
            }
            catch(const Ice::SocketException&)
            {
                //
                // Expected
                //
                test(validEx());
                return;
            }
            catch(const Ice::LocalException& e)
            {
                cerr << "Caught unexpected : " << e << endl;
                test(false);
                return;
            }
            catch(...)
            {
                test(false);
                return;
            }
        }
    }

    State
    getState()
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        return _state;
    }

    bool
    validEx()
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        return _state == StateDeactivating || _state == StateDeactivated;
    }

    void
    setState(State s)
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        _state = s;
    }

private:
    vector<Test::ServantPrx>& _servants;
    State _state;
};
typedef IceUtil::Handle<ReadForeverThread> ReadForeverThreadPtr;

class AddForeverThread : public Thread, public IceUtil::Monitor<IceUtil::RecMutex>
{
public:

    enum State { StateRunning, StateDeactivating, StateDeactivated };

    AddForeverThread(const Test::RemoteEvictorPrx& evictor, int prefix) :
        _evictor(evictor),
        _state(StateRunning)
    {
        ostringstream ostr;
        ostr << prefix;
        _prefix = ostr.str();
    }
    
    virtual void
    run()
    {
        int index = 0;

        for(;;)
        {
            ostringstream ostr;
            ostr << _prefix << "-" << index;
            index++;
            string id = ostr.str();
            try
            {
                if(getState() == StateDeactivated)
                {
                    _evictor->createServant(id, 0);
                    test(false);
                }
                else
                {
                    _evictor->createServant(id, 0);
                }
            }
            catch(const Test::EvictorDeactivatedException&)
            {
                test(validEx());
                //
                // Expected
                //
                return;
            }
            catch(const Ice::ObjectNotExistException&)
            {
                test(validEx());
                //
                // Expected
                //
                return;
            }
            catch(const Ice::LocalException& e)
            {
                cerr << "Caught unexpected : " << e << endl;
                test(false);
                return;
            }
            catch(const IceUtil::Exception& e)
            {
                cerr << "Caught IceUtil::Exception : " << e << endl;
                cerr << "Index is " << index << endl;
                test(false);
                return;
            }
            catch(const std::exception& e)
            {
                cerr << "Caught std::exception : " << e.what() << endl;
                test(false);
                return;
            }
            catch(...)
            {
                cerr << "Caught unknown exception" << endl;
                test(false);
                return;
            }
        }
    }

    State
    getState()
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        return _state;
    }
    bool
    validEx()
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        return _state == StateDeactivating || _state == StateDeactivated;
    }

    void
    setState(State s)
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        _state = s;
    }

private:
    Test::RemoteEvictorPrx _evictor;
    string _prefix;
    State _state;
};
typedef IceUtil::Handle<AddForeverThread> AddForeverThreadPtr;

class CreateDestroyThread : public Thread
{
public:

    CreateDestroyThread(const Test::RemoteEvictorPrx& evictor, int id, int size) :
        _evictor(evictor),
        _size(size)
    {
        ostringstream ostr;
        ostr << id;
        _id = ostr.str();
    }
    
    virtual void
    run()
    {
        try
        {
            int loops = 50;
            while(loops-- > 0)
            {
                for(int i = 0; i < _size; i++)
                {
                    ostringstream ostr;
                    ostr << i;
                    string id = ostr.str();
                    if(id == _id)
                    {
                        //
                        // Create when odd, destroy when even.
                        //
                        
                        if(loops % 2 == 0)
                        {
                            Test::ServantPrx servant = _evictor->getServant(id);
                            servant->destroy();
                            
                            //
                            // Twice
                            //
                            try
                            {
                                servant->destroy();
                                test(false);
                            }
                            catch(const Ice::ObjectNotExistException&)
                            {
                                // Expected
                            }
                        }
                        else
                        {
                            Test::ServantPrx servant = _evictor->createServant(id, i);
                            
                            //
                            // Twice
                            //
                            try
                            {
                                servant = _evictor->createServant(id, 0);
                                test(false);
                            }
                            catch(const Test::AlreadyRegisteredException&)
                            {
                                // Expected
                            }
                        }
                    }
                    else
                    {
                        //
                        // Just read/write the value
                        //
                        Test::ServantPrx servant = _evictor->getServant(id);
                        try
                        {
                            int val = servant->getValue();
                            test(val == i || val == -i);
                            servant->setValue(-val);
                        }
                        catch(const Ice::ObjectNotExistException&)
                        {
                            // Expected from time to time
                        }
                    }
                }
            }
        }
        catch(const std::exception& ex)
        {
            cout << "Caught unexpected : " << ex.what() << endl;
            test(false);
        }
        catch(...)
        {
            //
            // Unexpected!
            //
            test(false);
        }
    }
private:
    Test::RemoteEvictorPrx _evictor;
    string _id;
    int _size;
};


class TransferThread : public Thread
{
public:
    TransferThread(const Test::AccountPrxSeq& accounts) :
        _accounts(accounts)
    {
    }
    
    void
    run()
    {
        int transferOp = 0;
        IceUtil::Time now = IceUtil::Time::now();
        for(int i = 0; i < 500; i++)
        {
            //
            // Limit the time this thread runs. We don't want the test
            // to take too long on slow machines/VMs
            //
            if(IceUtil::Time::now() - now > IceUtil::Time::seconds(60))
            {
                cerr << "warning: exiting transfer thread after one minute and " << i << " iterations" << endl; 
                break;
            }

            //
            // Transfer 100 at random between two distinct accounts 
            //
            Test::AccountPrx from = _accounts[IceUtilInternal::random(static_cast<int>(_accounts.size()))];
            
            Test::AccountPrx to;
            do
            {
                to = _accounts[IceUtilInternal::random(static_cast<int>(_accounts.size()))];
            }
            while(from == to);
                

            try
            {
                //
                // Alternate between transfer methods
                //
                switch(transferOp)
                {
                    case 0:
                    {
                        from->transfer(100, to);
                        break;
                    }
                    case 1:
                    {
                        from->transfer2(100, to);
                        break;
                    }
                    case 2:
                    {
                        from->transfer3(100, to);
                        break;
                    }
                    default:
                    {
                        test(false);
                    }
                };
                transferOp++;
                transferOp = transferOp % 3;
            }
            catch(const Test::InsufficientFundsException&)
            {
                //
                // Expected from time to time
                //
            }
            catch(const Ice::Exception& e)
            {
                cerr << "Caught Ice::Exception: " << e << endl;
                test(false);
            }
            catch(...)
            {
                cerr << "caught some other exception" << endl;
                //
                // Unexpected
                //
                test(false);
            }
            
            /*
              if(i % 100 == 0)
              {
              cerr << "." << flush;
              }
            */
        }
    }

private:

    Test::AccountPrxSeq _accounts;
};
    


int
run(int, char**, const Ice::CommunicatorPtr& communicator, bool transactional, bool shutdown)
{
    string ref = "factory:default -p 12010";
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    Test::RemoteEvictorFactoryPrx factory = Test::RemoteEvictorFactoryPrx::checkedCast(base);

    if(transactional)
    {
        cout << "testing transactional Freeze Evictor... " << flush;
    }
    else
    {
        cout << "testing background-save Freeze Evictor... " << flush;
    }
    
    const Ice::Int size = 5;
    Ice::Int i;
    
    Test::RemoteEvictorPrx evictor = factory->createEvictor("Test", transactional);
    
    evictor->setSize(size);
    
    //
    // Create some servants 
    //
    vector<Test::ServantPrx> servants;
    for(i = 0; i < size; i++)
    {
        ostringstream ostr;
        ostr << i;
        string id = ostr.str();
        servants.push_back(evictor->createServant(id, i));
        servants[i]->ice_ping();
        
        Test::FacetPrx facet1 = Test::FacetPrx::uncheckedCast(servants[i], "facet1");
        try
        {
            facet1->ice_ping();
            test(false);
        }
        catch(const Ice::FacetNotExistException&)
        {
            // Expected
        }

        servants[i]->addFacet("facet1", "data");
        facet1->ice_ping();
        facet1 = Test::FacetPrx::checkedCast(servants[i], "facet1");
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

    if(!transactional)
    {
        // 
        // Test saving while busy
        //
        for(i = 0; i < size; i++)
        {
            //
            // Start a mutating operation so that the object is not idle.
            //
            servants[i]->begin_setValueAsync(i + 300);
            
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
    // Call an operation that does not exist on the servant
    //    
    try
    {
        Test::AccountPrx::uncheckedCast(servants[0])->getBalance();
        test(false);
    }
    catch(const Ice::OperationNotExistException&)
    {
    }
  
    //
    // Remove all facets
    //
    for(i = 0; i < size; i++)
    {
        servants[i]->removeFacet("facet1");
        servants[i]->removeFacet("facet2");
    }

    //
    // Check they are all gone
    //
    for(i = 0; i < size; i++)
    {
        Test::FacetPrx facet1 = Test::FacetPrx::checkedCast(servants[i], "facet1");
        test(facet1 == 0);
        Test::FacetPrx facet2 = Test::FacetPrx::checkedCast(servants[i], "facet2");
        test(facet2 == 0);
    }

    evictor->setSize(0);
    evictor->setSize(size);

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

        try
        {
            servants[i]->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
            // Expected
        }
    }
          
    //
    // Recreate servants, set transient value
    //  
    servants.clear();
    for(i = 0; i < size; i++)
    {
        ostringstream ostr;
        ostr << i;
        string id = ostr.str();
        servants.push_back(evictor->createServant(id, i));
        servants[i]->setTransientValue(i);
    }
    
    //
    // Evict all
    //
    evictor->saveNow();
    evictor->setSize(0);
    evictor->setSize(size);
    
    //
    // Check the transient value
    //
    for(i = 0; i < size; i++)
    {
        test(servants[i]->getTransientValue() == -1);
    }

    if(!transactional)
    {
    
        //
        // Now with keep
        //
        for(i = 0; i < size; i++)
        {
            servants[i]->keepInCache();
            servants[i]->keepInCache();
            servants[i]->setTransientValue(i);
        }
        evictor->saveNow();
        evictor->setSize(0);
        evictor->setSize(size);
   
    
        //
        // Check the transient value
        //
        for(i = 0; i < size; i++)
        {
            test(servants[i]->getTransientValue() == i);
        }

        //
        // Again, after one release
        //
        for(i = 0; i < size; i++)
        {
            servants[i]->release();
        }
        evictor->saveNow();
        evictor->setSize(0);
        evictor->setSize(size);
        for(i = 0; i < size; i++)
        {
            test(servants[i]->getTransientValue() == i);
        }

        //
        // Again, after a second release
        //
        for(i = 0; i < size; i++)
        {
            servants[i]->release();
        }
        evictor->saveNow();
        evictor->setSize(0);
        evictor->setSize(size);
        for(i = 0; i < size; i++)
        {
            test(servants[i]->getTransientValue() == -1);
        }


        //
        // Release one more time
        //
        for(i = 0; i < size; i++)
        {
            try
            {
                servants[i]->release();
                test(false);
            }
            catch(const Test::NotRegisteredException&)
            {
                // Expected
            }
        }
    }

    if(transactional)
    {
        int totalBalance = servants[0]->getTotalBalance();
        test(totalBalance == 0);
        
        Test::AccountPrxSeq accounts = servants[0]->getAccounts();
        test(accounts.size() > 0);

        totalBalance = servants[0]->getTotalBalance();
        test(totalBalance > 0);

        const int threadCount = static_cast<int>(accounts.size());
        
        vector<ThreadPtr> threads(threadCount);
        for(i = 0; i < threadCount; i++)
        {
            threads[i] = new TransferThread(accounts);
            threads[i]->start();
        }

        for(i = 0; i < threadCount; i++)
        {
            threads[i]->getThreadControl().join();
        }
       
        //
        // Check that the total balance did not change!
        //
        test(totalBalance == servants[0]->getTotalBalance());
    }
        
    //
    // Deactivate and recreate evictor, to ensure that servants
    // are restored properly after database close and reopen.
    //
    evictor->deactivate();
    
    evictor = factory->createEvictor("Test", transactional);

    evictor->setSize(size);
    for(i = 0; i < size; i++)
    {
        ostringstream ostr;
        ostr << i;
        string id = ostr.str();

        servants[i] = evictor->getServant(id);
        test(servants[i]->getValue() == i);
    }

    //
    // Test concurrent lookups with a smaller evictor
    // size and one missing servant
    //
    evictor->setSize(size / 2);
    servants[0]->destroy();

    {
        const int threadCount = size * 2;
        
        ThreadPtr threads[threadCount];
        for(i = 0; i < threadCount; i++)
        {
            threads[i] = new ReadThread(servants);
            threads[i]->start();
        }
        
        for(i = 0; i < threadCount; i++)
        {
            threads[i]->getThreadControl().join();
        }
    }
    
    //
    // Clean up.
    //
    evictor->destroyAllServants("");
    evictor->destroyAllServants("facet1");
    evictor->destroyAllServants("facet2");

    //
    // CreateDestroy threads
    //
    {
        const int threadCount = size;;
        
        ThreadPtr threads[threadCount];
        for(i = 0; i < threadCount; i++)
        {
            threads[i] = new CreateDestroyThread(evictor, i, size);
            threads[i]->start();
        }
        
        for(i = 0; i < threadCount; i++)
        {
            threads[i]->getThreadControl().join();
        }

        //
        // Verify all destroyed
        // 
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
    }

    //
    // Recreate servants.
    //  
    servants.clear();
    for(i = 0; i < size; i++)
    {
        ostringstream ostr;
        ostr << i;
        string id = ostr.str();
        servants.push_back(evictor->createServant(id, i));
    }

    //
    // Deactivate in the middle of remote AMD operations
    // (really testing Ice here)
    //
    {
        const int threadCount = size;
        
        ReadForeverThreadPtr threads[threadCount];
        for(i = 0; i < threadCount; i++)
        {
            threads[i] = new ReadForeverThread(servants);
            threads[i]->start();
        }

        ThreadControl::sleep(Time::milliSeconds(500));
        for(i = 0; i < threadCount; i++)
        {
            threads[i]->setState(ReadForeverThread::StateDeactivating);
        }
        evictor->deactivate();
        for(i = 0; i < threadCount; i++)
        {
            threads[i]->setState(ReadForeverThread::StateDeactivated);
        }

        for(i = 0; i < threadCount; i++)
        {
            threads[i]->getThreadControl().join();
        }
    }

    //
    // Resurrect
    //
    evictor = factory->createEvictor("Test", transactional);
    evictor->destroyAllServants("");

    //
    // Deactivate in the middle of adds
    //
    {
        const int threadCount = size;
        
        AddForeverThreadPtr threads[threadCount];
        for(i = 0; i < threadCount; i++)
        {
            threads[i] = new AddForeverThread(evictor, i);
            threads[i]->start();
        }

        ThreadControl::sleep(Time::milliSeconds(500));
        for(i = 0; i < threadCount; i++)
        {
            threads[i]->setState(AddForeverThread::StateDeactivating);
        }
        evictor->deactivate();
        for(i = 0; i < threadCount; i++)
        {
            threads[i]->setState(AddForeverThread::StateDeactivated);
        }
        
        for(i = 0; i < threadCount; i++)
        {
            threads[i]->getThreadControl().join();
        }
    }
    
    
    //
    // Clean up.
    //
    evictor = factory->createEvictor("Test", transactional);
    evictor->destroyAllServants("");
    evictor->deactivate();

    cout << "ok" << endl;

    if(shutdown)
    {
        factory->shutdown();
    }
    
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
        status = run(argc, argv, communicator, false, false);
        if(status == 0)
        {
            status = run(argc, argv, communicator, true, true);
        }
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
