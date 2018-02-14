// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include <TestCommon.h>
#include <ByteIntMap.h>
#include <IntIdentityMap.h>
#include <IntIdentityMapWithIndex.h>
#include <SortedMap.h>
#include <WstringWstringMap.h>
#include <Freeze/TransactionHolder.h>

#include <algorithm>

using namespace std;
using namespace Ice;
using namespace Freeze;
using namespace Test;


// #define SHOW_EXCEPTIONS 1
#define CONCURRENT_TIME 3


#ifdef __SUNPRO_CC
extern
#else
static 
#endif
Byte alphabetChars[] = "abcdefghijklmnopqrstuvwxyz";

vector<Byte> alphabet;

// The extern in the following function is due to a Sun C++ 5.4 template bug
//
extern void
ForEachTest(const pair<const Byte, const Int>&)
{
}

extern bool
FindIfTest(const pair<const Byte, const Int>& p)
{
    return p.first == 'b';
}

extern bool
FindFirstOfTest(const pair<const Byte, const Int>& p, Byte q)
{
    return p.first == q;
}


void
populateDB(const Freeze::ConnectionPtr& connection, ByteIntMap& m)
{
    vector<Byte> localAlphabet;
    localAlphabet.assign(alphabetChars, alphabetChars + sizeof(alphabetChars) - 1);
    size_t length = localAlphabet.size();
    
    for(;;)
    {
        try
        {
            TransactionHolder txHolder(connection);
            for(size_t j = 0; j < length; ++j)
            {
                m.put(ByteIntMap::value_type(localAlphabet[j], static_cast<Int>(j)));
            }
            txHolder.commit();
            break;
        }
        catch(const DeadlockException&)
        {
#ifdef SHOW_EXCEPTIONS
            cerr << "t" << flush;
#endif

            length = length / 2;
            //
            // Try again
            //
        }
    }
}

class ReadThread : public IceUtil::Thread
{
public:

    ReadThread(const CommunicatorPtr& communicator, const string& envName, const string& dbName) :
        _connection(createConnection(communicator, envName)),
        _map(_connection, dbName),
        _done(false)
    {
    }

    virtual void
    run()
    {
        bool more = false;

        do
        {
            for(;;)
            {
                try
                {
                    for(ByteIntMap::iterator p = _map.begin(); p != _map.end(); ++p)
                    {
                        test((p->first == p->second + 'a') || (p->first == p->second + 'A'));
                        IceUtil::ThreadControl::yield();
                    }
                    break; // for(;;)
                }
                catch(const DeadlockException&)
                {
#ifdef SHOW_EXCEPTIONS
                    cerr << "r" << flush;
#endif
                    //
                    // Try again
                    //
                }
                catch(const InvalidPositionException&)
                {
#ifdef SHOW_EXCEPTIONS
                    cerr << "i" << flush;
#endif
                    break;
                }
            }

            {
                IceUtil::Mutex::Lock lock(_doneMutex);
                more = !_done;
            }

        } while(more);
    }

    void stop()
    {
        IceUtil::Mutex::Lock lock(_doneMutex);
        _done = true;
    }

private:

    Freeze::ConnectionPtr _connection;
    ByteIntMap  _map;
    bool _done;
    IceUtil::Mutex _doneMutex;
};

typedef IceUtil::Handle<ReadThread> ReadThreadPtr;


class WriteThread : public IceUtil::Thread
{
public:

    WriteThread(const CommunicatorPtr& communicator, const string& envName, const string& dbName) :
        _connection(createConnection(communicator, envName)),
        _map(_connection, dbName),
        _done(false)
    {
    }

    virtual void
    run()
    {
        bool more = false;

        //
        // Delete an recreate each object
        //
        do
        {
            for(;;)
            {
                try
                {
                    TransactionHolder txHolder(_connection);
                    for(ByteIntMap::iterator p = _map.begin(); p != _map.end(); ++p)
                    {
                        p.set(p->second + 1);
                        _map.erase(p);
                    }
                    txHolder.commit();
                    break; // for (;;)
                }
                catch(const DeadlockException&)
                {
#ifdef SHOW_EXCEPTIONS
                    cerr << "w" << flush;
#endif
                    //
                    // Try again
                    //
                }
                catch(const InvalidPositionException&)
                {
#ifdef SHOW_EXCEPTIONS
                    cerr << "I" << flush;
#endif
                    break;
                }
            }
            populateDB(_connection, _map);

            
            //
            // Now update without a transaction
            //
               
            for(char c = 'a'; c != 'd'; ++c)
            {
                for(;;)
                {
                    bool thrownBySet = false;
                    try
                    {
                        ByteIntMap::iterator p = _map.find(c);
                        try
                        {
                            if(p != _map.end())
                            {
                                if(p->first == p->second + 'a')
                                {
                                    p.set(p->first - 'A');
                                }
                                else
                                {
                                    p.set(p->first - 'a');
                                }   
                            }
                        }
                        catch(const DeadlockException&)
                        {
                            thrownBySet = true;
                            throw;
                        }
                        break; // for (;;)
                    }    
                    catch(const DeadlockException&)
                    {
                        if(!thrownBySet)
                        {
                            cerr << "DeadlockException thrown by destructor!" << endl;
                            test(false);
                        }
                        
#ifdef SHOW_EXCEPTIONS
                        if(thrownBySet)
                        {
                            cerr << "S" << flush;
                        }
                        else
                        {
                            cerr << "D" << flush;
                        }
#endif
                        // Try again
                    }
                    catch(const InvalidPositionException&)
                    {
#ifdef SHOW_EXCEPTIONS
                        cerr << "I" << flush;
#endif
                        break;
                    }
                }
            }

            {
                IceUtil::Mutex::Lock lock(_doneMutex);
                more = !_done;
            }
        } while(more);
    }

    void stop()
    {
        IceUtil::Mutex::Lock lock(_doneMutex);
        _done = true;
    }

private:

    Freeze::ConnectionPtr _connection;
    ByteIntMap  _map;
    bool _done;
    IceUtil::Mutex _doneMutex;
};

typedef IceUtil::Handle<WriteThread> WriteThreadPtr;


int
run(const CommunicatorPtr& communicator, const string& envName)
{
    Freeze::ConnectionPtr connection = createConnection(communicator, envName);
    const string dbName = "binary";

    //
    // Open/close db within transaction
    //
    {
        TransactionHolder txHolder(connection);
        ByteIntMap m1(connection, dbName);

        m1.put((ByteIntMap::value_type('a', 1)));
        // rollback in dtor of txHolder
    }
 
    ByteIntMap m1(connection, dbName);
    
    //
    // Populate the database with the alphabet
    //
    populateDB(connection, m1);
    alphabet.assign(alphabetChars, alphabetChars + sizeof(alphabetChars) - 1);

    //
    // Test ==, swap and communicator()
    //
    ByteIntMap m(connection, dbName + "-tmp");
    test(!(m == m1));
    test(m != m1);
    m.swap(m1);
    test(!(m == m1));
    test(m != m1);
    test(m1.size() == 0);
    test(m.communicator() == (m1.communicator() == communicator));


    vector<Byte>::const_iterator j;
    ByteIntMap::iterator p;
    ByteIntMap::const_iterator cp;

    cout << "testing populate... " << flush;
    //
    // First try non-const iterator
    //
    for(j = alphabet.begin(); j != alphabet.end(); ++j)
    {
        p = m.find(*j);
        test(p != m.end());
        test(p->first == *j && p->second == j - alphabet.begin());
    }
    //
    // Next try const iterator
    //
    for(j = alphabet.begin(); j != alphabet.end(); ++j)
    {
        cp = m.find(*j);
        test(cp != m.end());
        test(cp->first == *j && cp->second == j - alphabet.begin());
    }
   
    test(!m.empty());
    test(m.size() == alphabet.size());
    cout << "ok" << endl;

    cout << "testing map::find... " << flush;
    j = find(alphabet.begin(), alphabet.end(), 'n');
    
    cp = m.find(*j);
    test(cp != m.end());
    test(cp->first == 'n' && cp->second == j - alphabet.begin());
    cout << "ok" << endl;

    cout << "testing erase... " << flush;

    //
    // erase first offset characters (first offset characters is
    // important for later verification of the correct second value in
    // the map).
    //
    int offset = 3;
    vector<Byte> bytes;
    bytes.push_back('a');
    bytes.push_back('b');
    bytes.push_back('c');
    for(j = bytes.begin(); j != bytes.end(); ++j)
    {
        p = m.find(*j);
        test(p != m.end());
        m.erase(p);
        
        //
        // Release locks to avoid self deadlock
        //
        p = m.end();
        
        p = m.find(*j);
        test(p == m.end());
        vector<Byte>::iterator r = find(alphabet.begin(), alphabet.end(), *j);
        test(r != alphabet.end());
        alphabet.erase(r);
    }

    for(j = alphabet.begin(); j != alphabet.end(); ++j)
    {
        cp = m.find(*j);
        test(cp != m.end());
        test(cp->first == *j && cp->second == (j - alphabet.begin()) + offset);
    }

    cout << "ok" << endl;

    //
    // Get an iterator for the deleted element - this should fail.
    //
    cout << "testing map::find (again)... " << flush;
    cp = m.find('a');
    test(cp == m.end());
    cout << "ok" << endl;

    cout << "testing iterators... " << flush;
    p = m.begin();
    ByteIntMap::iterator p2 = p;

    //
    // Verify both iterators point at the same element, and that
    // element is in the map.
    //
    test(p == p2);
    test(p->first == p2->first && p->second == p2->second);
    test(find(alphabet.begin(), alphabet.end(), p->first) != alphabet.end());

    //
    // Create iterator that points at 'n'
    //
    p = m.find('n');
    p2 = p;

    //
    // Verify both iterators point at 'n'
    //
    test(p == p2);
    test(p->first == 'n' && p->second == 13);
    test(p2->first == 'n' && p2->second == 13);

    //
    // Create cursor that points at 'n'
    //
    p = m.find('n');
    test(p->first == 'n' && p->second == 13);
    ++p;

    p2 = p;

    //
    // Verify cloned cursors are independent
    //
    test(p->first != 'n' && p->second != 13);
    pair<const Byte, const Int> data = *p;
    ++p;

    test(p->first != data.first && p->second != data.second);
    ++p;

    test(p2->first == data.first && p2->second == data.second);
  
    p = m.find('n');
    p2 = ++p;
    test(p2->first == p->first);

    char c = p2->first;
    p2 = p++;
    test(c == p2->first); // p2 should still be the same
    test(p2->first != p->first && (++p2)->first == p->first);
    
    cout << "ok" << endl;

    //
    // Test writing into an iterator.
    //
    cout << "testing iterator.set... " << flush;

    p = m.find('d');
    test(p != m.end() && p->second == 3);

    test(m.find('a') == m.end());
    m.put(ByteIntMap::value_type('a', 1));

    p = m.find('a');
    test(p != m.end() && p->second == 1);

    
    m.put(ByteIntMap::value_type('a', 0));
    
    p = m.find('a');
    test(p != m.end() && p->second == 0);
    //
    // Test inserts
    // 
    
    ByteIntMap::value_type i3('a', 7);
    pair<ByteIntMap::iterator, bool> insertResult = m.insert(i3);

    test(insertResult.first == m.find('a'));
    test(insertResult.first->second == 0);
    test(insertResult.second == false);
    insertResult.first = m.end();
    
    p = m.insert(m.end(), i3);
    test(p == m.find('a'));
    test(p->second == 0);

    ByteIntMap::value_type i4('b', 7);
    
    insertResult = m.insert(i4);
    test(insertResult.first == m.find('b'));
    test(insertResult.first->second == 7);
    test(insertResult.second == true);
    insertResult.first = m.end();
    
    ByteIntMap::value_type i5('c', 8);
    
    p = m.insert(m.end(), i5);
    test(p == m.find('c'));
    test(p->second == 8);

    p = m.find('a');
    test(p != m.end() && p->second == 0);
    p.set(1);
    test(p != m.end() && p->second == 1);

    //
    // This is necessary to release the locks held
    // by p and avoid a self-deadlock
    //
    p = m.end();
    
    p = m.find('a');
    test(p != m.end() && p->second == 1);
    cout << "ok" << endl;
    
    //
    // Re-populate
    //
    populateDB(connection, m);
    alphabet.assign(alphabetChars, alphabetChars + sizeof(alphabetChars) - 1);

    cout << "testing algorithms... " << flush;

    for_each(m.begin(), m.end(), ForEachTest);

    //
    // Inefficient, but this is just a test. Ensure that both forms of
    // operator== & != are tested.
    //
    ByteIntMap::value_type toFind('n', 13);
    
    p = find(m.begin(), m.end(), toFind);
    test(p != m.end());
    test(*p == toFind);
    test(toFind == *p);
    test(!(*p != toFind));
    test(!(toFind != *p));

    p = find_if(m.begin(), m.end(), FindIfTest);
    test(p->first == 'b');

    //
    // find_first_of. First construct a map with keys n, o, p,
    // q. The test must find one of the types (it doesn't matter
    // which since the container doesn't have to maintain sorted
    // order).
    //
    j = find(alphabet.begin(), alphabet.end(), 'n');
    map<Byte, const Int> pairs;
    pairs.insert(pair<const Byte, const Int>(*j, static_cast<Int>(j - alphabet.begin())));
    ++j;
    pairs.insert(pair<const Byte, const Int>(*j, static_cast<Int>(j - alphabet.begin())));
    ++j;
    pairs.insert(pair<const Byte, const Int>(*j, static_cast<Int>(j - alphabet.begin())));
    ++j;
    pairs.insert(pair<const Byte, const Int>(*j, static_cast<Int>(j - alphabet.begin())));

    p = find_first_of(m.begin(), m.end(), pairs.begin(), pairs.end());
    test(p != m.end());
    test(p->first == 'n' || p->first == 'o' || p->first == 'p' || p->first == 'q');

    j = find(alphabet.begin(), alphabet.end(), 'n');
    p = find_first_of(m.begin(), m.end(), j, j + 4, FindFirstOfTest);
    test(p != m.end());
    test(p->first == 'n' || p->first == 'o' || p->first == 'p' || p->first == 'q');

    pairs.clear();
    for(p = m.begin(); p != m.end(); ++p)
    {
        pairs.insert(pair<const Byte, const Int>(p->first, p->second));
    }
    test(pairs.size() == m.size());

    map<Byte, const Int>::const_iterator pit;
    for(pit = pairs.begin(); pit != pairs.end(); ++pit)
    {
        p = m.find(pit->first);
        test(p != m.end());
    }
    cout << "ok" << endl;

    cout << "testing clear... " << flush;
    test(m.size() > 0);
    m.clear();
    test(m.size() == 0);
    cout << "ok" << endl;

    cout << "testing index ... " << flush;
    m.clear();
    populateDB(connection, m);

    //
    // Exact match
    //
    size_t length = alphabet.size();
    for(size_t k = 0; k < length; ++k)
    {
        p = m.findByValue(static_cast<Int>(k));
        test(p != m.end());
        test(p->first == alphabet[k]);
        test(++p == m.end());
    }

    //
    // 2 items at 17
    // 
    m.put(ByteIntMap::value_type(alphabet[21], static_cast<Int>(17)));

    p = m.findByValue(17);
    test(p != m.end());
    test(p->first == alphabet[17] || p->first == alphabet[21]);
    test(++p != m.end());
    test(p->first == alphabet[17] || p->first == alphabet[21]);
    test(++p == m.end());
    test(m.valueCount(17) == 2);

    p = m.findByValue(17);
    test(p != m.end());
    m.erase(p);
    test(++p != m.end());
    test(p->first == alphabet[17] || p->first == alphabet[21]);
    test(++p == m.end());
    test(m.valueCount(17) == 1);

    p = m.findByValue(17);
    test(p != m.end());
    test(p->first == alphabet[17] || p->first == alphabet[21]);
    
    try
    {
        p.set(18);
        test(false);
    }
    catch(const DatabaseException&)
    {
        // Expected
    }
    test(p->first == alphabet[17] || p->first == alphabet[21]);
    test(++p == m.end());
    test(m.valueCount(17) == 1);

    m.put(ByteIntMap::value_type(alphabet[21], static_cast<Int>(17)));

    //
    // Non-exact match
    //
    p = m.findByValue(21);
    test(p == m.end());

    test(m.valueCount(21) == 0);

    p = m.findByValue(21, false);
    test(p == m.end());

    p = m.findByValue(22, false);
    int previous = 21;
    int count = 0;
    while(p != m.end())
    {
        test(p->second > previous);
        previous = p->second;
        ++p;
        count++;
    }
    test(count == 4);
    cout << "ok " << endl;

    cout << "testing unreferenced connection+transaction... " << flush;
    {
        Freeze::ConnectionPtr c2 = createConnection(communicator, envName);
        ByteIntMap m2(c2, dbName);

        TransactionPtr tx = c2->beginTransaction();

        p = m2.findByValue(17);
        test(p != m2.end());

        m2.put(ByteIntMap::value_type(alphabet[21], static_cast<Int>(99)));

        p = m2.findByValue(17);
        test(p == m2.end());

        test(c2->currentTransaction() != 0);
        test(tx->getConnection() != 0);
    }
    //
    // Should roll back here
    //
    p = m.findByValue(17);
    test(p != m.end());
    cout << "ok " << endl;

    cout << "testing concurrent access... " << flush;
    m.clear();
    populateDB(connection, m);

    vector<IceUtil::ThreadControl> controls;
    vector<ReadThreadPtr> readThreads;
    vector<WriteThreadPtr> writeThreads;

    for(int i = 0; i < 5; ++i)
    {
        ReadThreadPtr rt = new ReadThread(communicator, envName, dbName);
        controls.push_back(rt->start());
        readThreads.push_back(rt);

        WriteThreadPtr wt = new WriteThread(communicator, envName, dbName);
        controls.push_back(wt->start());
        writeThreads.push_back(wt);
    }

    IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(CONCURRENT_TIME));

    for(vector<WriteThreadPtr>::iterator q = writeThreads.begin(); q != writeThreads.end(); ++q)
    {
        (*q)->stop();
    }

    for(vector<ReadThreadPtr>::iterator q = readThreads.begin(); q != readThreads.end(); ++q)
    {
        (*q)->stop();
    }

    for(vector<IceUtil::ThreadControl>::iterator q = controls.begin(); q != controls.end(); ++q)
    {
        q->join();
    }
    cout << "ok" << endl;


    cout << "testing index creation... " << flush;
    
    {
        IntIdentityMap iim(connection, "intIdentity");
        
        Ice::Identity odd;
        odd.name = "foo";
        odd.category = "odd";
        
        Ice::Identity even;
        even.name = "bar";
        even.category = "even";
            
        TransactionHolder txHolder(connection);
        for(int i = 0; i < 1000; i++)
        {
            if(i % 2 == 0)
            {
                iim.put(IntIdentityMap::value_type(i, even));
            }
            else
            {
                iim.put(IntIdentityMap::value_type(i, odd));
            }
        }
        txHolder.commit();
        iim.closeDb();
    }
    
    { 
        IntIdentityMapWithIndex iim(connection, "intIdentity");
        test(iim.categoryCount("even") == 500);
        test(iim.categoryCount("odd") == 500);

        {
            int count = 0;
            IntIdentityMapWithIndex::iterator p = iim.findByCategory("even");
            while(p != iim.end())
            {
                test(p->first % 2 == 0);
                ++p;
                ++count;
            }
            test(count == 500);
        }
         
        {   
            int count = 0;
            IntIdentityMapWithIndex::iterator p = iim.findByCategory("odd");
            while(p != iim.end())
            {
                test(p->first % 2 == 1);
                ++p;
                ++count;
            }
            test(count == 500);
        }
        iim.destroy();
    }
    cout << "ok" << endl;

    cout << "testing sorting... " << flush;
    { 
        SortedMap sm(connection, "sortedMap");
            
        TransactionHolder txHolder(connection);
        for(int i = 0; i < 1000; i++)
        {
            int k = rand() % 1000; 

            Ice::Identity id;
            id.name = "foo";
            id.category = 'a' + static_cast<char>(k % 26);

            sm.put(SortedMap::value_type(k, id));
        }
        txHolder.commit();
    }
    
    { 
        SortedMap sm(connection, "sortedMap");
        {
            for(int i = 0; i < 100; ++i)
            {
                int k = rand() % 1000;
                SortedMap::iterator p = sm.lower_bound(k);
                if(p != sm.end())
                {
                    test(p->first >= k);
                    SortedMap::iterator q = sm.upper_bound(k);
                    if(q == sm.end())
                    {
                        test(p->first == k);
                    }
                    else
                    {
                        test((p->first == k && q->first > k) || 
                             (p->first > k && q->first == p->first));
                    }
                }
            }
        }
         
        {
            for(int i = 0; i < 100; ++i)
            {
                string category;
                category = static_cast<char>('a' + rand() % 26);
               
                SortedMap::iterator p = sm.findByCategory(category);
                if(p != sm.end())
                {
                    SortedMap::iterator q = sm.lowerBoundForCategory(category);
                    test(p == q);
                    do
                    {
                        q++;
                    } while(q != sm.end() && q->second.category == category);
                    
                    if(q != sm.end())
                    {
                        test(q == sm.upperBoundForCategory(category));
                    }
                }
                else
                {
                    SortedMap::iterator q = sm.lowerBoundForCategory(category);
                    if(q != sm.end())
                    {
                        test(p != q);
                        test(q->second.category < category);
                        category = q->second.category;
                        
                        do
                        {
                            q++;
                        } while(q != sm.end() && q->second.category == category);
                        
                        if(q != sm.end())
                        {
                            test(q == sm.upperBoundForCategory(category));
                        }
                    }
                }
            }
        }

        {
            string category = "z";
            SortedMap::iterator p = sm.lowerBoundForCategory(category);
            
            while(p != sm.end())
            {
                test(p->second.category <= category);
                category = p->second.category;
                // cerr << category << ":" << p->first << endl;
                ++p;
            }
        }
        
        sm.clear();
    }

    cout << "ok" << endl;

    cout << "testing wstring... " << flush;

    { 
        WstringWstringMap wsm(connection, "wstringMap");
            
        TransactionHolder txHolder(connection);
        wsm.put(WstringWstringMap::value_type(L"AAAAA", L"aaaaa"));
        wsm.put(WstringWstringMap::value_type(L"BBBBB", L"bbbbb"));
        wsm.put(WstringWstringMap::value_type(L"CCCCC", L"ccccc"));
        wsm.put(WstringWstringMap::value_type(L"DDDDD", L"ddddd"));
        wsm.put(WstringWstringMap::value_type(L"EEEEE", L"eeeee"));
        txHolder.commit();
    }

    { 
        WstringWstringMap wsm(connection, "wstringMap");
        {
             WstringWstringMap::iterator p = wsm.find(L"BBBBB");
             test(p != wsm.end());
             test(p->second == L"bbbbb");
             
             p = wsm.findByValue(L"ddddd");
             test(p != wsm.end());
             test(p->first == L"DDDDD");
        }
        wsm.clear();
    }

    cout << "ok" << endl;

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;
 
    string envName = "db";

    try
    {
        communicator = Ice::initialize(argc, argv);
        if(argc != 1)
        {
            envName = argv[1];
            envName += "/";
            envName += "db";
        }
       
        status = run(communicator, envName);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }


    cout << "testing manual code... " << flush;

    //
    // From manual
    //
    
    Freeze::ConnectionPtr connection =
        Freeze::createConnection(communicator, envName);

    // Instantiate the map.
    //
    ByteIntMap map(connection, "simple");

    // Clear the map.
    //
    map.clear();

    Ice::Int i;
    ByteIntMap::iterator p;

    // Populate the map.
    //
    for (i = 0; i < 26; i++) 
    {
        Ice::Byte key = static_cast<Ice::Byte>('a' + i);
        map.insert(pair<const Byte, const int>(key, i));
    }

    // Iterate over the map and change the values.
    //
    for (p = map.begin(); p != map.end(); ++p)
    {
        p.set(p->second + 1);
    }

    // Find and erase the last element.
    //
    p = map.find(static_cast<Ice::Byte>('z'));
    assert(p != map.end());
    map.erase(p);

    // Clean up.
    //
    connection->close();

    cout << "ok" << endl;

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
