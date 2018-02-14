// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/Options.h>
#include <IceUtil/CtrlCHandler.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <Slice/Preprocessor.h>
#include <Slice/CPlusPlusUtil.h>
#include <Slice/FileTracker.h>
#include <Slice/Util.h>
#include <IceUtil/OutputUtil.h>
#include <IceUtil/StringUtil.h>
#include <cstring>

using namespace std;
using namespace IceUtil;
using namespace IceUtilInternal;
using namespace Slice;

namespace
{

IceUtil::Mutex* globalMutex = 0;
bool interrupted = false;

class Init
{
public:

    Init()
    {
        globalMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete globalMutex;
        globalMutex = 0;
    }
};

Init init;

string ICE_ENCODING_COMPARE = "Freeze::IceEncodingCompare";

}

void
interruptedCallback(int /*signal*/)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(globalMutex);

    interrupted = true;
}

class MetaDataVisitor : public ParserVisitor
{
public:

    MetaDataVisitor() :
        _useWstring(false)
    {
    }

    virtual bool visitModuleStart(const ModulePtr& p)
    {
        setUseWstring(p);
        return true;
    }

    virtual void visitModuleEnd(const ModulePtr&)
    {
        resetUseWstring();
    }

    virtual bool visitClassDefStart(const ClassDefPtr& p)
    {
        setUseWstring(p);
        checkMetaData(p->dataMembers());
        resetUseWstring();
        return true;
    }

    virtual bool visitStructStart(const StructPtr& p)
    {
        setUseWstring(p);
        checkMetaData(p->dataMembers());
        resetUseWstring();
        return true;
    }

private:

    void checkMetaData(const DataMemberList& dataMembers)
    {
        for(DataMemberList::const_iterator p = dataMembers.begin(); p != dataMembers.end(); ++p)
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast((*p)->type());
            if(builtin && builtin->kind() == Builtin::KindString)
            {
                StringList metaData = (*p)->getMetaData();
                for(StringList::const_iterator q = metaData.begin(); q != metaData.end(); ++q)
                {
                    if(*q == "cpp:type:string" || *q == "cpp:type:wstring")
                    {
                        continue;
                    }
                }
                metaData.push_back(_useWstring ? "cpp:type:wstring" : "cpp:type:string");
                (*p)->setMetaData(metaData);
            }
        }
    }

    void setUseWstring(ContainedPtr p)
    {
        _useWstringHist.push_back(_useWstring);
        StringList metaData = p->getMetaData();
        if(find(metaData.begin(), metaData.end(), "cpp:type:wstring") != metaData.end())
        {
            _useWstring = true;
        }
        else if(find(metaData.begin(), metaData.end(), "cpp:type:string") != metaData.end())
        {
            _useWstring = false;
        }
    }

    void resetUseWstring()
    {
        _useWstring = _useWstringHist.back();
        _useWstringHist.pop_back();
    }

    bool _useWstring;
    std::list<bool> _useWstringHist;
};

struct DictIndex
{
    string member;
    bool caseSensitive;
    bool sort;
    string userCompare;

    bool operator==(const DictIndex& rhs) const
    {
        return member == rhs.member;
    }

    bool operator!=(const DictIndex& rhs) const
    {
        return member != rhs.member;
    }
};

struct Dict
{
    string name;
    string key;
    StringList keyMetaData;
    string value;
    StringList valueMetaData;
    bool sort;
    string userCompare;

    vector<DictIndex> indices;
};

struct Index
{
    string name;
    string type;
    string member;
    bool caseSensitive;
};

struct IndexType
{
    TypePtr type;
    StringList metaData;
};

void
usage(const string& n)
{
    getErrorStream() << "Usage: " << n << " [options] file-base [slice-files...]\n";
    getErrorStream() <<
        "Options:\n"
        "-h, --help            Show this message.\n"
        "-v, --version         Display the Ice version.\n"
        "--validate            Validate command line options.\n"
        "--header-ext EXT      Use EXT instead of the default `h' extension.\n"
        "--source-ext EXT      Use EXT instead of the default `cpp' extension.\n"
        "--add-header HDR[,GUARD]\n"
        "                      Add #include for HDR (with guard GUARD) to\n"
        "                      generated source file.\n"
        "-DNAME                Define NAME as 1.\n"
        "-DNAME=DEF            Define NAME as DEF.\n"
        "-UNAME                Remove any definition for NAME.\n"
        "-IDIR                 Put DIR in the include file search path.\n"
        "-E                    Print preprocessor output on stdout.\n"
        "--include-dir DIR     Use DIR as the header include directory in\n"
        "                      source files.\n"
        "--dll-export SYMBOL   Use SYMBOL for DLL exports.\n"
        "--dict NAME,KEY,VALUE[,sort[,COMPARE]]\n"
        "                      Create a Freeze dictionary with the name NAME,\n"
        "                      using KEY as key, and VALUE as value. This\n"
        "                      option may be specified multiple times for\n"
        "                      different names. NAME may be a scoped name.\n"
        "                      By default, keys are sorted using their binary\n"
        "                      Ice-encoding representation. Use 'sort' to sort\n"
        "                      with the COMPARE functor class. COMPARE's default\n"
        "                      value is std::less<KEY>\n"
        "--index NAME,TYPE,MEMBER[,{case-sensitive|case-insensitive}]\n"
        "                      Create a Freeze evictor index with the name\n"
        "                      NAME for member MEMBER of class TYPE. This\n"
        "                      option may be specified multiple times for\n"
        "                      different names. NAME may be a scoped name.\n"
        "                      When member is a string, the case can be\n"
        "                      sensitive or insensitive (default is sensitive).\n"
        "--dict-index DICT[,MEMBER][,{case-sensitive|case-insensitive}]\n"
        "                 [,sort[,COMPARE]]\n"
        "                      Add an index to dictionary DICT. If MEMBER is \n"
        "                      specified, then DICT's VALUE must be a class or\n"
        "                      a struct, and MEMBER must designate a member of\n"
        "                      VALUE. Otherwise, the entire VALUE is used for \n"
        "                      indexing. When the secondary key is a string, \n"
        "                      the case can be sensitive or insensitive (default\n"
        "                      is sensitive).\n"
        "                      By default, keys are sorted using their binary\n"
        "                      Ice-encoding representation. Use 'sort' to sort\n"
        "                      with the COMPARE functor class. COMPARE's default\n"
        "                      value is std::less<secondary key type>.\n"
        "--output-dir DIR      Create files in the directory DIR.\n"
        "--depend              Generate dependencies for input Slice files.\n"
        "--depend-xml          Generate dependencies in XML format.\n"
        "--depend-file FILE    Write dependencies to FILE instead of standard output.\n"
        "-d, --debug           Print debug messages.\n"
        "--ice                 Allow reserved Ice prefix in Slice identifiers.\n"
        "--underscore          Allow underscores in Slice identifiers.\n"
        ;
}

void
checkIdentifier(string t, string s)
{
    if(s.empty() || (!IceUtilInternal::isAlpha(s[0]) && s[0] != '_'))
    {
        ostringstream os;
        os << t << "' is not a valid type name";
        throw os.str();
    }

    for(unsigned int i = 1; i < s.size(); ++i)
    {
        if(!isalnum(static_cast<unsigned char>(s[i])) && s[i] != '_')
        {
            ostringstream os;
            os << t << "' is not a valid type name";
            throw os.str();
        }
    }
}

void
printFreezeTypes(Output& out, const vector<Dict>& dicts, const vector<Index>& indices)
{
    out << '\n';
    out << "\n// Freeze types in this file:";
    for(vector<Dict>::const_iterator p = dicts.begin(); p != dicts.end(); ++p)
    {
        out << "\n// name=\"" << p->name << "\", key=\""
            << p->key << "\", value=\"" << p->value << "\"";
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

template<class T>
inline string
getCompare(const T& t, const string& keyType)
{
    if(t.sort)
    {
        if(t.userCompare == "")
        {
            return "std::less< " + keyType + ">";
        }
        else
        {
            return t.userCompare;
        }
    }
    else
    {
        return ICE_ENCODING_COMPARE;
    }
}

string
getTypeId(const TypePtr& type, const StringList& metaData)
{
    string typeId = type->typeId();
    BuiltinPtr builtInType = BuiltinPtr::dynamicCast(type);
    if(builtInType &&  builtInType->kind() == Builtin::KindString && metaData.size() != 0 &&
       metaData.front() == "cpp:type:wstring")
    {
        typeId = "wstring";
    }
    return typeId;
}

void
writeDictH(const string& name, const Dict& dict, const vector<IndexType> indexTypes, const TypePtr& keyType,
           const StringList& keyMetaData, const TypePtr& valueType, const StringList& valueMetaData, Output& H,
           const string& dllExport)
{
    const string keyTypeS = typeToString(keyType, keyMetaData);
    const string valueTypeS = typeToString(valueType, valueMetaData);
    const string compare = getCompare(dict, keyTypeS);
    const string keyCodec = string("::Freeze::MapKeyCodec< ") + keyTypeS + " >";
    const string valueCodec =
        string(valueType->usesClasses() ? "::Freeze::MapObjectValueCodec" : "::Freeze::MapValueCodec") +
        "< " + valueTypeS + " >";

    const string templateParams = string("< ") + keyTypeS + ", " + valueTypeS + ", " + keyCodec + ", " + valueCodec +
        ", " + compare + " >";

    const string keyCompareParams = string("< ") + keyTypeS + ", " + keyCodec + ", " + compare + " >";

    vector<string> capitalizedMembers;

    for(size_t i = 0; i < dict.indices.size(); ++i)
    {
        const string& member = dict.indices[i].member;
        if(!member.empty())
        {
            string capitalizedMember = member;
            capitalizedMember[0] = toupper(static_cast<unsigned char>(capitalizedMember[0]));
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
    /*
    H << nl << "typedef std::pair<const " << keyTypeS << ", const " << valueTypeS << " > value_type;";

    H << nl << "typedef Freeze::Iterator" << templateParams << " iterator;";
    H << nl << "typedef Freeze::ConstIterator" << templateParams << " const_iterator;";
    H << nl << "typedef size_t size_type;";
    H << nl << "typedef ptrdiff_t difference_type;";
    */

    //
    // Nested index classes
    //

    for(size_t i = 0; i < capitalizedMembers.size(); ++i)
    {
        string className = capitalizedMembers[i] + "Index";

        string indexCompare = getCompare(dict.indices[i], typeToString(indexTypes[i].type, indexTypes[i].metaData));

        string indexCompareParams = string("< ") + typeToString(indexTypes[i].type, indexTypes[i].metaData) + ", "
            + className + ", " + indexCompare + " >";

        H << sp << nl << "class " << dllExport << className
          << " : public Freeze::MapIndex" << indexCompareParams;
        H << sb;

        H.dec();
        H << sp << nl << "public:";
        H << sp;
        H.inc();
        H << nl << capitalizedMembers[i] << "Index(const std::string&, const "
          << indexCompare << "& = " << indexCompare << "());";

        H << sp;

        //
        // Codec
        //
        H << nl << "static void write(" << inputTypeToString(indexTypes[i].type, 0, indexTypes[i].metaData)
          << ", Freeze::Key&, const Ice::CommunicatorPtr&, const Ice::EncodingVersion&);";

        H << nl << "static void read("
          << typeToString(indexTypes[i].type, indexTypes[i].metaData)
          << "&, const Freeze::Key&, const ::Ice::CommunicatorPtr&, const Ice::EncodingVersion&);";

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
    H << nl << name << "(const Freeze::ConnectionPtr&, const std::string&, "
      << "bool = true, const " << compare << "& = " << compare << "());";
    H << sp;
    H << nl << "template <class _InputIterator>"
      << nl << name << "(const Freeze::ConnectionPtr& __connection, "
      << "const std::string& __dbName, bool __createDb, _InputIterator __first, _InputIterator __last, "
      << "const " << compare << "& __compare = " << compare << "())";
    H.inc();
    if(capitalizedMembers.empty())
    {
        H << nl << ": Freeze::Map" << templateParams <<"(__connection, __dbName, keyTypeId(), valueTypeId(), "
          << "__createDb, __first, __last, __compare)";
        H.dec();
        H << sb;
        H << eb;
    }
    else
    {
        H << nl << ": Freeze::Map" << templateParams <<"(__connection->getCommunicator(), __connection->getEncoding())";
        H.dec();
        H << sb;
        H << nl << "Freeze::KeyCompareBasePtr __keyCompare = "
          << "new Freeze::KeyCompare" << keyCompareParams << "(__compare, this->_communicator, this->_encoding);";
        H << nl << "std::vector<Freeze::MapIndexBasePtr> __indices;";
        for(size_t i = 0; i < capitalizedMembers.size(); ++i)
        {
            string indexName = dict.indices[i].member;
            if(indexName.empty())
            {
                indexName = "index";
            }
            indexName = string("\"") + indexName + "\"";

            H << nl << "__indices.push_back(new " << capitalizedMembers[i] << "Index(" << indexName << "));";
        }
        H << nl << "this->_helper.reset(Freeze::MapHelper::create(__connection, __dbName, keyTypeId(), valueTypeId(), "
          << "__keyCompare, __indices, __createDb));";
        H << nl << "while(__first != __last)";
        H << sb;
        H << nl << "put(*__first);";
        H << nl << "++__first;";
        H << eb;
        H << eb;

        //
        // Recreate
        //
        H << sp << nl << "static void recreate(const Freeze::ConnectionPtr&, const std::string&, "
          << "const " << compare << "& = " << compare << "());";
    }

    H << sp;
    H << nl << "static std::string keyTypeId();";
    H << nl << "static std::string valueTypeId();";

    //
    // Find, begin, lowerBound, upperBound, equalRange and count functions
    //
    for(size_t i = 0; i < capitalizedMembers.size(); ++i)
    {
        H << sp;
        H << nl << "iterator findBy" << capitalizedMembers[i]
          << "(" << inputTypeToString(indexTypes[i].type, false, indexTypes[i].metaData) << ", bool = true);";
        H << nl << "const_iterator findBy" << capitalizedMembers[i]
          << "(" << inputTypeToString(indexTypes[i].type, false, indexTypes[i].metaData) << ", bool = true) const;";

        H << nl << "iterator beginFor" << capitalizedMembers[i] << "();";
        H << nl << "const_iterator beginFor" << capitalizedMembers[i] << "() const;";

        H << nl << "iterator endFor" << capitalizedMembers[i] << "();";
        H << nl << "const_iterator endFor" << capitalizedMembers[i] << "() const;";

        H << nl << "iterator lowerBoundFor" << capitalizedMembers[i]
          << "(" << inputTypeToString(indexTypes[i].type, false, indexTypes[i].metaData) << ");";
        H << nl << "const_iterator lowerBoundFor" << capitalizedMembers[i]
          << "(" << inputTypeToString(indexTypes[i].type, false, indexTypes[i].metaData) << ") const;";

        H << nl << "iterator upperBoundFor" << capitalizedMembers[i]
          << "(" << inputTypeToString(indexTypes[i].type, false, indexTypes[i].metaData) << ");";
        H << nl << "const_iterator upperBoundFor" << capitalizedMembers[i]
          << "(" << inputTypeToString(indexTypes[i].type, false, indexTypes[i].metaData) << ") const;";

        H << nl << "std::pair<iterator, iterator> equalRangeFor"
          << capitalizedMembers[i] << "(" << inputTypeToString(indexTypes[i].type, false, indexTypes[i].metaData)
          << ");";

        H << nl << "std::pair<const_iterator, const_iterator> equalRangeFor"
          << capitalizedMembers[i] << "(" << inputTypeToString(indexTypes[i].type, false, indexTypes[i].metaData)
          << ") const;";

        string countFunction = dict.indices[i].member.empty() ? string("valueCount")
            : dict.indices[i].member + "Count";

        H << nl << "int " << countFunction
          << "(" << inputTypeToString(indexTypes[i].type, false, indexTypes[i].metaData) << ") const;";

    }

    H << eb << ';';
}

void
writeDictC(const string& name, const string& absolute, const Dict& dict, const vector<IndexType> indexTypes,
           const TypePtr& keyType, const StringList& keyMetaData, const TypePtr& valueType,
           const StringList& valueMetaData, Output& C)
{
    const string keyTypeS = typeToString(keyType, keyMetaData);
    const string valueTypeS = typeToString(valueType, valueMetaData);
    const string compare = getCompare(dict, keyTypeS);
    const string keyCodec = string("::Freeze::MapKeyCodec< ") + keyTypeS + " >";
    const string valueCodec =
        string(valueType->usesClasses() ? "::Freeze::MapObjectValueCodec" : "::Freeze::MapValueCodec") +
        "< " + valueTypeS + " >";

    const string templateParams = string("< ") + keyTypeS + ", " + valueTypeS + ", " + keyCodec + ", " + valueCodec +
        ", " + compare + " >";

    const string keyCompareParams = string("< ") + keyTypeS + ", " + keyCodec + ", " + compare + " >";

    vector<string> capitalizedMembers;

    for(size_t i = 0; i < dict.indices.size(); ++i)
    {
        const string& member = dict.indices[i].member;
        if(!member.empty())
        {
            string capitalizedMember = member;
            capitalizedMember[0] = toupper(static_cast<unsigned char>(capitalizedMember[0]));
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
    for(size_t i = 0; i < capitalizedMembers.size(); ++i)
    {
        string className = capitalizedMembers[i] + "Index";

        string indexCompare = getCompare(dict.indices[i], typeToString(indexTypes[i].type, indexTypes[i].metaData));

        string indexCompareParams = string("< ") + typeToString(indexTypes[i].type, indexTypes[i].metaData) + ", "
            + className + ", " + indexCompare + " >";

        C << sp << nl << absolute << "::" << className << "::" << className
          << "(const std::string& __name, "
          << "const " << indexCompare << "& __compare)";

        C.inc();
        C << nl << ": Freeze::MapIndex" << indexCompareParams << "(__name, __compare)";
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
            C << nl << valueTypeS << " __x;";
            C << nl << valueCodec << "::read(__x, __v, _communicator, _encoding);";
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
            C << nl << "write(" << param << ", __k, _communicator, _encoding);";
        }
        C << eb;

        C << sp << nl << "void"
          << nl << absolute << "::" << className << "::"
          << "write(" << inputTypeToString(indexTypes[i].type, false, indexTypes[i].metaData)
          << " __index, Freeze::Key& __bytes, const Ice::CommunicatorPtr& __communicator, "
          << "const Ice::EncodingVersion& __encoding)";
        C << sb;

        if(optimize)
        {
            C << nl << valueCodec << "::write(__index, __bytes, __communicator, __encoding);";
        }
        else
        {
            assert(!indexTypes[i].type->usesClasses());

            C << nl << "IceInternal::InstancePtr __instance = IceInternal::getInstance(__communicator);";
            C << nl << "IceInternal::BasicStream __stream(__instance.get(), __encoding);";

            string valueS;
            if(dict.indices[i].caseSensitive)
            {
                valueS = "__index";
            }
            else
            {
                C << nl << typeToString(indexTypes[i].type, indexTypes[i].metaData)
                  << " __lowerCaseIndex = IceUtilInternal::toLower(__index);";
                valueS = "__lowerCaseIndex";
            }

            writeMarshalUnmarshalCode(C, indexTypes[i].type, false, 0, valueS, true, indexTypes[i].metaData, 0,
                                      "__stream", false);
            C << nl << "::std::vector<Ice::Byte>(__stream.b.begin(), __stream.b.end()).swap(__bytes);";
        }
        C << eb;

        C << sp << nl << "void"
          << nl << absolute << "::" << className << "::"
          << "read(" << typeToString(indexTypes[i].type, indexTypes[i].metaData)
          << "& __index, const Freeze::Key& __bytes, const Ice::CommunicatorPtr& __communicator, "
          << "const Ice::EncodingVersion& __encoding)";
        C << sb;

        if(optimize)
        {
            C << nl << valueCodec << "::read(__index, __bytes, __communicator, __encoding);";
        }
        else
        {
            C << nl << "IceInternal::InstancePtr __instance = IceInternal::getInstance(__communicator);";
            C << nl << "IceInternal::BasicStream __stream(__instance.get(), __encoding, ";
            C << "&__bytes[0], &__bytes[0] + __bytes.size());";

            writeMarshalUnmarshalCode(C, indexTypes[i].type, false, 0, "__index", false, indexTypes[i].metaData, 0,
                                      "__stream", false);
        }
        C << eb;
    }

    //
    // Constructor
    //
    C << sp << nl << absolute << "::" << name
      << "(const Freeze::ConnectionPtr& __connection, const std::string& __dbName ,"
      << "bool __createDb, const " << compare << "& __compare)";
    if(capitalizedMembers.empty())
    {
        C.inc();
        C << nl << ": Freeze::Map" << templateParams
          <<"(__connection, __dbName, keyTypeId(), valueTypeId(), __createDb, __compare)";
        C.dec();
        C << sb;
        C << eb;
    }
    else
    {
        C.inc();
        C << nl << ": Freeze::Map" << templateParams <<"(__connection->getCommunicator(), __connection->getEncoding())";
        C.dec();
        C << sb;
        C << nl << "Freeze::KeyCompareBasePtr __keyCompare = "
          << "new Freeze::KeyCompare" << keyCompareParams << "(__compare, _communicator, _encoding);";
        C << nl << "std::vector<Freeze::MapIndexBasePtr> __indices;";
        for(size_t i = 0; i < capitalizedMembers.size(); ++i)
        {
            string indexName = dict.indices[i].member;
            if(indexName.empty())
            {
                indexName = "index";
            }
            indexName = string("\"") + indexName + "\"";

            C << nl << "__indices.push_back(new " << capitalizedMembers[i] << "Index(" << indexName << "));";
        }
        C << nl << "_helper.reset(Freeze::MapHelper::create(__connection, __dbName, keyTypeId(), valueTypeId(), "
          << "__keyCompare, __indices, __createDb));";
        C << eb;

        //
        // Recreate
        //
        C << sp << nl << "void"
          << nl << absolute
          << "::recreate(const Freeze::ConnectionPtr& __connection, const std::string& __dbName ,"
          << " const " << compare << "& __compare)";
        C << sb;
        C << nl << "Freeze::KeyCompareBasePtr __keyCompare = "
          << "new Freeze::KeyCompare" << keyCompareParams << "(__compare, __connection->getCommunicator()"
          <<", __connection->getEncoding());";
        C << nl << "std::vector<Freeze::MapIndexBasePtr> __indices;";
        for(size_t i = 0; i < capitalizedMembers.size(); ++i)
        {
            string indexName = dict.indices[i].member;
            if(indexName.empty())
            {
                indexName = "index";
            }
            indexName = string("\"") + indexName + "\"";

            C << nl << "__indices.push_back(new " << capitalizedMembers[i] << "Index(" << indexName << "));";
        }
        C << nl << "Freeze::MapHelper::recreate(__connection, __dbName, keyTypeId(), valueTypeId(), __keyCompare, "
          << "__indices);";
        C << eb;
    }

    C << sp << nl << "std::string"
      << nl << absolute << "::keyTypeId()";
    C << sb;
    C << nl << "return \"" << getTypeId(keyType, keyMetaData) << "\";";
    C << eb;
    C << sp << nl << "std::string"
      << nl << absolute << "::valueTypeId()";
    C << sb;
    C << nl << "return \"" << getTypeId(valueType, valueMetaData) << "\";";
    C << eb;

    //
    // Find and count functions
    //
    for(size_t i = 0; i < capitalizedMembers.size(); ++i)
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
          << "(" << inputTypeToString(indexTypes[i].type, false, indexTypes[i].metaData)
          << " __index, bool __onlyDups)";
        C << sb;
        C << nl << "Freeze::Key __bytes;";
        C << nl << indexClassName << "::" << "write(__index, __bytes, _communicator, _encoding);";
        C << nl << "return iterator(_helper->index(" << indexName
          << ")->untypedFind(__bytes, false, __onlyDups), _communicator, _encoding);";
        C << eb;

        C << sp << nl << absolute << "::const_iterator"
          << nl << absolute << "::" << "findBy" << capitalizedMembers[i]
          << "(" << inputTypeToString(indexTypes[i].type, false, indexTypes[i].metaData)
          << " __index, bool __onlyDups) const";
        C << sb;
        C << nl << "Freeze::Key __bytes;";
        C << nl << indexClassName << "::" << "write(__index, __bytes, _communicator, _encoding);";
        C << nl << "return const_iterator(_helper->index(" << indexName
          << ")->untypedFind(__bytes, true, __onlyDups), _communicator, _encoding);";
        C << eb;

        C << sp << nl << absolute << "::iterator"
          << nl << absolute << "::" << "beginFor" << capitalizedMembers[i] << "()";
        C << sb;
        C << nl << "return iterator(_helper->index(" << indexName << ")->begin(false), _communicator, _encoding);";
        C << eb;

        C << sp << nl << absolute << "::const_iterator"
          << nl << absolute << "::" << "beginFor" << capitalizedMembers[i] << "() const";
        C << sb;
        C << nl << "return const_iterator(_helper->index(" << indexName << ")->begin(true), _communicator, _encoding);";
        C << eb;

        C << sp << nl << absolute << "::iterator"
          << nl << absolute << "::" << "endFor" << capitalizedMembers[i] << "()";
        C << sb;
        C << nl << "return iterator();";
        C << eb;

        C << sp << nl << absolute << "::const_iterator"
          << nl << absolute << "::" << "endFor" << capitalizedMembers[i] << "() const";
        C << sb;
        C << nl << "return const_iterator();";
        C << eb;

        C << sp << nl << absolute << "::iterator"
          << nl << absolute << "::" << "lowerBoundFor" << capitalizedMembers[i]
          << "(" << inputTypeToString(indexTypes[i].type, false, indexTypes[i].metaData) << " __index)";
        C << sb;
        C << nl << "Freeze::Key __bytes;";
        C << nl << indexClassName << "::" << "write(__index, __bytes, _communicator, _encoding);";
        C << nl << "return iterator(_helper->index(" << indexName
          << ")->untypedLowerBound(__bytes, false), _communicator, _encoding);";
        C << eb;

        C << sp << nl << absolute << "::const_iterator"
          << nl << absolute << "::" << "lowerBoundFor" << capitalizedMembers[i]
          << "(" << inputTypeToString(indexTypes[i].type, false, indexTypes[i].metaData) << " __index) const";
        C << sb;
        C << nl << "Freeze::Key __bytes;";
        C << nl << indexClassName << "::" << "write(__index, __bytes, _communicator, _encoding);";
        C << nl << "return const_iterator(_helper->index(" << indexName
          << ")->untypedLowerBound(__bytes, true), _communicator, _encoding);";
        C << eb;

        C << sp << nl << absolute << "::iterator"
          << nl << absolute << "::" << "upperBoundFor" << capitalizedMembers[i]
          << "(" << inputTypeToString(indexTypes[i].type, false, indexTypes[i].metaData) << " __index)";
        C << sb;
        C << nl << "Freeze::Key __bytes;";
        C << nl << indexClassName << "::" << "write(__index, __bytes, _communicator, _encoding);";
        C << nl << "return iterator(_helper->index(" << indexName
          << ")->untypedUpperBound(__bytes, false), _communicator, _encoding);";
        C << eb;

        C << sp << nl << absolute << "::const_iterator"
          << nl << absolute << "::" << "upperBoundFor" << capitalizedMembers[i]
          << "(" << inputTypeToString(indexTypes[i].type, false, indexTypes[i].metaData) << " __index) const";
        C << sb;
        C << nl << "Freeze::Key __bytes;";
        C << nl << indexClassName << "::" << "write(__index, __bytes, _communicator, _encoding);";
        C << nl << "return const_iterator(_helper->index(" << indexName
          << ")->untypedUpperBound(__bytes, true), _communicator, _encoding);";
        C << eb;

        C << sp << nl << "std::pair<" << absolute << "::iterator, "
          << absolute << "::iterator>"
          << nl << absolute << "::" << "equalRangeFor" << capitalizedMembers[i]
          << "(" << inputTypeToString(indexTypes[i].type, false, indexTypes[i].metaData) << " __index)";
        C << sb;
        C << nl << "return std::make_pair(lowerBoundFor" << capitalizedMembers[i]
          << "(__index), upperBoundFor" << capitalizedMembers[i] << "(__index));";
        C << eb;

        C << sp << nl << "std::pair<" << absolute << "::const_iterator, "
          << absolute << "::const_iterator>"
          << nl << absolute << "::" << "equalRangeFor" << capitalizedMembers[i]
          << "(" << inputTypeToString(indexTypes[i].type, false, indexTypes[i].metaData) << " __index) const";
        C << sb;
        C << nl << "return std::make_pair(lowerBoundFor" << capitalizedMembers[i]
          << "(__index), upperBoundFor" << capitalizedMembers[i] << "(__index));";
        C << eb;

        string countFunction = dict.indices[i].member.empty() ? string("valueCount")
            : dict.indices[i].member + "Count";

        C << sp << nl << "int"
          << nl << absolute << "::" << countFunction
          << "(" << inputTypeToString(indexTypes[i].type, false, indexTypes[i].metaData) << " __index) const";
        C << sb;
        C << nl << "Freeze::Key __bytes;";
        C << nl << indexClassName << "::" << "write(__index, __bytes, _communicator, _encoding);";
        C << nl << "return _helper->index(" << indexName
          << ")->untypedCount(__bytes);";
        C << eb;
    }
}

void
writeDict(const string& n, const UnitPtr& u, const Dict& dict, Output& H, Output& C, const string& dllExport)
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

        checkIdentifier(absolute, s);

        scope.push_back(s);
    }

    checkIdentifier(absolute, name);

    TypeList keyTypes = u->lookupType(dict.key, false);
    if(keyTypes.empty())
    {
        ostringstream os;
        os << "`" << dict.key << "' is not a valid type";
        throw os.str();
    }
    TypePtr keyType = keyTypes.front();

    TypeList valueTypes = u->lookupType(dict.value, false);
    if(valueTypes.empty())
    {
        ostringstream os;
        os << "`" << dict.value << "' is not a valid type";
        throw os.str();
    }
    TypePtr valueType = valueTypes.front();

    for(vector<string>::const_iterator q = scope.begin(); q != scope.end(); ++q)
    {
        H << sp;
        H << nl << "namespace " << *q << nl << '{';
    }

    vector<IndexType> indexTypes;

    for(vector<DictIndex>::const_iterator p = dict.indices.begin(); p != dict.indices.end(); ++p)
    {
        const DictIndex& index = *p;
        if(index.member.empty())
        {
            if(dict.indices.size() > 1)
            {
                ostringstream os;
                os << "bad index for dictionary `" << dict.name << "'";
                throw os.str();
            }

            bool containsSequence = false;
            if(!Dictionary::legalKeyType(valueType, containsSequence))
            {
                ostringstream os;
                os << "`" << dict.value << "' is not a valid index type";
                throw os.str();
            }
            if(containsSequence)
            {
                getErrorStream() << n << ": warning: use of sequences in dictionary keys has been deprecated";
            }

            if(index.caseSensitive == false)
            {
                //
                // Let's check value is a string
                //

                BuiltinPtr builtInType = BuiltinPtr::dynamicCast(valueType);

                if(builtInType == 0 || builtInType->kind() != Builtin::KindString)
                {
                    ostringstream os;
                    os << "VALUE is a `" << dict.value << "', not a string";
                    throw os.str();
                }
            }
            IndexType iType;
            iType.type = valueType;
            iType.metaData = dict.valueMetaData;
            indexTypes.push_back(iType);
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
                    ostringstream os;
                    os << "`" << dict.value << "' is neither a class nor a struct.";
                    throw os.str();
                }
                dataMembers = structDecl->dataMembers();
            }
            DataMemberList::const_iterator d = dataMembers.begin();
            while(d != dataMembers.end() && dataMember == 0)
            {
                if((*d)->name() == index.member)
                {
                    dataMember = *d;
                }
                else
                {
                    ++d;
                }
            }

            if(dataMember == 0)
            {
                ostringstream os;
                os << "The value of `" << dict.name
                   << "' has no data member named `" << index.member << "'";
                throw os.str();
            }

            TypePtr dataMemberType = dataMember->type();

            bool containsSequence = false;
            if(!Dictionary::legalKeyType(dataMemberType, containsSequence))
            {
                ostringstream os;
                os << "`" << index.member << "' cannot be used as an index";
                throw os.str();
            }
            if(containsSequence)
            {
                getErrorStream() << n << ": warning: use of sequences in dictionary keys has been deprecated";
            }

            if(index.caseSensitive == false)
            {
                //
                // Let's check member is a string
                //
                BuiltinPtr memberType = BuiltinPtr::dynamicCast(dataMemberType);
                if(memberType == 0 || memberType->kind() != Builtin::KindString)
                {
                    ostringstream os;
                    os << "`" << index.member << "' is not a string ";
                    throw os.str();
                }
            }
            IndexType iType;
            iType.type = dataMemberType;
            iType.metaData = dataMember->getMetaData();
            indexTypes.push_back(iType);
        }
    }

    writeDictH(name, dict, indexTypes, keyType, dict.keyMetaData, valueType, dict.valueMetaData, H, dllExport);

    for(vector<string>::const_iterator q = scope.begin(); q != scope.end(); ++q)
    {
        H << sp;
        H << nl << '}';
    }

    writeDictC(name, absolute, dict, indexTypes, keyType, dict.keyMetaData, valueType, dict.valueMetaData, C);
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
    string inputType = inputTypeToString(memberType, false);

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
    C << nl << "IceInternal::BasicStream __stream(__instance.get(), _encoding);";

    string valueS;
    if(caseSensitive)
    {
        valueS = "__index";
    }
    else
    {
        C << nl << typeToString(memberType) << " __lowerCaseIndex = IceUtilInternal::toLower(__index);";
        valueS = "__lowerCaseIndex";
    }

    writeMarshalUnmarshalCode(C, memberType, false, 0, valueS, true, StringList(), 0, "__stream", false);
    if(memberType->usesClasses())
    {
        C << nl << "__stream.writePendingObjects();";
    }
    C << nl << "::std::vector<Ice::Byte>(__stream.b.begin(), __stream.b.end()).swap(__bytes);";
    C << eb;
}

void
writeIndex(const string& /*n*/, const UnitPtr& u, const Index& index, Output& H, Output& C, const string& dllExport)
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

        checkIdentifier(absolute, s);

        scope.push_back(s);
    }

    checkIdentifier(absolute, name);

    TypeList types = u->lookupType(index.type, false);
    if(types.empty())
    {
        ostringstream os;
        os << "`" << index.type << "' is not a valid type";
        throw os.str();
    }
    TypePtr type = types.front();

    ClassDeclPtr classDecl = ClassDeclPtr::dynamicCast(type);
    if(classDecl == 0)
    {
        ostringstream os;
        os << "`" << index.type << "' is not a class";
        throw os.str();
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
        ostringstream os;
        os << "`" << index.type << "' has no data member named `" << index.member << "'";
        throw os.str();
    }

    if(index.caseSensitive == false)
    {
        //
        // Let's check member is a string
        //
        BuiltinPtr memberType = BuiltinPtr::dynamicCast(dataMember->type());
        if(memberType == 0 || memberType->kind() != Builtin::KindString)
        {
            ostringstream os;
            os << "`" << index.member << "'is not a string";
            throw os.str();
        }
    }

    for(vector<string>::const_iterator q = scope.begin(); q != scope.end(); ++q)
    {
        H << sp;
        H << nl << "namespace " << *q << nl << '{';
    }

    writeIndexH(inputTypeToString(dataMember->type(), false), name, H, dllExport);

    for(vector<string>::const_iterator q = scope.begin(); q != scope.end(); ++q)
    {
        H << sp;
        H << nl << '}';
    }

    writeIndexC(type, dataMember->type(), index.member, index.caseSensitive, absolute, name, C);
}

void
gen(const string& name, const UnitPtr& u, const vector<string>& includePaths, const vector<string>& extraHeaders,
    const vector<Dict>& dicts, const vector<Index>& indices, const string& include, const string& headerExtension,
    const string& sourceExtension, string dllExport, const StringList& includes, const vector<string>& args,
    const string& output)
{
    string fileH = args[0];
    fileH += "." + headerExtension;
    string includeH = fileH;
    string fileC = args[0];
    fileC += "." + sourceExtension;

    if(!output.empty())
    {
        fileH = output + '/' + fileH;
        fileC = output + '/' + fileC;
    }

    u->mergeModules();
    u->sort();

    IceUtilInternal::Output H;
    H.open(fileH.c_str());
    if(!H)
    {
        ostringstream os;
        os << "cannot open `" << fileH << "': " << strerror(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }

    FileTracker::instance()->addFile(fileH);

    printHeader(H);
    printGeneratedHeader(H, string(args[0]) + ".ice");


    printFreezeTypes(H, dicts, indices);

    IceUtilInternal::Output CPP;
    CPP.open(fileC.c_str());
    if(!CPP)
    {
        ostringstream os;
        os << "cannot open `" << fileC << "': " << strerror(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }
    FileTracker::instance()->addFile(fileC);

    printHeader(CPP);
    printGeneratedHeader(CPP, string(args[0]) + ".ice");

    printFreezeTypes(CPP, dicts, indices);

    for(vector<string>::const_iterator i = extraHeaders.begin(); i != extraHeaders.end(); ++i)
    {
        string hdr = *i;
        string guard;
        string::size_type pos = hdr.rfind(',');
        if(pos != string::npos)
        {
            hdr = i->substr(0, pos);
            guard = i->substr(pos + 1);
        }
        if(!guard.empty())
        {
            CPP << "\n#ifndef " << guard;
            CPP << "\n#define " << guard;
        }
        CPP << "\n#include <";
        if(!include.empty())
        {
            CPP << include << '/';
        }
        CPP << hdr << '>';
        if(!guard.empty())
        {
            CPP << "\n#endif";
        }
    }

    string s = fileH;
    transform(s.begin(), s.end(), s.begin(), ToIfdef());
    H << "\n#ifndef __" << s << "__";
    H << "\n#define __" << s << "__";
    H << '\n';

    H << "\n#include <IceUtil/PushDisableWarnings.h>";

    if(dicts.size() > 0)
    {
        H << "\n#include <Freeze/Map.h>";
    }

    if(indices.size() > 0)
    {
        H << "\n#include <Freeze/Index.h>";
    }

    for(StringList::const_iterator p = includes.begin(); p != includes.end(); ++p)
    {
        H << "\n#include <" << changeInclude(*p, includePaths) << "." + headerExtension + ">";
    }


    CPP << "\n#include <IceUtil/PushDisableWarnings.h>";
    CPP << "\n#include <Ice/BasicStream.h>";
    CPP << "\n#include <IceUtil/StringUtil.h>";
    CPP << "\n#include <IceUtil/PopDisableWarnings.h>";
    CPP << "\n#include <";
    if(include.size())
    {
        CPP << include << '/';
    }
    CPP << includeH << '>';

    printVersionCheck(H);
    printVersionCheck(CPP);

    printDllExportStuff(H, dllExport);
    if(dllExport.size())
    {
        dllExport += " ";
    }

    for(vector<Dict>::const_iterator p = dicts.begin(); p != dicts.end(); ++p)
    {
        writeDict(name, u, *p, H, CPP, dllExport);
    }

    for(vector<Index>::const_iterator q = indices.begin(); q != indices.end(); ++q)
    {
        writeIndex(name, u, *q, H, CPP, dllExport);
    }

    H << "\n\n#include <IceUtil/PopDisableWarnings.h>";
    H << "\n#endif\n";
    CPP << '\n';

    H.close();
    CPP.close();
}

int
compile(const vector<string>& argv)
{
    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("", "validate");
    opts.addOpt("", "header-ext", IceUtilInternal::Options::NeedArg, "h");
    opts.addOpt("", "source-ext", IceUtilInternal::Options::NeedArg, "cpp");
    opts.addOpt("", "add-header", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("D", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("U", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("I", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("E");
    opts.addOpt("", "include-dir", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "dll-export", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "dict", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("", "index", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("", "dict-index", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("", "output-dir", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "depend");
    opts.addOpt("", "depend-xml");
    opts.addOpt("", "depend-file", IceUtilInternal::Options::NeedArg, "");
    opts.addOpt("d", "debug");
    opts.addOpt("", "ice");
    opts.addOpt("", "underscore");

    bool validate = find(argv.begin(), argv.end(), "--validate") != argv.end();
    vector<string> args;
    try
    {
        args = opts.parse(argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        getErrorStream() << argv[0] << ": error: " << e.reason << endl;
        if(!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if(opts.isSet("help"))
    {
        usage(argv[0]);
        return EXIT_SUCCESS;
    }

    if(opts.isSet("version"))
    {
        getErrorStream() << ICE_STRING_VERSION << endl;
        return EXIT_SUCCESS;
    }

    string headerExtension = opts.optArg("header-ext");
    string sourceExtension = opts.optArg("source-ext");

    vector<string> cppArgs;
    vector<string> extraHeaders = opts.argVec("add-header");
    vector<string> optargs = opts.argVec("D");

    for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs.push_back("-D" + *i);
    }

    optargs = opts.argVec("U");
    for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs.push_back("-U" + *i);
    }

    vector<string> includePaths = opts.argVec("I");
    for(vector<string>::const_iterator i = includePaths.begin(); i != includePaths.end(); ++i)
    {
        cppArgs.push_back("-I" + Preprocessor::normalizeIncludePath(*i));
    }

    // Convert include paths to full paths.
    for(vector<string>::iterator p = includePaths.begin(); p != includePaths.end(); ++p)
    {
        *p = fullPath(*p);
    }

    bool preprocess= opts.isSet("E");

    string include = opts.optArg("include-dir");

    string dllExport = opts.optArg("dll-export");

    vector<Dict> dicts;
    optargs = opts.argVec("dict");
    for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
    {
        string s = IceUtilInternal::removeWhitespace(*i);

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
            if(s.find("[\"") == 0)
            {
                string::size_type end = s.find("\"]");
                if(end != string::npos && end < pos)
                {
                    dict.key = s.substr(end + 2, pos - end - 2);
                    dict.keyMetaData.push_back(s.substr(2, end - 2));
                }
                else
                {
                    dict.key = s.substr(0, pos);
                }
            }
            else
            {
                dict.key = s.substr(0, pos);
            }
            s.erase(0, pos + 1);
        }
        pos = s.find(',');
        if(pos == string::npos)
        {
            if(s.find("[\"") == 0)
            {
                string::size_type end = s.find("\"]");
                if(end != string::npos)
                {
                    dict.value = s.substr(end + 2);
                    dict.valueMetaData.push_back(s.substr(2, end - 2));
                }
                else
                {
                    dict.value = s;
                }
            }
            else
            {
                dict.value = s;
            }
            dict.sort = false;
        }
        else
        {
            if(s.find("[\"") == 0)
            {
                string::size_type end = s.find("\"]");
                if(end != string::npos && end < pos)
                {
                    dict.value = s.substr(end + 2, pos - end - 2);
                    dict.valueMetaData.push_back(s.substr(2, end - 2));
                }
                else
                {
                    dict.value = s.substr(0, pos);
                }
            }
            else
            {
                dict.value = s.substr(0, pos);
            }
            s.erase(0, pos + 1);

            pos = s.find(',');
            if(pos == string::npos)
            {
                if(s != "sort")
                {
                    getErrorStream() << argv[0] << ": error: " << *i << ": nothing or ',sort' expected after value-type"
                                     << endl;
                    if(!validate)
                    {
                        usage(argv[0]);
                    }
                    return EXIT_FAILURE;
                }
                dict.sort = true;
            }
            else
            {
                string sort = s.substr(0, pos);
                s.erase(0, pos + 1);
                if(sort != "sort")
                {
                    getErrorStream() << argv[0] << ": error: " << *i << ": nothing or ',sort' expected after value-type"
                                     << endl;
                    if(!validate)
                    {
                        usage(argv[0]);
                    }
                    return EXIT_FAILURE;
                }
                dict.sort = true;
                dict.userCompare = s;
            }
        }

        if(dict.name.empty())
        {
            getErrorStream() << argv[0] << ": error: " << *i << ": no name specified" << endl;
            if(!validate)
            {
                usage(argv[0]);
            }
            return EXIT_FAILURE;
        }

        if(dict.key.empty())
        {
            getErrorStream() << argv[0] << ": error: " << *i << ": no key specified" << endl;
            if(!validate)
            {
                usage(argv[0]);
            }
            return EXIT_FAILURE;
        }

        if(dict.value.empty())
        {
            getErrorStream() << argv[0] << ": error: " << *i << ": no value specified" << endl;
            if(!validate)
            {
                usage(argv[0]);
            }
            return EXIT_FAILURE;
        }

        dicts.push_back(dict);
    }

    vector<Index> indices;
    optargs = opts.argVec("index");
    for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
    {
        string s = IceUtilInternal::removeWhitespace(*i);

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
            getErrorStream() << argv[0] << ": error: " << *i << ": no name specified" << endl;
            if(!validate)
            {
                usage(argv[0]);
            }
            return EXIT_FAILURE;
        }

        if(index.type.empty())
        {
            getErrorStream() << argv[0] << ": error: " << *i << ": no type specified" << endl;
            if(!validate)
            {
                usage(argv[0]);
            }
            return EXIT_FAILURE;
        }

        if(index.member.empty())
        {
            getErrorStream() << argv[0] << ": error: " << *i << ": no member specified" << endl;
            if(!validate)
            {
                usage(argv[0]);
            }
            return EXIT_FAILURE;
        }

        if(caseString != "case-sensitive" && caseString != "case-insensitive")
        {
            getErrorStream() << argv[0] << ": error: " << *i << ": the case can be `case-sensitive' or "
                             << "`case-insensitive'" << endl;
            if(!validate)
            {
                usage(argv[0]);
            }
            return EXIT_FAILURE;
        }
        index.caseSensitive = (caseString == "case-sensitive");

        indices.push_back(index);
    }

    optargs = opts.argVec("dict-index");
    for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
    {
        string s = IceUtilInternal::removeWhitespace(*i);

        string dictName;
        DictIndex index;
        index.sort = false;
        index.caseSensitive = true;

        string::size_type pos = s.find(',');
        if(pos == string::npos)
        {
            dictName = s;
        }
        else
        {
            dictName = s.substr(0, pos);
            s.erase(0, pos + 1);

            bool done = false;
            while(!done)
            {
                pos = s.find(',');
                if(pos == string::npos)
                {
                    if(s == "sort")
                    {
                        index.sort = true;
                    }
                    else if(s == "case-sensitive")
                    {
                        index.caseSensitive = true;
                    }
                    else if(s == "case-insensitive")
                    {
                        index.caseSensitive = false;
                    }
                    else if(index.member.empty())
                    {
                        if(s == "\\sort")
                        {
                            index.member = "sort";
                        }
                        else
                        {
                            index.member = s;
                        }
                    }
                    else
                    {
                        getErrorStream() << argv[0] << ": error: " << *i << ": syntax error" << endl;
                        if(!validate)
                        {
                            usage(argv[0]);
                        }
                        return EXIT_FAILURE;
                    }
                    done = true;
                }
                else
                {
                    string subs = s.substr(0, pos);
                    s.erase(0, pos + 1);

                    if(subs == "sort")
                    {
                        index.sort = true;
                        index.userCompare = s;
                        done = true;
                    }
                    else if(subs == "case-sensitive")
                    {
                        index.caseSensitive = true;
                    }
                    else if(subs == "case-insensitive")
                    {
                        index.caseSensitive = false;
                    }
                    else if(index.member.empty())
                    {
                        if(subs == "\\sort")
                        {
                            index.member = "sort";
                        }
                        else
                        {
                            index.member = subs;
                        }
                    }
                    else
                    {
                        getErrorStream() << argv[0] << ": error: " << *i << ": syntax error" << endl;
                        if(!validate)
                        {
                            usage(argv[0]);
                        }
                        return EXIT_FAILURE;
                    }
                }
            }
        }

        if(dictName.empty())
        {
            getErrorStream() << argv[0] << ": error: " << *i << ": no dictionary specified" << endl;
            if(!validate)
            {
                usage(argv[0]);
            }
            return EXIT_FAILURE;
        }

        bool found = false;
        for(vector<Dict>::iterator p = dicts.begin(); p != dicts.end(); ++p)
        {
            if(p->name == dictName)
            {
                if(find(p->indices.begin(), p->indices.end(), index) != p->indices.end())
                {
                    getErrorStream() << argv[0] << ": error: --dict-index " << *i
                         << ": this dict-index is defined twice" << endl;
                    return EXIT_FAILURE;
                }
                p->indices.push_back(index);
                found = true;
                break;
            }
        }
        if(!found)
        {
            getErrorStream() << argv[0] << ": error: " << *i << ": unknown dictionary" << endl;
            if(!validate)
            {
                usage(argv[0]);
            }
            return EXIT_FAILURE;
        }
    }

    string output = opts.optArg("output-dir");

    bool depend = opts.isSet("depend");
    bool dependxml = opts.isSet("depend-xml");

    string dependFile = opts.optArg("depend-file");

    bool debug = opts.isSet("debug");

    bool ice = opts.isSet("ice");

    bool underscore = opts.isSet("underscore");

    if(dicts.empty() && indices.empty() && !(depend || dependxml))
    {
        getErrorStream() << argv[0] << ": error: no Freeze types specified" << endl;
        if(!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if(args.empty())
    {
        getErrorStream() << argv[0] << ": error: no file name base specified" << endl;
        if(!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if(depend && dependxml)
    {
        getErrorStream() << argv[0] << ": error: cannot specify both --depend and --depend-xml" << endl;
        if(!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if(validate)
    {
        return EXIT_SUCCESS;
    }

    UnitPtr u = Unit::createUnit(true, false, ice, underscore);

    StringList includes;

    int status = EXIT_SUCCESS;

    IceUtil::CtrlCHandler ctrlCHandler;
    ctrlCHandler.setCallback(interruptedCallback);

    DependOutputUtil out(dependFile);
    if(dependxml)
    {
        out.os() << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dependencies>" << endl;
    }

    for(vector<string>::size_type idx = 1; idx < args.size(); ++idx)
    {
        if(depend || dependxml)
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], args[idx], cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2FREEZE__");

            if(cppHandle == 0)
            {
                out.cleanup();
                u->destroy();
                return EXIT_FAILURE;
            }

            status = u->parse(args[idx], cppHandle, debug);

            if(status == EXIT_FAILURE)
            {
                out.cleanup();
                u->destroy();
                return EXIT_FAILURE;
            }

            if(!icecpp->printMakefileDependencies(out.os(), depend ? Preprocessor::CPlusPlus : Preprocessor::SliceXML, includePaths,
                                                  "-D__SLICE2FREEZE__",  sourceExtension, headerExtension))
            {
                out.cleanup();
                u->destroy();
                return EXIT_FAILURE;
            }

            if(!icecpp->close())
            {
                out.cleanup();
                u->destroy();
                return EXIT_FAILURE;
            }
        }
        else
        {
            string sliceFile = args[idx];

            PreprocessorPtr icecpp = Preprocessor::create(argv[0], sliceFile, cppArgs);

            //
            // Add an include file for each Slice file. Note that the .h extension
            // is replaced with headerExtension later.
            //

            string headerFile = sliceFile;
            string::size_type pos = headerFile.rfind('.');
            if(pos != string::npos)
            {
                headerFile.erase(pos);
            }
            headerFile += ".h";

            includes.push_back(headerFile);

            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2FREEZE__");

            if(cppHandle == 0)
            {
                u->destroy();
                return EXIT_FAILURE;
            }

            if(preprocess)
            {
                char buf[4096];
                while(fgets(buf, static_cast<int>(sizeof(buf)), cppHandle) != NULL)
                {
                    if(fputs(buf, stdout) == EOF)
                    {
                        u->destroy();
                        return EXIT_FAILURE;
                    }
                }
            }
            else
            {
                status = u->parse(args[idx], cppHandle, debug);

                MetaDataVisitor visitor;
                u->visit(&visitor, false);
            }

            if(!icecpp->close())
            {
                u->destroy();
                return EXIT_FAILURE;
            }
        }

        {
            IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(globalMutex);

            if(interrupted)
            {
                return EXIT_FAILURE;
            }
        }
    }

    if(dependxml)
    {
        out.os() << "</dependencies>\n";
    }

    if(depend || dependxml)
    {
        u->destroy();
        return EXIT_SUCCESS;
    }

    if(status == EXIT_SUCCESS && !preprocess)
    {
        try
        {
            gen(argv[0], u, includePaths, extraHeaders, dicts, indices, include, headerExtension,
                sourceExtension, dllExport, includes, args, output);
        }
        catch(const string& ex)
        {
            // If a file could not be created, then cleanup any
            // created files.
            FileTracker::instance()->cleanup();
            u->destroy();
            getErrorStream() << argv[0] << ": error: " << ex << endl;
            return EXIT_FAILURE;
        }
        catch(const Slice::FileException& ex)
        {
            // If a file could not be created, then cleanup any
            // created files.
            FileTracker::instance()->cleanup();
            u->destroy();
            getErrorStream() << argv[0] << ": error: " << ex.reason() << endl;
            return EXIT_FAILURE;
        }
        catch(...)
        {
            getErrorStream() << argv[0] << ": error: unknown exception" << endl;
            FileTracker::instance()->cleanup();
            u->destroy();
            return EXIT_FAILURE;
        }
    }

    u->destroy();

    {
        IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(globalMutex);

        if(interrupted)
        {
            FileTracker::instance()->cleanup();
            return EXIT_FAILURE;
        }
    }

    return status;
}

#ifdef _WIN32
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    vector<string> args = argvToArgs(argc, argv);
    try
    {
        return compile(args);
    }
    catch(const std::exception& ex)
    {
        getErrorStream() << args[0] << ": error:" << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch(const std::string& msg)
    {
        getErrorStream() << args[0] << ": error:" << msg << endl;
        return EXIT_FAILURE;
    }
    catch(const char* msg)
    {
        getErrorStream() << args[0] << ": error:" << msg << endl;
        return EXIT_FAILURE;
    }
    catch(...)
    {
        getErrorStream() << args[0] << ": error:" << "unknown exception" << endl;
        return EXIT_FAILURE;
    }
}
