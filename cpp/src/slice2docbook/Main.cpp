// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Gen.h>
#include <fstream>

using namespace std;
using namespace Slice;

void
usage(const char* n)
{
    cerr << "Usage: " << n << " [options] docbook-file slice-files...\n";
    cerr <<
	"Options:\n"
	"-h, --help           Show this message.\n"
	"-v, --version        Display the Ice version.\n"
	"-DNAME               Define NAME as 1.\n"
	"-DNAME=DEF           Define NAME as DEF.\n"
	"-UNAME               Remove any definition for NAME.\n"
	"-IDIR                Put DIR in the include file search path.\n"
	"-s, --stand-alone    Create stand-alone docbook file.\n"
	"--no-globals         Don't document the global module.\n"
	"--chapter            Use \"chapter\" instead of \"section\" as\n"
	"                     top-level element.\n"
	"-d, --debug          Print debug messages.\n"
	;
}

int
main(int argc, char* argv[])
{
    string cpp("cpp -C");
    bool debug = false;
    bool standAlone = false;
    bool noGlobals = false;
    bool chapter = false;

    int idx = 1;
    while (idx < argc)
    {
	if (strncmp(argv[idx], "-I", 2) == 0)
	{
	    cpp += ' ';
	    cpp += argv[idx];

	    for (int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if (strncmp(argv[idx], "-D", 2) == 0 || strncmp(argv[idx], "-U", 2) == 0)
	{
	    cpp += ' ';
	    cpp += argv[idx];

	    for (int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if (strcmp(argv[idx], "-s") == 0 || strcmp(argv[idx], "--stand-alone") == 0)
	{
	    standAlone = true;
	    for (int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if (strcmp(argv[idx], "--no-globals") == 0)
	{
	    noGlobals = true;
	    for (int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if (strcmp(argv[idx], "--chapter") == 0)
	{
	    chapter = true;
	    for (int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if (strcmp(argv[idx], "-h") == 0 || strcmp(argv[idx], "--help") == 0)
	{
	    usage(argv[0]);
	    return EXIT_SUCCESS;
	}
	else if (strcmp(argv[idx], "-v") == 0 || strcmp(argv[idx], "--version") == 0)
	{
	    cout << ICE_STRING_VERSION << endl;
	    return EXIT_SUCCESS;
	}
	else if (strcmp(argv[idx], "-d") == 0 || strcmp(argv[idx], "--debug") == 0)
	{
	    debug = true;
	    for (int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if (argv[idx][0] == '-')
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

    if (argc < 2)
    {
	cerr << argv[0] << ": no docbook file specified" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    string docbook(argv[1]);
    string suffix;
    string::size_type pos = docbook.rfind('.');
    if (pos != string::npos)
    {
	suffix = docbook.substr(pos);
	transform(suffix.begin(), suffix.end(), suffix.begin(), tolower);
    }
    if (suffix != ".sgml")
    {
	cerr << argv[0] << ": docbook file must end with `.sgml'" << endl;
	return EXIT_FAILURE;
    }

    if (argc < 3)
    {
	cerr << argv[0] << ": no input file" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    UnitPtr unit = Unit::createUnit(true, false);

    int status = EXIT_SUCCESS;

    for (idx = 2 ; idx < argc ; ++idx)
    {
	string file(argv[idx]);
	string suffix;
	string::size_type pos = file.rfind('.');
	if (pos != string::npos)
	{
	    suffix = file.substr(pos);
	    transform(suffix.begin(), suffix.end(), suffix.begin(), tolower);
	}
	if (suffix != ".ice")
	{
	    cerr << argv[0] << ": input files must end with `.ice'" << endl;
	    unit->destroy();
	    return EXIT_FAILURE;
	}

	ifstream test(argv[idx]);
	if (!test)
	{
	    cerr << argv[0] << ": can't open `" << argv[idx] << "' for reading: " << strerror(errno) << endl;
	    unit->destroy();
	    return EXIT_FAILURE;
	}
	test.close();

	string cmd = cpp + " " + argv[idx];
#ifdef WIN32
	FILE* cppHandle = _popen(cmd.c_str(), "r");
#else
	FILE* cppHandle = popen(cmd.c_str(), "r");
#endif
	if (cppHandle == NULL)
	{
	    cerr << argv[0] << ": can't run C++ preprocessor: " << strerror(errno) << endl;
	    unit->destroy();
	    return EXIT_FAILURE;
	}
	
	int parseStatus = unit->parse(cppHandle, debug);
	if (parseStatus == EXIT_FAILURE)
	{
	    status = EXIT_FAILURE;
	}
	
#ifdef WIN32
	_pclose(cppHandle);
#else
	pclose(cppHandle);
#endif
	
    }

    if (status == EXIT_SUCCESS)
    {
	Gen gen(argv[0], docbook, standAlone, noGlobals, chapter);
	if (!gen)
	{
	    unit->destroy();
	    return EXIT_FAILURE;
	}
	gen.generate(unit);
    }
    
    unit->destroy();

    return status;
}
