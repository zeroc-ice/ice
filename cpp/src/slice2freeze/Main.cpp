// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <Slice/Preprocessor.h>
#include <Slice/CPlusPlusUtil.h>
#include <IceUtil/OutputUtil.h>

using namespace std;
using namespace IceUtil;
using namespace Slice;

struct DictIndex
{
    string member;
    bool caseSensitive;

    bool operator==(const DictIndex& rhs) const
    {
	return member == rhs.member;
    }
};

struct Dict
{
    string name;
    string key;
    string value;
  
    vector<DictIndex> indices;
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
	"--dict-index DICT[,MEMBER][,{case-sensitive|case-insensitive}] \n"
	"                      Add an index to dictionary DICT. If MEMBER is \n"
        "                      specified, then DICT's VALUE must be a class or\n"
	"                      a struct, and MEMBER must designate a member of\n"
	"                      VALUE. Otherwise, the entire VALUE is used for \n"
	"                      indexing. When the secondary key is a string, \n"
	"                      the case can be sensitive or insensitive (default\n"
	"                      is sensitive).\n"
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
    H << sp << nl << "class " << dllExport << name;
    H << sb;
    H.dec();
    H << sp << nl << "public:";
    H << sp;
    H.inc();
    H << nl << "static void write(" << inputTypeToString(type)
      << ", Freeze::" << freezeType << "& bytes, const ::Ice::CommunicatorPtr& communicator);";
    H << nl << "static void read(" << typeToString(type) << "&, const Freeze::" << freezeType << "& bytes, "
      << "const ::Ice::CommunicatorPtr& communicator);";
    H << nl << "static const std::string& typeId();";
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
    C << nl << "::std::vector<Ice::Byte>(stream.b.begin(), stream.b.end()).swap(bytes);";
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
    C << nl << "stream.b.resize(bytes.size());";
    C << nl << "::memcpy(&stream.b[0], &bytes[0], bytes.size());";
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

    string staticName = "__";
    for(string::const_iterator p = name.begin(); p != name.end(); ++p)
    {
	if((*p) == ':')
	{
	    staticName += '_';
	}
	else
	{
	    staticName += *p;
	}
    }
    staticName += "_typeId";

    C << sp << nl << "static const ::std::string " << staticName << " = \"" << type->typeId() << "\";";

    C << sp << nl << "const ::std::string&" << nl << name << "::typeId()";
    C << sb;
    C << nl << "return " << staticName << ";";

    C << eb;
}

void
writeDictWithIndicesH(const string& name, const Dict& dict, 
		      const vector<TypePtr> indexTypes, 
		      const TypePtr& keyType, const TypePtr& valueType,
		      Output& H, const string& dllExport)
{
    
    string templateParams = string("< ") + typeToString(keyType) + ", "
	+ typeToString(valueType) + ", " + name + "KeyCodec, " 
	+ name + "ValueCodec>";
    
    vector<string> capitalizedMembers;
    size_t i;
    for(i = 0; i < dict.indices.size(); ++i)
    {
	const string& member = dict.indices[i].member;
	if(!member.empty())
	{
	    string capitalizedMember = member;
	    capitalizedMember[0] = toupper(capitalizedMember[0]);
	    capitalizedMembers.push_back(capitalizedMember);
	}
	else
	{
	    capitalizedMembers.push_back("Value");
	}
    }

    H << sp << nl << "class " << dllExport << name 
      << " : public Freeze::Map" << templateParams;
    H << sb;
    H.dec();
    H << sp << nl << "public:";
    H << sp;
    H.inc();

    //
    // Typedefs
    //
    H << nl << "typedef std::pair<const " << typeToString(keyType)
      << ", const" << typeToString(valueType) << "> value_type;";

    H << nl << "typedef Freeze::Iterator" << templateParams << " iterator;";
    H << nl << "typedef Freeze::ConstIterator" << templateParams << " const_iterator;";
    H << nl << "typedef size_t size_type;";
    H << nl << "typedef ptrdiff_t difference_type;";
    
    //
    // Nested index classes
    //

    for(i = 0; i < capitalizedMembers.size(); ++i)
    {
	H << sp << nl << "class " << dllExport << capitalizedMembers[i] << "Index"
	  << " : public Freeze::MapIndexBase";	
	H << sb;

	H.dec();
	H << sp << nl << "public:";
	H << sp;
	H.inc();
	H << nl << capitalizedMembers[i] << "Index(const std::string&);";
	
	H << sp;
	H << nl << "static void writeIndex(" << inputTypeToString(indexTypes[i])
	  << ", Freeze::Key&, const Ice::CommunicatorPtr&);";


	H.dec();
	H << sp << nl << "protected:";
	H << sp;
	H.inc();

	H << nl << "virtual void marshalKey(const Freeze::Value&, Freeze::Key&) const;";
	
	H << eb << ';';
    }

    //
    // Constructors
    //
    H << sp;
    H << nl << name << "(const Freeze::ConnectionPtr&, const std::string&, bool = true);";
    H << sp;
    H << nl << "template <class _InputIterator>"
      << nl << name << "(const Freeze::ConnectionPtr& __connection, const std::string& __dbName, bool __createDb, "
      << "_InputIterator __first, _InputIterator __last)";
    H.inc();
    H << nl << ": Freeze::Map" << templateParams <<"(__connection->getCommunicator())";
    H.dec();
    H << sb;
    H << nl << "std::vector<Freeze::MapIndexBasePtr> __indices;";
    for(i = 0; i < capitalizedMembers.size(); ++i)
    {
	string indexName = dict.indices[i].member;
	if(indexName.empty())
	{
	    indexName = "index";
	}
	indexName = string("\"") + indexName + "\"";

	H << nl << "__indices.push_back(new " << capitalizedMembers[i] << "Index(" << indexName << "));";
    }
    H << nl << "this->_helper.reset(Freeze::MapHelper::create(__connection, __dbName, "
      << name + "KeyCodec::typeId(), "
      << name + "ValueCodec::typeId(), __indices, __createDb));";
    H << nl << "while(__first != __last)";
    H << sb;
    H << nl << "put(*__first);";
    H << nl << "++__first;";
    H << eb;
    H << eb;

    //
    // Find and count functions
    //
    for(i = 0; i < capitalizedMembers.size(); ++i)
    {
	H << sp;
	H << nl << "iterator findBy" << capitalizedMembers[i]
	  << "(" << inputTypeToString(indexTypes[i]) << ");";
	H << nl << "const_iterator findBy" << capitalizedMembers[i]
	  << "(" << inputTypeToString(indexTypes[i]) << ") const;";
	
	string countFunction = dict.indices[i].member.empty() ? "valueCount" 
	    : dict.indices[i].member + "Count";

	H << nl << "int " << countFunction
	  << "(" << inputTypeToString(indexTypes[i]) << ") const;";
    }
    
    H << eb << ';';
}

void
writeDictWithIndicesC(const string& name, const string& absolute, const Dict& dict, 
		      const vector<TypePtr> indexTypes, 
		      const TypePtr& keyType, const TypePtr& valueType,
		      Output& C)
{
    string templateParams = string("< ") + typeToString(keyType) + ", "
	+ typeToString(valueType) + ", " + name + "KeyCodec, " 
	+ name + "ValueCodec>";
    
    vector<string> capitalizedMembers;
    size_t i;
    for(i = 0; i < dict.indices.size(); ++i)
    {
	const string& member = dict.indices[i].member;
	if(!member.empty())
	{
	    string capitalizedMember = member;
	    capitalizedMember[0] = toupper(capitalizedMember[0]);
	    capitalizedMembers.push_back(capitalizedMember);
	}
	else
	{
	    capitalizedMembers.push_back("Value");
	}
    }
    

    //
    // Nested index classes
    //
    
    for(i = 0; i < capitalizedMembers.size(); ++i)
    {
	string className = capitalizedMembers[i] + "Index";

	C << sp << nl << absolute << "::" << className << "::" << className
	  << "(const std::string& __name)";

	C.inc();
	C << nl << ": Freeze::MapIndexBase(__name)";
	C.dec();
	C << sb;
	C << eb;

	C << sp << nl << "void" 
	  << nl << absolute << "::" << className << "::" 
	  << "marshalKey(const Freeze::Value& __v, Freeze::Key& __k) const";
	C << sb;
	
	bool optimize = false;

	if(dict.indices[i].member.empty() && dict.indices[i].caseSensitive)
	{
	    optimize = true;
	    C << nl << "__k = __v;";
	}
	else
	{
	    //
	    // Can't optimize
	    //
	    C << nl << typeToString(valueType) << " __x;";
	    C << nl << absolute << "ValueCodec::read(__x, __v, _communicator);";
	    string param = "__x";
	    
	    if(!dict.indices[i].member.empty())
	    {
		if(ClassDeclPtr::dynamicCast(valueType) != 0)
		{
		    param += "->" + dict.indices[i].member;
		}
		else
		{
		    param += "." + dict.indices[i].member;
		}
	    }
	    C << nl << "writeIndex(" << param << ", __k, _communicator);";
	}
	C << eb;
	
	C << sp << nl << "void" 
	  << nl << absolute << "::" << className << "::" 
	  << "writeIndex(" << inputTypeToString(indexTypes[i])
	  << " __index, Freeze::Key& __bytes, const Ice::CommunicatorPtr& __communicator)";
	C << sb;
	
	if(optimize)
	{
	    C << nl << absolute << "ValueCodec::write(__index, __bytes, __communicator);";
	}
	else
	{
	    C << nl << "IceInternal::InstancePtr __instance = IceInternal::getInstance(__communicator);";
	    C << nl << "IceInternal::BasicStream __stream(__instance.get());";
	    
	    string valueS;
	    if(dict.indices[i].caseSensitive)
	    {
		valueS = "__index";
	    }
	    else
	    {
		C << nl << typeToString(indexTypes[i]) << " __lowerCaseIndex = __index;";
		C << nl << "std::transform(__lowerCaseIndex.begin(), __lowerCaseIndex.end(), __lowerCaseIndex.begin(), tolower);";
		valueS = "__lowerCaseIndex";
	    }
	    
	    writeMarshalUnmarshalCode(C, indexTypes[i], valueS, true, "__stream", false);
	    if(indexTypes[i]->usesClasses())
	    {
		C << nl << "__stream.writePendingObjects();";
	    }
	    C << nl << "::std::vector<Ice::Byte>(__stream.b.begin(), __stream.b.end()).swap(__bytes);";
	}
	C << eb;
    }


    //
    // Constructor
    //
    
    C << sp << nl << absolute << "::" << name
      << "(const Freeze::ConnectionPtr& __connection, const std::string& __dbName , bool __createDb)";
    
    C.inc();
    C << nl << ": Freeze::Map" << templateParams <<"(__connection->getCommunicator())";
    C.dec();
    C << sb;
    C << nl << "std::vector<Freeze::MapIndexBasePtr> __indices;";
    for(i = 0; i < capitalizedMembers.size(); ++i)
    {
	string indexName = dict.indices[i].member;
	if(indexName.empty())
	{
	    indexName = "index";
	}
	indexName = string("\"") + indexName + "\"";

	C << nl << "__indices.push_back(new " << capitalizedMembers[i] << "Index(" << indexName << "));";
    }
    C << nl << "_helper.reset(Freeze::MapHelper::create(__connection, __dbName, "
      << absolute + "KeyCodec::typeId(), "
      << absolute + "ValueCodec::typeId(), __indices, __createDb));";
    C << eb;

    //
    // Find and count functions
    //
    for(i = 0; i < capitalizedMembers.size(); ++i)
    {	
	string indexClassName = capitalizedMembers[i] + "Index";
	
	string indexName = dict.indices[i].member;
	if(indexName.empty())
	{
	    indexName = "index";
	}
	indexName = string("\"") + indexName + "\"";

	C << sp << nl << absolute << "::iterator"
	  << nl << absolute << "::" << "findBy" << capitalizedMembers[i]
	  << "(" << inputTypeToString(indexTypes[i]) << " __index)";
	C << sb;
	C << nl << "Freeze::Key __bytes;";
	C << nl << indexClassName << "::" << "writeIndex(__index, __bytes, _communicator);";
	C << nl << "return iterator(_helper->index(" << indexName 
	  << ")->untypedFind(__bytes, false), _communicator);";
	C << eb;

	C << sp << nl << absolute << "::const_iterator"
	  << nl << absolute << "::" << "findBy" << capitalizedMembers[i]
	  << "(" << inputTypeToString(indexTypes[i]) << " __index) const";
	C << sb;
	C << nl << "Freeze::Key __bytes;";
	C << nl << indexClassName << "::" << "writeIndex(__index, __bytes, _communicator);";
	C << nl << "return const_iterator(_helper->index(" << indexName 
	  << ")->untypedFind(__bytes, true), _communicator);";
	C << eb;

	string countFunction = dict.indices[i].member.empty() ? "valueCount" 
	    : dict.indices[i].member + "Count";

	C << sp << nl << "int"
	  << nl << absolute << "::" << countFunction
	  << "(" << inputTypeToString(indexTypes[i]) << " __index) const";
	C << sb;
	C << nl << "Freeze::Key __bytes;";
	C << nl << indexClassName << "::" << "writeIndex(__index, __bytes, _communicator);";
	C << nl << "return _helper->index(" << indexName 
	  << ")->untypedCount(__bytes);";
	C << eb;
    }
}


bool
writeDict(const string& n, UnitPtr& u, const Dict& dict, Output& H, Output& C, const string& dllExport)
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

    vector<TypePtr> indexTypes;

    if(dict.indices.size() == 0)
    {
	H << sp << nl << "typedef Freeze::Map< " << typeToString(keyType) << ", " << typeToString(valueType) << ", "
	  << name << "KeyCodec, " << name << "ValueCodec> " << name << ";";
    }
    else
    {
	for(vector<DictIndex>::const_iterator p = dict.indices.begin();
	    p != dict.indices.end(); ++p)
	{
	    const DictIndex& index = *p;
	    if(index.member.empty())
	    {
		if(dict.indices.size() > 1)
		{
		    cerr << n << ": bad index for dictionary `" << dict.name << "'" << endl;
		    return false;
		}
		
		if(!Dictionary::legalKeyType(valueType))
		{
		    cerr << n << ": `" << dict.value << "' is not a valid index type" << endl;
		    return false; 
		}


		if(index.caseSensitive == false)
		{
		    //
		    // Let's check value is a string
		    //
		
		    BuiltinPtr builtInType = BuiltinPtr::dynamicCast(valueType);
		    
		    if(builtInType == 0 || builtInType->kind() != Builtin::KindString)
		    {
			cerr << n << ": VALUE is a `" << dict.value << "', not a string" << endl;
			return false; 
		    }
		}
		indexTypes.push_back(valueType);
	    }
	    else
	    {
		DataMemberPtr dataMember = 0;
		DataMemberList dataMembers;
		
		ClassDeclPtr classDecl = ClassDeclPtr::dynamicCast(valueType);
		if(classDecl != 0)
		{
		    dataMembers = classDecl->definition()->allDataMembers();
		}
		else
		{
		    StructPtr structDecl = StructPtr::dynamicCast(valueType);
		    if(structDecl == 0)
		    {
			cerr << n << ": `" << dict.value << "' is neither a class nor a struct." << endl;
			return false;
		    }
		    dataMembers = structDecl->dataMembers();
		}
		DataMemberList::const_iterator q = dataMembers.begin();
		while(q != dataMembers.end() && dataMember == 0)
		{
		    if((*q)->name() == index.member)
		    {
			dataMember = *q;
		    }
		    else
		    {
			++q;
		    }
		}
		
		if(dataMember == 0)
		{
		    cerr << n << ": The value of `" << dict.name << "' has no data member named `" << index.member << "'" << endl;
		    return false;
		}
		
		TypePtr dataMemberType = dataMember->type();

		if(!Dictionary::legalKeyType(dataMemberType))
		{
		    cerr << n << ": `" << index.member << "' cannot be used as an index" << endl;
		    return false; 
		}

		if(index.caseSensitive == false)
		{
		    //
		    // Let's check member is a string
		    //
		    BuiltinPtr memberType = BuiltinPtr::dynamicCast(dataMemberType);
		    if(memberType == 0 || memberType->kind() != Builtin::KindString)
		    {
			cerr << n << ": `" << index.member << "' is not a string " << endl;
			return false;
		    }
		}
		indexTypes.push_back(dataMemberType);
	    }
	}
	writeDictWithIndicesH(name, dict, indexTypes, keyType, valueType, H, dllExport);
    }


    for(q = scope.begin(); q != scope.end(); ++q)
    {
	H << sp;
	H << nl << '}';
    }

    writeCodecC(keyType, absolute + "KeyCodec", "Key", false, C);
    writeCodecC(valueType, absolute + "ValueCodec", "Value", true, C);
    
    if(indexTypes.size() > 0)
    {
	writeDictWithIndicesC(name, absolute, dict, indexTypes, keyType, valueType, C);
    }

    return true;
}


void
writeIndexH(const string& memberTypeString, const string& name, Output& H, const string& dllExport)
{
    H << sp << nl << "class " << dllExport << name
      << " : public Freeze::Index";
    H << sb;
    H.dec();
    H << sp << nl << "public:";
    H << sp;
    H.inc();
    
    H << nl << name << "(const std::string&, const std::string& = \"\");";
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

    C << sp << nl << fullName << "::" << name 
      << "(const ::std::string& __name, const ::std::string& __facet)";
    C.inc();
    C << nl << ": Freeze::Index(__name, __facet)";
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
    C << nl << "::std::vector<Ice::Byte>(__stream.b.begin(), __stream.b.end()).swap(__bytes);";
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
    string headerExtension;
    string sourceExtension;
    vector<string> includePaths;
    string include;
    string dllExport;
    vector<Dict> dicts;
    vector<Index> indices;
    string output;
    bool debug = false;
    bool ice = false;
    bool caseSensitive = false;

    IceUtil::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("", "header-ext", IceUtil::Options::NeedArg, "h");
    opts.addOpt("", "source-ext", IceUtil::Options::NeedArg, "cpp");
    opts.addOpt("D", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("U", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("I", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("", "include-dir", IceUtil::Options::NeedArg);
    opts.addOpt("", "dll-export", IceUtil::Options::NeedArg);
    opts.addOpt("", "dict", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("", "index", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("", "dict-index", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("", "output-dir", IceUtil::Options::NeedArg);
    opts.addOpt("d", "debug");
    opts.addOpt("", "ice");
    opts.addOpt("", "case-sensitive");
     
    vector<string> args;
    try
    {
        args = opts.parse(argc, argv);
    }
    catch(const IceUtil::Options::BadOpt& e)
    {
	cerr << argv[0] << ": " << e.reason << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    if(opts.isSet("h") || opts.isSet("help"))
    {
	usage(argv[0]);
	return EXIT_SUCCESS;
    }
    if(opts.isSet("v") || opts.isSet("version"))
    {
	cout << ICE_STRING_VERSION << endl;
	return EXIT_SUCCESS;
    }

    headerExtension = opts.optArg("header-ext");
    sourceExtension = opts.optArg("source-ext");

    if(opts.isSet("D"))
    {
	vector<string> optargs = opts.argVec("D");
	for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
	{
	    cppArgs += " -D" + *i;
	}
    }
    if(opts.isSet("U"))
    {
	vector<string> optargs = opts.argVec("U");
	for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
	{
	    cppArgs += " -U" + *i;
	}
    }
    if(opts.isSet("I"))
    {
	includePaths = opts.argVec("I");
	for(vector<string>::const_iterator i = includePaths.begin(); i != includePaths.end(); ++i)
	{
	    cppArgs += " -I" + *i;
	}
    }
    if(opts.isSet("include-dir"))
    {
	include = opts.optArg("include-dir");
    }
    if(opts.isSet("dll-export"))
    {
	dllExport = opts.optArg("dll-export");
    }
    if(opts.isSet("dict"))
    {
	vector<string> args = opts.argVec("dict");
	for(vector<string>::const_iterator i = args.begin(); i != args.end(); ++i)
	{
	    string s = *i;
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
		cerr << argv[0] << ": " << *i << ": no name specified" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }

	    if(dict.key.empty())
	    {
		cerr << argv[0] << ": " << *i << ": no key specified" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }

	    if(dict.value.empty())
	    {
		cerr << argv[0] << ": " << *i << ": no value specified" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }

	    dicts.push_back(dict);
	}
    }
    if(opts.isSet("index"))
    {
	vector<string> args = opts.argVec("index");
	for(vector<string>::const_iterator i = args.begin(); i != args.end(); ++i)
	{
	    string s = *i;
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
		cerr << argv[0] << ": " << *i << ": no name specified" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }

	    if(index.type.empty())
	    {
		cerr << argv[0] << ": " << *i << ": no type specified" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }

	    if(index.member.empty())
	    {
		cerr << argv[0] << ": " << *i << ": no member specified" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }
	    
	    if(caseString != "case-sensitive" && caseString != "case-insensitive")
	    {
		cerr << argv[0] << ": " << *i << ": the case can be `case-sensitive' or `case-insensitive'" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }
	    index.caseSensitive = (caseString == "case-sensitive");

	    indices.push_back(index);
	}
    }
    if(opts.isSet("dict-index"))
    {
	vector<string> args = opts.argVec("dict-index");
	for(vector<string>::const_iterator i = args.begin(); i != args.end(); ++i)
	{
	    string s = *i;
	    s.erase(remove_if(s.begin(), s.end(), ::isspace), s.end());
	    
	    string dictName;
	    DictIndex index;
	    string::size_type pos;
	  
	    string caseString = "case-sensitive";
	    pos = s.find(',');
	    if(pos != string::npos)
	    {
		dictName = s.substr(0, pos);
		s.erase(0, pos + 1);

		pos = s.find(',');
		if(pos != string::npos)
		{
		    index.member = s.substr(0, pos);
		    s.erase(0, pos + 1);
		    caseString = s;
		}
		else
		{
		    if(s == "case-sensitive" || s == "case-insensitive")
		    {
			caseString = s;
		    }
		    else
		    {
			index.member = s;
		    }
		}
	    }
	    else
	    {
		dictName = s;
	    }

	    if(dictName.empty())
	    {
		cerr << argv[0] << ": " << *i << ": no dictionary specified" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }

	    if(caseString != "case-sensitive" && caseString != "case-insensitive")
	    {
		cerr << argv[0] << ": " << *i << ": the case can be `case-sensitive' or `case-insensitive'" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }
	    index.caseSensitive = (caseString == "case-sensitive");

	    bool found = false;
	    for(vector<Dict>::iterator p = dicts.begin(); p != dicts.end(); ++p)
	    {
		if(p->name == dictName)
		{
		    if(find(p->indices.begin(), p->indices.end(), index) != p->indices.end())
		    {
			cerr << argv[0] << ": --dict-index " << *i << ": this dict-index is defined twice" << endl;
			return EXIT_FAILURE;
		    }
		    p->indices.push_back(index);
		    found = true;
		    break;
		}
	    }
	    if(!found)
	    {
		cerr << argv[0] << ": " << *i << ": unknown dictionary" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	    }
	}
    }
    if(opts.isSet("output-dir"))
    {
	output = opts.optArg("output-dir");
    }
    debug = opts.isSet("d") || opts.isSet("debug");
    ice = opts.isSet("ice");
    caseSensitive = opts.isSet("case-sensitive");

    if(dicts.empty() && indices.empty())
    {
	cerr << argv[0] << ": no Freeze types specified" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    if(args.empty())
    {
	cerr << argv[0] << ": no file name base specified" << endl;
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    string fileH = args[0];
    fileH += "." + headerExtension;
    string fileC = args[0];
    fileC += "." + sourceExtension;
    if(!output.empty())
    {
	fileH = output + '/' + fileH;
	fileC = output + '/' + fileC;
    }

    UnitPtr u = Unit::createUnit(true, false, ice, caseSensitive);

    StringList includes;

    int status = EXIT_SUCCESS;

    for(vector<string>::size_type idx = 1; idx < args.size(); ++idx)
    {
	Preprocessor icecpp(argv[0], args[idx], cppArgs);

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
	
	status = u->parse(cppHandle, debug, false);

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
		    if(!writeDict(argv[0], u, *p, H, C, dllExport))
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
