// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Slice/Parser.h>
#include <Slice/OutputUtil.h>
#include <fstream>

using namespace std;
using namespace Slice;

void
usage(const char* n)
{
    cerr << "Usage: " << n << " [options] type-name file-base slice-files...\n";
    cerr <<
	"Options:\n"
	"-h, --help           Show this message.\n"
	"-v, --version        Display the Ice version.\n"
	"-DNAME               Define NAME as 1.\n"
	"-DNAME=DEF           Define NAME as DEF.\n"
	"-UNAME               Remove any definition for NAME.\n"
	"-IDIR                Put DIR in the include file search path.\n"
	"--key KEY            Use KEY as the key for the generated type.\n"
	"--value VALUE        Use VALUE as the value for the generated type.\n"
	"-d, --debug          Print debug messages.\n"
	;
}

bool
checkIdentifier(string n, string t, string s)
{
    if (s.empty() || (!isalpha(s[0]) && s[0] != '_'))
    {
	cerr << n << ": `" << t << "' is not a valid type name" << endl;
	return false;
    }
    
    for (unsigned int i = 1; i < s.size(); ++i)
    {
	if (!isalnum(s[i]) && s[i] != '_')
	{
	    cerr << n << ": `" << t << "' is not a valid type name" << endl;
	    return false;
	}
    }

    return true;
}

void
printHeader(Output& out, string t)
{
    static const char* header = 
"// **********************************************************************\n"
"//\n"
"// Copyright (c) 2001\n"
"// MutableRealms, Inc.\n"
"// Huntsville, AL, USA\n"
"//\n"
"// All Rights Reserved\n"
"//\n"
"// **********************************************************************\n"
	;

    out << header;
    out << "\n// Freeze type generated for Slice type `" << t << "'";
    out << "\n// Ice version " << ICE_STRING_VERSION;
    out << '\n';
}

int
main(int argc, char* argv[])
{
    string cpp("cpp");
    bool debug = false;
    string key;
    string value;

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
	else if (strcmp(argv[idx], "--key") == 0)
	{
	    if (idx + 1 >= argc || argv[idx + 1][0] == '-')
            {
		cerr << argv[0] << ": argument expected for`" << argv[idx] << "'" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
            }
	    
	    key = argv[idx + 1];
	    for (int i = idx ; i + 2 < argc ; ++i)
	    {
		argv[i] = argv[i + 2];
	    }
	    argc -= 2;
	}
	else if (strcmp(argv[idx], "--value") == 0)
	{
	    if (idx + 1 >= argc || argv[idx + 1][0] == '-')
            {
		cerr << argv[0] << ": argument expected for`" << argv[idx] << "'" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
            }
	    
	    value = argv[idx + 1];
	    for (int i = idx ; i + 2 < argc ; ++i)
	    {
		argv[i] = argv[i + 2];
	    }
	    argc -= 2;
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

    if (key.empty())
    {
	cerr << argv[0] << ": key must be specified" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    if (value.empty())
    {
	cerr << argv[0] << ": value must be specified" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    if (argc < 2)
    {
	cerr << argv[0] << ": no type name specified" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    string absolute(argv[2]);
    if (absolute.find("::") == 0)
    {
	absolute.erase(2);
    }

    vector<string> scope;
    string name(absolute);
    string::size_type pos;
    while ((pos == name.find("::")) != string::npos)
    {
	string s = name.substr(0, pos);
	name.erase(pos + 2);
	
	if (!checkIdentifier(argv[0], absolute, s))
	{
	    return EXIT_FAILURE;
	}
	
	scope.push_back(s);
    }

    if (!checkIdentifier(argv[0], absolute, name))
    {
	return EXIT_FAILURE;
    }

    if (argc < 3)
    {
	cerr << argv[0] << ": no file name base specified" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    string fileH = argv[2];
    fileH += ".h";
    string fileC = argv[2];
    fileH += ".cpp";

    UnitPtr unit = Unit::createUnit(true, false);

    int status = EXIT_SUCCESS;

    for (idx = 3 ; idx < argc ; ++idx)
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
	Output H;
	H.open(fileH.c_str());
	if (!H)
	{
	    cerr << argv[0] << ": can't open `" << fileH << "' for writing: " << strerror(errno) << endl;
	    unit->destroy();
	    return EXIT_FAILURE;
	}
	printHeader(H, absolute);
	
	Output C;
	C.open(fileC.c_str());
	if (!C)
	{
	    cerr << argv[0] << ": can't open `" << fileC << "' for writing: " << strerror(errno) << endl;
	    unit->destroy();
	    return EXIT_FAILURE;
	}
	printHeader(C, absolute);
	
	try
	{
	}
	catch(...)
	{
	    unit->destroy();
	    return EXIT_FAILURE;
	}
    }
    
    unit->destroy();

    return status;
}
