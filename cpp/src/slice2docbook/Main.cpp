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
    cerr << "Usage: " << n << " [options] slice-files ...\n";
    cerr <<	
"Options:\n"
"-h, --help           Show this message.\n"
"-DNAME               Define NAME as 1.\n"
"-DNAME=DEF           Define NAME as DEF.\n"
"-UNAME               Remove any definition for NAME.\n"
"-IDIR                Put DIR in the include file search path.\n"
"-d, --debug          Print debug messages.\n"
	;
}

int
main(int argc, char* argv[])
{
    string cpp("cpp -C");
    vector<string> includePaths;
    bool debug = false;

    int idx = 1;
    while(idx < argc)
    {
	if(strncmp(argv[idx], "-I", 2) == 0)
	{
	    cpp += ' ';
	    cpp += argv[idx];

	    string path = argv[idx] + 2;
	    if(path.length())
		includePaths.push_back(path);

	    for(int i = idx ; i + 1 < argc ; ++i)
		argv[i] = argv[i + 1];
	    --argc;
	}
	else if(strncmp(argv[idx], "-D", 2) == 0 ||
		strncmp(argv[idx], "-U", 2) == 0)
	{
	    cpp += ' ';
	    cpp += argv[idx];

	    for(int i = idx ; i + 1 < argc ; ++i)
		argv[i] = argv[i + 1];
	    --argc;
	}
	else if(strcmp(argv[idx], "-h") == 0 ||
		strcmp(argv[idx], "--help") == 0)
	{
	    usage(argv[0]);
	    return EXIT_SUCCESS;
	}
	else if(strcmp(argv[idx], "-d") == 0 ||
		strcmp(argv[idx], "--debug") == 0)
	{
	    debug = true;
	    for(int i = idx ; i + 1 < argc ; ++i)
		argv[i] = argv[i + 1];
	    --argc;
	}
	else if(argv[idx][0] == '-')
	{
	    cerr << argv[0] << ": error: unknown option `" << argv[idx] << "'"
		 << endl;
	    usage(argv[0]);
	    return EXIT_FAILURE;
	}
	else
	    ++idx;
    }

    if(argc < 2)
    {
	cerr << argv[0] << ": error: no input file" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    Unit_ptr unit = Unit::createUnit(true, true);

    int status = EXIT_SUCCESS;

    for(idx = 1 ; idx < argc ; ++idx)
    {
	string base(argv[idx]);
	string suffix;
	string::size_type pos = base.rfind('.');
	if(pos != string::npos)
	{
	    suffix = base.substr(pos);
	    transform(suffix.begin(), suffix.end(), suffix.begin(), tolower);
	}
	if(suffix != ".ice")
	{
	    cerr << argv[0] << ": input files must end with `.ice'"
		 << endl;
	    unit -> destroy();
	    return EXIT_FAILURE;
	}
	base.erase(pos);

	ifstream test(argv[idx]);
	if(!test)
	{
	    cerr << argv[0] << ": can't open `" << argv[idx]
		 << "' for reading: " << strerror(errno) << endl;
	    unit -> destroy();
	    return EXIT_FAILURE;
	}
	test.close();

	string cmd = cpp + " " + argv[idx];
#ifdef WIN32
	FILE* cppHandle = _popen(cmd.c_str(), "r");
#else
	FILE* cppHandle = popen(cmd.c_str(), "r");
#endif
	if(cppHandle == NULL)
	{
	    cerr << argv[0] << ": can't run C++ preprocessor: "
		 << strerror(errno) << endl;
	    unit -> destroy();
	    return EXIT_FAILURE;
	}
	
	int parseStatus = unit -> parse(cppHandle, debug);
	if(parseStatus == EXIT_FAILURE)
	{
	    status = EXIT_FAILURE;
	}
	
#ifdef WIN32
	_pclose(cppHandle);
#else
	pclose(cppHandle);
#endif
	
    }

    if(status == EXIT_SUCCESS)
    {
	Gen gen(argv[0]);
	if(!gen)
	{
	    unit -> destroy();
	    return EXIT_FAILURE;
	}
	gen.generate(unit);
    }
    
    unit -> destroy();

    return status;
}
