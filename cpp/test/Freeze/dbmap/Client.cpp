// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include <TestCommon.h>
#include <ByteIntMapXML.h>
#include <ByteIntMapBinary.h>

#include <algorithm>

using namespace std;
using namespace Ice;
using namespace Freeze;

static Byte alphabetChars[] = "abcdefghijklmnopqrstuvwxyz";
vector<Byte> alphabet;

static void
ForEachTest(const pair<Byte, Int>&)
{
}

static bool
FindIfTest(const pair<Byte, Int>& p)
{
    return p.first == 'b';
}

static bool
FindFirstOfTest(const pair<Byte, Int>& p, Byte q)
{
    return p.first == q;
}

template<class MAP>
static void
populateDB(MAP& m)
{
    alphabet.assign(alphabetChars, alphabetChars + sizeof(alphabetChars) - 1);

    for(vector<Byte>::const_iterator j = alphabet.begin(); j != alphabet.end(); ++j)
    {
	m.insert(make_pair(*j, j-alphabet.begin()));
    }
}

template<class MAP>
class StressThread : public IceUtil::Thread
{
public:

    StressThread(MAP& m) :
	_map(m)
    {
    }

    virtual void
    run()
    {
	for(int i = 0; i < 50; ++i)
	{
	    typename MAP::iterator p = _map.begin();
	    assert(p != _map.end());
	    Byte b = p->second;
	    test(b >= 0);
	}
    }

private:

    MAP& _map;
};

template<class MAP>
static int
run(int argc, char* argv[], MAP& m)
{
    //
    // Populate the database with the alphabet
    //
    populateDB(m);

    vector<Byte>::const_iterator j;
    typename MAP::iterator p;
    typename MAP::iterator cp;

    cout << "  testing populate... ";
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

    cout << "  testing map::find... ";
    j = find(alphabet.begin(), alphabet.end(), 'n');
    
    cp = m.find(*j);
    test(cp != m.end());
    test(cp->first == 'n' && cp->second == j - alphabet.begin());
    cout << "ok" << endl;

    cout << "  testing erase... ";

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
    cout << "  testing map::find (again)... ";
    cp = m.find('a');
    test(cp == m.end());
    cout << "ok" << endl;

    cout << "  testing iterators... ";
    p = m.begin();
    typename MAP::iterator p2 = p;

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
    pair<Byte, Int> data = *p;
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
    cout << "  testing iterator.set... ";

    p = m.find('d');
    test(p != m.end() && p->second == 3);

    test(m.find('a') == m.end());
    typename MAP::value_type i1('a', 1);
    m.insert(i1);
    //
    // Note: VC++ won't accept this
    //
    //m.insert(typename MAP::value_type('a', 1));

    p = m.find('a');
    test(p != m.end() && p->second == 1);

    typename MAP::value_type i2('a', 0);
    m.insert(i2);
    //
    // Note: VC++ won't accept this
    //
    //m.insert(typename MAP::value_type('a', 0));
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

    cout << "  testing algorithms... ";

    for_each(m.begin(), m.end(), ForEachTest);

    //
    // Inefficient, but this is just a test. Ensure that both forms of
    // operator== & != are tested.
    //
    typename MAP::value_type toFind('n', 13);
    
    p = find(m.begin(), m.end(), toFind);
    test(p != m.end());
    test(*p == toFind);
    test(toFind == *p);
    test(!(*p != toFind));
    test(!(toFind != *p));

    p = find_if(m.begin(), m.end(), FindIfTest);
    test(p->first == 'b');

    //
    // find_first_of. First construct a vector of pairs (n, o, p,
    // q). The test must find one of the types (it doesn't matter
    // which since the container doesn't have to maintain sorted
    // order).
    //
    j = find(alphabet.begin(), alphabet.end(), 'n');
    vector< pair <Byte, Int> > pairs;
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
    p = find_first_of(m.begin(), m.end(), j, j + 4, FindFirstOfTest);
    test(p != m.end());
    test(p->first == 'n' || p->first == 'o' || p->first == 'p' || p->first == 'q');

    pairs.clear();
    copy(m.begin(), m.end(), back_inserter(pairs));
    test(pairs.size() == m.size());

    vector<pair<Byte, Int> >::const_iterator pit;
    for(pit = pairs.begin(); pit != pairs.end(); ++pit)
    {
	p = m.find(pit->first);
	test(p != m.end());
    }
    cout << "ok" << endl;

    cout << "  testing concurrent access... " << flush;
    vector<IceUtil::ThreadControl> controls;
    for(int i = 0; i < 10; ++i)
    {
	IceUtil::ThreadPtr t = new StressThread<MAP>(m);
	controls.push_back(t->start());
    }
    for(vector<IceUtil::ThreadControl>::iterator q = controls.begin(); q != controls.end(); ++q)
    {
	q->join();
    }
    cout << "ok" << endl;

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;
    DBEnvironmentPtr dbEnv;
    string dbEnvDir = "db";
    DBPtr xmlDB, binaryDB;

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
	xmlDB = dbEnv->openDB("xml", true);
        ByteIntMapXML xml(xmlDB);
        cout << "testing XML encoding..." << endl;
	status = run(argc, argv, xml);
        if(status == EXIT_SUCCESS)
        {
            binaryDB = dbEnv->openDB("binary", true);
            ByteIntMapBinary binary(binaryDB);
            cout << "testing binary encoding..." << endl;
            status = run(argc, argv, binary);
        }
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    if(xmlDB)
    {
	try
	{
	    xmlDB->close();
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
	xmlDB = 0;
    }

    if(binaryDB)
    {
	try
	{
	    binaryDB->close();
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
	binaryDB = 0;
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
