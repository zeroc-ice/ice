// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/Functional.h>
#include <Gen.h>
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

Slice::JavaVisitor::JavaVisitor(const string& dir, const string& package) :
    _dir(dir),
    _package(package),
    _out(0)
{
}

Slice::JavaVisitor::~JavaVisitor()
{
    assert(_out == 0);
}

bool
Slice::JavaVisitor::open(const string& absolute)
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
Slice::JavaVisitor::close()
{
    assert(_out != 0);
    *_out << nl;
    delete _out;
    _out = 0;
}

Output&
Slice::JavaVisitor::output() const
{
    assert(_out != 0);
    return *_out;
}

string
Slice::JavaVisitor::fixKwd(const string& name) const
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
Slice::JavaVisitor::getAbsolute(const string& scoped,
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
Slice::JavaVisitor::typeToString(const TypePtr& type, TypeMode mode,
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

string
Slice::JavaVisitor::getParams(const OperationPtr& op, const string& scope)
{
    TypeStringList inParams = op->inputParameters();
    TypeStringList outParams = op->outputParameters();
    TypeStringList::const_iterator q;

    string params;

    for (q = inParams.begin(); q != inParams.end(); ++q)
    {
        if (q != inParams.begin())
        {
            params += ", ";
        }

        string typeString = typeToString(q->first, TypeModeIn, scope);
        params += typeString;
        params += ' ';
        params += fixKwd(q->second);
    }

    for (q = outParams.begin(); q != outParams.end(); ++q)
    {
        if (q != outParams.begin() || !inParams.empty())
        {
            params += ", ";
        }

        string typeString = typeToString(q->first, TypeModeOut, scope);
        params += typeString;
        params += ' ';
        params += fixKwd(q->second);
    }

    return params;
}

string
Slice::JavaVisitor::getArgs(const OperationPtr& op, const string& scope)
{
    TypeStringList inParams = op->inputParameters();
    TypeStringList outParams = op->outputParameters();
    TypeStringList::const_iterator q;

    string args;

    for (q = inParams.begin(); q != inParams.end(); ++q)
    {
        if (q != inParams.begin())
        {
            args += ", ";
        }

        args += fixKwd(q->second);
    }

    for (q = outParams.begin(); q != outParams.end(); ++q)
    {
        if (q != outParams.begin() || !inParams.empty())
        {
            args += ", ";
        }

        args += fixKwd(q->second);
    }

    return args;
}

static bool
exceptionIsLocal(const ::Slice::ExceptionPtr exception)
{
    return exception->isLocal();
}

void
Slice::JavaVisitor::writeThrowsClause(const string& scope,
                                      const ExceptionList& throws)
{
    //
    // Don't include local exceptions in the throws clause
    //
    ExceptionList::size_type localCount = 0;

    //
    // MSVC gets confused if
    // ::IceUtil::memFun(&::Slice::Exception::isLocal)); is used hence
    // the exceptionIsLocal function.
    //
    localCount = count_if(throws.begin(), throws.end(),	exceptionIsLocal);

    Output& out = output();
    if (throws.size() - localCount > 0)
    {
        out.inc();
        out << nl;
        out << "throws ";
        out.useCurrentPosAsIndent();
        ExceptionList::const_iterator r;
        int count = 0;
        for (r = throws.begin(); r != throws.end(); ++r)
        {
            if (!(*r)->isLocal())
            {
                if (count > 0)
                {
                    out << "," << nl;
                }
                out << getAbsolute((*r)->scoped(), scope);
                count++;
            }
        }
        out.restoreIndent();
        out.dec();
    }
}

void
Slice::JavaVisitor::writeDelegateThrowsClause(const string& scope,
                                              const ExceptionList& throws)
{
    Output& out = output();
    out.inc();
    out << nl;
    out << "throws ";
    out.useCurrentPosAsIndent();
    out << "Ice.LocationForward,";
    out << nl << "IceInternal.NonRepeatable";

    //
    // Don't include local exceptions in the throws clause
    //
    ExceptionList::const_iterator r;
    for (r = throws.begin(); r != throws.end(); ++r)
    {
        if (!(*r)->isLocal())
        {
            out << "," << nl;
            out << getAbsolute((*r)->scoped(), scope);
        }
    }
    out.restoreIndent();
    out.dec();
}

void
Slice::JavaVisitor::writeMarshalUnmarshalCode(Output& out, const string& scope,
                                              const TypePtr& type,
                                              const string& param,
                                              bool marshal, int& iter,
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
                    if (holder)
                    {
                        out << nl << stream
                            << ".readObject(Ice.Object.__classIds[0], "
                            << v << ");";
                    }
                    else
                    {
                        out << sb;
                        out << nl << "Ice.ObjectHolder __h = new "
                            << "Ice.ObjectHolder();";
                        out << nl << stream
                            << ".readObject(Ice.Object.__classIds[0], __h);";
                        out << nl << v << " = __h.value;";
                        out << eb;
                    }
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
            out << nl << typeS << "Helper.__write(" << stream << ", "
                << v << ");";
        }
        else
        {
            out << nl << v << " = " << typeS << "Helper.__read(" << stream
                << ");";
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
                out << sb;
                out << nl << "Ice.ObjectHolder __h = new "
                    << "Ice.ObjectHolder();";
                out << nl << "if (" << stream << ".readObject(" << typeS
                    << ".__classIds[0], __h))";
                out << sb;
                out << nl << v << " = (" << typeS << ")__h.value;";
                out << eb;
                out << nl << "else";
                out << sb;
                out << nl << v << " = new " << typeS << "();";
                out << nl << stream << ".readObject(" << v << ");";
                out << eb;
                out << eb;
            }
            else
            {
                out << sb;
                out << nl << "Ice.ObjectHolder __h = new "
                    << "Ice.ObjectHolder();";
                out << nl << stream << ".readObject(\"\", __h);";
                out << nl << v << " = (" << typeS << ")__h.value;";
                out << eb;
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
                        out << nl << v << " = " << stream
                            << ".readShortSeq();";
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
                        out << nl << v << " = " << stream
                            << ".readFloatSeq();";
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
                        out << nl << v << " = " << stream
                            << ".readDoubleSeq();";
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
                        out << nl << v << " = " << stream
                            << ".readStringSeq();";
                    }
                    break;
                }
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                {
                    string typeS = getAbsolute(seq->scoped(), scope);
                    if (marshal)
                    {
                        out << nl << typeS << "Helper.write(" << stream
                            << ", " << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << typeS << "Helper.read("
                            << stream << ");";
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
                out << nl << typeS << "Helper.write(" << stream << ", "
                    << v << ");";
            }
            else
            {
                out << nl << v << " = " << typeS << "Helper.read(" << stream
                    << ");";
            }
        }
        return;
    }

    ConstructedPtr constructed = ConstructedPtr::dynamicCast(type);
    assert(constructed);
    string typeS = getAbsolute(constructed->scoped(), scope);
    if (marshal)
    {
        out << nl << typeS << "Helper.write(" << stream << ", "
            << v << ");";
    }
    else
    {
        out << nl << v << " = " << typeS << "Helper.read(" << stream
            << ");";
    }
}

void
Slice::JavaVisitor::writeHashCode(Output& out, const TypePtr& type,
                                  const string& name, int& iter)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindLong:
            {
                out << nl << "__h = 5 * __h + (int)" << name << ';';
                break;
            }
            case Builtin::KindBool:
            {
                out << nl << "__h = 5 * __h + (" << name << " ? 1 : 0);";
                break;
            }
            case Builtin::KindInt:
            {
                out << nl << "__h = 5 * __h + " << name << ';';
                break;
            }
            case Builtin::KindFloat:
            {
                out << nl << "__h = 5 * __h + "
                    << "java.lang.Float.floatToIntBits(" << name << ");";
                break;
            }
            case Builtin::KindDouble:
            {
                out << nl << "__h = 5 * __h + (int)"
                    << "java.lang.Double.doubleToLongBits(" << name << ");";
                break;
            }
            case Builtin::KindString:
            {
                out << nl << "__h = 5 * __h + " << name << ".hashCode();";
                break;
            }
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindLocalObject:
            {
                out << nl << "if (" << name << " != null)";
                out << sb;
                out << nl << "__h = 5 * __h + " << name << ".hashCode();";
                out << eb;
                break;
            }
        }
        return;
    }

    ProxyPtr prx = ProxyPtr::dynamicCast(type);
    if (prx)
    {
        out << nl << "if (" << name << " != null)";
        out << sb;
        out << nl << "__h = 5 * __h + " << name << ".hashCode();";
        out << eb;
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if (cl)
    {
        out << nl << "if (" << name << " != null)";
        out << sb;
        out << nl << "__h = 5 * __h + " << name << ".hashCode();";
        out << eb;
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if (seq)
    {
        out << nl << "for (int __i" << iter << " = 0; __i" << iter
            << " < " << name << ".length; __i" << iter << "++)";
        out << sb;
        ostringstream elem;
        elem << name << "[__i" << iter << ']';
        iter++;
        writeHashCode(out, seq->type(), elem.str(), iter);
        out << eb;
        return;
    }

    ConstructedPtr constructed = ConstructedPtr::dynamicCast(type);
    assert(constructed);
    out << nl << "__h = 5 * __h + " << name << ".hashCode();";
}

void
Slice::JavaVisitor::writeDispatch(Output& out, const ClassDefPtr& p)
{
    string name = fixKwd(p->name());
    string scope = p->scope();
    string scoped = p->scoped();
    ClassList bases = p->bases();

    ClassList allBases = p->allBases();
    StringList ids;
    transform(allBases.begin(), allBases.end(), back_inserter(ids),
              ::IceUtil::memFun(&ClassDef::scoped));
    StringList other;
    other.push_back(scoped);
    other.push_back("::Ice::Object");
    other.sort();
    ids.merge(other);
    ids.unique();

    ClassList allBaseClasses;
    ClassDefPtr cl;
    if (!bases.empty())
    {
        cl = bases.front();
    }
    else
    {
        cl = 0;
    }
    while (cl && !cl->isInterface())
    {
        allBaseClasses.push_back(cl);
        ClassList baseBases = cl->bases();
        if (!baseBases.empty())
        {
            cl = baseBases.front();
        }
        else
        {
            cl = 0;
        }
    }
    StringList classIds;
    transform(allBaseClasses.begin(), allBaseClasses.end(),
              back_inserter(classIds),
              ::IceUtil::memFun(&ClassDef::scoped));
    classIds.push_front(scoped);
    classIds.push_back("::Ice::Object");

    StringList::const_iterator q;

    out << sp << nl << "public static final String[] __ids =";
    out << sb;
    q = ids.begin();
    while (q != ids.end())
    {
        out << nl << '"' << *q << '"';
        if (++q != ids.end())
        {
            out << ',';
        }
    }
    out << eb << ';';

    out << sp << nl << "public static final String[] __classIds =";
    out << sb;
    q = classIds.begin();
    while (q != classIds.end())
    {
        out << nl << '"' << *q << '"';
        if (++q != classIds.end())
        {
            out << ',';
        }
    }
    out << eb << ';';

    //
    // ice_isA
    //
    out << sp << nl << "public boolean"
        << nl << "ice_isA(String s, Ice.Current current)";
    out << sb;
    out << nl << "return java.util.Arrays.binarySearch(__ids, s) >= 0;";
    out << eb;

    //
    // __getClassIds
    //
    out << sp << nl << "public String[]" << nl << "__getClassIds()";
    out << sb;
    out << nl << "return __classIds;";
    out << eb;

    //
    // Dispatch operations
    //
    OperationList ops = p->operations();
    OperationList::const_iterator r;
    for (r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = (*r);
        ContainerPtr container = op->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        assert(cl);

        string opName = fixKwd(op->name());
        out << sp << nl << "public static IceInternal.DispatchStatus"
            << nl << "___" << opName << "(" << name
            << " __obj, IceInternal.Incoming __in, Ice.Current __current)";
        out << sb;

        TypePtr ret = op->returnType();
        string retS = typeToString(ret, TypeModeReturn, scope);
        int iter;

        TypeStringList inParams = op->inputParameters();
        TypeStringList outParams = op->outputParameters();
        TypeStringList::const_iterator q;

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

	//
	// MSVC gets confused if
	// ::IceUtil::memFun(&::Slice::Exception::isLocal)); is used
	// hence the exceptionIsLocal function.
	//
        remove_if(throws.begin(), throws.end(), exceptionIsLocal);

        if (!inParams.empty())
        {
            out << nl << "IceInternal.BasicStream __is = __in.is();";
        }
        if (!outParams.empty() || ret || throws.size() > 0)
        {
            out << nl << "IceInternal.BasicStream __os = __in.os();";
        }

        //
        // Unmarshal 'in' params
        //
        iter = 0;
        for (q = inParams.begin(); q != inParams.end(); ++q)
        {
            string typeS = typeToString(q->first, TypeModeIn, scope);
            out << nl << typeS << ' ' << fixKwd(q->second) << ';';
            writeMarshalUnmarshalCode(out, scope, q->first, fixKwd(q->second),
                                      false, iter);
        }

        //
        // Create holders for 'out' params
        //
        for (q = outParams.begin(); q != outParams.end(); ++q)
        {
            string typeS = typeToString(q->first, TypeModeOut, scope);
            out << nl << typeS << ' ' << fixKwd(q->second) << " = new "
                << typeS << "();";
        }

        if (!throws.empty())
        {
            out << nl << "try";
            out << sb;
        }

        //
        // Call servant
        //
        out << nl;
        if (ret)
        {
            out << retS << " __ret = ";
        }
        out << "__obj." << opName << '(';
        for (q = inParams.begin(); q != inParams.end(); ++q)
        {
            out << fixKwd(q->second) << ", ";
        }
        for (q = outParams.begin(); q != outParams.end(); ++q)
        {
            out << fixKwd(q->second) << ", ";
        }
        out << "__current);";

        //
        // Marshal 'out' params
        //
        for (q = outParams.begin(); q != outParams.end(); ++q)
        {
            writeMarshalUnmarshalCode(out, scope, q->first, fixKwd(q->second),
                                      true, iter, true);
        }
        //
        // Marshal result
        //
        if (ret)
        {
            writeMarshalUnmarshalCode(out, scope, ret, "__ret", true, iter);
        }

        out << nl << "return IceInternal.DispatchStatus.DispatchOK;";

        //
        // User exceptions
        //
        if (!throws.empty())
        {
            out << eb;
            ExceptionList::const_iterator r;
            for (r = throws.begin(); r != throws.end(); ++r)
            {
                string exS = getAbsolute((*r)->scoped(), scope);
                out << nl << "catch (" << exS << " ex)";
                out << sb;
                out << nl << "__os.writeUserException(ex);";
                out << nl << "return IceInternal.DispatchStatus."
                    << "DispatchUserException;";
                out << eb;
            }
        }

        out << eb;
    }

    //
    // __dispatch
    //
    OperationList allOps = p->allOperations();
    if (!allOps.empty())
    {
        StringList allOpNames;
        transform(allOps.begin(), allOps.end(), back_inserter(allOpNames),
                  ::IceUtil::memFun(&Operation::name));
        allOpNames.push_back("ice_isA");
        allOpNames.push_back("ice_ping");
        allOpNames.sort();
        allOpNames.unique();

        StringList::const_iterator q;

        out << sp << nl << "private final static String[] __all =";
        out << sb;
        q = allOpNames.begin();
        while (q != allOpNames.end())
        {
            out << nl << '"' << *q << '"';
            if (++q != allOpNames.end())
            {
                out << ',';
            }
        }
        out << eb << ';';

        out << sp << nl << "public IceInternal.DispatchStatus"
            << nl << "__dispatch(IceInternal.Incoming in, Ice.Current current)";
        out << sb;
        out << nl << "int pos = java.util.Arrays.binarySearch(__all, "
            << "current.operation);";
        out << nl << "if (pos < 0)";
        out << sb;
        out << nl << "return IceInternal.DispatchStatus."
            << "DispatchOperationNotExist;";
        out << eb;
        out << sp << nl << "switch (pos)";
        out << sb;
        int i = 0;
        for (q = allOpNames.begin(); q != allOpNames.end(); ++q)
        {
            string opName = fixKwd(*q);

            out << nl << "case " << i++ << ':';
            out << sb;
            if (opName == "ice_isA")
            {
                out << nl << "return ___ice_isA(this, in, current);";
            }
            else if (opName == "ice_ping")
            {
                out << nl << "return ___ice_ping(this, in, current);";
            }
            else
            {
                //
                // There's probably a better way to do this
                //
                for (OperationList::const_iterator r = allOps.begin();
                     r != allOps.end();
                     ++r)
                {
                    if ((*r)->name() == (*q))
                    {
                        ContainerPtr container = (*r)->container();
                        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
                        assert(cl);
                        if (cl->name() == p->name())
                        {
                            out << nl << "return ___" << opName
                                << "(this, in, current);";
                        }
                        else
                        {
                            string base;
                            if (cl->isInterface())
                            {
                                base = getAbsolute(cl->scoped(), scope, "_",
                                                   "Disp");
                            }
                            else
                            {
                                base = getAbsolute(cl->scoped(), scope);
                            }
                            out << nl << "return " << base << ".___" << opName
                                << "(this, in, current);";
                        }
                        break;
                    }
                }
            }
            out << eb;
        }
        out << eb;
        out << sp << nl << "assert(false);";
        out << nl << "return IceInternal.DispatchStatus."
            << "DispatchOperationNotExist;";
        out << eb;
    }
}

void
Slice::JavaVisitor::printHeader()
{
    static const char* header =
"// **********************************************************************\n"
"//\n"
"// Copyright (c) 2001\n"
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

Slice::Gen::Gen(const string& name, const string& base,
                const vector<string>& includePaths,
                const string& package, const string& dir) :
    _base(base),
    _includePaths(includePaths),
    _package(package),
    _dir(dir)
{
}

Slice::Gen::~Gen()
{
}

bool
Slice::Gen::operator!() const
{
    return false;
}

void
Slice::Gen::generate(const UnitPtr& unit)
{
    TypesVisitor typesVisitor(_dir, _package);
    unit->visit(&typesVisitor);

    HolderVisitor holderVisitor(_dir, _package);
    unit->visit(&holderVisitor);

    HelperVisitor helperVisitor(_dir, _package);
    unit->visit(&helperVisitor);

    ProxyVisitor proxyVisitor(_dir, _package);
    unit->visit(&proxyVisitor);

    DelegateVisitor delegateVisitor(_dir, _package);
    unit->visit(&delegateVisitor);

    DelegateMVisitor delegateMVisitor(_dir, _package);
    unit->visit(&delegateMVisitor);

    DispatcherVisitor dispatcherVisitor(_dir, _package);
    unit->visit(&dispatcherVisitor);
}

void
Slice::Gen::generateImpl(const UnitPtr& unit)
{
    ImplVisitor implVisitor(_dir, _package);
    unit->visit(&implVisitor);
}

Slice::Gen::TypesVisitor::TypesVisitor(const string& dir,
                                       const string& package) :
    JavaVisitor(dir, package)
{
}

bool
Slice::Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = fixKwd(p->name());
    string scoped = p->scoped();
    ClassList bases = p->bases();
    string scope = p->scope();
    string absolute = getAbsolute(scoped);

    if (!open(absolute))
    {
        return false;
    }

    Output& out = output();

    //
    // Local interfaces map to Java interfaces
    //
    if (p->isInterface())
    {
        out << sp << nl << "public interface " << name;
        if (!bases.empty())
        {
            out << " extends ";
            out.useCurrentPosAsIndent();
            ClassList::const_iterator q = bases.begin();
            while (q != bases.end())
            {
                out << getAbsolute((*q)->scoped(), scope);
                if (++q != bases.end())
                {
                    out << ',' << nl;
                }
            }
            out.restoreIndent();
        }
    }
    else
    {
        out << sp << nl << "public ";
        if (p->isAbstract())
        {
            out << "abstract ";
        }
        out << "class " << name;
        out.useCurrentPosAsIndent();
        if (bases.empty() || bases.front()->isInterface())
        {
            if (p->isLocal())
            {
                out << " extends Ice.LocalObject";
            }
            else
            {
                out << " extends Ice.Object";
            }
        }
        else
        {
            out << " extends ";
            ClassDefPtr base = bases.front();
            out << getAbsolute(base->scoped(), scope);
            bases.pop_front();
        }

        //
        // Implement interfaces
        //
        if (!bases.empty())
        {
            out << nl << " implements ";
            out.useCurrentPosAsIndent();
            ClassList::const_iterator q = bases.begin();
            while (q != bases.end())
            {
                out << getAbsolute((*q)->scoped(), scope);
                if (++q != bases.end())
                {
                    out << ',' << nl;
                }
            }
            out.restoreIndent();
        }
        out.restoreIndent();
    }

    out << sb;

    //
    // hashCode
    //
    if (!p->isInterface())
    {
        bool baseHasDataMembers = false;
        ClassList l = p->bases();
        while (!l.empty() && !l.front()->isInterface())
        {
            if (l.front()->hasDataMembers())
            {
                baseHasDataMembers = true;
                break;
            }
            l = l.front()->bases();
        }

        if (p->hasDataMembers() || baseHasDataMembers)
        {
            out << sp << nl << "public int"
                << nl << "hashCode()";
            out << sb;
            if (p->hasDataMembers())
            {
                DataMemberList members = p->dataMembers();
                DataMemberList::const_iterator d;
                int iter;

                out << nl << "int __h = 0;";
                iter = 0;
                for (d = members.begin(); d != members.end(); ++d)
                {
                    string memberName = fixKwd((*d)->name());
                    writeHashCode(out, (*d)->type(), memberName, iter);
                }
                if (baseHasDataMembers)
                {
                    out << nl << "__h = 5 * __h + super.hashCode();";
                }
                out << nl << "return __h;";
            }
            else
            {
                out << nl << "return super.hashCode();";
            }
            out << eb;
        }
    }

    if (!p->isInterface() && !p->isLocal())
    {
        //
        // __dispatch, etc.
        //
        writeDispatch(out, p);

        DataMemberList members = p->dataMembers();
        DataMemberList::const_iterator d;
        int iter;

        //
        // __write
        //
        out << sp << nl << "public void"
            << nl << "__write(IceInternal.BasicStream __os)";
        out << sb;
        iter = 0;
        for (d = members.begin(); d != members.end(); ++d)
        {
            writeMarshalUnmarshalCode(out, scope, (*d)->type(),
                                      fixKwd((*d)->name()), true, iter, false);
        }
        out << nl << "super.__write(__os);";
        out << eb;

        //
        // __read
        //
        out << sp << nl << "public void"
            << nl << "__read(IceInternal.BasicStream __is)";
        out << sb;
        iter = 0;
        for (d = members.begin(); d != members.end(); ++d)
        {
            writeMarshalUnmarshalCode(out, scope, (*d)->type(),
                                      fixKwd((*d)->name()), false, iter,
                                      false);
        }
        out << nl << "super.__read(__is);";
        out << eb;
    }

    return true;
}

void
Slice::Gen::TypesVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    Output& out = output();
    out << eb;
    close();
}

void
Slice::Gen::TypesVisitor::visitOperation(const OperationPtr& p)
{
    string name = fixKwd(p->name());
    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    string scope = cl->scope();

    TypePtr ret = p->returnType();
    string retS = typeToString(ret, TypeModeReturn, scope);

    string params = getParams(p, scope);

    Output& out = output();

    out << sp;
    out << nl;
    if (!cl->isInterface())
    {
        out << "public abstract ";
    }
    out << retS << ' ' << name << '(' << params;
    if (!cl->isLocal())
    {
        if (!params.empty())
        {
            out << ", ";
        }
        out << "Ice.Current current";
    }
    out << ')';

    ExceptionList throws = p->throws();
    throws.sort();
    throws.unique();
    writeThrowsClause(scope, throws);
    out << ';';
}

bool
Slice::Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string name = fixKwd(p->name());
    string scoped = p->scoped();
    ExceptionPtr base = p->base();
    string scope = p->scope();
    string absolute = getAbsolute(scoped);

    if (!open(absolute))
    {
        return false;
    }

    Output& out = output();

    out << sp << nl << "public class " << name << " extends ";

    if (!base)
    {
        if (p->isLocal())
        {
            out << "Ice.LocalException";
        }
        else
        {
            out << "Ice.UserException";
        }
    }
    else
    {
        out << getAbsolute(base->scoped(), p->scope());
    }
    out << sb;

    out << sp << nl << "public String" << nl << "ice_name()";
    out << sb;
    out << nl << "return \"" << scoped.substr(2) << "\";";
    out << eb;

    if (!p->isLocal())
    {
        ExceptionList allBases = p->allBases();
        StringList exceptionIds;
        transform(allBases.begin(), allBases.end(),
                  back_inserter(exceptionIds),
                  ::IceUtil::memFun(&Exception::scoped));
        exceptionIds.push_front(scoped);
        exceptionIds.push_back("::Ice::UserException");

        StringList::const_iterator q;

        out << sp << nl << "private static final String[] __exceptionIds =";
        out << sb;
        q = exceptionIds.begin();
        while (q != exceptionIds.end())
        {
            out << nl << '"' << *q << '"';
            if (++q != exceptionIds.end())
            {
                out << ',';
            }
        }
        out << eb << ';';
        out << sp << nl << "public String[]" << nl << "__getExceptionIds()";
        out << sb;
        out << nl << "return __exceptionIds;";
        out << eb;
    }

    return true;
}

void
Slice::Gen::TypesVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    Output& out = output();

    if (!p->isLocal())
    {
        string name = fixKwd(p->name());
        string scoped = p->scoped();
        string scope = p->scope();
        ExceptionPtr base = p->base();

        DataMemberList members = p->dataMembers();
        DataMemberList::const_iterator d;
        int iter;

        //
        // __write
        //
        out << sp << nl << "public void"
            << nl << "__write(IceInternal.BasicStream __os)";
        out << sb;
        iter = 0;
        for (d = members.begin(); d != members.end(); ++d)
        {
            writeMarshalUnmarshalCode(out, scope, (*d)->type(),
                                      fixKwd((*d)->name()), true, iter, false);
        }
        if (base)
        {
            out << nl << "super.__write(__os);";
        }
        out << eb;

        //
        // __read
        //
        out << sp << nl << "public void"
            << nl << "__read(IceInternal.BasicStream __is)";
        out << sb;
        iter = 0;
        for (d = members.begin(); d != members.end(); ++d)
        {
            writeMarshalUnmarshalCode(out, scope, (*d)->type(),
                                      fixKwd((*d)->name()), false, iter,
                                      false);
        }
        if (base)
        {
            out << nl << "super.__read(__is);";
        }
        out << eb;
    }

    out << eb;
    close();
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    string name = fixKwd(p->name());
    string scoped = p->scoped();
    string absolute = getAbsolute(scoped);

    if (!open(absolute))
    {
        return false;
    }

    Output& out = output();

    out << sp << nl << "public final class " << name;
    out << sb;

    return true;
}

void
Slice::Gen::TypesVisitor::visitStructEnd(const StructPtr& p)
{
    string scope = p->scope();

    Output& out = output();

    DataMemberList members = p->dataMembers();
    DataMemberList::const_iterator d;
    int iter;

    string typeS = typeToString(p, TypeModeIn, scope);

    if (!members.empty())
    {
        //
        // equals
        //
        out << sp << nl << "public boolean"
            << nl << "equals(java.lang.Object rhs)";
        out << sb;
        out << nl << typeS << " _r = null;";
        out << nl << "try";
        out << sb;
        out << nl << "_r = (" << typeS << ")rhs;";
        out << eb;
        out << nl << "catch (ClassCastException ex)";
        out << sb;
        out << eb;
        out << sp << nl << "if (_r != null)";
        out << sb;
        for (d = members.begin(); d != members.end(); ++d)
        {
            string memberName = fixKwd((*d)->name());
            BuiltinPtr b = BuiltinPtr::dynamicCast((*d)->type());
            if (b)
            {
                switch (b->kind())
                {
                    case Builtin::KindByte:
                    case Builtin::KindBool:
                    case Builtin::KindShort:
                    case Builtin::KindInt:
                    case Builtin::KindLong:
                    case Builtin::KindFloat:
                    case Builtin::KindDouble:
                    {
                        out << nl << "if (" << memberName << " != _r."
                            << memberName << ")";
                        out << sb;
                        out << nl << "return false;";
                        out << eb;
                        break;
                    }

                    case Builtin::KindString:
                    case Builtin::KindObject:
                    case Builtin::KindObjectProxy:
                    case Builtin::KindLocalObject:
                    {
                        out << nl << "if (!" << memberName << ".equals(_r."
                            << memberName << "))";
                        out << sb;
                        out << nl << "return false;";
                        out << eb;
                        break;
                    }
                }
            }
            else
            {
                out << nl << "if (!" << memberName << ".equals(_r."
                    << memberName << "))";
                out << sb;
                out << nl << "return false;";
                out << eb;
            }
        }
        out << sp << nl << "return true;";
        out << eb;
        out << sp << nl << "return false;";
        out << eb;

        //
        // hashCode
        //
        out << sp << nl << "public int"
            << nl << "hashCode()";
        out << sb;
        out << nl << "int __h = 0;";
        iter = 0;
        for (d = members.begin(); d != members.end(); ++d)
        {
            string memberName = fixKwd((*d)->name());
            writeHashCode(out, (*d)->type(), memberName, iter);
        }
        out << nl << "return __h;";
        out << eb;
    }

    if (!p->isLocal())
    {
        //
        // __write
        //
        out << sp << nl << "public void"
            << nl << "__write(IceInternal.BasicStream __os)";
        out << sb;
        iter = 0;
        for (d = members.begin(); d != members.end(); ++d)
        {
            writeMarshalUnmarshalCode(out, scope, (*d)->type(),
                                      fixKwd((*d)->name()), true, iter, false);
        }
        out << eb;

        //
        // __read
        //
        out << sp << nl << "public void"
            << nl << "__read(IceInternal.BasicStream __is)";
        out << sb;
        iter = 0;
        for (d = members.begin(); d != members.end(); ++d)
        {
            writeMarshalUnmarshalCode(out, scope, (*d)->type(),
                                      fixKwd((*d)->name()), false, iter,
                                      false);
        }
        out << eb;
    }

    out << eb;
    close();
}

void
Slice::Gen::TypesVisitor::visitDataMember(const DataMemberPtr& p)
{
    string name = fixKwd(p->name());
    ContainerPtr container = p->container();
    ContainedPtr contained = ContainedPtr::dynamicCast(container);
    string s = typeToString(p->type(), TypeModeMember, contained->scope());
    Output& out = output();
    out << sp << nl << "public " << s << ' ' << name << ';';
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string name = fixKwd(p->name());
    string scoped = p->scoped();
    string absolute = getAbsolute(scoped);
    EnumeratorList enumerators = p->getEnumerators();
    EnumeratorList::const_iterator en;
    int sz = enumerators.size();

    if (!open(absolute))
    {
        return;
    }

    Output& out = output();

    out << sp << nl << "public final class " << name;
    out << sb;
    out << nl << "private static " << name << "[] __values = new "
        << name << "[" << sz << "];";
    out << nl << "private int __value;";
    out << sp;
    int n;
    for (en = enumerators.begin(), n = 0; en != enumerators.end(); ++en, ++n)
    {
        string member = fixKwd((*en)->name());
        out << nl << "public static final int _" << member << " = "
            << n << ';';
        out << nl << "public static final " << name << ' ' << member
            << " = new " << name << "(_" << member << ");";
    }

    out << sp << nl << "public static " << name << nl << "convert(int val)";
    out << sb;
    out << nl << "assert val < " << sz << ';';
    out << nl << "return __values[val];";
    out << eb;

    out << sp << nl << "public int" << nl << "value()";
    out << sb;
    out << nl << "return __value;";
    out << eb;

    out << sp << nl << "private" << nl << name << "(int val)";
    out << sb;
    out << nl << "__value = val;";
    out << nl << "__values[val] = this;";
    out << eb;

    if (!p->isLocal())
    {
        //
        // write
        //
        out << sp << nl << "public void" << nl
            << "__write(IceInternal.BasicStream __os)";
        out << sb;
        if (sz <= 0x7f)
        {
            out << nl << "__os.writeByte((byte)__value);";
        }
        else if (sz <= 0x7fff)
        {
            out << nl << "__os.writeShort((short)__value);";
        }
        else
        {
            out << nl << "__os.writeInt(__value);";
        }
        out << eb;

        //
        // read
        //
        out << sp << nl << "public static " << name << nl
            << "__read(IceInternal.BasicStream __is)";
        out << sb;
        if (sz <= 0x7f)
        {
            out << nl << "int __v = __is.readByte();";
        }
        else if (sz <= 0x7fff)
        {
            out << nl << "int __v = __is.readShort();";
        }
        else
        {
            out << nl << "int __v = __is.readInt();";
        }
        out << nl << "return " << name << ".convert(__v);";
        out << eb;
    }

    out << eb;
    close();
}

Slice::Gen::HolderVisitor::HolderVisitor(const string& dir,
                                         const string& package) :
    JavaVisitor(dir, package)
{
}

bool
Slice::Gen::HolderVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    ClassDeclPtr decl = p->declaration();
    writeHolder(decl);

    if (!p->isInterface() && !p->isLocal())
    {
        string name = fixKwd(p->name());
        string absolute = getAbsolute(p->scoped());

        if (open(absolute + "PrxHolder"))
        {
            Output& out = output();
            out << sp << nl << "public final class " << name << "PrxHolder";
            out << sb;
            out << nl << "public " << name << "Prx value;";
            out << eb;
            close();
        }
    }

    return false;
}

bool
Slice::Gen::HolderVisitor::visitStructStart(const StructPtr& p)
{
    writeHolder(p);
    return false;
}

void
Slice::Gen::HolderVisitor::visitSequence(const SequencePtr& p)
{
    writeHolder(p);
}

void
Slice::Gen::HolderVisitor::visitDictionary(const DictionaryPtr& p)
{
    writeHolder(p);
}

void
Slice::Gen::HolderVisitor::visitEnum(const EnumPtr& p)
{
    writeHolder(p);
}

void
Slice::Gen::HolderVisitor::writeHolder(const TypePtr& p)
{
    ContainedPtr contained = ContainedPtr::dynamicCast(p);
    assert(contained);
    string name = fixKwd(contained->name());
    string absolute = getAbsolute(contained->scoped());
    string holder = absolute + "Holder";

    if (open(holder))
    {
        Output& out = output();
        string typeS = typeToString(p, TypeModeIn, contained->scope());
        out << sp << nl << "public final class " << name << "Holder";
        out << sb;
        out << nl << "public " << typeS << " value;";
        out << eb;
        close();
    }
}

Slice::Gen::HelperVisitor::HelperVisitor(const string& dir,
                                         const string& package) :
    JavaVisitor(dir, package)
{
}

bool
Slice::Gen::HelperVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if (p->isLocal())
    {
        return false;
    }

    //
    // Proxy helper
    //
    string name = fixKwd(p->name());
    string scoped = p->scoped();
    ClassList bases = p->bases();
    string scope = p->scope();
    string absolute = getAbsolute(scoped);

    if (!open(absolute + "PrxHelper"))
    {
        return false;
    }

    Output& out = output();

    //
    // A proxy helper class serves two purposes: it implements the
    // proxy interface, and provides static helper methods for use
    // by applications (e.g., checkedCast, etc.)
    //
    out << sp << nl << "public final class " << name
        << "PrxHelper extends Ice.ObjectPrxHelper implements " << name
        << "Prx";

    out << sb;

    OperationList ops = p->allOperations();

    OperationList::const_iterator r;
    for (r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = (*r);
        string opName = fixKwd(op->name());
        TypePtr ret = op->returnType();
        string retS = typeToString(ret, TypeModeReturn, scope);

        string params = getParams(op, scope);
        string args = getArgs(op, scope);

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

        //
        // Write two versions of the operation - with and without a
        // context parameter
        //
        out << sp;
        out << nl;
        out << "public final " << retS << nl << opName << '(' << params
            << ")";
        writeThrowsClause(scope, throws);
        out << sb;
        out << nl;
        if (ret)
        {
            out << "return ";
        }
        out << opName << '(' << args;
        if (!args.empty())
        {
            out << ", ";
        }
        out << "null);";
        out << eb;

        out << sp;
        out << nl;
        out << "public final " << retS << nl << opName << '(' << params;
        if (!params.empty())
        {
            out << ", ";
        }
        out << "java.util.Map __context)";
        writeThrowsClause(scope, throws);
        out << sb;
        out << nl << "int __cnt = 0;";
        out << nl << "while (true)";
        out << sb;
        out << nl << "Ice._ObjectDel __delBase = __getDelegate();";
        out << nl << '_' << name << "Del __del = (_" << name
            << "Del)__delBase;";
        out << nl << "try";
        out << sb;
        out << nl;
        if (ret)
        {
            out << "return ";
        }
        out << "__del." << opName << '(' << args;
        if (!args.empty())
        {
            out << ", ";
        }
        out << "__context);";
        if (!ret)
        {
            out << nl << "return;";
        }
        out << eb;
        out << nl << "catch (Ice.LocationForward __ex)";
        out << sb;
        out << nl << "__locationForward(__ex);";
        out << eb;
        out << nl << "catch (IceInternal.NonRepeatable __ex)";
        out << sb;
	list<string> metaData = op->getMetaData();
	bool nonmutating = find(metaData.begin(), metaData.end(), "nonmutating") != metaData.end();
        if (nonmutating)
        {
            out << nl << "__cnt = __handleException(__ex.get(), __cnt);";
        }
        else
        {
            out << nl << "__rethrowException(__ex.get());";
        }
        out << eb;
        out << nl << "catch (Ice.LocalException __ex)";
        out << sb;
        out << nl << "__cnt = __handleException(__ex, __cnt);";
        out << eb;
        out << eb;
        out << eb;
    }

    //
    // checkedCast
    //
    out << sp << nl << "public static " << name << "Prx"
        << nl << "checkedCast(Ice.ObjectPrx p)";
    out << sb;
    out << nl << "return checkedCast(p, \"\");";
    out << eb;

    out << sp << nl << "public static " << name << "Prx"
        << nl << "checkedCast(Ice.ObjectPrx p, String facet)";
    out << sb;
    out << nl << name << "Prx result = null;";
    out << nl << "if (p != null)";
    out << sb;
    out << nl << "if (facet.equals(p.ice_getFacet()))";
    out << sb;
    out << nl << "try";
    out << sb;
    out << nl << "result = (" << name << "Prx)p;";
    out << eb;
    out << nl << "catch (ClassCastException ex)";
    out << sb;
    out << nl << "if (p.ice_isA(\"" << scoped << "\"))";
    out << sb;
    out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    out << nl << "h.__copyFrom(p);";
    out << nl << "result = h;";
    out << eb;
    out << eb;
    out << eb;
    out << nl << "else";
    out << sb;
    out << nl << "Ice.ObjectPrx pp = p.ice_newFacet(facet);";
    out << nl << "try";
    out << sb;
    out << nl << "if (pp.ice_isA(\"" << scoped << "\"))";
    out << sb;
    out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    out << nl << "h.__copyFrom(pp);";
    out << nl << "result = h;";
    out << eb;
    out << eb;
    out << nl << "catch (Ice.FacetNotExistException ex)";
    out << sb;
    out << eb;
    out << eb;
    out << eb;
    out << sp;
    out << nl << "return result;";
    out << eb;

    //
    // uncheckedCast
    //
    out << sp << nl << "public static " << name << "Prx"
        << nl << "uncheckedCast(Ice.ObjectPrx p)";
    out << sb;
    out << nl << "return uncheckedCast(p, \"\");";
    out << eb;

    out << sp << nl << "public static " << name << "Prx"
        << nl << "uncheckedCast(Ice.ObjectPrx p, String facet)";
    out << sb;
    out << nl << name << "Prx result = null;";
    out << nl << "if (p != null)";
    out << sb;
    out << nl << "if (facet.equals(p.ice_getFacet()))";
    out << sb;
    out << nl << "try";
    out << sb;
    out << nl << "result = (" << name << "Prx)p;";
    out << eb;
    out << nl << "catch (ClassCastException ex)";
    out << sb;
    out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    out << nl << "h.__copyFrom(p);";
    out << nl << "result = h;";
    out << eb;
    out << eb;
    out << nl << "else";
    out << sb;
    out << nl << "Ice.ObjectPrx pp = p.ice_newFacet(facet);";
    out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    out << nl << "h.__copyFrom(pp);";
    out << nl << "result = h;";
    out << eb;
    out << eb;
    out << sp;
    out << nl << "return result;";
    out << eb;

    //
    // __createDelegateM
    //
    out << sp << nl << "protected Ice._ObjectDelM"
        << nl << "__createDelegateM()";
    out << sb;
    out << nl << "return new _" << name << "DelM();";
    out << eb;

    //
    // __createDelegateD
    //
    out << sp << nl << "protected Ice._ObjectDelD"
        << nl << "__createDelegateD()";
    out << sb;
    out << nl << "return null;";
    out << eb;

    //
    // __write
    //
    out << sp << nl << "public static void"
        << nl << "__write(IceInternal.BasicStream __os, " << name << "Prx v)";
    out << sb;
    out << nl << "__os.writeProxy(v);";
    out << eb;

    //
    // __read
    //
    out << sp << nl << "public static " << name << "Prx"
        << nl << "__read(IceInternal.BasicStream __is)";
    out << sb;
    out << nl << "Ice.ObjectPrx proxy = __is.readProxy();";
    out << nl << "if (proxy != null)";
    out << sb;
    out << nl << name << "PrxHelper result = new " << name << "PrxHelper();";
    out << nl << "result.__copyFrom(proxy);";
    out << nl << "return result;";
    out << eb;
    out << nl << "return null;";
    out << eb;

    out << eb;
    close();

    return false;
}

void
Slice::Gen::HelperVisitor::visitSequence(const SequencePtr& p)
{
    //
    // Don't generate helper for a sequence of a local type
    //
    if (p->isLocal())
    {
        return;
    }

    //
    // Determine sequence depth
    //
    int depth = 0;
    TypePtr origContent = p->type();
    SequencePtr s = SequencePtr::dynamicCast(origContent);
    while (s)
    {
        depth++;
        origContent = s->type();
        s = SequencePtr::dynamicCast(origContent);
    }

    //
    // Only generate a Helper class for non-primitive, non-local sequences
    //
    BuiltinPtr b = BuiltinPtr::dynamicCast(p->type());
    if (b && b->kind() != Builtin::KindObject &&
        b->kind() != Builtin::KindObjectProxy)
    {
        return;
    }

    string name = fixKwd(p->name());
    string absolute = getAbsolute(p->scoped());
    string helper = absolute + "Helper";
    string scope = p->scope();
    string typeS = typeToString(p, TypeModeIn, scope);

    if (open(helper))
    {
        Output& out = output();
        int iter;

        string origContentS = typeToString(origContent, TypeModeIn, scope);

        out << sp << nl << "public final class " << name << "Helper";
        out << sb;

        //
        // write
        //
        out << nl << "public static void"
            << nl << "write(IceInternal.BasicStream __os, " << typeS
            << " __v)";
        out << sb;
        out << nl << "__os.writeInt(__v.length);";
        out << nl << "for (int __i = 0; __i < __v.length; __i++)";
        out << sb;
        iter = 0;
        writeMarshalUnmarshalCode(out, scope, p->type(), "__v[__i]", true,
                                  iter, false);
        out << eb;
        out << eb;

        //
        // read
        //
        out << sp << nl << "public static " << typeS
            << nl << "read(IceInternal.BasicStream __is)";
        out << sb;
        out << nl << "int __len = __is.readInt();";
        out << nl << typeS << " __v = new " << origContentS << "[__len]";
        while (depth--)
        {
            out << "[]";
        }
        out << ';';
        if (b && b->kind() == Builtin::KindObject)
        {
            //
            // Optimization to avoid excessive allocation of ObjectHolder
            //
            out << nl << "Ice.ObjectHolder __h = new Ice.ObjectHolder();";
            out << nl << "for (int __i = 0; __i < __len; __i++)";
            out << sb;
            out << nl << "__is.readObject(Ice.Object.__classIds[0], __h);";
            out << nl << "__v[__i] = __h.value;";
            out << eb;
        }
        else
        {
            out << nl << "for (int __i = 0; __i < __len; __i++)";
            out << sb;
            iter = 0;
            writeMarshalUnmarshalCode(out, scope, p->type(), "__v[__i]", false,
                                      iter, false);
            out << eb;
        }
        out << nl << "return __v;";
        out << eb;

        out << eb;
        close();
    }
}

void
Slice::Gen::HelperVisitor::visitDictionary(const DictionaryPtr& p)
{
    //
    // Don't generate helper for a dictionary containing a local type
    //
    if (p->isLocal())
    {
        return;
    }

    TypePtr key = p->keyType();
    TypePtr value = p->valueType();

    string absolute = getAbsolute(p->scoped());
    string helper = absolute + "Helper";

    if (open(helper))
    {
        Output& out = output();
        string name = fixKwd(p->name());
        string scope = p->scope();
        string keyS = typeToString(key, TypeModeIn, scope);
        string valueS = typeToString(value, TypeModeIn, scope);
        int iter;
        int i;

        out << sp << nl << "public final class " << name << "Helper";
        out << sb;

        //
        // write
        //
        out << nl << "public static void"
            << nl << "write(IceInternal.BasicStream __os, "
            << "java.util.Map __v)";
        out << sb;
        out << nl << "__os.writeInt(__v.size());";
        out << nl << "java.util.Iterator __i = __v.entrySet().iterator();";
        out << nl << "while (__i.hasNext())";
        out << sb;
        out << nl << "java.util.Map.Entry __e = (java.util.Map.Entry)"
            << "__i.next();";
        iter = 0;
        for (i = 0; i < 2; i++)
        {
            string val;
            string arg;
            TypePtr type;
            if (i == 0)
            {
                arg = "__e.getKey()";
                type = key;
            }
            else
            {
                arg = "__e.getValue()";
                type = value;
            }

            BuiltinPtr b = BuiltinPtr::dynamicCast(type);
            if (b)
            {
                switch (b->kind())
                {
                    case Builtin::KindByte:
                    {
                        val = "((java.lang.Byte)" + arg + ").byteValue()";
                        break;
                    }
                    case Builtin::KindBool:
                    {
                        val = "((java.lang.Boolean)" + arg +
                            ").booleanValue()";
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

            if (val.empty())
            {
                val = "((" + typeToString(type, TypeModeIn, scope) + ")" +
                    arg + ")";
            }
            writeMarshalUnmarshalCode(out, scope, type, val, true, iter,
                                      false);
        }
        out << eb;
        out << eb;

        //
        // read
        //
        out << sp << nl << "public static java.util.Map"
            << nl << "read(IceInternal.BasicStream __is)";
        out << sb;
        out << nl << "int __sz = __is.readInt();";
        out << nl << "java.util.Map __r = new java.util.HashMap(__sz);";
        {
            //
            // Optimization to avoid excessive allocation of ObjectHolder
            //
            ClassDeclPtr ck = ClassDeclPtr::dynamicCast(key);
            ClassDeclPtr cv = ClassDeclPtr::dynamicCast(value);
            BuiltinPtr bk = BuiltinPtr::dynamicCast(key);
            BuiltinPtr bv = BuiltinPtr::dynamicCast(value);
            if (ck || cv ||
                (bk && bk->kind() == Builtin::KindObject) ||
                (bv && bv->kind() == Builtin::KindObject))
            {
                out << nl << "Ice.ObjectHolder __h = new Ice.ObjectHolder();";
            }
        }
        out << nl << "for (int __i = 0; __i < __sz; __i++)";
        out << sb;
        iter = 0;
        for (i = 0; i < 2; i++)
        {
            string arg;
            TypePtr type;
            if (i == 0)
            {
                arg = "__key";
                type = key;
            }
            else
            {
                arg = "__value";
                type = value;
            }

            BuiltinPtr b = BuiltinPtr::dynamicCast(type);
            if (b)
            {
                switch (b->kind())
                {
                    case Builtin::KindByte:
                    {
                        out << nl << "java.lang.Byte " << arg
                            << " = new java.lang.Byte(__is.readByte());";
                        break;
                    }
                    case Builtin::KindBool:
                    {
                        out << nl << "java.lang.Boolean " << arg
                            << " = new java.lang.Boolean(__is.readBool());";
                        break;
                    }
                    case Builtin::KindShort:
                    {
                        out << nl << "java.lang.Short " << arg
                            << " = new java.lang.Short(__is.readShort());";
                        break;
                    }
                    case Builtin::KindInt:
                    {
                        out << nl << "java.lang.Integer " << arg
                            << " = new java.lang.Integer(__is.readInt());";
                        break;
                    }
                    case Builtin::KindLong:
                    {
                        out << nl << "java.lang.Long " << arg
                            << " = new java.lang.Long(__is.readLong());";
                        break;
                    }
                    case Builtin::KindFloat:
                    {
                        out << nl << "java.lang.Float " << arg
                            << " = new java.lang.Float(__is.readFloat());";
                        break;
                    }
                    case Builtin::KindDouble:
                    {
                        out << nl << "java.lang.Double " << arg
                            << " = new java.lang.Double(__is.readDouble());";
                        break;
                    }
                    case Builtin::KindString:
                    {
                        out << nl << "java.lang.String " << arg
                            << " = __is.readString();";
                        break;
                    }
                    case Builtin::KindObject:
                    {
                        out << nl << "Ice.Object " << arg << ';';
                        out << nl << "__is.readObject("
                            << "Ice.Object.__classIds[0], __h);";
                        out << nl << arg << " = __h.value;";
                        break;
                    }
                    case Builtin::KindObjectProxy:
                    {
                        out << nl << "Ice.ObjectPrx " << arg
                            << " = __is.readProxy();";
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
                string s = typeToString(type, TypeModeIn, scope);
                out << nl << s << ' ' << arg << ';';
                ClassDeclPtr c = ClassDeclPtr::dynamicCast(type);
                if (c)
                {
                    out << nl << "__is.readObject(\"\", __h);";
                    out << nl << arg << " = __h.value;";
                    ClassDefPtr def = c->definition();
                    if (def && !def->isAbstract())
                    {
                        out << nl << "if (__is.readObject(" << s
                            << ".__classIds[0], __h))";
                        out << sb;
                        out << nl << arg << " = (" << s << ")__h.value;";
                        out << eb;
                        out << nl << "else";
                        out << sb;
                        out << nl << arg << " = new " << s << "();";
                        out << nl << "__is.readObject(" << arg << ");";
                        out << eb;
                    }
                    else
                    {
                        out << nl << "__is.readObject(\"\", __h);";
                        out << nl << arg << " = (" << s << ")__h.value;";
                    }
                }
                else
                {
                    writeMarshalUnmarshalCode(out, scope, type, arg, false,
                                              iter, false);
                }
            }
        }
        out << nl << "__r.put(__key, __value);";
        out << eb;
        out << nl << "return __r;";
        out << eb;

        out << eb;
        close();
    }
}

Slice::Gen::ProxyVisitor::ProxyVisitor(const string& dir,
                                       const string& package) :
    JavaVisitor(dir, package)
{
}

bool
Slice::Gen::ProxyVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if (p->isLocal())
    {
        return false;
    }

    string name = fixKwd(p->name());
    string scoped = p->scoped();
    ClassList bases = p->bases();
    string scope = p->scope();
    string absolute = getAbsolute(scoped);

    if (!open(absolute + "Prx"))
    {
        return false;
    }

    Output& out = output();

    //
    // Generate a Java interface as the user-visible type
    //
    out << sp << nl << "public interface " << name << "Prx extends ";
    if (bases.empty())
    {
        out << "Ice.ObjectPrx";
    }
    else
    {
        out.useCurrentPosAsIndent();
        ClassList::const_iterator q = bases.begin();
        while (q != bases.end())
        {
            out << getAbsolute((*q)->scoped(), scope) << "Prx";
            if (++q != bases.end())
            {
                out << ',' << nl;
            }
        }
        out.restoreIndent();
    }

    out << sb;

    return true;
}

void
Slice::Gen::ProxyVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    Output& out = output();
    out << eb;
    close();
}

void
Slice::Gen::ProxyVisitor::visitOperation(const OperationPtr& p)
{
    string name = fixKwd(p->name());
    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    string scope = cl->scope();

    TypePtr ret = p->returnType();
    string retS = typeToString(ret, TypeModeReturn, scope);

    string params = getParams(p, scope);

    Output& out = output();

    ExceptionList throws = p->throws();
    throws.sort();
    throws.unique();

    //
    // Write two versions of the operation - with and without a
    // context parameter
    //
    out << sp;
    out << nl;
    out << "public " << retS << ' ' << name << '(' << params << ")";
    writeThrowsClause(scope, throws);
    out << ';';
    out << nl;
    out << "public " << retS << ' ' << name << '(' << params;
    if (!params.empty())
    {
        out << ", ";
    }
    out << "java.util.Map __context)";
    writeThrowsClause(scope, throws);
    out << ';';
}

Slice::Gen::DelegateVisitor::DelegateVisitor(const string& dir,
                                             const string& package) :
    JavaVisitor(dir, package)
{
}

bool
Slice::Gen::DelegateVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if (p->isLocal())
    {
        return false;
    }

    string name = fixKwd(p->name());
    string scoped = p->scoped();
    ClassList bases = p->bases();
    string scope = p->scope();
    string absolute = getAbsolute(scoped, "", "_", "Del");

    if (!open(absolute))
    {
        return false;
    }

    Output& out = output();

    out << sp << nl << "public interface _" << name
        << "Del extends ";
    if (bases.empty())
    {
        out << "Ice._ObjectDel";
    }
    else
    {
        out.useCurrentPosAsIndent();
        ClassList::const_iterator q = bases.begin();
        while (q != bases.end())
        {
            out << getAbsolute((*q)->scoped(), scope, "_", "Del");
            if (++q != bases.end())
            {
                out << ',' << nl;
            }
        }
        out.restoreIndent();
    }

    out << sb;

    OperationList ops = p->operations();

    OperationList::const_iterator r;
    for (r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = (*r);
        string opName = fixKwd(op->name());
        TypePtr ret = op->returnType();
        string retS = typeToString(ret, TypeModeReturn, scope);

        string params = getParams(op, scope);

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

        out << sp;
        out << nl;
        out << retS << ' ' << opName << '(' << params;
        if (!params.empty())
        {
            out << ", ";
        }
        out << "java.util.Map __context)";
        writeDelegateThrowsClause(scope, throws);
        out << ';';
    }

    out << eb;
    close();

    return false;
}

Slice::Gen::DelegateMVisitor::DelegateMVisitor(const string& dir,
                                               const string& package) :
    JavaVisitor(dir, package)
{
}

bool
Slice::Gen::DelegateMVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if (p->isLocal())
    {
        return false;
    }

    string name = fixKwd(p->name());
    string scoped = p->scoped();
    ClassList bases = p->bases();
    string scope = p->scope();
    string absolute = getAbsolute(scoped, "", "_", "DelM");

    if (!open(absolute))
    {
        return false;
    }

    Output& out = output();

    out << sp << nl << "public final class _" << name
        << "DelM extends Ice._ObjectDelM implements _" << name << "Del";
    out << sb;

    OperationList ops = p->allOperations();

    OperationList::const_iterator r;
    for (r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = (*r);
        string opName = fixKwd(op->name());
        TypePtr ret = op->returnType();
        string retS = typeToString(ret, TypeModeReturn, scope);
        int iter;

        TypeStringList inParams = op->inputParameters();
        TypeStringList outParams = op->outputParameters();
        TypeStringList::const_iterator q;

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

	//
	// MSVC gets confused if
	// ::IceUtil::memFun(&::Slice::Exception::isLocal)); is used
	// hence the exceptionIsLocal function.
	//
        remove_if(throws.begin(), throws.end(), exceptionIsLocal);

        string params = getParams(op, scope);

        out << sp;
        out << nl;
        out << "public " << retS << nl << opName << '(' << params;
        if (!params.empty())
        {
            out << ", ";
        }
        out << "java.util.Map __context)";
        writeDelegateThrowsClause(scope, throws);
        out << sb;
	list<string> metaData = op->getMetaData();
	bool nonmutating = find(metaData.begin(), metaData.end(), "nonmutating") != metaData.end();
        out << nl << "IceInternal.Outgoing __out = new IceInternal.Outgoing(__connection, __reference, \""
            << op->name() << "\", " << (nonmutating ? "true" : "false") << ", __context);";
        if (!inParams.empty())
        {
            out << nl << "IceInternal.BasicStream __os = __out.os();";
        }
        if (!outParams.empty() || ret || throws.size() > 0)
        {
            out << nl << "IceInternal.BasicStream __is = __out.is();";
        }
        iter = 0;
        for (q = inParams.begin(); q != inParams.end(); ++q)
        {
            writeMarshalUnmarshalCode(out, scope, q->first, fixKwd(q->second),
                                      true, iter);
        }
        out << nl << "if (!__out.invoke())";
        out << sb;
        if (throws.size() > 0)
        {
            //
            // The try/catch block is necessary because throwException()
            // can raise UserException
            //
            out << nl << "try";
            out << sb;
            out << nl << "final String[] __throws =";
            out << sb;
            ExceptionList::const_iterator r;
            for (r = throws.begin(); r != throws.end(); ++r)
            {
                if (r != throws.begin())
                {
                    out << ",";
                }
                out << nl << "\"" << (*r)->scoped() << "\"";
            }
            out << eb;
            out << ';';
            out << nl << "switch (__is.throwException(__throws))";
            out << sb;
            int count = 0;
            for (r = throws.begin(); r != throws.end(); ++r)
            {
                out << nl << "case " << count << ':';
                out << sb;
                string abs = getAbsolute((*r)->scoped(), scope);
                out << nl << abs << " __ex = new " << abs << "();";
                out << nl << "__ex.__read(__is);";
                out << nl << "throw __ex;";
                out << eb;
                count++;
            }
            out << eb;
            out << eb;
            for (r = throws.begin(); r != throws.end(); ++r)
            {
                out << nl << "catch (" << getAbsolute((*r)->scoped(), scope)
                    << " __ex)";
                out << sb;
                out << nl << "throw __ex;";
                out << eb;
            }
            out << nl << "catch (Ice.UserException __ex)";
            out << sb;
            out << eb;
        }
        out << nl << "throw new Ice.UnknownUserException();";
        out << eb;

        for (q = outParams.begin(); q != outParams.end(); ++q)
        {
            writeMarshalUnmarshalCode(out, scope, q->first, fixKwd(q->second),
                                      false, iter, true);
        }

        if (ret)
        {
            out << nl << retS << " __ret;";
            writeMarshalUnmarshalCode(out, scope, ret, "__ret", false, iter);
            out << nl << "return __ret;";
        }

        out << eb;
    }

    out << eb;
    close();

    return false;
}

Slice::Gen::DispatcherVisitor::DispatcherVisitor(const string& dir,
                                                 const string& package) :
    JavaVisitor(dir, package)
{
}

bool
Slice::Gen::DispatcherVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if (p->isLocal() || !p->isInterface())
    {
        return false;
    }

    string name = fixKwd(p->name());
    string scoped = p->scoped();
    ClassList bases = p->bases();
    string scope = p->scope();
    string absolute = getAbsolute(scoped, "", "_", "Disp");

    if (!open(absolute))
    {
        return false;
    }

    Output& out = output();

    out << sp << nl << "public abstract class _" << name
        << "Disp extends Ice.Object implements " << name;
    out << sb;

    writeDispatch(out, p);

    out << eb;
    close();

    return false;
}

Slice::Gen::ImplVisitor::ImplVisitor(const string& dir,
                                     const string& package) :
    JavaVisitor(dir, package)
{
}

void
Slice::Gen::ImplVisitor::writeAssign(Output& out, const string& scope,
                                     const TypePtr& type, const string& name,
                                     int& iter)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindByte:
            {
                out << nl << name << " = (byte)0;";
                break;
            }
            case Builtin::KindBool:
            {
                out << nl << name << " = false;";
                break;
            }
            case Builtin::KindShort:
            {
                out << nl << name << " = (short)0;";
                break;
            }
            case Builtin::KindInt:
            {
                out << nl << name << " = 0;";
                break;
            }
            case Builtin::KindLong:
            {
                out << nl << name << " = 0L;";
                break;
            }
            case Builtin::KindFloat:
            {
                out << nl << name << " = 0.0f;";
                break;
            }
            case Builtin::KindDouble:
            {
                out << nl << name << " = 0.0;";
                break;
            }
            case Builtin::KindString:
            {
                out << nl << name << " = \"\";";
                break;
            }
            case Builtin::KindObject:
            {
                out << nl << name << " = null;";
                break;
            }
            case Builtin::KindObjectProxy:
            {
                out << nl << name << " = null;";
                break;
            }
            case Builtin::KindLocalObject:
            {
                out << nl << name << " = null;";
                break;
            }
        }
        return;
    }

    ProxyPtr prx = ProxyPtr::dynamicCast(type);
    if (prx)
    {
        out << nl << name << " = null;";
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if (cl)
    {
        out << nl << name << " = null;";
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if (st)
    {
        string typeS = getAbsolute(st->scoped(), scope);
        out << nl << name << " = new " << typeS << "();";
        DataMemberList members = st->dataMembers();
        DataMemberList::const_iterator d;
        for (d = members.begin(); d != members.end(); ++d)
        {
            string memberName = name + "." + fixKwd((*d)->name());
            writeAssign(out, scope, (*d)->type(), memberName, iter);
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if (en)
    {
        string typeS = getAbsolute(en->scoped(), scope);
        EnumeratorList enumerators = en->getEnumerators();
        out << nl << name << " = " << typeS << '.'
            << fixKwd(enumerators.front()->name()) << ';';
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if (seq)
    {
        //
        // Determine sequence depth
        //
        int depth = 0;
        TypePtr origContent = seq->type();
        SequencePtr s = SequencePtr::dynamicCast(origContent);
        while (s)
        {
            depth++;
            origContent = s->type();
            s = SequencePtr::dynamicCast(origContent);
        }

        string origContentS = typeToString(origContent, TypeModeIn, scope);
        out << nl << name << " = new " << origContentS << "[5]";
        while (depth--)
        {
            out << "[]";
        }
        out << ';';
        out << nl << "for (int __i" << iter << " = 0; __i" << iter << " < "
            << name << ".length; __i" << iter << "++)";
        out << sb;
        ostringstream elem;
        elem << name << "[__i" << iter << ']';
        iter++;
        writeAssign(out, scope, seq->type(), elem.str(), iter);
        out << eb;
        return;
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    assert(dict);
    out << nl << name << " = new java.util.HashMap();";
}

bool
Slice::Gen::ImplVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if (!p->isAbstract())
    {
        return false;
    }

    string name = fixKwd(p->name());
    string scoped = p->scoped();
    ClassList bases = p->bases();
    string scope = p->scope();
    string absolute = getAbsolute(scoped, "", "", "I");

    if (!open(absolute))
    {
        return false;
    }

    Output& out = output();

    out << sp << nl << "public final class " << name << 'I';
    if (p->isInterface())
    {
        if (p->isLocal())
        {
            out << " implements " << name;
        }
        else
        {
            out << " extends _" << name << "Disp";
        }
    }
    else
    {
        out << " extends " << name;
    }
    out << sb;

    out << nl << "public" << nl << name << "I()";
    out << sb;
    out << eb;

    OperationList ops = p->allOperations();

    OperationList::const_iterator r;
    for (r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = (*r);
        string opName = fixKwd(op->name());

        TypePtr ret = op->returnType();
        string retS = typeToString(ret, TypeModeReturn, scope);
        string params = getParams(op, scope);

        out << sp << nl << "public " << retS
            << nl << opName << "(" << params;
        if (!p->isLocal())
        {
            if (!params.empty())
            {
                out << ", ";
            }
            out << "Ice.Current current";
        }
        out << ')';

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();
        writeThrowsClause(scope, throws);

        out << sb;

        TypeStringList outParams = op->outputParameters();
        TypeStringList::const_iterator q;
        int iter = 0;

        //
        // Assign values to 'out' params
        //
        for (q = outParams.begin(); q != outParams.end(); ++q)
        {
            string param = fixKwd(q->second) + ".value";
            writeAssign(out, scope, q->first, param, iter);
        }

        //
        // Return value
        //
        if (ret)
        {
            out << sp << nl << retS << " __r;";
            writeAssign(out, scope, ret, "__r", iter);
            out << nl << "return __r;";
        }

        out << eb;
    }

    out << eb;
    close();

    return false;
}
