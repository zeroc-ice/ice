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
#include <GenUtil.h>
#include <fstream>

using namespace std;
using namespace Slice;

struct Dict
{
    string name;
    string key;
    string value;
};

struct ToIfdef
{
    char operator()(char c)
    {
	if (!isalnum(c))
	{
	    return '_';
	}
	else
	{
	    return c;
	}
    }
};

void
usage(const char* n)
{
    cerr << "Usage: " << n << " [options] file-base slice-files...\n";
    cerr <<
	"Options:\n"
	"-h, --help            Show this message.\n"
	"-v, --version         Display the Ice version.\n"
	"-DNAME                Define NAME as 1.\n"
	"-DNAME=DEF            Define NAME as DEF.\n"
	"-UNAME                Remove any definition for NAME.\n"
	"-IDIR                 Put DIR in the include file search path.\n"
	"--include-dir DIR    Use DIR as the header include directory.\n"
	"--dll-export SYMBOL  Use SYMBOL for DLL exports.\n"
	"--dict NAME,KEY,VALUE Create a Freeze dictionary with the name NAME,\n"
	"                      using KEY as key, and VALUE as value. This\n"
	"                      option may be specified multiple times for\n"
	"                      different names. NAME may be a scoped name.\n"
	"-d, --debug           Print debug messages.\n"
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
printHeader(Output& out, const vector<Dict>& dicts)
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
    out << "\n// Ice version " << ICE_STRING_VERSION;
    out << '\n';
    out << "\n// Freeze types in this file:";
    for (vector<Dict>::const_iterator p = dicts.begin(); p != dicts.end(); ++p)
    {
	out << "\n// name=\"" << p->name << "\", key=\"" << p->key << "\", value=\"" << p->value << "\"";
    }
    out << '\n';
}

bool
writeDict(string n, UnitPtr& unit, const Dict& dict, Output& H, Output& C)
{
    string absolute = dict.name;
    if (absolute.find("::") == 0)
    {
	absolute.erase(0, 2);
    }
    string name = absolute;
    vector<string> scope;
    string::size_type pos;
    while ((pos = name.find("::")) != string::npos)
    {
	string s = name.substr(0, pos);
	name.erase(0, pos + 2);
	
	if (!checkIdentifier(n, absolute, s))
	{
	    return false;
	}
	
	scope.push_back(s);
    }
    
    if (!checkIdentifier(n, absolute, name))
    {
	return false;
    }

    TypeList keyTypes = unit->lookupType(dict.key, false);
    if (keyTypes.empty())
    {
	cerr << n << ": `" << dict.key << "' is not a valid type" << endl;
	return false;
    }
    TypePtr keyType = keyTypes.front();
    
    TypeList valueTypes = unit->lookupType(dict.value, false);
    if (valueTypes.empty())
    {
	cerr << n << ": `" << dict.value << "' is not a valid type" << endl;
	return false;
    }
    TypePtr valueType = valueTypes.front();
    
    vector<string>::const_iterator q;
    
    for (q = scope.begin(); q != scope.end(); ++q)
    {
	H << sp;
	H << nl << "namespace " << *q << nl << '{';
    }
    
    H << sp << nl << "class " << name << " : public ::IceUtil::Shared, public JTCMutex";
    H << sb;
    H.dec();
    H << sp << nl << "public:";
    H.inc();
    H << sp << nl << name << "(const ::Freeze::DBPtr&);";
    H << sp;
    H << nl << "void put(" << inputTypeToString(keyType) << ", " << inputTypeToString(valueType) << ", bool);";
    H << nl << returnTypeToString(valueType) << " get(" << inputTypeToString(keyType) << ");";
    H << nl << "void del(" << inputTypeToString(keyType) << ");";
    H.dec();
    H << sp << nl << "private:";
    H.inc();
    H << sp << nl << "::Freeze::DBPtr _db;";
    H << eb << ';';
    H << sp << nl << "typedef IceUtil::Handle<" << name << "> " << name << "Ptr;";
    
    C << sp << nl << absolute << "::" << absolute << "(const ::Freeze::DBPtr& db) :";
    C.inc();
    C << nl << "_db(db)";
    C.dec();
    C << sb;
    C << eb;
    C << sp << nl << "void" << nl << "put(" << inputTypeToString(keyType) << " key, " << inputTypeToString(valueType)
      << " value, bool txn)";
    C << sb;
    C << eb;
    C << sp << nl << returnTypeToString(valueType) << nl << "get(" << inputTypeToString(keyType) << " key)";
    C << sb;
    C << eb;
    C << sp << nl << "void" << nl << "del(" << inputTypeToString(keyType) << " key)";
    C << sb;
    C << eb;
    
    for (q = scope.begin(); q != scope.end(); ++q)
    {
	H << sp;
	H << nl << '}';
    }

    return true;
}

int
main(int argc, char* argv[])
{
    string cpp("cpp");
    string include;
    string dllExport;
    bool debug = false;
    vector<Dict> dicts;

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
	else if (strcmp(argv[idx], "--dict") == 0)
	{
	    if (idx + 1 >= argc || argv[idx + 1][0] == '-')
            {
		cerr << argv[0] << ": argument expected for`" << argv[idx] << "'" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
            }

	    string s = argv[idx + 1];
	    s.erase(remove_if(s.begin(), s.end(), isspace), s.end());
	    
	    Dict dict;

	    string::size_type pos;
	    pos = s.find(',');
	    if (pos != string::npos)
	    {
		dict.name = s.substr(0, pos);
		s.erase(0, pos + 1);
	    }
	    pos = s.find(',');
	    if (pos != string::npos)
	    {
		dict.key = s.substr(0, pos);
		s.erase(0, pos + 1);
	    }
	    dict.value = s;

	    if (dict.name.empty())
	    {
		cerr << argv[0] << ": " << argv[idx] << ": no name specified" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }

	    if (dict.key.empty())
	    {
		cerr << argv[0] << ": " << argv[idx] << ": no key specified" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }

	    if (dict.value.empty())
	    {
		cerr << argv[0] << ": " << argv[idx] << ": no value specified" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }

	    dicts.push_back(dict);

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
	else if (strcmp(argv[idx], "--dll-export") == 0)
	{
	    if (idx + 1 >= argc)
            {
		cerr << argv[0] << ": argument expected for`" << argv[idx] << "'" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
            }
	    
	    dllExport = argv[idx + 1];
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

    if (dicts.empty())
    {
	cerr << argv[0] << ": no Freeze types specified" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    if (argc < 2)
    {
	cerr << argv[0] << ": no file name base specified" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    string fileH = argv[1];
    fileH += ".h";
    string fileC = argv[1];
    fileC += ".cpp";

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
	unit->mergeModules();
	unit->sort();

	Output H;
	H.open(fileH.c_str());
	if (!H)
	{
	    cerr << argv[0] << ": can't open `" << fileH << "' for writing: " << strerror(errno) << endl;
	    unit->destroy();
	    return EXIT_FAILURE;
	}
	printHeader(H, dicts);

	string s = fileH;
	transform(s.begin(), s.end(), s.begin(), ToIfdef());
	H << "\n#ifndef __" << s << "__";
	H << "\n#define __" << s << "__";
	H << '\n';
	H << "\n#include <Freeze/DB.h>";
	
	Output C;
	C.open(fileC.c_str());
	if (!C)
	{
	    cerr << argv[0] << ": can't open `" << fileC << "' for writing: " << strerror(errno) << endl;
	    unit->destroy();
	    return EXIT_FAILURE;
	}
	printHeader(C, dicts);
	
	C << "\n#include <";
	if (include.size())
	{
	    C << include << '/';
	}
	C << fileH << '>';

	for (vector<Dict>::const_iterator p = dicts.begin(); p != dicts.end(); ++p)
	{
	    try
	    {
		if (!writeDict(argv[0], unit, *p, H, C))
		{
		    unit->destroy();
		    return EXIT_FAILURE;
		}
	    }
	    catch(...)
	    {
		cerr << argv[0] << ": unknown exception" << endl;
		unit->destroy();
		return EXIT_FAILURE;
	    }
	}

	H << "\n\n#endif\n";
	C << '\n';
    }
    
    unit->destroy();

    return status;
}
