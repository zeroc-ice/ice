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
using namespace Ice;
using namespace Freeze;

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
validatePrimitive(const DBEnvironmentPtr& dbEnv)
{
    DBPtr db;

    cout << "validating primitive transformations... " << flush;

    try
    {

        //
        // Validate byte to short transformation
        //
        db = dbEnv->openDB("byteToShort", false);

        {
            IntShortMap map(db);
            for(IntShortMap::iterator p = map.begin(); p != map.end(); ++p)
            {
                test(p->second == p->first);
            }
        }

        db->close();
        db = 0;

        //
        // Validate short to int transformation
        //
        db = dbEnv->openDB("shortToInt", false);

        {
            IntIntMap map(db);
            for(IntIntMap::iterator p = map.begin(); p != map.end(); ++p)
            {
                test(p->second == p->first);
            }
        }

        db->close();
        db = 0;

        //
        // Validate int to long transformation
        //
        db = dbEnv->openDB("intToLong", false);

        {
            IntLongMap map(db);
            for(IntLongMap::iterator p = map.begin(); p != map.end(); ++p)
            {
                test(p->second == p->first);
            }
        }

        db->close();
        db = 0;

        //
        // Validate long to byte transformation
        //
        db = dbEnv->openDB("longToByte", false);

        {
            IntByteMap map(db);
            for(IntByteMap::iterator p = map.begin(); p != map.end(); ++p)
            {
                test(p->second == p->first);
            }
        }

        db->close();
        db = 0;

        //
        // Validate float to double transformation
        //
        db = dbEnv->openDB("floatToDouble", false);

        {
            IntDoubleMap map(db);
            for(IntDoubleMap::iterator p = map.begin(); p != map.end(); ++p)
            {
                test(p->second - p->first <= 0.001);
            }
        }

        db->close();
        db = 0;

        cout << "ok" << endl;
    }
    catch(...)
    {
        cout << "failed" << endl;
        if(db)
        {
            db->close();
        }
        throw;
    }

    if(db)
    {
        db->close();
    }
}

static void
validatePrimitiveSequence(const DBEnvironmentPtr& dbEnv)
{
    Int i;

    DBPtr db;

    cout << "validating primitive sequence transformations... " << flush;

    try
    {
        //
        // Validate byte to short sequence transformation
        //
        db = dbEnv->openDB("byteToShortSeq", false);

        {
            IntSeq1Map map(db);
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

        db->close();
        db = 0;

        //
        // Validate short to int sequence transformation
        //
        db = dbEnv->openDB("shortToIntSeq", false);

        {
            IntSeq2Map map(db);
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

        db->close();
        db = 0;

        //
        // Validate int to long sequence transformation
        //
        db = dbEnv->openDB("intToLongSeq", false);

        {
            IntSeq3Map map(db);
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

        db->close();
        db = 0;

        //
        // Validate long to byte sequence transformation
        //
        db = dbEnv->openDB("longToByteSeq", false);

        {
            IntSeq4Map map(db);
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

        db->close();
        db = 0;

        cout << "ok" << endl;
    }
    catch(...)
    {
        cout << "failed" << endl;
        if(db)
        {
            db->close();
        }
        throw;
    }

    if(db)
    {
        db->close();
    }
}

static void
validateEnum(const DBEnvironmentPtr& dbEnv)
{
    DBPtr db;

    cout << "validating enumeration transformations... " << flush;

    try
    {
        //
        // Validate removal of E1::three
        //
        db = dbEnv->openDB("enum", false);

        {
            IntE1Map map(db);
            test(map.size() == 2);
            IntE1Map::iterator p;
            p = map.find(0);
            test(p != map.end());
            test(p->second == Test::one);
            p = map.find(1);
            test(p != map.end());
            test(p->second == Test::two);
        }

        db->close();
        db = 0;

        cout << "ok" << endl;
    }
    catch(...)
    {
        cout << "failed" << endl;
        if(db)
        {
            db->close();
        }
        throw;
    }

    if(db)
    {
        db->close();
    }
}

static void
validateDictionary(const DBEnvironmentPtr& dbEnv)
{
    DBPtr db;

    cout << "validating dictionary transformations... " << flush;

    try
    {
        //
        // Validate removal of E1::three
        //
        db = dbEnv->openDB("dict", false);

        {
            IntD1Map map(db);
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

        db->close();
        db = 0;

        cout << "ok" << endl;
    }
    catch(...)
    {
        cout << "failed" << endl;
        if(db)
        {
            db->close();
        }
        throw;
    }

    if(db)
    {
        db->close();
    }
}

static void
validateStruct(const DBEnvironmentPtr& dbEnv)
{
    DBPtr db;

    cout << "validating struct transformations... " << flush;

    try
    {
        //
        // Validate S1
        //
        db = dbEnv->openDB("struct", false);

        {
            IntS1Map map(db);
            for(IntS1Map::iterator p = map.begin(); p != map.end(); ++p)
            {
                test(p->second.i == p->first);
                test(p->second.s == 0);
            }
        }

        db->close();
        db = 0;

        cout << "ok" << endl;
    }
    catch(...)
    {
        cout << "failed" << endl;
        if(db)
        {
            db->close();
        }
        throw;
    }

    if(db)
    {
        db->close();
    }
}

static void
validateClass(const DBEnvironmentPtr& dbEnv)
{
    DBPtr db;

    CommunicatorPtr communicator = dbEnv->getCommunicator();
    communicator->addObjectFactory(Test::C1::ice_factory(), Test::C1::ice_staticId());
    communicator->addObjectFactory(Test::C2::ice_factory(), Test::C2::ice_staticId());

    cout << "validating class transformations... " << flush;

    try
    {
        //
        // Validate C2
        //
        db = dbEnv->openDB("class", false);

        {
            IntC1Map map(db);
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

        db->close();
        db = 0;

        cout << "ok" << endl;
    }
    catch(...)
    {
        cout << "failed" << endl;
        if(db)
        {
            db->close();
        }
        throw;
    }

    communicator->removeObjectFactory(Test::C1::ice_staticId());
    communicator->removeObjectFactory(Test::C2::ice_staticId());

    if(db)
    {
        db->close();
    }
}

static void
validateEvictor(const DBEnvironmentPtr& dbEnv)
{
    DBPtr db;
    EvictorPtr evictor;

    CommunicatorPtr communicator = dbEnv->getCommunicator();
    communicator->addObjectFactory(Test::C1::ice_factory(), Test::C1::ice_staticId());
    communicator->addObjectFactory(Test::C2::ice_factory(), Test::C2::ice_staticId());

    cout << "validating evictor transformations... " << flush;

    try
    {
        //
        // Validate C2
        //
        db = dbEnv->openDB("evictor", false);
        evictor = db->createEvictor(db->createEvictionStrategy());

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
        db->close();
        db = 0;

        cout << "ok" << endl;
    }
    catch(...)
    {
        cout << "failed" << endl;
        if(db)
        {
            db->close();
        }
        throw;
    }

    communicator->removeObjectFactory(Test::C1::ice_staticId());
    communicator->removeObjectFactory(Test::C2::ice_staticId());

    if(db)
    {
        db->close();
    }
}

static int
run(int argc, char* argv[], const CommunicatorPtr& communicator)
{
    string dbEnvDir = "db";

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

            dbEnvDir = argv[idx + 1];
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

    DBEnvironmentPtr dbEnv;

    try
    {
        dbEnv = Freeze::initialize(communicator, dbEnvDir);
        validatePrimitive(dbEnv);
        validatePrimitiveSequence(dbEnv);
        validateEnum(dbEnv);
        validateDictionary(dbEnv);
        validateStruct(dbEnv);
        validateClass(dbEnv);
        validateEvictor(dbEnv);
    }
    catch(...)
    {
        if(dbEnv)
        {
            dbEnv->close();
        }
        throw;
    }

    if(dbEnv)
    {
        dbEnv->close();
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
