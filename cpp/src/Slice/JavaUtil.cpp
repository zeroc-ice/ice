// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
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
            sp();
            nl();
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
"// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.\n"
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

Slice::JavaGenerator::JavaGenerator(const string& dir) :
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

    JavaOutput* out = new JavaOutput;
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
        "double", "else", "equals", "extends", "false", "final", "finalize",
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
                                   const StringList& metaData) const
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
            return getAbsolute(dict, package, "", "Holder");
        }
        else
        {
            return "java.util.Map";
        }
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        string listType = findMetaData(metaData);
        if(mode == TypeModeOut)
        {
            if(listType.empty())
            {
                return getAbsolute(seq, package, "", "Holder");
            }
            else if(listType == "java.util.ArrayList")
            {
                return "Ice.ArrayListHolder";
            }
            else if(listType == "java.util.LinkedList")
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
            if(listType.empty())
            {
                StringList l = seq->getMetaData();
                listType = findMetaData(l);
            }
            if(!listType.empty())
            {
                return listType;
            }
            else
            {
                return typeToString(seq->type(), mode, package) + "[]";
            }
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
    StringList typeMetaData = seq->getMetaData();
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
        // Stop if the inner sequence type has metadata.
        //
        string m = findMetaData(s->getMetaData());
        if(!m.empty())
        {
            break;
        }
        depth++;
        origContent = s->type();
        s = SequencePtr::dynamicCast(origContent);
    }
    string origContentS = typeToString(origContent, TypeModeIn, package);

    TypePtr type = seq->type();

    if(!listType.empty())
    {
        //
        // Marshal/unmarshal a custom sequence type
        //
        BuiltinPtr b = BuiltinPtr::dynamicCast(type);
        if(b && b->kind() != Builtin::KindObject && b->kind() != Builtin::KindObjectProxy)
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
                iter++;
                string it = o.str();
                out << nl << "java.util.Iterator " << it << " = " << v << ".iterator();";
                out << nl << "while(" << it << ".hasNext())";
                out << sb;
                out << nl << origContentS << " __elem = (" << origContentS << ")" << it << ".next();";
                writeMarshalUnmarshalCode(out, package, type, "__elem", true, iter, false);
                out << eb; // while
                out << eb; // else
            }
            else
            {
                bool isObject = false;
		BuiltinPtr builtin = BuiltinPtr::dynamicCast(origContent);
                ClassDeclPtr cl = ClassDeclPtr::dynamicCast(origContent);
		if((builtin && builtin->kind() == Builtin::KindObject) || cl)
                {
                    isObject = true;
                }
                out << nl << v << " = new " << listType << "();";
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
                    if(builtin)
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
		    out << nl << origContentS << " __elem;";
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
		    if(!SequencePtr::dynamicCast(seq->type()))
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
    string stream = marshal ? "__out" : "__in";
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
Slice::JavaGenerator::writeStreamSequenceMarshalUnmarshalCode(Output& out,
                                                              const string& package,
                                                              const SequencePtr& seq,
                                                              const string& param,
                                                              bool marshal,
                                                              int& iter,
                                                              bool useHelper,
                                                              const StringList& metaData)
{
    string stream = marshal ? "__out" : "__in";
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
    StringList typeMetaData = seq->getMetaData();
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
        // Stop if the inner sequence type has metadata.
        //
        string m = findMetaData(s->getMetaData());
        if(!m.empty())
        {
            break;
        }
        depth++;
        origContent = s->type();
        s = SequencePtr::dynamicCast(origContent);
    }
    string origContentS = typeToString(origContent, TypeModeIn, package);

    if(!listType.empty())
    {
        //
        // Marshal/unmarshal a custom sequence type
        //
        BuiltinPtr b = BuiltinPtr::dynamicCast(seq->type());
        if(b && b->kind() != Builtin::KindObject && b->kind() != Builtin::KindObjectProxy)
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
                iter++;
                string it = o.str();
                out << nl << "java.util.Iterator " << it << " = " << v << ".iterator();";
                out << nl << "while(" << it << ".hasNext())";
                out << sb;
                out << nl << origContentS << " __elem = (" << origContentS << ")" << it << ".next();";
                writeStreamMarshalUnmarshalCode(out, package, seq->type(), "__elem", true, iter, false);
                out << eb; // while
                out << eb; // else
            }
            else
            {
                bool isObject = false;
                BuiltinPtr builtin = BuiltinPtr::dynamicCast(origContent);
                ClassDeclPtr cl = ClassDeclPtr::dynamicCast(origContent);
                if((builtin && builtin->kind() == Builtin::KindObject) || cl)
                {
                    isObject = true;
                }
                out << nl << v << " = new " << listType << "();";
                out << nl << "final int __len" << iter << " = " << stream << ".readSize();";
                if(isObject)
                {
                    if(builtin)
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
                    writeStreamMarshalUnmarshalCode(out, package, seq->type(), "__elem", false, iter, false,
                                                    StringList(), patchParams.str());
                }
                else
                {
                    out << nl << origContentS << " __elem;";
                    writeStreamMarshalUnmarshalCode(out, package, seq->type(), "__elem", false, iter, false);
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
                writeStreamMarshalUnmarshalCode(out, package, seq->type(), o.str(), true, iter, false);
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
                ostringstream patchParams;
                if(isObject)
                {
                    patchParams << "new IceInternal.SequencePatcher(" << v << ", " << origContentS
                                << ".class, __type" << iter << ", __i" << iter << ')';
                    writeStreamMarshalUnmarshalCode(out, package, seq->type(), o.str(), false, iter, false,
                                                    StringList(), patchParams.str());
                }
                else
                {
                    writeStreamMarshalUnmarshalCode(out, package, seq->type(), o.str(), false, iter, false);
                }
                out << eb;
                iter++;
            }
        }
    }
}

string
Slice::JavaGenerator::findMetaData(const StringList& metaData)
{
    static const string prefix = "java:";
    for(StringList::const_iterator q = metaData.begin(); q != metaData.end(); ++q)
    {
        string str = *q;
        if(str.find(prefix) == 0)
        {
            string::size_type pos = str.find(':', prefix.size());
            if(pos != string::npos)
            {
                //
                // Correct syntax is "java:type:java.util.LinkedList".
                //
                if(str.substr(prefix.size(), pos - prefix.size()) == "type")
                {
                    return str.substr(pos + 1);
                }
            }
            else
            {
                //
                // Deprecated "java:java.util.LinkedList" syntax.
                //
                return str.substr(prefix.size());
            }
        }
    }

    return "";
}

void
Slice::JavaGenerator::validateMetaData(const UnitPtr& unit)
{
    MetaDataVisitor visitor;
    unit->visit(&visitor, true);
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
                static const string packagePrefix = "java:package:";
                if(s.find(packagePrefix) != 0 || s.size() == packagePrefix.size())
                {
                    cout << file << ": warning: ignoring invalid global metadata `" << s << "'" << endl;
                }
            }
            _history.insert(s);
        }
    }

    StringList metaData = getMetaData(p);
    validate(p, metaData, p->definitionContext()->filename(), p->line());
    return true;
}

void
Slice::JavaGenerator::MetaDataVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    StringList metaData = getMetaData(p);
    validate(p, metaData, p->definitionContext()->filename(), p->line());
}

bool
Slice::JavaGenerator::MetaDataVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    StringList metaData = getMetaData(p);
    validate(p, metaData, p->definitionContext()->filename(), p->line());
    return true;
}

bool
Slice::JavaGenerator::MetaDataVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    StringList metaData = getMetaData(p);
    validate(p, metaData, p->definitionContext()->filename(), p->line());
    return true;
}

bool
Slice::JavaGenerator::MetaDataVisitor::visitStructStart(const StructPtr& p)
{
    StringList metaData = getMetaData(p);
    validate(p, metaData, p->definitionContext()->filename(), p->line());
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
	    cout << p->definitionContext()->filename() << ":" << p->line()
		 << ": warning: invalid metadata for operation" << endl;
	}
	else
	{
	    validate(returnType, metaData, p->definitionContext()->filename(), p->line());
	}
    }

    ParamDeclList params = p->parameters();
    for(ParamDeclList::iterator q = params.begin(); q != params.end(); ++q)
    {
	metaData = getMetaData(*q);
	validate((*q)->type(), metaData, p->definitionContext()->filename(), (*q)->line());
    }
}

void
Slice::JavaGenerator::MetaDataVisitor::visitDataMember(const DataMemberPtr& p)
{
    StringList metaData = getMetaData(p);
    validate(p->type(), metaData, p->definitionContext()->filename(), p->line());
}

void
Slice::JavaGenerator::MetaDataVisitor::visitSequence(const SequencePtr& p)
{
    StringList metaData = getMetaData(p);
    validate(p, metaData, p->definitionContext()->filename(), p->line());
}

void
Slice::JavaGenerator::MetaDataVisitor::visitDictionary(const DictionaryPtr& p)
{
    StringList metaData = getMetaData(p);
    validate(p, metaData, p->definitionContext()->filename(), p->line());
}

void
Slice::JavaGenerator::MetaDataVisitor::visitEnum(const EnumPtr& p)
{
    StringList metaData = getMetaData(p);
    validate(p, metaData, p->definitionContext()->filename(), p->line());
}

void
Slice::JavaGenerator::MetaDataVisitor::visitConst(const ConstPtr& p)
{
    StringList metaData = getMetaData(p);
    validate(p, metaData, p->definitionContext()->filename(), p->line());
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
			cout << file << ":" << cont->line() << ": warning: metadata `" << s
			     << "' uses deprecated syntax" << endl;
			//
			// Translate java:X into java:type:X.
			//
			result.push_back(prefix + "type:" + s.substr(prefix.size()));
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
Slice::JavaGenerator::MetaDataVisitor::validate(const SyntaxTreeBasePtr& p, const StringList& metaData,
						const string& file, const string& line)
{
    //
    // Currently only sequence types can be affected by metadata.
    //
    if(!metaData.empty() && !SequencePtr::dynamicCast(p))
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
