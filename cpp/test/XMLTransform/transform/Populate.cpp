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
#include <db_cxx.h>

#include <fstream>

#include <sys/stat.h>
#ifdef _WIN32
#   define FREEZE_DB_MODE 0
#else
#   define FREEZE_DB_MODE (S_IRUSR | S_IWUSR)
#endif

using namespace std;
using namespace Freeze;
using namespace Ice;

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
transformPrimitive(const CommunicatorPtr& communicator, DbEnv& dbEnv)
{
    Int i;

    StringSeq dummy;

    cout << "transforming primitives... " << flush;

  
    //
    // Transform byte to short
    //
    {
	IntByteMap map(communicator, dbEnv, "byteToShort");
	map.clear();
	for(i = 0; i < NUM_KEYS; i++)
	{
	    map.put(IntByteMap::value_type(i, i));
	}
    }
    
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "byteToShort", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE); 
	emitSchemas("xs:byte", "xs:short");
	{
	    XMLTransform::DBTransformer transformer(dbEnv, db, dummy, dummy, paths, paths, false);
	    transformer.transform(oldSchema, newSchema);
	}
	db.close(0);
    }
    
    
    {
	IntShortMap map(communicator, dbEnv, "shortToInt");
	map.clear();
	for(i = 0; i < NUM_KEYS; i++)
	{
	    map.put(IntShortMap::value_type(i, i));
	}
    }
    
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "shortToInt", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE); 
	emitSchemas("xs:short", "xs:int");
	{
	    XMLTransform::DBTransformer transformer(dbEnv, db, dummy, dummy, paths, paths, false);
	    transformer.transform(oldSchema, newSchema);
	}
	db.close(0);
    }
    

    //
    // Transform int to long
    //
    
    
    {
	IntIntMap map(communicator, dbEnv, "intToLong");
	map.clear();
	for(i = 0; i < NUM_KEYS; i++)
	{
	    map.put(IntIntMap::value_type(i, i));
	}
    }
    
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "intToLong", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE); 
	
	emitSchemas("xs:int", "xs:long");
	{
	    XMLTransform::DBTransformer transformer(dbEnv, db, dummy, dummy, paths, paths, false);
	    transformer.transform(oldSchema, newSchema);
	}
	db.close(0);
    }

    //
    // Transform long to byte
    //
    {
	IntLongMap map(communicator, dbEnv, "longToByte");
	map.clear();
	for(i = 0; i < NUM_KEYS; i++)
	{
	    map.put(IntLongMap::value_type(i, i));
	}
    }
    
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "longToByte", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE); 
	
	emitSchemas("xs:long", "xs:byte");
	{
	    XMLTransform::DBTransformer transformer(dbEnv, db, dummy, dummy, paths, paths, false);
	    transformer.transform(oldSchema, newSchema);
	}
	db.close(0);
    }
    
    
    //
    // Transform float to double
    //
    {
	IntFloatMap map(communicator, dbEnv, "floatToDouble");
	map.clear();
	for(i = 0; i < NUM_KEYS; i++)
	{
	    map.put(IntFloatMap::value_type(i, static_cast<float>(i)));
	}
    }
    
    
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "floatToDouble", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE); 
	emitSchemas("xs:float", "xs:double");
	{
	    XMLTransform::DBTransformer transformer(dbEnv, db, dummy, dummy, paths, paths, false);
	    transformer.transform(oldSchema, newSchema);
	}
	db.close(0);
    }
    
    
    //
    // Transform long to byte (should fail)
    //
    
    {
	IntLongMap map(communicator, dbEnv, "failure");
	map.clear();
	Long l;
	l = SCHAR_MIN;
	map.put(IntLongMap::value_type(0, l));
	map.put(IntLongMap::value_type(1, l - 1)); // Out of range for byte.
	l = SCHAR_MAX;
	map.put(IntLongMap::value_type(2, l));
	map.put(IntLongMap::value_type(3, l + 1)); // Out of range for byte.
    }
    
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "failure", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE); 
	emitSchemas("xs:long", "xs:byte");
	try
	{
	    XMLTransform::DBTransformer transformer(dbEnv, db, dummy, dummy, paths, paths, false);
	    transformer.transform(oldSchema, newSchema);
	    test(false);
	}
	catch(const XMLTransform::IllegalTransform&)
	{
	    // Expected.
	}
	db.close(0);
    }
    
    //
    // Transform long to short (should fail)
    //
    {
	IntLongMap map(communicator, dbEnv, "failure");
	map.clear();
	Long l;
	l = SHRT_MIN;
	map.put(IntLongMap::value_type(0, l));
	map.put(IntLongMap::value_type(1, l - 1)); // Out of range for short.
	l = SHRT_MAX;
	map.put(IntLongMap::value_type(2, l));
	map.put(IntLongMap::value_type(3, l + 1)); // Out of range for short.
    }
    
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "failure", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE); 
	emitSchemas("xs:long", "xs:short");
	try
	{
	    XMLTransform::DBTransformer transformer(dbEnv, db, dummy, dummy, paths, paths, false);
	    transformer.transform(oldSchema, newSchema);
	    test(false);
	}
	catch(const XMLTransform::IllegalTransform&)
	{
	    // Expected.
	}
	    db.close(0);
    }
    
    //
    // Transform long to int (should fail)
    //
    
    {
	IntLongMap map(communicator, dbEnv, "failure");
	map.clear();
	Long l;
	l = INT_MIN;
	map.put(IntLongMap::value_type(0, l));
	map.put(IntLongMap::value_type(1, l - 1)); // Out of range for int.
	l = INT_MAX;
	map.put(IntLongMap::value_type(2, l));
	map.put(IntLongMap::value_type(3, l + 1)); // Out of range for int.
    }
    
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "failure", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE); 
	emitSchemas("xs:long", "xs:int");
	try
	{
	    XMLTransform::DBTransformer transformer(dbEnv, db, dummy, dummy, paths, paths, false);
	    transformer.transform(oldSchema, newSchema);
	    test(false);
	}
	catch(const XMLTransform::IllegalTransform&)
	{
	    // Expected.
	}
	db.close(0);
    }
    
    cout << "ok" << endl;
       
}

static void
transformPrimitiveSequence(const CommunicatorPtr& communicator, DbEnv& dbEnv)
{
    Int i;

    StringSeq dummy;

    cout << "transforming primitive sequences... " << flush;

   
    //
    // Transform byte to short sequence
    //

    {
	IntSeq1Map map(communicator, dbEnv, "byteToShortSeq");
	map.clear();
	Test::Seq1 seq;
	for(i = 0; i < NUM_ELEMENTS; i++)
	{
	    seq.push_back(i);
	}
	map.put(IntSeq1Map::value_type(0, seq));
    }
   
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "byteToShortSeq", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE); 
        emitSchemas("tns:_internal.Test.Seq1Type", "tns:_internal.Test.Seq1Type");
        {
            XMLTransform::DBTransformer transformer(dbEnv, db, dummy, dummy, paths, paths, false);
            transformer.transform(oldSchema, newSchema);
        }
	db.close(0);
    }

       
    {
	IntSeq2Map map(communicator, dbEnv, "shortToIntSeq");
	map.clear();
	Test::Seq2 seq;
	for(i = 0; i < NUM_ELEMENTS; i++)
	{
	    seq.push_back(i);
	}
	map.put(IntSeq2Map::value_type(0, seq));
    }
       
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "shortToIntSeq", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE); 
        emitSchemas("tns:_internal.Test.Seq2Type", "tns:_internal.Test.Seq2Type");
        {
            XMLTransform::DBTransformer transformer(dbEnv, db, dummy, dummy, paths, paths, false);
            transformer.transform(oldSchema, newSchema);
        }
	db.close(0);
    }

    //
    // Transform int to long sequence
    //
    {
	IntSeq3Map map(communicator, dbEnv, "intToLongSeq");
	map.clear();
	Test::Seq3 seq;
	for(i = 0; i < NUM_ELEMENTS; i++)
	{
	    seq.push_back(i);
	}
	map.put(IntSeq3Map::value_type(0, seq));
    }
   
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "intToLongSeq", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE); 
        emitSchemas("tns:_internal.Test.Seq3Type", "tns:_internal.Test.Seq3Type");
        {
            XMLTransform::DBTransformer transformer(dbEnv, db, dummy, dummy, paths, paths, false);
            transformer.transform(oldSchema, newSchema);
        }
	db.close(0);
    }

    //
    // Transform long to byte sequence
    //
    {
	
	IntSeq4Map map(communicator, dbEnv, "longToByteSeq");
	map.clear();
	Test::Seq4 seq;
	for(i = 0; i < NUM_ELEMENTS; i++)
	{
	    seq.push_back(i);
	}
	map.put(IntSeq4Map::value_type(0, seq));
    }
    
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "longToByteSeq", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE); 
        emitSchemas("tns:_internal.Test.Seq4Type", "tns:_internal.Test.Seq4Type");
        {
            XMLTransform::DBTransformer transformer(dbEnv, db, dummy, dummy, paths, paths, false);
            transformer.transform(oldSchema, newSchema);
        }
        db.close(0);
    }

        //
        // Transform long to byte sequence (should fail)
        //
      
    {
	IntSeq4Map map(communicator, dbEnv, "failure");
	map.clear();
	Test::Seq4 seq;
	Long l;
	l = SCHAR_MIN;
	seq.push_back(l);
	seq.push_back(l - 1); // Out of range for byte.
	l = SCHAR_MAX;
	seq.push_back(l);
	seq.push_back(l + 1); // Out of range for byte.
	map.put(IntSeq4Map::value_type(0, seq));
    }
    
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "failure", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE); 
        emitSchemas("tns:_internal.Test.Seq4Type", "tns:_internal.Test.Seq4Type");
        try
        {
            XMLTransform::DBTransformer transformer(dbEnv, db, dummy, dummy, paths, paths, false);
            transformer.transform(oldSchema, newSchema);
            test(false);
        }
        catch(const XMLTransform::IllegalTransform&)
        {
            // Expected.
        }
	
        db.close(0);
    }

    cout << "ok" << endl;
}

static void
transformEnum(const CommunicatorPtr& communicator, DbEnv& dbEnv)
{
  
    StringSeq dummy;

    cout << "transforming enumerations... " << flush;

    //
    // Don't use E1::three, which is removed in new schema
    //
   
    {
	IntE1Map map(communicator, dbEnv, "enum");
	map.clear();
	map.put(IntE1Map::value_type(0, Test::one));
	map.put(IntE1Map::value_type(1, Test::two));
    }
    
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "enum", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE);
        emitSchemas("tns:_internal.Test.E1Type", "tns:_internal.Test.E1Type");
        {
            XMLTransform::DBTransformer transformer(dbEnv, db, dummy, dummy, paths, paths, false);
            transformer.transform(oldSchema, newSchema);
        }
        db.close(0);
    }
        

        //
        // Use E1::three, which is removed in new schema (should fail)
        //
      
    {
	IntE1Map map(communicator, dbEnv, "failure");
	map.clear();
	map.put(IntE1Map::value_type(0, Test::one));
	map.put(IntE1Map::value_type(1, Test::two));
	map.put(IntE1Map::value_type(2, Test::three));
    }
    
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "failure", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE);

	emitSchemas("tns:_internal.Test.E1Type", "tns:_internal.Test.E1Type");
        try
        {
            XMLTransform::DBTransformer transformer(dbEnv, db, dummy, dummy, paths, paths, false);
            transformer.transform(oldSchema, newSchema);
            test(false);
        }
        catch(const XMLTransform::IllegalTransform&)
        {
            // Expected.
        }
        db.close(0);
    }

    cout << "ok" << endl;
}

static void
transformDictionary(const CommunicatorPtr& communicator, DbEnv& dbEnv)
{
    StringSeq dummy;

    cout << "transforming dictionaries... " << flush;

   
    //
    // Don't use E1::three, which is removed in new schema
    //
   
    {
	IntD1Map map(communicator, dbEnv, "dict");
	map.clear();
	Test::D1 dict;
	dict.insert(make_pair(string("one"), Test::one));
	dict.insert(make_pair(string("two"), Test::two));
	map.put(IntD1Map::value_type(0, dict));
    }
   
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "dict", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE);
        emitSchemas("tns:_internal.Test.D1Type", "tns:_internal.Test.D1Type");
        {
            XMLTransform::DBTransformer transformer(dbEnv, db, dummy, dummy, paths, paths, false);
            transformer.transform(oldSchema, newSchema);
        }
        db.close(0);
    }
        

    //
    // Use E1::three, which is removed in new schema (should fail)
    //

    {
	IntD1Map map(communicator, dbEnv, "failure");
	map.clear();
	Test::D1 dict;
	dict.insert(make_pair(string("one"), Test::one));
	dict.insert(make_pair(string("two"), Test::two));
	dict.insert(make_pair(string("three"), Test::three));
	map.put(IntD1Map::value_type(0, dict));
    }
  
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "failure", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE);
        emitSchemas("tns:_internal.Test.D1Type", "tns:_internal.Test.D1Type");
        try
        {
            XMLTransform::DBTransformer transformer(dbEnv, db, dummy, dummy, paths, paths, false);
            transformer.transform(oldSchema, newSchema);
            test(false);
        }
        catch(const XMLTransform::IllegalTransform&)
        {
            // Expected.
        }
        db.close(0);
    }
        
    cout << "ok" << endl;
}

static void
transformStruct(const CommunicatorPtr& communicator, DbEnv& dbEnv)
{
    StringSeq dummy;

    cout << "transforming structs... " << flush;
  
    {
	IntS1Map map(communicator, dbEnv, "struct");
	map.clear();
	Test::S1 s1;
	s1.b = false;
	s1.i = 0;
	map.put(IntS1Map::value_type(0, s1));
	s1.b = true;
	s1.i = 1;
	map.put(IntS1Map::value_type(1, s1));
	s1.b = true;
	s1.i = 2;
	map.put(IntS1Map::value_type(2, s1));
    }
   
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "struct", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE);
	emitSchemas("tns:_internal.Test.S1Type", "tns:_internal.Test.S1Type");
	{
	    XMLTransform::DBTransformer transformer(dbEnv, db, dummy, dummy, paths, paths, false);
	    transformer.transform(oldSchema, newSchema);
	}
	db.close(0);
    }
        

    //
    // Transform S1 (should fail)
    //
   
    {
	IntS1Map map(communicator, dbEnv, "failure");
	map.clear();
	Test::S1 s1;
	s1.b = false;
	s1.i = SCHAR_MIN;
	map.put(IntS1Map::value_type(0, s1));
	s1.b = true;
	s1.i = SCHAR_MAX;
	map.put(IntS1Map::value_type(1, s1));
	s1.b = true;
	s1.i = ((Int)SCHAR_MAX) + 1; // Out of range for byte
	map.put(IntS1Map::value_type(2, s1));
    }
   
    {
        Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "failure", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE);
        emitSchemas("tns:_internal.Test.S1Type", "tns:_internal.Test.S1Type");
        try
        {
            XMLTransform::DBTransformer transformer(dbEnv, db, dummy, dummy, paths, paths, false);
            transformer.transform(oldSchema, newSchema);
            test(false);
        }
        catch(const XMLTransform::IllegalTransform&)
        {
            // Expected.
        }
        db.close(0);
    }
        

    //
    // Make sure nothing changed.
    //
    {
	IntS1Map map(communicator, dbEnv, "failure");
	for(IntS1Map::iterator p = map.begin(); p != map.end(); ++p)
	{
	    Test::S1 s1 = p->second;
	    s1.b = false;
	}
    }
  
    cout << "ok" << endl;
}

static void
transformClass(const CommunicatorPtr& communicator, DbEnv& dbEnv)
{
    StringSeq dummy;

    cout << "transforming classes... " << flush;

    //
    // Transform C2
    //
  
    {
	IntC1Map map(communicator, dbEnv, "class");
	map.clear();
	Test::C2Ptr c2;
	c2 = new Test::C2;
	c2->s = "0";
	c2->f = 0;
	c2->b = 0;
	c2->i = 0;
	c2->l = 0;
	c2->d = 0;
	map.put(IntC1Map::value_type(0, c2));
	c2 = new Test::C2;
	c2->s = "1";
	c2->f = 1;
	c2->b = 1;
	c2->i = 1;
	c2->l = 1;
	c2->d = 1;
	map.put(IntC1Map::value_type(1, c2));
	c2 = new Test::C2;
	c2->s = "2";
	c2->f = 2;
	c2->b = 2;
	c2->i = 2;
	c2->l = 2;
	c2->d = 2;
	map.put(IntC1Map::value_type(2, c2));
	
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
	map.put(IntC1Map::value_type(3, c2));
    }
   
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "class", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE);
	emitSchemas("tns:_internal.Test.C1Type", "tns:_internal.Test.C1Type");
	{
	    XMLTransform::DBTransformer transformer(dbEnv, db, dummy, dummy, paths, paths, false);
	    transformer.transform(oldSchema, newSchema);
	}
	db.close(0);
    }
    
    
    //
    // Transform C2 (should fail)
    //
   
    {
	IntC1Map map(communicator, dbEnv, "failure");
	map.clear();
	Test::C2Ptr c2;
	c2 = new Test::C2;
	c2->s = "0";
	c2->f = 0;
	c2->b = 0;
	c2->i = 0;
	c2->l = INT_MIN;
	c2->d = 0;
	map.put(IntC1Map::value_type(0, c2));
	c2 = new Test::C2;
	c2->s = "1";
	c2->f = 1;
	c2->b = 1;
	c2->i = 1;
	c2->l = INT_MAX;
	c2->d = 1;
	map.put(IntC1Map::value_type(1, c2));
	c2 = new Test::C2;
	c2->s = "2";
	c2->f = 2;
	c2->b = 2;
	c2->i = 2;
	c2->l = INT_MAX;
	c2->l++; // Out of range for int
	c2->d = 2;
	map.put(IntC1Map::value_type(2, c2));
    }
   
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "failure", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE);
	emitSchemas("tns:_internal.Test.C1Type", "tns:_internal.Test.C1Type");
	try
	{
	    XMLTransform::DBTransformer transformer(dbEnv, db, dummy, dummy, paths, paths, false);
	    transformer.transform(oldSchema, newSchema);
	    test(false);
	}
	catch(const XMLTransform::IllegalTransform&)
	{
	    // Expected.
	}
	db.close(0);
    }
    
    {
	IntC1Map map(communicator, dbEnv, "failure");
	for(IntC1Map::iterator p = map.begin(); p != map.end(); ++p)
	{
	    Test::C1Ptr c1 = p->second;
	}
    }
    
    cout << "ok" << endl;
}

static void
transformEvictor(const CommunicatorPtr& communicator, DbEnv& dbEnv)
{

    StringSeq loadOld, loadNew;
    Identity ident;
    Freeze::EvictorPtr evictor;

    loadOld.push_back("TestOld.xsd");
    loadOld.push_back("Identity.xsd");
    loadOld.push_back("ObjectRecord.xsd");
    loadNew.push_back("TestNew.xsd");
    loadNew.push_back("Identity.xsd");
    loadNew.push_back("ObjectRecord.xsd");

    static string evictorSchema =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<xs:schema xmlns:xs=\"http://www.w3.org/2002/XMLSchema\""
        "           elementFormDefault=\"qualified\""
        "           xmlns:tns=\"http://www.noorg.org/schemas\""
        "           xmlns:ice=\"http://www.zeroc.com/schemas\""
        "           targetNamespace=\"http://www.noorg.org/schemas\">"
        "    <xs:import namespace=\"http://www.zeroc.com/schemas\" schemaLocation=\"ice.xsd\"/>"
        "    <xs:element name=\"Key\" type=\"_internal.Ice.IdentityType\"/>"
        "    <xs:element name=\"Value\" type=\"_internal.Freeze.ObjectRecordType\"/></xs:schema>";

    cout << "transforming evictor map... " << flush;

  
    //
    // Transform C2
    //
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD | DB_TRUNCATE | DB_CREATE;
	db.open(0, "evictor", 0, DB_BTREE, flags, FREEZE_DB_MODE);
	db.close(0);
    }
    
    evictor = Freeze::createEvictor(communicator, dbEnv, "evictor");
    
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
    evictor->deactivate("");
    
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "evictor", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE);
	{
	    XMLTransform::DBTransformer transformer(dbEnv, db, loadOld, loadNew, paths, paths, false);
	    transformer.transform(evictorSchema);
	}
	db.close(0);
    }
    
    
    //
    // Transform C2 (should fail)
    //
    
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD | DB_TRUNCATE | DB_CREATE;
	db.open(0, "failure", 0, DB_BTREE, flags, FREEZE_DB_MODE);
	db.close(0);
    }
    
    evictor = Freeze::createEvictor(communicator, dbEnv, "failure");
    
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
    evictor->deactivate("");
    
    {
	Db db(&dbEnv, 0);
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	db.open(0, "failure", 0, DB_UNKNOWN, flags, FREEZE_DB_MODE);
	try
	{
	    XMLTransform::DBTransformer transformer(dbEnv, db, loadOld, loadNew, paths, paths, false);
	    transformer.transform(evictorSchema);
	    test(false);
	}
	catch(const XMLTransform::IllegalTransform&)
	{
	    // Expected.
	}
	db.close(0);
    }
    
    cout << "ok" << endl;    
}

static int
run(int argc, char* argv[], const CommunicatorPtr& communicator)
{
    string envName = "db";

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

    DbEnv dbEnv(0);
    dbEnv.set_flags(DB_TXN_NOSYNC, true);

    u_int32_t flags = DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN |
	DB_PRIVATE | DB_THREAD | DB_RECOVER | DB_CREATE;
    
    dbEnv.open(envName.c_str(), flags, FREEZE_DB_MODE);

    transformPrimitive(communicator, dbEnv);
    transformPrimitiveSequence(communicator, dbEnv);
    transformEnum(communicator, dbEnv);
    transformDictionary(communicator, dbEnv);
    transformStruct(communicator,dbEnv);
    transformClass(communicator, dbEnv);
    transformEvictor(communicator, dbEnv);

    dbEnv.close(0);
    
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
    catch(const DbException& ex)
    {
	cerr << ex.what() << endl;
	status = EXIT_FAILURE;
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
