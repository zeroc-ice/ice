// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Slice/JavaUtil.h>
#include <IceUtil/Functional.h>
#include <limits>

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

Slice::JavaGenerator::JavaGenerator(const string& dir, const string& package) :
    _dir(dir),
    _package(package),
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
    string package;
    string file;
    string path = _dir;

    assert(_out == 0);

    string::size_type pos = absolute.rfind('.');
    if(pos != string::npos)
    {
        package = absolute.substr(0, pos);
        file = absolute.substr(pos + 1);
        string dir = package;

        //
        // Create package directories if necessary
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
                cerr << "can't create directory `" << path << "': "
                     << strerror(errno) << endl;
                return false;
            }
        }
        while(pos != string::npos);
    }
    else
    {
        file = absolute;
    }
    file += ".java";

    //
    // Open class file
    //
    if(!path.empty())
    {
        path += "/";
    }
    path += file;
    _out = new Output();
    _out->open(path.c_str());
    if(!(*_out))
    {
        cerr << "can't open `" << path << "' for writing: "
             << strerror(errno) << endl;
        close();
        return false;
    }

    printHeader();

    if(!package.empty())
    {
        *_out << sp << nl << "package " << package << ';';
    }

    return true;
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
    // Keyword list. *Must* be kept in alphabetical order.
    //
    static const string keywordList[] = 
    {       
        "abstract", "assert", "boolean", "break", "byte", "case", "catch",
        "char", "class", "clone", "const", "continue", "default", "do",
        "double", "else", "equals", "extends", "false", "final", "finalize",
	"finally", "float", "for", "getClass", "goto", "hashCode", "if",
	"implements", "import", "instanceof", "int", "interface", "long",
	"native", "new", "notify", "notifyAll", "null", "package", "private",
	"protected", "public", "return", "short", "static", "strictfp", "super", "switch",
        "synchronized", "this", "throw", "throws", "toString", "transient",
        "true", "try", "void", "volatile", "wait", "while"
    };
    bool found =  binary_search(&keywordList[0],
	                        &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
				name);
    return found ? "_" + name : name;
}
//
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
Slice::JavaGenerator::getAbsolute(const string& scoped,
                                  const string& scope,
                                  const string& prefix,
                                  const string& suffix) const
{
    string result;
    string::size_type start = 0;
    string fscoped = fixKwd(scoped);
    string fscope = fixKwd(scope);

    if(!fscope.empty())
    {
        //
        // Only remove the scope if the resulting symbol is unscoped.
        // For example:
        //
        // scope=::A, scoped=::A::B, result=B
        // scope=::A, scoped=::A::B::C, result=::A::B::C
        //
	string::size_type fscopeSize = fscope.size();
	if(fscoped.compare(0, fscopeSize, fscope) == 0)
	{
	    if(fscoped.size() > fscopeSize && fscoped[fscopeSize - 1] == ':' &&
               fscoped.find(':', fscopeSize) == string::npos)
	    {
		start = fscopeSize;
	    }
	}
    }

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
            fix = prefix + fixKwd(fscoped.substr(start)) + suffix;
        }
        else
        {
            assert(fscoped[pos + 1] == ':');
            fix = fixKwd(fscoped.substr(start, pos - start));
            start = pos + 2;
        }

        if(!result.empty())
        {
            result += ".";
        }
        result += fix;
    }
    while(pos != string::npos);

    if(!_package.empty())
    {
        return _package + "." + result;
    }
    else
    {
        return result;
    }
}

string
Slice::JavaGenerator::typeToString(const TypePtr& type,
                                   TypeMode mode,
                                   const string& scope,
                                   const list<string>& metaData) const
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
        "Ice.LocalObject"
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
        string result = getAbsolute(cl->scoped(), scope);
        if(mode == TypeModeOut)
        {
            result += "Holder";
        }
        return result;
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        string result = getAbsolute(proxy->_class()->scoped() + "Prx", scope);
        if(mode == TypeModeOut)
        {
            result += "Holder";
        }
        return result;
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        if(mode == TypeModeOut)
        {
            return getAbsolute(dict->scoped(), scope) + "Holder";
        }
        else
        {
            return "java.util.Map";
        }
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        if(mode == TypeModeOut)
        {
            return getAbsolute(seq->scoped(), scope) + "Holder";
        }
        else
        {
            string listType = findMetaData(metaData);
            if(listType.empty())
            {
                list<string> l = seq->getMetaData();
                listType = findMetaData(l);
            }
            if(!listType.empty())
            {
                return listType;
            }
            else
            {
                TypePtr content = seq->type();
                return typeToString(content, mode, scope) + "[]";
            }
        }
    }

    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
        if(mode == TypeModeOut)
        {
            return getAbsolute(contained->scoped(), scope) + "Holder";
        }
        else
        {
            return getAbsolute(contained->scoped(), scope);
        }
    }

    return "???";
}

void
Slice::JavaGenerator::writeMarshalUnmarshalCode(Output& out,
                                                const string& scope,
                                                const TypePtr& type,
                                                const string& param,
                                                bool marshal,
                                                int& iter,
                                                bool holder,
                                                const list<string>& metaData)
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
                if(marshal)
                {
                    out << nl << stream << ".writeObject(" << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readObject(\"\", null);";
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
        string typeS = typeToString(type, TypeModeIn, scope);
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
        if(marshal)
        {
            out << nl << stream << ".writeObject(" << v << ");";
        }
        else
        {
            string typeS = typeToString(type, TypeModeIn, scope);
            ClassDefPtr def = cl->definition();
            if(def && !def->isAbstract())
            {
                out << nl << v << " = (" << typeS << ')' << stream << ".readObject(" << typeS << ".ice_staticId(), "
                    << typeS << "._factory);";
            }
            else
            {
                out << nl << v << " = (" << typeS << ')' << stream << ".readObject(\"\", null);";
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
            string typeS = typeToString(type, TypeModeIn, scope);
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
            string typeS = typeToString(type, TypeModeIn, scope);
            out << nl << v << " = " << typeS << ".__read(" << stream << ");";
        }
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        writeSequenceMarshalUnmarshalCode(out, scope, seq, v, marshal, iter, true, metaData);
        return;
    }

    ConstructedPtr constructed = ConstructedPtr::dynamicCast(type);
    assert(constructed);
    string typeS = getAbsolute(constructed->scoped(), scope);
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
Slice::JavaGenerator::writeSequenceMarshalUnmarshalCode(Output& out,
                                                        const string& scope,
                                                        const SequencePtr& seq,
                                                        const string& param,
                                                        bool marshal,
                                                        int& iter,
                                                        bool useHelper,
                                                        const list<string>& metaData)
{
    string stream = marshal ? "__os" : "__is";
    string v = param;

    //
    // Check for metadata that overrides the default sequence
    // mapping. If no metadata is found, we use a regular Java
    // array. If metadata is found, the value of the metadata
    // must be the name of a class which implements the
    // java.util.List interface.
    //
    // There are two sources of metadata - that passed into
    // this function (which most likely comes from a data
    // member definition), and that associated with the type
    // itself. If data member metadata is found, and does
    // not match the type's metadata, then we cannot use
    // the type's Helper class for marshalling - we must
    // generate marshalling code inline.
    //
    string listType = findMetaData(metaData);
    list<string> typeMetaData = seq->getMetaData();
    if(listType.empty())
    {
        listType = findMetaData(typeMetaData);
    }
    else
    {
        string s = findMetaData(typeMetaData);
        if(listType != s)
        {
            useHelper = false;
        }
    }

    //
    // If we can use the sequence's helper, it's easy.
    //
    if(useHelper)
    {
        string typeS = getAbsolute(seq->scoped(), scope);
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
        depth++;
        origContent = s->type();
        s = SequencePtr::dynamicCast(origContent);
    }
    string origContentS = typeToString(origContent, TypeModeIn, scope);

    if(!listType.empty())
    {
        BuiltinPtr b = BuiltinPtr::dynamicCast(seq->type());
        if(b && b->kind() != Builtin::KindObject && b->kind() != Builtin::KindObjectProxy)
        {
            if(marshal)
            {
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
            }
            else
            {
                out << nl << v << " = new " << listType << "();";
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
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readByteSeq();";
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
            string typeS = getAbsolute(seq->scoped(), scope);
            if(marshal)
            {
                out << nl << stream << ".writeSize(" << v << ".size());";
                ostringstream o;
                o << "__i" << iter;
                iter++;
                string it = o.str();
                out << nl << "java.util.Iterator " << it << " = " << v << ".iterator();";
                out << nl << "while(" << it << ".hasNext())";
                out << sb;
                out << nl << origContentS << " __elem = (" << origContentS << ")" << it << ".next();";
                writeMarshalUnmarshalCode(out, scope, seq->type(), "__elem", true, iter, false);
                out << eb;
            }
            else
            {
                out << nl << v << " = new " << listType << "();";
                out << nl << "int __len" << iter << " = " << stream << ".readSize();";
                out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __len" << iter << "; __i" << iter
                    << "++)";
                out << sb;
                out << nl << origContentS << " __elem;";
                iter++;
                writeMarshalUnmarshalCode(out, scope, seq->type(), "__elem", false, iter, false);
                out << nl << v << ".add(__elem);";
                out << eb;
            }
        }
    }
    else
    {
        BuiltinPtr b = BuiltinPtr::dynamicCast(seq->type());
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
                out << nl << stream << ".writeSize(" << v << ".length);";
                out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < " << v << ".length; __i" << iter
                    << "++)";
                out << sb;
                ostringstream o;
                o << v << "[__i" << iter << "]";
                iter++;
                writeMarshalUnmarshalCode(out, scope, seq->type(), o.str(), true, iter, false);
                out << eb;
            }
            else
            {
                out << nl << "int __len" << iter << " = " << stream << ".readSize();";
                out << nl << v << " = new " << origContentS << "[__len" << iter << "]";
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
                iter++;
                writeMarshalUnmarshalCode(out, scope, seq->type(), o.str(), false, iter, false);
                out << eb;
            }
        }
    }
}

void
Slice::JavaGenerator::writeGenericMarshalUnmarshalCode(Output& out,
                                                       const string& scope,
                                                       const TypePtr& type,
                                                       const string& tn,
                                                       const string& param,
                                                       bool marshal,
                                                       int& iter,
                                                       bool holder,
                                                       const list<string>& metaData)
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

    string name;
    if(tn.empty())
    {
        name = "\"" + param + "\"";
    }
    else
    {
        name = tn;
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
                    out << nl << stream << ".writeByte(" << name << ", " << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readByte(" << name << ");";
                }
                break;
            }
            case Builtin::KindBool:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeBool(" << name << ", " << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readBool(" << name << ");";
                }
                break;
            }
            case Builtin::KindShort:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeShort(" << name << ", " << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readShort(" << name << ");";
                }
                break;
            }
            case Builtin::KindInt:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeInt(" << name << ", " << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readInt(" << name << ");";
                }
                break;
            }
            case Builtin::KindLong:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeLong(" << name << ", " << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readLong(" << name << ");";
                }
                break;
            }
            case Builtin::KindFloat:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeFloat(" << name << ", " << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readFloat(" << name << ");";
                }
                break;
            }
            case Builtin::KindDouble:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeDouble(" << name << ", " << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readDouble(" << name << ");";
                }
                break;
            }
            case Builtin::KindString:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeString(" << name << ", " << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readString(" << name << ");";
                }
                break;
            }
            case Builtin::KindObject:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeObject(" << name << ", " << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readObject(" << name << ", \"\", null);";
                }
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeProxy(" << name << ", " << v << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readProxy(" << name << ");";
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
        string typeS = typeToString(type, TypeModeIn, scope);
        if(marshal)
        {
            out << nl << typeS << "Helper.ice_marshal(" << name << ", " << stream << ", " << v << ");";
        }
        else
        {
            out << nl << v << " = " << typeS << "Helper.ice_unmarshal(" << name << ", " << stream << ");";
        }
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(marshal)
        {
            out << nl << stream << ".writeObject(" << name << ", " << v << ");";
        }
        else
        {
            string typeS = typeToString(type, TypeModeIn, scope);
            ClassDefPtr def = cl->definition();
            if(def && !def->isAbstract())
            {
                out << nl << v << " = (" << typeS << ')' << stream << ".readObject(" << name << ", " << typeS
                    << ".ice_staticId(), " << typeS << "._factory);";
            }
            else
            {
                out << nl << v << " = (" << typeS << ')' << stream << ".readObject(" << name << ", \"\", null);";
            }
        }
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(marshal)
        {
            out << nl << v << ".ice_marshal(" << name << ", " << stream << ");";
        }
        else
        {
            string typeS = typeToString(type, TypeModeIn, scope);
            out << nl << v << " = new " << typeS << "();";
            out << nl << v << ".ice_unmarshal(" << name << ", " << stream << ");";
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        if(marshal)
        {
            out << nl << v << ".ice_marshal(" << name << ", " << stream << ");";
        }
        else
        {
            string typeS = typeToString(type, TypeModeIn, scope);
            out << nl << v << " = " << typeS << ".ice_unmarshal(" << name << ", " << stream << ");";
        }
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        writeGenericSequenceMarshalUnmarshalCode(out, scope, seq, name, v, marshal, iter, true, metaData);
        return;
    }

    ConstructedPtr constructed = ConstructedPtr::dynamicCast(type);
    assert(constructed);
    string typeS = getAbsolute(constructed->scoped(), scope);
    if(marshal)
    {
        out << nl << typeS << "Helper.ice_marshal(" << name << ", " << stream << ", " << v << ");";
    }
    else
    {
        out << nl << v << " = " << typeS << "Helper.ice_unmarshal(" << name << ", " << stream << ");";
    }
}

void
Slice::JavaGenerator::writeGenericSequenceMarshalUnmarshalCode(Output& out,
                                                               const string& scope,
                                                               const SequencePtr& seq,
                                                               const string& name,
                                                               const string& param,
                                                               bool marshal,
                                                               int& iter,
                                                               bool useHelper,
                                                               const list<string>& metaData)
{
    string stream = marshal ? "__os" : "__is";
    string v = param;

    //
    // Check for metadata that overrides the default sequence
    // mapping. If no metadata is found, we use a regular Java
    // array. If metadata is found, the value of the metadata
    // must be the name of a class which implements the
    // java.util.List interface.
    //
    // There are two sources of metadata - that passed into
    // this function (which most likely comes from a data
    // member definition), and that associated with the type
    // itself. If data member metadata is found, and does
    // not match the type's metadata, then we cannot use
    // the type's Helper class for marshalling - we must
    // generate marshalling code inline.
    //
    string listType = findMetaData(metaData);
    list<string> typeMetaData = seq->getMetaData();
    if(listType.empty())
    {
        listType = findMetaData(typeMetaData);
    }
    else
    {
        string s = findMetaData(typeMetaData);
        if(listType != s)
        {
            useHelper = false;
        }
    }

    //
    // If we can use the sequence's helper, it's easy.
    //
    if(useHelper)
    {
        string typeS = getAbsolute(seq->scoped(), scope);
        if(marshal)
        {
            out << nl << typeS << "Helper.ice_marshal(" << name << ", " << stream << ", " << v << ");";
        }
        else
        {
            out << nl << v << " = " << typeS << "Helper.ice_unmarshal(" << name << ", " << stream << ");";
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
        depth++;
        origContent = s->type();
        s = SequencePtr::dynamicCast(origContent);
    }
    string origContentS = typeToString(origContent, TypeModeIn, scope);

    if(!listType.empty())
    {
        BuiltinPtr b = BuiltinPtr::dynamicCast(seq->type());
        if(b && b->kind() != Builtin::KindObject && b->kind() != Builtin::KindObjectProxy)
        {
            if(marshal)
            {
                out << nl << stream << ".startWriteSequence(" << name << ", " << v << ".size());";
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
                        out << nl << stream << ".writeByte(\"e\", __elem.byteValue());";
                        break;
                    }
                    case Builtin::KindBool:
                    {
                        out << nl << "java.lang.Boolean __elem = (java.lang.Boolean)" << it << ".next();";
                        out << nl << stream << ".writeBool(\"e\", __elem.booleanValue());";
                        break;
                    }
                    case Builtin::KindShort:
                    {
                        out << nl << "java.lang.Short __elem = (java.lang.Short)" << it << ".next();";
                        out << nl << stream << ".writeShort(\"e\", __elem.shortValue());";
                        break;
                    }
                    case Builtin::KindInt:
                    {
                        out << nl << "java.lang.Integer __elem = (java.lang.Integer)" << it << ".next();";
                        out << nl << stream << ".writeInt(\"e\", __elem.intValue());";
                        break;
                    }
                    case Builtin::KindLong:
                    {
                        out << nl << "java.lang.Long __elem = (java.lang.Long)" << it << ".next();";
                        out << nl << stream << ".writeLong(\"e\", __elem.longValue());";
                        break;
                    }
                    case Builtin::KindFloat:
                    {
                        out << nl << "java.lang.Float __elem = (java.lang.Float)" << it << ".next();";
                        out << nl << stream << ".writeFloat(\"e\", __elem.floatValue());";
                        break;
                    }
                    case Builtin::KindDouble:
                    {
                        out << nl << "java.lang.Double __elem = (java.lang.Double)" << it << ".next();";
                        out << nl << stream << ".writeDouble(\"e\", __elem.doubleValue());";
                        break;
                    }
                    case Builtin::KindString:
                    {
                        out << nl << "java.lang.String __elem = (java.lang.String)" << it << ".next();";
                        out << nl << stream << ".writeString(\"e\", __elem);";
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
                out << nl << stream << ".endWriteSequence();";
            }
            else
            {
                out << nl << v << " = new " << listType << "();";
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
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readByteSeq(" << name << ");";
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
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readByteSeq(" << name << ");";
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
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readShortSeq(" << name << ");";
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
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readIntSeq(" << name << ");";
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
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readLongSeq(" << name << ");";
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
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readFloatSeq(" << name << ");";
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
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readDoubleSeq(" << name
                            << ");";
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
                        out << nl << o.str() << " __seq" << iter << " = " << stream << ".readStringSeq(" << name
                            << ");";
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
            string typeS = getAbsolute(seq->scoped(), scope);
            if(marshal)
            {
                out << nl << stream << ".startWriteSequence(" << name << ", " << v << ".size());";
                ostringstream o;
                o << "__i" << iter;
                iter++;
                string it = o.str();
                out << nl << "java.util.Iterator " << it << " = " << v << ".iterator();";
                out << nl << "while(" << it << ".hasNext())";
                out << sb;
                out << nl << origContentS << " __elem = (" << origContentS << ")" << it << ".next();";
                writeGenericMarshalUnmarshalCode(out, scope, seq->type(), "\"e\"", "__elem", true, iter, false);
                out << eb;
                out << nl << stream << ".endWriteSequence();";
            }
            else
            {
                out << nl << v << " = new " << listType << "();";
                out << nl << "int __len" << iter << " = " << stream << ".startReadSequence(" << name << ");";
                out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < __len" << iter << "; __i" << iter
                    << "++)";
                out << sb;
                out << nl << origContentS << " __elem;";
                iter++;
                writeGenericMarshalUnmarshalCode(out, scope, seq->type(), "\"e\"", "__elem", false, iter, false);
                out << nl << v << ".add(__elem);";
                out << eb;
            }
        }
    }
    else
    {
        BuiltinPtr b = BuiltinPtr::dynamicCast(seq->type());
        if(b && b->kind() != Builtin::KindObject && b->kind() != Builtin::KindObjectProxy)
        {
            switch(b->kind())
            {
                case Builtin::KindByte:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeByteSeq(" << name << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readByteSeq(" << name << ");";
                    }
                    break;
                }
                case Builtin::KindBool:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeBoolSeq(" << name << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readBoolSeq(" << name << ");";
                    }
                    break;
                }
                case Builtin::KindShort:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeShortSeq(" << name << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readShortSeq(" << name << ");";
                    }
                    break;
                }
                case Builtin::KindInt:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeIntSeq(" << name << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readIntSeq(" << name << ");";
                    }
                    break;
                }
                case Builtin::KindLong:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeLongSeq(" << name << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readLongSeq(" << name << ");";
                    }
                    break;
                }
                case Builtin::KindFloat:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeFloatSeq(" << name << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readFloatSeq(" << name << ");";
                    }
                    break;
                }
                case Builtin::KindDouble:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeDoubleSeq(" << name << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readDoubleSeq(" << name << ");";
                    }
                    break;
                }
                case Builtin::KindString:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeStringSeq(" << name << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readStringSeq(" << name << ");";
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
            string typeS = getAbsolute(seq->scoped(), scope);
            if(marshal)
            {
                out << nl << stream << ".startWriteSequence(" << name << ", " << v << ".length);";
                out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < " << v << ".length; __i" << iter
                    << "++)";
                out << sb;
                ostringstream o;
                o << v << "[__i" << iter << "]";
                iter++;
                writeGenericMarshalUnmarshalCode(out, scope, seq->type(), "\"e\"", o.str(), true, iter, false);
                out << eb;
                out << nl << stream << ".endWriteSequence();";
            }
            else
            {
                out << nl << "int __len" << iter << " = " << stream << ".startReadSequence(" << name << ");";
                out << nl << v << " = new " << origContentS << "[__len" << iter << "]";
                int d = depth;
                while(d--)
                {
                    out << "[]";
                }
                out << ';';
                out << nl << "for(int __i" << iter << " = 0; __i" << iter << " < " << v << ".length; __i" << iter
                    << "++)";
                out << sb;
                ostringstream o;
                o << v << "[__i" << iter << "]";
                iter++;
                writeGenericMarshalUnmarshalCode(out, scope, seq->type(), "\"e\"", o.str(), false, iter, false);
                out << eb;
                out << nl << stream << ".endReadSequence();";
            }
        }
    }
}

void
Slice::JavaGenerator::printHeader()
{
    static const char* header =
"// **********************************************************************\n"
"//\n"
"// Copyright (c) 2002\n"
"// Mutable Realms, Inc.\n"
"// Huntsville, AL, USA\n"
"//\n"
"// All Rights Reserved\n"
"//\n"
"// **********************************************************************\n"
        ;

    Output& out = output();
    out << header;
    out << "\n// Ice version " << ICE_STRING_VERSION;
}

string
Slice::JavaGenerator::findMetaData(const list<string>& metaData)
{
    static const string prefix = "java:";
    for(list<string>::const_iterator q = metaData.begin(); q != metaData.end(); ++q)
    {
        if((*q).find(prefix) == 0)
        {
            return (*q).substr(prefix.size());
        }
    }

    return "";
}
