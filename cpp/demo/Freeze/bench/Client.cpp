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

#include <Freeze/Application.h>
#include <BenchTypes.h>
#include <cstdlib>

using namespace std;

static void
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
	_start = IceUtil::Time::now();
    }

    double
    stop()
    {
	if(!_stopped)
	{
	    _stopped = true;
	    _stop = IceUtil::Time::now();
	}

	return (_stop - _start) * 1000.0;
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

    int _max;
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

    int _min;
    int _max;
    int _current;
};

class TestApp : public Freeze::Application
{
public:
    
    TestApp(const string&);

    virtual int runFreeze(int, char*[], const Freeze::DBEnvironmentPtr&);

private:

    void IntIntMapTest(const Freeze::DBEnvironmentPtr&);
    void generatedRead(IntIntMap&, int, const GeneratorPtr&);
    void Struct1Struct2MapTest(const Freeze::DBEnvironmentPtr&);
    void Struct1Class1MapTest(const Freeze::DBEnvironmentPtr&);
    void Struct1ObjectMapTest(const Freeze::DBEnvironmentPtr&);
    void IntIntMapReadTest(const Freeze::DBEnvironmentPtr&);

    StopWatch _watch;
    int _repetitions;
};

TestApp::TestApp(const string& dbEnvName) :
    Freeze::Application(dbEnvName),
    _repetitions(10000)
{
}

void
TestApp::IntIntMapTest(const Freeze::DBEnvironmentPtr& dbEnv)
{
    Freeze::DBPtr db = dbEnv->openDB("IntIntMap", true);

    IntIntMap m(db);

    //
    // Populate the database.
    //
    int i;
    _watch.start();
    for(i = 0; i < _repetitions; ++i)
    {
	m.insert(IntIntMap::value_type(i, i));
    }
    double total = _watch.stop();
    double perRecord = total / _repetitions;

    cout << "\ttime for " << _repetitions << " writes: " << total  << "ms" << endl;
    cout << "\ttime per write: " << perRecord << "ms" << endl;

    //
    // Read each record.
    //
    _watch.start();
    for(i = 0; i < _repetitions; ++i)
    {
	IntIntMap::const_iterator p = m.find(i);
	test(p != m.end());
	test(p->second == i);
    }
    total = _watch.stop();
    perRecord = total / _repetitions;

    cout << "\ttime for " << _repetitions << " reads: " << total  << "ms" << endl;
    cout << "\ttime per read: " << perRecord << "ms" << endl;

    //
    // Remove each record.
    //
    _watch.start();
    for(i = 0; i < _repetitions; ++i)
    {
	m.erase(i);
    }
    total = _watch.stop();
    perRecord = total / _repetitions;

    cout << "\ttime for " << _repetitions << " removes: " << total  << "ms" << endl;
    cout << "\ttime per remove: " << perRecord << "ms" << endl;

    db->close();
}

void
TestApp::generatedRead(IntIntMap& m, int reads , const GeneratorPtr& gen)
{
    _watch.start();
    for(int i = 0; i < reads; ++i)
    {
	int key = gen->next();
	IntIntMap::const_iterator p = m.find(key);
	test(p != m.end());
	test(p->second == key);
    }
    double total = _watch.stop();
    double perRecord = total / reads;

    cout << "\ttime for " << reads << " reads of " << gen->toString() << " records: " << total << "ms" << endl;
    cout << "\ttime per read: " << perRecord << "ms" << endl;
    
}

void
TestApp::IntIntMapReadTest(const Freeze::DBEnvironmentPtr& dbEnv)
{
    Freeze::DBPtr db = dbEnv->openDB("IntIntMap", true);

    IntIntMap m(db);

    //
    // Populate the database.
    //
    int i;
    _watch.start();
    for(i = 0; i < _repetitions; ++i)
    {
	m.insert(IntIntMap::value_type(i, i));
    }
    double total = _watch.stop();
    double perRecord = total / _repetitions;

    cout << "\ttime for " << _repetitions << " writes: " << total  << "ms" << endl;
    cout << "\ttime per write: " << perRecord << "ms" << endl;

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

    cout << "\ttime for " << _repetitions << " removes: " << total  << "ms" << endl;
    cout << "\ttime per remove: " << perRecord << "ms" << endl;
*/

    db->close();
}

void
TestApp::Struct1Struct2MapTest(const Freeze::DBEnvironmentPtr& dbEnv)
{
    Freeze::DBPtr db = dbEnv->openDB("Struct1Struct2", true);

    Struct1Struct2Map m(db);

    //
    // Populate the database.
    //
    Struct1 s1;
    Struct2 s2;
    int i;
    _watch.start();
    for(i = 0; i < _repetitions; ++i)
    {
	s1.l = i;
	ostringstream os;
	os << i;
	s2.s = os.str();
	m.insert(Struct1Struct2Map::value_type(s1, s2));
    }
    double total = _watch.stop();
    double perRecord = total / _repetitions;

    cout << "\ttime for " << _repetitions << " writes: " << total  << "ms" << endl;
    cout << "\ttime per write: " << perRecord << "ms" << endl;

    //
    // Read each record.
    //
    _watch.start();
    for(i = 0; i < _repetitions; ++i)
    {
	s1.l = i;
	Struct1Struct2Map::const_iterator p = m.find(s1);
	test(p != m.end());
	ostringstream os;
	os << i;
	test(p->second.s == os.str());
    }
    total = _watch.stop();
    perRecord = total / _repetitions;

    cout << "\ttime for " << _repetitions << " reads: " << total  << "ms" << endl;
    cout << "\ttime per read: " << perRecord << "ms" << endl;

    //
    // Remove each record.
    //
    _watch.start();
    for(i = 0; i < _repetitions; ++i)
    {
	s1.l = i;
	m.erase(s1);
    }
    total = _watch.stop();
    perRecord = total / _repetitions;

    cout << "\ttime for " << _repetitions << " removes: " << total  << "ms" << endl;
    cout << "\ttime per remove: " << perRecord << "ms" << endl;

    db->close();
}
void
TestApp::Struct1Class1MapTest(const Freeze::DBEnvironmentPtr& dbEnv)
{
    Freeze::DBPtr db = dbEnv->openDB("Struct1Class1", true);

    Struct1Class1Map m(db);

    //
    // Populate the database.
    //
    Struct1 s1;
    Class1Ptr c1 = new Class1();
    int i;
    _watch.start();
    for(i = 0; i < _repetitions; ++i)
    {
	s1.l = i;
	ostringstream os;
	os << i;
	c1->s = os.str();
	m.insert(Struct1Class1Map::value_type(s1, c1));
    }
    double total = _watch.stop();
    double perRecord = total / _repetitions;

    cout << "\ttime for " << _repetitions << " writes: " << total  << "ms" << endl;
    cout << "\ttime per write: " << perRecord << "ms" << endl;

    //
    // Read each record.
    //
    _watch.start();
    for(i = 0; i < _repetitions; ++i)
    {
	s1.l = i;
	Struct1Class1Map::const_iterator p = m.find(s1);
	test(p != m.end());
	ostringstream os;
	os << i;
	test(p->second->s == os.str());
    }
    total = _watch.stop();
    perRecord = total / _repetitions;

    cout << "\ttime for " << _repetitions << " reads: " << total  << "ms" << endl;
    cout << "\ttime per read: " << perRecord << "ms" << endl;

    //
    // Remove each record.
    //
    _watch.start();
    for(i = 0; i < _repetitions; ++i)
    {
	s1.l = i;
	m.erase(s1);
    }
    total = _watch.stop();
    perRecord = total / _repetitions;

    cout << "\ttime for " << _repetitions << " removes: " << total  << "ms" << endl;
    cout << "\ttime per remove: " << perRecord << "ms" << endl;

    db->close();
}

void
TestApp::Struct1ObjectMapTest(const Freeze::DBEnvironmentPtr& dbEnv)
{
    Freeze::DBPtr db = dbEnv->openDB("Struct1Object", true);

    Struct1ObjectMap m(db);

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
	m.insert(Struct1ObjectMap::value_type(s1, o));
    }
    double total = _watch.stop();
    double perRecord = total / _repetitions;

    cout << "\ttime for " << _repetitions << " writes: " << total  << "ms" << endl;
    cout << "\ttime per write: " << perRecord << "ms" << endl;

    //
    // Read each record.
    //
    _watch.start();
    for(i = 0; i < _repetitions; ++i)
    {
	s1.l = i;
	Struct1ObjectMap::const_iterator p = m.find(s1);
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

    cout << "\ttime for " << _repetitions << " reads: " << total  << "ms" << endl;
    cout << "\ttime per read: " << perRecord << "ms" << endl;

    //
    // Remove each record.
    //
    _watch.start();
    for(i = 0; i < _repetitions; ++i)
    {
	s1.l = i;
	m.erase(s1);
    }
    total = _watch.stop();
    perRecord = total / _repetitions;

    cout << "\ttime for " << _repetitions << " removes: " << total  << "ms" << endl;
    cout << "\ttime per remove: " << perRecord << "ms" << endl;

    db->close();
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
    
    void
    install(const Ice::CommunicatorPtr& communicator)
    {
	communicator->addObjectFactory(this, "::Class1");
	communicator->addObjectFactory(this, "::Class2");
    }
};
typedef IceUtil::Handle<MyFactory> MyFactoryPtr;

int
TestApp::runFreeze(int argc, char* argv[], const Freeze::DBEnvironmentPtr& dbEnv)
{
    cout <<"IntIntMap" << endl;
    IntIntMapTest(dbEnv);
    
    cout <<"Struct1Struct2Map" << endl;
    Struct1Struct2MapTest(dbEnv);
    
    cout <<"Struct1Class1Map" << endl;
    Struct1Class1MapTest(dbEnv);
    
    MyFactoryPtr factory = new MyFactory();
    factory->install(dbEnv->getCommunicator());
    
    cout <<"Struct1ObjectMap" << endl;
    Struct1ObjectMapTest(dbEnv);
    
    cout <<"IntIntMap (read test)" << endl;
    IntIntMapReadTest(dbEnv);
    
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    TestApp app("db");
    return app.main(argc, argv, "config");
}
