// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <Slice/Preprocessor.h>
#include <Slice/JavaUtil.h>

#ifdef __BCPLUSPLUS__
#  include <iterator>
#endif

using namespace std;
using namespace Slice;
using namespace IceUtil;

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

    bool generate(UnitPtr&, const Dict&);

    bool generate(UnitPtr&, const Index&);

private:
    string varToObject(const TypePtr&, const string&);
    string objectToVar(const TypePtr&, const string&);

    string _prog;
};

FreezeGenerator::FreezeGenerator(const string& prog, const string& dir)
    : JavaGenerator(dir),
      _prog(prog)
{
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
                result = string("new java.lang.Byte(") + param + ")";
                break;
            }
            case Builtin::KindBool:
            {
                result = string("new java.lang.Boolean(") + param + ")";
                break;
            }
            case Builtin::KindShort:
            {
                result = string("new java.lang.Short(") + param + ")";
                break;
            }
            case Builtin::KindInt:
            {
                result = string("new java.lang.Integer(") + param + ")";
                break;
            }
            case Builtin::KindLong:
            {
                result = string("new java.lang.Long(") + param + ")";
                break;
            }
            case Builtin::KindFloat:
            {
                result = string("new java.lang.Float(") + param + ")";
                break;
            }
            case Builtin::KindDouble:
            {
                result = string("new java.lang.Double(") + param + ")";
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
    string result = string("((") + typeToString(type, TypeModeIn) + ")" + param + ")";
    
    BuiltinPtr b = BuiltinPtr::dynamicCast(type);
    if(b != 0)
    {
        switch(b->kind())
        {
            case Builtin::KindByte:
            {
                result = string("((java.lang.Byte)") + param + ").byteValue()";
                break;
            }
            case Builtin::KindBool:
            {
                result = string("((java.lang.Boolean)") + param + ").booleanValue()";
                break;
            }
            case Builtin::KindShort:
            {
                result = string("((java.lang.Short)") + param + ").shortValue()";
                break;
            }
            case Builtin::KindInt:
            {
                result = string("((java.lang.Integer)") + param + ").intValue()";
                break;
            }
            case Builtin::KindLong:
            {
                result = string("((java.lang.Long)") + param + ").longValue()";
                break;
            }
            case Builtin::KindFloat:
            {
                result = string("((java.lang.Float)") + param + ").floatValue()";
                break;
            }
            case Builtin::KindDouble:
            {
                result = string("((java.lang.Double)") + param + ").doubleValue()";
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

bool
FreezeGenerator::generate(UnitPtr& u, const Dict& dict)
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
        cerr << _prog << ": `" << dict.key << "' is not a valid type" << endl;
        return false;
    }
    TypePtr keyType = keyTypes.front();
    
    TypeList valueTypes = u->lookupType(dict.value, false);
    if(valueTypes.empty())
    {
        cerr << _prog << ": `" << dict.value << "' is not a valid type" << endl;
        return false;
    }
    TypePtr valueType = valueTypes.front();

    vector<TypePtr> indexTypes;
    vector<string> capitalizedMembers;
    vector<string> indexNames;
    size_t i;
    
    for(i = 0; i < dict.indices.size(); ++i)
    {
        const DictIndex& index = dict.indices[i];
        const string& member = index.member;

        if(index.member.empty())
        {
            if(dict.indices.size() > 1)
            {
                cerr << _prog << ": bad index for dictionary `" << dict.name << "'" << endl;
                return false;
            }

            if(!Dictionary::legalKeyType(valueType))
            {
                cerr << _prog << ": `" << dict.value << "' is not a valid index type" << endl;
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
                    cerr << _prog << ": VALUE is a `" << dict.value << "', not a string " << endl;
                    return false; 
                }
            }
            indexTypes.push_back(valueType);
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
                    cerr << _prog << ": `" << dict.value << "' is neither a class nor a struct." << endl;
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
                cerr << _prog << ": The value of `" << dict.name 
                     << "' has no data member named `" << index.member << "'" << endl;
                return false;
            }
            
            TypePtr dataMemberType = dataMember->type();
            
            if(!Dictionary::legalKeyType(dataMemberType))
            {
                cerr << _prog << ": `" << index.member << "' cannot be used as an index" << endl;
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
                    cerr << _prog << ": `" << index.member << "' is not a string " << endl;
                    return false;
                }
            }
            indexTypes.push_back(dataMemberType);

            string capitalizedMember = member;
            capitalizedMember[0] = toupper(capitalizedMember[0]);
            capitalizedMembers.push_back(capitalizedMember);
            indexNames.push_back(member);
        }
    }


    if(!open(dict.name))
    {
        cerr << _prog << ": unable to open class " << dict.name << endl;
        return false;
    }

    Output& out = output();

    out << sp << nl << "public class " << name << " extends Freeze.Map";
    out << sb;

    //
    // Constructors
    //
    if(dict.indices.size() > 0)
    {
        out << sp << nl << "public" << nl << name 
            << "(Freeze.Connection __connection, String __dbName, boolean __createDb, "
            << "java.util.Comparator __comparator, java.util.Map __indexComparators)";
        out << sb;
        
        out << nl << "super(__connection, __dbName, __comparator);";
        out << nl << "_indices = new Freeze.Map.Index[" << dict.indices.size() << "];";
        for(i = 0; i < dict.indices.size(); ++i)
        {
            out << nl << "_indices[" << i << "] = new " << capitalizedMembers[i] 
                << "Index(\"" << indexNames[i] << "\");";
        }
        out << nl << "init(_indices, __dbName, \"" << keyType->typeId() << "\", \""
            << valueType->typeId() << "\", __createDb, __indexComparators);";
        out << eb;
    }

    out << sp << nl << "public" << nl << name 
        << "(Freeze.Connection __connection, String __dbName, boolean __createDb, "
        << "java.util.Comparator __comparator)";
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
    
    out << sp << nl << "public" << nl << name 
        << "(Freeze.Connection __connection, String __dbName, boolean __createDb)";
    out << sb;
    out << nl << "this(__connection, __dbName, __createDb, null);";
    out << eb;

    out << sp << nl << "public" << nl << name 
        << "(Freeze.Connection __connection, String __dbName)";
    out << sb;
    out << nl << "this(__connection, __dbName, true);";
    out << eb;

    //
    // findBy and count methods
    // 
    for(i = 0; i < capitalizedMembers.size(); ++i)
    {
        string indexClassName = capitalizedMembers[i] + "Index";

        out << sp << nl << "public Freeze.Map.EntryIterator";
        out << nl << "findBy" << capitalizedMembers[i] << "("
            << typeToString(indexTypes[i], TypeModeIn) << " __index, boolean __onlyDups)";
        out << sb;
        out << nl << "return _indices[" << i << "].untypedFind("
            << varToObject(indexTypes[i], "__index") << ", __onlyDups);"; 
        out << eb;

        out << sp << nl << "public Freeze.Map.EntryIterator";
        out << nl << "findBy" << capitalizedMembers[i] << "("
            << typeToString(indexTypes[i], TypeModeIn) << " __index)";
        out << sb;
        out << nl << "return _indices[" << i << "].untypedFind("
            << varToObject(indexTypes[i], "__index") << ", true);"; 
        out << eb;
        
        string countMethod = dict.indices[i].member.empty() ?
            string("valueCount") : dict.indices[i].member + "Count";
        out << sp << nl << "public int";
        out << nl << countMethod << "("
            << typeToString(indexTypes[i], TypeModeIn) << " __index)";
        out << sb;
        out << nl << "return _indices[" << i << "].untypedCount("
            << varToObject(indexTypes[i], "__index") << ");"; 
        out << eb;
    }
    
    //
    // Top-level encode/decode
    //
    for(i = 0; i < 2; i++)
    {
        string keyValue;
        TypePtr type;
        bool encaps;

        if(i == 0)
        {
            keyValue = "Key";
            type = keyType;
            //
            // Do not encapsulate keys.
            //
            encaps = false;
        }
        else
        {
            keyValue = "Value";
            type = valueType;
            encaps = true;
        }

        string valS = objectToVar(type, "o");
        string typeS;

        BuiltinPtr b = BuiltinPtr::dynamicCast(type);
        if(b != 0)
        {
            typeS = builtinTable[b->kind()];
        }
        else
        {
            typeS = typeToString(type, TypeModeIn);
        }

        int iter;

        //
        // encode
        //
        out << sp << nl << "public byte[]" << nl << "encode" << keyValue
            << "(Object o, Ice.Communicator communicator)";
        out << sb;
        out << nl << "assert(o instanceof " << typeS << ");";
        out << nl << "IceInternal.BasicStream __os = "
            << "new IceInternal.BasicStream(Ice.Util.getInstance(communicator));";
        if(encaps)
        {
            out << nl << "__os.startWriteEncaps();";
        }
        iter = 0;
        writeMarshalUnmarshalCode(out, "", type, valS, true, iter, false);
        if(type->usesClasses())
        {
            out << nl << "__os.writePendingObjects();";
        }
        if(encaps)
        {
            out << nl << "__os.endWriteEncaps();";
        }
        out << nl << "java.nio.ByteBuffer __buf = __os.prepareWrite();";
        out << nl << "byte[] __r = new byte[__buf.limit()];";
        out << nl << "__buf.get(__r);";
        out << nl << "return __r;";
        out << eb;

        //
        // decode
        //
        out << sp << nl << "public Object" << nl << "decode" << keyValue
            << "(byte[] b, Ice.Communicator communicator)";
        out << sb;
        out << nl << "IceInternal.BasicStream __is = new IceInternal.BasicStream(Ice.Util.getInstance(communicator));";
        if(type->usesClasses())
        {
            out << nl << "__is.sliceObjects(false);";
        }
        out << nl << "__is.resize(b.length, true);";
        out << nl << "java.nio.ByteBuffer __buf = __is.prepareRead();";
        out << nl << "__buf.position(0);";
        out << nl << "__buf.put(b);";
        out << nl << "__buf.position(0);";
        if(encaps)
        {
            out << nl << "__is.startReadEncaps();";
        }
        iter = 0;
        list<string> metaData;
        string patchParams;
        if((b && b->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(type))
        {
            out << nl << "Patcher __p = new Patcher();";
            patchParams = "__p";
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
                    out << nl << "__r = new java.lang.Byte(__is.readByte());";
                    break;
                }
                case Builtin::KindBool:
                {
                    out << nl << "__r = new java.lang.Boolean(__is.readBool());";
                    break;
                }
                case Builtin::KindShort:
                {
                    out << nl << "__r = new java.lang.Short(__is.readShort());";
                    break;
                }
                case Builtin::KindInt:
                {
                    out << nl << "__r = new java.lang.Integer(__is.readInt());";
                    break;
                }
                case Builtin::KindLong:
                {
                    out << nl << "__r = new java.lang.Long(__is.readLong());";
                    break;
                }
                case Builtin::KindFloat:
                {
                    out << nl << "__r = new java.lang.Float(__is.readFloat());";
                    break;
                }
                case Builtin::KindDouble:
                {
                    out << nl << "__r = new java.lang.Double(__is.readDouble());";
                    break;
                }
                case Builtin::KindString:
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                {
                    writeMarshalUnmarshalCode(out, "", type, "__r", false, iter, false, metaData, patchParams);
                    break;
                }
            }
        }
        else
        {
            writeMarshalUnmarshalCode(out, "", type, "__r", false, iter, false, metaData, patchParams);
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
    for(i = 0; i < capitalizedMembers.size(); ++i)
    {
        string indexClassName = capitalizedMembers[i] + "Index";
        out << sp << nl << "private class " << indexClassName << " extends Freeze.Map.Index";
        out << sb;

        //
        // encodeKey
        //
        out << sp << nl << "public byte[]";
        out << nl << "encodeKey(Object key, Ice.Communicator communicator)";
        out << sb;
        if(dict.indices[i].member.empty())
        {
            //
            // Encode the full value (with an encaps!)
            //
            string keyS = "key";
            if(!dict.indices[i].caseSensitive)
            {
                keyS = "((String)key).toLowerCase()";
            }

            out << nl << "return encodeValue(" << keyS << ", communicator);";
        }
        else
        {
            //
            // No encaps
            //
            string keyS = dict.indices[i].caseSensitive ? 
                "key" : "((String)key).toLowerCase()";

            keyS = objectToVar(indexTypes[i], keyS);

            out << nl << "IceInternal.BasicStream __os = "
                << "new IceInternal.BasicStream(Ice.Util.getInstance(communicator));";
            int iter = 0;
            writeMarshalUnmarshalCode(out, "", indexTypes[i], keyS, true, iter, false);
            assert(!indexTypes[i]->usesClasses());

            out << nl << "java.nio.ByteBuffer buf = __os.prepareWrite();";
            out << nl << "byte[] r = new byte[buf.limit()];";
            out << nl << "buf.get(r);";
            out << nl << "return r;";
        }
        out << eb;

        //
        // decodekey
        //
        out << sp << nl << "public Object";
        out << nl << "decodeKey(byte[] bytes, Ice.Communicator communicator)";
        out << sb;
        if(dict.indices[i].member.empty())
        {
            //
            // Decode the full value (with an encaps!)
            //
            out << nl << "return decodeValue(bytes, communicator);";
        }
        else
        {
            out << nl << "IceInternal.BasicStream __is = new IceInternal.BasicStream(Ice.Util.getInstance(communicator));";
            out << nl << "__is.resize(bytes.length, true);";
            out << nl << "java.nio.ByteBuffer buf = __is.prepareRead();";
            out << nl << "buf.position(0);";
            out << nl << "buf.put(bytes);";
            out << nl << "buf.position(0);";
            
            int iter = 0;
            list<string> metaData;
            string patchParams;

            string typeS;
            BuiltinPtr b = BuiltinPtr::dynamicCast(indexTypes[i]);
            if(b != 0)
            {
                typeS = builtinTable[b->kind()];
            }
            else
            {
                typeS = typeToString(indexTypes[i], TypeModeIn);
            }
            out << nl << typeS << " r;";
         
            if(b != 0)
            {
                switch(b->kind())
                {
                    case Builtin::KindByte:
                    {
                        out << nl << "r = new java.lang.Byte(__is.readByte());";
                        break;
                    }
                    case Builtin::KindBool:
                    {
                        out << nl << "r = new java.lang.Boolean(__is.readBool());";
                        break;
                    }
                    case Builtin::KindShort:
                    {
                        out << nl << "r = new java.lang.Short(__is.readShort());";
                        break;
                    }
                    case Builtin::KindInt:
                    {
                        out << nl << "r = new java.lang.Integer(__is.readInt());";
                        break;
                    }
                    case Builtin::KindLong:
                    {
                        out << nl << "r = new java.lang.Long(__is.readLong());";
                        break;
                    }
                    case Builtin::KindFloat:
                    {
                        out << nl << "r = new java.lang.Float(__is.readFloat());";
                        break;
                    }
                    case Builtin::KindDouble:
                    {
                        out << nl << "r = new java.lang.Double(__is.readDouble());";
                        break;
                    }
                    case Builtin::KindString:
                    case Builtin::KindObject:
                    case Builtin::KindObjectProxy:
                    case Builtin::KindLocalObject:
                    {
                        writeMarshalUnmarshalCode(out, "", indexTypes[i], "r", false, iter, false, metaData, patchParams);
                        break;
                    }
                }
            }
            else
            {
                writeMarshalUnmarshalCode(out, "", indexTypes[i], "r", false, iter, false, metaData, patchParams);
            }
            out << nl << "return r;";
        }
        out << eb;

        //
        // compare
        //
        out << sp << nl << "public int";
        out << nl << "compare(Object o1, Object o2)";
        out << sb;
        out << nl << "assert _comparator != null;";
        out << nl << "byte[] d1 = (byte[])o1;";
        out << nl << "byte[] d2 = (byte[])o2;";
        out << nl << "Ice.Communicator communicator = ((Freeze.Connection)_connection).getCommunicator();";
        out << nl << "return _comparator.compare(";
        out.inc();
        out << nl << "decodeKey(d1, communicator),";
        out << nl << "decodeKey(d2, communicator));";
        out.dec();
        out << eb;

        //
        // extractKey from value
        //
        out << sp << nl << "public Object";
        out << nl << "extractKey(Object value)";
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
            out << nl << typeToString(valueType, TypeModeIn)
                << " typedValue = ("
                << typeToString(valueType, TypeModeIn) << ")value;";
         
            string member = string("typedValue.") + dict.indices[i].member;
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
            out << sp << nl << "protected byte[]";
            out << nl << "marshalKey(byte[] value)";
            out << sb;
            out << nl << "return value;";
            out << eb;
        }

        out << sp << nl << "private " << indexClassName << "(String name)";
        out << sb;
        out << nl << "super(name);";
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
    out << sp << nl << "private Freeze.Map.Index[] _indices;";

    out << eb;


    close();

    return true;
}

bool
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
        cerr << _prog << ": `" << index.type << "' is not a valid type" << endl;
        return false;
    }
    TypePtr type = types.front();

    ClassDeclPtr classDecl = ClassDeclPtr::dynamicCast(type);
    if(classDecl == 0)
    {
        cerr << _prog << ": `" << index.type << "' is not a class" << endl;
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
        cerr << _prog << ": `" << index.type << "' has no data member named `" << index.member << "'" << endl;
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
            cerr << _prog << ": `" << index.member << "'is not a string " << endl;
            return false; 
        }
    }

    string memberTypeString = typeToString(dataMember->type(), TypeModeIn);
    
    if(!open(index.name))
    {
        cerr << _prog << ": unable to open class " << index.name << endl;
        return false;
    }

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

    out << sp << nl << "protected byte[]" << nl 
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

    out << sp << nl << "private byte[]" << nl 
        << "marshalKey(" << memberTypeString << " __key)";
    out << sb;
    out << nl << "IceInternal.BasicStream __os = "
        << "new IceInternal.BasicStream(Ice.Util.getInstance(communicator()));";
    int iter = 0;
    writeMarshalUnmarshalCode(out, "", dataMember->type(), valueS, true, iter, false);
    if(type->usesClasses())
    {
        out << nl << "__os.writePendingObjects();";
    }
    out << nl << "java.nio.ByteBuffer __buf = __os.prepareWrite();";
    out << nl << "byte[] __r = new byte[__buf.limit()];";
    out << nl << "__buf.get(__r);";
    out << nl << "return __r;";
    out << eb;

    out << eb;

    close();

    return true;
}


void
usage(const char* n)
{
    cerr << "Usage: " << n << " [options] [slice-files...]\n";
    cerr <<
        "Options:\n"
        "-h, --help                Show this message.\n"
        "-v, --version             Display the Ice version.\n"
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
        "-d, --debug               Print debug messages.\n"
        "--ice                     Permit `Ice' prefix (for building Ice source code only)\n"
        "--meta META               Define global metadata directive META.\n"
        ;
    // Note: --case-sensitive is intentionally not shown here!
}

int
main(int argc, char* argv[])
{
    IceUtil::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("D", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("U", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("I", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("E");
    opts.addOpt("", "include-dir", IceUtil::Options::NeedArg);
    opts.addOpt("", "dict", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("", "index", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("", "dict-index", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("", "output-dir", IceUtil::Options::NeedArg);
    opts.addOpt("", "depend");
    opts.addOpt("d", "debug");
    opts.addOpt("", "ice");
    opts.addOpt("", "meta", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("", "case-sensitive");
     
    vector<string> args;
    try
    {
        args = opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtil::BadOptException& e)
    {
        cerr << argv[0] << ": " << e.reason << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if(opts.isSet("help"))
    {
        usage(argv[0]);
        return EXIT_SUCCESS;
    }

    if(opts.isSet("version"))
    {
        cout << ICE_STRING_VERSION << endl;
        return EXIT_SUCCESS;
    }

    string cppArgs;
    vector<string> optargs = opts.argVec("D");
    vector<string>::const_iterator i;
    for(i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs += " -D" + Preprocessor::addQuotes(*i);
    }

    optargs = opts.argVec("U");
    for(i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs += " -U" + Preprocessor::addQuotes(*i);
    }

    vector<string> includePaths = opts.argVec("I");
    for(i = includePaths.begin(); i != includePaths.end(); ++i)
    {
	cppArgs += " -I" + Preprocessor::normalizeIncludePath(*i);
    }

    bool preprocess = opts.isSet("E");

    string include = opts.optArg("include-dir");

    vector<Dict> dicts;
    optargs = opts.argVec("dict");
    for(i = optargs.begin(); i != optargs.end(); ++i)
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

    vector<Index> indices;
    optargs = opts.argVec("index");
    for(i = optargs.begin(); i != optargs.end(); ++i)
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

    if(opts.isSet("dict-index"))
    {
        vector<string> optargs = opts.argVec("dict-index");
        for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
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

    string output = opts.optArg("output-dir");

    bool depend = opts.isSet("depend");

    bool debug = opts.isSet("debug");

    bool ice = opts.isSet("ice");

    StringList globalMetadata;
    vector<string> v = opts.argVec("meta");
    copy(v.begin(), v.end(), back_inserter(globalMetadata));

    bool caseSensitive = opts.isSet("case-sensitive");

    if(dicts.empty() && indices.empty())
    {
        cerr << argv[0] << ": no Freeze types specified" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    UnitPtr u = Unit::createUnit(true, false, ice, caseSensitive, globalMetadata);

    int status = EXIT_SUCCESS;

    for(vector<string>::size_type idx = 0; idx < args.size(); ++idx)
    {
        if(depend)
        {
            Preprocessor icecpp(argv[0], args[idx], cppArgs);
            icecpp.printMakefileDependencies(Preprocessor::Java);
        }
        else
        {
            Preprocessor icecpp(argv[0], args[idx], cppArgs);
            FILE* cppHandle = icecpp.preprocess(false);

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
                status = u->parse(cppHandle, debug);
            }

            if(!icecpp.close())
            {
                u->destroy();
                return EXIT_FAILURE;
            }       
        }
    }

    if(depend)
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
                if(!gen.generate(u, *p))
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
                if(!gen.generate(u, *q))
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
    
    u->destroy();

    return status;
}
