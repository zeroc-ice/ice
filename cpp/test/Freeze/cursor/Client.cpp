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
#include <Ice/BasicStream.h>
#include <TestCommon.h>

#include <algorithm>

using namespace std;
using namespace Ice;
using namespace Freeze;

class KeyCodec
{
public:

    typedef char value_type;

    static Freeze::Key
    write(const char& key, const IceInternal::InstancePtr& instance)
    {
	IceInternal::BasicStream keyStream(instance.get());
	keyStream.write(key);
	return keyStream.b;
    }

    static void
    read(char& key, const Freeze::Key& bytes, const IceInternal::InstancePtr& instance)
    {
	IceInternal::BasicStream valueStream(instance.get());
	valueStream.b = bytes;
	valueStream.i = valueStream.b.begin();
	valueStream.read(key);
    }
};

class ValueCodec
{
public:

    typedef Ice::Int value_type;

    static Freeze::Value
    write(const Ice::Int& value, const IceInternal::InstancePtr& instance)
    {
	IceInternal::BasicStream valueStream(instance.get());
	valueStream.write(value);
	return valueStream.b;
    }

    static void
    read(Ice::Int& value, const Freeze::Value& bytes, const IceInternal::InstancePtr& instance)
    {
	IceInternal::BasicStream valueStream(instance.get());
	valueStream.b = bytes;
	valueStream.i = valueStream.b.begin();
	valueStream.read(value);
    }
};

static void
addValue(const DBPtr& db, char key, int value)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(db->getCommunicator());

    Freeze::Key k;
    Freeze::Value v;

    k = KeyCodec::write(key, instance);
    v = ValueCodec::write(value, instance);

    db->put(k, v);
}

static char alphabetChars[] = "abcdefghijklmnopqrstuvwxyz";
vector<char> alphabet(alphabetChars, alphabetChars + sizeof(alphabetChars)-1);

static void
populateDB(const DBPtr& db)
{
    for(vector<char>::const_iterator j = alphabet.begin() ; j != alphabet.end(); ++j)
    {
	addValue(db, *j, j-alphabet.begin());
    }
}

static void
readValue(const DBPtr& db, const Freeze::Key& k, const Freeze::Value& v, char& key, int& value)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(db->getCommunicator());

    KeyCodec::read(key, k, instance);
    ValueCodec::read(value, v, instance);
}

static int
run(int argc, char* argv[], const DBEnvironmentPtr& dbEnv)
{
    DBPtr db = dbEnv->openDB("test", true);
    IceInternal::InstancePtr instance = IceInternal::getInstance(db->getCommunicator());

    //
    // Populate the database with the alphabet
    //
    populateDB(db);

    Freeze::Key k;
    Freeze::Value v;

    char key;
    int value;

    DBCursorPtr cursor, clone;
    vector<char>::const_iterator j;

    cout << "testing populate... ";
    cursor = db->getCursor();
    j = alphabet.begin();
    try
    {
	do
	{
	    cursor->curr(k, v);
	    readValue(db, k, v, key, value);
	    test(key == *j && value == j - alphabet.begin());
	    ++j;
	}
	while(cursor->next());
    }
    catch(const DBNotFoundException&)
    {
	test(false);
    }
    cursor->close();
    cout << "ok" << endl;

    cout << "testing contains... ";
    try
    {
	for(j = alphabet.begin(); j != alphabet.end(); ++j)
	{
	    k = KeyCodec::write(*j, instance);
	    test(db->contains(k));
	}
    }
    catch(const DBException&)
    {
	test(false);
    }
    k = KeyCodec::write('0', instance);
    test(!db->contains(k));
    cout << "ok" << endl;

    cout << "testing DB::getCursorAtKey... ";
    k = KeyCodec::write('n', instance);
    j = find(alphabet.begin(), alphabet.end(), 'n');
    cursor = db->getCursorAtKey(k);
    try
    {
	do
	{
	    cursor->curr(k, v);
	    readValue(db, k, v, key, value);
	    test(key == *j && value == j - alphabet.begin());
	    ++j;
	}
	while(cursor->next());
    }
    catch(const DBNotFoundException&)
    {
	test(false);
    }
    cursor->close();
    cout << "ok" << endl;

    cout << "testing remove... ";
    cursor = db->getCursor();
    j = alphabet.begin();

    try
    {
	do
	{
	    cursor->curr(k, v);
	    readValue(db, k, v, key, value);
	    test(key == *j && value == j - alphabet.begin());
	    cursor->del();
	    ++j;
	    if(key == 'c')
		break;
	}
	while(cursor->next());
    }
    catch(const DBNotFoundException&)
    {
	test(false);
    }
    cursor->close();

    cursor = db->getCursor();
    j = find(alphabet.begin(), alphabet.end(), 'd');
    try
    {
	do
	{
	    cursor->curr(k, v);
	    readValue(db, k, v, key, value);
	    test(key == *j && value == j - alphabet.begin());
	    ++j;
	}
	while(cursor->next());
    }
    catch(const DBNotFoundException&)
    {
    }
    cursor->close();
    cout << "ok" << endl;

    //
    // Get a cursor for the deleted element - this should fail.
    //
    cout << "testing DB::getCursorAtKey (again)... ";
    try
    {
	k = KeyCodec::write('a', instance);
	cursor = db->getCursorAtKey(k);
	test(false);
     }
    catch(const DBNotFoundException&)
    {
	// Ignore
    }
    cout << "ok" << endl;

    cout << "testing clone... ";
    cursor = db->getCursor();
    clone = cursor->clone();

    //
    // Verify both cursors point at 'd'
    //
    cursor->curr(k, v);
    readValue(db, k, v, key, value);
    test(key == 'd' && value == 3);

    clone->curr(k, v);
    readValue(db, k, v, key, value);
    test(key == 'd' && value == 3);

    cursor->close();
    clone->close();

    //
    // Create cursor that points at 'n'
    //
    k = KeyCodec::write('n', instance);
    cursor = db->getCursorAtKey(k);
    clone = cursor->clone();

    //
    // Verify both cursors point at 'n'
    //
    cursor->curr(k, v);
    readValue(db, k, v, key, value);
    test(key == 'n' && value == 13);

    clone->curr(k, v);
    readValue(db, k, v, key, value);
    test(key == 'n' && value == 13);

    cursor->close();
    clone->close();

    //
    // Create cursor that points at 'n'
    //
    k = KeyCodec::write('n', instance);
    cursor = db->getCursorAtKey(k);
    cursor->curr(k, v);
    cursor->next();
    readValue(db, k, v, key, value);
    test(key == 'n' && value == 13);

    clone = cursor->clone();

    //
    // Verify cloned cursors are independent
    //
    cursor->curr(k, v);
    cursor->next();
    readValue(db, k, v, key, value);
    test(key == 'o' && value == 14);

    cursor->curr(k, v);
    cursor->next();
    readValue(db, k, v, key, value);
    test(key == 'p' && value == 15);

    clone->curr(k, v);
    clone->next();
    readValue(db, k, v, key, value);
    test(key == 'o' && value == 14);

    cursor->close();
    clone->close();
    
    cout << "ok" << endl;

    db->close();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;
    DBEnvironmentPtr dbEnv;
    string dbEnvDir = "db";

    try
    {
	communicator = Ice::initialize(argc, argv);
	if(argc != 1)
	{
	    dbEnvDir = argv[1];
	    dbEnvDir += "/";
	    dbEnvDir += "db";
	}
	dbEnv = Freeze::initialize(communicator, dbEnvDir);
	status = run(argc, argv, dbEnv);
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    if(dbEnv)
    {
	try
	{
	    dbEnv->close();
	}
	catch(const DBException& ex)
	{
	    cerr << argv[0] << ": " << ex << ": " << ex.message << endl;
	    status = EXIT_FAILURE;
	}
	catch(const Exception& ex)
	{
	    cerr << argv[0] << ": " << ex << endl;
	    status = EXIT_FAILURE;
	}
	catch(...)
	{
	    cerr << argv[0] << ": unknown exception" << endl;
	    status = EXIT_FAILURE;
	}
	dbEnv = 0;
    }

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
