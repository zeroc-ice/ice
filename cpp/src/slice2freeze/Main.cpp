// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Slice/CPlusPlusUtil.h>
#include <IceUtil/OutputUtil.h>
#include <fstream>

using namespace std;
using namespace IceUtil;
using namespace Slice;

struct Dict
{
    string name;
    string key;
    string value;
};

void
usage(const char* n)
{
    cerr << "Usage: " << n << " [options] file-base [slice-files...]\n";
    cerr <<
	"Options:\n"
	"-h, --help            Show this message.\n"
	"-v, --version         Display the Ice version.\n"
	"-DNAME                Define NAME as 1.\n"
	"-DNAME=DEF            Define NAME as DEF.\n"
	"-UNAME                Remove any definition for NAME.\n"
	"-IDIR                 Put DIR in the include file search path.\n"
	"--include-dir DIR     Use DIR as the header include directory.\n"
	"--dll-export SYMBOL   Use SYMBOL for DLL exports.\n"
	"--dict NAME,KEY,VALUE Create a Freeze dictionary with the name NAME,\n"
	"                      using KEY as key, and VALUE as value. This\n"
	"                      option may be specified multiple times for\n"
	"                      different names. NAME may be a scoped name.\n"
	"--binary              Use the binary encoding.\n"
	"--output-dir DIR      Create files in the directory DIR.\n"
	"-d, --debug           Print debug messages.\n"
        "--ice                 Permit `Ice' prefix (for building Ice source code only)\n"
	;
    // Note: --case-sensitive is intentionally not shown here!
}

bool
checkIdentifier(string n, string t, string s)
{
    if(s.empty() || (!isalpha(s[0]) && s[0] != '_'))
    {
	cerr << n << ": `" << t << "' is not a valid type name" << endl;
	return false;
    }
    
    for(unsigned int i = 1; i < s.size(); ++i)
    {
	if(!isalnum(s[i]) && s[i] != '_')
	{
	    cerr << n << ": `" << t << "' is not a valid type name" << endl;
	    return false;
	}
    }

    return true;
}

void
printFreezeTypes(Output& out, const vector<Dict>& dicts)
{
    out << '\n';
    out << "\n// Freeze types in this file:";
    for(vector<Dict>::const_iterator p = dicts.begin(); p != dicts.end(); ++p)
    {
	out << "\n// name=\"" << p->name << "\", key=\"" << p->key << "\", value=\"" << p->value << "\"";
    }
    out << '\n';
}

void
writeCodecH(const TypePtr& type, const string& name, const string& freezeType, Output& H, const string& dllExport)
{
    H << sp << nl << dllExport << "class " << name;
    H << sb;
    H.dec();
    H << sp << nl << "public:";
    H << sp;
    H.inc();
    H << nl << "static void write(" << inputTypeToString(type)
      << ", Freeze::" << freezeType << "& bytes, const ::Ice::CommunicatorPtr& communicator);";
    H << nl << "static void read(" << typeToString(type) << "&, const Freeze::" << freezeType << "& bytes, "
      << "const ::Ice::CommunicatorPtr& communicator);";
    H << eb << ';';
}

void
writeCodecC(const TypePtr& type, const string& name, const string& freezeType, Output& C, bool binary)
{
    string quotedFreezeType = "\"" + freezeType + "\"";

    C << sp << nl << "void" << nl << name << "::write(" << inputTypeToString(type) << " v, "
      << "Freeze::" << freezeType << "& bytes, const ::Ice::CommunicatorPtr& communicator)";
    C << sb;
    if(binary)
    {
        C << nl << "IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);";
        C << nl << "IceInternal::BasicStream stream(instance);";
        writeMarshalUnmarshalCode(C, type, "v", true, "stream", false);
        C << nl << "bytes = stream.b;";
    }
    else
    {
        C << nl << "std::ostringstream ostr;";
        C << nl << "Ice::StreamPtr stream = new IceXML::StreamI(communicator, ostr);";
        writeGenericMarshalUnmarshalCode(C, type, "v", true, quotedFreezeType, "stream", true);
        C << nl << "const std::string& str = ostr.str();";
        C << nl << "bytes.resize(str.size());";
        C << nl << "std::copy(str.begin(), str.end(), bytes.begin());";
    }
    C << eb;


    C << sp << nl << "void" << nl << name << "::read(" << typeToString(type) << "& v, "
      << "const Freeze::" << freezeType << "& bytes, const ::Ice::CommunicatorPtr& communicator)";
    C << sb;
    if(binary)
    {
        C << nl << "IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);";
        C << nl << "IceInternal::BasicStream stream(instance);";
        C << nl << "stream.b = bytes;";
        C << nl << "stream.i = stream.b.begin();";
        writeMarshalUnmarshalCode(C, type, "v", false, "stream", false);
    }
    else
    {
        //
        // COMPILERFIX: string data(bytes.begin(), bytes.end());
        //
        // This won't link with STLport 4.5.3 stldebug version.
        //
        C << nl << "std::string str;";
        C << nl << "str.append(\"<data>\");";
        C << nl << "str.append(&bytes[0], bytes.size());";
        C << nl << "str.append(\"</data>\");";
        C << nl << "std::istringstream istr(str);";
        C << nl << "Ice::StreamPtr stream = new IceXML::StreamI(communicator, istr, false);";
        writeGenericMarshalUnmarshalCode(C, type, "v", false, quotedFreezeType, "stream", true);
    }
    C << eb;
}

bool
writeCodecs(const string& n, UnitPtr& unit, const Dict& dict, Output& H, Output& C, const string& dllExport,
            bool binary)
{
    string absolute = dict.name;
    if(absolute.find("::") == 0)
    {
	absolute.erase(0, 2);
    }
    string name = absolute;
    vector<string> scope;
    string::size_type pos;
    while((pos = name.find("::")) != string::npos)
    {
	string s = name.substr(0, pos);
	name.erase(0, pos + 2);
	
	if(!checkIdentifier(n, absolute, s))
	{
	    return false;
	}
	
	scope.push_back(s);
    }
    
    if(!checkIdentifier(n, absolute, name))
    {
	return false;
    }

    TypeList keyTypes = unit->lookupType(dict.key, false);
    if(keyTypes.empty())
    {
	cerr << n << ": `" << dict.key << "' is not a valid type" << endl;
	return false;
    }
    TypePtr keyType = keyTypes.front();
    
    TypeList valueTypes = unit->lookupType(dict.value, false);
    if(valueTypes.empty())
    {
	cerr << n << ": `" << dict.value << "' is not a valid type" << endl;
	return false;
    }
    TypePtr valueType = valueTypes.front();
    
    vector<string>::const_iterator q;
    
    for(q = scope.begin(); q != scope.end(); ++q)
    {
	H << sp;
	H << nl << "namespace " << *q << nl << '{';
    }

    writeCodecH(keyType, name + "KeyCodec", "Key", H, dllExport);
    writeCodecH(valueType, name + "ValueCodec", "Value", H, dllExport);

    H << sp << nl << "typedef Freeze::DBMap< " << typeToString(keyType) << ", " << typeToString(valueType) << ", "
      << name << "KeyCodec, " << name << "ValueCodec> " << name << ";";

    for(q = scope.begin(); q != scope.end(); ++q)
    {
	H << sp;
	H << nl << '}';
    }

    writeCodecC(keyType, absolute + "KeyCodec", "Key", C, binary);
    writeCodecC(valueType, absolute + "ValueCodec", "Value", C, binary);

    return true;
}

int
main(int argc, char* argv[])
{
    string cpp("cpp");
    vector<string> includePaths;
    string include;
    string dllExport;
    bool binary = false;
    string output;
    bool debug = false;
    bool ice = false;
    bool caseSensitive = false;
    vector<Dict> dicts;

    int idx = 1;
    while(idx < argc)
    {
	if(strncmp(argv[idx], "-I", 2) == 0)
	{
	    cpp += ' ';
	    cpp += argv[idx];

	    string path = argv[idx] + 2;
	    if(path.length())
	    {
		includePaths.push_back(path);
	    }

	    for(int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if(strncmp(argv[idx], "-D", 2) == 0 || strncmp(argv[idx], "-U", 2) == 0)
	{
	    cpp += ' ';
	    cpp += argv[idx];

	    for(int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if(strcmp(argv[idx], "--dict") == 0)
	{
	    if(idx + 1 >= argc || argv[idx + 1][0] == '-')
            {
		cerr << argv[0] << ": argument expected for`" << argv[idx] << "'" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
            }

	    string s = argv[idx + 1];
	    s.erase(remove_if(s.begin(), s.end(), ::isspace), s.end());
	    
	    Dict dict;

	    string::size_type pos;
	    pos = s.find(',');
	    if(pos != string::npos)
	    {
		dict.name = s.substr(0, pos);
		s.erase(0, pos + 1);
	    }
	    pos = s.find(',');
	    if(pos != string::npos)
	    {
		dict.key = s.substr(0, pos);
		s.erase(0, pos + 1);
	    }
	    dict.value = s;

	    if(dict.name.empty())
	    {
		cerr << argv[0] << ": " << argv[idx] << ": no name specified" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }

	    if(dict.key.empty())
	    {
		cerr << argv[0] << ": " << argv[idx] << ": no key specified" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }

	    if(dict.value.empty())
	    {
		cerr << argv[0] << ": " << argv[idx] << ": no value specified" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }

	    dicts.push_back(dict);

	    for(int i = idx ; i + 2 < argc ; ++i)
	    {
		argv[i] = argv[i + 2];
	    }
	    argc -= 2;
	}
	else if(strcmp(argv[idx], "-h") == 0 || strcmp(argv[idx], "--help") == 0)
	{
	    usage(argv[0]);
	    return EXIT_SUCCESS;
	}
	else if(strcmp(argv[idx], "-v") == 0 || strcmp(argv[idx], "--version") == 0)
	{
	    cout << ICE_STRING_VERSION << endl;
	    return EXIT_SUCCESS;
	}
	else if(strcmp(argv[idx], "-d") == 0 || strcmp(argv[idx], "--debug") == 0)
	{
	    debug = true;
	    for(int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if(strcmp(argv[idx], "--ice") == 0)
	{
	    ice = true;
	    for(int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if(strcmp(argv[idx], "--case-sensitive") == 0)
	{
	    caseSensitive = true;
	    for(int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if(strcmp(argv[idx], "--include-dir") == 0)
	{
	    if(idx + 1 >= argc)
            {
		cerr << argv[0] << ": argument expected for`" << argv[idx] << "'" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
            }
	    
	    include = argv[idx + 1];
	    for(int i = idx ; i + 2 < argc ; ++i)
	    {
		argv[i] = argv[i + 2];
	    }
	    argc -= 2;
	}
	else if(strcmp(argv[idx], "--dll-export") == 0)
	{
	    if(idx + 1 >= argc)
            {
		cerr << argv[0] << ": argument expected for`" << argv[idx] << "'" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
            }
	    
	    dllExport = argv[idx + 1];
	    for(int i = idx ; i + 2 < argc ; ++i)
	    {
		argv[i] = argv[i + 2];
	    }
	    argc -= 2;
	}
	else if(strcmp(argv[idx], "--binary") == 0)
	{
	    binary = true;
	    for(int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if(strcmp(argv[idx], "--output-dir") == 0)
	{
	    if(idx + 1 >= argc)
	    {
		cerr << argv[0] << ": argument expected for`" << argv[idx] << "'" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }
	    
	    output = argv[idx + 1];
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

    if(dicts.empty())
    {
	cerr << argv[0] << ": no Freeze types specified" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    if(argc < 2)
    {
	cerr << argv[0] << ": no file name base specified" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    string fileH = argv[1];
    fileH += ".h";
    string fileC = argv[1];
    fileC += ".cpp";
    if(!output.empty())
    {
	fileH = output + '/' + fileH;
	fileC = output + '/' + fileC;
    }

    UnitPtr unit = Unit::createUnit(true, false, ice, caseSensitive);

    StringList includes;

    int status = EXIT_SUCCESS;

    for(idx = 2 ; idx < argc ; ++idx)
    {
	string base(argv[idx]);
	string suffix;
	string::size_type pos = base.rfind('.');
	if(pos != string::npos)
	{
	    suffix = base.substr(pos);
	    transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);
	}
	if(suffix != ".ice")
	{
	    cerr << argv[0] << ": input files must end with `.ice'" << endl;
	    return EXIT_FAILURE;
	}
	base.erase(pos);

	includes.push_back(base + ".h");

	ifstream test(argv[idx]);
	if(!test)
	{
	    cerr << argv[0] << ": can't open `" << argv[idx] << "' for reading: " << strerror(errno) << endl;
	    return EXIT_FAILURE;
	}
	test.close();

	string cmd = cpp + " " + argv[idx];
#ifdef _WIN32
	FILE* cppHandle = _popen(cmd.c_str(), "r");
#else
	FILE* cppHandle = popen(cmd.c_str(), "r");
#endif
	if(cppHandle == 0)
	{
	    cerr << argv[0] << ": can't run C++ preprocessor: " << strerror(errno) << endl;
	    unit->destroy();
	    return EXIT_FAILURE;
	}
	
	status = unit->parse(cppHandle, debug);
	
#ifdef _WIN32
	_pclose(cppHandle);
#else
	pclose(cppHandle);
#endif
    }

    if(status == EXIT_SUCCESS)
    {
	unit->mergeModules();
	unit->sort();

	{
	    for(vector<string>::iterator p = includePaths.begin(); p != includePaths.end(); ++p)
	    {
		if(p->length() && (*p)[p->length() - 1] != '/')
		{
		    *p += '/';
		}
	    }
	}

	Output H;
	H.open(fileH.c_str());
	if(!H)
	{
	    cerr << argv[0] << ": can't open `" << fileH << "' for writing: " << strerror(errno) << endl;
	    unit->destroy();
	    return EXIT_FAILURE;
	}
	printHeader(H);
	printFreezeTypes(H, dicts);

	Output C;
	C.open(fileC.c_str());
	if(!C)
	{
	    cerr << argv[0] << ": can't open `" << fileC << "' for writing: " << strerror(errno) << endl;
	    unit->destroy();
	    return EXIT_FAILURE;
	}
	printHeader(C);
	printFreezeTypes(C, dicts);
	
	string s = fileH;
	transform(s.begin(), s.end(), s.begin(), ToIfdef());
	H << "\n#ifndef __" << s << "__";
	H << "\n#define __" << s << "__";
	H << '\n';
	H << "\n#include <Freeze/Map.h>";
	
	{
	    for(StringList::const_iterator p = includes.begin(); p != includes.end(); ++p)
	    {
		H << "\n#include <" << changeInclude(*p, includePaths) << ".h>";
	    }
	}

        if(binary)
        {
            C << "\n#include <Ice/BasicStream.h>";
        }
        else
        {
            C << "\n#include <IceXML/StreamI.h>";
            C << "\n#include <algorithm>";
        }
	C << "\n#include <";
	if(include.size())
	{
	    C << include << '/';
	}
	C << fileH << '>';

	printVersionCheck(H);
	printVersionCheck(C);

	printDllExportStuff(H, dllExport);
	if(dllExport.size())
	{
	    dllExport += " ";
	}

	{
	    for(vector<Dict>::const_iterator p = dicts.begin(); p != dicts.end(); ++p)
	    {
		try
		{
		    if(!writeCodecs(argv[0], unit, *p, H, C, dllExport, binary))
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
	}

	H << "\n\n#endif\n";
	C << '\n';
    }
    
    unit->destroy();

    return status;
}
