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
    cerr << "Usage: " << n << " [options] slice-file type-id\n";
    cerr <<
	"Options:\n"
	"-h, --help	      Show this message.\n"
	"-v, --version	      Display the Ice version.\n"
	"-DNAME		      Define NAME as 1.\n"
	"-DNAME=DEF	      Define NAME as DEF.\n"
	"-UNAME		      Remove any definition for NAME.\n"
	"-IDIR		      Put DIR in the include file search path.\n"
	"-d, --debug	      Print debug messages.\n"
	;
}

int
main(int argc, char* argv[])
{
    string cpp("cpp -C");
    bool debug = false;
    string include;
    string output;
    vector<string> includePaths;

    int idx = 1;
    while (idx < argc)
    {
	if (strncmp(argv[idx], "-I", 2) == 0)
	{
	    cpp += ' ';
	    cpp += argv[idx];

	    string path = argv[idx] + 2;
	    if (path.length())
	    {
		includePaths.push_back(path);
	    }

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
	else if (strcmp(argv[idx], "--include-dir") == 0)
	{
	    if (idx + 1 >= argc)
	    {
		cerr << argv[0] << ": argument expected for`" << argv[idx] << "'" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }
	    
	    include = argv[idx + 1];
	    for (int i = idx ; i + 2 < argc ; ++i)
	    {
		argv[i] = argv[i + 2];
	    }
	    argc -= 2;
	}
	else if (strcmp(argv[idx], "--output-dir") == 0)
	{
	    if (idx + 1 >= argc)
	    {
		cerr << argv[0] << ": argument expected for`" << argv[idx] << "'" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }
	    
	    output = argv[idx + 1];
	    for (int i = idx ; i + 2 < argc ; ++i)
	    {
		argv[i] = argv[i + 2];
	    }
	    argc -= 2;
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

    if (argc != 3)
    {
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    string sourceFile = argv[1];
    string typeId = argv[2];

    int status = EXIT_SUCCESS;

    string base(sourceFile);
    string suffix;
    string::size_type pos = base.rfind('.');
    if (pos != string::npos)
    {
	suffix = base.substr(pos);
	transform(suffix.begin(), suffix.end(), suffix.begin(), tolower);
    }
    if (suffix != ".ice")
    {
	cerr << argv[0] << ": input files must end with `.ice'" << endl;
	unit->destroy();
	return EXIT_FAILURE;
    }
    base.erase(pos);

    ifstream test(sourceFile.c_str());
    if (!test)
    {
	cerr << argv[0] << ": can't open `" << sourceFile << "' for reading: " << strerror(errno) << endl;
	return EXIT_FAILURE;
    }
    test.close();

    string cmd = cpp + " " + sourceFile;
#ifdef WIN32
    FILE* cppHandle = _popen(cmd.c_str(), "r");
#else
    FILE* cppHandle = popen(cmd.c_str(), "r");
#endif
    if (cppHandle == 0)
    {
	cerr << argv[0] << ": can't run C++ preprocessor: " << strerror(errno) << endl;
	unit->destroy();
	return EXIT_FAILURE;
    }
    
    UnitPtr unit = Unit::createUnit(false, false);
    int parseStatus = unit->parse(cppHandle, debug);
    
#ifdef WIN32
    _pclose(cppHandle);
#else
    pclose(cppHandle);
#endif
    
    if (parseStatus == EXIT_FAILURE)
    {
	status = EXIT_FAILURE;
    }
    else
    {
	ContainedList contained = unit->findContents(typeId);
	if (contained.empty() || contained.front()->containedType() != Contained::ContainedTypeClass)
	{
	    cerr << argv[0] << ": invalid type: " << typeId << endl;
	    status = EXIT_FAILURE;
	}
	else
	{
	    ClassDefPtr p = ClassDefPtr::dynamicCast(contained.front());
	    assert(p);
	    Gen gen(argv[0], base, include, includePaths, output, p);
	    if (!gen)
	    {
		unit->destroy();
		return EXIT_FAILURE;
	    }
	    gen.generate(unit);
	}
    }
    
    unit->destroy();

    return status;
}
