// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
	IceInternal::BasicStream keyStream(instance);
	keyStream.write(key);
	return keyStream.b;
    }

    static void
    read(char& key, const Freeze::Key& bytes, const IceInternal::InstancePtr& instance)
    {
	IceInternal::BasicStream valueStream(instance);
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
	IceInternal::BasicStream valueStream(instance);
	valueStream.write(value);
	return valueStream.b;
    }

    static void
    read(Ice::Int& value, const Freeze::Value& bytes, const IceInternal::InstancePtr& instance)
    {
	IceInternal::BasicStream valueStream(instance);
	valueStream.b = bytes;
	valueStream.i = valueStream.b.begin();
	valueStream.read(value);
    }
};

typedef DbMap<char, int, KeyCodec, ValueCodec> CharIntMap;

static char alphabetChars[] = "abcdefghijklmnopqrstuvwxyz";
vector<char> alphabet(alphabetChars, alphabetChars + sizeof(alphabetChars)-1);

static void
populateDB(CharIntMap& m)
{
    for (vector<char>::const_iterator j = alphabet.begin() ; j != alphabet.end(); ++j)
    {
	m.insert(CharIntMap::value_type(*j, j-alphabet.begin()));
    }
}

static int
run(int argc, char* argv[], const DBPtr& db)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(db->getCommunicator());

    CharIntMap m(db);

    //
    // Populate the database with the alphabet
    //
    populateDB(m);

    //char key;
    //int value;

    vector<char>::const_iterator j;
    CharIntMap::iterator p;
    CharIntMap::iterator cp;

    cout << "testing populate... ";
    //
    // First try non-const iterator
    //
    for (j = alphabet.begin() ; j != alphabet.end() ; ++j)
    {
	p = m.find(*j);
	test(p != m.end());
	test(p->first == *j && p->second == j - alphabet.begin());
    }
    //
    // Next try const iterator
    //
    for (j = alphabet.begin() ; j != alphabet.end() ; ++j)
    {
	cp = m.find(*j);
	test(cp != m.end());
	test(cp->first == *j && cp->second == j - alphabet.begin());
    }
    cout << "ok" << endl;

    cout << "testing map::find... ";
    j = find(alphabet.begin(), alphabet.end(), 'n');
    
    cp = m.find(*j);
    test(cp != m.end());
    for (; cp != m.end(); ++cp, ++j)
    {
	test(cp->first == *j && cp->second == j - alphabet.begin());
    }
    cout << "ok" << endl;

    cout << "testing erase... ";

    j = alphabet.begin();
    for (p = m.begin() ; p != m.end() ; ++j)
    {
	bool done = (p->first == 'c');
	test(p->first == *j && p->second == j - alphabet.begin());
	CharIntMap::iterator tmp = p;
	++p;
	m.erase(tmp);
	if (done)
	    break;
    }

    j = find(alphabet.begin(), alphabet.end(), 'd');
    for (cp = m.begin(); cp != m.end(); ++cp, ++j)
    {
	test(cp->first == *j && cp->second == j - alphabet.begin());
    }

    cout << "ok" << endl;

    //
    // Get a iterator for the deleted element - this should fail.
    //
    cout << "testing map::find (again)... ";
    cp = m.find('a');
    test(cp == m.end());
    cout << "ok" << endl;

    cout << "testing iterators... ";
    p = m.begin();
    CharIntMap::iterator p2 = p;

    //
    // Verify both iterators point at 'd'
    //
    test(p->first == 'd' && p->second == 3);
    test(p2->first == 'd' && p2->second == 3);

    //
    // Create iterator that points at 'n'
    //
    p = m.find('n');
    p2 = p;

    //
    // Verify both iterators point at 'n'
    //
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
    test(p->first == 'o' && p->second == 14);
    ++p;

    test(p->first == 'p' && p->second == 15);
    ++p;

    test(p2->first == 'o' && p2->second == 14);

    cout << "ok" << endl;

    cout << "testing operator[]... ";
    test(m['d'] == 3);
    test(m.find('a') == m.end());
    test(m['a'] == 0);
    test(m.find('a') != m.end());
    m['a'] = 1;
    test(m['a'] == 1);
    m['a'] = 0;
    test(m['a'] == 0);
    cout << "ok" << endl;
    
    //
    // Re-populate
    //
    populateDB(m);

    cout << "testing algorithms... ";

    void ForEachTest(const CharIntMap::value_type&);
    for_each(m.begin(), m.end(), ForEachTest);

    //
    // Inefficient, but this is just a test. Ensure that both forms of
    // operator== & != are tested.
    //
    CharIntMap::value_type toFind('n', 13);
    
    p = find(m.begin(), m.end(), toFind);
    test(p != m.end());
    test(*p == toFind);
    test(toFind == *p);
    test(!(*p != toFind));
    test(!(toFind != *p));


    bool FindIfTest(const CharIntMap::value_type&);
    p = find_if(m.begin(), m.end(), FindIfTest);
    test(p->first == 'b');

    //
    // find_first_of. First construct a vector of pairs (n, o, p,
    // q). The test must find one of the types (it doesn't matter
    // which since the container doesn't have to maintain sorted
    // order).
    //
    j = find(alphabet.begin(), alphabet.end(), 'n');
    vector<CharIntMap::value_type> pairs;
    pairs.push_back(CharIntMap::value_type(*j, j - alphabet.begin()));
    ++j;
    pairs.push_back(CharIntMap::value_type(*j, j - alphabet.begin()));
    ++j;
    pairs.push_back(CharIntMap::value_type(*j, j - alphabet.begin()));
    ++j;
    pairs.push_back(CharIntMap::value_type(*j, j - alphabet.begin()));

    p = find_first_of(m.begin(), m.end(), pairs.begin(), pairs.end());
    test(p != m.end());
    test(p->first == 'n' || p->first == 'o' || p->first == 'p' || p->first == 'q');

    j = find(alphabet.begin(), alphabet.end(), 'n');
    bool FindFirstOfTest(const CharIntMap::value_type& p, char q);
    p = find_first_of(m.begin(), m.end(), j, j + 4, FindFirstOfTest);
    test(p != m.end());
    test(p->first == 'n' || p->first == 'o' || p->first == 'p' || p->first == 'q');

    pairs.clear();
    copy(m.begin(), m.end(), back_inserter(pairs));
    test(pairs.size() == m.size());

    vector<CharIntMap::value_type>::const_iterator pit;
    for (pit = pairs.begin() ; pit != pairs.end() ; ++pit)
    {
	p = m.find(pit->first);
	test(p != m.end());
    }
    cout << "ok" << endl;

    return EXIT_SUCCESS;
}

void
ForEachTest(const CharIntMap::value_type&)
{
}

bool
FindIfTest(const CharIntMap::value_type& p)
{
    return p.first == 'b';
}

bool
FindFirstOfTest(const CharIntMap::value_type& p, char q)
{
    return p.first == q;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;
    DBEnvironmentPtr dbEnv;
    string dbEnvDir = "db";
    DBPtr db;

    try
    {
	communicator = Ice::initialize(argc, argv);
	if (argc != 1)
	{
	    dbEnvDir = argv[1];
	    dbEnvDir += "/";
	    dbEnvDir += "db";
	}
	dbEnv = Freeze::initialize(communicator, dbEnvDir);
	db = dbEnv->openDB("test");
	status = run(argc, argv, db);
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    if (db)
    {
	try
	{
	    db->close();
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
	db = 0;
    }

    if (dbEnv)
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
