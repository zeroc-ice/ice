// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
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

#ifdef WIN32
#include <direct.h>
#endif

#ifndef WIN32
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
    if (pos != string::npos)
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
            if (!path.empty())
            {
                path += "/";
            }
            pos = dir.find('.', start);
            if (pos != string::npos)
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
            if (result == 0)
            {
                continue;
            }
#ifdef WIN32
            result = _mkdir(path.c_str());
#else       
            result = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
#endif
            if (result != 0)
            {
                cerr << "can't create directory `" << path << "': "
                     << strerror(errno) << endl;
                return false;
            }
        }
        while (pos != string::npos);
    }
    else
    {
        file = absolute;
    }
    file += ".java";

    //
    // Open class file
    //
    if (!path.empty())
    {
        path += "/";
    }
    path += file;
    _out = new Output();
    _out->open(path.c_str());
    if (!(*_out))
    {
        cerr << "can't open `" << path << "' for writing: "
             << strerror(errno) << endl;
        close();
        return false;
    }

    printHeader();

    if (!package.empty())
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

string
Slice::JavaGenerator::fixKwd(const string& name) const
{
    //
    // Alphabetical list of Java keywords
    //
    static const char* keywords[] = 
    {       
        "abstract", "assert", "boolean", "break", "byte", "case", "catch",
        "char", "class", "clone", "const", "continue", "default", "do",
        "double", "else", "equals", "extends", "false", "final", "finalize",
        "finally", "float", "for", "getClass", "goto", "hashCode", "if",
        "implements", "import", "instanceof", "int", "interface", "long",
        "native", "new", "notify", "notifyAll", "null", "package", "private",
        "protected", "public", "return", "short", "static", "super", "switch",
        "synchronized", "this", "throw", "throws", "toString", "transient",
        "true", "try", "void", "volatile", "wait", "while"
    };

    int i = 0;
    int j = sizeof(keywords) / sizeof(const char*);

    while (i < j)
    {
        int mid = (i + j) / 2;
        string str = keywords[mid];
        int n = str.compare(name);
        if (n == 0)
        {
            string result = "_" + name;
            return result;
        }
        else if (n > 0)
        {
            j = mid;
        }
        else
        {
            i = mid + 1;
        }
    }

    return name;
}

string
Slice::JavaGenerator::getAbsolute(const string& scoped,
                                  const string& scope,
                                  const string& prefix,
                                  const string& suffix) const
{
    string result;
    string::size_type start = 0;

    if (!scope.empty())
    {
        //
        // Only remove the scope if the resulting symbol is unscoped.
        // For example:
        //
        // scope=::A, scoped=::A::B, result=B
        // scope=::A, scoped=::A::B::C, result=::A::B::C
        //
        string::size_type scopeSize = scope.size();
        if (scoped.compare(0, scopeSize, scope) == 0)
        {
            start = scoped.find(':', scopeSize);
            if (start == string::npos)
            {
                start = scopeSize;
            }
            else
            {
                start = 0;
            }
        }
    }

    //
    // Skip leading "::"
    //
    if (scoped[start] == ':')
    {
        assert(scoped[start + 1] == ':');
        start += 2;
    }

    //
    // Convert all occurrences of "::" to "."
    //
    string::size_type pos;
    do
    {
        pos = scoped.find(':', start);
        string fix;
        if (pos == string::npos)
        {
            fix = prefix + fixKwd(scoped.substr(start)) + suffix;
        }
        else
        {
            assert(scoped[pos + 1] == ':');
            fix = fixKwd(scoped.substr(start, pos - start));
            start = pos + 2;
        }

        if (!result.empty())
        {
            result += ".";
        }
        result += fix;
    }
    while (pos != string::npos);

    if (!_package.empty())
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
                                   const string& scope) const
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

    if (!type)
    {
        assert(mode == TypeModeReturn);
        return "void";
    }

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if (builtin)
    {
        if (mode == TypeModeOut)
        {
            return builtinHolderTable[builtin->kind()];
        }
        else
        {
            return builtinTable[builtin->kind()];
        }
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if (cl)
    {
        string result = getAbsolute(cl->scoped(), scope);
        if (mode == TypeModeOut)
        {
            result += "Holder";
        }
        return result;
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if (proxy)
    {
        string result = getAbsolute(proxy->_class()->scoped() + "Prx", scope);
        if (mode == TypeModeOut)
        {
            result += "Holder";
        }
        return result;
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if (dict)
    {
        if (mode == TypeModeOut)
        {
            return getAbsolute(dict->scoped(), scope) + "Holder";
        }
        else
        {
            return "java.util.Map";
        }
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if (seq)
    {
        if (mode == TypeModeOut)
        {
            return getAbsolute(seq->scoped(), scope) + "Holder";
        }
        else
        {
            TypePtr content = seq->type();
            return typeToString(content, mode, scope) + "[]";
        }
    }

    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if (contained)
    {
        if (mode == TypeModeOut)
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
                                                bool holder)
{
    string stream = marshal ? "__os" : "__is";
    string v;
    if (holder)
    {
        v = param + ".value";
    }
    else
    {
        v = param;
    }

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindByte:
            {
                if (marshal)
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
                if (marshal)
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
                if (marshal)
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
                if (marshal)
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
                if (marshal)
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
                if (marshal)
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
                if (marshal)
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
                if (marshal)
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
                if (marshal)
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
                if (marshal)
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
    if (prx)
    {
        string typeS = typeToString(type, TypeModeIn, scope);
        if (marshal)
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
    if (cl)
    {
        if (marshal)
        {
            out << nl << stream << ".writeObject(" << v << ");";
        }
        else
        {
            string typeS = typeToString(type, TypeModeIn, scope);
            ClassDefPtr def = cl->definition();
            if (def && !def->isAbstract())
            {
                out << nl << v << " = (" << typeS << ')' << stream << ".readObject(" << typeS << ".__classIds[0], "
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
    if (st)
    {
        if (marshal)
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
    if (en)
    {
        if (marshal)
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
    if (seq)
    {
        BuiltinPtr b = BuiltinPtr::dynamicCast(seq->type());
        if (b)
        {
            switch (b->kind())
            {
                case Builtin::KindByte:
                {
                    if (marshal)
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
                    if (marshal)
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
                    if (marshal)
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
                    if (marshal)
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
                    if (marshal)
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
                    if (marshal)
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
                    if (marshal)
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
                    if (marshal)
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
                {
                    string typeS = getAbsolute(seq->scoped(), scope);
                    if (marshal)
                    {
                        out << nl << typeS << "Helper.write(" << stream << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << typeS << "Helper.read(" << stream << ");";
                    }
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
            ContainedPtr cont = ContainedPtr::dynamicCast(type);
            assert(cont);
            string typeS = getAbsolute(cont->scoped(), scope);
            if (marshal)
            {
                out << nl << typeS << "Helper.write(" << stream << ", " << v << ");";
            }
            else
            {
                out << nl << v << " = " << typeS << "Helper.read(" << stream << ");";
            }
        }
        return;
    }

    ConstructedPtr constructed = ConstructedPtr::dynamicCast(type);
    assert(constructed);
    string typeS = getAbsolute(constructed->scoped(), scope);
    if (marshal)
    {
        out << nl << typeS << "Helper.write(" << stream << ", " << v << ");";
    }
    else
    {
        out << nl << v << " = " << typeS << "Helper.read(" << stream << ");";
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
                                                       bool holder)
{
    string stream = marshal ? "__os" : "__is";
    string v;
    if (holder)
    {
        v = param + ".value";
    }
    else
    {
        v = param;
    }

    string name;
    if (tn.empty())
    {
        name = "\"" + param + "\"";
    }
    else
    {
        name = tn;
    }

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindByte:
            {
                if (marshal)
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
                if (marshal)
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
                if (marshal)
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
                if (marshal)
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
                if (marshal)
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
                if (marshal)
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
                if (marshal)
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
                if (marshal)
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
                if (marshal)
                {
                    out << nl << stream << ".writeObject(" << name << ", " << v << ");";
                }
                else
                {
                    out << nl << "v = " << stream << ".readObject(" << name << ", \"\", null);";
                }
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if (marshal)
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
    if (prx)
    {
        string typeS = typeToString(type, TypeModeIn, scope);
        if (marshal)
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
    if (cl)
    {
        if (marshal)
        {
            out << nl << stream << ".writeObject(" << name << ", " << v << ");";
        }
        else
        {
            string typeS = typeToString(type, TypeModeIn, scope);
            ClassDefPtr def = cl->definition();
            if (def && !def->isAbstract())
            {
                out << nl << v << " = (" << typeS << ')' << stream << ".readObject(" << name << ", " << typeS
                    << ".__classIds[0], " << typeS << "._factory);";
            }
            else
            {
                out << nl << v << " = (" << typeS << ')' << stream << ".readObject(" << name << ", \"\", null);";
            }
        }
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if (st)
    {
        if (marshal)
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
    if (en)
    {
        if (marshal)
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
    if (seq)
    {
        BuiltinPtr b = BuiltinPtr::dynamicCast(seq->type());
        if (b)
        {
            switch (b->kind())
            {
                case Builtin::KindByte:
                {
                    if (marshal)
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
                    if (marshal)
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
                    if (marshal)
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
                    if (marshal)
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
                    if (marshal)
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
                    if (marshal)
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
                    if (marshal)
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
                    if (marshal)
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
                {
                    string typeS = getAbsolute(seq->scoped(), scope);
                    if (marshal)
                    {
                        out << nl << typeS << "Helper.ice_marshal(" << name << ", " << stream << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << typeS << "Helper.ice_unmarshal(" << name << ", " << stream << ");";
                    }
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
            ContainedPtr cont = ContainedPtr::dynamicCast(type);
            assert(cont);
            string typeS = getAbsolute(cont->scoped(), scope);
            if (marshal)
            {
                out << nl << typeS << "Helper.ice_marshal(" << name << ", " << stream << ", " << v << ");";
            }
            else
            {
                out << nl << v << " = " << typeS << "Helper.ice_unmarshal(" << name << ", " << stream << ");";
            }
        }
        return;
    }

    ConstructedPtr constructed = ConstructedPtr::dynamicCast(type);
    assert(constructed);
    string typeS = getAbsolute(constructed->scoped(), scope);
    if (marshal)
    {
        out << nl << typeS << "Helper.ice_marshal(" << name << ", " << stream << ", " << v << ");";
    }
    else
    {
        out << nl << v << " = " << typeS << "Helper.ice_unmarshal(" << name << ", " << stream << ");";
    }
}

void
Slice::JavaGenerator::printHeader()
{
    static const char* header =
"// **********************************************************************\n"
"//\n"
"// Copyright (c) 2002\n"
"// MutableRealms, Inc.\n"
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
