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
//#include <JavaUtil.h>
#include <limits>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;
using namespace Slice;

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
                path += dir.substr(start, pos);
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
Slice::JavaVisitor::getAbsolute(const string& scoped,
                                const string& scope) const
{
    string str = scoped;
    string::size_type pos = 0;

    if (!scope.empty())
    {
        //
        // Only remove the scope if the resulting symbol is unscoped.
        // For example:
        //
        // scope=::A, scoped=::A::B, result=B
        // scope=::A, scoped=::A::B::C, result=::A::B::C
        //
        if (str.compare(0, scope.size(), scope) == 0 &&
            str.find('.', scope.size()) == string::npos)
        {
            str.erase(0, scope.size());
        }
    }

    //
    // Skip leading "::"
    //
    if (str[0] == ':')
    {
        assert(str[1] == ':');
        str.erase(0, 2);
    }

    //
    // Convert all occurrences of "::" to "."
    //
    while ((pos = str.find(':', pos)) != string::npos)
    {
        assert(str[pos + 1] == ':');
        str.replace(pos, 2, ".");
    }

    if (!_package.empty())
    {
        return _package + "." + str;
    }
    else
    {
        return str;
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
        "String", // string
        "String", // wstring
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
        "Ice.StringHolder", // string
        "Ice.StringHolder", // wstring
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
        return getAbsolute(contained->scoped(), scope);
    }

    return "???";
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
}

Slice::Gen::TypesVisitor::TypesVisitor(const string& dir,
                                       const string& package) :
    JavaVisitor(dir, package)
{
}

bool
Slice::Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = p->name();
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
        out << sp << nl << "public abstract class " << name;
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

    if (!p->isInterface() && !p->isLocal())
    {
        // TODO: ids, _isA, dispatching, etc.
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
    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    string name = p->name();
    string scoped = p->scoped();
    string scope = p->scope();

    TypePtr ret = p->returnType();
    string retS = typeToString(ret, TypeModeReturn, scope);

    TypeStringList inParams = p->inputParameters();
    TypeStringList outParams = p->outputParameters();
    TypeStringList::const_iterator q;

    string params = "(";
    string args = "(";

    for (q = inParams.begin(); q != inParams.end(); ++q)
    {
        if (q != inParams.begin())
        {
            params += ", ";
            args += ", ";
        }

        string typeString = typeToString(q->first, TypeModeIn, scope);
        params += typeString;
        params += ' ';
        params += q->second;
        args += q->second;
    }

    for (q = outParams.begin(); q != outParams.end(); ++q)
    {
        if (q != outParams.begin() || !inParams.empty())
        {
            params += ", ";
            args += ", ";
        }

        string typeString = typeToString(q->first, TypeModeOut, scope);
        params += typeString;
        params += ' ';
        params += q->second;
        args += q->second;
    }

    params += ')';
    args += ')';

    Output& out = output();

    out << sp;
    out << nl;
    if (!cl->isInterface())
    {
        out << "public ";
    }
    out << retS << ' ' << name << params;

    //
    // Don't include local exceptions in the throws clause
    //
    ExceptionList throws = p->throws();
    throws.sort();
    throws.unique();
    int localCount = 0;
    count_if(throws.begin(), throws.end(),
             ::IceUtil::memFun(&Exception::isLocal), localCount);
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

    out << ';';
}

bool
Slice::Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string name = p->name();
    string scoped = p->scoped();
    ExceptionPtr base = p->base();
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

    out << sp << nl << "public String" << nl << "_name()";
    out << sb;
    out << nl << "return \"" << scoped.substr(2) << "\";";
    out << eb;

    // TODO - need _print() in Java? Already have printStackTrace()
#if 0
    if (p->isLocal())
    {
        H << nl << _dllExport << "virtual void _print(::std::ostream&) const;";
    }
#endif

    // TODO - need _clone()?
#if 0
    H << nl << _dllExport << "virtual ::Ice::Exception* _clone() const;";
    C << sp << nl << "::Ice::Exception*" << nl << scoped.substr(2) << "::_clone() const";
    C << sb;
    C << nl << "return new " << name << "(*this);";
    C << eb;

    H << nl << _dllExport << "virtual void _throw() const;";
    C << sp << nl << "void" << nl << scoped.substr(2) << "::_throw() const";
    C << sb;
    C << nl << "throw *this;";
    C << eb;
#endif

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
        string name = p->name();
        string scoped = p->scoped();

        ExceptionPtr base = p->base();

        TypeStringList memberList;
        DataMemberList dataMembers = p->dataMembers();
        for (DataMemberList::const_iterator q = dataMembers.begin();
             q != dataMembers.end();
             ++q)
        {
            memberList.push_back(make_pair((*q)->type(), (*q)->name()));
        }
        out << sp << nl << "public void" << nl
            << "__write(IceInternal.Stream __os)";
        out << sb;
        // TODO
        //writeMarshalCode(_out, memberList, 0);
        if (base)
        {
            out << nl << "super.__write(__os);";
        }
        out << eb;
        out << sp << nl << "public void" << nl
            << "__read(IceInternal.Stream __is)";
        out << sb;
        // TODO
        //writeUnmarshalCode(_out, memberList, 0);
        if (base)
        {
            out << nl << "super.__read(__os);";
        }
        out << eb;
    }

    out << eb;
    close();
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    string name = p->name();
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
    Output& out = output();
    out << eb;
    close();
}

void
Slice::Gen::TypesVisitor::visitDataMember(const DataMemberPtr& p)
{
    string name = p->name();
    ContainerPtr container = p->container();
    ContainedPtr contained = ContainedPtr::dynamicCast(container);
    string s = typeToString(p->type(), TypeModeMember, contained->scope());
    Output& out = output();
    out << sp << nl << "public " << s << ' ' << name << ';';
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string name = p->name();
    EnumeratorList enumerators = p->getEnumerators();
    string scoped = p->scoped();
    string absolute = getAbsolute(scoped);

    if (!open(absolute))
    {
        return;
    }

    Output& out = output();

    out << sp << nl << "public final class " << name;
    out << sb;
    EnumeratorList::const_iterator en = enumerators.begin();
    int n;
    for (en = enumerators.begin(), n = 0; en != enumerators.end(); ++en, ++n)
    {
        out << nl << "public static final long _" << (*en)->name() << " = "
            << n << ';';
        out << nl << "public static final " << name << ' ' << (*en)->name()
            << " = new " << name << "(_" << (*en)->name() << ");";
    }

    int sz = enumerators.size();

    out << sp << nl << "public static " << name << nl << "convert(long val)";
    out << sb;
    out << nl << "assert val < " << sz << ';';
    out << nl << "return __values[val];";
    out << eb;

    out << sp << nl << "public long" << nl << "value()";
    out << sb;
    out << nl << "return __value;";
    out << eb;

    out << sp << nl << "private" << nl << name << "(long val)";
    out << sb;
    out << nl << "__value = val;";
    out << nl << "__values[val] = this;";
    out << eb;
    out << sp << nl << "private static " << name << "[] __values = new "
        << name << "[" << sz << "];";
    out << sp << nl << "private long __value;";

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
    string name = contained->name();
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
    // TODO: Need helper for local classes?

    if (!p->isLocal())
    {
        string name = p->name();
        string absolute = getAbsolute(p->scoped());
        string helper = absolute + "Helper";

        if (open(helper))
        {
            Output& out = output();
            out << sp << nl << "public final class " << name << "Helper";
            out << sb;
            out << nl << "public static void" << nl << "write()";
            out << sb;
            out << eb;
            out << sp << nl << "public static void" << nl << "read()";
            out << sb;
            out << eb;
            out << eb;
            close();
        }
    }

    return false;
}

bool
Slice::Gen::HelperVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    // TODO: Need helper for local classes?

    if (!p->isLocal())
    {
        string absolute = getAbsolute(p->scoped());
        string helper = absolute + "Helper";

        if (open(helper))
        {
            string name = p->name();
            ExceptionPtr base = p->base();
#if 0
            string baseAbsolute;
            if (base)
            {
                baseAbsolute = getAbsolute(base->scoped(), p->scope());
            }
#endif

            Output& out = output();
            out << sp << nl << "public final class " << name << "Helper";
            out << sb;
            out << nl << "public static void" << nl << "write()";
            out << sb;
            out << eb;
            out << sp << nl << "public static void" << nl << "read()";
            out << sb;
            out << eb;
            out << eb;
            close();
        }
    }

    return false;
}

bool
Slice::Gen::HelperVisitor::visitStructStart(const StructPtr& p)
{
    string name = p->name();
    string absolute = getAbsolute(p->scoped());
    string helper = absolute + "Helper";

    if (open(helper))
    {
#if 0
        TypeStringList memberList;
        DataMemberList dataMembers = p->dataMembers();
        for (DataMemberList::const_iterator q = dataMembers.begin();
             q != dataMembers.end();
             ++q)
        {
            memberList.push_back(make_pair((*q)->type(), (*q)->name()));
        }
#endif

        Output& out = output();
        out << sp << nl << "public final class " << name << "Helper";
        out << sb;
        out << nl << "public static void" << nl << "write()";
        out << sb;
        out << eb;
        out << sp << nl << "public static void" << nl << "read()";
        out << sb;
        out << eb;
        out << eb;
        close();
    }

    return false;
}

void
Slice::Gen::HelperVisitor::visitSequence(const SequencePtr& p)
{
    string name = p->name();
    string absolute = getAbsolute(p->scoped());
    string helper = absolute + "Helper";

    if (open(helper))
    {
        Output& out = output();
        out << sp << nl << "public final class " << name << "Helper";
        out << sb;
        out << nl << "public static void" << nl << "write()";
        out << sb;
        out << eb;
        out << sp << nl << "public static void" << nl << "read()";
        out << sb;
        out << eb;
        out << eb;
        close();
    }
}

void
Slice::Gen::HelperVisitor::visitDictionary(const DictionaryPtr& p)
{
    string name = p->name();
    string absolute = getAbsolute(p->scoped());
    string helper = absolute + "Helper";

    if (open(helper))
    {
        Output& out = output();
        out << sp << nl << "public final class " << name << "Helper";
        out << sb;
        out << nl << "public static void" << nl << "write()";
        out << sb;
        out << eb;
        out << sp << nl << "public static void" << nl << "read()";
        out << sb;
        out << eb;
        out << eb;
        close();
    }
}

void
Slice::Gen::HelperVisitor::visitEnum(const EnumPtr& p)
{
    string name = p->name();
    string absolute = getAbsolute(p->scoped());
    string helper = absolute + "Helper";
    EnumeratorList enumerators = p->getEnumerators();
    int sz = enumerators.size();

    if (open(helper))
    {
        Output& out = output();
        out << sp << nl << "public final class " << name << "Helper";
        out << sb;

        //
        // write
        //
        out << sp << nl << "public static void" << nl
            << "write(Ice.Stream _ice_os, " << name << " _ice_v)";
        out << sb;
        if (sz <= 0x7f)
        {
            out << nl << "_ice_os.writeByte((byte)_ice_v.value());";
        }
        else if (sz <= 0x7fff)
        {
            out << nl << "_ice_os.writeShort((short)_ice_v.value());";
        }
        else if (sz <= 0x7fffffff)
        {
            out << nl << "_ice_os.writeInt((int)_ice_v.value());";
        }
        else
        {
            out << nl << "_ice_os.writeLong(_ice_v.value());";
        }
        out << eb;

        //
        // read
        //
        out << sp << nl << "public static " << name << nl
            << "read(Ice.Stream _ice_is)";
        out << sb;
        if (sz <= 0x7f)
        {
            out << nl << "long _ice_v = _ice_is.readByte();";
        }
        else if (sz <= 0x7fff)
        {
            out << nl << "long _ice_v = _ice_is.readShort();";
        }
        else if (sz <= 0x7fffffff)
        {
            out << nl << "long _ice_v = _ice_is.readInt();";
        }
        else
        {
            out << nl << "long _ice_v = _ice_is.readLong();";
        }
        out << nl << "return " << name << ".convert(_ice_v);";
        out << eb;

        out << eb;
        close();
    }
}
