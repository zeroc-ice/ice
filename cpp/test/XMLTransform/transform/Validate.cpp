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

#include <Freeze/Freeze.h>
#include <TestCommon.h>
#include <IntByteMap.h>
#include <IntShortMap.h>
#include <IntIntMap.h>
#include <IntLongMap.h>
#include <IntFloatMap.h>
#include <IntDoubleMap.h>
#include <IntSeq1MapNew.h>
#include <IntSeq2MapNew.h>
#include <IntSeq3MapNew.h>
#include <IntSeq4MapNew.h>
#include <IntE1MapNew.h>
#include <IntD1MapNew.h>
#include <IntS1MapNew.h>
#include <IntC1MapNew.h>

#include <fstream>

using namespace std;
using namespace Freeze;
using namespace Ice;

#define NUM_KEYS        50
#define NUM_ELEMENTS    50

static void
usage(const char* n)
{
    cerr << "Usage: " << n << " [options]\n";
    cerr <<        
        "Options:\n"
        "--db-dir DIR         Use directory DIR for the database environment.\n"
        ;
}

static void
validatePrimitive(const CommunicatorPtr& communicator, const string& envName)
{  
    cout << "validating primitive transformations... " << flush;
    
    //
    // Validate byte to short transformation
    //
  
    {
	IntShortMap map(communicator, envName, "byteToShort", false);
	for(IntShortMap::iterator p = map.begin(); p != map.end(); ++p)
	{
	    test(p->second == p->first);
	}
    }

    //
    // Validate short to int transformation
    //
   
    {
	IntIntMap map(communicator, envName, "shortToInt", false);
	for(IntIntMap::iterator p = map.begin(); p != map.end(); ++p)
	{
	    test(p->second == p->first);
	}
    }
        
    //
    // Validate int to long transformation
    //
    
    {
	IntLongMap map(communicator, envName, "intToLong", false);
	for(IntLongMap::iterator p = map.begin(); p != map.end(); ++p)
	{
	    test(p->second == p->first);
	}
    }
    
    //
    // Validate long to byte transformation
    //
    {
	IntByteMap map(communicator, envName, "longToByte", false);
	for(IntByteMap::iterator p = map.begin(); p != map.end(); ++p)
	{
	    test(p->second == p->first);
	}
    }
   
    {
	IntDoubleMap map(communicator, envName, "floatToDouble", false);
	for(IntDoubleMap::iterator p = map.begin(); p != map.end(); ++p)
	{
	    test(p->second - p->first <= 0.001);
	}
    }

    cout << "ok" << endl;
    
}

static void
validatePrimitiveSequence(const CommunicatorPtr& communicator, const string& envName)
{
    Int i;

    cout << "validating primitive sequence transformations... " << flush;

    //
    // Validate byte to short sequence transformation
    //
   
    {
	IntSeq1Map map(communicator, envName, "byteToShortSeq", false);
	test(map.size() == 1);
	IntSeq1Map::iterator p = map.find(0);
	test(p != map.end());
	const Test::Seq1& seq = p->second;
	test(seq.size() == NUM_ELEMENTS);
	for(i = 0; i < NUM_ELEMENTS; i++)
	{
	    test(seq[i] == i);
	}
    }
    
    //
    // Validate short to int sequence transformation
    //
 
    {
	IntSeq2Map map(communicator, envName, "shortToIntSeq", false);
	test(map.size() == 1);
	IntSeq2Map::iterator p = map.find(0);
	test(p != map.end());
	const Test::Seq2& seq = p->second;
	test(seq.size() == NUM_ELEMENTS);
	for(i = 0; i < NUM_ELEMENTS; i++)
	{
	    test(seq[i] == i);
	}
    }
    
    //
    // Validate int to long sequence transformation
    //
   
    {
	IntSeq3Map map(communicator, envName, "intToLongSeq", false);
	test(map.size() == 1);
	IntSeq3Map::iterator p = map.find(0);
	test(p != map.end());
	const Test::Seq3& seq = p->second;
	test(seq.size() == NUM_ELEMENTS);
	for(i = 0; i < NUM_ELEMENTS; i++)
	{
	    test(seq[i] == i);
	}
    }

    //
    // Validate long to byte sequence transformation
    //    
    {
	IntSeq4Map map(communicator, envName, "longToByteSeq", false);
	test(map.size() == 1);
	IntSeq4Map::iterator p = map.find(0);
	test(p != map.end());
	const Test::Seq4& seq = p->second;
	test(seq.size() == NUM_ELEMENTS);
	for(i = 0; i < NUM_ELEMENTS; i++)
	{
	    test(seq[i] == i);
	}
    }

    cout << "ok" << endl;
}

static void
validateEnum(const CommunicatorPtr& communicator, const string& envName)
{
    cout << "validating enumeration transformations... " << flush;

   
    //
    // Validate removal of E1::three
    //
    {
	IntE1Map map(communicator, envName, "enum", false);
	test(map.size() == 2);
	IntE1Map::iterator p;
	p = map.find(0);
	test(p != map.end());
	test(p->second == Test::one);
	p = map.find(1);
	test(p != map.end());
	test(p->second == Test::two);
    }

    cout << "ok" << endl;
}

static void
validateDictionary(const CommunicatorPtr& communicator, const string& envName)
{
    cout << "validating dictionary transformations... " << flush;

    //
    // Validate removal of E1::three
    //
    {
	IntD1Map map(communicator, envName, "dict", false);
	IntD1Map::iterator p = map.find(0);
	test(p != map.end());
	const Test::D1& dict = p->second;
	test(dict.size() == 2);
	Test::D1::const_iterator q;
	q = dict.find("one");
	test(q != dict.end());
	test(q->second == Test::one);
	q = dict.find("two");
	test(q != dict.end());
	test(q->second == Test::two);
    }
    cout << "ok" << endl;
}

static void
validateStruct(const CommunicatorPtr& communicator, const string& envName)
{
    cout << "validating struct transformations... " << flush;

   
    //
    // Validate S1
    //
    {
	IntS1Map map(communicator, envName, "struct", false);
	for(IntS1Map::iterator p = map.begin(); p != map.end(); ++p)
	{
	    test(p->second.i == p->first);
	    test(p->second.s == 0);
	}
    }
    cout << "ok" << endl;
   
}

static void
validateClass(const CommunicatorPtr& communicator, const string& envName)
{
    communicator->addObjectFactory(Test::C1::ice_factory(), Test::C1::ice_staticId());
    communicator->addObjectFactory(Test::C2::ice_factory(), Test::C2::ice_staticId());

    cout << "validating class transformations... " << flush;

    {
	IntC1Map map(communicator, envName, "class", false);
	for(IntC1Map::iterator p = map.begin(); p != map.end(); ++p)
	{
	    Test::C2Ptr c2 = Test::C2Ptr::dynamicCast(p->second);
	    test(c2);
	    test(c2->s.size() == 1 && c2->s[0] == '0' + p->first);
	    test(c2->b == p->first);
	    test(c2->l == p->first);
	    test(c2->i == p->first);
	    if(c2->s == "3")
	    {
		//
		// Test facets
		//
		Ice::ObjectPtr obj;
		Test::C1Ptr c1;
		
		obj = c2->ice_findFacet("c1-0");
		test(obj);
		c1 = Test::C1Ptr::dynamicCast(obj);
		test(c1);
		test(c1->s == "c1-0");
		test(c1->b == 0);
		
		obj = c2->ice_findFacet("c1-1");
		test(obj);
		c1 = Test::C1Ptr::dynamicCast(obj);
		test(c1);
		test(c1->s == "c1-1");
		test(c1->b == 1);
		
		obj = c2->ice_findFacet("c1-2");
		test(obj);
		Test::C2Ptr c2Facet = Test::C2Ptr::dynamicCast(obj);
		test(c2Facet);
		test(c2Facet->s == "c1-2");
		test(c2Facet->b == 2);
		test(c2Facet->i == 2);
		test(c2Facet->l == 2);
		
		obj = c2Facet->ice_findFacet("c2-0");
		test(obj);
		c1 = Test::C1Ptr::dynamicCast(obj);
		test(c1);
		test(c1->s == "c2-0");
		test(c1->b == 0);
	    }
	}
    }

    cout << "ok" << endl;

    communicator->removeObjectFactory(Test::C1::ice_staticId());
    communicator->removeObjectFactory(Test::C2::ice_staticId());   
}

static void
validateEvictor(const CommunicatorPtr& communicator, const string& envName)
{
    Freeze::EvictorPtr evictor;

    communicator->addObjectFactory(Test::C1::ice_factory(), Test::C1::ice_staticId());
    communicator->addObjectFactory(Test::C2::ice_factory(), Test::C2::ice_staticId());

    cout << "validating evictor transformations... " << flush;

  
    //
    // Validate C2
    //
    evictor = Freeze::createEvictor(communicator, envName, "evictor", false);
    {
	EvictorIteratorPtr iter = evictor->getIterator();
	while(iter->hasNext())
	{
	    Current current;
	    LocalObjectPtr cookie;
	    
	    current.id = iter->next();
	    
	    ObjectPtr object = evictor->locate(current, cookie);
	    test(object);
	    
	    Test::C2Ptr c2 = Test::C2Ptr::dynamicCast(object);
	    test(c2);
	    test(c2->s.size() == 1 && c2->s == current.id.name);
	    int i = current.id.name[0] - '0';
	    test(c2->b == i);
	    test(c2->l == i);
	    test(c2->i == i);
	    if(c2->s == "3")
	    {
		//
		// Test facets
		//
		Ice::ObjectPtr obj;
		Test::C1Ptr c1;
		
		obj = c2->ice_findFacet("c1-0");
		test(obj);
		c1 = Test::C1Ptr::dynamicCast(obj);
		test(c1);
		test(c1->s == "c1-0");
		test(c1->b == 0);
		
		obj = c2->ice_findFacet("c1-1");
		test(obj);
		c1 = Test::C1Ptr::dynamicCast(obj);
		test(c1);
		test(c1->s == "c1-1");
		test(c1->b == 1);
		
		obj = c2->ice_findFacet("c1-2");
		test(obj);
		Test::C2Ptr c2Facet = Test::C2Ptr::dynamicCast(obj);
		test(c2Facet);
		test(c2Facet->s == "c1-2");
		test(c2Facet->b == 2);
		test(c2Facet->i == 2);
		test(c2Facet->l == 2);
		
		obj = c2Facet->ice_findFacet("c2-0");
		test(obj);
		c1 = Test::C1Ptr::dynamicCast(obj);
		test(c1);
		test(c1->s == "c2-0");
		test(c1->b == 0);
	    }
	    
	    evictor->finished(current, object, cookie);
	}
	iter->destroy();
    }
    
    evictor->deactivate("");
    
    cout << "ok" << endl;

    communicator->removeObjectFactory(Test::C1::ice_staticId());
    communicator->removeObjectFactory(Test::C2::ice_staticId());
}

static int
run(int argc, char* argv[], const CommunicatorPtr& communicator)
{
    string envName = "db";

    int idx = 1;
    while(idx < argc)
    {
        if(strcmp(argv[idx], "--db-dir") == 0)
        {
            if(idx + 1 >= argc)
            {
                cerr << argv[0] << ": argument expected for`" << argv[idx] << "'" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }

            envName = argv[idx + 1];
            for(int i = idx ; i + 2 < argc ; ++i)
            {
                argv[i] = argv[i + 2];
            }
            argc -= 2;
        }
        else if(argv[idx][0] == '-')
        {
            cerr << argv[0] << ": unknown option `" << argv[idx] << "'" << endl;
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        else
        {
            ++idx;
        }
    }

    try
    {
        validatePrimitive(communicator, envName);
        validatePrimitiveSequence(communicator, envName);
        validateEnum(communicator, envName);
        validateDictionary(communicator, envName);
        validateStruct(communicator, envName);
        validateClass(communicator, envName);
        validateEvictor(communicator, envName);
    }
    catch(Exception& ex)
    {
	cerr << "Caught Exception" << endl;
	cerr << ex << endl;
    }
    catch(...)
    {
	cerr << "Caught exception" << endl;
        throw;
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
        status = run(argc, argv, communicator);
    }
    catch(const Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }

    try
    {
        communicator->destroy();
    }
    catch(const Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }

    return status;
}
