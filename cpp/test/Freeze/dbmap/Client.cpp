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
#include <IceXML/StreamI.h>
#include <TestCommon.h>

#include <algorithm>

using namespace std;
using namespace Ice;
using namespace Freeze;

class KeyCodec
{
public:

    typedef char value_type;

    static void
    write(const char& key, Freeze::Key& bytes, const ::Ice::CommunicatorPtr& communicator)
    {
	ostringstream os;
	os << "<data>";
	Ice::StreamPtr stream = new ::IceXML::StreamI(communicator, os);
	stream->writeByte("Key", key);
	os << "</data>";
	bytes.resize(os.str().size());
	memcpy(&bytes[0], os.str().data(), os.str().size());
    }

    static void
    read(char& key, const Freeze::Key& bytes, const ::Ice::CommunicatorPtr& communicator)
    {
	//
	// COMPILERFIX: string data(bytes.begin(), bytes.end());
	//
	// This won't link with STLport 4.5.3 stldebug version.
	//
	string data;
	data.assign(&bytes[0], bytes.size());

	istringstream is(data);
	Ice::StreamPtr stream = new ::IceXML::StreamI(communicator, is, false);
	key = stream->readByte("Key");
    }
};

class ValueCodec
{
public:

    typedef Ice::Int value_type;

    static void
    write(const Ice::Int& value, Freeze::Value& bytes, const ::Ice::CommunicatorPtr& communicator)
    {
	ostringstream os;
	os << "<data>";
	Ice::StreamPtr stream = new ::IceXML::StreamI(communicator, os);
	stream->writeInt("Value", value);
	os << "</data>";
	bytes.resize(os.str().size());
	memcpy(&bytes[0], os.str().data(), os.str().size());
    }

    static void
    read(Ice::Int& value, const Freeze::Value& bytes, const ::Ice::CommunicatorPtr& communicator)
    {
	//
	// COMPILERFIX: string data(bytes.begin(), bytes.end());
	//
	// This won't link with STLport 4.5.3 stldebug version.
	//
	string data;
	data.assign(&bytes[0], bytes.size());
	
	istringstream is(data);
	Ice::StreamPtr stream = new ::IceXML::StreamI(communicator, is, false);
	value = stream->readInt("Value");
    }
};

typedef DBMap<char, int, KeyCodec, ValueCodec> CharIntMap;

static char alphabetChars[] = "abcdefghijklmnopqrstuvwxyz";
vector<char> alphabet;

static void
populateDB(CharIntMap& m)
{
    alphabet.assign(alphabetChars, alphabetChars + sizeof(alphabetChars)-1);

    for (vector<char>::const_iterator j = alphabet.begin() ; j != alphabet.end(); ++j)
    {
	m.insert(make_pair(*j, j-alphabet.begin()));
    }
}

static int
run(int argc, char* argv[], const DBPtr& db)
{
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

    test(!m.empty());
    test(m.size() == alphabet.size());
    cout << "ok" << endl;

    cout << "testing map::find... ";
    j = find(alphabet.begin(), alphabet.end(), 'n');
    
    cp = m.find(*j);
    test(cp != m.end());
    test(cp->first == 'n' && cp->second == j - alphabet.begin());
    cout << "ok" << endl;

    cout << "testing erase... ";

    //
    // erase first offset characters (first offset characters is
    // important for later verification of the correct second value in
    // the map).
    //
    int offset = 3;
    vector<char> chars;
    chars.push_back('a');
    chars.push_back('b');
    chars.push_back('c');
    for (j = chars.begin(); j != chars.end(); ++j)
    {
	p = m.find(*j);
	test(p != m.end());
	m.erase(p);
	p = m.find(*j);
	test(p == m.end());
	vector<char>::iterator r = find(alphabet.begin(), alphabet.end(), *j);
	test(r != alphabet.end());
	alphabet.erase(r);
    }

    for (j = alphabet.begin() ; j != alphabet.end() ; ++j)
    {
	cp = m.find(*j);
	test(cp != m.end());
	test(cp->first == *j && cp->second == (j - alphabet.begin()) + offset);
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
    // Verify both iterators point at the same element, and that
    // element is in the map.
    //
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
    pair<char, int> data = *p;
    ++p;

    test(p->first != data.first && p->second != data.second);
    ++p;

    test(p2->first == data.first && p2->second == data.second);

    cout << "ok" << endl;

    //
    // Test writing into an iterator.
    //
    cout << "testing iterator.set... ";

    p = m.find('d');
    test(p != m.end() && p->second == 3);

    test(m.find('a') == m.end());
    m.insert(CharIntMap::value_type('a', 1));

    p = m.find('a');
    test(p != m.end() && p->second == 1);

    m.insert(CharIntMap::value_type('a', 0));
    p = m.find('a');
    test(p != m.end() && p->second == 0);

    p = m.find('a');
    test(p != m.end() && p->second == 0);
    p.set(1);
    test(p != m.end() && p->second == 1);
    p = m.find('a');
    test(p != m.end() && p->second == 1);
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
    vector< pair <char, int> > pairs;
    pairs.push_back(make_pair(*j, j - alphabet.begin()));
    ++j;
    pairs.push_back(make_pair(*j, j - alphabet.begin()));
    ++j;
    pairs.push_back(make_pair(*j, j - alphabet.begin()));
    ++j;
    pairs.push_back(make_pair(*j, j - alphabet.begin()));

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

    vector<pair<char, int> >::const_iterator pit;
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
	db = dbEnv->openDB("test", true);
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
