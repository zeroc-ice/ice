// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Slice/JavaUtil.h>
#include <Slice/FileTracker.h>
#include <Slice/Util.h>
#include <Slice/MD5.h>
#include <IceUtil/Functional.h>
#include <IceUtil/FileUtil.h>

#include <sys/types.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#endif

#ifndef _WIN32
#include <unistd.h>
#endif

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

namespace
{

void
hashAdd(long& hashCode, const std::string& value)
{
    for(std::string::const_iterator p = value.begin(); p != value.end(); ++p)
    {
        hashCode = ((hashCode << 5) + hashCode) ^ *p;
    }
}

string
typeToBufferString(const TypePtr& type)
{
    static const char* builtinBufferTable[] =
    {
        "java.nio.ByteBuffer",
        "???",
        "java.nio.ShortBuffer",
        "java.nio.IntBuffer",
        "java.nio.LongBuffer",
        "java.nio.FloatBuffer",
        "java.nio.DoubleBuffer",
        "???",
        "???",
        "???",
        "???"
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(!builtin)
    {
        return "???";
    }
    else
    {
        return builtinBufferTable[builtin->kind()];
    }
}
}

long
Slice::computeSerialVersionUUID(const ClassDefPtr& p)
{
    ostringstream os;

    ClassList bases = p->bases();
    os << "Name: " << p->scoped();

    os << " Bases: [";
    for(ClassList::const_iterator i = bases.begin(); i != bases.end();)
    {
        os << (*i)->scoped();
        i++;
        if(i != bases.end())
        {
            os << ", ";
        }
    }
    os << "]";

    os << " Members: [";
    DataMemberList members = p->dataMembers();
    for(DataMemberList::const_iterator i = members.begin(); i != members.end();)
    {
        os << (*i)->name() << ":" << (*i)->type();
        i++;
        if(i != members.end())
        {
            os << ", ";
        }
    }
    os << "]";

    const string data = os.str();
    long hashCode = 5381;
    hashAdd(hashCode, data);
    return hashCode;
}

long
Slice::computeSerialVersionUUID(const StructPtr& p)
{
    ostringstream os;

    os << "Name: " << p->scoped();
    os << " Members: [";
    DataMemberList members = p->dataMembers();
    for(DataMemberList::const_iterator i = members.begin(); i != members.end();)
    {
        os << (*i)->name() << ":" << (*i)->type();
        i++;
        if(i != members.end())
        {
            os << ", ";
        }
    }
    os << "]";

    const string data = os.str();
    long hashCode = 5381;
    hashAdd(hashCode, data);
    return hashCode;
}

long
Slice::computeSerialVersionUUID(const ExceptionPtr& p)
{
    ostringstream os;

    os << "Name: " << p->scoped();
    os << " Members: [";
    DataMemberList members = p->dataMembers();
    for(DataMemberList::const_iterator i = members.begin(); i != members.end();)
    {
        os << (*i)->name() << ":" << (*i)->type();
        i++;
        if(i != members.end())
        {
            os << ", ";
        }
    }
    os << "]";

    const string data = os.str();
    long hashCode = 5381;
    hashAdd(hashCode, data);
    return hashCode;
}



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

void
Slice::JavaOutput::openClass(const string& cls, const string& prefix, const string& sliceFile)
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

            IceUtilInternal::structstat st;
            if(!IceUtilInternal::stat(path, &st))
            {
                if(!(st.st_mode & S_IFDIR))
                {
                    ostringstream os;
                    os << "failed to create package directory `" << path
                       << "': file already exists and is not a directory";
                    throw FileException(__FILE__, __LINE__, os.str());
                }
                continue;
            }

            if(IceUtilInternal::mkdir(path, 0777) != 0)
            {
                ostringstream os;
                os << "cannot create directory `" << path << "': " << strerror(errno);
                throw FileException(__FILE__, __LINE__, os.str());
            }
            FileTracker::instance()->addDirectory(path);
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
        FileTracker::instance()->addFile(path);
        printHeader();
        printGeneratedHeader(*this, sliceFile);
        if(!package.empty())
        {
            separator();
            print("package ");
            print(package.c_str());
            print(";");
        }
    }
    else
    {
        ostringstream os;
        os << "cannot open file `" << path << "': " << strerror(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }
}

void
Slice::JavaOutput::printHeader()
{
    static const char* header =
"// **********************************************************************\n"
"//\n"
"// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.\n"
"//\n"
"// This copy of Ice is licensed to you under the terms described in the\n"
"// ICE_LICENSE file included in this distribution.\n"
"//\n"
"// **********************************************************************\n"
        ;

    print(header);
    print("//\n");
    print("// Ice version ");
    print(ICE_STRING_VERSION);
    print("\n");
    print("//\n");
}

const string Slice::JavaGenerator::_getSetMetaData = "java:getset";

Slice::JavaGenerator::JavaGenerator(const string& dir) :
    _dir(dir),
    _out(0)
{
}

Slice::JavaGenerator::~JavaGenerator()
{
    // If open throws an exception other generators could be left open
    // during the stack unwind.
    if(_out != 0)
    {
        close();
    }
    assert(_out == 0);
}

void
Slice::JavaGenerator::open(const string& absolute, const string& file)
{
    assert(_out == 0);

    JavaOutput* out = createOutput();
    try
    {
        out->openClass(absolute, _dir, file);
    }
    catch(const FileException&)
    {
        delete out;
        throw;
    }
    _out = out;
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
Slice::JavaGenerator::getPackagePrefix(const ContainedPtr& cont) const
{
    UnitPtr unit = cont->container()->unit();
    string file = cont->file();
    assert(!file.empty());

    map<string, string>::const_iterator p = _filePackagePrefix.find(file);
    if(p != _filePackagePrefix.end())
    {
        return p->second;
    }

    static const string prefix = "java:package:";
    DefinitionContextPtr dc = unit->findDefinitionContext(file);
    assert(dc);
    string q = dc->findMetaData(prefix);
    if(!q.empty())
    {
        q = q.substr(prefix.size());
    }
    _filePackagePrefix[file] = q;
    return q;
}

string
Slice::JavaGenerator::getPackage(const ContainedPtr& cont) const
{
    string scope = convertScopedName(cont->scope());
    string prefix = getPackagePrefix(cont);
    if(!prefix.empty())
    {
        if(!scope.empty())
        {
            return prefix + "." + scope;
        }
        else
        {
            return prefix;
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
Slice::JavaGenerator::getStaticId(const TypePtr& type, const string& package) const
{
    BuiltinPtr b = BuiltinPtr::dynamicCast(type);
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);

    assert((b && b->kind() == Builtin::KindObject) || cl);

    if(b)
    {
        return "Ice.ObjectImpl.ice_staticId()";
    }
    else if(cl->isInterface())
    {
        return getAbsolute(cl, package, "_", "Disp") + ".ice_staticId()";
    }
    else
    {
        return getAbsolute(cl, package) + ".ice_staticId()";
    }
}

bool
Slice::JavaGenerator::useOptionalMapping(const OperationPtr& p)
{
    //
    // The "java:optional" metadata can be applied to an operation or its
    // interface to force the mapping to use the Ice.Optional types.
    //
    // Without the tag, parameters use the normal (non-optional) mapping.
    //
    static const string tag = "java:optional";

    ClassDefPtr cl = ClassDefPtr::dynamicCast(p->container());
    assert(cl);

    return p->hasMetaData(tag) || cl->hasMetaData(tag);
}

string
Slice::JavaGenerator::getOptionalFormat(const TypePtr& type)
{
    BuiltinPtr bp = BuiltinPtr::dynamicCast(type);
    if(bp)
    {
        switch(bp->kind())
        {
        case Builtin::KindByte:
        case Builtin::KindBool:
        {
            return "Ice.OptionalFormat.F1";
        }
        case Builtin::KindShort:
        {
            return "Ice.OptionalFormat.F2";
        }
        case Builtin::KindInt:
        case Builtin::KindFloat:
        {
            return "Ice.OptionalFormat.F4";
        }
        case Builtin::KindLong:
        case Builtin::KindDouble:
        {
            return "Ice.OptionalFormat.F8";
        }
        case Builtin::KindString:
        {
            return "Ice.OptionalFormat.VSize";
        }
        case Builtin::KindObject:
        {
            return "Ice.OptionalFormat.Class";
        }
        case Builtin::KindObjectProxy:
        {
            return "Ice.OptionalFormat.FSize";
        }
        case Builtin::KindLocalObject:
        {
            assert(false);
            break;
        }
        }
    }

    if(EnumPtr::dynamicCast(type))
    {
        return "Ice.OptionalFormat.Size";
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        return seq->type()->isVariableLength() ? "Ice.OptionalFormat.FSize" : "Ice.OptionalFormat.VSize";
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    if(d)
    {
        return (d->keyType()->isVariableLength() || d->valueType()->isVariableLength()) ?
            "Ice.OptionalFormat.FSize" : "Ice.OptionalFormat.VSize";
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        return st->isVariableLength() ? "Ice.OptionalFormat.FSize" : "Ice.OptionalFormat.VSize";
    }

    if(ProxyPtr::dynamicCast(type))
    {
        return "Ice.OptionalFormat.FSize";
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    assert(cl);
    return "Ice.OptionalFormat.Class";
}

string
Slice::JavaGenerator::typeToString(const TypePtr& type,
                                   TypeMode mode,
                                   const string& package,
                                   const StringList& metaData,
                                   bool formal,
                                   bool optional) const
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
    static const char* builtinOptionalTable[] =
    {
        "Ice.ByteOptional",
        "Ice.BooleanOptional",
        "Ice.ShortOptional",
        "Ice.IntOptional",
        "Ice.LongOptional",
        "Ice.FloatOptional",
        "Ice.DoubleOptional",
        "???",
        "???",
        "???",
        "???"
    };

    if(!type)
    {
        assert(mode == TypeModeReturn);
        return "void";
    }

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        if(optional)
        {
            switch(builtin->kind())
            {
                case Builtin::KindByte:
                case Builtin::KindBool:
                case Builtin::KindShort:
                case Builtin::KindInt:
                case Builtin::KindLong:
                case Builtin::KindFloat:
                case Builtin::KindDouble:
                {
                    return builtinOptionalTable[builtin->kind()];
                }
                case Builtin::KindString:
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                {
                    break;
                }
            }
        }
        else
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
    }

    if(optional)
    {
        return "Ice.Optional<" + typeToString(type, TypeModeIn, package, metaData, formal) + ">";
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
            //
            // Only use the type's generated holder if the instance and
            // formal types match.
            //
            string instanceType, formalType;
            getDictionaryTypes(dict, "", metaData, instanceType, formalType);
            string origInstanceType, origFormalType;
            getDictionaryTypes(dict, "", StringList(), origInstanceType, origFormalType);
            if(formalType == origFormalType && instanceType == origInstanceType)
            {
                return getAbsolute(dict, package, "", "Holder");
            }

            //
            // The custom type may or may not be compatible with the type used
            // in the generated holder. We use a generic holder that holds a value of the
            // formal custom type.
            //
            return string("Ice.Holder<") + formalType + " >";
        }
        else
        {
            string instanceType, formalType;
            getDictionaryTypes(dict, package, metaData, instanceType, formalType);
            return formal ? formalType : instanceType;
        }
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        if(mode == TypeModeOut)
        {
            string instanceType, formalType;
            getSequenceTypes(seq, "", metaData, instanceType, formalType);
            if(sequenceHasHolder(seq))
            {
                //
                // Only use the type's generated holder if the instance and
                // formal types match.
                //
                string origInstanceType, origFormalType;
                getSequenceTypes(seq, "", StringList(), origInstanceType, origFormalType);
                if(formalType == origFormalType && instanceType == origInstanceType)
                {
                    return getAbsolute(seq, package, "", "Holder");
                }
            }

            //
            // The custom type may or may not be compatible with the type used
            // in the generated holder. We use a generic holder that holds a value of the
            // formal custom type.
            //
            return string("Ice.Holder<") + formalType + " >";
        }
        else
        {
            string instanceType, formalType;
            getSequenceTypes(seq, package, metaData, instanceType, formalType);
            return formal ? formalType : instanceType;
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
                                         bool formal) const
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

    return typeToString(type, mode, package, metaData, formal);
}

void
Slice::JavaGenerator::writeMarshalUnmarshalCode(Output& out,
                                                const string& package,
                                                const TypePtr& type,
                                                OptionalMode mode,
                                                bool optionalMapping,
                                                int tag,
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

    const bool optionalParam = mode == OptionalInParam || mode == OptionalOutParam || mode == OptionalReturnParam;

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindByte:
            {
                if(marshal)
                {
                    if(optionalParam)
                    {
                        out << nl << stream << ".writeByte(" << tag << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << stream << ".writeByte(" << v << ");";
                    }
                }
                else
                {
                    if(optionalParam)
                    {
                        out << nl << stream << ".readByte(" << tag << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readByte();";
                    }
                }
                break;
            }
            case Builtin::KindBool:
            {
                if(marshal)
                {
                    if(optionalParam)
                    {
                        out << nl << stream << ".writeBool(" << tag << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << stream << ".writeBool(" << v << ");";
                    }
                }
                else
                {
                    if(optionalParam)
                    {
                        out << nl << stream << ".readBool(" << tag << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readBool();";
                    }
                }
                break;
            }
            case Builtin::KindShort:
            {
                if(marshal)
                {
                    if(optionalParam)
                    {
                        out << nl << stream << ".writeShort(" << tag << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << stream << ".writeShort(" << v << ");";
                    }
                }
                else
                {
                    if(optionalParam)
                    {
                        out << nl << stream << ".readShort(" << tag << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readShort();";
                    }
                }
                break;
            }
            case Builtin::KindInt:
            {
                if(marshal)
                {
                    if(optionalParam)
                    {
                        out << nl << stream << ".writeInt(" << tag << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << stream << ".writeInt(" << v << ");";
                    }
                }
                else
                {
                    if(optionalParam)
                    {
                        out << nl << stream << ".readInt(" << tag << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readInt();";
                    }
                }
                break;
            }
            case Builtin::KindLong:
            {
                if(marshal)
                {
                    if(optionalParam)
                    {
                        out << nl << stream << ".writeLong(" << tag << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << stream << ".writeLong(" << v << ");";
                    }
                }
                else
                {
                    if(optionalParam)
                    {
                        out << nl << stream << ".readLong(" << tag << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readLong();";
                    }
                }
                break;
            }
            case Builtin::KindFloat:
            {
                if(marshal)
                {
                    if(optionalParam)
                    {
                        out << nl << stream << ".writeFloat(" << tag << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << stream << ".writeFloat(" << v << ");";
                    }
                }
                else
                {
                    if(optionalParam)
                    {
                        out << nl << stream << ".readFloat(" << tag << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readFloat();";
                    }
                }
                break;
            }
            case Builtin::KindDouble:
            {
                if(marshal)
                {
                    if(optionalParam)
                    {
                        out << nl << stream << ".writeDouble(" << tag << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << stream << ".writeDouble(" << v << ");";
                    }
                }
                else
                {
                    if(optionalParam)
                    {
                        out << nl << stream << ".readDouble(" << tag << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readDouble();";
                    }
                }
                break;
            }
            case Builtin::KindString:
            {
                if(marshal)
                {
                    if(optionalParam)
                    {
                        out << nl << stream << ".writeString(" << tag << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << stream << ".writeString(" << v << ");";
                    }
                }
                else
                {
                    if(optionalParam)
                    {
                        out << nl << stream << ".readString(" << tag << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readString();";
                    }
                }
                break;
            }
            case Builtin::KindObject:
            {
                if(marshal)
                {
                    if(optionalParam)
                    {
                        out << nl << stream << ".writeObject(" << tag << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << stream << ".writeObject(" << v << ");";
                    }
                }
                else
                {
                    if(optionalParam)
                    {
                        out << nl << stream << ".readObject(" << tag << ", " << param << ");";
                    }
                    else if(holder && mode == OptionalNone)
                    {
                        out << nl << stream << ".readObject(" << param << ");";
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
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if(marshal)
                {
                    if(optionalParam)
                    {
                        out << nl << stream << ".writeProxy(" << tag << ", " << v << ");";
                    }
                    else if(mode == OptionalMember)
                    {
                        out << nl << "int __pos = " << stream << ".startSize();";
                        out << nl << stream << ".writeProxy(" << v << ");";
                        out << nl << stream << ".endSize(__pos);";
                    }
                    else
                    {
                        out << nl << stream << ".writeProxy(" << v << ");";
                    }
                }
                else
                {
                    if(optionalParam)
                    {
                        out << nl << stream << ".readProxy(" << tag << ", " << v << ");";
                    }
                    else if(mode == OptionalMember)
                    {
                        out << nl << stream << ".skip(4);";
                        out << nl << v << " = " << stream << ".readProxy();";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readProxy();";
                    }
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
            if(optionalParam)
            {
                if(optionalMapping)
                {
                    out << nl << "if(" << v << " != null && " << v << ".isSet() && " << stream << ".writeOpt(" << tag
                        << ", " << getOptionalFormat(type) << "))";
                    out << sb;
                    out << nl << "int __pos = " << stream << ".startSize();";
                    out << nl << typeS << "Helper.__write(" << stream << ", " << v << ".get());";
                    out << nl << stream << ".endSize(__pos);";
                    out << eb;
                }
                else
                {
                    out << nl << "if(" << stream << ".writeOpt(" << tag << ", " << getOptionalFormat(type) << "))";
                    out << sb;
                    out << nl << "int __pos = " << stream << ".startSize();";
                    out << nl << typeS << "Helper.__write(" << stream << ", " << v << ");";
                    out << nl << stream << ".endSize(__pos);";
                    out << eb;
                }
            }
            else if(mode == OptionalMember)
            {
                out << nl << "int __pos = " << stream << ".startSize();";
                out << nl << typeS << "Helper.__write(" << stream << ", " << v << ");";
                out << nl << stream << ".endSize(__pos);";
            }
            else
            {
                out << nl << typeS << "Helper.__write(" << stream << ", " << v << ");";
            }
        }
        else
        {
            if(optionalParam)
            {
                out << nl << "if(" << stream << ".readOpt(" << tag << ", " << getOptionalFormat(type) << "))";
                out << sb;
                out << nl << stream << ".skip(4);";
                out << nl << v << ".set(" << typeS << "Helper.__read(" << stream << "));";
                out << eb;
                if(mode == OptionalOutParam)
                {
                    out << nl << "else";
                    out << sb;
                    out << nl << v << ".clear();";
                    out << eb;
                }
            }
            else if(mode == OptionalMember)
            {
                out << nl << stream << ".skip(4);";
                out << nl << v << " = " << typeS << "Helper.__read(" << stream << ");";
            }
            else
            {
                out << nl << v << " = " << typeS << "Helper.__read(" << stream << ");";
            }
        }
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(marshal)
        {
            if(optionalParam)
            {
                out << nl << stream << ".writeObject(" << tag << ", " << v << ");";
            }
            else
            {
                out << nl << stream << ".writeObject(" << v << ");";
            }
        }
        else
        {
            if(optionalParam)
            {
                string typeS = typeToString(type, TypeModeIn, package);
                out << nl << "if(" << stream << ".readOpt(" << tag << ", " << getOptionalFormat(type) << "))";
                out << sb;
                out << nl << stream << ".readObject(new Ice.OptionalObject(" << v << ", " << typeS << ".class, "
                    << getStaticId(type, package) << "));";
                out << eb;
                if(mode == OptionalOutParam)
                {
                    out << nl << "else";
                    out << sb;
                    out << nl << v << ".clear();";
                    out << eb;
                }
            }
            else
            {
                if(holder && mode == OptionalNone)
                {
                    out << nl << stream << ".readObject(" << param << ");";
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
        string typeS = typeToString(type, TypeModeIn, package, metaData);
        if(marshal)
        {
            if(optionalParam || mode == OptionalMember)
            {
                string val;
                if(optionalParam)
                {
                    if(optionalMapping)
                    {
                        out << nl << "if(" << v << " != null && " << v << ".isSet() && " << stream << ".writeOpt("
                            << tag << ", " << getOptionalFormat(type) << "))";
                        val = v + ".get()";
                    }
                    else
                    {
                        out << nl << "if(" << stream << ".writeOpt(" << tag << ", " << getOptionalFormat(type) << "))";
                        val = v;
                    }
                    out << sb;
                }
                else
                {
                    val = v;
                }

                if(st->isVariableLength())
                {
                    out << nl << "int __pos = " <<  stream << ".startSize();";
                    out << nl << typeS << ".__write(" << stream << ", " << val << ");";
                    out << nl << stream << ".endSize(__pos);";
                }
                else
                {
                    out << nl << stream << ".writeSize(" << st->minWireSize() << ");";
                    out << nl << typeS << ".__write(" << stream << ", " << val << ");";
                }
                if(optionalParam)
                {
                    out << eb;
                }
            }
            else
            {
                out << nl << typeS << ".__write(" << stream << ", " << v << ");";
            }
        }
        else
        {
            if(optionalParam)
            {
                out << nl << "if(" << stream << ".readOpt(" << tag << ", " << getOptionalFormat(type) << "))";
                out << sb;

                if(st->isVariableLength())
                {
                    out << nl << stream << ".skip(4);";
                }
                else
                {
                    out << nl << stream << ".skipSize();";
                }

                out << nl << typeS << " __tmpOpt = new " << typeS << "();";
                out << nl << "__tmpOpt.__read(" << stream << ");";
                out << nl << v << ".set(__tmpOpt);";

                out << eb;

                if(mode == OptionalOutParam)
                {
                    out << nl << "else";
                    out << sb;
                    out << nl << v << ".clear();";
                    out << eb;
                }
            }
            else if(mode == OptionalMember)
            {
                if(st->isVariableLength())
                {
                    out << nl << stream << ".skip(4);";
                }
                else
                {
                    out << nl << stream << ".skipSize();";
                }
                out << nl << v << " = " << typeS << ".__read(" << stream << ", " << v << ");";
            }
            else
            {
                out << nl << v << " = " << typeS << ".__read(" << stream << ", " << v << ");";
            }
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        string typeS = typeToString(type, TypeModeIn, package, metaData);
        if(marshal)
        {
            if(optionalParam)
            {
                if(optionalMapping)
                {
                    out << nl << "if(" << v << " != null && " << v << ".isSet() && " << stream << ".writeOpt(" << tag
                        << ", " << getOptionalFormat(type) << "))";
                    out << sb;
                    out << nl << typeS << ".__write(" << stream << ", " << v << ".get());";
                    out << eb;
                }
                else
                {
                    out << nl << "if(" << stream << ".writeOpt(" << tag << ", " << getOptionalFormat(type) << "))";
                    out << sb;
                    out << nl << typeS << ".__write(" << stream << ", " << v << ");";
                    out << eb;
                }
            }
            else
            {
                out << nl << typeS << ".__write(" << stream << ", " << v << ");";
            }
        }
        else
        {
            if(optionalParam)
            {
                out << nl << "if(" << stream << ".readOpt(" << tag << ", " << getOptionalFormat(type) << "))";
                out << sb;
                out << nl << v << ".set(" << typeS << ".__read(" << stream << "));";
                out << eb;
                if(mode == OptionalOutParam)
                {
                    out << nl << "else";
                    out << sb;
                    out << nl << v << ".clear();";
                    out << eb;
                }
            }
            else
            {
                out << nl << v << " = " << typeS << ".__read(" << stream << ");";
            }
        }
        return;
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        if(optionalParam || mode == OptionalMember)
        {
            string typeS = typeToString(type, TypeModeIn, package, metaData);
            TypePtr keyType = dict->keyType();
            TypePtr valueType = dict->valueType();

            if(marshal)
            {
                if(optionalParam)
                {
                    if(optionalMapping)
                    {
                        out << nl << "if(" << v << " != null && " << v << ".isSet() && " << stream << ".writeOpt("
                            << tag << ", " << getOptionalFormat(type) << "))";
                        out << sb;
                    }
                    else
                    {
                        out << nl << "if(" << stream << ".writeOpt(" << tag << ", " << getOptionalFormat(type) << "))";
                        out << sb;
                    }
                }

                if(keyType->isVariableLength() || valueType->isVariableLength())
                {
                    string d = optionalParam && optionalMapping ? v + ".get()" : v;
                    out << nl << "int __pos = " <<  stream << ".startSize();";
                    writeDictionaryMarshalUnmarshalCode(out, package, dict, d, marshal, iter, true, metaData);
                    out << nl << stream << ".endSize(__pos);";
                }
                else
                {
                    const size_t wireSize = keyType->minWireSize() + valueType->minWireSize();
                    string tmpName;
                    if(optionalParam && optionalMapping)
                    {
                        tmpName = "__optDict";
                        out << nl << "final " << typeS << ' ' << tmpName << " = " << v << ".get();";
                    }
                    else
                    {
                        tmpName = v;
                    }
                    out << nl << "final int __optSize = " << tmpName << " == null ? 0 : " << tmpName << ".size();";
                    out << nl << stream << ".writeSize(__optSize > 254 ? __optSize * " << wireSize
                        << " + 5 : __optSize * " << wireSize << " + 1);";
                    writeDictionaryMarshalUnmarshalCode(out, package, dict, tmpName, marshal, iter, true, metaData);
                }

                if(optionalParam)
                {
                    out << eb;
                }
            }
            else
            {
                string tmpName;

                if(optionalParam)
                {
                    tmpName = "__optDict";
                    out << nl << "if(" << stream << ".readOpt(" << tag << ", " << getOptionalFormat(type) << "))";
                    out << sb;
                    out << nl << typeS << ' ' << tmpName << ';';
                }
                else
                {
                    tmpName = v;
                }

                if(keyType->isVariableLength() || valueType->isVariableLength())
                {
                    out << nl << stream << ".skip(4);";
                }
                else
                {
                    out << nl << stream << ".skipSize();";
                }

                writeDictionaryMarshalUnmarshalCode(out, package, dict, tmpName, marshal, iter, true, metaData);

                if(optionalParam)
                {
                    out << nl << v << ".set(" << tmpName << ");";
                    out << eb;
                    if(mode == OptionalOutParam)
                    {
                        out << nl << "else";
                        out << sb;
                        out << nl << v << ".clear();";
                        out << eb;
                    }
                }
            }
        }
        else
        {
            writeDictionaryMarshalUnmarshalCode(out, package, dict, v, marshal, iter, true, metaData);
        }
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        if(optionalParam || mode == OptionalMember)
        {
            string typeS = typeToString(type, TypeModeIn, package, metaData);
            TypePtr elemType = seq->type();
            BuiltinPtr elemBuiltin = BuiltinPtr::dynamicCast(elemType);

            if(optionalParam && elemBuiltin && elemBuiltin->kind() != Builtin::KindObject &&
               elemBuiltin->kind() != Builtin::KindObjectProxy && !hasTypeMetaData(seq, metaData))
            {
                static const char* builtinTable[] =
                {
                    "Byte",
                    "Bool",
                    "Short",
                    "Int",
                    "Long",
                    "Float",
                    "Double",
                    "String",
                    "???",
                    "???",
                    "???"
                };

                switch(elemBuiltin->kind())
                {
                    case Builtin::KindByte:
                    case Builtin::KindBool:
                    case Builtin::KindShort:
                    case Builtin::KindInt:
                    case Builtin::KindLong:
                    case Builtin::KindFloat:
                    case Builtin::KindDouble:
                    case Builtin::KindString:
                    {
                        string bs = builtinTable[elemBuiltin->kind()];

                        if(marshal)
                        {
                            out << nl << stream << ".write" << bs << "Seq(" << tag << ", " << v << ");";
                        }
                        else
                        {
                            out << nl << stream << ".read" << bs << "Seq(" << tag << ", " << v << ");";
                        }
                        return;
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

            string ignore;
            const size_t wireSize = elemType->minWireSize();

            if(marshal)
            {
                if(optionalParam)
                {
                    if(optionalMapping)
                    {
                        out << nl << "if(" << v << " != null && " << v << ".isSet() && " << stream << ".writeOpt("
                            << tag << ", " << getOptionalFormat(type) << "))";
                    }
                    else
                    {
                        out << nl << "if(" << stream << ".writeOpt(" << tag << ", " << getOptionalFormat(type) << "))";
                    }

                    out << sb;
                }

                if(elemType->isVariableLength())
                {
                    string s = optionalParam && optionalMapping ? v + ".get()" : v;
                    out << nl << "int __pos = " <<  stream << ".startSize();";
                    writeSequenceMarshalUnmarshalCode(out, package, seq, s, marshal, iter, true, metaData);
                    out << nl << stream << ".endSize(__pos);";
                }
                else if(findMetaData("java:type:", metaData, ignore) ||
                        findMetaData("java:type:", seq->getMetaData(), ignore))
                {
                    //
                    // The sequence is an instance of java.util.List<E>, where E is a fixed-size type.
                    // If the element type is bool or byte, we do NOT write an extra size.
                    //

                    string tmpName;
                    if(optionalParam && optionalMapping)
                    {
                        tmpName = "__optSeq";
                        out << nl << "final " << typeS << ' ' << tmpName << " = " << v << ".get();";
                    }
                    else
                    {
                        tmpName = v;
                    }

                    if(wireSize > 1)
                    {
                        out << nl << "final int __optSize = " << tmpName << " == null ? 0 : " << tmpName << ".size();";
                        out << nl << stream << ".writeSize(__optSize > 254 ? __optSize * " << wireSize
                            << " + 5 : __optSize * " << wireSize << " + 1);";
                    }
                    writeSequenceMarshalUnmarshalCode(out, package, seq, tmpName, marshal, iter, true, metaData);
                }
                else if(findMetaData("java:protobuf:", seq->getMetaData(), ignore) ||
                        findMetaData("java:serializable:", seq->getMetaData(), ignore))
                {
                    //
                    // This just writes a byte sequence.
                    //
                    string s = optionalParam && optionalMapping ? v + ".get()" : v;
                    writeSequenceMarshalUnmarshalCode(out, package, seq, s, marshal, iter, true, metaData);
                }
                else
                {
                    //
                    // At this point we have a regular Java array of a fixed-size type.
                    //

                    string tmpName;
                    if(optionalParam && optionalMapping)
                    {
                        tmpName = "__optSeq";
                        out << nl << "final " << typeS << ' ' << tmpName << " = " << v << ".get();";
                    }
                    else
                    {
                        tmpName = v;
                    }

                    if(wireSize > 1)
                    {
                        out << nl << "final int __optSize = " << tmpName << " == null ? 0 : " << tmpName << ".length;";
                        out << nl << stream << ".writeSize(__optSize > 254 ? __optSize * " << wireSize
                            << " + 5 : __optSize * " << wireSize << " + 1);";
                    }

                    writeSequenceMarshalUnmarshalCode(out, package, seq, tmpName, marshal, iter, true, metaData);
                }

                if(optionalParam)
                {
                    out << eb;
                }
            }
            else
            {
                string tmpName;
                if(optionalParam)
                {
                    tmpName = "__optSeq";
                    out << nl << "if(" << stream << ".readOpt(" << tag << ", " << getOptionalFormat(type) << "))";
                    out << sb;
                    out << nl << typeS << ' ' << tmpName << ';';
                }
                else
                {
                    tmpName = v;
                }

                if(elemType->isVariableLength())
                {
                    out << nl << stream << ".skip(4);";
                }
                else if(wireSize > 1)
                {
                    if(findMetaData("java:type:", metaData, ignore) ||
                       findMetaData("java:type:", seq->getMetaData(), ignore))
                    {
                        //
                        // The sequence is an instance of java.util.List<E>, where E is a fixed-size type.
                        //

                        out << nl << stream << ".skipSize();";
                    }
                    else if(!findMetaData("java:protobuf:", seq->getMetaData(), ignore) &&
                            !findMetaData("java:serializable:", seq->getMetaData(), ignore))
                    {
                        out << nl << stream << ".skipSize();";
                    }
                }

                writeSequenceMarshalUnmarshalCode(out, package, seq, tmpName, marshal, iter, true, metaData);

                if(optionalParam)
                {
                    out << nl << v << ".set(" << tmpName << ");";
                    out << eb;
                    if(mode == OptionalOutParam)
                    {
                        out << nl << "else";
                        out << sb;
                        out << nl << v << ".clear();";
                        out << eb;
                    }
                }
            }
        }
        else
        {
            writeSequenceMarshalUnmarshalCode(out, package, seq, v, marshal, iter, true, metaData);
        }
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

    string instanceType;

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
    string formalType;
    getDictionaryTypes(dict, "", metaData, instanceType, formalType);
    string origInstanceType, origFormalType;
    getDictionaryTypes(dict, "", StringList(), origInstanceType, origFormalType);
    if((formalType != origFormalType) || (!marshal && instanceType != origInstanceType))
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
        string keyObjectS = typeToObjectString(key, TypeModeIn, package);
        string valueObjectS = typeToObjectString(value, TypeModeIn, package);
        out << nl << "for(java.util.Map.Entry<" << keyObjectS << ", " << valueObjectS << "> __e : " << v
            << ".entrySet())";
        out << sb;
        for(int i = 0; i < 2; i++)
        {
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
            writeMarshalUnmarshalCode(out, package, type, OptionalNone, false, 0, arg, true, iter, false);
        }
        out << eb;
        out << eb;
    }
    else
    {
        out << nl << v << " = new " << instanceType << "();";
        out << nl << "int __sz" << iterS << " = __is.readSize();";
        out << nl << "for(int __i" << iterS << " = 0; __i" << iterS << " < __sz" << iterS << "; __i" << iterS << "++)";
        out << sb;
        for(int i = 0; i < 2; i++)
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
            if(ClassDeclPtr::dynamicCast(type) || (b && b->kind() == Builtin::KindObject))
            {
                string keyTypeStr = typeToObjectString(key, TypeModeIn, package);
                string valueTypeStr = typeToObjectString(value, TypeModeIn, package);
                writeMarshalUnmarshalCode(out, package, type, OptionalNone, false, 0, arg, false, iter, false,
                                          StringList(),
                                          "new IceInternal.DictionaryPatcher<" + keyTypeStr + ", " + valueTypeStr +
                                          ">(" + v + ", " + typeS + ".class, \"" + type->typeId() + "\", __key)");
            }
            else
            {
                if(StructPtr::dynamicCast(type))
                {
                    out << nl << typeS << ' ' << arg << " = null;";
                }
                else
                {
                    out << nl << typeS << ' ' << arg << ';';
                }
                writeMarshalUnmarshalCode(out, package, type, OptionalNone, false, 0, arg, false, iter, false);
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

    //
    // If the sequence is a byte sequence, check if there's the serializable or protobuf metadata to
    // get rid of these two easy cases first.
    //
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
    if(builtin && builtin->kind() == Builtin::KindByte)
    {
        string meta;
        static const string protobuf = "java:protobuf:";
        static const string serializable = "java:serializable:";
        if(seq->findMetaData(serializable, meta))
        {
            if(marshal)
            {
                out << nl << stream << ".writeSerializable(" << v << ");";
            }
            else
            {
                string type = typeToString(seq, TypeModeIn, package);
                out << nl << v << " = (" << type << ")" << stream << ".readSerializable();";
            }
            return;
        }
        else if(seq->findMetaData(protobuf, meta))
        {
            if(marshal)
            {
                out << nl << "if(!" << v << ".isInitialized())";
                out << sb;
                out << nl << "throw new Ice.MarshalException(\"type not fully initialized\");";
                out << eb;
                out << nl << stream << ".writeByteSeq(" << v << ".toByteArray());";
            }
            else
            {
                string type = typeToString(seq, TypeModeIn, package);
                out << nl << "try";
                out << sb;
                out << nl << v << " = " << type << ".parseFrom(" << stream << ".readByteSeq());";
                out << eb;
                out << nl << "catch(com.google.protobuf.InvalidProtocolBufferException __ex)";
                out << sb;
                out << nl << "throw new Ice.MarshalException(__ex);";
                out << eb;
            }
            return;
        }
    }

    if(builtin &&
       (builtin->kind() == Builtin::KindByte || builtin->kind() == Builtin::KindShort ||
        builtin->kind() == Builtin::KindInt || builtin->kind() == Builtin::KindLong ||
        builtin->kind() == Builtin::KindFloat || builtin->kind() == Builtin::KindDouble))
    {
        string meta;
        static const string bytebuffer = "java:buffer";
        if(seq->findMetaData(bytebuffer, meta) || findMetaData(bytebuffer, metaData, meta))
        {
            switch(builtin->kind())
            {
                case Builtin::KindByte:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeByteBuffer(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readByteBuffer();";
                    }
                    break;
                }
                case Builtin::KindShort:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeShortBuffer(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readShortBuffer();";
                    }
                    break;
                }
                case Builtin::KindInt:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeIntBuffer(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readIntBuffer();";
                    }
                    break;
                }
                case Builtin::KindLong:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeLongBuffer(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readLongBuffer();";
                    }
                    break;
                }
                case Builtin::KindFloat:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeFloatBuffer(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readFloatBuffer();";
                    }
                    break;
                }
                case Builtin::KindDouble:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeDoubleBuffer(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readDoubleBuffer();";
                    }
                    break;
                }
                case Builtin::KindBool:
                case Builtin::KindString:
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                {
                    assert(false);
                    break;
                }
            }
            return;
        }
    }

    bool customType = false;
    string instanceType;

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
    string formalType;
    customType = getSequenceTypes(seq, "", metaData, instanceType, formalType);
    string origInstanceType, origFormalType;
    getSequenceTypes(seq, "", StringList(), origInstanceType, origFormalType);
    if((formalType != origFormalType) || (!marshal && instanceType != origInstanceType))
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
    // Determine sequence depth.
    //
    int depth = 0;
    TypePtr origContent = seq->type();
    SequencePtr s = SequencePtr::dynamicCast(origContent);
    while(s)
    {
        //
        // Stop if the inner sequence type has a custom, serializable or protobuf type.
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
            string typeS = typeToString(type, TypeModeIn, package);
            out << nl << "for(" << typeS << " __elem : " << v << ')';
            out << sb;
            writeMarshalUnmarshalCode(out, package, type, OptionalNone, false, 0, "__elem", true, iter, false);
            out << eb;
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
            out << nl << v << " = new " << instanceType << "();";
            out << nl << "final int __len" << iter << " = " << stream << ".readAndCheckSeqSize(" << type->minWireSize()
                << ");";
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
                patchParams << "new IceInternal.ListPatcher<" << origContentS << ">(" << v << ", " << origContentS
                            << ".class, __type" << iter << ", __i" << iter << ')';
                writeMarshalUnmarshalCode(out, package, type, OptionalNone, false, 0, "__elem", false, iter,
                                          false, StringList(), patchParams.str());
            }
            else
            {
                if(StructPtr::dynamicCast(type))
                {
                    out << nl << cont << " __elem = null;";
                }
                else
                {
                    out << nl << cont << " __elem;";
                }
                writeMarshalUnmarshalCode(out, package, type, OptionalNone, false, 0, "__elem", false, iter, false);
            }
            if(!isObject)
            {
                out << nl << v << ".add(__elem);";
            }
            out << eb;
            iter++;
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
                writeMarshalUnmarshalCode(out, package, type, OptionalNone, false, 0, o.str(), true, iter, false);
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
                out << nl << "final int __len" << iter << " = " << stream << ".readAndCheckSeqSize("
                    << type->minWireSize() << ");";
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
                // We cannot allocate an array of a generic type, such as
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
                    writeMarshalUnmarshalCode(out, package, type, OptionalNone, false, 0, o.str(), false, iter,
                                              false, StringList(), patchParams.str());
                }
                else
                {
                    writeMarshalUnmarshalCode(out, package, type, OptionalNone, false, 0, o.str(), false, iter, false);
                }
                out << eb;
                iter++;
            }
        }
    }
}

void
Slice::JavaGenerator::writeStreamMarshalUnmarshalCode(Output& out,
                                                      const string& package,
                                                      const TypePtr& type,
                                                      bool optional,
                                                      int /*tag*/,
                                                      const string& param,
                                                      bool marshal,
                                                      int& iter,
                                                      bool holder,
                                                      const StringList& metaData,
                                                      const string& patchParams)
{
    string stream = marshal ? "__outS" : "__inS";
    string v;
    if(holder && !optional)
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
                if(marshal)
                {
                    out << nl << stream << ".writeObject(" << v << ");";
                }
                else
                {
                    if(holder && !optional)
                    {
                        out << nl << stream << ".readObject(" << param << ");";
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
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if(marshal)
                {
                    if(optional)
                    {
                        out << nl << "int __pos = " <<  stream << ".startSize();";
                        out << nl << stream << ".writeProxy(" << v << ");";
                        out << nl << stream << ".endSize(__pos);";
                    }
                    else
                    {
                        out << nl << stream << ".writeProxy(" << v << ");";
                    }
                }
                else
                {
                    if(optional)
                    {
                        out << nl << stream << ".skip(4);";
                    }
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
            if(optional)
            {
                out << nl << "int __pos = " <<  stream << ".startSize();";
                out << nl << typeS << "Helper.write(" << stream << ", " << v << ");";
                out << nl << stream << ".endSize(__pos);";
            }
            else
            {
                out << nl << typeS << "Helper.write(" << stream << ", " << v << ");";
            }
        }
        else
        {
            if(optional)
            {
                out << nl << stream << ".skip(4);";
            }
            out << nl << v << " = " << typeS << "Helper.read(" << stream << ");";
        }
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(marshal)
        {
            out << nl << stream << ".writeObject(" << v << ");";
        }
        else
        {
            if(holder && !optional)
            {
                out << nl << stream << ".readObject(" << param << ");";
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
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        string typeS = typeToString(type, TypeModeIn, package);
        if(marshal)
        {
            if(optional)
            {
                if(st->isVariableLength())
                {
                    out << nl << "int __pos = " <<  stream << ".startSize();";
                    out << nl << typeS << ".ice_write(" << stream << ", " << v << ");";
                    out << nl << stream << ".endSize(__pos);";
                }
                else
                {
                    out << nl << stream << ".writeSize(" << st->minWireSize() << ");";
                    out << nl << typeS << ".ice_write(" << stream << ", " << v << ");";
                }
            }
            else
            {
                out << nl << typeS << ".ice_write(" << stream << ", " << v << ");";
            }
        }
        else
        {
            if(optional)
            {
                if(st->isVariableLength())
                {
                    out << nl << stream << ".skip(4);";
                }
                else
                {
                    out << nl << stream << ".skipSize();";
                }
            }
            out << nl << v << " = " << typeS << ".ice_read(" << stream << ", " << v << ");";
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        string typeS = typeToString(type, TypeModeIn, package);
        if(marshal)
        {
            out << nl << typeS << ".ice_write(" << stream << ", " << v << ");";
        }
        else
        {
            out << nl << v << " = " << typeS << ".ice_read(" << stream << ");";
        }
        return;
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        if(optional)
        {
            string typeS = typeToString(type, TypeModeIn, package, metaData);
            TypePtr keyType = dict->keyType();
            TypePtr valueType = dict->valueType();

            if(marshal)
            {
                if(keyType->isVariableLength() || valueType->isVariableLength())
                {
                    out << nl << "int __pos = " <<  stream << ".startSize();";
                    writeStreamDictionaryMarshalUnmarshalCode(out, package, dict, v, marshal, iter, true, metaData);
                    out << nl << stream << ".endSize(__pos);";
                }
                else
                {
                    const size_t wireSize = keyType->minWireSize() + valueType->minWireSize();
                    out << nl << "final int __optSize = " << v << " == null ? 0 : " << v << ".size();";
                    out << nl << stream << ".writeSize(__optSize > 254 ? __optSize * " << wireSize
                        << " + 5 : __optSize * " << wireSize << " + 1);";
                    writeStreamDictionaryMarshalUnmarshalCode(out, package, dict, v, marshal, iter, true, metaData);
                }
            }
            else
            {
                if(keyType->isVariableLength() || valueType->isVariableLength())
                {
                    out << nl << stream << ".skip(4);";
                }
                else
                {
                    out << nl << stream << ".skipSize();";
                }

                writeStreamDictionaryMarshalUnmarshalCode(out, package, dict, v, marshal, iter, true, metaData);
            }
        }
        else
        {
            writeStreamDictionaryMarshalUnmarshalCode(out, package, dict, v, marshal, iter, true, metaData);
        }
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        if(optional)
        {
            string typeS = typeToString(type, TypeModeIn, package, metaData);
            TypePtr elemType = seq->type();
            BuiltinPtr elemBuiltin = BuiltinPtr::dynamicCast(elemType);

            string ignore;

            if(marshal)
            {
                if(elemType->isVariableLength())
                {
                    out << nl << "int __pos = " << stream << ".startSize();";
                    writeStreamSequenceMarshalUnmarshalCode(out, package, seq, v, marshal, iter, true, metaData);
                    out << nl << stream << ".endSize(__pos);";
                }
                else if(findMetaData("java:type:", metaData, ignore) ||
                        findMetaData("java:type:", seq->getMetaData(), ignore))
                {
                    //
                    // The sequence is an instance of java.util.List<E>, where E is a fixed-size type.
                    // If the element type is bool or byte, we do NOT write an extra size.
                    //

                    if(!elemBuiltin ||
                       (elemBuiltin->kind() != Builtin::KindByte && elemBuiltin->kind() != Builtin::KindBool))
                    {
                        out << nl << "final int __optSize = " << v << " == null ? 0 : " << v << ".size();";
                        const size_t wireSize = elemType->minWireSize();

                        if(wireSize > 1)
                        {
                            out << nl << stream << ".writeSize(__optSize > 254 ? __optSize * " << wireSize
                                << " + 5 : __optSize * " << wireSize << " + 1);";
                        }
                        else
                        {
                            //
                            // The element type could be a struct with a wire size of 1.
                            //
                            out << nl << stream << ".writeSize(__optSize > 254 ? __optSize + 5 : __optSize + 1);";
                        }
                    }
                    writeStreamSequenceMarshalUnmarshalCode(out, package, seq, v, marshal, iter, true, metaData);
                }
                else if(findMetaData("java:protobuf:", seq->getMetaData(), ignore) ||
                        findMetaData("java:serializable:", seq->getMetaData(), ignore))
                {
                    //
                    // This just writes a byte sequence.
                    //
                    writeStreamSequenceMarshalUnmarshalCode(out, package, seq, v, marshal, iter, true, metaData);
                }
                else
                {
                    //
                    // At this point we have a regular Java array of a fixed-size type.
                    //

                    if(!elemBuiltin || (elemBuiltin->kind() != Builtin::KindByte &&
                       elemBuiltin->kind() != Builtin::KindBool))
                    {
                        out << nl << "final int __optSize = " << v << " == null ? 0 : " << v << ".length;";
                        const size_t wireSize = elemType->minWireSize();

                        if(wireSize > 1)
                        {
                            out << nl << stream << ".writeSize(__optSize > 254 ? __optSize * " << wireSize
                                << " + 5 : __optSize * " << wireSize << " + 1);";
                        }
                        else
                        {
                            //
                            // The element type could be a struct with a wire size of 1.
                            //
                            out << nl << stream << ".writeSize(__optSize > 254 ? __optSize + 5 : __optSize + 1);";
                        }
                    }

                    writeStreamSequenceMarshalUnmarshalCode(out, package, seq, v, marshal, iter, true, metaData);
                }
            }
            else
            {
                if(elemType->isVariableLength())
                {
                    out << nl << stream << ".skip(4);";
                }
                else if(findMetaData("java:type:", metaData, ignore) ||
                        findMetaData("java:type:", seq->getMetaData(), ignore))
                {
                    //
                    // The sequence is an instance of java.util.List<E>, where E is a fixed-size type.
                    // If the element type is bool or byte, we do NOT write an extra size.
                    //

                    if(!elemBuiltin ||
                       (elemBuiltin->kind() != Builtin::KindByte && elemBuiltin->kind() != Builtin::KindBool))
                    {
                        out << nl << stream << ".skipSize();";
                    }
                }
                else if(!findMetaData("java:protobuf:", seq->getMetaData(), ignore) &&
                        !findMetaData("java:serializable:", seq->getMetaData(), ignore))
                {
                    if(!elemBuiltin ||
                       (elemBuiltin->kind() != Builtin::KindByte && elemBuiltin->kind() != Builtin::KindBool))
                    {
                        out << nl << stream << ".skipSize();";
                    }
                }

                writeStreamSequenceMarshalUnmarshalCode(out, package, seq, v, marshal, iter, true, metaData);
            }
        }
        else
        {
            writeStreamSequenceMarshalUnmarshalCode(out, package, seq, v, marshal, iter, true, metaData);
        }
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
    string instanceType, formalType;
    getDictionaryTypes(dict, "", metaData, instanceType, formalType);
    string origInstanceType, origFormalType;
    getDictionaryTypes(dict, "", StringList(), origInstanceType, origFormalType);
    if((formalType != origFormalType) || (!marshal && instanceType != origInstanceType))
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
        string keyObjectS = typeToObjectString(key, TypeModeIn, package);
        string valueObjectS = typeToObjectString(value, TypeModeIn, package);
        out << nl << "for(java.util.Map.Entry<" << keyObjectS << ", " << valueObjectS << "> __e : " << v
            << ".entrySet())";
        out << sb;
        for(int i = 0; i < 2; i++)
        {
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
            writeStreamMarshalUnmarshalCode(out, package, type, false, 0, arg, true, iter, false);
        }
        out << eb;
        out << eb;
    }
    else
    {
        out << nl << v << " = new " << instanceType << "();";
        out << nl << "int __sz" << iterS << " = __inS.readSize();";
        out << nl << "for(int __i" << iterS << " = 0; __i" << iterS << " < __sz" << iterS << "; __i" << iterS << "++)";
        out << sb;
        for(int i = 0; i < 2; i++)
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
            string s = typeToString(type, TypeModeIn, package);
            if(ClassDeclPtr::dynamicCast(type) || (b && b->kind() == Builtin::KindObject))
            {
                string keyTypeStr = typeToObjectString(key, TypeModeIn, package);
                string valueTypeStr = typeToObjectString(value, TypeModeIn, package);
                writeStreamMarshalUnmarshalCode(out, package, type, false, 0, arg, false, iter, false,
                                                StringList(), "new IceInternal.DictionaryPatcher<" + keyTypeStr +
                                                ", " + valueTypeStr + ">(" + v + ", " + s + ".class, \"" +
                                                type->typeId() + "\", __key)");
            }
            else
            {
                if(StructPtr::dynamicCast(type))
                {
                    out << nl << s << ' ' << arg << " = null;";
                }
                else
                {
                    out << nl << s << ' ' << arg << ';';
                }
                writeStreamMarshalUnmarshalCode(out, package, type, false, 0, arg, false, iter, false);
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

    //
    // If the sequence is a byte sequence, check if there's the serializable or protobuf metadata to
    // get rid of these two easy cases first.
    //
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
    if(builtin && builtin->kind() == Builtin::KindByte)
    {
        string meta;
        static const string protobuf = "java:protobuf:";
        static const string serializable = "java:serializable:";
        if(seq->findMetaData(serializable, meta))
        {
            if(marshal)
            {
                out << nl << stream << ".writeSerializable(" << v << ");";
            }
            else
            {
                string type = typeToString(seq, TypeModeIn, package);
                out << nl << v << " = (" << type << ")" << stream << ".readSerializable();";
            }
            return;
        }
        else if(seq->findMetaData(protobuf, meta))
        {
            if(marshal)
            {
                out << nl << "if(!" << v << ".isInitialized())";
                out << sb;
                out << nl << "throw new Ice.MarshalException(\"type not fully initialized\");";
                out << eb;
                out << nl << stream << ".writeByteSeq(" << v << ".toByteArray());";
            }
            else
            {
                string type = meta.substr(protobuf.size());
                out << nl << "try";
                out << sb;
                out << nl << v << " = " << type << ".parseFrom(" << stream << ".readByteSeq());";
                out << eb;
                out << nl << "catch(com.google.protobuf.InvalidProtocolBufferException __ex)";
                out << sb;
                out << nl << "Ice.MarshalException __mex = new Ice.MarshalException();";
                out << nl << "__mex.initCause(__ex);";
                out << nl << "throw __mex;";
                out << eb;
            }
            return;
        }
    }

    if(builtin &&
       (builtin->kind() == Builtin::KindByte || builtin->kind() == Builtin::KindShort ||
        builtin->kind() == Builtin::KindInt || builtin->kind() == Builtin::KindLong ||
        builtin->kind() == Builtin::KindFloat || builtin->kind() == Builtin::KindDouble))
    {
        string meta;
        static const string bytebuffer = "java:buffer";
        if(seq->findMetaData(bytebuffer, meta) || findMetaData(bytebuffer, metaData, meta))
        {
            switch(builtin->kind())
            {
                case Builtin::KindByte:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeByteBuffer(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readByteBuffer();";
                    }
                    break;
                }
                case Builtin::KindShort:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeShortBuffer(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readShortBuffer();";
                    }
                    break;
                }
                case Builtin::KindInt:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeIntBuffer(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readIntBuffer();";
                    }
                    break;
                }
                case Builtin::KindLong:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeLongBuffer(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readLongBuffer();";
                    }
                    break;
                }
                case Builtin::KindFloat:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeFloatBuffer(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readFloatBuffer();";
                    }
                    break;
                }
                case Builtin::KindDouble:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeDoubleBuffer(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readDoubleBuffer();";
                    }
                    break;
                }
                case Builtin::KindBool:
                case Builtin::KindString:
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                {
                    assert(false);
                    break;
                }
            }
            return;
        }
    }

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
    string instanceType, formalType;
    bool customType = getSequenceTypes(seq, "", metaData, instanceType, formalType);
    string origInstanceType, origFormalType;
    getSequenceTypes(seq, "", StringList(), origInstanceType, origFormalType);
    if((formalType != origFormalType) || (!marshal && instanceType != origInstanceType))
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
        // Stop if the inner sequence type has a custom, serializable or protobuf type.
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
            string typeS = typeToString(type, TypeModeIn, package);
            out << nl << "for(" << typeS << " __elem : " << v << ')';
            out << sb;
            writeStreamMarshalUnmarshalCode(out, package, type, false, 0, "__elem", true, iter, false);
            out << eb;
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
            out << nl << v << " = new " << instanceType << "();";
            out << nl << "final int __len" << iter << " = " << stream << ".readAndCheckSeqSize(" << type->minWireSize()
                << ");";
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
                patchParams << "new IceInternal.ListPatcher<" << origContentS << ">(" << v << ", " << origContentS
                            << ".class, __type" << iter << ", __i" << iter << ')';
                writeStreamMarshalUnmarshalCode(out, package, type, false, 0, "__elem", false, iter, false,
                                                StringList(), patchParams.str());
            }
            else
            {
                if(StructPtr::dynamicCast(type))
                {
                    out << nl << cont << " __elem = null;";
                }
                else
                {
                    out << nl << cont << " __elem;";
                }
                writeStreamMarshalUnmarshalCode(out, package, type, false, 0, "__elem", false, iter, false);
            }
            if(!isObject)
            {
                out << nl << v << ".add(__elem);";
            }
            out << eb;
            iter++;
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
                writeStreamMarshalUnmarshalCode(out, package, type, false, 0, o.str(), true, iter, false);
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
                out << nl << "final int __len" << iter << " = " << stream << ".readAndCheckSeqSize("
                    << type->minWireSize() << ");";
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
                // We cannot allocate an array of a generic type, such as
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
                    writeStreamMarshalUnmarshalCode(out, package, type, false, 0, o.str(), false, iter, false,
                                                    StringList(), patchParams.str());
                }
                else
                {
                    writeStreamMarshalUnmarshalCode(out, package, type, false, 0, o.str(), false, iter, false);
                }
                out << eb;
                iter++;
            }
        }
    }
}

bool
Slice::JavaGenerator::findMetaData(const string& prefix, const StringList& metaData, string& value)
{
    for(StringList::const_iterator q = metaData.begin(); q != metaData.end(); ++q)
    {
        if(q->find(prefix) == 0)
        {
            value = *q;
            return true;
        }
    }

    return false;
}

bool
Slice::JavaGenerator::getTypeMetaData(const StringList& metaData, string& instanceType, string& formalType)
{
    //
    // Extract the instance type and an optional formal type.
    // The correct syntax is "java:type:instance-type[:formal-type]".
    //
    static const string prefix = "java:type:";
    string directive;
    if(findMetaData(prefix, metaData, directive))
    {
        string::size_type pos = directive.find(':', prefix.size());
        if(pos != string::npos)
        {
            instanceType = directive.substr(prefix.size(), pos - prefix.size());
            formalType = directive.substr(pos + 1);
        }
        else
        {
            instanceType = directive.substr(prefix.size());
            formalType.clear();
        }
        return true;
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
        string directive;

        if(findMetaData(prefix, localMetaData, directive))
        {
            return true;
        }

        StringList metaData = cont->getMetaData();

        if(findMetaData(prefix, metaData, directive))
        {
            return true;
        }

        if(findMetaData("java:protobuf:", metaData, directive) ||
           findMetaData("java:serializable:", metaData, directive))
        {
            SequencePtr seq = SequencePtr::dynamicCast(cont);
            if(seq)
            {
                BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
                if(builtin && builtin->kind() == Builtin::KindByte)
                {
                    return true;
                }
            }
        }

        if(findMetaData("java:buffer", localMetaData, directive))
        {
            SequencePtr seq = SequencePtr::dynamicCast(cont);
            if(seq)
            {
                BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
                if(builtin &&
                   (builtin->kind() == Builtin::KindByte || builtin->kind() == Builtin::KindShort ||
                    builtin->kind() == Builtin::KindInt || builtin->kind() == Builtin::KindLong ||
                    builtin->kind() == Builtin::KindFloat || builtin->kind() == Builtin::KindDouble))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool
Slice::JavaGenerator::getDictionaryTypes(const DictionaryPtr& dict,
                                         const string& package,
                                         const StringList& metaData,
                                         string& instanceType,
                                         string& formalType) const
{
    //
    // Get the types of the key and value.
    //
    string keyTypeStr = typeToObjectString(dict->keyType(), TypeModeIn, package);
    string valueTypeStr = typeToObjectString(dict->valueType(), TypeModeIn, package);

    //
    // Collect metadata for a custom type.
    //
    if(getTypeMetaData(metaData, instanceType, formalType) ||
       getTypeMetaData(dict->getMetaData(), instanceType, formalType))
    {
        assert(!instanceType.empty());
        if(formalType.empty())
        {
            formalType = "java.util.Map<" + keyTypeStr + ", " + valueTypeStr + ">";
        }
        return true;
    }

    //
    // Return a default type for the platform.
    //
    instanceType = "java.util.HashMap<" + keyTypeStr + ", " + valueTypeStr + ">";
    formalType = "java.util.Map<" + keyTypeStr + ", " + valueTypeStr + ">";
    return false;
}

bool
Slice::JavaGenerator::getSequenceTypes(const SequencePtr& seq,
                                       const string& package,
                                       const StringList& metaData,
                                       string& instanceType,
                                       string& formalType) const
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
    if(builtin)
    {
        if(builtin->kind() == Builtin::KindByte)
        {
            string prefix = "java:serializable:";
            string meta;
            if(seq->findMetaData(prefix, meta))
            {
                instanceType = formalType = meta.substr(prefix.size());
                return true;
            }
            prefix = "java:protobuf:";
            if(seq->findMetaData(prefix, meta))
            {
                instanceType = formalType = meta.substr(prefix.size());
                return true;
            }
        }

        if((builtin->kind() == Builtin::KindByte || builtin->kind() == Builtin::KindShort ||
            builtin->kind() == Builtin::KindInt || builtin->kind() == Builtin::KindLong ||
            builtin->kind() == Builtin::KindFloat || builtin->kind() == Builtin::KindDouble))
        {
            string prefix = "java:buffer";
            string meta;
            string ignore;
            if(seq->findMetaData(prefix, meta) || findMetaData(prefix, metaData, ignore))
            {
                instanceType = formalType = typeToBufferString(seq->type());
                return true;
            }
        }
    }

    //
    // Collect metadata for a custom type.
    //
    if(getTypeMetaData(metaData, instanceType, formalType) ||
       getTypeMetaData(seq->getMetaData(), instanceType, formalType))
    {
        assert(!instanceType.empty());
        if(formalType.empty())
        {
            formalType = "java.util.List<" + typeToObjectString(seq->type(), TypeModeIn, package) + ">";
        }
        return true;
    }

    //
    // The default mapping is a native array.
    //
    instanceType = formalType = typeToString(seq->type(), TypeModeIn, package, metaData) + "[]";
    return false;
}

bool
Slice::JavaGenerator::sequenceHasHolder(const SequencePtr& p) const
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p->type());
    if(builtin && builtin->kind() == Builtin::KindByte)
    {
        string prefix = "java:serializable:";
        string meta;
        if(p->findMetaData(prefix, meta))
        {
            return false;
        }
        prefix = "java:protobuf:";
        if(p->findMetaData(prefix, meta))
        {
            return false;
        }
    }

    if(builtin &&
       (builtin->kind() == Builtin::KindByte || builtin->kind() == Builtin::KindShort ||
        builtin->kind() == Builtin::KindInt || builtin->kind() == Builtin::KindLong ||
        builtin->kind() == Builtin::KindFloat || builtin->kind() == Builtin::KindDouble))
    {
        string meta;
        string prefix = "java:buffer";
        if(p->findMetaData(prefix, meta))
        {
            return false;
        }
    }

    return true;
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
Slice::JavaGenerator::MetaDataVisitor::visitUnitStart(const UnitPtr& p)
{
    static const string prefix = "java:";

    //
    // Validate global metadata in the top-level file and all included files.
    //
    StringList files = p->allFiles();

    for(StringList::iterator q = files.begin(); q != files.end(); ++q)
    {
        string file = *q;
        DefinitionContextPtr dc = p->findDefinitionContext(file);
        assert(dc);
        StringList globalMetaData = dc->getMetaData();
        for(StringList::const_iterator r = globalMetaData.begin(); r != globalMetaData.end(); ++r)
        {
            string s = *r;
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

                    if(!ok)
                    {
                        emitWarning(file, "",  "ignoring invalid global metadata `" + s + "'");
                    }
                }
                _history.insert(s);
            }
        }
    }
    return true;
}

bool
Slice::JavaGenerator::MetaDataVisitor::visitModuleStart(const ModulePtr& p)
{
    StringList metaData = getMetaData(p);
    validateType(p, metaData, p->file(), p->line());
    validateGetSet(p, metaData, p->file(), p->line());
    return true;
}

void
Slice::JavaGenerator::MetaDataVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    StringList metaData = getMetaData(p);
    validateType(p, metaData, p->file(), p->line());
    validateGetSet(p, metaData, p->file(), p->line());
}

bool
Slice::JavaGenerator::MetaDataVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    StringList metaData = getMetaData(p);
    validateType(p, metaData, p->file(), p->line());
    validateGetSet(p, metaData, p->file(), p->line());
    return true;
}

bool
Slice::JavaGenerator::MetaDataVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    StringList metaData = getMetaData(p);
    validateType(p, metaData, p->file(), p->line());
    validateGetSet(p, metaData, p->file(), p->line());
    return true;
}

bool
Slice::JavaGenerator::MetaDataVisitor::visitStructStart(const StructPtr& p)
{
    StringList metaData = getMetaData(p);
    validateType(p, metaData, p->file(), p->line());
    validateGetSet(p, metaData, p->file(), p->line());
    return true;
}

void
Slice::JavaGenerator::MetaDataVisitor::visitOperation(const OperationPtr& p)
{
    if(p->hasMetaData("UserException"))
    {
        ClassDefPtr cl = ClassDefPtr::dynamicCast(p->container());
        if(!cl->isLocal())
        {
            ostringstream os;
            os << "ignoring invalid metadata `UserException': directive applies only to local operations "
               << "but enclosing " << (cl->isInterface() ? "interface" : "class") << " `" << cl->name()
               << "' is not local";
            emitWarning(p->file(), p->line(), os.str());
        }
    }
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
                    emitWarning(p->file(), p->line(), "ignoring invalid metadata `" + *q +
                                "' for operation with void return type");
                    break;
                }
            }
        }
        else
        {
            validateType(returnType, metaData, p->file(), p->line());
        }
    }

    ParamDeclList params = p->parameters();
    for(ParamDeclList::iterator q = params.begin(); q != params.end(); ++q)
    {
        metaData = getMetaData(*q);
        validateType((*q)->type(), metaData, p->file(), (*q)->line());
    }

    validateGetSet(p, metaData, p->file(), p->line());
}

void
Slice::JavaGenerator::MetaDataVisitor::visitDataMember(const DataMemberPtr& p)
{
    StringList metaData = getMetaData(p);
    validateType(p->type(), metaData, p->file(), p->line());
    validateGetSet(p, metaData, p->file(), p->line());
}

void
Slice::JavaGenerator::MetaDataVisitor::visitSequence(const SequencePtr& p)
{
    static const string protobuf = "java:protobuf:";
    static const string serializable = "java:serializable:";
    static const string bytebuffer = "java:buffer";
    StringList metaData = getMetaData(p);
    const string file =  p->file();
    const string line = p->line();
    for(StringList::const_iterator q = metaData.begin(); q != metaData.end(); )
    {
        string s = *q++;
        if(_history.count(s) == 0) // Don't complain about the same metadata more than once.
        {
            if(s.find(protobuf) == 0 || s.find(serializable) == 0)
            {
                //
                // Remove from list so validateType does not try to handle as well.
                //
                metaData.remove(s);

                BuiltinPtr builtin = BuiltinPtr::dynamicCast(p->type());
                if(!builtin || builtin->kind() != Builtin::KindByte)
                {
                    _history.insert(s);
                    emitWarning(file, line, "ignoring invalid metadata `" + s + "': " +
                                "this metadata can only be used with a byte sequence");
                }
            }
            else if(s.find(bytebuffer) == 0)
            {
                metaData.remove(s);

                BuiltinPtr builtin = BuiltinPtr::dynamicCast(p->type());
                if(!builtin ||
                   (builtin->kind() != Builtin::KindByte && builtin->kind() != Builtin::KindShort &&
                    builtin->kind() != Builtin::KindInt && builtin->kind() != Builtin::KindLong &&
                    builtin->kind() != Builtin::KindFloat && builtin->kind() != Builtin::KindDouble))
                {
                    _history.insert(s);
                    emitWarning(file, line, "ignoring invalid metadata `" + s + "': " +
                                "this metadata can not be used with this type");
                }
            }
        }
    }

    validateType(p, metaData, file, line);
    validateGetSet(p, metaData, file, line);
}

void
Slice::JavaGenerator::MetaDataVisitor::visitDictionary(const DictionaryPtr& p)
{
    StringList metaData = getMetaData(p);
    validateType(p, metaData, p->file(), p->line());
    validateGetSet(p, metaData, p->file(), p->line());
}

void
Slice::JavaGenerator::MetaDataVisitor::visitEnum(const EnumPtr& p)
{
    StringList metaData = getMetaData(p);
    validateType(p, metaData, p->file(), p->line());
    validateGetSet(p, metaData, p->file(), p->line());
}

void
Slice::JavaGenerator::MetaDataVisitor::visitConst(const ConstPtr& p)
{
    StringList metaData = getMetaData(p);
    validateType(p, metaData, p->file(), p->line());
    validateGetSet(p, metaData, p->file(), p->line());
}

StringList
Slice::JavaGenerator::MetaDataVisitor::getMetaData(const ContainedPtr& cont)
{
    static const string prefix = "java:";

    StringList metaData = cont->getMetaData();
    StringList result;

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
                        else if(rest == "buffer")
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
                else if(s.substr(prefix.size(), pos - prefix.size()) == "serializable")
                {
                    result.push_back(s);
                    continue;
                }
                else if(s.substr(prefix.size(), pos - prefix.size()) == "protobuf")
                {
                    result.push_back(s);
                    continue;
                }
                else if(s.substr(prefix.size(), pos - prefix.size()) == "serialVersionUID")
                {
                    result.push_back(s);
                    continue;
                }

                emitWarning(cont->file(), cont->line(), "ignoring invalid metadata `" + s + "'");
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
            emitWarning(file, line, "invalid metadata for " + str);
        }
        else if(i->find("java:buffer") == 0)
        {
            SequencePtr seq = SequencePtr::dynamicCast(p);
            if(seq)
            {
                BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
                if(builtin &&
                   (builtin->kind() == Builtin::KindByte || builtin->kind() == Builtin::KindShort ||
                    builtin->kind() == Builtin::KindInt || builtin->kind() == Builtin::KindLong ||
                    builtin->kind() == Builtin::KindFloat || builtin->kind() == Builtin::KindDouble))
                {
                    continue;
                }

            }
            emitWarning(file, line, "ignoring invalid metadata `" + *i + "'");
        }
        else if(i->find("java:protobuf:") == 0 || i->find("java:serializable:") == 0)
        {
            //
            // Only valid in sequence defintion which is checked in visitSequence
            //
            emitWarning(file, line, "ignoring invalid metadata `" + *i + "'");
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
            emitWarning(file, line, "invalid metadata for " + str);
        }
    }
}
