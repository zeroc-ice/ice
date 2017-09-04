// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/CtrlCHandler.h>
#include <IceUtil/IceUtil.h>
#include <IceUtil/InputUtil.h>
#include <IceUtil/Options.h>
#include <IceUtil/OutputUtil.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/ConsoleUtil.h>
#include <IceUtil/FileUtil.h>
#include <Slice/Checksum.h>
#include <Slice/Preprocessor.h>
#include <Slice/FileTracker.h>
#include <Slice/Parser.h>
#include <Slice/Util.h>
#include <cstring>
#include <climits>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#  include <direct.h>
#else
#  include <unistd.h>
#endif

using namespace std;
using namespace Slice;
using namespace IceUtilInternal;

namespace
{

string
lowerCase(const string& s)
{
    string result(s);
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

string
lookupKwd(const string& name)
{
    //
    // Keyword list. *Must* be kept in alphabetical order.
    //
    static const string keywordList[] =
    {
        "break", "case", "catch", "classdef", "continue", "else", "elseif", "end", "for", "function", "global",
        "if", "otherwise", "parfor", "persistent", "return", "spmd", "switch", "try", "while"
    };
    bool found =  binary_search(&keywordList[0],
                                &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
                                name);
    return found ? "slice_" + name : name;
}

//
// Split a scoped name into its components and return the components as a list of (unscoped) identifiers.
//
vector<string>
splitScopedName(const string& scoped)
{
    assert(scoped[0] == ':');
    vector<string> ids;
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

string
fixIdent(const string& ident)
{
    if(ident[0] != ':')
    {
        return lookupKwd(ident);
    }
    vector<string> ids = splitScopedName(ident);
    transform(ids.begin(), ids.end(), ids.begin(), ptr_fun(lookupKwd));
    stringstream result;
    for(vector<string>::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        result << "::" + *i;
    }
    return result.str();
}

string
replace(string s, string patt, string val)
{
    string r = s;
    string::size_type pos = r.find(patt);
    while(pos != string::npos)
    {
        r.replace(pos, patt.size(), val);
        pos += val.size();
        pos = r.find(patt, pos);
    }
    return r;
}

string
scopedToName(const string& scoped)
{
    string str = scoped;
    if(str.find("::") == 0)
    {
        str.erase(0, 2);
    }

    str = replace(str, "::", ".");

    return fixIdent(str);
}

//
// Get the fully-qualified name of the given definition. If a suffix is provided,
// it is prepended to the definition's unqualified name. If the nameSuffix
// is provided, it is appended to the container's name.
//
string
getAbsolute(const ContainedPtr& cont, const string& pfx = std::string(), const string& suffix = std::string())
{
    return scopedToName(cont->scope() + pfx + cont->name() + suffix);
}

void
printHeader(IceUtilInternal::Output& out)
{
    static const char* header =
        "%{\n"
        "**********************************************************************\n"
        "\n"
        "Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.\n"
        "\n"
        "This copy of Ice is licensed to you under the terms described in the\n"
        "ICE_LICENSE file included in this distribution.\n"
        "\n"
        "**********************************************************************\n"
        "%}\n"
        ;

    out << header;
    out << "%\n";
    out << "% Ice version " << ICE_STRING_VERSION << "\n";
    out << "%\n";
}

string
typeToString(const TypePtr& type)
{
    static const char* builtinTable[] =
    {
        "uint8",
        "logical",
        "int16",
        "int32",
        "int64",
        "single",
        "double",
        "char",
        "Ice.Object",
        "Ice.ObjectPrx",
        "",
        "Ice.Value"
    };

    if(!type)
    {
        return "void";
    }

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        return builtinTable[builtin->kind()];
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        return getAbsolute(cl);
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        return getAbsolute(proxy->_class(), "", "Prx");
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        return "containers.Map";
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        return "???";
    }

    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
        return getAbsolute(contained);
    }

    return "???";
}

string
dictionaryTypeToString(const TypePtr& type, bool key)
{
    assert(type);

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindBool:
            case Builtin::KindByte:
            case Builtin::KindShort:
            {
                //
                // containers.Map supports a limited number of key types.
                //
                return key ? "int32" : typeToString(type);
            }
            case Builtin::KindInt:
            case Builtin::KindLong:
            case Builtin::KindString:
            {
                return typeToString(type);
            }
            case Builtin::KindFloat:
            case Builtin::KindDouble:
            {
                assert(!key);
                return typeToString(type);
            }
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindLocalObject:
            case Builtin::KindValue:
            {
                assert(!key);
                return "any";
            }
            default:
            {
                return "???";
            }
        }
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        //
        // containers.Map doesn't natively support enumerators as keys but we can work around it using int32.
        //
        return key ? "int32" : "any";
    }

    return "any";
}

bool
declarePropertyType(const TypePtr& type, bool optional)
{
    return !optional && !SequencePtr::dynamicCast(type) && !ProxyPtr::dynamicCast(type) &&
        !ClassDefPtr::dynamicCast(type);
}

string
constantValue(const TypePtr& type, const SyntaxTreeBasePtr& valueType, const string& value)
{
    ConstPtr constant = ConstPtr::dynamicCast(valueType);
    if(constant)
    {
        return getAbsolute(constant) + ".value";
    }
    else
    {
        BuiltinPtr bp;
        if((bp = BuiltinPtr::dynamicCast(type)))
        {
            switch(bp->kind())
            {
                case Builtin::KindString:
                {
                    return "sprintf('" + toStringLiteral(value, "\a\b\f\n\r\t\v", "", Matlab, 255) + "')";
                }
                case Builtin::KindBool:
                case Builtin::KindByte:
                case Builtin::KindShort:
                case Builtin::KindInt:
                case Builtin::KindLong:
                case Builtin::KindFloat:
                case Builtin::KindDouble:
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                case Builtin::KindValue:
                {
                    return value;
                }

                default:
                {
                    return "???";
                }
            }

        }
        else if(EnumPtr::dynamicCast(type))
        {
            EnumeratorPtr e = EnumeratorPtr::dynamicCast(valueType);
            assert(e);
            return getAbsolute(e);
        }
        else
        {
            return value;
        }
    }
}

string
defaultValue(const DataMemberPtr& m)
{
    if(m->defaultValueType())
    {
        return constantValue(m->type(), m->defaultValueType(), m->defaultValue());
    }
    else if(m->optional())
    {
        return "[]";
    }
    else
    {
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(m->type());
        if(builtin)
        {
            switch(builtin->kind())
            {
                case Builtin::KindString:
                    return "''";
                case Builtin::KindBool:
                    return "false";
                case Builtin::KindByte:
                case Builtin::KindShort:
                case Builtin::KindInt:
                case Builtin::KindLong:
                case Builtin::KindFloat:
                case Builtin::KindDouble:
                    return "0";
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                case Builtin::KindValue:
                    return "[]";
            }
        }

        DictionaryPtr dict = DictionaryPtr::dynamicCast(m->type());
        if(dict)
        {
            return "containers.Map('KeyType', '" + typeToString(dict->keyType()) + "', 'ValueType', '" +
                typeToString(dict->valueType()) + "')";
        }

        return "[]";
    }
}

}

//
// CodeVisitor generates the Matlab mapping for a translation unit.
//
class CodeVisitor : public ParserVisitor
{
public:

    CodeVisitor(const string&);

    virtual bool visitClassDefStart(const ClassDefPtr&);
    virtual bool visitExceptionStart(const ExceptionPtr&);
    virtual bool visitStructStart(const StructPtr&);
    virtual void visitSequence(const SequencePtr&);
    virtual void visitDictionary(const DictionaryPtr&);
    virtual void visitEnum(const EnumPtr&);
    virtual void visitConst(const ConstPtr&);

private:

    void openClass(const string&, IceUtilInternal::Output&);

    struct MemberInfo
    {
        string fixedName;
        bool inherited;
        DataMemberPtr dataMember;
    };
    typedef list<MemberInfo> MemberInfoList;

    //
    // Convert an operation mode into a string.
    //
    string getOperationMode(Slice::Operation::Mode);

    void collectClassMembers(const ClassDefPtr&, MemberInfoList&, bool);
    void collectExceptionMembers(const ExceptionPtr&, MemberInfoList&, bool);

    struct ParamInfo
    {
        string fixedName;
        TypePtr type;
        bool optional;
        int tag;
        ParamDeclPtr param; // 0 == return value
    };
    typedef list<ParamInfo> ParamInfoList;

    ParamInfoList getInParams(const OperationPtr&);
    ParamInfoList getOutParams(const OperationPtr&);

    string getOptionalFormat(const TypePtr&);
    string getFormatType(FormatType);

    void marshal(IceUtilInternal::Output&, const string&, const string&, const TypePtr&, bool, int, int&);
    void unmarshal(IceUtilInternal::Output&, const string&, const string&, const TypePtr&, bool, int, int&);

    const string _dir;
};

//
// CodeVisitor implementation.
//
CodeVisitor::CodeVisitor(const string& dir) :
    _dir(dir)
{
}

bool
CodeVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    const string name = fixIdent(p->name());
    const string scoped = p->scoped();
    const ClassList bases = p->bases();
    const string self = name == "obj" ? "this" : "obj";

    if(p->hasMetaData("matlab:internal"))
    {
        return false;
    }

    if(!p->isInterface())
    {
        ClassDefPtr base;
        if(!bases.empty() && !bases.front()->isInterface())
        {
            base = bases.front();
        }

        IceUtilInternal::Output out;
        openClass(scoped, out);

        out << nl << "classdef " << name;
        if(base)
        {
            out << " < " << getAbsolute(base);
        }
        else if(!p->isLocal())
        {
            out << " < Ice.Value";
        }

        const DataMemberList members = p->dataMembers();
        if(!members.empty())
        {
            out.inc();
            out << nl << "properties";
            out.inc();
            for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
            {
                out << nl << fixIdent((*q)->name());
                if(declarePropertyType((*q)->type(), (*q)->optional()))
                {
                    out << " " << typeToString((*q)->type());
                }
            }
            out.dec();
            out << nl << "end";
        }

        MemberInfoList allMembers;
        collectClassMembers(p, allMembers, false);

        if(!allMembers.empty() || !p->isLocal())
        {
            out << nl << "methods";
            out.inc();
        }

        if(!allMembers.empty())
        {
            vector<string> allNames;
            for(MemberInfoList::const_iterator q = allMembers.begin(); q != allMembers.end(); ++q)
            {
                allNames.push_back(q->fixedName);
            }
            //
            // Constructor
            //
            out << nl << "function " << self << " = " << name << spar << allNames << epar;
            out.inc();
            out << nl << "if nargin == 0";
            out.inc();
            for(MemberInfoList::const_iterator q = allMembers.begin(); q != allMembers.end(); ++q)
            {
                out << nl << q->fixedName << " = " << defaultValue(q->dataMember) << ';';
            }
            out.dec();
            out << nl << "end";
            if(base || !p->isLocal())
            {
                out << nl << self << " = " << self << "@" << (base ? getAbsolute(base) : "Ice.Value") << spar;
                for(MemberInfoList::const_iterator q = allMembers.begin(); q != allMembers.end(); ++q)
                {
                    if(q->inherited)
                    {
                        out << q->fixedName;
                    }
                }
                out << epar << ';';
            }
            for(MemberInfoList::const_iterator q = allMembers.begin(); q != allMembers.end(); ++q)
            {
                if(!q->inherited)
                {
                    out << nl << self << "." << q->fixedName << " = " << q->fixedName << ';';
                }
            }
            out.dec();
            out << nl << "end";
        }

        if(!p->isLocal())
        {
            const DataMemberList optionalMembers = p->orderedOptionalDataMembers();

            out << nl << "function id = ice_id(obj)";
            out.inc();
            out << nl << "id = obj.ice_staticId();";
            out.dec();
            out << nl << "end";
            out << nl << "function iceWriteImpl_(obj, os)";
            out.inc();
            out << nl << "os.startSlice(obj.ice_staticId(), " << p->compactId() << (!base ? ", true" : ", false")
                << ");";
            for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
            {
                if(!(*d)->optional())
                {
                    int idx = 0;
                    marshal(out, "os", "obj." + fixIdent((*d)->name()), (*d)->type(), false, 0, idx);
                }
            }
            for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
            {
                int idx = 0;
                marshal(out, "os", "obj." + fixIdent((*d)->name()), (*d)->type(), true, (*d)->tag(), idx);
            }
            out << nl << "os.endSlice();";
            if(base)
            {
                out << nl << "iceWriteImpl_@" << getAbsolute(base) << "(obj);";
            }
            out.dec();
            out << nl << "end";
            out << nl << "function obj = iceReadImpl_(obj, is)";
            out.inc();
            out << nl << "is.startSlice();";
            for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
            {
                if(!(*d)->optional())
                {
                    int idx = 0;
                    unmarshal(out, "is", "obj." + fixIdent((*d)->name()), (*d)->type(), false, 0, idx);
                }
            }
            for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
            {
                int idx = 0;
                unmarshal(out, "is", "obj." + fixIdent((*d)->name()), (*d)->type(), true, (*d)->tag(), idx);
            }
            out << nl << "os.endSlice();";
            if(base)
            {
                out << nl << "obj = iceReadImpl_@" << getAbsolute(base) << "(obj);";
            }
            out.dec();
            out << nl << "end";
        }

        if(!allMembers.empty() || !p->isLocal())
        {
            out.dec();
            out << nl << "end";
        }

        if(!p->isLocal())
        {
            out << nl << "methods(Static)";
            out.inc();
            out << nl << "function id = ice_staticId()";
            out.inc();
            out << nl << "id = '" << scoped << "';";
            out.dec();
            out << nl << "end";
            out.dec();
            out << nl << "end";
            out.dec();
        }

        out.dec();
        out << nl << "end";
        out << nl;

        out.close();
    }
    else if(!p->isLocal())
    {
        //
        // Generate proxy class.
        //

        IceUtilInternal::Output out;
        openClass(scoped + "Prx", out);

        const string prxName = name + "Prx";
        const string abs = getAbsolute(p, "", "Prx");

        out << nl << "classdef " << prxName << " < ";
        if(!bases.empty())
        {
            for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
            {
                if(q != bases.begin())
                {
                    out << " & ";
                }
                out << getAbsolute(*q, "", "Prx");
            }
        }
        else
        {
            out << "Ice.ObjectPrx";
        }

        out.inc();

        out << nl << "methods";
        out.inc();

        //
        // Constructor.
        //
        out << nl << "function obj = " << prxName << "(impl)";
        out.inc();
        if(!bases.empty())
        {
            for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
            {
                out << nl << "obj = obj@" << getAbsolute(*q, "", "Prx") << "(impl);";
            }
        }
        else
        {
            out << nl << "obj = obj@Ice.ObjectPrx(impl);";
        }
        out.dec();
        out << nl << "end";

        //
        // Operations.
        //
        const OperationList ops = p->operations();
        for(OperationList::const_iterator q = ops.begin(); q != ops.end(); ++q)
        {
            OperationPtr op = *q;
            const ParamInfoList inParams = getInParams(op);
            const ParamInfoList outParams = getOutParams(op);
            const bool twowayOnly = !outParams.empty();

            ExceptionList exceptions = op->throws();
            exceptions.sort();
            exceptions.unique();

            //
            // Arrange exceptions into most-derived to least-derived order. If we don't
            // do this, a base exception handler can appear before a derived exception
            // handler, causing compiler warnings and resulting in the base exception
            // being marshaled instead of the derived exception.
            //
#if defined(__SUNPRO_CC)
            exceptions.sort(Slice::derivedToBaseCompare);
#else
            exceptions.sort(Slice::DerivedToBaseCompare());
#endif

            //
            // Ensure no parameter is named "obj".
            //
            string self = "obj";
            for(ParamInfoList::const_iterator r = outParams.begin(); r != outParams.end(); ++r)
            {
                if(r->fixedName == "obj")
                {
                    self = "obj_";
                }
            }
            for(ParamInfoList::const_iterator r = inParams.begin(); r != inParams.end(); ++r)
            {
                if(r->fixedName == "obj")
                {
                    self = "obj_";
                }
            }

            //
            // Synchronous method.
            //
            out << nl << "function ";
            if(outParams.size() > 1)
            {
                out << "[";
                for(ParamInfoList::const_iterator r = outParams.begin(); r != outParams.end(); ++r)
                {
                    if(r != outParams.begin())
                    {
                        out << ", ";
                    }
                    out << r->fixedName;
                }
                out << "] = ";
            }
            else if(outParams.size() == 1)
            {
                out << outParams.begin()->fixedName << " = ";
            }
            out << fixIdent(op->name()) << spar;

            out << self;
            for(ParamInfoList::const_iterator r = inParams.begin(); r != inParams.end(); ++r)
            {
                out << r->fixedName;
            }
            out << "varargin"; // For the optional context
            out << epar;
            out.inc();

            if(!inParams.empty())
            {
                if(op->format() == DefaultFormat)
                {
                    out << nl << "os_ = " << self << ".startWriteParams_();";
                }
                else
                {
                    out << nl << "os_ = " << self << ".startWriteParamsWithFormat_(" << getFormatType(op->format())
                        << ");";
                }
                for(ParamInfoList::const_iterator r = inParams.begin(); r != inParams.end(); ++r)
                {
                    int idx = 0;
                    marshal(out, "os_", r->fixedName, r->type, r->optional, r->tag, idx);
                }
                out << nl << self << ".endWriteParams_(os_);";
            }

            out << nl << "[ok_, is_] = " << self << ".invoke_('" << op->name() << "', '"
                << getOperationMode(op->sendMode()) << "', " << (twowayOnly ? "true" : "false")
                << ", " << (inParams.empty() ? "[]" : "os_") << ", varargin{:});";

            if(outParams.empty() && exceptions.empty())
            {
                out << nl << self << ".checkNoResponse_(ok_, is_);";
            }
            else
            {
                out << nl << "if ok_";
                out.inc();
                if(outParams.empty())
                {
                    out << nl << "is_.skipEmptyEncapsulation();";
                }
                else
                {
                    out << nl << "is_.startEncapsulation();";
                    //
                    // The return value (if any) appears first. We have to unmarshal any out parameters
                    // before the return value.
                    //
                    for(ParamInfoList::const_iterator r = outParams.begin(); r != outParams.end(); ++r)
                    {
                        if(r->param)
                        {
                            int idx = 0;
                            unmarshal(out, "is_", r->fixedName, r->type, r->optional, r->tag, idx);
                        }
                    }
                    //
                    // Now do the return value if necessary.
                    //
                    if(!outParams.begin()->param)
                    {
                        ParamInfoList::const_iterator r = outParams.begin();
                        int idx = 0;
                        unmarshal(out, "is_", r->fixedName, r->type, r->optional, r->tag, idx);
                    }
                    out << nl << "is_.endEncapsulation();";
                }
                out.dec();
                out << nl << "else";
                out.inc();
                out << nl << self << ".throwUserException_" << spar << "is_";
                for(ExceptionList::const_iterator e = exceptions.begin(); e != exceptions.end(); ++e)
                {
                    out << "'" + getAbsolute(*e) + "'";
                }
                out << epar << ';';
                out.dec();
                out << nl << "end";
            }

            out.dec();
            out << nl << "end";

            //
            // Asynchronous method.
            //
            out << nl << "function r_ = " << fixIdent(op->name()) << "Async" << spar;
            out << self;
            for(ParamInfoList::const_iterator r = inParams.begin(); r != inParams.end(); ++r)
            {
                out << r->fixedName;
            }
            out << "varargin"; // For the optional context
            out << epar;
            out.inc();

            if(!inParams.empty())
            {
                if(op->format() == DefaultFormat)
                {
                    out << nl << "os_ = " << self << ".startWriteParams_();";
                }
                else
                {
                    out << nl << "os_ = " << self << ".startWriteParamsWithFormat_(" << getFormatType(op->format())
                        << ");";
                }
                for(ParamInfoList::const_iterator r = inParams.begin(); r != inParams.end(); ++r)
                {
                    int idx = 0;
                    marshal(out, "os_", r->fixedName, r->type, r->optional, r->tag, idx);
                }
                out << nl << self << ".endWriteParams_(os_);";
            }

            if(!outParams.empty() || !exceptions.empty())
            {
                out << nl << "function varargout = unmarshal(ok_, is_)";
                out.inc();
                out << nl << "if ok_";
                out.inc();
                if(outParams.empty())
                {
                    out << nl << "is_.skipEmptyEncapsulation();";
                }
                else
                {
                    out << nl << "is_.startEncapsulation();";
                    int pos = op->returnType() ? 2 : 1;
                    //
                    // The return value (if any) appears first. We have to unmarshal any out parameters
                    // before the return value.
                    //
                    for(ParamInfoList::const_iterator r = outParams.begin(); r != outParams.end(); ++r)
                    {
                        if(r->param)
                        {
                            int idx = 0;
                            unmarshal(out, "is_", r->fixedName, r->type, r->optional, r->tag, idx);
                            out << nl << "varargout{" << pos++ << "} = " << r->fixedName << ';';
                        }
                    }
                    //
                    // Now do the return value if necessary.
                    //
                    if(!outParams.begin()->param)
                    {
                        ParamInfoList::const_iterator r = outParams.begin();
                        int idx = 0;
                        unmarshal(out, "is_", r->fixedName, r->type, r->optional, r->tag, idx);
                        out << nl << "varargout{1} = " << r->fixedName << ';';
                    }
                    out << nl << "is_.endEncapsulation();";
                }
                out.dec();
                out << nl << "else";
                out.inc();
                out << nl << self << ".throwUserException_" << spar << "is_";
                for(ExceptionList::const_iterator e = exceptions.begin(); e != exceptions.end(); ++e)
                {
                    out << "'" + getAbsolute(*e) + "'";
                }
                out << epar << ';';
                out.dec();
                out << nl << "end";
                out.dec();
                out << nl << "end";
            }

            out << nl << "r_ = " << self << ".invokeAsync_('" << op->name() << "', '"
                << getOperationMode(op->sendMode()) << "', " << (twowayOnly ? "true" : "false") << ", "
                << (inParams.empty() ? "[]" : "os_") << ", " << outParams.size() << ", ";
            if(!outParams.empty() || !exceptions.empty())
            {
                out << "@unmarshal";
            }
            else
            {
                out << "[]";
            }
            out << ", varargin{:});";

            out.dec();
            out << nl << "end";
        }

        out.dec();
        out << nl << "end";

        out << nl << "methods(Static)";
        out.inc();
        out << nl << "function id = ice_staticId()";
        out.inc();
        out << nl << "id = '" << scoped << "';";
        out.dec();
        out << nl << "end";
        out << nl << "function r = ice_read(is_)";
        out.inc();
        out << nl << "r = Ice.ObjectPrx.read_(is_, '" << abs << "');";
        out.dec();
        out << nl << "end";
        out << nl << "function r = checkedCast(p, varargin)";
        out.inc();
        out << nl << "r = Ice.ObjectPrx.checkedCast_(p, " << abs << ".ice_staticId(), '" << abs << "', varargin{:});";
        out.dec();
        out << nl << "end";
        out << nl << "function r = uncheckedCast(p, varargin)";
        out.inc();
        out << nl << "r = Ice.ObjectPrx.uncheckedCast_(p, '" << abs << "', varargin{:});";
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";

        out.dec();
        out << nl << "end";
        out << nl;

        out.close();
    }
    else
    {
        //
        // Generate local abstract class.
        //

        IceUtilInternal::Output out;
        openClass(scoped, out);

        out << nl << "classdef (Abstract) " << name;
        if(!bases.empty())
        {
            out << " < ";
            for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
            {
                if(q != bases.begin())
                {
                    out << " & ";
                }
                out << " < " << getAbsolute(*q);
            }
        }

        const OperationList ops = p->operations();
        if(!ops.empty())
        {
            out.inc();
            out << nl << "methods(Abstract)";
            out.inc();
            for(OperationList::const_iterator q = ops.begin(); q != ops.end(); ++q)
            {
                OperationPtr op = *q;
                const ParamInfoList outParams = getOutParams(op);
                out << nl;
                if(outParams.size() > 1)
                {
                    out << "[";
                    for(ParamInfoList::const_iterator r = outParams.begin(); r != outParams.end(); ++r)
                    {
                        if(r != outParams.begin())
                        {
                            out << ", ";
                        }
                        out << r->fixedName;
                    }
                    out << "] = ";
                }
                else if(outParams.size() == 1)
                {
                    out << outParams.begin()->fixedName << " = ";
                }
                out << fixIdent(op->name()) << spar;
                string self = "obj";
                const ParamInfoList inParams = getInParams(op);
                for(ParamInfoList::const_iterator r = outParams.begin(); r != outParams.end(); ++r)
                {
                    if(r->fixedName == "obj")
                    {
                        self = "obj_";
                    }
                }
                for(ParamInfoList::const_iterator r = inParams.begin(); r != inParams.end(); ++r)
                {
                    if(r->fixedName == "obj")
                    {
                        self = "obj_";
                    }
                }
                out << self;
                for(ParamInfoList::const_iterator r = inParams.begin(); r != inParams.end(); ++r)
                {
                    out << r->fixedName;
                }
                out << epar;
            }
            out.dec();
            out << nl << "end";
            out.dec();
        }

        out << nl << "end";
        out << nl;

        out.close();
    }

    return false;
}

bool
CodeVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    const string name = fixIdent(p->name());
    const string scoped = p->scoped();

    IceUtilInternal::Output out;
    openClass(scoped, out);

    ExceptionPtr base = p->base();

    out << nl << "classdef " << name;
    if(base)
    {
        out << " < " << getAbsolute(base);
    }
    else if(p->isLocal())
    {
        out << " < Ice.LocalException";
    }
    else
    {
        out << " < Ice.UserException";
    }

    const DataMemberList members = p->dataMembers();
    if(!members.empty())
    {
        out.inc();
        out << nl << "properties";
        out.inc();
        for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
        {
            out << nl << fixIdent((*q)->name());
            if(declarePropertyType((*q)->type(), (*q)->optional()))
            {
                out << " " << typeToString((*q)->type());
            }
        }
        out.dec();
        out << nl << "end";
    }

    MemberInfoList allMembers;
    collectExceptionMembers(p, allMembers, false);

    vector<string> allNames;
    for(MemberInfoList::const_iterator q = allMembers.begin(); q != allMembers.end(); ++q)
    {
        allNames.push_back(q->fixedName);
    }
    out << nl << "methods";
    out.inc();

    const string self = name == "obj" ? "this" : "obj";

    //
    // Constructor
    //
    out << nl << "function " << self << " = " << name << spar << "ice_exid" << "ice_exmsg" << allNames << epar;
    out.inc();
    string exid = getAbsolute(p);
    const string exmsg = getAbsolute(p); // TODO: Allow a message to be specified via metadata?
    //
    // The ID argument must use colon separators.
    //
    string::size_type pos = exid.find('.');
    assert(pos != string::npos);
    while(pos != string::npos)
    {
        exid[pos] = ':';
        pos = exid.find('.', pos);
    }

    if(!allMembers.empty())
    {
        out << nl << "if nargin <= 2";
        out.inc();
        for(MemberInfoList::const_iterator q = allMembers.begin(); q != allMembers.end(); ++q)
        {
            out << nl << q->fixedName << " = " << defaultValue(q->dataMember) << ';';
        }
        out.dec();
        out << nl << "end";
    }

    out << nl << "if nargin == 0 || isempty(ice_exid)";
    out.inc();
    out << nl << "ice_exid = '" << exid << "';";
    out.dec();
    out << nl << "end";

    out << nl << "if nargin < 2 || isempty(ice_exmsg)";
    out.inc();
    out << nl << "ice_exmsg = '" << exmsg << "';";
    out.dec();
    out << nl << "end";

    if(!base)
    {
        out << nl << self << " = " << self << "@" << (p->isLocal() ? "Ice.LocalException" : "Ice.UserException")
            << spar << "ice_exid" << "ice_exmsg" << epar << ';';
    }
    else
    {
        out << nl << self << " = " << self << "@" << getAbsolute(base) << spar << "ice_exid" << "ice_exmsg";
        for(MemberInfoList::const_iterator q = allMembers.begin(); q != allMembers.end(); ++q)
        {
            if(q->inherited)
            {
                out << q->fixedName;
            }
        }
        out << epar << ';';
    }
    for(MemberInfoList::const_iterator q = allMembers.begin(); q != allMembers.end(); ++q)
    {
        if(!q->inherited)
        {
            out << nl << self << "." << q->fixedName << " = " << q->fixedName << ';';
        }
    }
    out.dec();
    out << nl << "end";

    out << nl << "function id = ice_id(obj)";
    out.inc();
    out << nl << "id = '" << scoped << "';";
    out.dec();
    out << nl << "end";

    out << nl << "function obj = readImpl_(obj, is_)";
    out.inc();
    out << nl << "is_.startSlice();";
    for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
    {
        int idx = 0;
        unmarshal(out, "is_", "obj." + fixIdent((*q)->name()), (*q)->type(), false, 0, idx);
    }
    out << nl << "is_.endSlice();";
    if(base)
    {
        out << nl << "obj = readImpl_@" << getAbsolute(base) << "(obj);";
    }
    out.dec();
    out << nl << "end";

    if(p->usesClasses(false))
    {
        if(!base || (base && !base->usesClasses(false)))
        {
            out << nl << "function r = usesClasses_(obj)";
            out.inc();
            out << nl << "r = true;";
            out.dec();
            out << nl << "end";
        }
    }

    out.dec();
    out << nl << "end";

    out.dec();
    out << nl << "end";
    out << nl;

    out.close();

    return false;
}

bool
CodeVisitor::visitStructStart(const StructPtr& p)
{
    const string name = fixIdent(p->name());
    const string scoped = p->scoped();
    const string abs = getAbsolute(p);

    IceUtilInternal::Output out;
    openClass(scoped, out);

    out << nl << "classdef " << name;

    const DataMemberList members = p->dataMembers();
    out.inc();
    out << nl << "properties";
    out.inc();
    vector<string> memberNames;
    for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
    {
        const string m = fixIdent((*q)->name());
        memberNames.push_back(m);
        out << nl << m;
        if(declarePropertyType((*q)->type(), false))
        {
            out << " " << typeToString((*q)->type());
        }
    }
    out.dec();
    out << nl << "end";

    out << nl << "methods";
    out.inc();
    string self = name == "obj" ? "this" : "obj";
    out << nl << "function " << self << " = " << name << spar << memberNames << epar;
    out.inc();
    out << nl << "if nargin > 0";
    out.inc();
    for(vector<string>::const_iterator q = memberNames.begin(); q != memberNames.end(); ++q)
    {
        out << nl << self << "." << *q << " = " << *q << ';';
    }
    out.dec();
    out << nl << "else";
    out.inc();
    for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
    {
        out << nl << self << "." << fixIdent((*q)->name()) << " = " << defaultValue(*q) << ';';
    }
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    out.dec();
    out << nl << "end";

    if(!p->isLocal())
    {
        out << nl << "methods(Static)";
        out.inc();
        out << nl << "function r = ice_read(is_)";
        out.inc();
        out << nl << "r = " << abs << "();";
        for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
        {
            int idx = 0;
            unmarshal(out, "is_", "r." + fixIdent((*q)->name()), (*q)->type(), false, 0, idx);
        }
        out.dec();
        out << nl << "end";
        out << nl << "function ice_write(os_, v_)";
        out.inc();
        out << nl << "if isempty(v_)";
        out.inc();
        out << nl << "v_ = " << abs << "();";
        out.dec();
        out << nl << "end";
        for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
        {
            int idx = 0;
            marshal(out, "os_", "v_." + fixIdent((*q)->name()), (*q)->type(), false, 0, idx);
        }
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";
    }

    out.dec();
    out << nl << "end";
    out << nl;

    out.close();

    return false;
}

void
CodeVisitor::visitSequence(const SequencePtr& p)
{
    if(p->isLocal())
    {
        return;
    }

    const TypePtr content = p->type();

    const BuiltinPtr bp = BuiltinPtr::dynamicCast(content);
    if(bp)
    {
        switch(bp->kind())
        {
            case Builtin::KindBool:
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindLong:
            case Builtin::KindFloat:
            case Builtin::KindDouble:
            case Builtin::KindString:
            {
                return;
            }
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindValue:
            {
                break;
            }
            case Builtin::KindLocalObject:
            {
                assert(false);
            }
        }
    }

    const string name = fixIdent(p->name());
    const string scoped = p->scoped();

    IceUtilInternal::Output out;
    openClass(scoped, out);

    out << nl << "classdef " << name;
    out.inc();
    out << nl << "methods(Static)";
    out.inc();

    out << nl << "function write(os, seq)";
    out.inc();
    out << nl << "sz = length(seq);";
    out << nl << "os.writeSize(sz);";
    out << nl << "for i = 1:sz";
    out.inc();
    int idx = 0;
    marshal(out, "os", "seq{i}", content, false, 0, idx);
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    out << nl << "function r = read(is)";
    out.inc();
    out << nl << "sz = is.readSize();";
    out << nl << "r = {};";
    out << nl << "for i = 1:sz";
    out.inc();
    idx = 0;
    unmarshal(out, "is", "r{i}", content, false, 0, idx);
    out.dec();
    out << nl << "end";

    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";
    out << nl;

    out.close();
}

void
CodeVisitor::visitDictionary(const DictionaryPtr& p)
{
    const TypePtr key = p->keyType();
    const TypePtr value = p->valueType();

    const StructPtr st = StructPtr::dynamicCast(key);

    const string name = fixIdent(p->name());
    const string scoped = p->scoped();
    const string abs = getAbsolute(p);
    const string self = name == "obj" ? "this" : "obj";

    IceUtilInternal::Output out;
    openClass(scoped, out);

    out << nl << "classdef " << name;
    out.inc();
    out << nl << "methods(Access=private)";
    out.inc();
    //
    // Declare a private constructor so that programs can't instantiate this type. They need to use new().
    //
    out << nl << "function " << self << " = " << name << "()";
    out.inc();
    out << nl << "% Use new()";
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";
    out << nl << "methods(Static)";
    out.inc();
    out << nl << "function r = new()";
    out.inc();
    if(st)
    {
        out << nl << "r = struct();";
    }
    else
    {
        out << nl << "r = containers.Map('KeyType', '" << dictionaryTypeToString(key, true) << "', 'ValueType', '"
            << dictionaryTypeToString(value, false) << "');";
    }
    out.dec();
    out << nl << "end";

    if(!p->isLocal())
    {
        out << nl << "function write(os, d)";
        out.inc();
        out << nl << "if isempty(d)";
        out.inc();
        out << nl << "os.writeSize(0);";
        out.dec();
        out << nl << "else";
        out.inc();
        if(st)
        {
            out << nl << "sz = length(d);";
            out << nl << "os.writeSize(sz);";
            out << nl << "for i = 1:sz";
            out.inc();
            int idx = 0;
            marshal(out, "os", "d(i).key", key, false, 0, idx);
            marshal(out, "os", "d(i).value", value, false, 0, idx);
            out.dec();
            out << nl << "end";
        }
        else
        {
            out << nl << "sz = d.Count;";
            out << nl << "os.writeSize(sz);";
            out << nl << "keys = d.keys();";
            out << nl << "values = d.values();";
            out << nl << "for i = 1:sz";
            out.inc();
            int idx = 0;
            out << nl << "k = keys{i};";
            out << nl << "v = values{i};";
            marshal(out, "os", "k", key, false, 0, idx);
            marshal(out, "os", "v", value, false, 0, idx);
            out.dec();
            out << nl << "end";
        }
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";

        out << nl << "function r = read(is)";
        out.inc();
        out << nl << "sz = is.readSize();";
        out << nl << "r = " << abs << ".new();";
        out << nl << "for i = 1:sz";
        out.inc();
        int idx = 0;
        unmarshal(out, "is", "k", key, false, 0, idx);
        unmarshal(out, "is", "v", value, false, 0, idx);
        if(st)
        {
            out << nl << "r(i).key = k;";
            out << nl << "r(i).value = v;";
        }
        else if(EnumPtr::dynamicCast(key))
        {
            out << nl << "r(int32(k)) = v;";
        }
        else
        {
            out << nl << "r(k) = v;";
        }
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";
    }

    out.dec();
    out << nl << "end";

    out.dec();
    out << nl << "end";
    out << nl;

    out.close();
}

void
CodeVisitor::visitEnum(const EnumPtr& p)
{
    const string name = fixIdent(p->name());
    const string scoped = p->scoped();
    const string abs = getAbsolute(p);

    IceUtilInternal::Output out;
    openClass(scoped, out);

    out << nl << "classdef " << name << " < int32";

    const EnumeratorList enumerators = p->enumerators();
    out.inc();
    out << nl << "enumeration";
    out.inc();
    for(EnumeratorList::const_iterator q = enumerators.begin(); q != enumerators.end(); ++q)
    {
        out << nl << fixIdent((*q)->name()) << " (" << (*q)->value() << ")";
    }
    out.dec();
    out << nl << "end";

    out << nl << "methods(Static)";
    out.inc();
    if(!p->isLocal())
    {
        out << nl << "function ice_write(os, v)";
        out.inc();
        out << nl << "if isempty(v)";
        out.inc();
        string firstEnum = fixIdent(enumerators.front()->name());
        out << nl << "os.writeEnum(int32(" << abs << "." << firstEnum << "), " << p->maxValue() << ");";
        out.dec();
        out << nl << "else";
        out.inc();
        out << nl << "os.writeEnum(int32(v), " << p->maxValue() << ");";
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";
        out << nl << "function r = ice_read(is)";
        out.inc();
        out << nl << "v = is.readEnum(" << p->maxValue() << ");";
        out << nl << "r = " << abs << ".ice_getValue(v);";
        out.dec();
        out << nl << "end";
    }
    out << nl << "function r = ice_getValue(v)";
    out.inc();
    out << nl << "switch v";
    out.inc();
    for(EnumeratorList::const_iterator q = enumerators.begin(); q != enumerators.end(); ++q)
    {
        out << nl << "case " << (*q)->value();
        out.inc();
        out << nl << "r = " << abs << "." << fixIdent((*q)->name()) << ";";
        out.dec();
    }
    out << nl << "otherwise";
    out.inc();
    out << nl << "throw(Ice.MarshalException('', '', sprintf('enumerator value %d is out of range', v)));";
    out.dec();
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    out.dec();
    out << nl << "end";

    out.dec();
    out << nl << "end";
    out << nl;
    out.close();
}

void
CodeVisitor::visitConst(const ConstPtr& p)
{
    const string name = fixIdent(p->name());
    const string scoped = p->scoped();

    IceUtilInternal::Output out;
    openClass(scoped, out);

    out << nl << "classdef " << name;

    out.inc();
    out << nl << "properties(Constant)";
    out.inc();
    out << nl << "value " << typeToString(p->type()) << " = "
        << constantValue(p->type(), p->valueType(), p->value());
    out.dec();
    out << nl << "end";

    out.dec();
    out << nl << "end";
    out << nl;
    out.close();
    out.close();
}

void
CodeVisitor::openClass(const string& scoped, IceUtilInternal::Output& out)
{
    vector<string> v = splitScopedName(scoped);
    assert(v.size() > 1);

    string path;
    if(!_dir.empty())
    {
        path = _dir + "/";
    }

    //
    // Create a package directory corresponding to each Slice module.
    //
    for(vector<string>::size_type i = 0; i < v.size() - 1; i++)
    {
        path += "+" + lookupKwd(v[i]);
        if(!IceUtilInternal::directoryExists(path))
        {
            if(IceUtilInternal::mkdir(path, 0777) != 0)
            {
                ostringstream os;
                os << "cannot create directory `" << path << "': " << strerror(errno);
                throw FileException(__FILE__, __LINE__, os.str());
            }
            FileTracker::instance()->addDirectory(path);
        }
        path += "/";
    }

    //
    // There are two options:
    //
    // 1) Create a subdirectory named "@ClassName" containing a file "ClassName.m".
    // 2) Create a file named "ClassName.m".
    //
    // The class directory is useful if you want to add additional supporting files for the class. We only
    // generate a single file for a class so we use option 2.
    //
    const string cls = lookupKwd(v[v.size() - 1]);
    path += "/" + cls + ".m";

    out.open(path);
    printHeader(out);
    FileTracker::instance()->addFile(path);
}

string
CodeVisitor::getOperationMode(Slice::Operation::Mode mode)
{
    switch(mode)
    {
        case Operation::Normal:
            return "Normal";
        case Operation::Nonmutating:
            return "Nonmutating";
        case Operation::Idempotent:
            return "Idempotent";
        default:
            return "???";
    }
}

void
CodeVisitor::collectClassMembers(const ClassDefPtr& p, MemberInfoList& allMembers, bool inherited)
{
    ClassList bases = p->bases();
    if(!bases.empty() && !bases.front()->isInterface())
    {
        collectClassMembers(bases.front(), allMembers, true);
    }

    DataMemberList members = p->dataMembers();

    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        MemberInfo m;
        m.fixedName = fixIdent((*q)->name());
        m.inherited = inherited;
        m.dataMember = *q;
        allMembers.push_back(m);
    }
}

void
CodeVisitor::collectExceptionMembers(const ExceptionPtr& p, MemberInfoList& allMembers, bool inherited)
{
    ExceptionPtr base = p->base();
    if(base)
    {
        collectExceptionMembers(base, allMembers, true);
    }

    DataMemberList members = p->dataMembers();

    for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
    {
        MemberInfo m;
        m.fixedName = fixIdent((*q)->name());
        m.inherited = inherited;
        m.dataMember = *q;
        allMembers.push_back(m);
    }
}

CodeVisitor::ParamInfoList
CodeVisitor::getInParams(const OperationPtr& op)
{
    const ParamDeclList l = op->inParameters();
    ParamInfoList r;
    for(ParamDeclList::const_iterator p = l.begin(); p != l.end(); ++p)
    {
        ParamInfo info;
        info.fixedName = fixIdent((*p)->name());
        info.type = (*p)->type();
        info.optional = (*p)->optional();
        info.tag = (*p)->tag();
        info.param = *p;
        r.push_back(info);
    }
    return r;
}

CodeVisitor::ParamInfoList
CodeVisitor::getOutParams(const OperationPtr& op)
{
    const ParamDeclList l = op->outParameters();
    ParamInfoList r;

    if(op->returnType())
    {
        ParamInfo info;
        info.fixedName = "result";
        for(ParamDeclList::const_iterator p = l.begin(); p != l.end(); ++p)
        {
            if((*p)->name() == "result")
            {
                info.fixedName = "result_";
                break;
            }
        }
        info.type = op->returnType();
        info.optional = op->returnIsOptional();
        info.tag = op->returnTag();
        r.push_back(info);
    }

    for(ParamDeclList::const_iterator p = l.begin(); p != l.end(); ++p)
    {
        ParamInfo info;
        info.fixedName = fixIdent((*p)->name());
        info.type = (*p)->type();
        info.optional = (*p)->optional();
        info.tag = (*p)->tag();
        info.param = *p;
        r.push_back(info);
    }

    return r;
}

string
CodeVisitor::getOptionalFormat(const TypePtr& type)
{
    BuiltinPtr bp = BuiltinPtr::dynamicCast(type);
    if(bp)
    {
        switch(bp->kind())
        {
        case Builtin::KindByte:
        case Builtin::KindBool:
        {
            return "'OptionalFormatF1'";
        }
        case Builtin::KindShort:
        {
            return "'OptionalFormatF2'";
        }
        case Builtin::KindInt:
        case Builtin::KindFloat:
        {
            return "'OptionalFormatF4'";
        }
        case Builtin::KindLong:
        case Builtin::KindDouble:
        {
            return "'OptionalFormatF8'";
        }
        case Builtin::KindString:
        {
            return "'OptionalFormatVSize'";
        }
        case Builtin::KindObject:
        {
            return "'OptionalFormatClass'";
        }
        case Builtin::KindObjectProxy:
        {
            return "'OptionalFormatFSize'";
        }
        case Builtin::KindLocalObject:
        {
            assert(false);
            break;
        }
        case Builtin::KindValue:
        {
            return "'OptionalFormatClass'";
        }
        }
    }

    if(EnumPtr::dynamicCast(type))
    {
        return "'OptionalFormatSize'";
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        return seq->type()->isVariableLength() ? "'OptionalFormatFSize'" : "'OptionalFormatVSize'";
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    if(d)
    {
        return (d->keyType()->isVariableLength() || d->valueType()->isVariableLength()) ?
            "'OptionalFormatFSize'" : "'OptionalFormatVSize'";
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        return st->isVariableLength() ? "'OptionalFormatFSize'" : "'OptionalFormatVSize'";
    }

    if(ProxyPtr::dynamicCast(type))
    {
        return "'OptionalFormatFSize'";
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    assert(cl);
    return "'OptionalFormatClass'";
}

string
CodeVisitor::getFormatType(FormatType type)
{
    switch(type)
    {
    case DefaultFormat:
        return "Ice.FormatType.DefaultFormat";
    case CompactFormat:
        return "Ice.FormatType.CompactFormat";
    case SlicedFormat:
        return "Ice.FormatType.SlicedFormat";
    default:
        assert(false);
    }

    return "???";
}

void
CodeVisitor::marshal(IceUtilInternal::Output& out, const string& stream, const string& v, const TypePtr& type,
                     bool optional, int tag, int& idx)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindByte:
            {
                if(optional)
                {
                    out << nl << stream << ".writeByteOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeByte(" << v << ");";
                }
                break;
            }
            case Builtin::KindBool:
            {
                if(optional)
                {
                    out << nl << stream << ".writeBoolOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeBool(" << v << ");";
                }
                break;
            }
            case Builtin::KindShort:
            {
                if(optional)
                {
                    out << nl << stream << ".writeShortOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeShort(" << v << ");";
                }
                break;
            }
            case Builtin::KindInt:
            {
                if(optional)
                {
                    out << nl << stream << ".writeIntOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeInt(" << v << ");";
                }
                break;
            }
            case Builtin::KindLong:
            {
                if(optional)
                {
                    out << nl << stream << ".writeLongOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeLong(" << v << ");";
                }
                break;
            }
            case Builtin::KindFloat:
            {
                if(optional)
                {
                    out << nl << stream << ".writeFloatOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeFloat(" << v << ");";
                }
                break;
            }
            case Builtin::KindDouble:
            {
                if(optional)
                {
                    out << nl << stream << ".writeDoubleOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeDouble(" << v << ");";
                }
                break;
            }
            case Builtin::KindString:
            {
                if(optional)
                {
                    out << nl << stream << ".writeStringOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeString(" << v << ");";
                }
                break;
            }
            case Builtin::KindObject:
            case Builtin::KindValue:
            {
                if(optional)
                {
                    out << nl << stream << ".writeValueOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeValue(" << v << ");";
                }
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if(optional)
                {
                    out << nl << stream << ".writeProxyOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeProxy(" << v << ");";
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
        if(optional)
        {
            out << nl << stream << ".writeProxyOpt(" << tag << ", " << v << ");";
        }
        else
        {
            out << nl << stream << ".writeProxy(" << v << ");";
        }
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(optional)
        {
            out << nl << stream << ".writeValueOpt(" << tag << ", " << v << ");";
        }
        else
        {
            out << nl << stream << ".writeValue(" << v << ");";
        }
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        const string typeS = getAbsolute(st);
        if(optional)
        {
            if(optional)
            {
                out << nl << "if ~isempty(" << v << ") && " << stream << ".writeOptional(" << tag << ", "
                    << getOptionalFormat(type) << ")";
                out.inc();
            }

            if(st->isVariableLength())
            {
                out << nl << "pos = " <<  stream << ".startSize();";
                out << nl << typeS << ".ice_write(" << stream << ", " << v << ");";
                out << nl << stream << ".endSize(pos);";
            }
            else
            {
                out << nl << stream << ".writeSize(" << st->minWireSize() << ");";
                out << nl << typeS << ".ice_write(" << stream << ", " << v << ");";
            }
            if(optional)
            {
                out.dec();
                out << nl << "end";
            }
        }
        else
        {
            out << nl << typeS << ".ice_write(" << stream << ", " << v << ");";
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        const string typeS = getAbsolute(en);
        if(optional)
        {
            out << nl << "if " << stream << ".writeOptional(" << tag << ", " << getOptionalFormat(type) << ")";
            out.inc();
            out << nl << typeS << ".ice_write(" << stream << ", " << v << ");";
            out.dec();
            out << nl << "end";
        }
        else
        {
            out << nl << typeS << ".ice_write(" << stream << ", " << v << ");";
        }
        return;
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        if(optional)
        {
            out << nl << "if " << stream << ".writeOptional(" << tag << ", " << getOptionalFormat(dict) << "))";
            out.inc();
            out << nl << getAbsolute(dict) << ".write(" << stream << ", " << v << ");";
            out.dec();
            out << nl << "end";
        }
        else
        {
            out << nl << getAbsolute(dict) << ".write(" << stream << ", " << v << ");";
        }
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        const TypePtr content = seq->type();
        const BuiltinPtr b = BuiltinPtr::dynamicCast(content);

        if(b && b->kind() != Builtin::KindObject && b->kind() != Builtin::KindObjectProxy &&
           b->kind() != Builtin::KindValue)
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
                "???",
                "???"
            };
            string bs = builtinTable[b->kind()];
            out << nl << stream << ".write" << builtinTable[b->kind()] << "Seq";
            if(optional)
            {
                out << "Opt(" << tag << ", ";
            }
            else
            {
                out << "(";
            }
            out << v << ");";
            return;
        }

        if(optional)
        {
            out << nl << "if " << stream << ".writeOptional(" << tag << ", " << getOptionalFormat(seq) << "))";
            out.inc();
            out << nl << getAbsolute(seq) << ".write(" << stream << ", " << v << ");";
            out.dec();
            out << nl << "end";
        }
        else
        {
            out << nl << getAbsolute(seq) << ".write(" << stream << ", " << v << ");";
        }
        return;
    }

    assert(false);
}


void
CodeVisitor::unmarshal(IceUtilInternal::Output& out, const string& stream, const string& v, const TypePtr& type,
                       bool optional, int tag, int& idx)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindByte:
            {
                if(optional)
                {
                    out << nl << v << " = " << stream << ".readByteOpt(" << tag << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readByte();";
                }
                break;
            }
            case Builtin::KindBool:
            {
                if(optional)
                {
                    out << nl << v << " = " << stream << ".readBoolOpt(" << tag << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readBool();";
                }
                break;
            }
            case Builtin::KindShort:
            {
                if(optional)
                {
                    out << nl << v << " = " << stream << ".readShortOpt(" << tag << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readShort();";
                }
                break;
            }
            case Builtin::KindInt:
            {
                if(optional)
                {
                    out << nl << v << " = " << stream << ".readIntOpt(" << tag << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readInt();";
                }
                break;
            }
            case Builtin::KindLong:
            {
                if(optional)
                {
                    out << nl << v << " = " << stream << ".readLongOpt(" << tag << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readLong();";
                }
                break;
            }
            case Builtin::KindFloat:
            {
                if(optional)
                {
                    out << nl << v << " = " << stream << ".readFloatOpt(" << tag << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readFloat();";
                }
                break;
            }
            case Builtin::KindDouble:
            {
                if(optional)
                {
                    out << nl << v << " = " << stream << ".readDoubleOpt(" << tag << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readDouble();";
                }
                break;
            }
            case Builtin::KindString:
            {
                if(optional)
                {
                    out << nl << v << " = " << stream << ".readStringOpt(" << tag << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readString();";
                }
                break;
            }
            case Builtin::KindObject:
            case Builtin::KindValue:
            {
#if 0 // TBD
                if(optional)
                {
                    out << nl << stream << ".readValue(" << tag << ", " << param << ");";
                }
                else if(holder && mode == OptionalNone)
                {
                    out << nl << stream << ".readValue(" << param << ");";
                }
                else
                {
                    if(patchParams.empty())
                    {
                        out << nl << stream << ".readValue(new Patcher());";
                    }
                    else
                    {
                        out << nl << stream << ".readValue(" << patchParams << ");";
                    }
                }
#endif
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if(optional)
                {
                    out << nl << v << " = " << stream << ".readProxyOpt(" << tag << ");";
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
        const string typeS = getAbsolute(prx->_class(), "", "Prx");
        if(optional)
        {
            out << nl << "if " << stream << ".readOptional(" << tag << ", " << getOptionalFormat(type) << ")";
            out.inc();
            out << nl << stream << ".skip(4);";
            out << nl << v << " = " << typeS << ".ice_read(" << stream << ");";
            out.dec();
            out << nl << "end";
        }
        else
        {
            out << nl << v << " = " << typeS << ".ice_read(" << stream << ");";
        }
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
#if 0 // TBD
        if(optional)
        {
            const string typeS = typeToString(type, TypeModeIn, package);
            out << nl << "if(" << stream << ".readOptional(" << tag << ", " << getOptionalFormat(type) << "))";
            out << sb;
            out << nl << stream << ".readValue(new Ice.OptionalObject(" << v << ", " << typeS << ".class, "
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
                out << nl << stream << ".readValue(" << param << ");";
            }
            else
            {
                if(patchParams.empty())
                {
                    out << nl << stream << ".readValue(new Patcher());";
                }
                else
                {
                    out << nl << stream << ".readValue(" << patchParams << ");";
                }
            }
        }
#endif
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        const string typeS = getAbsolute(st);
        if(optional)
        {
            out << nl << "if " << stream << ".readOptional(" << tag << ", " << getOptionalFormat(type) << ")";
            out.inc();

            if(st->isVariableLength())
            {
                out << nl << stream << ".skip(4);";
            }
            else
            {
                out << nl << stream << ".skipSize();";
            }

            out << nl << v << " = " << typeS << ".ice_read(" << stream << ");";

            out.dec();
            out << nl << "end";
        }
        else
        {
            out << nl << v << " = " << typeS << ".ice_read(" << stream << ");";
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        const string typeS = getAbsolute(en);
        if(optional)
        {
            out << nl << "if " << stream << ".readOptional(" << tag << ", " << getOptionalFormat(type) << ")";
            out.inc();
            out << nl << v << " = " << typeS << ".ice_read(" << stream << ");";
            out.dec();
            out << nl << "end";
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
            out << nl << "if " << stream << ".readOptional(" << tag << ", " << getOptionalFormat(dict) << "))";
            out.inc();
            out << nl << v << " = " << getAbsolute(dict) << ".read(" << stream << ");";
            out.dec();
            out << nl << "end";
        }
        else
        {
            out << nl << v << " = " << getAbsolute(dict) << ".read(" << stream << ");";
        }
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        const TypePtr content = seq->type();
        const BuiltinPtr b = BuiltinPtr::dynamicCast(content);

        if(b && b->kind() != Builtin::KindObject && b->kind() != Builtin::KindObjectProxy &&
           b->kind() != Builtin::KindValue)
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
                "???",
                "???"
            };
            string bs = builtinTable[b->kind()];
            out << nl << v << " = " << stream << ".read" << builtinTable[b->kind()] << "Seq";
            if(optional)
            {
                out << "Opt(" << tag << ");";
            }
            else
            {
                out << "();";
            }
            return;
        }

        if(optional)
        {
            out << nl << "if " << stream << ".readOptional(" << tag << ", " << getOptionalFormat(seq) << "))";
            out.inc();
            out << nl << v << " = " << getAbsolute(seq) << ".read(" << stream << ");";
            out.dec();
            out << nl << "end";
        }
        else
        {
            out << nl << v << " = " << getAbsolute(seq) << ".read(" << stream << ");";
        }
        return;
    }

    assert(false);
}

static void
generate(const UnitPtr& un, const string& dir, bool all, bool checksum, const vector<string>& includePaths)
{
    CodeVisitor codeVisitor(dir);
    un->visit(&codeVisitor, false);

#if 0
    if(checksum)
    {
        ChecksumMap checksums = createChecksums(un);
        if(!checksums.empty())
        {
            out << sp;
            if(ns)
            {
                out << "namespace"; // Global namespace.
                out << sb;
                out << "new Ice\\SliceChecksumInit(array(";
                for(ChecksumMap::const_iterator p = checksums.begin(); p != checksums.end();)
                {
                    out << nl << "\"" << p->first << "\" => \"";
                    ostringstream str;
                    str.flags(ios_base::hex);
                    str.fill('0');
                    for(vector<unsigned char>::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
                    {
                        str << static_cast<int>(*q);
                    }
                    out << str.str() << "\"";
                    if(++p != checksums.end())
                    {
                        out << ",";
                    }
                }
                out << "));";
                out << eb;
            }
            else
            {
                for(ChecksumMap::const_iterator p = checksums.begin(); p != checksums.end(); ++p)
                {
                    out << nl << "$Ice_sliceChecksums[\"" << p->first << "\"] = \"";
                    ostringstream str;
                    str.flags(ios_base::hex);
                    str.fill('0');
                    for(vector<unsigned char>::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
                    {
                        str << static_cast<int>(*q);
                    }
                    out << str.str() << "\";";
                }
            }
        }
    }

    out << nl; // Trailing newline.
#endif
}

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

static void
interruptedCallback(int /*signal*/)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(globalMutex);

    interrupted = true;
}

static void
usage(const string& n)
{
    consoleErr << "Usage: " << n << " [options] slice-files...\n";
    consoleErr <<
        "Options:\n"
        "-h, --help               Show this message.\n"
        "-v, --version            Display the Ice version.\n"
        "-DNAME                   Define NAME as 1.\n"
        "-DNAME=DEF               Define NAME as DEF.\n"
        "-UNAME                   Remove any definition for NAME.\n"
        "-IDIR                    Put DIR in the include file search path.\n"
        "-E                       Print preprocessor output on stdout.\n"
        "--output-dir DIR         Create files in the directory DIR.\n"
        "-d, --debug              Print debug messages.\n"
        "--depend                 Generate Makefile dependencies.\n"
        "--depend-xml             Generate dependencies in XML format.\n"
        "--depend-file FILE       Write dependencies to FILE instead of standard output.\n"
        "--validate               Validate command line options.\n"
        "--all                    Generate code for Slice definitions in included files.\n"
        "--checksum               Generate checksums for Slice definitions.\n"
        ;
}

int
compile(const vector<string>& argv)
{
    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("", "validate");
    opts.addOpt("D", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("U", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("I", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("E");
    opts.addOpt("", "output-dir", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "depend");
    opts.addOpt("", "depend-xml");
    opts.addOpt("", "depend-file", IceUtilInternal::Options::NeedArg, "");
    opts.addOpt("d", "debug");
    opts.addOpt("", "all");
    opts.addOpt("", "checksum");

    bool validate = find(argv.begin(), argv.end(), "--validate") != argv.end();

    vector<string> args;
    try
    {
        args = opts.parse(argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        consoleErr << argv[0] << ": error: " << e.reason << endl;
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
        consoleErr << ICE_STRING_VERSION << endl;
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

    string output = opts.optArg("output-dir");

    bool depend = opts.isSet("depend");

    bool dependxml = opts.isSet("depend-xml");

    string dependFile = opts.optArg("depend-file");

    bool debug = opts.isSet("debug");

    bool all = opts.isSet("all");

    bool checksum = opts.isSet("checksum");

    if(args.empty())
    {
        consoleErr << argv[0] << ": error: no input file" << endl;
        if(!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if(depend && dependxml)
    {
        consoleErr << argv[0] << ": error: cannot specify both --depend and --depend-xml" << endl;
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

    int status = EXIT_SUCCESS;

    IceUtil::CtrlCHandler ctrlCHandler;
    ctrlCHandler.setCallback(interruptedCallback);

    ostringstream os;
    if(dependxml)
    {
        os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dependencies>" << endl;
    }

    for(vector<string>::const_iterator i = args.begin(); i != args.end(); ++i)
    {
        //
        // Ignore duplicates.
        //
        vector<string>::iterator p = find(args.begin(), args.end(), *i);
        if(p != i)
        {
            continue;
        }

        if(depend || dependxml)
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2PHP__");

            if(cppHandle == 0)
            {
                return EXIT_FAILURE;
            }

            UnitPtr u = Unit::createUnit(false, false, false, false);
            int parseStatus = u->parse(*i, cppHandle, debug);
            u->destroy();

            if(parseStatus == EXIT_FAILURE)
            {
                return EXIT_FAILURE;
            }

            if(!icecpp->printMakefileDependencies(os, depend ? Preprocessor::PHP : Preprocessor::SliceXML,
                                                  includePaths, "-D__SLICE2PHP__"))
            {
                return EXIT_FAILURE;
            }

            if(!icecpp->close())
            {
                return EXIT_FAILURE;
            }
        }
        else
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2PHP__");

            if(cppHandle == 0)
            {
                return EXIT_FAILURE;
            }

            if(preprocess)
            {
                char buf[4096];
                while(fgets(buf, static_cast<int>(sizeof(buf)), cppHandle) != ICE_NULLPTR)
                {
                    if(fputs(buf, stdout) == EOF)
                    {
                        return EXIT_FAILURE;
                    }
                }
                if(!icecpp->close())
                {
                    return EXIT_FAILURE;
                }
            }
            else
            {
                UnitPtr u = Unit::createUnit(false, all, false, false);
                int parseStatus = u->parse(*i, cppHandle, debug);

                if(!icecpp->close())
                {
                    u->destroy();
                    return EXIT_FAILURE;
                }

                if(parseStatus == EXIT_FAILURE)
                {
                    status = EXIT_FAILURE;
                }
                else
                {
                    string base = icecpp->getBaseName();
                    string::size_type pos = base.find_last_of("/\\");
                    if(pos != string::npos)
                    {
                        base.erase(0, pos + 1);
                    }

                    try
                    {
                        generate(u, output, all, checksum, includePaths);
                    }
                    catch(const Slice::FileException& ex)
                    {
                        //
                        // If a file could not be created, then cleanup any created files.
                        //
                        FileTracker::instance()->cleanup();
                        u->destroy();
                        consoleErr << argv[0] << ": error: " << ex.reason() << endl;
                        return EXIT_FAILURE;
                    }
                    catch(const string& err)
                    {
                        FileTracker::instance()->cleanup();
                        consoleErr << argv[0] << ": error: " << err << endl;
                        status = EXIT_FAILURE;
                    }
                }

                u->destroy();
            }
        }

        {
            IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(globalMutex);

            if(interrupted)
            {
                FileTracker::instance()->cleanup();
                return EXIT_FAILURE;
            }
        }
    }

    if(dependxml)
    {
        os << "</dependencies>\n";
    }

    if(depend || dependxml)
    {
        writeDependencies(os.str(), dependFile);
    }

    return status;
}

#ifdef _WIN32
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    vector<string> args = Slice::argvToArgs(argc, argv);
    try
    {
        return compile(args);
    }
    catch(const std::exception& ex)
    {
        consoleErr << args[0] << ": error:" << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch(const std::string& msg)
    {
        consoleErr << args[0] << ": error:" << msg << endl;
        return EXIT_FAILURE;
    }
    catch(const char* msg)
    {
        consoleErr << args[0] << ": error:" << msg << endl;
        return EXIT_FAILURE;
    }
    catch(...)
    {
        consoleErr << args[0] << ": error:" << "unknown exception" << endl;
        return EXIT_FAILURE;
    }
}
