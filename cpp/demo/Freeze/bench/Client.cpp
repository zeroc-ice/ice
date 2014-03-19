// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Freeze/Freeze.h>
#include <BenchTypes.h>
#include <cstdlib>

using namespace std;
using namespace Demo;

void
testFailed(const char* expr, const char* file, unsigned int line)
{
    std::cout << "failed!" << std::endl;
    std::cout << file << ':' << line << ": assertion `" << expr << "' failed" << std::endl;
    abort();
}

#define test(ex) ((ex) ? ((void)0) : testFailed(#ex, __FILE__, __LINE__))

class StopWatch
{
public:

    StopWatch()
    {
    }

    void
    start()
    {
        _stopped = false;
        _start = IceUtil::Time::now(IceUtil::Time::Monotonic);
    }

    IceUtil::Time
    stop()
    {
        if(!_stopped)
        {
            _stopped = true;
            _stop = IceUtil::Time::now(IceUtil::Time::Monotonic);
        }

        return _stop - _start;
    }

private:

    bool _stopped;
    IceUtil::Time _start;
    IceUtil::Time _stop;
};

class Generator : public IceUtil::Shared
{
public:

    virtual ~Generator() { }

    virtual int next() = 0;
    virtual string toString() = 0;
};
typedef IceUtil::Handle<Generator> GeneratorPtr;

class RandomGenerator : public Generator
{
public:

    RandomGenerator(int seed, int max) :
        _max(max)
    {
        srand(seed);
    }

    virtual int
    next()
    {
        return rand() % _max;
    }

    virtual string
    toString()
    {
        ostringstream os;
        os << "random(" << _max << ")";
        return os.str();
    }

private:

    const int _max;
};

class SequentialGenerator : public Generator
{
public:

    SequentialGenerator(int min, int max) :
        _min(min),
        _max(max),
        _current(0)
    {
    }

    virtual int
    next()
    {
        int n = _current;
        ++_current;
        if(_current > _max)
        {
            _current = _min;
        }
        return n;
    }

    virtual string
    toString()
    {
        ostringstream os;
        os << ((_max - _min)+1);
        return os.str();
    }

private:

    const int _min;
    const int _max;
    int _current;
};

class TestApp : public Ice::Application
{
public:
    
    TestApp(const string&);

    virtual int run(int, char*[]);

private:

    void IntIntMapIndexTest(IntIntMap&) 
    {}
    
    void IntIntMapIndexTest(IndexedIntIntMap&);
    template<class T> void IntIntMapTest(const string&, T* = 0);   
    
    void generatedReadWithIndex(IntIntMap&, int, const GeneratorPtr&)
    {}
    void generatedReadWithIndex(IndexedIntIntMap&, int, const GeneratorPtr&);
    template<class T> void generatedRead(T&, int, const GeneratorPtr&);   
    void Struct1Struct2MapIndexTest(Struct1Struct2Map&) 
    {}
    void Struct1Struct2MapIndexTest(IndexedStruct1Struct2Map&);
    template<class T> void Struct1Struct2MapTest(const string& mapName, T* = 0);
   
    void Struct1Class1MapIndexTest(Struct1Class1Map&) 
    {}
    void Struct1Class1MapIndexTest(IndexedStruct1Class1Map&);
    template<class T> void Struct1Class1MapTest(const string& mapName, T* = 0);
   
    void IntIntMapReadIndexTest(IntIntMap&)
    {}
    void IntIntMapReadIndexTest(IndexedIntIntMap&);
    template<class T> void IntIntMapReadTest(const string& mapName, T* = 0);
    
    void Struct1ObjectMapTest();

    const string _envName;
    Freeze::ConnectionPtr _connection;
    StopWatch _watch;
    int _repetitions;
};

TestApp::TestApp(const string& envName) :
    Ice::Application(Ice::NoSignalHandling),
    _envName(envName),
    _repetitions(10000)
{
}

void
TestApp::IntIntMapIndexTest(IndexedIntIntMap& m)
{
    //
    // Read each record.
    //
    _watch.start();
    for(int i = 0; i < _repetitions; ++i)
    {
        IndexedIntIntMap::iterator p = m.findByValue(i);
        test(p != m.end());
        test(p->second == i);
    }
    IceUtil::Time total = _watch.stop();
    IceUtil::Time perRecord = total / _repetitions;

    cout << "\ttime for " << _repetitions << " reverse (indexed) reads: " << total * 1000 << "ms"
         << endl;
    cout << "\ttime per reverse read: " << perRecord * 1000 << "ms" << endl;
}

template<class T> void 
TestApp::IntIntMapTest(const string& mapName, T*)
{
    T m(_connection, mapName);      
    //
    // Populate the database.
    //
    int i;
    _watch.start();
    {
        Freeze::TransactionHolder txHolder(_connection);
        for(i = 0; i < _repetitions; ++i)
        {
            m.put(typename T::value_type(i, i));
        }
        txHolder.commit();
    }
    IceUtil::Time total = _watch.stop();
    IceUtil::Time perRecord = total / _repetitions;

    cout << "\ttime for " << _repetitions << " writes: " << total * 1000 << "ms" << endl;
    cout << "\ttime per write: " << perRecord * 1000 << "ms" << endl;
    
    //
    // Read each record.
    //
    _watch.start();
    for(i = 0; i < _repetitions; ++i)
    {
        typename T::iterator p = m.find(i);
        test(p != m.end());
        test(p->second == i);
    }
    total = _watch.stop();
    perRecord = total / _repetitions;
    
    cout << "\ttime for " << _repetitions << " reads: " << total * 1000  << "ms" << endl;
    cout << "\ttime per read: " << perRecord * 1000 << "ms" << endl;
    
    //
    // Optional index sub-test
    //
    IntIntMapIndexTest(m);
    
    //
    // Remove each record.
    //
    _watch.start();
    {
        Freeze::TransactionHolder txHolder(_connection);
        for(i = 0; i < _repetitions; ++i)
        {
            m.erase(i);
        }
        txHolder.commit();
    }
    total = _watch.stop();
    perRecord = total / _repetitions;
    
    cout << "\ttime for " << _repetitions << " removes: " << total * 1000 << "ms" << endl;
    cout << "\ttime per remove: " << perRecord * 1000 << "ms" << endl;
}

void
TestApp::generatedReadWithIndex(IndexedIntIntMap& m, int reads, const GeneratorPtr& gen)
{
    _watch.start();
    for(int i = 0; i < reads; ++i)
    {
        int value = gen->next();
        IndexedIntIntMap::iterator p = m.findByValue(value);
        test(p != m.end());
        test(p->second == value);
    }
    IceUtil::Time total = _watch.stop();
    IceUtil::Time perRecord = total / reads;

    cout << "\ttime for " << reads << " reverse (indexed) reads of " << gen->toString() << " records: "
         << total * 1000 << "ms" << endl;
    cout << "\ttime per reverse read: " << perRecord * 1000 << "ms" << endl;
}

template<class T> void 
TestApp::generatedRead(T& m, int reads , const GeneratorPtr& gen)
{
    _watch.start();
    for(int i = 0; i < reads; ++i)
    {
        int key = gen->next();
        typename T::iterator p = m.find(key);
        test(p != m.end());
        test(p->second == key);
    }
    IceUtil::Time total = _watch.stop();
    IceUtil::Time perRecord = total / reads;
    
    cout << "\ttime for " << reads << " reads of " << gen->toString() << " records: " << total * 1000 << "ms"
            << endl;
    cout << "\ttime per read: " << perRecord * 1000 << "ms" << endl;
    
    generatedReadWithIndex(m, reads, gen);
}
void
TestApp::Struct1Struct2MapIndexTest(IndexedStruct1Struct2Map& m)
{
    int i;
    _watch.start();
    for(i = 0; i < _repetitions; ++i)
    {
        ostringstream os;
        os << i;

        IndexedStruct1Struct2Map::iterator p = m.findByS(os.str());
        test(p != m.end());
        test(p->first.l == i);
        test(p->second.s1.l == i);
    }
    
    for(i = 0; i < _repetitions; ++i)
    {
        Struct1 s1;
        s1.l = i;
        IndexedStruct1Struct2Map::iterator p = m.findByS1(s1);
        test(p != m.end());
        test(p->first.l == i);
        test(p->second.s1.l == i);
    }

    IceUtil::Time total = _watch.stop();
    IceUtil::Time perRecord = total / (2 *_repetitions);

    cout << "\ttime for " << 2 *_repetitions << " indexed reads: " << total * 1000 << "ms" << endl;
    cout << "\ttime per indexed read: " << perRecord * 1000 << "ms" << endl;
}

template<class T> void 
TestApp::Struct1Struct2MapTest(const string& mapName, T*)
{
    T m(_connection, mapName);
    
    //
    // Populate the database.
    //
    Struct1 s1;
    Struct2 s2;
    int i;
    _watch.start();
    {
        Freeze::TransactionHolder txHolder(_connection);
        for(i = 0; i < _repetitions; ++i)
        {
            s1.l = i;
            ostringstream os;
            os << i;
            s2.s = os.str();
            s2.s1 = s1;
            m.put(typename T::value_type(s1, s2));
        }
        txHolder.commit();
    }
    IceUtil::Time total = _watch.stop();
    IceUtil::Time perRecord = total / _repetitions;
    
    cout << "\ttime for " << _repetitions << " writes: " << total * 1000 << "ms" << endl;
    cout << "\ttime per write: " << perRecord * 1000 << "ms" << endl;
    
    //
    // Read each record.
    //
    _watch.start();
    for(i = 0; i < _repetitions; ++i)
    {
        s1.l = i;
        typename T::iterator p = m.find(s1);
        test(p != m.end());
        ostringstream os;
        os << i;
        test(p->second.s == os.str());
    }
    total = _watch.stop();
    perRecord = total / _repetitions;
    
    cout << "\ttime for " << _repetitions << " reads: " << total * 1000 << "ms" << endl;
    cout << "\ttime per read: " << perRecord * 1000 << "ms" << endl;
    
    //
    // Optional index test
    //
    Struct1Struct2MapIndexTest(m);
    
    //
    // Remove each record.
    //
    _watch.start();
    {
        Freeze::TransactionHolder txHolder(_connection);
        for(i = 0; i < _repetitions; ++i)
        {
            s1.l = i;
            m.erase(s1);
        }
        txHolder.commit();
    }
    total = _watch.stop();
    perRecord = total / _repetitions;
    
    cout << "\ttime for " << _repetitions << " removes: " << total * 1000 << "ms" << endl;
    cout << "\ttime per remove: " << perRecord * 1000 << "ms" << endl;
}

void
TestApp::Struct1Class1MapIndexTest(IndexedStruct1Class1Map& m)
{
    //
    // Read each record.
    //
    _watch.start();
    for(int i = 0; i < _repetitions; ++i)
    {
        ostringstream os;
        os << i;

        IndexedStruct1Class1Map::iterator p = m.findByS(os.str());
        test(p != m.end());
        test(p->first.l == i);
    }
    IceUtil::Time total = _watch.stop();
    IceUtil::Time perRecord = total / _repetitions;

    cout << "\ttime for " << _repetitions << " indexed reads: " << total * 1000 << "ms" << endl;
    cout << "\ttime per indexed read: " << perRecord * 1000 << "ms" << endl;
}

template<class T> void 
TestApp::Struct1Class1MapTest(const string& mapName, T*)
{
    T m(_connection, mapName);
    
    //
    // Populate the database.
    //
    Struct1 s1;
    Class1Ptr c1 = new Class1();
    int i;
    _watch.start();
    {
        Freeze::TransactionHolder txHolder(_connection);
        for(i = 0; i < _repetitions; ++i)
        {
            s1.l = i;
            ostringstream os;
            os << i;
            c1->s = os.str();
            m.put(typename T::value_type(s1, c1));
        }
        txHolder.commit();
    }
    IceUtil::Time total = _watch.stop();
    IceUtil::Time perRecord = total / _repetitions;
    
    cout << "\ttime for " << _repetitions << " writes: " << total * 1000 << "ms" << endl;
    cout << "\ttime per write: " << perRecord * 1000 << "ms" << endl;
    
    //
    // Read each record.
    //
    _watch.start();
    for(i = 0; i < _repetitions; ++i)
    {
        s1.l = i;
        typename T::iterator p = m.find(s1);
        test(p != m.end());
        ostringstream os;
        os << i;
        test(p->second->s == os.str());
    }
    total = _watch.stop();
    perRecord = total / _repetitions;
    
    cout << "\ttime for " << _repetitions << " reads: " << total * 1000 << "ms" << endl;
    cout << "\ttime per read: " << perRecord * 1000 << "ms" << endl;
    
    //
    // Optional index test
    //
    
    Struct1Class1MapIndexTest(m);
    
    //
    // Remove each record.
    //
    _watch.start();
    {
        Freeze::TransactionHolder txHolder(_connection);
        for(i = 0; i < _repetitions; ++i)
        {
            s1.l = i;
            m.erase(s1);
        }
        txHolder.commit();
    }
    total = _watch.stop();
    perRecord = total / _repetitions;
    
    cout << "\ttime for " << _repetitions << " removes: " << total * 1000 << "ms" << endl;
    cout << "\ttime per remove: " << perRecord * 1000 << "ms" << endl;
}

void
TestApp::Struct1ObjectMapTest()
{
    Struct1ObjectMap m(_connection, "Struct1Object");

    //
    // Populate the database.
    //
    Struct1 s1;
    Class1Ptr c1 = new Class1();
    Class2Ptr c2 = new Class2();

    c2->rec = c2;
    c2->obj = c1;
    int i;
    _watch.start();
    {
        Freeze::TransactionHolder txHolder(_connection);
        for(i = 0; i < _repetitions; ++i)
        {
            s1.l = i;
            Ice::ObjectPtr o;
            if((i % 2) == 0)
            {
                o = c2;
            }
            else
            {
                o = c1;
            }
            ostringstream os;
            os << i;
            c1->s = os.str();
            m.put(Struct1ObjectMap::value_type(s1, o));
        }
        txHolder.commit();
    }
    IceUtil::Time total = _watch.stop();
    IceUtil::Time perRecord = total / _repetitions;

    cout << "\ttime for " << _repetitions << " writes: " << total * 1000 << "ms" << endl;
    cout << "\ttime per write: " << perRecord * 1000 << "ms" << endl;

    //
    // Read each record.
    //
    _watch.start();
    for(i = 0; i < _repetitions; ++i)
    {
        s1.l = i;
        Struct1ObjectMap::iterator p = m.find(s1);
        test(p != m.end());
        Ice::ObjectPtr o = p->second;
        Class1Ptr nc1;
        if((i % 2) == 0)
        {
            Class2Ptr nc2 = Class2Ptr::dynamicCast(o);
            test(nc2);

            test(nc2->rec == nc2);
            nc1 = Class1Ptr::dynamicCast(nc2->obj);
        }
        else
        {
            nc1 = Class1Ptr::dynamicCast(o);
        }
        test(nc1);
        ostringstream os;
        os << i;
        test(nc1->s == os.str());
    }
    total = _watch.stop();
    perRecord = total / _repetitions;

    cout << "\ttime for " << _repetitions << " reads: " << total * 1000 << "ms" << endl;
    cout << "\ttime per read: " << perRecord * 1000 << "ms" << endl;

    //
    // Remove each record.
    //
    _watch.start();
    {
        Freeze::TransactionHolder txHolder(_connection);
        for(i = 0; i < _repetitions; ++i)
        {
            s1.l = i;
            m.erase(s1);
        }
        txHolder.commit();
    }
    total = _watch.stop();
    perRecord = total / _repetitions;

    cout << "\ttime for " << _repetitions << " removes: " << total * 1000 << "ms" << endl;
    cout << "\ttime per remove: " << perRecord * 1000 << "ms" << endl;
}

template<class T> void 
TestApp::IntIntMapReadTest(const string& mapName, T*)
{
    T m(_connection, mapName);
    
    //
    // Populate the database.
    //
    int i;
    _watch.start();
    {
        Freeze::TransactionHolder txHolder(_connection);
        for(i = 0; i < _repetitions; ++i)
        {
            m.put(typename T::value_type(i, i));
        }
        txHolder.commit();
    }
    IceUtil::Time total = _watch.stop();
    IceUtil::Time perRecord = total / _repetitions;
    
    cout << "\ttime for " << _repetitions << " writes: " << total * 1000 << "ms" << endl;
    cout << "\ttime per write: " << perRecord * 1000 << "ms" << endl;
    
    //
    // Do some read tests.
    //
    generatedRead(m, _repetitions, new SequentialGenerator(1000, 1000));
    generatedRead(m, _repetitions, new SequentialGenerator(2000, 2009));
    generatedRead(m, _repetitions, new SequentialGenerator(3000, 3099));
    generatedRead(m, _repetitions, new SequentialGenerator(4000, 4999));
    
    //
    // Do a random read test.
    //
    generatedRead(m, _repetitions, new RandomGenerator(0, 10000));
    
    //
    // Remove each record.
    //
    /*
        *      For this test I don't want to remove the records because I
        *      want to examine the cache stats for the database.
        *
        _watch.start();
        for(i = 0; i < _repetitions; ++i)
        {
            m.erase(i);
        }
        total = _watch.stop();
        perRecord = total / _repetitions;
        
        cout << "\ttime for " << _repetitions << " removes: " << total * 1000 << "ms" << endl;
        cout << "\ttime per remove: " << perRecord * 1000 << "ms" << endl;
    */
}

class MyFactory : public Ice::ObjectFactory
{
public:

    Ice::ObjectPtr
    create(const string& type)
    {
        if(type == "::Class1")
        {
            return new Class1();
        }
        else if(type == "::Class2")
        {
            return new Class2();
        }
        return 0;
    }
    
    void
    destroy()
    {
    }
};
typedef IceUtil::Handle<MyFactory> MyFactoryPtr;

int
TestApp::run(int argc, char*[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    _connection = Freeze::createConnection(communicator(), _envName);

    cout << "IntIntMap" << endl;
    IntIntMapTest<IntIntMap>("IntIntMap");
    
    cout << "IntIntMap with index" << endl;
    IntIntMapTest<IndexedIntIntMap>("IndexedIntIntMap");

    cout <<"Struct1Struct2Map" << endl;
    Struct1Struct2MapTest<Struct1Struct2Map>("Struct1Struct2Map");

    cout <<"Struct1Struct2Map with index" << endl;
    Struct1Struct2MapTest<IndexedStruct1Struct2Map>("IndexedStruct1Struct2Map");

    cout <<"Struct1Class1Map" << endl;
    Struct1Class1MapTest<Struct1Class1Map>("Struct1Class1Map");

    cout <<"Struct1Class1Map with index" << endl;
    Struct1Class1MapTest<IndexedStruct1Class1Map>("IndexedStruct1Class1Map");

    MyFactoryPtr factory = new MyFactory();

    cout <<"Struct1ObjectMap" << endl;
    Struct1ObjectMapTest();

    cout <<"IntIntMap (read test)" << endl;
    IntIntMapReadTest<IntIntMap>("IntIntMap");

    cout <<"IntIntMap with index (read test)" << endl;
    IntIntMapReadTest<IndexedIntIntMap>("IndexedIntIntMap");
    
    _connection->close();
    
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    TestApp app("db");
    return app.main(argc, argv);
}
