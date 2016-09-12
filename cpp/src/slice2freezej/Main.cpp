// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/CtrlCHandler.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <Slice/Preprocessor.h>
#include <Slice/FileTracker.h>
#include <Slice/JavaUtil.h>
#include <Slice/Util.h>
#include <iterator>

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

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

}

void
interruptedCallback(int /*signal*/)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(globalMutex);

    interrupted = true;
}

struct DictIndex
{
    string member;
    bool caseSensitive;

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

class FreezeGenerator : public JavaGenerator
{
public:
    FreezeGenerator(const string&, const string&);
    virtual ~FreezeGenerator();

    void generate(UnitPtr&, const Dict&);

    void generate(UnitPtr&, const Index&);

#ifdef __SUNPRO_CC
protected:
    using JavaGenerator::typeToObjectString;
#endif

private:

    string typeToObjectString(const TypePtr&);
    string varToObject(const TypePtr&, const string&);
    string objectToVar(const TypePtr&, const string&);

    const string _prog;
};

FreezeGenerator::FreezeGenerator(const string& prog, const string& dir)
    : JavaGenerator(dir),
      _prog(prog)
{
}

FreezeGenerator::~FreezeGenerator()
{
}

string
FreezeGenerator::typeToObjectString(const TypePtr& type)
{
    static const char* builtinTable[] =
    {
        "java.lang.Byte",
        "java.lang.Boolean",
        "java.lang.Short",
        "java.lang.Integer",
        "java.lang.Long",
        "java.lang.Float",
        "java.lang.Double",
        "java.lang.String",
        "Ice.Object",
        "Ice.ObjectPrx",
        "Ice.LocalObject"
    };

    BuiltinPtr b = BuiltinPtr::dynamicCast(type);
    if(b)
    {
        return builtinTable[b->kind()];
    }
    else
    {
        return typeToString(type, TypeModeIn);
    }
}

string
FreezeGenerator::varToObject(const TypePtr& type, const string& param)
{
    string result = param;

    BuiltinPtr b = BuiltinPtr::dynamicCast(type);
    if(b != 0)
    {
        switch(b->kind())
        {
            case Builtin::KindByte:
            {
                result = string("java.lang.Byte.valueOf(") + param + ")";
                break;
            }
            case Builtin::KindBool:
            {
                result = string("java.lang.Boolean.valueOf(") + param + ")";
                break;
            }
            case Builtin::KindShort:
            {
                result = string("java.lang.Short.valueOf(") + param + ")";
                break;
            }
            case Builtin::KindInt:
            {
                result = string("java.lang.Integer.valueOf(") + param + ")";
                break;
            }
            case Builtin::KindLong:
            {
                result = string("java.lang.Long.valueOf(") + param + ")";
                break;
            }
            case Builtin::KindFloat:
            {
                result = string("java.lang.Float.valueOf(") + param + ")";
                break;
            }
            case Builtin::KindDouble:
            {
                result = string("java.lang.Double.valueOf(") + param + ")";
                break;
            }
            case Builtin::KindString:
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindLocalObject:
                break;
        }
    }
    return result;
}

string
FreezeGenerator::objectToVar(const TypePtr& type, const string& param)
{
    string result = param;

    BuiltinPtr b = BuiltinPtr::dynamicCast(type);
    if(b != 0)
    {
        switch(b->kind())
        {
            case Builtin::KindByte:
            {
                result = param + ".byteValue()";
                break;
            }
            case Builtin::KindBool:
            {
                result = param + ".booleanValue()";
                break;
            }
            case Builtin::KindShort:
            {
                result = param + ".shortValue()";
                break;
            }
            case Builtin::KindInt:
            {
                result = param + ".intValue()";
                break;
            }
            case Builtin::KindLong:
            {
                result = param + ".longValue()";
                break;
            }
            case Builtin::KindFloat:
            {
                result = param + ".floatValue()";
                break;
            }
            case Builtin::KindDouble:
            {
                result = param + ".doubleValue()";
                break;
            }
            case Builtin::KindString:
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindLocalObject:
                break;
        }
    }
    return result;
}

void
FreezeGenerator::generate(UnitPtr& u, const Dict& dict)
{
    //
    // The dictionary name may include a package.
    //
    string name;
    string::size_type pos = dict.name.rfind('.');
    if(pos == string::npos)
    {
        name = dict.name;
    }
    else
    {
        name = dict.name.substr(pos + 1);
    }

    TypeList keyTypes = u->lookupType(dict.key, false);
    if(keyTypes.empty())
    {
        ostringstream os;
        os << "`" << dict.key << "' is not a valid type" << endl;
        throw os.str();
    }
    TypePtr keyType = keyTypes.front();

    TypeList valueTypes = u->lookupType(dict.value, false);
    if(valueTypes.empty())
    {
        ostringstream os;
        os << "`" << dict.value << "' is not a valid type" << endl;
        throw os.str();
    }
    TypePtr valueType = valueTypes.front();

    vector<TypePtr> indexTypes;
    vector<string> members;
    vector<string> capitalizedMembers;
    vector<string> indexNames;

    for(size_t i = 0; i < dict.indices.size(); ++i)
    {
        const DictIndex& index = dict.indices[i];
        const string& member = index.member;

        if(index.member.empty())
        {
            //
            // No member was specified, which means we use the map's value type as the index key.
            //

            if(dict.indices.size() > 1)
            {
                ostringstream os;
                os << "bad index for dictionary `" << dict.name << "'" << endl;
                throw os.str();
            }

            bool containsSequence = false;
            if(!Dictionary::legalKeyType(valueType, containsSequence))
            {
                ostringstream os;
                os << "`" << dict.value << "' is not a valid index type" << endl;
                throw os.str();
            }
            if(containsSequence)
            {
                getErrorStream() << _prog << ": warning: use of sequences in dictionary keys has been deprecated"
                                 << endl;
            }

            if(index.caseSensitive == false)
            {
                //
                // Verify that value type is a string.
                //
                BuiltinPtr b = BuiltinPtr::dynamicCast(valueType);
                if(b == 0 || b->kind() != Builtin::KindString)
                {
                    ostringstream os;
                    os << "VALUE is a `" << dict.value << "', not a string" << endl;
                    throw os.str();
                }
            }
            indexTypes.push_back(valueType);
            members.push_back("value");
            capitalizedMembers.push_back("Value");
            indexNames.push_back("index");
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
                    os << "`" << dict.value << "' is neither a class nor a struct" << endl;
                    throw os.str();
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
                ostringstream os;
                os << "The value of `" << dict.name << "' has no data member named `" << index.member << "'" << endl;
                throw os.str();
            }

            TypePtr dataMemberType = dataMember->type();

            bool containsSequence = false;
            if(!Dictionary::legalKeyType(dataMemberType, containsSequence))
            {
                ostringstream os;
                os << "`" << index.member << "' cannot be used as an index key" << endl;
                throw os.str();
            }
            if(containsSequence)
            {
                getErrorStream() << _prog << ": warning: use of sequences in dictionary keys has been deprecated"
                                 << endl;
            }

            if(index.caseSensitive == false)
            {
                //
                // Verify that member type is a string.
                //
                BuiltinPtr b = BuiltinPtr::dynamicCast(dataMemberType);
                if(b == 0 || b->kind() != Builtin::KindString)
                {
                    ostringstream os;
                    os << "`" << index.member << "' is not a string" << endl;
                    throw os.str();
                }
            }
            indexTypes.push_back(dataMemberType);

            members.push_back(member);
            string capitalizedMember = member;
            capitalizedMember[0] = toupper(static_cast<unsigned char>(capitalizedMember[0]));
            capitalizedMembers.push_back(capitalizedMember);
            indexNames.push_back(member);
        }
    }

    open(dict.name, u->currentFile());

    Output& out = output();

    string keyTypeS = typeToObjectString(keyType);
    string valueTypeS = typeToObjectString(valueType);

    out << sp << nl << "public class " << name << " extends Freeze.MapInternal.MapI<" << keyTypeS << ", "
        << valueTypeS << ">";
    out << sb;

    if(dict.indices.size() > 0)
    {
        out << sp;
        out << nl << "/**"
            << nl << " * Supplies a comparator for each index key."
            << nl << " */";
        out << nl << "public static class IndexComparators";
        out << sb;
        out << sp;
        out << nl << "/**"
            << nl << " * Default constructor assigns null to the comparator for each index key."
            << nl << " */";
        out << nl << "public" << nl << "IndexComparators()";
        out << sb;
        out << eb;

        out << sp;
        out << nl << "/**"
            << nl << " * This constructor accepts a comparator for each index key.";
        for(size_t i = 0; i < dict.indices.size(); ++i)
        {
            out << nl << " * @param " << members[i] << "Comparator Comparator for <code>" << members[i] << "</code>.";
        }
        out << nl << " */";
        out << nl << "public" << nl << "IndexComparators(";
        for(size_t i = 0; i < dict.indices.size(); ++i)
        {
            if(i > 0)
            {
                out << ", ";
            }
            out << "java.util.Comparator<" << typeToObjectString(indexTypes[i]) << "> " << members[i]
                << "Comparator";
        }
        out << ")";
        out << sb;
        for(size_t i = 0; i < dict.indices.size(); ++i)
        {
            out << nl << "this." << members[i] << "Comparator = " << members[i] << "Comparator;";
        }
        out << eb;

        out << sp;
        for(size_t i = 0; i < dict.indices.size(); ++i)
        {
            out << nl << "/** Comparator for <code>" << members[i] << "</code>. */";
            out << nl << "public java.util.Comparator<" << typeToObjectString(indexTypes[i]) << "> " << members[i]
                << "Comparator;";
        }
        out << eb;
    }

    //
    // Constructors
    //

    out << sp << nl << "private" << nl << name
        << "(Freeze.Connection __connection, String __dbName, java.util.Comparator<" << keyTypeS << "> __comparator";
    if(dict.indices.size() > 0)
    {
        out << ", IndexComparators __indexComparators";
    }
    out << ")";
    out << sb;

    out << nl << "super(__connection, __dbName, __comparator);";
    if(dict.indices.size() > 0)
    {
        out << nl << "_indices = new Freeze.MapIndex[" << dict.indices.size() << "];";
        for(size_t i = 0; i < dict.indices.size(); ++i)
        {
            out << nl << "_" << members[i] << "Index = new " << capitalizedMembers[i] << "Index(\"" << indexNames[i]
                << "\", __indexComparators == null ? null : __indexComparators." << members[i] << "Comparator);";
            out << nl << "_indices[" << i << "] = _" << members[i] << "Index;";
        }
    }
    out << eb;

    if(dict.indices.size() > 0)
    {
        out << sp;
        out << nl << "/**"
            << nl << " * Instantiates a Freeze map using the given connection. If the database"
            << nl << " * named in <code>__dbName</code> does not exist and <code>__createDb</code>"
            << nl << " * is true, the database is created automatically, otherwise this constructor"
            << nl << " * raises <code>DatabaseException</code>."
            << nl << " * @param __connection The Freeze connection associated with this map."
            << nl << " * @param __dbName The name of the Berkeley DB database."
            << nl << " * @param __createDb True if the database should be created if it does not"
            << nl << " *   already exist, false otherwise."
            << nl << " * @param __comparator A comparator for the map's main key, or null to use the"
            << nl << " *   default key comparison strategy."
            << nl << " * @param __indexComparators A map of string to comparator, representing the"
            << nl << " *   key comparator for each of the map's indices. The map uses the default"
            << nl << " *   key comparison strategy for an index if <code>__indexComparators</code>"
            << nl << " *   is null, or if no entry can be found in the comparators map for an index."
            << nl << " * @throws Freeze.DatabaseException If an error occurs during database operations."
            << nl << " */"
            << nl << "public" << nl << name
            << "(Freeze.Connection __connection, String __dbName, boolean __createDb, "
            << "java.util.Comparator<" << keyTypeS  << "> __comparator, "
            << "IndexComparators __indexComparators)";
        out << sb;
        out << nl << "this(__connection, __dbName, __comparator, __indexComparators);";
        out << nl << "init(_indices, __dbName, \"" << keyType->typeId() << "\", \"" << valueType->typeId()
            << "\", __createDb);";
        out << eb;
    }

    out << sp;
    out << nl << "/**"
        << nl << " * Instantiates a Freeze map using the given connection. If the database"
        << nl << " * named in <code>__dbName</code> does not exist and <code>__createDb</code>"
        << nl << " * is true, the database is created automatically, otherwise this constructor"
        << nl << " * raises <code>DatabaseException</code>."
        << nl << " * @param __connection The Freeze connection associated with this map."
        << nl << " * @param __dbName The name of the Berkeley DB database."
        << nl << " * @param __createDb True if the database should be created if it does not"
        << nl << " *   already exist, false otherwise."
        << nl << " * @param __comparator A comparator for the map's main key, or null to use the"
        << nl << " *   default key comparison strategy."
        << nl << " * @throws Freeze.DatabaseException If an error occurs during database operations."
        << nl << " */";
    out << nl << "public" << nl << name
        << "(Freeze.Connection __connection, String __dbName, boolean __createDb, "
        << "java.util.Comparator<" << keyTypeS << "> __comparator)";
    out << sb;
    if(dict.indices.size() > 0)
    {
        out << nl << "this(__connection, __dbName, __createDb, __comparator, null);";
    }
    else
    {
        out << nl << "super(__connection,  __dbName, \"" << keyType->typeId() << "\", \""
            << valueType->typeId() << "\", __createDb, __comparator);";
    }
    out << eb;

    out << sp;
    out << nl << "/**"
        << nl << " * Instantiates a Freeze map using the given connection. If the database"
        << nl << " * named in <code>__dbName</code> does not exist and <code>__createDb</code>"
        << nl << " * is true, the database is created automatically, otherwise this constructor"
        << nl << " * raises <code>DatabaseException</code>. The map uses the default key"
        << nl << " * comparison strategy."
        << nl << " * @param __connection The Freeze connection associated with this map."
        << nl << " * @param __dbName The name of the Berkeley DB database."
        << nl << " * @param __createDb True if the database should be created if it does not"
        << nl << " *   already exist, false otherwise."
        << nl << " * @throws Freeze.DatabaseException If an error occurs during database operations."
        << nl << " */";
    out << nl << "public" << nl << name
        << "(Freeze.Connection __connection, String __dbName, boolean __createDb)";
    out << sb;
    out << nl << "this(__connection, __dbName, __createDb, null);";
    out << eb;

    out << sp;
    out << nl << "/**"
        << nl << " * Instantiates a Freeze map using the given connection. If the database"
        << nl << " * named in <code>__dbName</code> does not exist, it is created automatically."
        << nl << " * The map uses the default key comparison strategy."
        << nl << " * @param __connection The Freeze connection associated with this map."
        << nl << " * @param __dbName The name of the Berkeley DB database."
        << nl << " * @throws Freeze.DatabaseException If an error occurs during database operations."
        << nl << " */";
    out << nl << "public" << nl << name << "(Freeze.Connection __connection, String __dbName)";
    out << sb;
    out << nl << "this(__connection, __dbName, true);";
    out << eb;

    //
    // recreate
    //
    if(dict.indices.size() > 0)
    {
        out << sp;
        out << nl << "/**"
            << nl << " * Copies an existing database. The new database has the name given in"
            << nl << " * <code>__dbName</code>, and the old database is renamed with a UUID"
            << nl << " * suffix."
            << nl << " * @param __connection The Freeze connection associated with this map."
            << nl << " * @param __dbName The name of the Berkeley DB database."
            << nl << " * @param __comparator A comparator for the map's main key, or null to use the"
            << nl << " *   default key comparison strategy."
            << nl << " * @param __indexComparators A map of string to comparator, representing the"
            << nl << " *   key comparator for each of the map's indices. The map uses the default"
            << nl << " *   key comparison strategy for an index if <code>__indexComparators</code>"
            << nl << " *   is null, or if no entry can be found in the comparators map for an index."
            << nl << " * @throws Freeze.DatabaseException If an error occurs during database operations."
            << nl << " */";
        out << nl << "public static void" << nl
            << "recreate(Freeze.Connection __connection, String __dbName, "
            << "java.util.Comparator<" << keyTypeS  << "> __comparator, "
            << "IndexComparators __indexComparators)";
        out << sb;
        out << nl << name << " __tmpMap = new " << name
            << "(__connection, __dbName, __comparator, __indexComparators);";
        out << nl << "recreate(__tmpMap, __dbName, \"" << keyType->typeId() << "\", \""
            << valueType->typeId() << "\", __tmpMap._indices);";
        out << eb;
    }

    out << sp;
    out << nl << "/**"
        << nl << " * Copies an existing database. The new database has the name given in"
        << nl << " * <code>__dbName</code>, and the old database is renamed with a UUID"
        << nl << " * suffix."
        << nl << " * @param __connection The Freeze connection associated with this map."
        << nl << " * @param __dbName The name of the Berkeley DB database."
        << nl << " * @param __comparator A comparator for the map's main key, or null to use the"
        << nl << " *   default key comparison strategy."
        << nl << " * @throws Freeze.DatabaseException If an error occurs during database operations."
        << nl << " */";
    out << nl << "public static void" << nl
        << "recreate(Freeze.Connection __connection, String __dbName, "
        << "java.util.Comparator<" << keyTypeS  << "> __comparator)";
    out << sb;
    if(dict.indices.size() > 0)
    {
        out << nl << "recreate(__connection, __dbName, __comparator, null);";
    }
    else
    {
        out << nl << name << " __tmpMap = new " << name << "(__connection, __dbName, __comparator);";
        out << nl << "recreate(__tmpMap, __dbName, \"" << keyType->typeId() << "\", \""
            << valueType->typeId() << "\", null);";
    }
    out << eb;

    //
    // Index methods
    //
    for(size_t i = 0; i < capitalizedMembers.size(); ++i)
    {
        string indexClassName = capitalizedMembers[i] + "Index";
        string indexTypeS = typeToString(indexTypes[i], TypeModeIn);
        string indexObjTypeS = typeToObjectString(indexTypes[i]);
        string indexObj = varToObject(indexTypes[i], "__key");

        out << sp;
        out << nl << "/**"
            << nl << " * Obtains an iterator ordered using the index value."
            << nl << " * The iterator's initial position is an element whose key matches <code>__key</code>; if"
            << nl << " * no such element exists, the returned iterator is empty (<code>hasNext</code> returns"
            << nl << " * false). If <code>__onlyDups</code> is true, the iterator only returns elements whose"
            << nl << " * key exactly matches <code>__key</code>; otherwise, the iterator continues to iterate over"
            << nl << " * the remaining elements in the map."
            << nl << " * @param __key The value at which the iterator begins."
            << nl << " * @param __onlyDups True if the iterator should be limited to elements whose key"
            << nl << " *   exactly matches <code>__key</code>, false otherwise."
            << nl << " * @return A new iterator."
            << nl << " * @throws Freeze.DatabaseException If an error occurs during database operations."
            << nl << " */";
        out << nl << "public Freeze.Map.EntryIterator<java.util.Map.Entry<" << keyTypeS << ", " << valueTypeS
            << ">>";
        out << nl << "findBy" << capitalizedMembers[i] << "(" << indexTypeS << " __key, boolean __onlyDups)";
        out << sb;
        out << nl << "return _" << members[i] << "Index.find(" << indexObj << ", __onlyDups);";
        out << eb;

        out << sp;
        out << nl << "/**"
            << nl << " * Obtains an iterator ordered using the values of member <code>" << members[i] << "</code>."
            << nl << " * The iterator's initial position is an element whose key matches <code>__key</code>; if"
            << nl << " * no such element exists, the returned iterator is empty (<code>hasNext</code> returns"
            << nl << " * false). This iterator only returns elements whose key exactly matches <code>__key</code>."
            << nl << " * @param __key The value at which the iterator begins."
            << nl << " * @return A new iterator."
            << nl << " * @throws Freeze.DatabaseException If an error occurs during database operations."
            << nl << " */";
        out << nl << "public Freeze.Map.EntryIterator<java.util.Map.Entry<" << keyTypeS << ", " << valueTypeS
            << ">>";
        out << nl << "findBy" << capitalizedMembers[i] << "(" << indexTypeS << " __key)";
        out << sb;
        out << nl << "return _" << members[i] << "Index.find(" << indexObj << ", true);";
        out << eb;

        out << sp;
        out << nl << "/**"
            << nl << " * Determines the number of elements whose index values match <code>__key</code>."
            << nl << " * @return The number of matching elements."
            << nl << " * @throws Freeze.DatabaseException If an error occurs during database operations."
            << nl << " */";
        string countMethod = dict.indices[i].member.empty() ?  string("valueCount") : dict.indices[i].member + "Count";
        out << nl << "public int";
        out << nl << countMethod << "(" << indexTypeS << " __key)";
        out << sb;
        out << nl << "return _" << members[i] << "Index.count(" << indexObj << ");";
        out << eb;

        string subMap = "Freeze.NavigableMap<" + indexObjTypeS + ", java.util.Set<java.util.Map.Entry<" + keyTypeS +
            ", " + valueTypeS + ">>>";

        out << sp;
        out << nl << "/**"
            << nl << " * Returns a view of the portion of this map whose keys are strictly less than"
            << nl << " * <code>__toKey</code>, or less than or equal to <code>__toKey</code> if"
            << nl << " * <code>__inclusive</code> is true. Insertions and removals via this map are"
            << nl << " * not supported."
            << nl << " * @param __toKey High endpoint of the keys in the returned map."
            << nl << " * @param __inclusive If true, the endpoint is included in the returned map;"
            << nl << " *   otherwise, the endpoint is excluded."
            << nl << " * @return A view of the portion of this map whose keys are strictly less than"
            << nl << " *   <code>__toKey</code>, or less than or equal to <code>__toKey</code> if"
            << nl << " *   <code>__inclusive</code> is true."
            << nl << " * @throws Freeze.DatabaseException If an error occurs during database operations."
            << nl << " */";
        out << nl << "public " + subMap;
        out << nl << "headMapFor" << capitalizedMembers[i] << "(" << indexTypeS << " __toKey, boolean __inclusive)";
        out << sb;
        out << nl << "return _" << members[i] << "Index.createHeadMap(" << varToObject(indexTypes[i], "__toKey")
            << ", __inclusive);";
        out << eb;

        out << sp;
        out << nl << "/**"
            << nl << " * Returns a view of the portion of this map whose keys are strictly less than"
            << nl << " * <code>__toKey</code>. Insertions and removals via this map are not supported."
            << nl << " * @param __toKey High endpoint of the keys in the returned map."
            << nl << " * @return A view of the portion of this map whose keys are strictly less than"
            << nl << " *   <code>__toKey</code>>"
            << nl << " * @throws Freeze.DatabaseException If an error occurs during database operations."
            << nl << " */";
        out << nl << "public " + subMap;
        out << nl << "headMapFor" << capitalizedMembers[i] << "(" << indexTypeS << " __toKey)";
        out << sb;
        out << nl << "return headMapFor" << capitalizedMembers[i] << "(__toKey, false);";
        out << eb;

        out << sp;
        out << nl << "/**"
            << nl << " * Returns a view of the portion of this map whose keys are strictly greater than"
            << nl << " * <code>__fromKey</code>, or greater than or equal to <code>__fromKey</code> if"
            << nl << " * <code>__inclusive</code> is true. Insertions and removals via this map are"
            << nl << " * not supported."
            << nl << " * @param __fromKey Low endpoint of the keys in the returned map."
            << nl << " * @param __inclusive If true, the endpoint is included in the returned map;"
            << nl << " *   otherwise, the endpoint is excluded."
            << nl << " * @return A view of the portion of this map whose keys are strictly greater than"
            << nl << " *   <code>__fromKey</code>, or greater than or equal to <code>__fromKey</code> if"
            << nl << " *   <code>__inclusive</code> is true."
            << nl << " * @throws Freeze.DatabaseException If an error occurs during database operations."
            << nl << " */";
        out << nl << "public " + subMap;
        out << nl << "tailMapFor" << capitalizedMembers[i] << "(" << indexTypeS << " __fromKey, boolean __inclusive)";
        out << sb;
        out << nl << "return _" << members[i] << "Index.createTailMap(" << varToObject(indexTypes[i], "__fromKey")
            << ", __inclusive);";
        out << eb;

        out << sp;
        out << nl << "/**"
            << nl << " * Returns a view of the portion of this map whose keys are greater than or equal"
            << nl << " * to <code>__fromKey</code>. Insertions and removals via this map are not supported."
            << nl << " * @param __fromKey Low endpoint of the keys in the returned map."
            << nl << " * @return A view of the portion of this map whose keys are greater than or equal"
            << nl << " *   to <code>__fromKey</code>."
            << nl << " * @throws Freeze.DatabaseException If an error occurs during database operations."
            << nl << " */";
        out << nl << "public " + subMap;
        out << nl << "tailMapFor" << capitalizedMembers[i] << "(" << indexTypeS << " __fromKey)";
        out << sb;
        out << nl << "return tailMapFor" << capitalizedMembers[i] << "(__fromKey, true);";
        out << eb;

        out << sp;
        out << nl << "/**"
            << nl << " * Returns a view of the portion of this map whose keys range from"
            << nl << " * <code>__fromKey</code> to <code>__toKey</code>. If <code>__fromKey</code>"
            << nl << " * and <code>__toKey</code> are equal, the returned map is empty unless"
            << nl << " * <code>__fromInclusive</code> and <code>__toInclusive</code> are both true."
            << nl << " * Insertions and removals via this map are not supported."
            << nl << " * @param __fromKey Low endpoint of the keys in the returned map."
            << nl << " * @param __fromInclusive If true, the low endpoint is included in the returned map;"
            << nl << " *   otherwise, the endpoint is excluded."
            << nl << " * @param __toKey High endpoint of the keys in the returned map."
            << nl << " * @param __toInclusive If true, the high endpoint is included in the returned map;"
            << nl << " *   otherwise, the endpoint is excluded."
            << nl << " * @return A view of the portion of this map whose keys range from"
            << nl << " *   <code>__fromKey</code> to <code>__toKey</code>."
            << nl << " * @throws Freeze.DatabaseException If an error occurs during database operations."
            << nl << " */";
        out << nl << "public " + subMap;
        out << nl << "subMapFor" << capitalizedMembers[i] << "(" << indexTypeS
            << " __fromKey, boolean __fromInclusive, " << indexTypeS << " __toKey, boolean __toInclusive)";
        out << sb;
        out << nl << "return _" << members[i] << "Index.createSubMap(" << varToObject(indexTypes[i], "__fromKey")
            << ", __fromInclusive, " << varToObject(indexTypes[i], "__toKey") << ", __toInclusive);";
        out << eb;

        out << sp;
        out << nl << "/**"
            << nl << " * Returns a view of the portion of this map whose keys are greater than"
            << nl << " * or equal to <code>__fromKey</code> and strictly less than <code>__toKey</code>."
            << nl << " * Insertions and removals via this map are not supported."
            << nl << " * @param __fromKey Low endpoint of the keys in the returned map."
            << nl << " * @param __toKey High endpoint of the keys in the returned map."
            << nl << " * @return A view of the portion of this map whose keys range from"
            << nl << " *   <code>__fromKey</code> to <code>__toKey</code>."
            << nl << " * @throws Freeze.DatabaseException If an error occurs during database operations."
            << nl << " */";
        out << nl << "public " + subMap;
        out << nl << "subMapFor" << capitalizedMembers[i] << "(" << indexTypeS << " __fromKey, " << indexTypeS
            << " __toKey)";
        out << sb;
        out << nl << "return subMapFor" << capitalizedMembers[i] << "(__fromKey, true, __toKey, false);";
        out << eb;

        out << sp;
        out << nl << "/**"
            << nl << " * Returns a view of this map whose keys are ordered by the index value."
            << nl << " * Insertions and removals via this map are not supported."
            << nl << " * @return A view of this map whose keys range are ordered by the index value."
            << nl << " * @throws Freeze.DatabaseException If an error occurs during database operations."
            << nl << " */";
        out << nl << "public " + subMap;
        out << nl << "mapFor" << capitalizedMembers[i] << "()";
        out << sb;
        out << nl << "return _" << members[i] << "Index.createMap();";
        out << eb;
    }

    //
    // Top-level encode/decode
    //
    for(size_t i = 0; i < 2; i++)
    {
        string keyValue;
        TypePtr type;
        bool encaps;
        string typeS;

        if(i == 0)
        {
            keyValue = "Key";
            type = keyType;
            typeS = keyTypeS;
            encaps = false; // Do not encapsulate keys.
        }
        else
        {
            keyValue = "Value";
            type = valueType;
            typeS = valueTypeS;
            encaps = true;
        }

        string valS = objectToVar(type, "v");

        int iter;

        //
        // encode
        //
        out << sp << nl << "public void" << nl << "encode" << keyValue << "(" << typeS
            << " v, IceInternal.BasicStream __os)";
        out << sb;
        if(encaps)
        {
            out << nl << "__os.startWriteEncaps();";
        }
        iter = 0;
        writeMarshalUnmarshalCode(out, "", type, OptionalNone, false, 0, valS, true, iter, false);
        if(type->usesClasses())
        {
            out << nl << "__os.writePendingObjects();";
        }
        if(encaps)
        {
            out << nl << "__os.endWriteEncaps();";
        }
        out << eb;

        //
        // decode
        //
        out << sp << nl << "public " << typeS << nl << "decode" << keyValue << "(IceInternal.BasicStream __is)";
        out << sb;
        if(type->usesClasses())
        {
            out << nl << "__is.sliceObjects(false);";
        }
        if(encaps)
        {
            out << nl << "__is.startReadEncaps();";
        }
        iter = 0;
        list<string> metaData;
        string patchParams;
        BuiltinPtr b = BuiltinPtr::dynamicCast(type);
        if((b && b->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(type))
        {
            out << nl << "Patcher __p = new Patcher();";
            patchParams = "__p";
        }
        else if(StructPtr::dynamicCast(type))
        {
            out << nl << typeS << " __r = null;";
        }
        else
        {
            out << nl << typeS << " __r;";
        }
        if(b)
        {
            switch(b->kind())
            {
            case Builtin::KindByte:
            {
                out << nl << "__r = java.lang.Byte.valueOf(__is.readByte());";
                break;
            }
            case Builtin::KindBool:
            {
                out << nl << "__r = java.lang.Boolean.valueOf(__is.readBool());";
                break;
            }
            case Builtin::KindShort:
            {
                out << nl << "__r = java.lang.Short.valueOf(__is.readShort());";
                break;
            }
            case Builtin::KindInt:
            {
                out << nl << "__r = java.lang.Integer.valueOf(__is.readInt());";
                break;
            }
            case Builtin::KindLong:
            {
                out << nl << "__r = java.lang.Long.valueOf(__is.readLong());";
                break;
            }
            case Builtin::KindFloat:
            {
                out << nl << "__r = java.lang.Float.valueOf(__is.readFloat());";
                break;
            }
            case Builtin::KindDouble:
            {
                out << nl << "__r = java.lang.Double.valueOf(__is.readDouble());";
                break;
            }
            case Builtin::KindString:
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindLocalObject:
            {
                writeMarshalUnmarshalCode(out, "", type, OptionalNone, false, 0, "__r", false, iter, false, metaData,
                                          patchParams);
                break;
            }
            }
        }
        else
        {
            writeMarshalUnmarshalCode(out, "", type, OptionalNone, false, 0, "__r", false, iter, false, metaData,
                                      patchParams);
        }
        if(type->usesClasses())
        {
            out << nl << "__is.readPendingObjects();";
        }
        if(encaps)
        {
            out << nl << "__is.endReadEncaps();";
        }
        if((b && b->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(type))
        {
            out << nl << "return __p.value;";
        }
        else
        {
            out << nl << "return __r;";
        }
        out << eb;
    }

    //
    // Inner index classes
    //
    for(size_t i = 0; i < capitalizedMembers.size(); ++i)
    {
        string indexClassName = capitalizedMembers[i] + "Index";
        string indexKeyTypeS = typeToObjectString(indexTypes[i]);

        out << sp << nl << "private class " << indexClassName << " extends Freeze.MapInternal.Index<" << keyTypeS
            << ", " << valueTypeS << ", " << indexKeyTypeS << ">";
        out << sb;

        //
        // encodeKey
        //
        out << sp << nl << "public void";
        out << nl << "encodeKey(" << indexKeyTypeS << " key, IceInternal.BasicStream __os)";
        out << sb;
        if(dict.indices[i].member.empty())
        {
            //
            // Encode the full value (with an encaps!)
            //
            string keyS = "key";
            if(!dict.indices[i].caseSensitive)
            {
                keyS = "key.toLowerCase()";
            }

            out << nl << "encodeValue(" << keyS << ", __os);";
        }
        else
        {
            //
            // No encaps
            //
            string keyS = dict.indices[i].caseSensitive ?  "key" : "key.toLowerCase()";

            keyS = objectToVar(indexTypes[i], keyS);

            int iter = 0;
            writeMarshalUnmarshalCode(out, "", indexTypes[i], OptionalNone, false, 0, keyS, true, iter, false);
            assert(!indexTypes[i]->usesClasses());
        }
        out << eb;

        //
        // decodeKey
        //
        out << sp << nl << "public " << indexKeyTypeS;
        out << nl << "decodeKey(IceInternal.BasicStream __is)";
        out << sb;
        if(dict.indices[i].member.empty())
        {
            //
            // Decode the full value (with an encaps!)
            //
            out << nl << "return decodeValue(__is);";
        }
        else
        {
            int iter = 0;
            list<string> metaData;
            string patchParams;

            if(StructPtr::dynamicCast(indexTypes[i]))
            {
                out << nl << indexKeyTypeS << " r = null;";
            }
            else
            {
                out << nl << indexKeyTypeS << " r;";
            }

            BuiltinPtr b = BuiltinPtr::dynamicCast(indexTypes[i]);
            if(b != 0)
            {
                switch(b->kind())
                {
                case Builtin::KindByte:
                {
                    out << nl << "r = java.lang.Byte.valueOf(__is.readByte());";
                    break;
                }
                case Builtin::KindBool:
                {
                    out << nl << "r = java.lang.Boolean.valueOf(__is.readBool());";
                    break;
                }
                case Builtin::KindShort:
                {
                    out << nl << "r = java.lang.Short.valueOf(__is.readShort());";
                    break;
                }
                case Builtin::KindInt:
                {
                    out << nl << "r = java.lang.Integer.valueOf(__is.readInt());";
                    break;
                }
                case Builtin::KindLong:
                {
                    out << nl << "r = java.lang.Long.valueOf(__is.readLong());";
                    break;
                }
                case Builtin::KindFloat:
                {
                    out << nl << "r = java.lang.Float.valueOf(__is.readFloat());";
                    break;
                }
                case Builtin::KindDouble:
                {
                    out << nl << "r = java.lang.Double.valueOf(__is.readDouble());";
                    break;
                }
                case Builtin::KindString:
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                {
                    writeMarshalUnmarshalCode(out, "", indexTypes[i], OptionalNone, false, 0, "r", false, iter, false,
                                              metaData, patchParams);
                    break;
                }
                }
            }
            else
            {
                writeMarshalUnmarshalCode(out, "", indexTypes[i], OptionalNone, false, 0, "r", false, iter, false,
                                          metaData, patchParams);
            }
            out << nl << "return r;";
        }
        out << eb;

        //
        // extractKey
        //
        out << sp << nl << "protected " << indexKeyTypeS;
        out << nl << "extractKey(" << valueTypeS << " value)";
        out << sb;
        if(dict.indices[i].member.empty())
        {
            if(dict.indices[i].caseSensitive)
            {
                out << nl << "return value;";
            }
            else
            {
                out << nl << "return value.toLowerCase();";
            }
        }
        else
        {
            string member = "value." + dict.indices[i].member;
            if(!dict.indices[i].caseSensitive)
            {
                member += ".toLowerCase()";
            }
            out << nl << "return " << varToObject(indexTypes[i], member) << ";";
        }
        out << eb;

        //
        // marshalKey optimization
        //
        if(dict.indices[i].member.empty() && dict.indices[i].caseSensitive)
        {
            out << sp << nl << "protected java.nio.ByteBuffer";
            out << nl << "marshalKey(java.nio.ByteBuffer value)";
            out << sb;
            out << nl << "return value;";
            out << eb;
        }

        //
        // Constructor
        //
        out << sp << nl << "private" << nl << indexClassName << "(String name, java.util.Comparator<" << indexKeyTypeS
            << "> comparator)";
        out << sb;
        out << nl << "super(" << name << ".this, name, comparator);";
        out << eb;
        out << eb;
    }

    //
    // Patcher class.
    //
    BuiltinPtr b = BuiltinPtr::dynamicCast(valueType);
    if((b && b->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(valueType))
    {
        string typeS = typeToString(valueType, TypeModeIn);
        out << sp << nl << "private static class Patcher implements IceInternal.Patcher";
        out << sb;
        out << sp << nl << "public void" << nl << "patch(Ice.Object v)";
        out << sb;
        if(b)
        {
            out << nl << "value = v;";
        }
        else
        {
            out << nl << "value = (" << typeS << ")v;";
        }
        out << eb;
        out << sp << nl << "public String" << nl << "type()";
        out << sb;
        if(b)
        {
            out << nl << "return \"::Ice::Object\";";
        }
        else
        {
            ClassDeclPtr decl = ClassDeclPtr::dynamicCast(valueType);
            out << nl << "return \"" << decl->scoped() << "\";";
        }
        out << eb;
        out << sp << nl << typeS << " value;";
        out << eb;
    }

    //
    // Fields
    //
    if(!dict.indices.empty())
    {
        out << sp << nl << "private Freeze.MapIndex[] _indices;";
    }
    for(size_t i = 0; i < dict.indices.size(); ++i)
    {
        out << nl << "private " << capitalizedMembers[i] << "Index _" << members[i] << "Index;";
    }

    out << eb;

    close();
}

void
FreezeGenerator::generate(UnitPtr& u, const Index& index)
{
    string name;
    string::size_type pos = index.name.rfind('.');
    if(pos == string::npos)
    {
        name = index.name;
    }
    else
    {
        name = index.name.substr(pos + 1);
    }

    TypeList types = u->lookupType(index.type, false);
    if(types.empty())
    {
        ostringstream os;
        os << "`" << index.type << "' is not a valid type" << endl;
        throw os.str();
    }
    TypePtr type = types.front();

    ClassDeclPtr classDecl = ClassDeclPtr::dynamicCast(type);
    if(classDecl == 0)
    {
        ostringstream os;
        os << "`" << index.type << "' is not a class" << endl;
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
        os << "`" << index.type << "' has no data member named `" << index.member << "'" << endl;
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
            os << "`" << index.member << "'is not a string " << endl;
            throw os.str();
        }
    }

    string memberTypeString = typeToString(dataMember->type(), TypeModeIn);

    open(index.name, u->currentFile());

    Output& out = output();

    out << sp << nl << "public class " << name << " extends Freeze.Index";
    out << sb;

    //
    // Constructors
    //
    out << sp << nl << "public" << nl << name << "(String __indexName, String __facet)";
    out << sb;
    out << nl << "super(__indexName, __facet);";
    out << eb;

    out << sp << nl << "public" << nl << name << "(String __indexName)";
    out << sb;
    out << nl << "super(__indexName, \"\");";
    out << eb;

    //
    // find and count
    //
    out << sp << nl << "public Ice.Identity[]" << nl
        << "findFirst(" << memberTypeString << " __index, int __firstN)";
    out << sb;
    out << nl << "return untypedFindFirst(marshalKey(__index), __firstN);";
    out << eb;

    out << sp << nl << "public Ice.Identity[]" << nl
        << "find(" << memberTypeString << " __index)";
    out << sb;
    out << nl << "return untypedFind(marshalKey(__index));";
    out << eb;

    out << sp << nl << "public int" << nl
        << "count(" << memberTypeString << " __index)";
    out << sb;
    out << nl << "return untypedCount(marshalKey(__index));";
    out << eb;

    //
    // Key marshalling
    //
    string typeString = typeToString(type, TypeModeIn);

    out << sp << nl << "protected java.nio.ByteBuffer" << nl
        << "marshalKey(Ice.Object __servant)";
    out << sb;
    out << nl << "if(__servant instanceof " << typeString << ")";
    out << sb;
    out << nl <<  memberTypeString << " __key = ((" << typeString << ")__servant)." << index.member << ";";
    out << nl << "return marshalKey(__key);";
    out << eb;
    out << nl << "else";
    out << sb;
    out << nl << "return null;";
    out << eb;
    out << eb;

    string valueS = index.caseSensitive ? "__key" : "__key.toLowerCase()";

    out << sp << nl << "private java.nio.ByteBuffer" << nl
        << "marshalKey(" << memberTypeString << " __key)";
    out << sb;
    out << nl << "IceInternal.BasicStream __os = "
        << "new IceInternal.BasicStream(IceInternal.Util.getInstance(communicator()), encoding(), false);";
    int iter = 0;
    writeMarshalUnmarshalCode(out, "", dataMember->type(), OptionalNone, false, 0, valueS, true, iter, false);
    if(dataMember->type()->usesClasses())
    {
        out << nl << "__os.writePendingObjects();";
    }
    out << nl << "return __os.prepareWrite().b;";
    out << eb;

    out << eb;

    close();
}

void
usage(const char* n)
{
    getErrorStream() << "Usage: " << n << " [options] [slice-files...]\n";
    getErrorStream() <<
        "Options:\n"
        "-h, --help                Show this message.\n"
        "-v, --version             Display the Ice version.\n"
        "--validate                Validate command line options.\n"
        "-DNAME                    Define NAME as 1.\n"
        "-DNAME=DEF                Define NAME as DEF.\n"
        "-UNAME                    Remove any definition for NAME.\n"
        "-IDIR                     Put DIR in the include file search path.\n"
        "-E                        Print preprocessor output on stdout.\n"
        "--include-dir DIR         Use DIR as the header include directory.\n"
        "--dict NAME,KEY,VALUE     Create a Freeze dictionary with the name NAME,\n"
        "                          using KEY as key, and VALUE as value. This\n"
        "                          option may be specified multiple times for\n"
        "                          different names. NAME may be a scoped name.\n"
        "--index NAME,TYPE,MEMBER[,{case-sensitive|case-insensitive}]\n"
        "                          Create a Freeze evictor index with the name\n"
        "                          NAME for member MEMBER of class TYPE. This\n"
        "                          option may be specified multiple times for\n"
        "                          different names. NAME may be a scoped name.\n"
        "                          When member is a string, the case can be\n"
        "                          sensitive or insensitive (default is sensitive).\n"
        "--dict-index DICT[,MEMBER][,{case-sensitive|case-insensitive}] \n"
        "                          Add an index to dictionary DICT. If MEMBER is \n"
        "                          specified, then DICT's VALUE must be a class or\n"
        "                          a struct, and MEMBER must designate a member of\n"
        "                          VALUE. Otherwise, the entire VALUE is used for \n"
        "                          indexing. When the secondary key is a string, \n"
        "                          the case can be sensitive or insensitive (default\n"
        "                          is sensitive).\n"
        "--output-dir DIR          Create files in the directory DIR.\n"
        "--depend                  Generate Makefile dependencies.\n"
        "--depend-xml              Generate dependencies in XML format.\n"
        "--depend-file FILE        Write dependencies to FILE instead of standard output.\n"
        "-d, --debug               Print debug messages.\n"
        "--ice                     Allow reserved Ice prefix in Slice identifiers.\n"
        "--underscore              Allow underscores in Slice identifiers.\n"
        "--meta META               Define global metadata directive META.\n"
        ;
}

int
compile(int argc, char* argv[])
{
    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("", "validate");
    opts.addOpt("D", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("U", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("I", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("E");
    opts.addOpt("", "include-dir", IceUtilInternal::Options::NeedArg);
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
    opts.addOpt("", "meta", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);

    bool validate = false;
    for(int i = 0; i < argc; ++i)
    {
        if(string(argv[i]) == "--validate")
        {
            validate = true;
            break;
        }
    }

    vector<string> args;
    try
    {
        args = opts.parse(argc, const_cast<const char**>(argv));
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

    vector<string> cppArgs;
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

    bool preprocess = opts.isSet("E");

    string include = opts.optArg("include-dir");

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
            dict.key = s.substr(0, pos);
            s.erase(0, pos + 1);
        }
        dict.value = s;

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

    if(opts.isSet("dict-index"))
    {
        vector<string> optargs = opts.argVec("dict-index");
        for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
        {
            string s = IceUtilInternal::removeWhitespace(*i);

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
                getErrorStream() << argv[0] << ": error: " << *i << ": no dictionary specified" << endl;
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
    }

    string output = opts.optArg("output-dir");

    bool depend = opts.isSet("depend");
    bool dependxml = opts.isSet("depend-xml");

    string dependFile = opts.optArg("depend-file");

    bool debug = opts.isSet("debug");

    bool ice = opts.isSet("ice");

    bool underscore = opts.isSet("underscore");

    StringList globalMetadata;
    vector<string> v = opts.argVec("meta");
    copy(v.begin(), v.end(), back_inserter(globalMetadata));

    if(dicts.empty() && indices.empty() && !(depend || dependxml))
    {
        getErrorStream() << argv[0] << ": error: no Freeze types specified" << endl;
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

    UnitPtr u = Unit::createUnit(true, false, ice, underscore, globalMetadata);

    int status = EXIT_SUCCESS;

    IceUtil::CtrlCHandler ctrlCHandler;
    ctrlCHandler.setCallback(interruptedCallback);

    DependOutputUtil out(dependFile);
    if(dependxml)
    {
        out.os() << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dependencies>" << endl;
    }

    for(vector<string>::size_type idx = 0; idx < args.size(); ++idx)
    {
        if(depend || dependxml)
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], args[idx], cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2FREEZEJ__");

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

            if(!icecpp->printMakefileDependencies(out.os(), depend ? Preprocessor::Java : Preprocessor::SliceXML, includePaths,
                                                  "-D__SLICE2FREEZEJ__"))
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
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], args[idx], cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-DICE_COMPILER=ICE_SLICE2FREEZEJ");

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
                out.cleanup();
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
        u->mergeModules();
        u->sort();

        FreezeGenerator gen(argv[0], output);

        JavaGenerator::validateMetaData(u);

        for(vector<Dict>::const_iterator p = dicts.begin(); p != dicts.end(); ++p)
        {
            try
            {
                gen.generate(u, *p);
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
                FileTracker::instance()->cleanup();
                getErrorStream() << argv[0] << ": error: unknown exception" << endl;
                u->destroy();
                return EXIT_FAILURE;
            }
        }

        for(vector<Index>::const_iterator q = indices.begin(); q != indices.end(); ++q)
        {
            try
            {
                gen.generate(u, *q);
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

int
main(int argc, char* argv[])
{
    try
    {
        return compile(argc, argv);
    }
    catch(const std::exception& ex)
    {
        getErrorStream() << argv[0] << ": error:" << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch(const std::string& msg)
    {
        getErrorStream() << argv[0] << ": error:" << msg << endl;
        return EXIT_FAILURE;
    }
    catch(const char* msg)
    {
        getErrorStream() << argv[0] << ": error:" << msg << endl;
        return EXIT_FAILURE;
    }
    catch(...)
    {
        getErrorStream() << argv[0] << ": error:" << "unknown exception" << endl;
        return EXIT_FAILURE;
    }
}
