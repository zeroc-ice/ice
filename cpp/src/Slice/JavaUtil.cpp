// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/JavaUtil.h>
#include <IceUtil/Functional.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#endif

#ifndef _WIN32
#include <unistd.h>
#endif

using namespace std;
using namespace Slice;
using namespace IceUtil;

Slice::JavaOutput::JavaOutput()
{
}

Slice::JavaOutput::JavaOutput(ostream& os) :
    Output(os)
{
}

Slice::JavaOutput::JavaOutput(const char* s) :
    Output(s)
{
}

bool
Slice::JavaOutput::openClass(const string& cls, const string& prefix)
{
    string package;
    string file;
    string path = prefix;

    string::size_type pos = cls.rfind('.');
    if(pos != string::npos)
    {
        package = cls.substr(0, pos);
        file = cls.substr(pos + 1);
        string dir = package;

        //
        // Create package directories if necessary.
        //
        pos = 0;
        string::size_type start = 0;
        do
        {
            if(!path.empty())
            {
                path += "/";
            }
            pos = dir.find('.', start);
            if(pos != string::npos)
            {
                path += dir.substr(start, pos - start);
                start = pos + 1;
            }
            else
            {
                path += dir.substr(start);
            }

            struct stat st;
            int result;
            result = stat(path.c_str(), &st);
            if(result == 0)
            {
                continue;
            }
#ifdef _WIN32
            result = _mkdir(path.c_str());
#else       
            result = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
#endif
            if(result != 0)
            {
                return false;
            }
        }
        while(pos != string::npos);
    }
    else
    {
        file = cls;
    }
    file += ".java";

    //
    // Open class file.
    //
    if(!path.empty())
    {
        path += "/";
    }
    path += file;

    open(path.c_str());
    if(isOpen())
    {
        printHeader();

        if(!package.empty())
        {
            separator();
            separator();
            print("package ");
            print(package.c_str());
            print(";");
        }

        return true;
    }

    return false;
}

void
Slice::JavaOutput::printHeader()
{
    static const char* header =
"// **********************************************************************\n"
"//\n"
"// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.\n"
"//\n"
"// This copy of Ice is licensed to you under the terms described in the\n"
"// ICE_LICENSE file included in this distribution.\n"
"//\n"
"// **********************************************************************\n"
        ;

    print(header);
    print("\n// Ice version ");
    print(ICE_STRING_VERSION);
}

const string Slice::JavaGenerator::_getSetMetaData = "java:getset";
const string Slice::JavaGenerator::_java2MetaData = "java:java2";
const string Slice::JavaGenerator::_java5MetaData = "java:java5";

Slice::JavaGenerator::JavaGenerator(const string& dir) :
    _featureProfile(Slice::Ice),
    _dir(dir),
    _out(0)
{
}

Slice::JavaGenerator::JavaGenerator(const string& dir, Slice::FeatureProfile profile) :
    _featureProfile(profile),
    _dir(dir),
    _out(0)
{
}

Slice::JavaGenerator::~JavaGenerator()
{
    assert(_out == 0);
}

bool
Slice::JavaGenerator::open(const string& absolute)
{
    assert(_out == 0);

    JavaOutput* out = createOutput();
    if(out->openClass(absolute, _dir))
    {
        _out = out;
    }
    else
    {
        delete out;
    }

    return _out != 0;
}

void
Slice::JavaGenerator::close()
{
    assert(_out != 0);
    *_out << nl;
    delete _out;
    _out = 0;
}

Output&
Slice::JavaGenerator::output() const
{
    assert(_out != 0);
    return *_out;
}

static string
lookupKwd(const string& name)
{
    //
    // Keyword list. *Must* be kept in alphabetical order. Note that checkedCast and uncheckedCast
    // are not Java keywords, but are in this list to prevent illegal code being generated if
    // someone defines Slice operations with that name.
    //
    // NOTE: Any changes made to this list must also be made in BasicStream.java.
    //
    static const string keywordList[] = 
    {       
        "abstract", "assert", "boolean", "break", "byte", "case", "catch",
        "char", "checkedCast", "class", "clone", "const", "continue", "default", "do",
        "double", "else", "enum", "equals", "extends", "false", "final", "finalize",
        "finally", "float", "for", "getClass", "goto", "hashCode", "if",
        "implements", "import", "instanceof", "int", "interface", "long",
        "native", "new", "notify", "notifyAll", "null", "package", "private",
        "protected", "public", "return", "short", "static", "strictfp", "super", "switch",
        "synchronized", "this", "throw", "throws", "toString", "transient",
        "true", "try", "uncheckedCast", "void", "volatile", "wait", "while"
    };
    bool found =  binary_search(&keywordList[0],
                                &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
                                name);
    return found ? "_" + name : name;
}

//
// Split a scoped name into its components and return the components as a list of (unscoped) identifiers.
//
static StringList
splitScopedName(const string& scoped)
{
    assert(scoped[0] == ':');
    StringList ids;
    string::size_type next = 0;
    string::size_type pos;
    while((pos = scoped.find("::", next)) != string::npos)
    {
        pos += 2;
        if(pos != scoped.size())
        {
            string::size_type endpos = scoped.find("::", pos);
            if(endpos != string::npos)
            {
                ids.push_back(scoped.substr(pos, endpos - pos));
            }
        }
        next = pos;
    }
    if(next != scoped.size())
    {
        ids.push_back(scoped.substr(next));
    }
    else
    {
        ids.push_back("");
    }

    return ids;
}

//
// If the passed name is a scoped name, return the identical scoped name,
// but with all components that are Java keywords replaced by
// their "_"-prefixed version; otherwise, if the passed name is
// not scoped, but a Java keyword, return the "_"-prefixed name;
// otherwise, return the name unchanged.
//
string
Slice::JavaGenerator::fixKwd(const string& name) const
{
    if(name.empty())
    {
        return name;
    }
    if(name[0] != ':')
    {
        return lookupKwd(name);
    }
    StringList ids = splitScopedName(name);
    transform(ids.begin(), ids.end(), ids.begin(), ptr_fun(lookupKwd));
    stringstream result;
    for(StringList::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        result << "::" + *i;
    }
    return result.str();
}

string
Slice::JavaGenerator::convertScopedName(const string& scoped, const string& prefix, const string& suffix) const
{
    string result;
    string::size_type start = 0;
    string fscoped = fixKwd(scoped);

    //
    // Skip leading "::"
    //
    if(fscoped[start] == ':')
    {
        assert(fscoped[start + 1] == ':');
        start += 2;
    }

    //
    // Convert all occurrences of "::" to "."
    //
    string::size_type pos;
    do
    {
        pos = fscoped.find(':', start);
        string fix;
        if(pos == string::npos)
        {
            string s = fscoped.substr(start);
            if(!s.empty())
            {
                fix = prefix + fixKwd(s) + suffix;
            }
        }
        else
        {
            assert(fscoped[pos + 1] == ':');
            fix = fixKwd(fscoped.substr(start, pos - start));
            start = pos + 2;
        }

        if(!result.empty() && !fix.empty())
        {
            result += ".";
        }
        result += fix;
    }
    while(pos != string::npos);

    return result;
}

string
Slice::JavaGenerator::getPackage(const ContainedPtr& cont) const
{
    string scope = convertScopedName(cont->scope());

    DefinitionContextPtr dc = cont->definitionContext();
    if(dc)
    {
        static const string prefix = "java:package:";
        string package = dc->findMetaData(prefix);
        if(!package.empty())
        {
            if(!scope.empty())
            {
                return package.substr(prefix.size()) + "." + scope;
            }
            else
            {
                return package.substr(prefix.size());
            }
        }
    }

    return scope;
}

string
Slice::JavaGenerator::getAbsolute(const ContainedPtr& cont,
                                  const string& package,
                                  const string& prefix,
                                  const string& suffix) const
{
    string name = cont->name();
    if(prefix == "" && suffix == "")
    {
        name = fixKwd(name);
    }
    string contPkg = getPackage(cont);
    if(contPkg == package)
    {
        return prefix + name + suffix;
    }
    else if(!contPkg.empty())
    {
        return contPkg + "." + prefix + name + suffix;
    }
    else
    {
        return prefix + name + suffix;
    }
}

string
Slice::JavaGenerator::typeToString(const TypePtr& type,
                                   TypeMode mode,
                                   const string& package,
                                   const StringList& metaData,
                                   bool abstract) const
{
    static const char* builtinTable[] =
    {
        "byte",
        "boolean",
        "short",
        "int",
        "long",
        "float",
        "double",
        "String",
        "Ice.Object",
        "Ice.ObjectPrx",
        "java.lang.Object"
    };
    static const char* builtinHolderTable[] =
    {
        "Ice.ByteHolder",
        "Ice.BooleanHolder",
        "Ice.ShortHolder",
        "Ice.IntHolder",
        "Ice.LongHolder",
        "Ice.FloatHolder",
        "Ice.DoubleHolder",
        "Ice.StringHolder",
        "Ice.ObjectHolder",
        "Ice.ObjectPrxHolder",
        "Ice.LocalObjectHolder"
    };

    if(!type)
    {
        assert(mode == TypeModeReturn);
        return "void";
    }

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        if(mode == TypeModeOut)
        {
            return builtinHolderTable[builtin->kind()];
        }
        else
        {
            return builtinTable[builtin->kind()];
        }
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        return getAbsolute(cl, package, "", mode == TypeModeOut ? "Holder" : "");
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        return getAbsolute(proxy->_class(), package, "", mode == TypeModeOut ? "PrxHolder" : "Prx");
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        if(mode == TypeModeOut)
        {
            if(_featureProfile == Slice::IceE)
            {
                return getAbsolute(dict, package, "", "Holder");
            }
            else
            {
                //
                // Only use the type's generated holder if the concrete and
                // abstract types match.
                //
                string concreteType, abstractType;
                getDictionaryTypes(dict, "", metaData, concreteType, abstractType);
                string origConcreteType, origAbstractType;
                getDictionaryTypes(dict, "", StringList(), origConcreteType, origAbstractType);
                if(abstractType == origAbstractType && concreteType == origConcreteType)
                {
                    return getAbsolute(dict, package, "", "Holder");
                }

                bool java2 = dict->definitionContext()->findMetaData(_java2MetaData) == _java2MetaData;

                //
                // The custom type may or may not be compatible with the type used
                // in the generated holder. For Java5, we can use a generic holder
                // that holds a value of the abstract custom type. Otherwise, we
                // use MapHolder.
                //
                if(java2)
                {
                    return "Ice.MapHolder";
                }
                else
                {
                    return string("Ice.Holder<") + abstractType + " >";
                }
            }
        }
        else
        {
            string concreteType, abstractType;
            getDictionaryTypes(dict, package, metaData, concreteType, abstractType);
            return abstract ? abstractType : concreteType;
        }
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        if(mode == TypeModeOut)
        {
            if(_featureProfile == Slice::IceE)
            {
                return getAbsolute(seq, package, "", "Holder");
            }
            else
            {
                //
                // Only use the type's generated holder if the concrete and
                // abstract types match.
                //
                string concreteType, abstractType;
                getSequenceTypes(seq, "", metaData, concreteType, abstractType);
                string origConcreteType, origAbstractType;
                getSequenceTypes(seq, "", StringList(), origConcreteType, origAbstractType);
                if(abstractType == origAbstractType && concreteType == origConcreteType)
                {
                    return getAbsolute(seq, package, "", "Holder");
                }

                bool java2 = seq->definitionContext()->findMetaData(_java2MetaData) == _java2MetaData;

                //
                // The custom type may or may not be compatible with the type used
                // in the generated holder. For Java5, we can use a generic holder
                // that holds a value of the abstract custom type. Otherwise, we
                // choose a predefined holder class.
                //
                if(java2)
                {
                    if(abstractType == "java.util.ArrayList")
                    {
                        return "Ice.ArrayListHolder";
                    }
                    else if(abstractType == "java.util.LinkedList")
                    {
                        return "Ice.LinkedListHolder";
                    }
                    else
                    {
                        return "Ice.ListHolder";
                    }
                }
                else
                {
                    return string("Ice.Holder<") + abstractType + " >";
                }
            }
        }
        else
        {
            string concreteType, abstractType;
            getSequenceTypes(seq, package, metaData, concreteType, abstractType);
            return abstract ? abstractType : concreteType;
        }
    }

    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
        if(mode == TypeModeOut)
        {
            return getAbsolute(contained, package, "", "Holder");
        }
        else
        {
            return getAbsolute(contained, package);
        }
    }

    return "???";
}

string
Slice::JavaGenerator::typeToObjectString(const TypePtr& type,
                                         TypeMode mode,
                                         const string& package,
                                         const StringList& metaData,
                                         bool abstract) const
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
        "java.lang.Object"
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin && mode != TypeModeOut)
    {
        return builtinTable[builtin->kind()];
    }

    return typeToString(type, mode, package, metaData, abstract);
}

void
Slice::JavaGenerator::writeMarshalUnmarshalCode(Output& out,
                                                const string& package,
                                                const TypePtr& type,
                                                const string& param,
                                                bool marshal,
                                                int& iter,
                                                bool holder,
                                                const StringList& metaData,
                                                const string& patchParams)
{
    string stream = marshal ? "__os" : "__is";
    string v;
    if(holder)
    {
        v = param + ".value";
    }
    else
    {
        v = param;
    }

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindByte:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeByte(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readByte();";
                }
                break;
            }
            case Builtin::KindBool:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeBool(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readBool();";
                }
                break;
            }
            case Builtin::KindShort:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeShort(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readShort();";
                }
                break;
            }
            case Builtin::KindInt:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeInt(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readInt();";
                }
                break;
            }
            case Builtin::KindLong:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeLong(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readLong();";
                }
                break;
            }
            case Builtin::KindFloat:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeFloat(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readFloat();";
                }
                break;
            }
            case Builtin::KindDouble:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeDouble(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readDouble();";
                }
                break;
            }
            case Builtin::KindString:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeString(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readString();";
                }
                break;
            }
            case Builtin::KindObject:
            {
                if(_featureProfile != Slice::IceE)
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeObject(" << v << ");";
                    }
                    else
                    {
                        if(holder)
                        {
                            out << nl << stream << ".readObject(" << param << ".getPatcher());";
                        }
                        else
                        {
                            if(patchParams.empty())
                            {
                                out << nl << stream << ".readObject(new Patcher());";
                            }
                            else
                            {
                                out << nl << stream << ".readObject(" << patchParams << ");";
                            }
                        }
                    }
                }
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeProxy(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readProxy();";
                }
                break;
            }
            case Builtin::KindLocalObject:
            {
                assert(false);
                break;
            }
        }
        return;
    }

    ProxyPtr prx = ProxyPtr::dynamicCast(type);
    if(prx)
    {
        string typeS = typeToString(type, TypeModeIn, package);
        if(marshal)
        {
            out << nl << typeS << "Helper.__write(" << stream << ", " << v << ");";
        }
        else
        {
            out << nl << v << " = " << typeS << "Helper.__read(" << stream << ");";
        }
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(_featureProfile != Slice::IceE)
        {
            if(marshal)
            {
                out << nl << stream << ".writeObject(" << v << ");";
            }
            else
            {
                string typeS = typeToString(type, TypeModeIn, package);
                if(holder)
                {
                    out << nl << stream << ".readObject(" << param << ".getPatcher());";
                }
                else
                {
                    if(patchParams.empty())
                    {
                        out << nl << stream << ".readObject(new Patcher());";
                    }
                    else
                    {
                        out << nl << stream << ".readObject(" << patchParams << ");";
                    }
                }
            }
        }
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(marshal)
        {
            out << nl << v << ".__write(" << stream << ");";
        }
        else
        {
            string typeS = typeToString(type, TypeModeIn, package);
            out << nl << v << " = new " << typeS << "();";
            out << nl << v << ".__read(" << stream << ");";
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        if(marshal)
        {
            out << nl << v << ".__write(" << stream << ");";
        }
        else
        {
            string typeS = typeToString(type, TypeModeIn, package);
            out << nl << v << " = " << typeS << ".__read(" << stream << ");";
        }
        return;
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        writeDictionaryMarshalUnmarshalCode(out, package, dict, v, marshal, iter, true, metaData);
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        writeSequenceMarshalUnmarshalCode(out, package, seq, v, marshal, iter, true, metaData);
        return;
    }

    ConstructedPtr constructed = ConstructedPtr::dynamicCast(type);
    assert(constructed);
    string typeS = getAbsolute(constructed, package);
    if(marshal)
    {
        out << nl << typeS << "Helper.write(" << stream << ", " << v << ");";
    }
    else
    {
        out << nl << v << " = " << typeS << "Helper.read(" << stream << ");";
    }
}

void
Slice::JavaGenerator::writeDictionaryMarshalUnmarshalCode(Output& out,
                                                          const string& package,
                                                          const DictionaryPtr& dict,
                                                          const string& param,
                                                          bool marshal,
                                                          int& iter,
                                                          bool useHelper,
                                                          const StringList& metaData)
{
    string stream = marshal ? "__os" : "__is";
    string v = param;

    bool java2 = false;
    string concreteType;

    if(_featureProfile != Slice::IceE)
    {
        java2 = dict->definitionContext()->findMetaData(_java2MetaData) == _java2MetaData;

        //
        // We have to determine whether it's possible to use the
        // type's generated helper class for this marshal/unmarshal
        // task. Since the user may have specified a custom type in
        // metadata, it's possible that the helper class is not
        // compatible and therefore we'll need to generate the code
        // in-line instead.
        //
        // Specifically, there may be "local" metadata (i.e., from
        // a data member or parameter definition) that overrides the
        // original type. We'll compare the mapped types with and
        // without local metadata to determine whether we can use
        // the helper.
        //
        string abstractType;
        getDictionaryTypes(dict, "", metaData, concreteType, abstractType);
        string origConcreteType, origAbstractType;
        getDictionaryTypes(dict, "", StringList(), origConcreteType, origAbstractType);
        if((abstractType != origAbstractType) || (!marshal && concreteType != origConcreteType))
        {
            useHelper = false;
        }
    }

    //
    // If we can use the helper, it's easy.
    //
    if(useHelper)
    {
        string typeS = getAbsolute(dict, package);
        if(marshal)
        {
            out << nl << typeS << "Helper.write(" << stream << ", " << v << ");";
        }
        else
        {
            out << nl << v << " = " << typeS << "Helper.read(" << stream << ");";
        }
        return;
    }

    TypePtr key = dict->keyType();
    TypePtr value = dict->valueType();

    string keyS = typeToString(key, TypeModeIn, package);
    string valueS = typeToString(value, TypeModeIn, package);
    int i;

    ostringstream o;
    o << iter;
    string iterS = o.str();
    iter++;

    if(marshal)
    {
        out << nl << "if(" << v << " == null)";
        out << sb;
        out << nl << "__os.writeSize(0);";
        out << eb;
        out << nl << "else";
        out << sb;
        out << nl << "__os.writeSize(" << v << ".size());";
        if(java2)
        {
            out << nl << "java.util.Iterator __i" << iterS << " = " << v << ".entrySet().iterator();";
            out << nl << "while(__i" << iterS << ".hasNext())";
            out << sb;
            out << nl << "java.util.Map.Entry __e = (java.util.Map.Entry)" << "__i" << iterS << ".next();";
        }
        else
        {
            string keyObjectS = typeToObjectString(key, TypeModeIn, package);
            string valueObjectS = typeToObjectString(value, TypeModeIn, package);
            out << nl << "for(java.util.Map.Entry<" << keyObjectS << ", " << valueObjectS << "> __e : " << v
                << ".entrySet())";
            out << sb;
        }
        for(i = 0; i < 2; i++)
        {
            string val;
            string arg;
            TypePtr type;
            if(i == 0)
            {
                arg = "__e.getKey()";
                type = key;
            }
            else
            {
                arg = "__e.getValue()";
                type = value;
            }

            //
            // We have to downcast unless we're using Java5.
            //
            if(java2)
            {
                BuiltinPtr b = BuiltinPtr::dynamicCast(type);
                if(b)
                {
                    switch(b->kind())
                    {
                        case Builtin::KindByte:
                        {
                            val = "((java.lang.Byte)" + arg + ").byteValue()";
                            break;
                        }
                        case Builtin::KindBool:
                        {
                            val = "((java.lang.Boolean)" + arg + ").booleanValue()";
                            break;
                        }
                        case Builtin::KindShort:
                        {
                            val = "((java.lang.Short)" + arg + ").shortValue()";
                            break;
                        }
                        case Builtin::KindInt:
                        {
                            val = "((java.lang.Integer)" + arg + ").intValue()";
                            break;
                        }
                        case Builtin::KindLong:
                        {
                            val = "((java.lang.Long)" + arg + ").longValue()";
                            break;
                        }
                        case Builtin::KindFloat:
                        {
                            val = "((java.lang.Float)" + arg + ").floatValue()";
                            break;
                        }
                        case Builtin::KindDouble:
                        {
                            val = "((java.lang.Double)" + arg + ").doubleValue()";
                            break;
                        }
                        case Builtin::KindString:
                        case Builtin::KindObject:
                        case Builtin::KindObjectProxy:
                        {
                            break;
                        }
                        case Builtin::KindLocalObject:
                        {
                            assert(false);
                            break;
                        }
                    }
                }

                if(val.empty())
                {
                    val = "((" + typeToString(type, TypeModeIn, package) + ')' + arg + ')';
                }
            }
            else
            {
                val = arg;
            }
            writeMarshalUnmarshalCode(out, package, type, val, true, iter, false);
        }
        out << eb;
        out << eb;
    }
    else
    {
        out << nl << v << " = new " << concreteType << "();";
        out << nl << "int __sz" << iterS << " = __is.readSize();";
        out << nl << "for(int __i" << iterS << " = 0; __i" << iterS << " < __sz" << iterS << "; __i" << iterS << "++)";
        out << sb;
        for(i = 0; i < 2; i++)
        {
            string arg;
            TypePtr type;
            string typeS;
            if(i == 0)
            {
                arg = "__key";
                type = key;
                typeS = keyS;
            }
            else
            {
                arg = "__value";
                type = value;
                typeS = valueS;
            }

            BuiltinPtr b = BuiltinPtr::dynamicCast(type);
            if(b && java2)
            {
                switch(b->kind())
                {
                    case Builtin::KindByte:
                    {
                        out << nl << "java.lang.Byte " << arg << " = new java.lang.Byte(__is.readByte());";
                        break;
                    }
                    case Builtin::KindBool:
                    {
                        out << nl << "java.lang.Boolean " << arg << " = new java.lang.Boolean(__is.readBool());";
                        break;
                    }
                    case Builtin::KindShort:
                    {
                        out << nl << "java.lang.Short " << arg << " = new java.lang.Short(__is.readShort());";
                        break;
                    }
                    case Builtin::KindInt:
                    {
                        out << nl << "java.lang.Integer " << arg << " = new java.lang.Integer(__is.readInt());";
                        break;
                    }
                    case Builtin::KindLong:
                    {
                        out << nl << "java.lang.Long " << arg << " = new java.lang.Long(__is.readLong());";
                        break;
                    }
                    case Builtin::KindFloat:
                    {
                        out << nl << "java.lang.Float " << arg << " = new java.lang.Float(__is.readFloat());";
                        break;
                    }
                    case Builtin::KindDouble:
                    {
                        out << nl << "java.lang.Double " << arg << " = new java.lang.Double(__is.readDouble());";
                        break;
                    }
                    case Builtin::KindString:
                    {
                        out << nl << "java.lang.String " << arg << " = __is.readString();";
                        break;
                    }
                    case Builtin::KindObject:
                    {
                        assert(i == 1); // Must be the element value, since an object cannot be a key.
                        out << nl << "__is.readObject(new IceInternal.DictionaryPatcher(" << v << ", " 
                            << valueS << ".class, \"" << value->typeId() << "\", __key));";
                        break;
                    }
                    case Builtin::KindObjectProxy:
                    {
                        out << nl << "Ice.ObjectPrx " << arg << " = __is.readProxy();";
                        break;
                    }
                    case Builtin::KindLocalObject:
                    {
                        assert(false);
                        break;
                    }
                }
            }
            else
            {
                if(ClassDeclPtr::dynamicCast(type) || (b && b->kind() == Builtin::KindObject))
                {
                    writeMarshalUnmarshalCode(out, package, type, arg, false, iter, false, StringList(),
                                              "new IceInternal.DictionaryPatcher(" + v + ", " + typeS + ".class, \"" +
                                              type->typeId() + "\", __key)");
                }
                else
                {
                    out << nl << typeS << ' ' << arg << ';';
                    writeMarshalUnmarshalCode(out, package, type, arg, false, iter, false);
                }
            }
        }
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(value);
        if(!(builtin && builtin->kind() == Builtin::KindObject) && !ClassDeclPtr::dynamicCast(value))
        {
            out << nl << "" << v << ".put(__key, __value);";
        }
        out << eb;
    }
}

void
Slice::JavaGenerator::writeSequenceMarshalUnmarshalCode(Output& out,
                                                        const string& package,
                                                        const SequencePtr& seq,
                                                        const string& param,
                                                        bool marshal,
                                                        int& iter,
                                                        bool useHelper,
                                                        const StringList& metaData)
{
    string stream = marshal ? "__os" : "__is";
    string v = param;

    bool java2 = false;
    bool customType = false;
    string concreteType;

    if(_featureProfile != Slice::IceE)
    {
        java2 = seq->definitionContext()->findMetaData(_java2MetaData) == _java2MetaData;

        //
        // We have to determine whether it's possible to use the
        // type's generated helper class for this marshal/unmarshal
        // task. Since the user may have specified a custom type in
        // metadata, it's possible that the helper class is not
        // compatible and therefore we'll need to generate the code
        // in-line instead.
        //
        // Specifically, there may be "local" metadata (i.e., from
        // a data member or parameter definition) that overrides the
        // original type. We'll compare the mapped types with and
        // without local metadata to determine whether we can use
        // the helper.
        //
        string abstractType;
        customType = getSequenceTypes(seq, "", metaData, concreteType, abstractType);
        string origConcreteType, origAbstractType;
        getSequenceTypes(seq, "", StringList(), origConcreteType, origAbstractType);
        if((abstractType != origAbstractType) || (!marshal && concreteType != origConcreteType))
        {
            useHelper = false;
        }
    }

    //
    // If we can use the helper, it's easy.
    //
    if(useHelper)
    {
        string typeS = getAbsolute(seq, package);
        if(marshal)
        {
            out << nl << typeS << "Helper.write(" << stream << ", " << v << ");";
        }
        else
        {
            out << nl << v << " = " << typeS << "Helper.read(" << stream << ");";
        }
        return;
    }

    //
    // Determine sequence depth.
    //
    int depth = 0;
    TypePtr origContent = seq->type();
    SequencePtr s = SequencePtr::dynamicCast(origContent);
    while(s)
    {
        //
        // Stop if the inner sequence type has a custom type.
        //
        if(hasTypeMetaData(s) && _featureProfile != Slice::IceE)
        {
            break;
        }
        depth++;
        origContent = s->type();
        s = SequencePtr::dynamicCast(origContent);
    }
    string origContentS = typeToString(origContent, TypeModeIn, package);

    TypePtr type = seq->type();

    if(customType)
    {
        //
        // Marshal/unmarshal a custom sequence type.
        //
        BuiltinPtr b = BuiltinPtr::dynamicCast(type);
        if(b && b->kind() != Builtin::KindObject && b->kind() != Builtin::KindObjectProxy && java2)
        {
            if(marshal)
            {
                out << nl << "if(" << v << " == null)";
                out << sb;
                out << nl << stream << ".writeSize(0);";
                out << eb;
                out << nl << "else";
                out << sb;
                out << nl << stream << ".writeSize(" << v << ".size());";
                ostringstream o;
                o << "__i" << iter;
                string it = o.str();
                iter++;
                out << nl << "java.util.Iterator " << it << " = " << v << ".iterator();";
                out << nl << "while(" << it << ".hasNext())";
                out << sb;

                switch(b->kind())
                {
                    case Builtin::KindByte:
                    {
                        out << nl << "java.lang.Byte __elem = (java.lang.Byte)" << it << ".next();";
                        out << nl << stream << ".writeByte(__elem.byteValue());";
                        break;
                    }
                    case Builtin::KindBool:
                    {
                        out << nl << "java.lang.Boolean __elem = (java.lang.Boolean)" << it << ".next();";
                        out << nl << stream << ".writeBool(__elem.booleanValue());";
                        break;
                    }
                    case Builtin::KindShort:
                    {
                        out << nl << "java.lang.Short __elem = (java.lang.Short)" << it << ".next();";
                        out << nl << stream << ".writeShort(__elem.shortValue());";
                        break;
                    }
                    case Builtin::KindInt:
                    {
                        out << nl << "java.lang.Integer __elem = (java.lang.Integer)" << it << ".next();";
                        out << nl << stream << ".writeInt(__elem.intValue());";
                        break;
                    }
                    case Builtin::KindLong:
                    {
                        out << nl << "java.lang.Long __elem = (java.lang.Long)" << it << ".next();";
                        out << nl << stream << ".writeLong(__elem.longValue());";
                        break;
                    }
                    case Builtin::KindFloat:
                    {
                        out << nl << "java.lang.Float __elem = (java.lang.Float)" << it << ".next();";
                        out << nl << stream << ".writeFloat(__elem.floatValue());";
                        break;
                    }
                    case Builtin::KindDouble:
                    {
                        out << nl << "java.lang.Double __elem = (java.lang.Double)" << it << ".next();";
                        out << nl << stream << ".writeDouble(__elem.doubleValue());";
                        break;
                    }
                    case Builtin::KindString:
                    {
                        out << nl << "java.lang.String __elem = (java.lang.String)" << it << ".next();";
                        out << nl << stream << ".writeString(__elem);";
                        break;
                    }
                    case Builtin::KindObject:
                    case Builtin::KindObjectProxy:
                    case Builtin::KindLocalObject:
                    {
                        assert(false);
                        break;
                    }
                }
                out << eb; // while
                out << eb;
            }
            else
            {
                out << nl << v << " = new " << concreteType << "();";
                ostringstream o;
                o << origContentS << "[]";
                int d = depth;
                while(d--)
                {
                    o << "[]";
                }
                switch(b->kind())
                {
                    case Builtin::KindByte:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readByteSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(new java.lang.Byte(__seq" << iter << "[__i" << iter << "]));";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindBool:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readBoolSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(__seq" << iter << "[__i" << iter
                            << "] ? java.lang.Boolean.TRUE : java.lang.Boolean.FALSE);";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindShort:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readShortSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(new java.lang.Short(__seq" << iter << "[__i" << iter << "]));";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindInt:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readIntSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(new java.lang.Integer(__seq" << iter << "[__i" << iter << "]));";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindLong:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readLongSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(new java.lang.Long(__seq" << iter << "[__i" << iter << "]));";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindFloat:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readFloatSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(new java.lang.Float(__seq" << iter << "[__i" << iter << "]));";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindDouble:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readDoubleSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(new java.lang.Double(__seq" << iter << "[__i" << iter << "]));";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindString:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readStringSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(__seq" << iter << "[__i" << iter << "]);";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindObject:
                    case Builtin::KindObjectProxy:
                    case Builtin::KindLocalObject:
                    {
                        assert(false);
                        break;
                    }
                }
            }
        }
        else
        {
            string typeS = getAbsolute(seq, package);
            ostringstream o;
            o << origContentS;
            int d = depth;
            while(d--)
            {
                o << "[]";
            }
            string cont = o.str();
            if(marshal)
            {
                out << nl << "if(" << v << " == null)";
                out << sb;
                out << nl << stream << ".writeSize(0);";
                out << eb;
                out << nl << "else";
                out << sb;
                out << nl << stream << ".writeSize(" << v << ".size());";
                if(java2)
                {
                    ostringstream oit;
                    oit << "__i" << iter;
                    iter++;
                    string it = oit.str();
                    out << nl << "java.util.Iterator " << it << " = " << v << ".iterator();";
                    out << nl << "while(" << it << ".hasNext())";
                    out << sb;
                    out << nl << cont << " __elem = (" << cont << ")" << it << ".next();";
                    writeMarshalUnmarshalCode(out, package, type, "__elem", true, iter, false);
                    out << eb;
                }
                else
                {
                    string typeS = typeToString(type, TypeModeIn, package);
                    out << nl << "for(" << typeS << " __elem : " << v << ')';
                    out << sb;
                    writeMarshalUnmarshalCode(out, package, type, "__elem", true, iter, false);
                    out << eb;
                }
                out << eb; // else
            }
            else
            {
                bool isObject = false;
                ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
                if((b && b->kind() == Builtin::KindObject) || cl)
                {
                    isObject = true;
                }
                out << nl << v << " = new " << concreteType << "();";
                out << nl << "final int __len" << iter << " = " << stream << ".readSize();";
                if(type->isVariableLength())
                {
                    out << nl << stream << ".startSeq(__len" << iter << ", " << type->minWireSize() << ");";
                }
                else
                {
                    out << nl << stream << ".checkFixedSeq(__len" << iter << ", " << type->minWireSize() << ");";
                }
                if(isObject)
                {
                    if(b)
                    {
                        out << nl << "final String __type" << iter << " = Ice.ObjectImpl.ice_staticId();";
                    }
                    else
                    {
                        assert(cl);
                        if(cl->isInterface())
                        {
                            out << nl << "final String __type" << iter << " = "
                                << getAbsolute(cl, package, "_", "Disp") << ".ice_staticId();";
                        }
                        else
                        {
                            out << nl << "final String __type" << iter << " = " << origContentS << ".ice_staticId();";
                        }
                    }
                }
                out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __len" << iter << "; __i" << iter
                    << "++)";
                out << sb;
                if(isObject)
                {
                    //
                    // Add a null value to the list as a placeholder for the element.
                    //
                    out << nl << v << ".add(null);";
                    ostringstream patchParams;
                    patchParams << "new IceInternal.ListPatcher(" << v << ", " << origContentS << ".class, __type"
                                << iter << ", __i" << iter << ')';
                    writeMarshalUnmarshalCode(out, package, type, "__elem", false, iter, false, StringList(),
                                              patchParams.str());
                }
                else
                {
                    out << nl << cont << " __elem;";
                    writeMarshalUnmarshalCode(out, package, type, "__elem", false, iter, false);
                }
                if(!isObject)
                {
                    out << nl << v << ".add(__elem);";
                }

                //
                // After unmarshaling each element, check that there are still enough bytes left in the stream
                // to unmarshal the remainder of the sequence, and decrement the count of elements
                // yet to be unmarshaled for sequences with variable-length element type (that is, for sequences
                // of classes, structs, dictionaries, sequences, strings, or proxies). This allows us to
                // abort unmarshaling for bogus sequence sizes at the earliest possible moment.
                // (For fixed-length sequences, we don't need to do this because the prediction of how many
                // bytes will be taken up by the sequence is accurate.)
                //
                if(type->isVariableLength())
                {
                    if(!SequencePtr::dynamicCast(type))
                    {
                        //
                        // No need to check for directly nested sequences because, at the at start of each
                        // sequence, we check anyway.
                        //
                        out << nl << stream << ".checkSeq();";
                    }
                    out << nl << stream << ".endElement();";
                }
                out << eb;
                if(type->isVariableLength())
                {
                    out << nl << stream << ".endSeq(__len" << iter << ");";
                }
                iter++;
            }
        }
    }
    else
    {
        BuiltinPtr b = BuiltinPtr::dynamicCast(type);
        if(b && b->kind() != Builtin::KindObject && b->kind() != Builtin::KindObjectProxy)
        {
            switch(b->kind())
            {
                case Builtin::KindByte:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeByteSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readByteSeq();";
                    }
                    break;
                }
                case Builtin::KindBool:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeBoolSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readBoolSeq();";
                    }
                    break;
                }
                case Builtin::KindShort:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeShortSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readShortSeq();";
                    }
                    break;
                }
                case Builtin::KindInt:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeIntSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readIntSeq();";
                    }
                    break;
                }
                case Builtin::KindLong:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeLongSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readLongSeq();";
                    }
                    break;
                }
                case Builtin::KindFloat:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeFloatSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readFloatSeq();";
                    }
                    break;
                }
                case Builtin::KindDouble:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeDoubleSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readDoubleSeq();";
                    }
                    break;
                }
                case Builtin::KindString:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeStringSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readStringSeq();";
                    }
                    break;
                }
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                {
                    assert(false);
                    break;
                }
            }
        }
        else
        {
            if(marshal)
            {
                out << nl << "if(" << v << " == null)";
                out << sb;
                out << nl << stream << ".writeSize(0);";
                out << eb;
                out << nl << "else";
                out << sb;
                out << nl << stream << ".writeSize(" << v << ".length);";
                out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < " << v << ".length; __i" << iter
                    << "++)";
                out << sb;
                ostringstream o;
                o << v << "[__i" << iter << "]";
                iter++;
                writeMarshalUnmarshalCode(out, package, type, o.str(), true, iter, false);
                out << eb;
                out << eb;
            }
            else
            {
                bool isObject = false;
                ClassDeclPtr cl = ClassDeclPtr::dynamicCast(origContent);
                if((b && b->kind() == Builtin::KindObject) || cl)
                {
                    isObject = true;
                }
                out << nl << "final int __len" << iter << " = " << stream << ".readSize();";
                if(type->isVariableLength())
                {
                    out << nl << stream << ".startSeq(__len" << iter << ", " << type->minWireSize() << ");";
                }
                else
                {
                    out << nl << stream << ".checkFixedSeq(__len" << iter << ", " << type->minWireSize() << ");";
                }
                if(isObject)
                {
                    if(b)
                    {
                        out << nl << "final String __type" << iter << " = Ice.ObjectImpl.ice_staticId();";
                    }
                    else
                    {
                        assert(cl);
                        if(cl->isInterface())
                        {
                            out << nl << "final String __type" << iter << " = "
                                << getAbsolute(cl, package, "_", "Disp") << ".ice_staticId();";
                        }
                        else
                        {
                            out << nl << "final String __type" << iter << " = " << origContentS << ".ice_staticId();";
                        }
                    }
                }
                //
                // In Java5, we cannot allocate an array of a generic type, such as
                //
                // arr = new Map<String, String>[sz];
                //
                // Attempting to compile this code results in a "generic array creation" error
                // message. This problem can occur when the sequence's element type is a
                // dictionary, or when the element type is a nested sequence that uses a custom
                // mapping.
                //
                // The solution is to rewrite the code as follows:
                //
                // arr = (Map<String, String>[])new Map[sz];
                //
                // Unfortunately, this produces an unchecked warning during compilation.
                //
                // A simple test is to look for a "<" character in the content type, which
                // indicates the use of a generic type.
                //
                string::size_type pos = origContentS.find('<');
                if(pos != string::npos)
                {
                    string nonGenericType = origContentS.substr(0, pos);
                    out << nl << v << " = (" << origContentS << "[]";
                    int d = depth;
                    while(d--)
                    {
                        out << "[]";
                    }
                    out << ")new " << nonGenericType << "[__len" << iter << "]";
                }
                else
                {
                    out << nl << v << " = new " << origContentS << "[__len" << iter << "]";
                }
                int d = depth;
                while(d--)
                {
                    out << "[]";
                }
                out << ';';
                out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __len" << iter << "; __i" << iter
                    << "++)";
                out << sb;
                ostringstream o;
                o << v << "[__i" << iter << "]";
                ostringstream patchParams;
                if(isObject)
                {
                    patchParams << "new IceInternal.SequencePatcher(" << v << ", " << origContentS
                                << ".class, __type" << iter << ", __i" << iter << ')';
                    writeMarshalUnmarshalCode(out, package, type, o.str(), false, iter, false, StringList(),
                                              patchParams.str());
                }
                else
                {
                    writeMarshalUnmarshalCode(out, package, type, o.str(), false, iter, false);
                }

                //
                // After unmarshaling each element, check that there are still enough bytes left in the stream
                // to unmarshal the remainder of the sequence, and decrement the count of elements
                // yet to be unmarshaled for sequences with variable-length element type (that is, for sequences
                // of classes, structs, dictionaries, sequences, strings, or proxies). This allows us to
                // abort unmarshaling for bogus sequence sizes at the earliest possible moment.
                // (For fixed-length sequences, we don't need to do this because the prediction of how many
                // bytes will be taken up by the sequence is accurate.)
                //
                if(type->isVariableLength())
                {
                    if(!SequencePtr::dynamicCast(type))
                    {
                        //
                        // No need to check for directly nested sequences because, at the at start of each
                        // sequence, we check anyway.
                        //
                        out << nl << stream << ".checkSeq();";
                    }
                    out << nl << stream << ".endElement();";
                }
                out << eb;
                if(type->isVariableLength())
                {
                    out << nl << stream << ".endSeq(__len" << iter << ");";
                }
                iter++;
            }
        }
    }
}

void
Slice::JavaGenerator::writeStreamMarshalUnmarshalCode(Output& out,
                                                      const string& package,
                                                      const TypePtr& type,
                                                      const string& param,
                                                      bool marshal,
                                                      int& iter,
                                                      bool holder,
                                                      const StringList& metaData,
                                                      const string& patchParams)
{
    string stream = marshal ? "__outS" : "__inS";
    string v;
    if(holder)
    {
        v = param + ".value";
    }
    else
    {
        v = param;
    }

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindByte:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeByte(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readByte();";
                }
                break;
            }
            case Builtin::KindBool:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeBool(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readBool();";
                }
                break;
            }
            case Builtin::KindShort:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeShort(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readShort();";
                }
                break;
            }
            case Builtin::KindInt:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeInt(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readInt();";
                }
                break;
            }
            case Builtin::KindLong:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeLong(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readLong();";
                }
                break;
            }
            case Builtin::KindFloat:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeFloat(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readFloat();";
                }
                break;
            }
            case Builtin::KindDouble:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeDouble(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readDouble();";
                }
                break;
            }
            case Builtin::KindString:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeString(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readString();";
                }
                break;
            }
            case Builtin::KindObject:
            {
                if(_featureProfile != Slice::IceE)
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeObject(" << v << ");";
                    }
                    else
                    {
                        if(holder)
                        {
                            out << nl << stream << ".readObject((Ice.ReadObjectCallback)" << param << ".getPatcher());";
                        }
                        else
                        {
                            if(patchParams.empty())
                            {
                                out << nl << stream << ".readObject(new Patcher());";
                            }
                            else
                            {
                                out << nl << stream << ".readObject(" << patchParams << ");";
                            }
                        }
                    }
                }
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeProxy(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readProxy();";
                }
                break;
            }
            case Builtin::KindLocalObject:
            {
                assert(false);
                break;
            }
        }
        return;
    }

    ProxyPtr prx = ProxyPtr::dynamicCast(type);
    if(prx)
    {
        string typeS = typeToString(type, TypeModeIn, package);
        if(marshal)
        {
            out << nl << typeS << "Helper.write(" << stream << ", " << v << ");";
        }
        else
        {
            out << nl << v << " = " << typeS << "Helper.read(" << stream << ");";
        }
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(_featureProfile != Slice::IceE)
        {
            if(marshal)
            {
                out << nl << stream << ".writeObject(" << v << ");";
            }
            else
            {
                string typeS = typeToString(type, TypeModeIn, package);
                if(holder)
                {
                    out << nl << stream << ".readObject(" << param << ".getPatcher());";
                }
                else
                {
                    if(patchParams.empty())
                    {
                        out << nl << stream << ".readObject(new Patcher());";
                    }
                    else
                    {
                        out << nl << stream << ".readObject(" << patchParams << ");";
                    }
                }
            }
        }
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(marshal)
        {
            out << nl << v << ".ice_write(" << stream << ");";
        }
        else
        {
            string typeS = typeToString(type, TypeModeIn, package);
            out << nl << v << " = new " << typeS << "();";
            out << nl << v << ".ice_read(" << stream << ");";
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        if(marshal)
        {
            out << nl << v << ".ice_write(" << stream << ");";
        }
        else
        {
            string typeS = typeToString(type, TypeModeIn, package);
            out << nl << v << " = " << typeS << ".ice_read(" << stream << ");";
        }
        return;
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        writeStreamDictionaryMarshalUnmarshalCode(out, package, dict, v, marshal, iter, true, metaData);
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        writeStreamSequenceMarshalUnmarshalCode(out, package, seq, v, marshal, iter, true, metaData);
        return;
    }

    ConstructedPtr constructed = ConstructedPtr::dynamicCast(type);
    assert(constructed);
    string typeS = getAbsolute(constructed, package);
    if(marshal)
    {
        out << nl << typeS << "Helper.write(" << stream << ", " << v << ");";
    }
    else
    {
        out << nl << v << " = " << typeS << "Helper.read(" << stream << ");";
    }
}

void
Slice::JavaGenerator::writeStreamDictionaryMarshalUnmarshalCode(Output& out,
                                                                const string& package,
                                                                const DictionaryPtr& dict,
                                                                const string& param,
                                                                bool marshal,
                                                                int& iter,
                                                                bool useHelper,
                                                                const StringList& metaData)
{
    string stream = marshal ? "__outS" : "__inS";
    string v = param;

    bool java2 = dict->definitionContext()->findMetaData(_java2MetaData) == _java2MetaData;

    //
    // We have to determine whether it's possible to use the
    // type's generated helper class for this marshal/unmarshal
    // task. Since the user may have specified a custom type in
    // metadata, it's possible that the helper class is not
    // compatible and therefore we'll need to generate the code
    // in-line instead.
    //
    // Specifically, there may be "local" metadata (i.e., from
    // a data member or parameter definition) that overrides the
    // original type. We'll compare the mapped types with and
    // without local metadata to determine whether we can use
    // the helper.
    //
    string concreteType, abstractType;
    getDictionaryTypes(dict, "", metaData, concreteType, abstractType);
    string origConcreteType, origAbstractType;
    getDictionaryTypes(dict, "", StringList(), origConcreteType, origAbstractType);
    if((abstractType != origAbstractType) || (!marshal && concreteType != origConcreteType))
    {
        useHelper = false;
    }

    //
    // If we can use the helper, it's easy.
    //
    if(useHelper)
    {
        string typeS = getAbsolute(dict, package);
        if(marshal)
        {
            out << nl << typeS << "Helper.write(" << stream << ", " << v << ");";
        }
        else
        {
            out << nl << v << " = " << typeS << "Helper.read(" << stream << ");";
        }
        return;
    }

    TypePtr key = dict->keyType();
    TypePtr value = dict->valueType();

    string keyS = typeToString(key, TypeModeIn, package);
    string valueS = typeToString(value, TypeModeIn, package);
    int i;

    ostringstream o;
    o << iter;
    string iterS = o.str();
    iter++;

    if(marshal)
    {
        out << nl << "if(" << v << " == null)";
        out << sb;
        out << nl << "__outS.writeSize(0);";
        out << eb;
        out << nl << "else";
        out << sb;
        out << nl << "__outS.writeSize(" << v << ".size());";
        if(java2)
        {
            out << nl << "java.util.Iterator __i" << iterS << " = " << v << ".entrySet().iterator();";
            out << nl << "while(__i" << iterS << ".hasNext())";
            out << sb;
            out << nl << "java.util.Map.Entry __e = (java.util.Map.Entry)" << "__i" << iterS << ".next();";
        }
        else
        {
            string keyObjectS = typeToObjectString(key, TypeModeIn, package);
            string valueObjectS = typeToObjectString(value, TypeModeIn, package);
            out << nl << "for(java.util.Map.Entry<" << keyObjectS << ", " << valueObjectS << "> __e : " << v
                << ".entrySet())";
            out << sb;
        }
        for(i = 0; i < 2; i++)
        {
            string val;
            string arg;
            TypePtr type;
            if(i == 0)
            {
                arg = "__e.getKey()";
                type = key;
            }
            else
            {
                arg = "__e.getValue()";
                type = value;
            }

            //
            // We have to downcast unless we're using Java5.
            //
            if(java2)
            {
                BuiltinPtr b = BuiltinPtr::dynamicCast(type);
                if(b)
                {
                    switch(b->kind())
                    {
                        case Builtin::KindByte:
                        {
                            val = "((java.lang.Byte)" + arg + ").byteValue()";
                            break;
                        }
                        case Builtin::KindBool:
                        {
                            val = "((java.lang.Boolean)" + arg + ").booleanValue()";
                            break;
                        }
                        case Builtin::KindShort:
                        {
                            val = "((java.lang.Short)" + arg + ").shortValue()";
                            break;
                        }
                        case Builtin::KindInt:
                        {
                            val = "((java.lang.Integer)" + arg + ").intValue()";
                            break;
                        }
                        case Builtin::KindLong:
                        {
                            val = "((java.lang.Long)" + arg + ").longValue()";
                            break;
                        }
                        case Builtin::KindFloat:
                        {
                            val = "((java.lang.Float)" + arg + ").floatValue()";
                            break;
                        }
                        case Builtin::KindDouble:
                        {
                            val = "((java.lang.Double)" + arg + ").doubleValue()";
                            break;
                        }
                        case Builtin::KindString:
                        case Builtin::KindObject:
                        case Builtin::KindObjectProxy:
                        {
                            break;
                        }
                        case Builtin::KindLocalObject:
                        {
                            assert(false);
                            break;
                        }
                    }
                }

                if(val.empty())
                {
                    val = "((" + typeToString(type, TypeModeIn, package) + ')' + arg + ')';
                }
            }
            else
            {
                val = arg;
            }
            writeStreamMarshalUnmarshalCode(out, package, type, val, true, iter, false);
        }
        out << eb;
        out << eb;
    }
    else
    {
        out << nl << v << " = new " << concreteType << "();";
        out << nl << "int __sz" << iterS << " = __inS.readSize();";
        out << nl << "for(int __i" << iterS << " = 0; __i" << iterS << " < __sz" << iterS << "; __i" << iterS << "++)";
        out << sb;
        for(i = 0; i < 2; i++)
        {
            string arg;
            TypePtr type;
            string typeS;
            if(i == 0)
            {
                arg = "__key";
                type = key;
                typeS = keyS;
            }
            else
            {
                arg = "__value";
                type = value;
                typeS = valueS;
            }

            BuiltinPtr b = BuiltinPtr::dynamicCast(type);
            if(b && java2)
            {
                switch(b->kind())
                {
                    case Builtin::KindByte:
                    {
                        out << nl << "java.lang.Byte " << arg << " = new java.lang.Byte(__inS.readByte());";
                        break;
                    }
                    case Builtin::KindBool:
                    {
                        out << nl << "java.lang.Boolean " << arg << " = new java.lang.Boolean(__inS.readBool());";
                        break;
                    }
                    case Builtin::KindShort:
                    {
                        out << nl << "java.lang.Short " << arg << " = new java.lang.Short(__inS.readShort());";
                        break;
                    }
                    case Builtin::KindInt:
                    {
                        out << nl << "java.lang.Integer " << arg << " = new java.lang.Integer(__inS.readInt());";
                        break;
                    }
                    case Builtin::KindLong:
                    {
                        out << nl << "java.lang.Long " << arg << " = new java.lang.Long(__inS.readLong());";
                        break;
                    }
                    case Builtin::KindFloat:
                    {
                        out << nl << "java.lang.Float " << arg << " = new java.lang.Float(__inS.readFloat());";
                        break;
                    }
                    case Builtin::KindDouble:
                    {
                        out << nl << "java.lang.Double " << arg << " = new java.lang.Double(__inS.readDouble());";
                        break;
                    }
                    case Builtin::KindString:
                    {
                        out << nl << "java.lang.String " << arg << " = __inS.readString();";
                        break;
                    }
                    case Builtin::KindObject:
                    {
                        out << nl << "__inS.readObject(new IceInternal.DictionaryPatcher(" << v << ", "
                            << valueS << ".class, \"" << value->typeId() << "\", __key));";
                        break;
                    }
                    case Builtin::KindObjectProxy:
                    {
                        out << nl << "Ice.ObjectPrx " << arg << " = __inS.readProxy();";
                        break;
                    }
                    case Builtin::KindLocalObject:
                    {
                        assert(false);
                        break;
                    }
                }
            }
            else
            {
                string s = typeToString(type, TypeModeIn, package);
                if(ClassDeclPtr::dynamicCast(type) || (b && b->kind() == Builtin::KindObject))
                {
                    writeStreamMarshalUnmarshalCode(out, package, type, arg, false, iter, false, StringList(),
                                                    "new IceInternal.DictionaryPatcher(" + v + ", " + s +
                                                    ".class, \"" + type->typeId() + "\", __key)");
                }
                else
                {
                    out << nl << s << ' ' << arg << ';';
                    writeStreamMarshalUnmarshalCode(out, package, type, arg, false, iter, false);
                }
            }
        }
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(value);
        if(!(builtin && builtin->kind() == Builtin::KindObject) && !ClassDeclPtr::dynamicCast(value))
        {
            out << nl << "" << v << ".put(__key, __value);";
        }
        out << eb;
    }
}

void
Slice::JavaGenerator::writeStreamSequenceMarshalUnmarshalCode(Output& out,
                                                              const string& package,
                                                              const SequencePtr& seq,
                                                              const string& param,
                                                              bool marshal,
                                                              int& iter,
                                                              bool useHelper,
                                                              const StringList& metaData)
{
    string stream = marshal ? "__outS" : "__inS";
    string v = param;

    bool java2 = seq->definitionContext()->findMetaData(_java2MetaData) == _java2MetaData;

    //
    // We have to determine whether it's possible to use the
    // type's generated helper class for this marshal/unmarshal
    // task. Since the user may have specified a custom type in
    // metadata, it's possible that the helper class is not
    // compatible and therefore we'll need to generate the code
    // in-line instead.
    //
    // Specifically, there may be "local" metadata (i.e., from
    // a data member or parameter definition) that overrides the
    // original type. We'll compare the mapped types with and
    // without local metadata to determine whether we can use
    // the helper.
    //
    string concreteType, abstractType;
    bool customType = getSequenceTypes(seq, "", metaData, concreteType, abstractType);
    string origConcreteType, origAbstractType;
    getSequenceTypes(seq, "", StringList(), origConcreteType, origAbstractType);
    if((abstractType != origAbstractType) || (!marshal && concreteType != origConcreteType))
    {
        useHelper = false;
    }

    //
    // If we can use the helper, it's easy.
    //
    if(useHelper)
    {
        string typeS = getAbsolute(seq, package);
        if(marshal)
        {
            out << nl << typeS << "Helper.write(" << stream << ", " << v << ");";
        }
        else
        {
            out << nl << v << " = " << typeS << "Helper.read(" << stream << ");";
        }
        return;
    }

    //
    // Determine sequence depth
    //
    int depth = 0;
    TypePtr origContent = seq->type();
    SequencePtr s = SequencePtr::dynamicCast(origContent);
    while(s)
    {
        //
        // Stop if the inner sequence type has a custom type.
        //
        if(hasTypeMetaData(s))
        {
            break;
        }
        depth++;
        origContent = s->type();
        s = SequencePtr::dynamicCast(origContent);
    }
    string origContentS = typeToString(origContent, TypeModeIn, package);

    TypePtr type = seq->type();

    if(customType)
    {
        //
        // Marshal/unmarshal a custom sequence type.
        //
        BuiltinPtr b = BuiltinPtr::dynamicCast(type);
        if(b && b->kind() != Builtin::KindObject && b->kind() != Builtin::KindObjectProxy && java2)
        {
            if(marshal)
            {
                out << nl << "if(" << v << " == null)";
                out << sb;
                out << nl << stream << ".writeSize(0);";
                out << eb;
                out << nl << "else";
                out << sb;
                out << nl << stream << ".writeSize(" << v << ".size());";
                ostringstream o;
                o << "__i" << iter;
                string it = o.str();
                iter++;
                out << nl << "java.util.Iterator " << it << " = " << v << ".iterator();";
                out << nl << "while(" << it << ".hasNext())";
                out << sb;

                switch(b->kind())
                {
                    case Builtin::KindByte:
                    {
                        out << nl << "java.lang.Byte __elem = (java.lang.Byte)" << it << ".next();";
                        out << nl << stream << ".writeByte(__elem.byteValue());";
                        break;
                    }
                    case Builtin::KindBool:
                    {
                        out << nl << "java.lang.Boolean __elem = (java.lang.Boolean)" << it << ".next();";
                        out << nl << stream << ".writeBool(__elem.booleanValue());";
                        break;
                    }
                    case Builtin::KindShort:
                    {
                        out << nl << "java.lang.Short __elem = (java.lang.Short)" << it << ".next();";
                        out << nl << stream << ".writeShort(__elem.shortValue());";
                        break;
                    }
                    case Builtin::KindInt:
                    {
                        out << nl << "java.lang.Integer __elem = (java.lang.Integer)" << it << ".next();";
                        out << nl << stream << ".writeInt(__elem.intValue());";
                        break;
                    }
                    case Builtin::KindLong:
                    {
                        out << nl << "java.lang.Long __elem = (java.lang.Long)" << it << ".next();";
                        out << nl << stream << ".writeLong(__elem.longValue());";
                        break;
                    }
                    case Builtin::KindFloat:
                    {
                        out << nl << "java.lang.Float __elem = (java.lang.Float)" << it << ".next();";
                        out << nl << stream << ".writeFloat(__elem.floatValue());";
                        break;
                    }
                    case Builtin::KindDouble:
                    {
                        out << nl << "java.lang.Double __elem = (java.lang.Double)" << it << ".next();";
                        out << nl << stream << ".writeDouble(__elem.doubleValue());";
                        break;
                    }
                    case Builtin::KindString:
                    {
                        out << nl << "java.lang.String __elem = (java.lang.String)" << it << ".next();";
                        out << nl << stream << ".writeString(__elem);";
                        break;
                    }
                    case Builtin::KindObject:
                    case Builtin::KindObjectProxy:
                    case Builtin::KindLocalObject:
                    {
                        assert(false);
                        break;
                    }
                }
                out << eb; // while
                out << eb;
            }
            else
            {
                out << nl << v << " = new " << concreteType << "();";
                ostringstream o;
                o << origContentS << "[]";
                int d = depth;
                while(d--)
                {
                    o << "[]";
                }
                switch(b->kind())
                {
                    case Builtin::KindByte:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readByteSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(new java.lang.Byte(__seq" << iter << "[__i" << iter << "]));";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindBool:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readBoolSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(__seq" << iter << "[__i" << iter
                            << "] ? java.lang.Boolean.TRUE : java.lang.Boolean.FALSE);";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindShort:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readShortSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(new java.lang.Short(__seq" << iter << "[__i" << iter << "]));";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindInt:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readIntSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(new java.lang.Integer(__seq" << iter << "[__i" << iter << "]));";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindLong:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readLongSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(new java.lang.Long(__seq" << iter << "[__i" << iter << "]));";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindFloat:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readFloatSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(new java.lang.Float(__seq" << iter << "[__i" << iter << "]));";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindDouble:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readDoubleSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(new java.lang.Double(__seq" << iter << "[__i" << iter << "]));";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindString:
                    {
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readStringSeq();";
                        out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __seq" << iter
                            << ".length; __i" << iter << "++)";
                        out << sb;
                        out << nl << v << ".add(__seq" << iter << "[__i" << iter << "]);";
                        out << eb;
                        iter++;
                        break;
                    }
                    case Builtin::KindObject:
                    case Builtin::KindObjectProxy:
                    case Builtin::KindLocalObject:
                    {
                        assert(false);
                        break;
                    }
                }
            }
        }
        else
        {
            string typeS = getAbsolute(seq, package);
            ostringstream o;
            o << origContentS;
            int d = depth;
            while(d--)
            {
                o << "[]";
            }
            string cont = o.str();
            if(marshal)
            {
                out << nl << "if(" << v << " == null)";
                out << sb;
                out << nl << stream << ".writeSize(0);";
                out << eb;
                out << nl << "else";
                out << sb;
                out << nl << stream << ".writeSize(" << v << ".size());";
                if(java2)
                {
                    ostringstream oit;
                    oit << "__i" << iter;
                    iter++;
                    string it = oit.str();
                    out << nl << "java.util.Iterator " << it << " = " << v << ".iterator();";
                    out << nl << "while(" << it << ".hasNext())";
                    out << sb;
                    out << nl << cont << " __elem = (" << cont << ")" << it << ".next();";
                    writeStreamMarshalUnmarshalCode(out, package, type, "__elem", true, iter, false);
                    out << eb;
                }
                else
                {
                    string typeS = typeToString(type, TypeModeIn, package);
                    out << nl << "for(" << typeS << " __elem : " << v << ')';
                    out << sb;
                    writeStreamMarshalUnmarshalCode(out, package, type, "__elem", true, iter, false);
                    out << eb;
                }
                out << eb; // else
            }
            else
            {
                bool isObject = false;
                ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
                if((b && b->kind() == Builtin::KindObject) || cl)
                {
                    isObject = true;
                }
                out << nl << v << " = new " << concreteType << "();";
                out << nl << "final int __len" << iter << " = " << stream << ".readSize();";
                if(isObject)
                {
                    if(b)
                    {
                        out << nl << "final String __type" << iter << " = Ice.ObjectImpl.ice_staticId();";
                    }
                    else
                    {
                        assert(cl);
                        if(cl->isInterface())
                        {
                            out << nl << "final String __type" << iter << " = "
                                << getAbsolute(cl, package, "_", "Disp") << ".ice_staticId();";
                        }
                        else
                        {
                            out << nl << "final String __type" << iter << " = " << origContentS << ".ice_staticId();";
                        }
                    }
                }
                out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __len" << iter << "; __i" << iter
                    << "++)";
                out << sb;
                if(isObject)
                {
                    //
                    // Add a null value to the list as a placeholder for the element.
                    //
                    out << nl << v << ".add(null);";
                    ostringstream patchParams;
                    patchParams << "new IceInternal.ListPatcher(" << v << ", " << origContentS << ".class, __type"
                                << iter << ", __i" << iter << ')';
                    writeStreamMarshalUnmarshalCode(out, package, type, "__elem", false, iter, false,
                                                    StringList(), patchParams.str());
                }
                else
                {
                    out << nl << cont << " __elem;";
                    writeStreamMarshalUnmarshalCode(out, package, type, "__elem", false, iter, false);
                }
                if(!isObject)
                {
                    out << nl << v << ".add(__elem);";
                }
                out << eb;
                iter++;
            }
        }
    }
    else
    {
        BuiltinPtr b = BuiltinPtr::dynamicCast(type);
        if(b && b->kind() != Builtin::KindObject && b->kind() != Builtin::KindObjectProxy)
        {
            switch(b->kind())
            {
                case Builtin::KindByte:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeByteSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readByteSeq();";
                    }
                    break;
                }
                case Builtin::KindBool:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeBoolSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readBoolSeq();";
                    }
                    break;
                }
                case Builtin::KindShort:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeShortSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readShortSeq();";
                    }
                    break;
                }
                case Builtin::KindInt:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeIntSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readIntSeq();";
                    }
                    break;
                }
                case Builtin::KindLong:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeLongSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readLongSeq();";
                    }
                    break;
                }
                case Builtin::KindFloat:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeFloatSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readFloatSeq();";
                    }
                    break;
                }
                case Builtin::KindDouble:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeDoubleSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readDoubleSeq();";
                    }
                    break;
                }
                case Builtin::KindString:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeStringSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readStringSeq();";
                    }
                    break;
                }
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                {
                    assert(false);
                    break;
                }
            }
        }
        else
        {
            if(marshal)
            {
                out << nl << "if(" << v << " == null)";
                out << sb;
                out << nl << stream << ".writeSize(0);";
                out << eb;
                out << nl << "else";
                out << sb;
                out << nl << stream << ".writeSize(" << v << ".length);";
                out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < " << v << ".length; __i" << iter
                    << "++)";
                out << sb;
                ostringstream o;
                o << v << "[__i" << iter << "]";
                iter++;
                writeStreamMarshalUnmarshalCode(out, package, type, o.str(), true, iter, false);
                out << eb;
                out << eb;
            }
            else
            {
                bool isObject = false;
                ClassDeclPtr cl = ClassDeclPtr::dynamicCast(origContent);
                if((b && b->kind() == Builtin::KindObject) || cl)
                {
                    isObject = true;
                }
                out << nl << "final int __len" << iter << " = " << stream << ".readSize();";
                if(isObject)
                {
                    if(b)
                    {
                        out << nl << "final String __type" << iter << " = Ice.ObjectImpl.ice_staticId();";
                    }
                    else
                    {
                        assert(cl);
                        if(cl->isInterface())
                        {
                            out << nl << "final String __type" << iter << " = "
                                << getAbsolute(cl, package, "_", "Disp") << ".ice_staticId();";
                        }
                        else
                        {
                            out << nl << "final String __type" << iter << " = " << origContentS << ".ice_staticId();";
                        }
                    }
                }
                //
                // In Java5, we cannot allocate an array of a generic type, such as
                //
                // arr = new Map<String, String>[sz];
                //
                // Attempting to compile this code results in a "generic array creation" error
                // message. This problem can occur when the sequence's element type is a
                // dictionary, or when the element type is a nested sequence that uses a custom
                // mapping.
                //
                // The solution is to rewrite the code as follows:
                //
                // arr = (Map<String, String>[])new Map[sz];
                //
                // Unfortunately, this produces an unchecked warning during compilation.
                //
                // A simple test is to look for a "<" character in the content type, which
                // indicates the use of a generic type.
                //
                string::size_type pos = origContentS.find('<');
                if(pos != string::npos)
                {
                    string nonGenericType = origContentS.substr(0, pos);
                    out << nl << v << " = (" << origContentS << "[]";
                    int d = depth;
                    while(d--)
                    {
                        out << "[]";
                    }
                    out << ")new " << nonGenericType << "[__len" << iter << "]";
                }
                else
                {
                    out << nl << v << " = new " << origContentS << "[__len" << iter << "]";
                }
                int d = depth;
                while(d--)
                {
                    out << "[]";
                }
                out << ';';
                out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __len" << iter << "; __i" << iter
                    << "++)";
                out << sb;
                ostringstream o;
                o << v << "[__i" << iter << "]";
                ostringstream patchParams;
                if(isObject)
                {
                    patchParams << "new IceInternal.SequencePatcher(" << v << ", " << origContentS
                                << ".class, __type" << iter << ", __i" << iter << ')';
                    writeStreamMarshalUnmarshalCode(out, package, type, o.str(), false, iter, false,
                                                    StringList(), patchParams.str());
                }
                else
                {
                    writeStreamMarshalUnmarshalCode(out, package, type, o.str(), false, iter, false);
                }
                out << eb;
                iter++;
            }
        }
    }
}

bool
Slice::JavaGenerator::getTypeMetaData(const StringList& metaData, string& concreteType, string& abstractType)
{
    //
    // Extract the concrete type and an optional abstract type.
    // The correct syntax is "java:type:concrete-type[:abstract-type]".
    //
    static const string prefix = "java:type:";
    for(StringList::const_iterator q = metaData.begin(); q != metaData.end(); ++q)
    {
        string str = *q;
        if(str.find(prefix) == 0)
        {
            string::size_type pos = str.find(':', prefix.size());
            if(pos != string::npos)
            {
                concreteType = str.substr(prefix.size(), pos - prefix.size());
                abstractType = str.substr(pos + 1);
            }
            else
            {
                concreteType = str.substr(prefix.size());
                abstractType.clear();
            }
            return true;
        }
    }

    return false;
}

bool
Slice::JavaGenerator::hasTypeMetaData(const TypePtr& type, const StringList& localMetaData)
{
    ContainedPtr cont = ContainedPtr::dynamicCast(type);
    if(cont)
    {
        static const string prefix = "java:type:";

        StringList::const_iterator q;
        for(q = localMetaData.begin(); q != localMetaData.end(); ++q)
        {
            string str = *q;
            if(str.find(prefix) == 0)
            {
                return true;
            }
        }

        StringList metaData = cont->getMetaData();
        for(q = metaData.begin(); q != metaData.end(); ++q)
        {
            string str = *q;
            if(str.find(prefix) == 0)
            {
                return true;
            }
        }
    }

    return false;
}

bool
Slice::JavaGenerator::getDictionaryTypes(const DictionaryPtr& dict,
                                         const string& package,
                                         const StringList& metaData,
                                         string& concreteType,
                                         string& abstractType) const
{
    bool customType = false;

    if(_featureProfile == Slice::IceE)
    {
        concreteType = abstractType = "java.util.Hashtable";
        return customType;
    }

    bool java2 = dict->definitionContext()->findMetaData(_java2MetaData) == _java2MetaData;

    //
    // Collect metadata for a custom type.
    //
    string ct, at;
    customType = getTypeMetaData(metaData, ct, at);
    if(!customType)
    {
        customType = getTypeMetaData(dict->getMetaData(), ct, at);
    }

    //
    // Get the types of the key and value.
    //
    string keyTypeStr;
    string valueTypeStr;
    if(!java2)
    {
        keyTypeStr = typeToObjectString(dict->keyType(), TypeModeIn, package);
        valueTypeStr = typeToObjectString(dict->valueType(), TypeModeIn, package);
    }

    //
    // Handle a custom type.
    //
    if(customType)
    {
        assert(!ct.empty());

        //
        // Check for portable syntax. Convert {type} to type<key, value> for Java5.
        //
        if(ct[0] == '{')
        {
            string::size_type pos = ct.find('}');
            if(pos != string::npos)
            {
                concreteType = ct.substr(1, pos - 1);
                if(!java2)
                {
                    concreteType += "<" + keyTypeStr + ", " + valueTypeStr + ">";
                }
            }
        }
        else
        {
            concreteType = ct;
        }

        if(!at.empty())
        {
            if(at[0] == '{')
            {
                string::size_type pos = at.find('}');
                if(pos != string::npos)
                {
                    abstractType = at.substr(1, pos - 1);
                    if(!java2)
                    {
                        abstractType += "<" + keyTypeStr + ", " + valueTypeStr + ">";
                    }
                }
            }
            else
            {
                abstractType = at;
            }
        }
    }

    //
    // Return a default type for the platform.
    //
    if(concreteType.empty())
    {
        concreteType = "java.util.HashMap";
        if(!java2)
        {
            concreteType += "<" + keyTypeStr + ", " + valueTypeStr + ">";
        }
    }

    //
    // If an abstract type is not defined, we use the concrete type as the default.
    // If instead we chose a default abstract type, such as Map<K, V>, then we
    // might inadvertently generate uncompilable code. The Java5 compiler does not
    // allow polymorphic assignment between generic types if it can weaken the
    // compile-time type safety rules.
    //
    if(abstractType.empty())
    {
        abstractType = "java.util.Map";
        if(!java2)
        {
            abstractType += "<" + keyTypeStr + ", " + valueTypeStr + ">";
        }
    }

    return customType;
}

bool
Slice::JavaGenerator::getSequenceTypes(const SequencePtr& seq,
                                       const string& package,
                                       const StringList& metaData,
                                       string& concreteType,
                                       string& abstractType) const
{
    bool customType = false;

    if(_featureProfile == Slice::IceE)
    {
        concreteType = abstractType = typeToString(seq->type(), TypeModeIn, package) + "[]";
        return customType;
    }

    bool java2 = seq->definitionContext()->findMetaData(_java2MetaData) == _java2MetaData;

    //
    // Collect metadata for a custom type.
    //
    string ct, at;
    customType = getTypeMetaData(metaData, ct, at);
    if(!customType)
    {
        customType = getTypeMetaData(seq->getMetaData(), ct, at);
    }

    //
    // Get the inner type.
    //
    string typeStr;
    if(!java2)
    {
        typeStr = typeToObjectString(seq->type(), TypeModeIn, package);
    }

    //
    // Handle a custom type.
    //
    if(customType)
    {
        assert(!ct.empty());

        //
        // Check for portable syntax. Convert {type} to type<key, value> for Java5.
        //
        if(ct[0] == '{')
        {
            string::size_type pos = ct.find('}');
            if(pos != string::npos)
            {
                concreteType = ct.substr(1, pos - 1);
                if(!java2)
                {
                    concreteType += "<" + typeStr + ">";
                }
            }
        }
        else
        {
            concreteType = ct;
        }

        if(!at.empty())
        {
            if(at[0] == '{')
            {
                string::size_type pos = at.find('}');
                if(pos != string::npos)
                {
                    abstractType = at.substr(1, pos - 1);
                    if(!java2)
                    {
                        abstractType += "<" + typeStr + ">";
                    }
                }
            }
            else
            {
                abstractType = at;
            }
        }
        else
        {
            //
            // If an abstract type is not defined, we use the concrete type as the default.
            // If instead we chose a default abstract type, such as List<T>, then we
            // might inadvertently generate uncompilable code. The Java5 compiler does not
            // allow polymorphic assignment between generic types if it can weaken the
            // compile-time type safety rules.
            //
            abstractType = "java.util.List";
            if(!java2)
            {
                abstractType += "<" + typeStr + ">";
            }
        }
    }

    //
    // The default mapping is a native array.
    //
    if(concreteType.empty())
    {
        concreteType = abstractType = typeToString(seq->type(), TypeModeIn, package) + "[]";
    }

    return customType;
}

JavaOutput*
Slice::JavaGenerator::createOutput()
{
    return new JavaOutput;
}

void
Slice::JavaGenerator::validateMetaData(const UnitPtr& u)
{
    MetaDataVisitor visitor;
    u->visit(&visitor, true);
}

bool
Slice::JavaGenerator::MetaDataVisitor::visitModuleStart(const ModulePtr& p)
{
    //
    // Validate global metadata.
    //
    DefinitionContextPtr dc = p->definitionContext();
    assert(dc);
    StringList globalMetaData = dc->getMetaData();
    string file = dc->filename();
    static const string prefix = "java:";
    for(StringList::const_iterator q = globalMetaData.begin(); q != globalMetaData.end(); ++q)
    {
        string s = *q;
        if(_history.count(s) == 0)
        {
            if(s.find(prefix) == 0)
            {
                bool ok = false;

                static const string packagePrefix = "java:package:";
                if(s.find(packagePrefix) == 0 && s.size() > packagePrefix.size())
                {
                    ok = true;
                }
                else if(s == _java2MetaData)
                {
                    ok = true;
                }
                else if(s == _java5MetaData)
                {
                    ok = true;
                }

                if(!ok)
                {
                    cout << file << ": warning: ignoring invalid global metadata `" << s << "'" << endl;
                }
            }
            _history.insert(s);
        }
    }

    StringList metaData = getMetaData(p);
    validateType(p, metaData, p->definitionContext()->filename(), p->line());
    validateGetSet(p, metaData, p->definitionContext()->filename(), p->line());
    return true;
}

void
Slice::JavaGenerator::MetaDataVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    StringList metaData = getMetaData(p);
    validateType(p, metaData, p->definitionContext()->filename(), p->line());
    validateGetSet(p, metaData, p->definitionContext()->filename(), p->line());
}

bool
Slice::JavaGenerator::MetaDataVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    StringList metaData = getMetaData(p);
    validateType(p, metaData, p->definitionContext()->filename(), p->line());
    validateGetSet(p, metaData, p->definitionContext()->filename(), p->line());
    return true;
}

bool
Slice::JavaGenerator::MetaDataVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    StringList metaData = getMetaData(p);
    validateType(p, metaData, p->definitionContext()->filename(), p->line());
    validateGetSet(p, metaData, p->definitionContext()->filename(), p->line());
    return true;
}

bool
Slice::JavaGenerator::MetaDataVisitor::visitStructStart(const StructPtr& p)
{
    StringList metaData = getMetaData(p);
    validateType(p, metaData, p->definitionContext()->filename(), p->line());
    validateGetSet(p, metaData, p->definitionContext()->filename(), p->line());
    return true;
}

void
Slice::JavaGenerator::MetaDataVisitor::visitOperation(const OperationPtr& p)
{
    StringList metaData = getMetaData(p);
    TypePtr returnType = p->returnType();
    if(!metaData.empty())
    {
        if(!returnType)
        {
            for(StringList::const_iterator q = metaData.begin(); q != metaData.end(); ++q)
            {
                if(q->find("java:type:", 0) == 0)
                {
                    cout << p->definitionContext()->filename() << ":" << p->line()
                         << ": warning: invalid metadata for operation" << endl;
                    break;
                }
            }
        }
        else
        {
            validateType(returnType, metaData, p->definitionContext()->filename(), p->line());
        }
    }

    ParamDeclList params = p->parameters();
    for(ParamDeclList::iterator q = params.begin(); q != params.end(); ++q)
    {
        metaData = getMetaData(*q);
        validateType((*q)->type(), metaData, p->definitionContext()->filename(), (*q)->line());
    }

    validateGetSet(p, metaData, p->definitionContext()->filename(), p->line());
}

void
Slice::JavaGenerator::MetaDataVisitor::visitDataMember(const DataMemberPtr& p)
{
    StringList metaData = getMetaData(p);
    validateType(p->type(), metaData, p->definitionContext()->filename(), p->line());
    validateGetSet(p, metaData, p->definitionContext()->filename(), p->line());
}

void
Slice::JavaGenerator::MetaDataVisitor::visitSequence(const SequencePtr& p)
{
    StringList metaData = getMetaData(p);
    validateType(p, metaData, p->definitionContext()->filename(), p->line());
    validateGetSet(p, metaData, p->definitionContext()->filename(), p->line());
}

void
Slice::JavaGenerator::MetaDataVisitor::visitDictionary(const DictionaryPtr& p)
{
    StringList metaData = getMetaData(p);
    validateType(p, metaData, p->definitionContext()->filename(), p->line());
    validateGetSet(p, metaData, p->definitionContext()->filename(), p->line());
}

void
Slice::JavaGenerator::MetaDataVisitor::visitEnum(const EnumPtr& p)
{
    StringList metaData = getMetaData(p);
    validateType(p, metaData, p->definitionContext()->filename(), p->line());
    validateGetSet(p, metaData, p->definitionContext()->filename(), p->line());
}

void
Slice::JavaGenerator::MetaDataVisitor::visitConst(const ConstPtr& p)
{
    StringList metaData = getMetaData(p);
    validateType(p, metaData, p->definitionContext()->filename(), p->line());
    validateGetSet(p, metaData, p->definitionContext()->filename(), p->line());
}

StringList
Slice::JavaGenerator::MetaDataVisitor::getMetaData(const ContainedPtr& cont)
{
    StringList metaData = cont->getMetaData();
    DefinitionContextPtr dc = cont->definitionContext();
    assert(dc);
    string file = dc->filename();

    StringList result;
    static const string prefix = "java:";

    for(StringList::const_iterator p = metaData.begin(); p != metaData.end(); ++p)
    {
        string s = *p;
        if(_history.count(s) == 0) // Don't complain about the same metadata more than once.
        {
            if(s.find(prefix) == 0)
            {
                string::size_type pos = s.find(':', prefix.size());
                if(pos == string::npos)
                {
                    if(s.size() > prefix.size())
                    {
                        string rest = s.substr(prefix.size());
                        if(rest == "getset")
                        {
                            result.push_back(s);
                        }
                        continue;
                    }
                }
                else if(s.substr(prefix.size(), pos - prefix.size()) == "type")
                {
                    result.push_back(s);
                    continue;
                }

                cout << file << ":" << cont->line() << ": warning: ignoring invalid metadata `" << s << "'" << endl;
            }

            _history.insert(s);
        }
    }

    return result;
}

void
Slice::JavaGenerator::MetaDataVisitor::validateType(const SyntaxTreeBasePtr& p, const StringList& metaData,
                                                    const string& file, const string& line)
{
    for(StringList::const_iterator i = metaData.begin(); i != metaData.end(); ++i)
    {
        //
        // Type metadata ("java:type:Foo") is only supported by sequences and dictionaries.
        //
        if(i->find("java:type:", 0) == 0 && (!SequencePtr::dynamicCast(p) && !DictionaryPtr::dynamicCast(p)))
        {
            string str;
            ContainedPtr cont = ContainedPtr::dynamicCast(p);
            if(cont)
            {
                str = cont->kindOf();
            }
            else
            {
                BuiltinPtr b = BuiltinPtr::dynamicCast(p);
                assert(b);
                str = b->typeId();
            }
            cout << file << ":" << line << ": warning: invalid metadata for " << str << endl;
        }
    }
}

void
Slice::JavaGenerator::MetaDataVisitor::validateGetSet(const SyntaxTreeBasePtr& p, const StringList& metaData,
                                                      const string& file, const string& line)
{
    for(StringList::const_iterator i = metaData.begin(); i != metaData.end(); ++i)
    {
        //
        // The "getset" metadata can only be specified on a class, struct, exception or data member.
        //
        if((*i) == "java:getset" &&
           (!ClassDefPtr::dynamicCast(p) && !StructPtr::dynamicCast(p) && !ExceptionPtr::dynamicCast(p) &&
            !DataMemberPtr::dynamicCast(p)))
        {
            string str;
            ContainedPtr cont = ContainedPtr::dynamicCast(p);
            if(cont)
            {
                str = cont->kindOf();
            }
            else
            {
                BuiltinPtr b = BuiltinPtr::dynamicCast(p);
                assert(b);
                str = b->typeId();
            }
            cout << file << ":" << line << ": warning: invalid metadata for " << str << endl;
        }
    }
}
