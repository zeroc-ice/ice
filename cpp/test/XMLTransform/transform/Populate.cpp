// **********************************************************************
//
// Copyright (c) 2002
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
#include <XMLTransform/XMLTransform.h>
#include <TestCommon.h>
#include <IntByteMap.h>
#include <IntShortMap.h>
#include <IntIntMap.h>
#include <IntLongMap.h>
#include <IntFloatMap.h>
#include <IntDoubleMap.h>
#include <IntSeq1MapOld.h>
#include <IntSeq2MapOld.h>
#include <IntSeq3MapOld.h>
#include <IntSeq4MapOld.h>
#include <IntE1MapOld.h>
#include <IntD1MapOld.h>
#include <IntS1MapOld.h>
#include <IntC1MapOld.h>

#include <fstream>

using namespace std;
using namespace Ice;
using namespace Freeze;

#define NUM_KEYS        50
#define NUM_ELEMENTS    50

static const string oldSchema = "old.xsd";
static const string newSchema = "new.xsd";
static StringSeq paths;

static void
usage(const char* n)
{
    cerr << "Usage: " << n << " [options]\n";
    cerr <<        
        "Options:\n"
        "-IDIR                Add directory DIR to the schema search path.\n"
        "--db-dir DIR         Use directory DIR for the database environment.\n"
        ;
}

static void
emitSchema(const string& file, const string& import, const string& valueType)
{
    ofstream out(file.c_str());
    test(out.good());

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl
        << "<xs:schema xmlns:xs=\"http://www.w3.org/2002/XMLSchema\"" << endl
        << "           elementFormDefault=\"qualified\"" << endl
        << "           xmlns:tns=\"http://www.noorg.org/schemas\"" << endl
        << "           targetNamespace=\"http://www.noorg.org/schemas\">" << endl
        << "    <xs:import namespace=\"http://www.noorg.org/schemas\" schemaLocation=\"" << import << "\"/>" << endl
        << "    <xs:element name=\"Key\" type=\"xs:int\"/>" << endl
        << "    <xs:element name=\"Value\" type=\"" << valueType << "\"/></xs:schema>" << endl;
}

static void
emitSchemas(const string& fromValueType, const string& toValueType)
{
    emitSchema(oldSchema, "TestOld.xsd", fromValueType);
    emitSchema(newSchema, "TestNew.xsd", toValueType);
}

static void
transformPrimitive(const DBEnvironmentPtr& dbEnv)
{
    Int i;

    DBPtr db;
    XMLTransform::DBTransformer transformer;
    StringSeq dummy;

    cout << "transforming primitives... " << flush;

    try
    {
        //
        // Transform byte to short
        //
        db = dbEnv->openDB("byteToShort", true);
        db->clear();

        {
            IntByteMap map(db);
            for(i = 0; i < NUM_KEYS; i++)
            {
                map.insert(make_pair(i, i));
            }
        }
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "byteToShort", false);
        emitSchemas("xs:byte", "xs:short");
        transformer.transform(dbEnv, db, dummy, dummy, paths, paths, oldSchema, newSchema);
        db->close();
        db = 0;

        //
        // Transform short to int
        //
        db = dbEnv->openDB("shortToInt", true);
        db->clear();

        {
            IntShortMap map(db);
            for(i = 0; i < NUM_KEYS; i++)
            {
                map.insert(make_pair(i, i));
            }
        }
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "shortToInt", false);
        emitSchemas("xs:short", "xs:int");
        transformer.transform(dbEnv, db, dummy, dummy, paths, paths, oldSchema, newSchema);
        db->close();
        db = 0;

        //
        // Transform int to long
        //
        db = dbEnv->openDB("intToLong", true);
        db->clear();

        {
            IntIntMap map(db);
            for(i = 0; i < NUM_KEYS; i++)
            {
                map.insert(make_pair(i, i));
            }
        }
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "intToLong", false);
        emitSchemas("xs:int", "xs:long");
        transformer.transform(dbEnv, db, dummy, dummy, paths, paths, oldSchema, newSchema);
        db->close();
        db = 0;

        //
        // Transform long to byte
        //
        db = dbEnv->openDB("longToByte", true);
        db->clear();

        {
            IntLongMap map(db);
            for(i = 0; i < NUM_KEYS; i++)
            {
                map.insert(make_pair(i, i));
            }
        }
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "longToByte", false);
        emitSchemas("xs:long", "xs:byte");
        transformer.transform(dbEnv, db, dummy, dummy, paths, paths, oldSchema, newSchema);
        db->close();
        db = 0;	

        //
        // Transform float to double
        //
        db = dbEnv->openDB("floatToDouble", true);
        db->clear();

        {
            IntFloatMap map(db);
            for(i = 0; i < NUM_KEYS; i++)
            {
                map.insert(make_pair(i, static_cast<float>(i)));
            }
        }
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "floatToDouble", false);
        emitSchemas("xs:float", "xs:double");
        transformer.transform(dbEnv, db, dummy, dummy, paths, paths, oldSchema, newSchema);
        db->close();
        db = 0;

        //
        // Transform long to byte (should fail)
        //
        db = dbEnv->openDB("failure", true);
        db->clear();

        {
            IntLongMap map(db);
            Long l;
            l = SCHAR_MIN;
            map.insert(make_pair(0, l));
            map.insert(make_pair(1, l - 1)); // Out of range for byte.
            l = SCHAR_MAX;
            map.insert(make_pair(2, l));
            map.insert(make_pair(3, l + 1)); // Out of range for byte.
        }
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "failure", false);
        emitSchemas("xs:long", "xs:byte");
        try
        {
            transformer.transform(dbEnv, db, dummy, dummy, paths, paths, oldSchema, newSchema);
            test(false);
        }
        catch(const XMLTransform::IllegalTransform&)
        {
            // Expected.
        }
        db->close();
        db = 0;

        //
        // Transform long to short (should fail)
        //
        db = dbEnv->openDB("failure", true);
        db->clear();

        {
            IntLongMap map(db);
            Long l;
            l = SHRT_MIN;
            map.insert(make_pair(0, l));
            map.insert(make_pair(1, l - 1)); // Out of range for short.
            l = SHRT_MAX;
            map.insert(make_pair(2, l));
            map.insert(make_pair(3, l + 1)); // Out of range for short.
        }
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "failure", false);
        emitSchemas("xs:long", "xs:short");
        try
        {
            transformer.transform(dbEnv, db, dummy, dummy, paths, paths, oldSchema, newSchema);
            test(false);
        }
        catch(const XMLTransform::IllegalTransform&)
        {
            // Expected.
        }
        db->close();
        db = 0;

        //
        // Transform long to int (should fail)
        //
        db = dbEnv->openDB("failure", true);
        db->clear();

        {
            IntLongMap map(db);
            Long l;
            l = INT_MIN;
            map.insert(make_pair(0, l));
            map.insert(make_pair(1, l - 1)); // Out of range for int.
            l = INT_MAX;
            map.insert(make_pair(2, l));
            map.insert(make_pair(3, l + 1)); // Out of range for int.
        }
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "failure", false);
        emitSchemas("xs:long", "xs:int");
        try
        {
            transformer.transform(dbEnv, db, dummy, dummy, paths, paths, oldSchema, newSchema);
            test(false);
        }
        catch(const XMLTransform::IllegalTransform&)
        {
            // Expected.
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
transformPrimitiveSequence(const DBEnvironmentPtr& dbEnv)
{
    Int i;

    DBPtr db;
    XMLTransform::DBTransformer transformer;
    StringSeq dummy;

    cout << "transforming primitive sequences... " << flush;

    try
    {
        //
        // Transform byte to short sequence
        //
        db = dbEnv->openDB("byteToShortSeq", true);
        db->clear();

        {
            IntSeq1Map map(db);
            Test::Seq1 seq;
            for(i = 0; i < NUM_ELEMENTS; i++)
            {
                seq.push_back(i);
            }
            map.insert(make_pair(0, seq));
        }
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "byteToShortSeq", false);
        emitSchemas("tns:_internal.Test.Seq1Type", "tns:_internal.Test.Seq1Type");
        transformer.transform(dbEnv, db, dummy, dummy, paths, paths, oldSchema, newSchema);
        db->close();
        db = 0;

        //
        // Transform short to int sequence
        //
        db = dbEnv->openDB("shortToIntSeq", true);
        db->clear();

        {
            IntSeq2Map map(db);
            Test::Seq2 seq;
            for(i = 0; i < NUM_ELEMENTS; i++)
            {
                seq.push_back(i);
            }
            map.insert(make_pair(0, seq));
        }
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "shortToIntSeq", false);
        emitSchemas("tns:_internal.Test.Seq2Type", "tns:_internal.Test.Seq2Type");
        transformer.transform(dbEnv, db, dummy, dummy, paths, paths, oldSchema, newSchema);
        db->close();
        db = 0;

        //
        // Transform int to long sequence
        //
        db = dbEnv->openDB("intToLongSeq", true);
        db->clear();

        {
            IntSeq3Map map(db);
            Test::Seq3 seq;
            for(i = 0; i < NUM_ELEMENTS; i++)
            {
                seq.push_back(i);
            }
            map.insert(make_pair(0, seq));
        }
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "intToLongSeq", false);
        emitSchemas("tns:_internal.Test.Seq3Type", "tns:_internal.Test.Seq3Type");
        transformer.transform(dbEnv, db, dummy, dummy, paths, paths, oldSchema, newSchema);
        db->close();
        db = 0;

        //
        // Transform long to byte sequence
        //
        db = dbEnv->openDB("longToByteSeq", true);
        db->clear();

        {
            IntSeq4Map map(db);
            Test::Seq4 seq;
            for(i = 0; i < NUM_ELEMENTS; i++)
            {
                seq.push_back(i);
            }
            map.insert(make_pair(0, seq));
        }
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "longToByteSeq", false);
        emitSchemas("tns:_internal.Test.Seq4Type", "tns:_internal.Test.Seq4Type");
        transformer.transform(dbEnv, db, dummy, dummy, paths, paths, oldSchema, newSchema);
        db->close();
        db = 0;

        //
        // Transform long to byte sequence (should fail)
        //
        db = dbEnv->openDB("failure", true);
        db->clear();

        {
            IntSeq4Map map(db);
            Test::Seq4 seq;
            Long l;
            l = SCHAR_MIN;
            seq.push_back(l);
            seq.push_back(l - 1); // Out of range for byte.
            l = SCHAR_MAX;
            seq.push_back(l);
            seq.push_back(l + 1); // Out of range for byte.
            map.insert(make_pair(0, seq));
        }
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "failure", false);
        emitSchemas("tns:_internal.Test.Seq4Type", "tns:_internal.Test.Seq4Type");
        try
        {
            transformer.transform(dbEnv, db, dummy, dummy, paths, paths, oldSchema, newSchema);
            test(false);
        }
        catch(const XMLTransform::IllegalTransform&)
        {
            // Expected.
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
transformEnum(const DBEnvironmentPtr& dbEnv)
{
    DBPtr db;
    XMLTransform::DBTransformer transformer;
    StringSeq dummy;

    cout << "transforming enumerations... " << flush;

    try
    {
        //
        // Don't use E1::three, which is removed in new schema
        //
        db = dbEnv->openDB("enum", true);
        db->clear();

        {
            IntE1Map map(db);
            map.insert(make_pair(0, Test::one));
            map.insert(make_pair(1, Test::two));
        }
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "enum", false);
        emitSchemas("tns:_internal.Test.E1Type", "tns:_internal.Test.E1Type");
        transformer.transform(dbEnv, db, dummy, dummy, paths, paths, oldSchema, newSchema);
        db->close();
        db = 0;

        //
        // Use E1::three, which is removed in new schema (should fail)
        //
        db = dbEnv->openDB("failure", true);
        db->clear();

        {
            IntE1Map map(db);
            map.insert(make_pair(0, Test::one));
            map.insert(make_pair(1, Test::two));
            map.insert(make_pair(2, Test::three));
        }
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "failure", false);
        emitSchemas("tns:_internal.Test.E1Type", "tns:_internal.Test.E1Type");
        try
        {
            transformer.transform(dbEnv, db, dummy, dummy, paths, paths, oldSchema, newSchema);
            test(false);
        }
        catch(const XMLTransform::IllegalTransform&)
        {
            // Expected.
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
transformDictionary(const DBEnvironmentPtr& dbEnv)
{
    DBPtr db;
    XMLTransform::DBTransformer transformer;
    StringSeq dummy;

    cout << "transforming dictionaries... " << flush;

    try
    {
        //
        // Don't use E1::three, which is removed in new schema
        //
        db = dbEnv->openDB("dict", true);
        db->clear();

        {
            IntD1Map map(db);
            Test::D1 dict;
            dict.insert(make_pair(string("one"), Test::one));
            dict.insert(make_pair(string("two"), Test::two));
            map.insert(make_pair(0, dict));
        }
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "dict", false);
        emitSchemas("tns:_internal.Test.D1Type", "tns:_internal.Test.D1Type");
        transformer.transform(dbEnv, db, dummy, dummy, paths, paths, oldSchema, newSchema);
        db->close();
        db = 0;

        //
        // Use E1::three, which is removed in new schema (should fail)
        //
        db = dbEnv->openDB("failure", true);
        db->clear();

        {
            IntD1Map map(db);
            Test::D1 dict;
            dict.insert(make_pair(string("one"), Test::one));
            dict.insert(make_pair(string("two"), Test::two));
            dict.insert(make_pair(string("three"), Test::three));
            map.insert(make_pair(0, dict));
        }
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "failure", false);
        emitSchemas("tns:_internal.Test.D1Type", "tns:_internal.Test.D1Type");
        try
        {
            transformer.transform(dbEnv, db, dummy, dummy, paths, paths, oldSchema, newSchema);
            test(false);
        }
        catch(const XMLTransform::IllegalTransform&)
        {
            // Expected.
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
transformStruct(const DBEnvironmentPtr& dbEnv)
{
    DBPtr db;
    XMLTransform::DBTransformer transformer;
    StringSeq dummy;

    cout << "transforming structs... " << flush;

    try
    {
        //
        // Transform S1
        //
        db = dbEnv->openDB("struct", true);
        db->clear();

        {
            IntS1Map map(db);
            Test::S1 s1;
            s1.b = false;
            s1.i = 0;
            map.insert(make_pair(0, s1));
            s1.b = true;
            s1.i = 1;
            map.insert(make_pair(1, s1));
            s1.b = true;
            s1.i = 2;
            map.insert(make_pair(2, s1));
        }
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "struct", false);
        emitSchemas("tns:_internal.Test.S1Type", "tns:_internal.Test.S1Type");
        transformer.transform(dbEnv, db, dummy, dummy, paths, paths, oldSchema, newSchema);
        db->close();
        db = 0;

        //
        // Transform S1 (should fail)
        //
        db = dbEnv->openDB("failure", true);
        db->clear();

        {
            IntS1Map map(db);
            Test::S1 s1;
            s1.b = false;
            s1.i = SCHAR_MIN;
            map.insert(make_pair(0, s1));
            s1.b = true;
            s1.i = SCHAR_MAX;
            map.insert(make_pair(1, s1));
            s1.b = true;
            s1.i = ((Int)SCHAR_MAX) + 1; // Out of range for byte
            map.insert(make_pair(2, s1));
        }
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "failure", false);
        emitSchemas("tns:_internal.Test.S1Type", "tns:_internal.Test.S1Type");
        try
        {
            transformer.transform(dbEnv, db, dummy, dummy, paths, paths, oldSchema, newSchema);
            test(false);
        }
        catch(const XMLTransform::IllegalTransform&)
        {
            // Expected.
        }
        db->close();
        db = 0;

	//
	// Make sure nothing changed.
	//
        db = dbEnv->openDB("failure", false);
        {
            IntS1Map map(db);
	    for(IntS1Map::iterator p = map.begin(); p != map.end(); ++p)
	    {
		Test::S1 s1 = p->second;
		s1.b = false;
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
transformClass(const DBEnvironmentPtr& dbEnv)
{
    DBPtr db;
    XMLTransform::DBTransformer transformer;
    StringSeq dummy;

    cout << "transforming classes... " << flush;

    try
    {
        //
        // Transform C2
        //
        db = dbEnv->openDB("class", true);
        db->clear();

        {
            IntC1Map map(db);
            Test::C2Ptr c2;
            c2 = new Test::C2;
            c2->s = "0";
            c2->f = 0;
            c2->b = 0;
            c2->i = 0;
            c2->l = 0;
            c2->d = 0;
            map.insert(make_pair(0, c2));
            c2 = new Test::C2;
            c2->s = "1";
            c2->f = 1;
            c2->b = 1;
            c2->i = 1;
            c2->l = 1;
            c2->d = 1;
            map.insert(make_pair(1, c2));
            c2 = new Test::C2;
            c2->s = "2";
            c2->f = 2;
            c2->b = 2;
            c2->i = 2;
            c2->l = 2;
            c2->d = 2;
            map.insert(make_pair(2, c2));

            //
            // Add an object with facets.
            //
            Test::C1Ptr c1Facet;
            Test::C2Ptr c2Facet;
            c2 = new Test::C2;
            c2->s = "3";
            c2->f = 3;
            c2->b = 3;
            c2->i = 3;
            c2->l = 3;
            c2->d = 3;
            c1Facet = new Test::C1;
            c1Facet->s = "c1-0";
            c1Facet->f = 0;
            c1Facet->b = 0;
            c2->ice_addFacet(c1Facet, "c1-0");
            c1Facet = new Test::C1;
            c1Facet->s = "c1-1";
            c1Facet->f = 1;
            c1Facet->b = 1;
            c2->ice_addFacet(c1Facet, "c1-1");
            c2Facet = new Test::C2;
            c2Facet->s = "c1-2";
            c2Facet->f = 2;
            c2Facet->b = 2;
            c2Facet->i = 2;
            c2Facet->l = 2;
            c2Facet->d = 2;
            c1Facet = new Test::C1;
            c1Facet->s = "c2-0";
            c1Facet->f = 0;
            c1Facet->b = 0;
            c2Facet->ice_addFacet(c1Facet, "c2-0"); // Nested facet
            c2->ice_addFacet(c2Facet, "c1-2");
            map.insert(make_pair(3, c2));
        }
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "class", false);
        emitSchemas("tns:_internal.Test.C1Type", "tns:_internal.Test.C1Type");
        transformer.transform(dbEnv, db, dummy, dummy, paths, paths, oldSchema, newSchema);
        db->close();
        db = 0;

        //
        // Transform C2 (should fail)
        //
        db = dbEnv->openDB("failure", true);
        db->clear();

        {
            IntC1Map map(db);
            Test::C2Ptr c2;
            c2 = new Test::C2;
            c2->s = "0";
            c2->f = 0;
            c2->b = 0;
            c2->i = 0;
            c2->l = INT_MIN;
            c2->d = 0;
            map.insert(make_pair(0, c2));
            c2 = new Test::C2;
            c2->s = "1";
            c2->f = 1;
            c2->b = 1;
            c2->i = 1;
            c2->l = INT_MAX;
            c2->d = 1;
            map.insert(make_pair(1, c2));
            c2 = new Test::C2;
            c2->s = "2";
            c2->f = 2;
            c2->b = 2;
            c2->i = 2;
            c2->l = INT_MAX;
            c2->l++; // Out of range for int
            c2->d = 2;
            map.insert(make_pair(2, c2));
        }
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "failure", false);
        emitSchemas("tns:_internal.Test.C1Type", "tns:_internal.Test.C1Type");
        try
        {
            transformer.transform(dbEnv, db, dummy, dummy, paths, paths, oldSchema, newSchema);
            test(false);
        }
        catch(const XMLTransform::IllegalTransform&)
        {
            // Expected.
        }
        db->close();
        db = 0;

	//
	// Make sure nothing changed.
	//
        db = dbEnv->openDB("failure", false);
        {
            IntC1Map map(db);
	    for(IntC1Map::iterator p = map.begin(); p != map.end(); ++p)
	    {
		Test::C1Ptr c1 = p->second;
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
transformEvictor(const DBEnvironmentPtr& dbEnv)
{
    DBPtr db;
    XMLTransform::DBTransformer transformer;
    StringSeq loadOld, loadNew;
    Identity ident;
    EvictorPtr evictor;

    loadOld.push_back("TestOld.xsd");
    loadOld.push_back("Identity.xsd");
    loadNew.push_back("TestNew.xsd");
    loadNew.push_back("Identity.xsd");

    static string evictorSchema =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<xs:schema xmlns:xs=\"http://www.w3.org/2002/XMLSchema\""
        "           elementFormDefault=\"qualified\""
        "           xmlns:tns=\"http://www.noorg.org/schemas\""
        "           xmlns:ice=\"http://www.zeroc.com/schemas\""
        "           targetNamespace=\"http://www.noorg.org/schemas\">"
        "    <xs:import namespace=\"http://www.zeroc.com/schemas\" schemaLocation=\"ice.xsd\"/>"
        "    <xs:element name=\"Key\" type=\"_internal.Ice.IdentityType\"/>"
        "    <xs:element name=\"Value\" type=\"ice:_internal.objectType\"/></xs:schema>";

    cout << "transforming evictor map... " << flush;

    try
    {
        //
        // Transform C2
        //
        db = dbEnv->openDB("evictor", true);
        db->clear();
        evictor = db->createEvictor(SaveUponEviction);

        {
            Test::C2Ptr c2;
            c2 = new Test::C2;
            c2->s = "0";
            c2->f = 0;
            c2->b = 0;
            c2->i = 0;
            c2->l = 0;
            c2->d = 0;
            ident.name = "0";
            evictor->createObject(ident, c2);
            c2 = new Test::C2;
            c2->s = "1";
            c2->f = 1;
            c2->b = 1;
            c2->i = 1;
            c2->l = 1;
            c2->d = 1;
            ident.name = "1";
            evictor->createObject(ident, c2);
            c2 = new Test::C2;
            c2->s = "2";
            c2->f = 2;
            c2->b = 2;
            c2->i = 2;
            c2->l = 2;
            c2->d = 2;
            ident.name = "2";
            evictor->createObject(ident, c2);

            //
            // Add an object with facets.
            //
            Test::C1Ptr c1Facet;
            Test::C2Ptr c2Facet;
            c2 = new Test::C2;
            c2->s = "3";
            c2->f = 3;
            c2->b = 3;
            c2->i = 3;
            c2->l = 3;
            c2->d = 3;
            c1Facet = new Test::C1;
            c1Facet->s = "c1-0";
            c1Facet->f = 0;
            c1Facet->b = 0;
            c2->ice_addFacet(c1Facet, "c1-0");
            c1Facet = new Test::C1;
            c1Facet->s = "c1-1";
            c1Facet->f = 1;
            c1Facet->b = 1;
            c2->ice_addFacet(c1Facet, "c1-1");
            c2Facet = new Test::C2;
            c2Facet->s = "c1-2";
            c2Facet->f = 2;
            c2Facet->b = 2;
            c2Facet->i = 2;
            c2Facet->l = 2;
            c2Facet->d = 2;
            c1Facet = new Test::C1;
            c1Facet->s = "c2-0";
            c1Facet->f = 0;
            c1Facet->b = 0;
            c2Facet->ice_addFacet(c1Facet, "c2-0"); // Nested facet
            c2->ice_addFacet(c2Facet, "c1-2");
            ident.name = "3";
            evictor->createObject(ident, c2);
        }
        evictor->deactivate();
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "evictor", false);
        transformer.transform(dbEnv, db, loadOld, loadNew, paths, paths, evictorSchema);
        db->close();
        db = 0;

        //
        // Transform C2 (should fail)
        //
        db = dbEnv->openDB("failure", true);
        db->clear();
        evictor = db->createEvictor(SaveUponEviction);

        {
            Test::C2Ptr c2;
            c2 = new Test::C2;
            c2->s = "0";
            c2->f = 0;
            c2->b = 0;
            c2->i = 0;
            c2->l = INT_MIN;
            c2->d = 0;
            ident.name = "0";
            evictor->createObject(ident, c2);
            c2 = new Test::C2;
            c2->s = "1";
            c2->f = 1;
            c2->b = 1;
            c2->i = 1;
            c2->l = INT_MAX;
            c2->d = 1;
            ident.name = "1";
            evictor->createObject(ident, c2);
            c2 = new Test::C2;
            c2->s = "2";
            c2->f = 2;
            c2->b = 2;
            c2->i = 2;
            c2->l = INT_MAX;
            c2->l++; // Out of range for int
            c2->d = 2;
            ident.name = "2";
            evictor->createObject(ident, c2);
        }
        evictor->deactivate();
        db->close();
        db = 0;

        db = dbEnv->openDBWithTxn(0, "failure", false);
        try
        {
            transformer.transform(dbEnv, db, loadOld, loadNew, paths, paths, evictorSchema);
            test(false);
        }
        catch(const XMLTransform::IllegalTransform&)
        {
            // Expected.
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

static int
run(int argc, char* argv[], const CommunicatorPtr& communicator)
{
    string dbEnvDir = "db";

    communicator->addObjectFactory(Test::C1::ice_factory(), Test::C1::ice_staticId());
    communicator->addObjectFactory(Test::C2::ice_factory(), Test::C2::ice_staticId());

    int idx = 1;
    while(idx < argc)
    {
        if(strncmp(argv[idx], "-I", 2) == 0)
        {
            string path = argv[idx] + 2;
            if(path.length())
            {
                paths.push_back(path);
            }

            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
        else if(strcmp(argv[idx], "--db-dir") == 0)
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
        dbEnv = Freeze::initializeWithTxn(communicator, dbEnvDir);
        transformPrimitive(dbEnv);
        transformPrimitiveSequence(dbEnv);
        transformEnum(dbEnv);
        transformDictionary(dbEnv);
        transformStruct(dbEnv);
        transformClass(dbEnv);
        transformEvictor(dbEnv);
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
