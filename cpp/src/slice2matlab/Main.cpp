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
// Split an absolute name into its components and return the components as a list of identifiers.
//
vector<string>
splitAbsoluteName(const string& abs)
{
    vector<string> ids;
    string::size_type start = 0;
    string::size_type pos;
    while((pos = abs.find(".", start)) != string::npos)
    {
        assert(pos > start);
        ids.push_back(abs.substr(start, pos - start));
        start = pos + 1;
    }
    if(start != abs.size())
    {
        ids.push_back(abs.substr(start));
    }

    return ids;
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
fixExceptionMemberIdent(const string& ident)
{
    //
    // User exceptions are subclasses of MATLAB's MException class. Subclasses cannot redefine a member that
    // conflicts with MException's properties. Unfortunately MException also has some undocumented non-public
    // properties that will cause run-time errors.
    //
    string s = fixIdent(ident);
    if(s == "identifier" ||
       s == "message" ||
       s == "stack" ||
       s == "cause" ||
       s == "type") // Undocumented
    {
        s.push_back('_');
    }

    return s;
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

map<string, string> _filePackagePrefix;

string
getPackagePrefix(const ContainedPtr& cont)
{
    UnitPtr unit = cont->container()->unit();
    string file = cont->file();
    assert(!file.empty());

    map<string, string>::const_iterator p = _filePackagePrefix.find(file);
    if(p != _filePackagePrefix.end())
    {
        return p->second;
    }

    static const string prefix = "matlab:package:";
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

//
// Get the fully-qualified name of the given definition. If a suffix is provided,
// it is prepended to the definition's unqualified name. If the nameSuffix
// is provided, it is appended to the container's name.
//
string
getAbsolute(const ContainedPtr& cont, const string& pfx = std::string(), const string& suffix = std::string())
{
    string pkg = getPackagePrefix(cont);
    if(!pkg.empty())
    {
        pkg += ".";
    }
    return pkg + scopedToName(cont->scope() + pfx + cont->name() + suffix);
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
        "Ice.Object", // Object
        "Ice.ObjectPrx", // ObjectPrx
        "", // LocalObject
        "Ice.Value" // Value
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
    if(optional || SequencePtr::dynamicCast(type) || ProxyPtr::dynamicCast(type) || ClassDeclPtr::dynamicCast(type))
    {
        return false;
    }

    BuiltinPtr b = BuiltinPtr::dynamicCast(type);
    if(b && (b->kind() == Builtin::KindObject || b->kind() == Builtin::KindObjectProxy ||
             b->kind() == Builtin::KindValue))
    {
        return false;
    }

    return true;
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
        return "Ice.Unset";
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
            return getAbsolute(dict) + ".new()";
        }

        return "[]";
    }
}

bool
isClass(const TypePtr& type)
{
    BuiltinPtr b = BuiltinPtr::dynamicCast(type);
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    return (b && (b->kind() == Builtin::KindObject || b->kind() == Builtin::KindValue)) || cl;
}

bool
needsConversion(const TypePtr& type)
{
    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        return isClass(seq->type()) || needsConversion(seq->type());
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        const DataMemberList members = st->dataMembers();
        for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
        {
            if(needsConversion((*q)->type()) || isClass((*q)->type()))
            {
                return true;
            }
        }
        return false;
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    if(d)
    {
        return needsConversion(d->valueType()) || isClass(d->valueType());
    }

    return false;
}

void
convertValueType(IceUtilInternal::Output& out, const string& dest, const string& src, const TypePtr& type,
                 bool optional)
{
    assert(needsConversion(type));

    if(optional)
    {
        out << nl << "if " << src << " ~= Ice.Unset";
        out.inc();
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        out << nl << dest << " = " << getAbsolute(seq) << ".convert(" << src << ");";
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    if(d)
    {
        out << nl << dest << " = " << getAbsolute(d) << ".convert(" << src << ");";
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        out << nl << dest << " = " << src << ".ice_convert();";
    }

    if(optional)
    {
        out.dec();
        out << nl << "end";
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
        int pos; // Only used for out params
        ParamDeclPtr param; // 0 == return value
    };
    typedef list<ParamInfo> ParamInfoList;

    ParamInfoList getAllInParams(const OperationPtr&);
    void getInParams(const OperationPtr&, ParamInfoList&, ParamInfoList&);
    ParamInfoList getAllOutParams(const OperationPtr&);
    void getOutParams(const OperationPtr&, ParamInfoList&, ParamInfoList&);

    string getOptionalFormat(const TypePtr&);
    string getFormatType(FormatType);

    void marshal(IceUtilInternal::Output&, const string&, const string&, const TypePtr&, bool, int);
    void unmarshal(IceUtilInternal::Output&, const string&, const string&, const TypePtr&, bool, int);

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
    if(p->hasMetaData("matlab:internal"))
    {
        return false;
    }

    const string name = fixIdent(p->name());
    const string scoped = p->scoped();
    const string abs = getAbsolute(p);
    const ClassList bases = p->bases();
    const string self = name == "obj" ? "this" : "obj";

    if(!p->isInterface())
    {
        ClassDefPtr base;
        if(!bases.empty() && !bases.front()->isInterface())
        {
            base = bases.front();
        }

        IceUtilInternal::Output out;
        openClass(abs, out);

        out << nl << "classdef " << name;
        if(base)
        {
            out << " < " << getAbsolute(base);
        }
        else if(!p->isLocal())
        {
            out << " < Ice.Value";
        }

        out.inc();

        const DataMemberList members = p->dataMembers();
        if(!members.empty())
        {
            if(p->hasMetaData("protected"))
            {
                //
                // All members are protected.
                //
                out << nl << "properties(Access=protected)";
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
            else
            {
                DataMemberList prot, pub;
                for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
                {
                    if((*q)->hasMetaData("protected"))
                    {
                        prot.push_back(*q);
                    }
                    else
                    {
                        pub.push_back(*q);
                    }
                }
                if(!pub.empty())
                {
                    out << nl << "properties";
                    out.inc();
                    for(DataMemberList::const_iterator q = pub.begin(); q != pub.end(); ++q)
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
                if(!prot.empty())
                {
                    out << nl << "properties(Access=protected)";
                    out.inc();
                    for(DataMemberList::const_iterator q = prot.begin(); q != prot.end(); ++q)
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
            }
        }

        const bool basePreserved = p->inheritsMetaData("preserve-slice");
        const bool preserved = p->hasMetaData("preserve-slice");

        MemberInfoList allMembers;
        collectClassMembers(p, allMembers, false);

        if(!allMembers.empty() || !p->isLocal() || (preserved && !basePreserved))
        {
            out << nl << "methods";
            out.inc();

            //
            // Constructor
            //
            if(!allMembers.empty())
            {
                vector<string> allNames;
                for(MemberInfoList::const_iterator q = allMembers.begin(); q != allMembers.end(); ++q)
                {
                    allNames.push_back(q->fixedName);
                }
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
                if(base)
                {
                    out << nl << self << " = " << self << "@" << getAbsolute(base) << spar;
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
                out << nl << "function id = ice_id(obj)";
                out.inc();
                out << nl << "id = obj.ice_staticId();";
                out.dec();
                out << nl << "end";

                if(preserved && !basePreserved)
                {
                    out << nl << "function r = ice_getSlicedData(obj)";
                    out.inc();
                    out << nl << "r = obj.iceSlicedData_;";
                    out.dec();
                    out << nl << "end";
                }
            }

            out.dec();
            out << nl << "end";
        }

        if(!p->isLocal())
        {
            DataMemberList convertMembers;
            for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
            {
                if(needsConversion((*d)->type()))
                {
                    convertMembers.push_back(*d);
                }
            }

            if((preserved && !basePreserved) || !convertMembers.empty())
            {
                out << nl << "methods(Hidden=true)";
                out.inc();

                if(preserved && !basePreserved)
                {
                    out << nl << "function iceWrite_(obj, os)";
                    out.inc();
                    out << nl << "os.startValue(obj.iceSlicedData_);";
                    out << nl << "obj.iceWriteImpl_(os);";
                    out << nl << "os.endValue();";
                    out.dec();
                    out << nl << "end";
                    out << nl << "function iceRead_(obj, is)";
                    out.inc();
                    out << nl << "is.startValue();";
                    out << nl << "obj.iceReadImpl_(is);";
                    out << nl << "obj.iceSlicedData_ = is.endValue(true);";
                    out.dec();
                    out << nl << "end";
                }

                if(!convertMembers.empty())
                {
                    out << nl << "function r = iceDelayPostUnmarshal_(obj)";
                    out.inc();
                    out << nl << "r = true;";
                    out.dec();
                    out << nl << "end";
                    out << nl << "function icePostUnmarshal_(obj)";
                    out.inc();
                    for(DataMemberList::const_iterator d = convertMembers.begin(); d != convertMembers.end(); ++d)
                    {
                        string m = "obj." + fixIdent((*d)->name());
                        convertValueType(out, m, m, (*d)->type(), (*d)->optional());
                    }
                    if(base)
                    {
                        out << nl << "icePostUnmarshal_@" << getAbsolute(base) << "(obj);";
                    }
                    out.dec();
                    out << nl << "end";
                }

                out.dec();
                out << nl << "end";
            }

            out << nl << "methods(Access=protected)";
            out.inc();

            const DataMemberList optionalMembers = p->orderedOptionalDataMembers();

            out << nl << "function iceWriteImpl_(obj, os)";
            out.inc();
            out << nl << "os.startSlice('" << scoped << "', " << p->compactId() << (!base ? ", true" : ", false")
                << ");";
            for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
            {
                if(!(*d)->optional())
                {
                    marshal(out, "os", "obj." + fixIdent((*d)->name()), (*d)->type(), false, 0);
                }
            }
            for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
            {
                marshal(out, "os", "obj." + fixIdent((*d)->name()), (*d)->type(), true, (*d)->tag());
            }
            out << nl << "os.endSlice();";
            if(base)
            {
                out << nl << "iceWriteImpl_@" << getAbsolute(base) << "(obj, os);";
            }
            out.dec();
            out << nl << "end";
            out << nl << "function iceReadImpl_(obj, is)";
            out.inc();
            out << nl << "is.startSlice();";
            for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
            {
                if(!(*d)->optional())
                {
                    if(isClass((*d)->type()))
                    {
                        unmarshal(out, "is", "@obj.iceSetMember_" + fixIdent((*d)->name()) + "_", (*d)->type(), false,
                                  0);
                    }
                    else
                    {
                        unmarshal(out, "is", "obj." + fixIdent((*d)->name()), (*d)->type(), false, 0);
                    }
                }
            }
            for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
            {
                if(isClass((*d)->type()))
                {
                    unmarshal(out, "is", "@obj.iceSetMember_" + fixIdent((*d)->name()) + "_", (*d)->type(), true,
                              (*d)->tag());
                }
                else
                {
                    unmarshal(out, "is", "obj." + fixIdent((*d)->name()), (*d)->type(), true, (*d)->tag());
                }
            }
            out << nl << "is.endSlice();";
            if(base)
            {
                out << nl << "iceReadImpl_@" << getAbsolute(base) << "(obj, is);";
            }
            out.dec();
            out << nl << "end";

            DataMemberList classMembers = p->classDataMembers();
            if(!classMembers.empty())
            {
                //
                // For each class data member, we generate a "set_<name>_" method that is called when the instance
                // is eventually unmarshaled.
                //
                for(DataMemberList::const_iterator d = classMembers.begin(); d != classMembers.end(); ++d)
                {
                    string m = fixIdent((*d)->name());
                    out << nl << "function iceSetMember_" << m << "_(obj, v)";
                    out.inc();
                    out << nl << "obj." << m << " = v;";
                    out.dec();
                    out << nl << "end";
                }
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
            out.dec();
            out << nl << "end";

            if(preserved && !basePreserved)
            {
                out << nl << "properties(Access=protected)";
                out.inc();
                out << nl << "iceSlicedData_";
                out.dec();
                out << nl << "end";
            }
        }

        out.dec();
        out << nl << "end";
        out << nl;

        out.close();

        if(p->compactId() >= 0)
        {
            ostringstream ostr;
            ostr << "IceCompactId.TypeId_" << p->compactId();

            openClass(ostr.str(), out);

            out << nl << "classdef TypeId_" << p->compactId();
            out.inc();

            out << nl << "properties(Constant)";
            out.inc();
            out << nl << "typeId = '" << scoped << "'";
            out.dec();
            out << nl << "end";

            out.dec();
            out << nl << "end";
            out << nl;

            out.close();
        }
    }
    else if(!p->isLocal())
    {
        //
        // Generate proxy class.
        //

        const string prxName = name + "Prx";
        const string prxAbs = getAbsolute(p, "", "Prx");

        IceUtilInternal::Output out;
        openClass(prxAbs, out);

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
        // Operations.
        //
        const OperationList ops = p->operations();
        for(OperationList::const_iterator q = ops.begin(); q != ops.end(); ++q)
        {
            OperationPtr op = *q;
            ParamInfoList requiredInParams, optionalInParams;
            getInParams(op, requiredInParams, optionalInParams);
            ParamInfoList requiredOutParams, optionalOutParams;
            getOutParams(op, requiredOutParams, optionalOutParams);
            const ParamInfoList allInParams = getAllInParams(op);
            const ParamInfoList allOutParams = getAllOutParams(op);
            const bool twowayOnly = op->returnsData();

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
            for(ParamInfoList::const_iterator r = allOutParams.begin(); r != allOutParams.end(); ++r)
            {
                if(r->fixedName == "obj")
                {
                    self = "obj_";
                }
            }
            for(ParamInfoList::const_iterator r = allInParams.begin(); r != allInParams.end(); ++r)
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
            if(allOutParams.size() > 1)
            {
                out << "[";
                for(ParamInfoList::const_iterator r = allOutParams.begin(); r != allOutParams.end(); ++r)
                {
                    if(r != allOutParams.begin())
                    {
                        out << ", ";
                    }
                    out << r->fixedName;
                }
                out << "] = ";
            }
            else if(allOutParams.size() == 1)
            {
                out << allOutParams.begin()->fixedName << " = ";
            }
            out << fixIdent(op->name()) << spar;

            out << self;
            for(ParamInfoList::const_iterator r = allInParams.begin(); r != allInParams.end(); ++r)
            {
                out << r->fixedName;
            }
            out << "varargin"; // For the optional context
            out << epar;
            out.inc();

            if(!allInParams.empty())
            {
                if(op->format() == DefaultFormat)
                {
                    out << nl << "os_ = " << self << ".startWriteParams_([]);";
                }
                else
                {
                    out << nl << "os_ = " << self << ".startWriteParams_(" << getFormatType(op->format()) << ");";
                }
                for(ParamInfoList::const_iterator r = requiredInParams.begin(); r != requiredInParams.end(); ++r)
                {
                    marshal(out, "os_", r->fixedName, r->type, false, 0);
                }
                for(ParamInfoList::const_iterator r = optionalInParams.begin(); r != optionalInParams.end(); ++r)
                {
                    marshal(out, "os_", r->fixedName, r->type, r->optional, r->tag);
                }
                if(op->sendsClasses(false))
                {
                    out << nl << "os_.writePendingValues();";
                }
                out << nl << self << ".endWriteParams_(os_);";
            }

            out << nl << "[ok_, is_] = " << self << ".invoke_('" << op->name() << "', '"
                << getOperationMode(op->sendMode()) << "', " << (twowayOnly ? "true" : "false")
                << ", " << (allInParams.empty() ? "[]" : "os_") << ", varargin{:});";

            if(!twowayOnly)
            {
                out << nl << self << ".checkNoResponse_(ok_, is_);";
            }
            else
            {
                out << nl << "if ok_";
                out.inc();
                if(allOutParams.empty())
                {
                    out << nl << "is_.skipEmptyEncapsulation();";
                }
                else
                {
                    out << nl << "is_.startEncapsulation();";
                    //
                    // To unmarshal results:
                    //
                    // * unmarshal all required out parameters
                    // * unmarshal the required return value (if any)
                    // * unmarshal all optional out parameters (this includes an optional return value)
                    //
                    ParamInfoList classParams;
                    ParamInfoList convertParams;
                    for(ParamInfoList::const_iterator r = requiredOutParams.begin(); r != requiredOutParams.end(); ++r)
                    {
                        if(r->param)
                        {
                            string name;
                            if(isClass(r->type))
                            {
                                out << nl << r->fixedName << "_h_ = Ice.ValueHolder();";
                                name = "@(v) " + r->fixedName + "_h_.set(v)";
                                classParams.push_back(*r);
                            }
                            else
                            {
                                name = r->fixedName;
                            }
                            unmarshal(out, "is_", name, r->type, false, -1);

                            if(needsConversion(r->type))
                            {
                                convertParams.push_back(*r);
                            }
                        }
                    }
                    //
                    // Now do the required return value if necessary.
                    //
                    if(!requiredOutParams.empty() && !requiredOutParams.begin()->param)
                    {
                        ParamInfoList::const_iterator r = requiredOutParams.begin();
                        string name;
                        if(isClass(r->type))
                        {
                            out << nl << r->fixedName << "_h_ = Ice.ValueHolder();";
                            name = "@(v) " + r->fixedName + "_h_.set(v)";
                            classParams.push_back(*r);
                        }
                        else
                        {
                            name = r->fixedName;
                        }
                        unmarshal(out, "is_", name, r->type, false, -1);

                        if(needsConversion(r->type))
                        {
                            convertParams.push_back(*r);
                        }
                    }
                    //
                    // Now unmarshal all optional out parameters. They are already sorted by tag.
                    //
                    for(ParamInfoList::const_iterator r = optionalOutParams.begin(); r != optionalOutParams.end(); ++r)
                    {
                        string name;
                        if(isClass(r->type))
                        {
                            out << nl << r->fixedName << "_h_ = Ice.ValueHolder();";
                            name = "@(v) " + r->fixedName + "_h_.set(v)";
                            classParams.push_back(*r);
                        }
                        else
                        {
                            name = r->fixedName;
                        }
                        unmarshal(out, "is_", name, r->type, r->optional, r->tag);

                        if(needsConversion(r->type))
                        {
                            convertParams.push_back(*r);
                        }
                    }
                    if(op->returnsClasses(false))
                    {
                        out << nl << "is_.readPendingValues();";
                    }
                    out << nl << "is_.endEncapsulation();";
                    //
                    // After calling readPendingValues(), all callback functions have been invoked.
                    // Now we need to collect the values.
                    //
                    for(ParamInfoList::const_iterator r = classParams.begin(); r != classParams.end(); ++r)
                    {
                        out << nl << r->fixedName << " = " << r->fixedName << "_h_.value;";
                    }

                    for(ParamInfoList::const_iterator r = convertParams.begin(); r != convertParams.end(); ++r)
                    {
                        convertValueType(out, r->fixedName, r->fixedName, r->type, r->optional);
                    }
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
            for(ParamInfoList::const_iterator r = allInParams.begin(); r != allInParams.end(); ++r)
            {
                out << r->fixedName;
            }
            out << "varargin"; // For the optional context
            out << epar;
            out.inc();

            if(!allInParams.empty())
            {
                if(op->format() == DefaultFormat)
                {
                    out << nl << "os_ = " << self << ".startWriteParams_([]);";
                }
                else
                {
                    out << nl << "os_ = " << self << ".startWriteParams_(" << getFormatType(op->format()) << ");";
                }
                for(ParamInfoList::const_iterator r = requiredInParams.begin(); r != requiredInParams.end(); ++r)
                {
                    marshal(out, "os_", r->fixedName, r->type, false, 0);
                }
                for(ParamInfoList::const_iterator r = optionalInParams.begin(); r != optionalInParams.end(); ++r)
                {
                    marshal(out, "os_", r->fixedName, r->type, r->optional, r->tag);
                }
                if(op->sendsClasses(false))
                {
                    out << nl << "os_.writePendingValues();";
                }
                out << nl << self << ".endWriteParams_(os_);";
            }

            if(twowayOnly)
            {
                out << nl << "function varargout = unmarshal(ok_, is_)";
                out.inc();
                out << nl << "if ok_";
                out.inc();
                if(allOutParams.empty())
                {
                    out << nl << "is_.skipEmptyEncapsulation();";
                }
                else
                {
                    out << nl << "is_.startEncapsulation();";
                    //
                    // To unmarshal results:
                    //
                    // * unmarshal all required out parameters
                    // * unmarshal the required return value (if any)
                    // * unmarshal all optional out parameters (this includes an optional return value)
                    //
                    for(ParamInfoList::const_iterator r = requiredOutParams.begin(); r != requiredOutParams.end(); ++r)
                    {
                        if(r->param)
                        {
                            string name;
                            if(isClass(r->type))
                            {
                                out << nl << r->fixedName << " = Ice.ValueHolder();";
                                name = "@(v) " + r->fixedName + ".set(v)";
                            }
                            else
                            {
                                name = r->fixedName;
                            }
                            unmarshal(out, "is_", name, r->type, r->optional, r->tag);
                        }
                    }
                    //
                    // Now do the required return value if necessary.
                    //
                    if(!requiredOutParams.empty() && !requiredOutParams.begin()->param)
                    {
                        ParamInfoList::const_iterator r = requiredOutParams.begin();
                        string name;
                        if(isClass(r->type))
                        {
                            out << nl << r->fixedName << " = Ice.ValueHolder();";
                            name = "@(v) " + r->fixedName + ".set(v)";
                        }
                        else
                        {
                            name = r->fixedName;
                        }
                        unmarshal(out, "is_", name, r->type, false, -1);
                    }
                    //
                    // Now unmarshal all optional out parameters. They are already sorted by tag.
                    //
                    for(ParamInfoList::const_iterator r = optionalOutParams.begin(); r != optionalOutParams.end(); ++r)
                    {
                        string name;
                        if(isClass(r->type))
                        {
                            out << nl << r->fixedName << " = Ice.ValueHolder();";
                            name = "@(v) " + r->fixedName + ".set(v)";
                        }
                        else
                        {
                            name = r->fixedName;
                        }
                        unmarshal(out, "is_", name, r->type, r->optional, r->tag);
                    }
                    if(op->returnsClasses(false))
                    {
                        out << nl << "is_.readPendingValues();";
                    }
                    out << nl << "is_.endEncapsulation();";
                    for(ParamInfoList::const_iterator r = requiredOutParams.begin(); r != requiredOutParams.end(); ++r)
                    {
                        if(isClass(r->type))
                        {
                            out << nl << "varargout{" << r->pos << "} = " << r->fixedName << ".value;";
                        }
                        else if(needsConversion(r->type))
                        {
                            ostringstream dest;
                            dest << "varargout{" << r->pos << "}";
                            convertValueType(out, dest.str(), r->fixedName, r->type, r->optional);
                        }
                        else
                        {
                            out << nl << "varargout{" << r->pos << "} = " << r->fixedName << ';';
                        }
                    }
                    for(ParamInfoList::const_iterator r = optionalOutParams.begin(); r != optionalOutParams.end(); ++r)
                    {
                        if(isClass(r->type))
                        {
                            out << nl << "varargout{" << r->pos << "} = " << r->fixedName << ".value;";
                        }
                        else if(needsConversion(r->type))
                        {
                            ostringstream dest;
                            dest << "varargout{" << r->pos << "}";
                            convertValueType(out, dest.str(), r->fixedName, r->type, r->optional);
                        }
                        else
                        {
                            out << nl << "varargout{" << r->pos << "} = " << r->fixedName << ';';
                        }
                    }
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
                << (allInParams.empty() ? "[]" : "os_") << ", " << allOutParams.size() << ", "
                << (twowayOnly ? "@unmarshal" : "[]") << ", varargin{:});";

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
        out << nl << "r = Ice.ObjectPrx.read_(is_, '" << prxAbs << "');";
        out.dec();
        out << nl << "end";
        out << nl << "function r = checkedCast(p, varargin)";
        out.inc();
        out << nl << "r = Ice.ObjectPrx.checkedCast_(p, " << prxAbs << ".ice_staticId(), '" << prxAbs
            << "', varargin{:});";
        out.dec();
        out << nl << "end";
        out << nl << "function r = uncheckedCast(p, varargin)";
        out.inc();
        out << nl << "r = Ice.ObjectPrx.uncheckedCast_(p, '" << prxAbs << "', varargin{:});";
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";

        //
        // Constructor.
        //
        out << nl << "methods(Hidden=true)";
        out.inc();
        out << nl << "function obj = " << prxName << "(impl, communicator)";
        out.inc();
        if(!bases.empty())
        {
            for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
            {
                out << nl << "obj = obj@" << getAbsolute(*q, "", "Prx") << "(impl, communicator);";
            }
        }
        else
        {
            out << nl << "obj = obj@Ice.ObjectPrx(impl, communicator);";
        }
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
        openClass(abs, out);

        out << nl << "classdef (Abstract) " << name;
        if(bases.empty())
        {
            out << " < handle";
        }
        else
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
                const ParamInfoList outParams = getAllOutParams(op);
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
                const ParamInfoList inParams = getAllInParams(op);
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
    const string abs = getAbsolute(p);
    const bool basePreserved = p->inheritsMetaData("preserve-slice");
    const bool preserved = p->hasMetaData("preserve-slice");

    IceUtilInternal::Output out;
    openClass(abs, out);

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
    out.inc();

    const DataMemberList members = p->dataMembers();
    if(!members.empty())
    {
        out << nl << "properties";
        out.inc();
        for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
        {
            out << nl << fixExceptionMemberIdent((*q)->name());
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
    MemberInfoList convertMembers;
    for(MemberInfoList::const_iterator q = allMembers.begin(); q != allMembers.end(); ++q)
    {
        allNames.push_back(q->fixedName);

        if(!q->inherited && needsConversion(q->dataMember->type()))
        {
            convertMembers.push_back(*q);
        }
    }
    out << nl << "methods";
    out.inc();

    const string self = name == "obj" ? "this" : "obj";

    //
    // Constructor
    //
    out << nl << "function " << self << " = " << name << spar << "ice_exid" << "ice_exmsg" << allNames << epar;
    out.inc();
    string exid = abs;
    const string exmsg = abs; // TODO: Allow a message to be specified via metadata?
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

    if(!p->isLocal() && preserved && !basePreserved)
    {
        out << nl << "function r = ice_getSlicedData(obj)";
        out.inc();
        out << nl << "r = obj.iceSlicedData_;";
        out.dec();
        out << nl << "end";
    }

    out.dec();
    out << nl << "end";

    if(!p->isLocal())
    {
        const DataMemberList classMembers = p->classDataMembers();
        if(!classMembers.empty() || !convertMembers.empty() || (preserved && !basePreserved))
        {
            out << nl << "methods(Hidden=true)";
            out.inc();

            if(preserved && !basePreserved)
            {
                //
                // Override read_ for the first exception in the hierarchy that has the "preserve-slice" metadata.
                //
                out << nl << "function obj = read_(obj, is)";
                out.inc();
                out << nl << "is.startException();";
                out << nl << "obj = obj.readImpl_(is);";
                out << nl << "obj.iceSlicedData_ = is.endException(true);";
                out.dec();
                out << nl << "end";
            }

            if(!classMembers.empty() || !convertMembers.empty())
            {
                out << nl << "function obj = postUnmarshal_(obj)";
                out.inc();
                for(DataMemberList::const_iterator q = classMembers.begin(); q != classMembers.end(); ++q)
                {
                    string m = fixExceptionMemberIdent((*q)->name());
                    out << nl << "obj." << m << " = obj." << m << ".value;";
                }
                for(MemberInfoList::const_iterator q = convertMembers.begin(); q != convertMembers.end(); ++q)
                {
                    string m = "obj." + q->fixedName;
                    convertValueType(out, m, m, q->dataMember->type(), q->dataMember->optional());
                }
                if(base && base->usesClasses(true))
                {
                    out << nl << "obj = postUnmarshal_@" << getAbsolute(base) << "(obj);";
                }
                out.dec();
                out << nl << "end";
            }

            out.dec();
            out << nl << "end";
        }

        out << nl << "methods(Access=protected)";
        out.inc();

        out << nl << "function obj = readImpl_(obj, is)";
        out.inc();
        out << nl << "is.startSlice();";
        for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
        {
            string m = fixExceptionMemberIdent((*q)->name());
            if(!(*q)->optional())
            {
                if(isClass((*q)->type()))
                {
                    out << nl << "obj." << m << " = Ice.ValueHolder();";
                    unmarshal(out, "is", "@(v) obj." + m + ".set(v)", (*q)->type(), false, 0);
                }
                else
                {
                    unmarshal(out, "is", "obj." + m, (*q)->type(), false, 0);
                }
            }
        }
        const DataMemberList optionalMembers = p->orderedOptionalDataMembers();
        for(DataMemberList::const_iterator q = optionalMembers.begin(); q != optionalMembers.end(); ++q)
        {
            string m = fixExceptionMemberIdent((*q)->name());
            if(isClass((*q)->type()))
            {
                out << nl << "obj." << m << " = Ice.ValueHolder();";
                unmarshal(out, "is", "@(v) obj." + m + ".set(v)", (*q)->type(), true, (*q)->tag());
            }
            else
            {
                unmarshal(out, "is", "obj." + m, (*q)->type(), true, (*q)->tag());
            }
        }
        out << nl << "is.endSlice();";
        if(base)
        {
            out << nl << "obj = readImpl_@" << getAbsolute(base) << "(obj, is);";
        }
        out.dec();
        out << nl << "end";

        out.dec();
        out << nl << "end";

        if(preserved && !basePreserved)
        {
            out << nl << "properties(Access=protected)";
            out.inc();
            out << nl << "iceSlicedData_";
            out.dec();
            out << nl << "end";
        }
    }

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
    openClass(abs, out);

    const DataMemberList members = p->dataMembers();
    const DataMemberList classMembers = p->classDataMembers();

    out << nl << "classdef " << name;

    out.inc();
    out << nl << "properties";
    out.inc();
    vector<string> memberNames;
    DataMemberList convertMembers;
    for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
    {
        const string m = fixIdent((*q)->name());
        memberNames.push_back(m);
        out << nl << m;
        if(declarePropertyType((*q)->type(), false))
        {
            out << " " << typeToString((*q)->type());
        }

        if(needsConversion((*q)->type()))
        {
            convertMembers.push_back(*q);
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
    out << nl << "function r = eq(obj, other)";
    out.inc();
    out << nl << "r = isequal(obj, other);";
    out.dec();
    out << nl << "end";
    out << nl << "function r = ne(obj, other)";
    out.inc();
    out << nl << "r = ~isequal(obj, other);";
    out.dec();
    out << nl << "end";

    if(!convertMembers.empty() || !classMembers.empty())
    {
        out << nl << "function obj = ice_convert(obj)";
        out.inc();
        for(DataMemberList::const_iterator q = convertMembers.begin(); q != convertMembers.end(); ++q)
        {
            string m = "obj." + fixIdent((*q)->name());
            convertValueType(out, m, m, (*q)->type(), (*q)->optional());
        }
        for(DataMemberList::const_iterator q = classMembers.begin(); q != classMembers.end(); ++q)
        {
            string m = "obj." + fixIdent((*q)->name());
            out << nl << m << " = " << m << ".value;";
        }
        out.dec();
        out << nl << "end";
    }

    out.dec();
    out << nl << "end";

    if(!p->isLocal())
    {
        out << nl << "methods(Static)";
        out.inc();
        out << nl << "function r = ice_read(is)";
        out.inc();
        out << nl << "r = " << abs << "();";
        for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
        {
            if(isClass((*q)->type()))
            {
                string m = fixIdent((*q)->name());
                out << nl << "r." << m << " = Ice.ValueHolder();";
                unmarshal(out, "is", "@(v) r." + m + ".set(v)", (*q)->type(), false, 0);
            }
            else
            {
                unmarshal(out, "is", "r." + fixIdent((*q)->name()), (*q)->type(), false, 0);
            }
        }
        out.dec();
        out << nl << "end";

        out << nl << "function r = ice_readOpt(is, tag)";
        out.inc();
        out << nl << "if is.readOptional(tag, " << getOptionalFormat(p) << ")";
        out.inc();
        if(p->isVariableLength())
        {
            out << nl << "is.skip(4);";
        }
        else
        {
            out << nl << "is.skipSize();";
        }
        out << nl << "r = " << abs << ".ice_read(is);";
        out.dec();
        out << nl << "else";
        out.inc();
        out << nl << "r = Ice.Unset;";
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";

        out << nl << "function ice_write(os, v)";
        out.inc();
        out << nl << "if isempty(v)";
        out.inc();
        out << nl << "v = " << abs << "();";
        out.dec();
        out << nl << "end";
        for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
        {
            marshal(out, "os", "v." + fixIdent((*q)->name()), (*q)->type(), false, 0);
        }
        out.dec();
        out << nl << "end";

        out << nl << "function ice_writeOpt(os, tag, v)";
        out.inc();
        out << nl << "if v ~= Ice.Unset && os.writeOptional(tag, " << getOptionalFormat(p) << ")";
        out.inc();
        if(p->isVariableLength())
        {
            out << nl << "pos = os.startSize();";
            out << nl << abs << ".ice_write(os, v);";
            out << nl << "os.endSize(pos);";
        }
        else
        {
            out << nl << "os.writeSize(" << p->minWireSize() << ");";
            out << nl << abs << ".ice_write(os, v);";
        }
        out.dec();
        out << nl << "end";
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
    const string abs = getAbsolute(p);
    const bool cls = isClass(content);
    const bool convert = needsConversion(content);

    IceUtilInternal::Output out;
    openClass(abs, out);

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
    marshal(out, "os", "seq{i}", content, false, 0);
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    out << nl << "function writeOpt(os, tag, seq)";
    out.inc();
    out << nl << "if seq ~= Ice.Unset && os.writeOptional(tag, " << getOptionalFormat(p) << ")";
    out.inc();
    if(p->type()->isVariableLength())
    {
        out << nl << "pos = os.startSize();";
        out << nl << abs << ".write(os, seq);";
        out << nl << "os.endSize(pos);";
    }
    else
    {
        //
        // The element is a fixed-size type. If the element type is bool or byte, we do NOT write an extra size.
        //
        const size_t sz = p->type()->minWireSize();
        if(sz > 1)
        {
            out << nl << "len = length(seq);";
            out << nl << "if len > 254";
            out.inc();
            out << nl << "os.writeSize(len * " << sz << " + 5);";
            out.dec();
            out << nl << "else";
            out.inc();
            out << nl << "os.writeSize(len * " << sz << " + 1);";
            out .dec();
            out << nl << "end";
        }
        out << nl << abs << ".write(os, seq);";
    }
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    out << nl << "function r = read(is)";
    out.inc();
    out << nl << "sz = is.readSize();";
    if(cls)
    {
        //
        // For a sequence<class>, read() returns an instance of Ice.CellArrayHandle that we later replace with
        // the cell array. See convert().
        //
        out << nl << "r = Ice.CellArrayHandle();";
        out << nl << "r.array = cell(1, sz);";
    }
    else
    {
        out << nl << "r = cell(1, sz);";
    }
    out << nl << "for i = 1:sz";
    out.inc();
    if(cls)
    {
        //
        // Ice.CellArrayHandle defines a set() method that we call from the lambda.
        //
        unmarshal(out, "is", "@(v) r.set(i, v)", content, false, 0);
    }
    else
    {
        unmarshal(out, "is", "r{i}", content, false, 0);
    }
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    out << nl << "function r = readOpt(is, tag)";
    out.inc();
    out << nl << "if is.readOptional(tag, " << getOptionalFormat(p) << ")";
    out.inc();
    if(p->type()->isVariableLength())
    {
        out << nl << "is.skip(4);";
    }
    else if(p->type()->minWireSize() > 1)
    {
        out << nl << "is.skipSize();";
    }
    out << nl << "r = " << abs << ".read(is);";
    out.dec();
    out << nl << "else";
    out.inc();
    out << nl << "r = Ice.Unset;";
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    if(cls || convert)
    {
        out << nl << "function r = convert(seq)";
        out.inc();
        if(cls)
        {
            out << nl << "r = seq.array;";
        }
        else
        {
            out << nl << "r = cell(1, length(seq));";
            out << nl << "for i = 1:length(seq)";
            out.inc();
            convertValueType(out, "r{i}", "seq{i}", content, false);
            out.dec();
            out << nl << "end";
        }
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
CodeVisitor::visitDictionary(const DictionaryPtr& p)
{
    const TypePtr key = p->keyType();
    const TypePtr value = p->valueType();
    const bool cls = isClass(value);
    const bool convert = needsConversion(value);

    const StructPtr st = StructPtr::dynamicCast(key);

    const string name = fixIdent(p->name());
    const string scoped = p->scoped();
    const string abs = getAbsolute(p);
    const string self = name == "obj" ? "this" : "obj";

    IceUtilInternal::Output out;
    openClass(abs, out);

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
        //
        // We use a struct array when the key is a structure type because we can't use containers.Map.
        //
        out << nl << "r = [];";
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
            marshal(out, "os", "d(i).key", key, false, 0);
            marshal(out, "os", "d(i).value", value, false, 0);
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
            out << nl << "k = keys{i};";
            out << nl << "v = values{i};";
            marshal(out, "os", "k", key, false, 0);
            marshal(out, "os", "v", value, false, 0);
            out.dec();
            out << nl << "end";
        }
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";

        out << nl << "function writeOpt(os, tag, d)";
        out.inc();
        out << nl << "if d ~= Ice.Unset && os.writeOptional(tag, " << getOptionalFormat(p) << ")";
        out.inc();
        if(key->isVariableLength() || value->isVariableLength())
        {
            out << nl << "pos = os.startSize();";
            out << nl << abs << ".write(os, d);";
            out << nl << "os.endSize(pos);";
        }
        else
        {
            const size_t sz = key->minWireSize() + value->minWireSize();
            if(cls)
            {
                out << nl << "len = length(d.array);";
            }
            else
            {
                out << nl << "len = length(d);";
            }
            out << nl << "if len > 254";
            out.inc();
            out << nl << "os.writeSize(len * " << sz << " + 5);";
            out.dec();
            out << nl << "else";
            out.inc();
            out << nl << "os.writeSize(len * " << sz << " + 1);";
            out .dec();
            out << nl << "end";
            out << nl << abs << ".write(os, d);";
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

        unmarshal(out, "is", "k", key, false, 0);

        if(cls)
        {
            out << nl << "v = Ice.ValueHolder();";
            unmarshal(out, "is", "@(v_) v.set(v_)", value, false, 0);
        }
        else
        {
            unmarshal(out, "is", "v", value, false, 0);
        }

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

        out << nl << "function r = readOpt(is, tag)";
        out.inc();
        out << nl << "if is.readOptional(tag, " << getOptionalFormat(p) << ")";
        out.inc();
        if(key->isVariableLength() || value->isVariableLength())
        {
            out << nl << "is.skip(4);";
        }
        else
        {
            out << nl << "is.skipSize();";
        }
        out << nl << "r = " << abs << ".read(is);";
        out.dec();
        out << nl << "else";
        out.inc();
        out << nl << "r = Ice.Unset;";
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";

        if(cls || convert)
        {
            out << nl << "function r = convert(d, obj)";
            out.inc();
            if(st)
            {
                out << nl << "for i = 1:length(d)";
                out.inc();
                if(cls)
                {
                    //
                    // Each entry has a temporary Ice.ValueHolder that we need to replace with the actual value.
                    //
                    out << nl << "d(i).value = d(i).value.value;";
                }
                else
                {
                    convertValueType(out, "d(i).value", "d(i).value", value, false);
                }
                out.dec();
                out << nl << "end";
            }
            else
            {
                out << nl << "keys = d.keys();";
                out << nl << "values = d.values();";
                out << nl << "for i = 1:d.Count";
                out.inc();
                out << nl << "k = keys{i};";
                out << nl << "v = values{i};";
                if(cls)
                {
                    //
                    // Each entry has a temporary Ice.ValueHolder that we need to replace with the actual value.
                    //
                    out << nl << "d(k) = v.value;";
                }
                else
                {
                    convertValueType(out, "d(k)", "v", value, false);
                }
                out.dec();
                out << nl << "end";
            }
            out << nl << "r = d;";
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
}

void
CodeVisitor::visitEnum(const EnumPtr& p)
{
    const string name = fixIdent(p->name());
    const string scoped = p->scoped();
    const string abs = getAbsolute(p);

    IceUtilInternal::Output out;
    openClass(abs, out);

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

        out << nl << "function ice_writeOpt(os, tag, v)";
        out.inc();
        out << nl << "if v ~= Ice.Unset && os.writeOptional(tag, " << getOptionalFormat(p) << ")";
        out.inc();
        out << nl << abs << ".ice_write(os, v);";
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

        out << nl << "function r = ice_readOpt(is, tag)";
        out.inc();
        out << nl << "if is.readOptional(tag, " << getOptionalFormat(p) << ")";
        out.inc();
        out << nl << "r = " << abs << ".ice_read(is);";
        out.dec();
        out << nl << "else";
        out.inc();
        out << nl << "r = Ice.Unset;";
        out.dec();
        out << nl << "end";
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
    const string abs = getAbsolute(p);

    IceUtilInternal::Output out;
    openClass(abs, out);

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
CodeVisitor::openClass(const string& abs, IceUtilInternal::Output& out)
{
    vector<string> v = splitAbsoluteName(abs);
    assert(v.size() > 1);

    string path;
    if(!_dir.empty())
    {
        path = _dir + "/";
    }

    //
    // Create a package directory corresponding to each component.
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
        m.fixedName = fixExceptionMemberIdent((*q)->name());
        m.inherited = inherited;
        m.dataMember = *q;
        allMembers.push_back(m);
    }
}

CodeVisitor::ParamInfoList
CodeVisitor::getAllInParams(const OperationPtr& op)
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

void
CodeVisitor::getInParams(const OperationPtr& op, ParamInfoList& required, ParamInfoList& optional)
{
    const ParamInfoList params = getAllInParams(op);
    for(ParamInfoList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        if(p->optional)
        {
            optional.push_back(*p);
        }
        else
        {
            required.push_back(*p);
        }
    }

    //
    // Sort optional parameters by tag.
    //
    class SortFn
    {
    public:
        static bool compare(const ParamInfo& lhs, const ParamInfo& rhs)
        {
            return lhs.tag < rhs.tag;
        }
    };
    optional.sort(SortFn::compare);
}

CodeVisitor::ParamInfoList
CodeVisitor::getAllOutParams(const OperationPtr& op)
{
    ParamDeclList params = op->outParameters();
    ParamInfoList l;
    int pos = 1;

    if(op->returnType())
    {
        ParamInfo info;
        info.fixedName = "result";
        info.pos = pos++;

        for(ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
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
        l.push_back(info);
    }

    for(ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        ParamInfo info;
        info.fixedName = fixIdent((*p)->name());
        info.type = (*p)->type();
        info.optional = (*p)->optional();
        info.tag = (*p)->tag();
        info.pos = pos++;
        info.param = *p;
        l.push_back(info);
    }

    return l;
}

void
CodeVisitor::getOutParams(const OperationPtr& op, ParamInfoList& required, ParamInfoList& optional)
{
    const ParamInfoList params = getAllOutParams(op);
    for(ParamInfoList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        if(p->optional)
        {
            optional.push_back(*p);
        }
        else
        {
            required.push_back(*p);
        }
    }

    //
    // Sort optional parameters by tag.
    //
    class SortFn
    {
    public:
        static bool compare(const ParamInfo& lhs, const ParamInfo& rhs)
        {
            return lhs.tag < rhs.tag;
        }
    };
    optional.sort(SortFn::compare);
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
        case Builtin::KindValue:
        {
            return "Ice.OptionalFormat.Class";
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
                     bool optional, int tag)
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
            out << nl << typeS << ".ice_writeOpt(" << stream << ", " << tag << ", " << v << ");";
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
            out << nl << typeS << ".ice_writeOpt(" << stream << ", " << tag << ", " << v << ");";
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
            out << nl << getAbsolute(dict) << ".writeOpt(" << stream << ", " << tag << ", " << v << ");";
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
            out << nl << getAbsolute(seq) << ".writeOpt(" << stream << ", " << tag << ", " << v << ");";
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
                       bool optional, int tag)
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
                if(optional)
                {
                    out << nl << stream << ".readValueOpt(" << tag << ", " << v << ", 'Ice.Value');";
                }
                else
                {
                    out << nl << stream << ".readValue(" << v << ", 'Ice.Value');";
                }
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
        if(prx->_class()->isInterface())
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
        }
        else
        {
            if(optional)
            {
                out << nl << v << " = " << stream << ".readProxyOpt(" << tag << ");";
            }
            else
            {
                out << nl << v << " = " << stream << ".readProxy();";
            }
        }
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        const string cls = cl->isInterface() ? "Ice.Value" : getAbsolute(cl);
        if(optional)
        {
            out << nl << stream << ".readValueOpt(" << tag << ", " << v << ", '" << cls << "');";
        }
        else
        {
            out << nl << stream << ".readValue(" << v << ", '" << cls << "');";
        }
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        const string typeS = getAbsolute(st);
        if(optional)
        {
            out << nl << v << " = " << typeS << ".ice_readOpt(" << stream << ", " << tag << ");";
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
            out << nl << v << " = " << typeS << ".ice_readOpt(" << stream << ", " << tag << ");";
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
            out << nl << v << " = " << getAbsolute(dict) << ".readOpt(" << stream << ", " << tag << ");";
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
            out << nl << v << " = " << getAbsolute(seq) << ".readOpt(" << stream << ", " << tag << ");";
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
