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

#include <Slice/Preprocessor.h>
#include <Slice/CPlusPlusUtil.h>
#include <IceUtil/OutputUtil.h>

using namespace std;
using namespace IceUtil;
using namespace Slice;

struct Dict
{
    string name;
    string key;
    string value;
};

struct Index
{
    string name;
    string type;
    string member;
    bool caseSensitive;
};

void
usage(const char* n)
{
    cerr << "Usage: " << n << " [options] file-base [slice-files...]\n";
    cerr <<
        "Options:\n"
        "-h, --help            Show this message.\n"
        "-v, --version         Display the Ice version.\n"
        "--header-ext EXT      Use EXT instead of the default `h' extension.\n"
        "--source-ext EXT      Use EXT instead of the default `cpp' extension.\n"
        "-DNAME                Define NAME as 1.\n"
        "-DNAME=DEF            Define NAME as DEF.\n"
        "-UNAME                Remove any definition for NAME.\n"
        "-IDIR                 Put DIR in the include file search path.\n"
        "--include-dir DIR     Use DIR as the header include directory in source files.\n"
        "--dll-export SYMBOL   Use SYMBOL for DLL exports.\n"
        "--dict NAME,KEY,VALUE Create a Freeze dictionary with the name NAME,\n"
        "                      using KEY as key, and VALUE as value. This\n"
        "                      option may be specified multiple times for\n"
        "                      different names. NAME may be a scoped name.\n"
	"--index NAME,TYPE,MEMBER[,{case-sensitive|case-insensitive}]\n" 
        "                      Create a Freeze evictor index with the name\n"
        "                      NAME for member MEMBER of class TYPE. This\n"
        "                      option may be specified multiple times for\n"
        "                      different names. NAME may be a scoped name.\n"
        "                      When member is a string, the case can be\n"
        "                      sensitive or insensitive (default is sensitive).\n"
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
printFreezeTypes(Output& out, const vector<Dict>& dicts, const vector<Index>& indices)
{
    out << '\n';
    out << "\n// Freeze types in this file:";
    for(vector<Dict>::const_iterator p = dicts.begin(); p != dicts.end(); ++p)
    {
	out << "\n// name=\"" << p->name << "\", key=\"" << p->key << "\", value=\"" << p->value << "\"";
    }
    
    for(vector<Index>::const_iterator q = indices.begin(); q != indices.end(); ++q)
    {
	out << "\n// name=\"" << q->name << "\", type=\"" << q->type 
	    << "\", member=\"" << q->member << "\"";
	if(q->caseSensitive == false)
	{
	    out << " (case insensitive)";
	}
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
writeCodecC(const TypePtr& type, const string& name, const string& freezeType, bool encaps, Output& C)
{
    string quotedFreezeType = "\"" + freezeType + "\"";

    C << sp << nl << "void" << nl << name << "::write(" << inputTypeToString(type) << " v, "
      << "Freeze::" << freezeType << "& bytes, const ::Ice::CommunicatorPtr& communicator)";
    C << sb;
    C << nl << "IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);";
    C << nl << "IceInternal::BasicStream stream(instance.get());";
    if(encaps)
    {
        C << nl << "stream.startWriteEncaps();";
    }
    writeMarshalUnmarshalCode(C, type, "v", true, "stream", false);
    if(type->usesClasses())
    {
        C << nl << "stream.writePendingObjects();";
    }
    if(encaps)
    {
        C << nl << "stream.endWriteEncaps();";
    }
    C << nl << "bytes.swap(stream.b);";
    C << eb;

    C << sp << nl << "void" << nl << name << "::read(" << typeToString(type) << "& v, "
      << "const Freeze::" << freezeType << "& bytes, const ::Ice::CommunicatorPtr& communicator)";
    C << sb;
    C << nl << "IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);";
    C << nl << "IceInternal::BasicStream stream(instance.get());";
    if(type->usesClasses())
    {
        C << nl << "stream.sliceObjects(false);";
    }
    C << nl << "stream.b = bytes;";
    C << nl << "stream.i = stream.b.begin();";
    if(encaps)
    {
        C << nl << "stream.startReadEncaps();";
    }
    writeMarshalUnmarshalCode(C, type, "v", false, "stream", false);
    if(type->usesClasses())
    {
        C << nl << "stream.readPendingObjects();";
    }
    if(encaps)
    {
        C << nl << "stream.endReadEncaps();";
    }
    C << eb;
}

bool
writeCodecs(const string& n, UnitPtr& u, const Dict& dict, Output& H, Output& C, const string& dllExport)
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

    TypeList keyTypes = u->lookupType(dict.key, false);
    if(keyTypes.empty())
    {
	cerr << n << ": `" << dict.key << "' is not a valid type" << endl;
	return false;
    }
    TypePtr keyType = keyTypes.front();
    
    TypeList valueTypes = u->lookupType(dict.value, false);
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

    H << sp << nl << "typedef Freeze::Map< " << typeToString(keyType) << ", " << typeToString(valueType) << ", "
      << name << "KeyCodec, " << name << "ValueCodec> " << name << ";";

    for(q = scope.begin(); q != scope.end(); ++q)
    {
	H << sp;
	H << nl << '}';
    }

    writeCodecC(keyType, absolute + "KeyCodec", "Key", false, C);
    writeCodecC(valueType, absolute + "ValueCodec", "Value", true, C);

    return true;
}


void
writeIndexH(const string& memberTypeString, const string& name, Output& H, const string& dllExport)
{
    H << sp << nl << dllExport << "class " << name
      << " : public Freeze::Index";
    H << sb;
    H.dec();
    H << sp << nl << "public:";
    H << sp;
    H.inc();
    
    H << nl << name << "(const std::string&);";
    H << sp << nl << "std::vector<Ice::Identity>";
    H << nl << "findFirst(" << memberTypeString << ", Ice::Int) const;";

    H << sp << nl << "std::vector<Ice::Identity>";
    H << nl << "find(" << memberTypeString << ") const;";

    H << sp << nl << "Ice::Int";
    H << nl << "count(" << memberTypeString << ") const;";
    H.dec();
    H << sp << nl << "private:";
    H << sp;
    H.inc();
    
    H << nl << "virtual bool";
    H << nl << "marshalKey(const Ice::ObjectPtr&, Freeze::Key&) const;";
    
    H << sp << nl << "void";
    H << nl << "marshalKey(" << memberTypeString << ", Freeze::Key&) const;";
    
    H << eb << ';';
    H << sp;
    H << nl << "typedef IceUtil::Handle<" << name << "> " << name << "Ptr;";
}

void
writeIndexC(const TypePtr& type, const TypePtr& memberType, const string& memberName,
	    bool caseSensitive, const string& fullName, const string& name, Output& C)
{
    string inputType = inputTypeToString(memberType);

    C << sp << nl << fullName << "::" << name << "(const ::std::string& __name)";
    C.inc();
    C << nl << ": Freeze::Index(__name)";
    C.dec();
    C << sb;
    C << eb;

    C << sp << nl << "std::vector<Ice::Identity>";
    C << nl << fullName << "::" << "findFirst(" << inputType << " __index, ::Ice::Int __firstN) const";
    C << sb;
    C << nl << "Freeze::Key __bytes;";
    C << nl << "marshalKey(__index, __bytes);";
    C << nl << "return untypedFindFirst(__bytes, __firstN);";
    C << eb;

    C << sp << nl << "std::vector<Ice::Identity>";
    C << nl << fullName << "::" << "find(" << inputType << " __index) const";
    C << sb;
    C << nl << "Freeze::Key __bytes;";
    C << nl << "marshalKey(__index, __bytes);";
    C << nl << "return untypedFind(__bytes);";
    C << eb;

    C << sp << nl << "Ice::Int";
    C << nl << fullName << "::" << "count(" << inputType << " __index) const";
    C << sb;
    C << nl << "Freeze::Key __bytes;";
    C << nl << "marshalKey(__index, __bytes);";
    C << nl << "return untypedCount(__bytes);";
    C << eb;

    string typeString = typeToString(type);
    
    C << sp << nl << "bool";
    C << nl << fullName << "::" << "marshalKey(const Ice::ObjectPtr& __servant, Freeze::Key& __bytes) const";
    C << sb;
    C << nl << typeString << " __s = " << typeString << "::dynamicCast(__servant);";
    C << nl << "if(__s != 0)";
    C << sb;
    C << nl << "marshalKey(__s->" << memberName << ", __bytes);";
    C << nl << "return true;";
    C << eb;
    C << nl << "else";
    C << sb;
    C << nl << "return false;";
    C << eb;
    C << eb;
    
    C << sp << nl << "void";
    C << nl << fullName << "::" << "marshalKey(" << inputType << " __index, Freeze::Key& __bytes) const";
    C << sb;
    C << nl << "IceInternal::InstancePtr __instance = IceInternal::getInstance(_communicator);";
    C << nl << "IceInternal::BasicStream __stream(__instance.get());";
    
    string valueS;
    if(caseSensitive)
    {
	valueS = "__index";
    }
    else
    {
	C << nl << typeToString(memberType) << " __lowerCaseIndex = __index;";
	C << nl << "std::transform(__lowerCaseIndex.begin(), __lowerCaseIndex.end(), __lowerCaseIndex.begin(), tolower);";
	valueS = "__lowerCaseIndex";
    }

    writeMarshalUnmarshalCode(C, memberType, valueS, true, "__stream", false);
    if(memberType->usesClasses())
    {
        C << nl << "__stream.writePendingObjects();";
    }
    C << nl << "__bytes.swap(__stream.b);";
    C << eb;
}

bool
writeIndex(const string& n, UnitPtr& u, const Index& index, Output& H, Output& C, const string& dllExport)
{
    string absolute = index.name;
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

    TypeList types = u->lookupType(index.type, false);
    if(types.empty())
    {
	cerr << n << ": `" << index.type << "' is not a valid type" << endl;
	return false;
    }
    TypePtr type = types.front();
    
    ClassDeclPtr classDecl = ClassDeclPtr::dynamicCast(type);
    if(classDecl == 0)
    {
	cerr << n << ": `" << index.type << "' is not a class" << endl;
        return false;
    }

    DataMemberList dataMembers = classDecl->definition()->allDataMembers();
    DataMemberPtr dataMember = 0;
    DataMemberList::const_iterator p = dataMembers.begin();
    while(p != dataMembers.end() && dataMember == 0)
    {
	if((*p)->name() == index.member)
	{
	    dataMember = *p;
	}
	else
	{
	    ++p;
	}
    }

    if(dataMember == 0)
    {
	cerr << n << ": `" << index.type << "' has no data member named `" << index.member << "'" << endl;
        return false;
    }
    
    if(index.caseSensitive == false)
    {
	//
	// Let's check member is a string
	//
	BuiltinPtr memberType = BuiltinPtr::dynamicCast(dataMember->type());
	if(memberType == 0 || memberType->kind() != Builtin::KindString)
	{
	    cerr << n << ": `" << index.member << "'is not a string " << endl;
	    return false; 
	}
    }
   
    vector<string>::const_iterator q;
    
    for(q = scope.begin(); q != scope.end(); ++q)
    {
	H << sp;
	H << nl << "namespace " << *q << nl << '{';
    }

    writeIndexH(inputTypeToString(dataMember->type()), name, H, dllExport);
    
    for(q = scope.begin(); q != scope.end(); ++q)
    {
	H << sp;
	H << nl << '}';
    }

    writeIndexC(type, dataMember->type(), index.member, index.caseSensitive, absolute, name, C);
    return true;
}

int
main(int argc, char* argv[])
{
    string cppArgs;
    string headerExtension = "h";
    string sourceExtension = "cpp";
    vector<string> includePaths;
    string include;
    string dllExport;
    string output;
    bool debug = false;
    bool ice = false;
    bool caseSensitive = false;
    vector<Dict> dicts;
    vector<Index> indices;
     
    int idx = 1;
    while(idx < argc)
    {
	if(strncmp(argv[idx], "-I", 2) == 0)
	{
	    cppArgs += ' ';
	    cppArgs += argv[idx];

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
	    cppArgs += ' ';
	    cppArgs += argv[idx];

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
	else if(strcmp(argv[idx], "--index") == 0)
        {
            if(idx + 1 >= argc || argv[idx + 1][0] == '-')
            {
                cerr << argv[0] << ": argument expected for `" << argv[idx] << "'" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }

            string s = argv[idx + 1];
            s.erase(remove_if(s.begin(), s.end(), ::isspace), s.end());
            
	    Index index;

            string::size_type pos;
            pos = s.find(',');
            if(pos != string::npos)
            {
                index.name = s.substr(0, pos);
                s.erase(0, pos + 1);
            }
            pos = s.find(',');
            if(pos != string::npos)
            {
                index.type = s.substr(0, pos);
                s.erase(0, pos + 1);
            }
	    pos = s.find(',');
	    string caseString;
	    if(pos != string::npos)
            {
                index.member = s.substr(0, pos);
                s.erase(0, pos + 1);
		caseString = s;
            }
	    else
	    {
		index.member = s;
		caseString = "case-sensitive";
	    }

            if(index.name.empty())
            {
                cerr << argv[0] << ": " << argv[idx] << ": no name specified" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }

            if(index.type.empty())
            {
                cerr << argv[0] << ": " << argv[idx] << ": no type specified" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }

            if(index.member.empty())
            {
                cerr << argv[0] << ": " << argv[idx] << ": no member specified" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }
	    
	    if(caseString != "case-sensitive" && caseString != "case-insensitive")
            {
                cerr << argv[0] << ": " << argv[idx]
		     << ": the case can be `case-sensitive' or `case-insensitive'" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }
	    index.caseSensitive = (caseString == "case-sensitive");

            indices.push_back(index);

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
	else if(strcmp(argv[idx], "--header-ext") == 0)
	{
	    if(idx + 1 >= argc)
	    {
		cerr << argv[0] << ": argument expected for`" << argv[idx] << "'" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }

	    headerExtension = argv[idx + 1];
	    for(int i = idx ; i + 2 < argc ; ++i)
	    {
		argv[i] = argv[i + 2];
	    }
	    argc -= 2;
	}
	else if(strcmp(argv[idx], "--source-ext") == 0)
	{
	    if(idx + 1 >= argc)
	    {
		cerr << argv[0] << ": argument expected for`" << argv[idx] << "'" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }

	    sourceExtension = argv[idx + 1];
	    for(int i = idx ; i + 2 < argc ; ++i)
	    {
		argv[i] = argv[i + 2];
	    }
	    argc -= 2;
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

    if(dicts.empty() && indices.empty())
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
    fileH += "." + headerExtension;
    string fileC = argv[1];
    fileC += "." + sourceExtension;
    if(!output.empty())
    {
	fileH = output + '/' + fileH;
	fileC = output + '/' + fileC;
    }

    UnitPtr u = Unit::createUnit(true, false, ice, caseSensitive);

    StringList includes;

    int status = EXIT_SUCCESS;

    for(idx = 2 ; idx < argc ; ++idx)
    {
	Preprocessor icecpp(argv[0], argv[idx], cppArgs);

        //
        // Add an include file for each Slice file. Note that the .h extension
        // is replaced with headerExtension later.
        //
	includes.push_back(icecpp.getBaseName() + ".h");

	FILE* cppHandle = icecpp.preprocess(false);

	if(cppHandle == 0)
	{
	    u->destroy();
	    return EXIT_FAILURE;
	}
	
	status = u->parse(cppHandle, debug);

	if(!icecpp.close())
	{
	    u->destroy();
	    return EXIT_FAILURE;	    
	}
    }

    if(status == EXIT_SUCCESS)
    {
	u->mergeModules();
	u->sort();

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
	    u->destroy();
	    return EXIT_FAILURE;
	}
	printHeader(H);
	printFreezeTypes(H, dicts, indices);

	Output C;
	C.open(fileC.c_str());
	if(!C)
	{
	    cerr << argv[0] << ": can't open `" << fileC << "' for writing: " << strerror(errno) << endl;
	    u->destroy();
	    return EXIT_FAILURE;
	}
	printHeader(C);
	printFreezeTypes(C, dicts, indices);
	
	string s = fileH;
	transform(s.begin(), s.end(), s.begin(), ToIfdef());
	H << "\n#ifndef __" << s << "__";
	H << "\n#define __" << s << "__";
	H << '\n';
	
	if(dicts.size() > 0)
	{
	    H << "\n#include <Freeze/Map.h>";
	}

	if(indices.size() > 0)
	{
	    H << "\n#include <Freeze/Index.h>";
	}

	
	{
	    for(StringList::const_iterator p = includes.begin(); p != includes.end(); ++p)
	    {
		H << "\n#include <" << changeInclude(*p, includePaths) << "." + headerExtension + ">";
	    }
	}

        C << "\n#include <Ice/BasicStream.h>";
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
		    if(!writeCodecs(argv[0], u, *p, H, C, dllExport))
		    {
			u->destroy();
			return EXIT_FAILURE;
		    }
		}
		catch(...)
		{
		    cerr << argv[0] << ": unknown exception" << endl;
		    u->destroy();
		    return EXIT_FAILURE;
		}
	    } 


	    for(vector<Index>::const_iterator q = indices.begin(); q != indices.end(); ++q)
	    {
		try
		{
		    if(!writeIndex(argv[0], u, *q, H, C, dllExport))
		    {
			u->destroy();
			return EXIT_FAILURE;
		    }
		}
		catch(...)
		{
		    cerr << argv[0] << ": unknown exception" << endl;
		    u->destroy();
		    return EXIT_FAILURE;
		}
	    }
	}

	H << "\n\n#endif\n";
	C << '\n';
    }
    
    u->destroy();

    return status;
}
