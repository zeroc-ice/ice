// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/RubyUtil.h>
#include <Slice/Checksum.h>
#include <Slice/Util.h>
#include <IceUtil/Functional.h>
#include <IceUtil/InputUtil.h>
#include <iterator>

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

namespace
{

string
getEscapedParamName(const OperationPtr& p, const string& name)
{
    ParamDeclList params = p->parameters();

    for(ParamDeclList::const_iterator i = params.begin(); i != params.end(); ++i)
    {
        if((*i)->name() == name)
        {
            return name + "_";
        }
    }
    return name;
}

}

namespace Slice
{
namespace Ruby
{

//
// CodeVisitor generates the Ruby mapping for a translation unit.
//
class CodeVisitor : public ParserVisitor
{
public:

    CodeVisitor(IceUtilInternal::Output&);

    virtual bool visitModuleStart(const ModulePtr&);
    virtual void visitModuleEnd(const ModulePtr&);
    virtual void visitClassDecl(const ClassDeclPtr&);
    virtual bool visitClassDefStart(const ClassDefPtr&);
    virtual bool visitExceptionStart(const ExceptionPtr&);
    virtual bool visitStructStart(const StructPtr&);
    virtual void visitSequence(const SequencePtr&);
    virtual void visitDictionary(const DictionaryPtr&);
    virtual void visitEnum(const EnumPtr&);
    virtual void visitConst(const ConstPtr&);

private:

    //
    // Return a Ruby symbol for the given parser element.
    //
    string getSymbol(const ContainedPtr&);

    //
    // Emit Ruby code to assign the given symbol in the current module.
    //
    void registerName(const string&);

    //
    // Emit the array that describes a Slice type.
    //
    void writeType(const TypePtr&);

    //
    // Get an initializer value for a given type.
    //
    string getInitializer(const DataMemberPtr&);

    //
    // Add a value to a hash code.
    //
    void writeHash(const string&, const TypePtr&, int&);

    //
    // Write a constant value.
    //
    void writeConstantValue(const TypePtr&, const SyntaxTreeBasePtr&, const string&);

    struct MemberInfo
    {
        string lowerName; // Mapped name beginning with a lower-case letter for use as the name of a local variable.
        string fixedName;
        bool inherited;
        DataMemberPtr dataMember;
    };
    typedef list<MemberInfo> MemberInfoList;

    //
    // Write constructor parameters with default values.
    //
    void writeConstructorParams(const MemberInfoList&);

    void collectClassMembers(const ClassDefPtr&, MemberInfoList&, bool);
    void collectExceptionMembers(const ExceptionPtr&, MemberInfoList&, bool);

    Output& _out;
    set<string> _classHistory;
};

}
}

static string
lookupKwd(const string& name)
{
    //
    // Keyword list. *Must* be kept in alphabetical order.
    //
    // This list only contains keywords that might conflict with a Slice
    // identifier, so keywords like "defined?" are not necessary.
    //
    // This list also contains the names of methods on Object that might
    // conflict with a Slice identifier, so names such as "inspect" and
    // "send" are included but "to_s" is not.
    //
    static const string keywordList[] =
    {
        "BEGIN", "END", "alias", "and", "begin", "break", "case", "class", "clone", "def", "display", "do", "dup",
        "else", "elsif", "end", "ensure", "extend", "false", "for", "freeze", "hash", "if", "in", "initialize_copy",
        "inspect", "instance_eval", "instance_variable_get", "instance_variable_set", "instance_variables", "method",
        "method_missing", "methods", "module", "new", "next", "nil", "not", "object_id", "or", "private_methods",
        "protected_methods", "public_methods", "redo", "rescue", "retry", "return", "self", "send",
        "singleton_methods", "super", "taint", "then", "to_a", "to_s", "true", "undef", "unless", "untaint", "until",
        "when", "while", "yield"
    };
    bool found =  binary_search(&keywordList[0],
                                &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
                                name);
    return found ? "_" + name : name;
}

//
// Split a scoped name into its components and return the components as a list of (unscoped) identifiers.
//
static vector<string>
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
            if(endpos != string::npos && endpos > pos)
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

    return ids;
}

//
// CodeVisitor implementation.
//
Slice::Ruby::CodeVisitor::CodeVisitor(Output& out) :
    _out(out)
{
}

bool
Slice::Ruby::CodeVisitor::visitModuleStart(const ModulePtr& p)
{
    _out << sp << nl << "module ";
    //
    // Ensure that Slice top-level modules are defined as top
    // level modules in Ruby
    //
    if(UnitPtr::dynamicCast(p->container()))
    {
        _out << "::";
    }
    _out << fixIdent(p->name(), IdentToUpper);
    _out.inc();
    return true;
}

void
Slice::Ruby::CodeVisitor::visitModuleEnd(const ModulePtr&)
{
    _out.dec();
    _out << nl << "end";
}

void
Slice::Ruby::CodeVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    //
    // Emit forward declarations.
    //
    string scoped = p->scoped();
    if(_classHistory.count(scoped) == 0)
    {
        string name = "T_" + fixIdent(p->name(), IdentToUpper);
        _out << sp << nl << "if not defined?(" << getAbsolute(p, IdentToUpper, "T_") << ')';
        _out.inc();
        if(p->isLocal())
        {
            _out << nl << name << " = ::Ice::__declareLocalClass('" << scoped << "')";
        }
        else
        {
            _out << nl << name << " = ::Ice::__declareClass('" << scoped << "')";
            _out << nl << name << "Prx = ::Ice::__declareProxy('" << scoped << "')";
        }
        _out.dec();
        _out << nl << "end";
        _classHistory.insert(scoped); // Avoid redundant declarations.
    }
}

bool
Slice::Ruby::CodeVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    bool isInterface = p->isInterface();
    bool isLocal = p->isLocal();
    bool isAbstract = isInterface || p->allOperations().size() > 0; // Don't use isAbstract() - see bug 3739

    //
    // Do not generate any code for local interfaces.
    //
    if(isLocal && isInterface)
    {
        return false;
    }

    _out << sp << nl << "if not defined?(" << getAbsolute(p, IdentToUpper) << "_Mixin)";
    _out.inc();

    //
    // Marker to avoid redefinitions, we don't use the actual class names at those might
    // be defined by IceRuby for some internal classes
    //
    _out << sp << nl << "module " << getAbsolute(p, IdentToUpper) << "_Mixin";
    _out << nl << "end";

    string scoped = p->scoped();
    string name = fixIdent(p->name(), IdentToUpper);
    ClassList bases = p->bases();
    ClassDefPtr base;
    OperationList ops = p->operations();

    DataMemberList members = p->dataMembers();

    if(isLocal || !isInterface)
    {
        if(!bases.empty() && !bases.front()->isInterface())
        {
            base = bases.front();
        }

        _out << nl << "class " << name;
        if(base)
        {
            _out << " < " << getAbsolute(base, IdentToUpper);
        }
        else if(!isLocal)
        {
            _out << " < ::Ice::Value";
        }
        _out.inc();

        //
        // initialize
        //
        MemberInfoList allMembers;
        collectClassMembers(p, allMembers, false);
        if(!allMembers.empty())
        {
            _out << sp << nl << "def initialize(";
            writeConstructorParams(allMembers);
            _out << ')';
            _out.inc();

            bool inheritsMembers = false;
            for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
            {
                if(q->inherited)
                {
                    inheritsMembers = true;
                    break;
                }
            }

            if(inheritsMembers)
            {
                _out << nl << "super" << spar;
                for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
                {
                    if(q->inherited)
                    {
                        _out << q->lowerName;
                    }
                }
                _out << epar;
            }

            for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
            {
                if(!q->inherited)
                {
                    _out << nl << '@' << q->fixedName << " = " << q->lowerName;
                }
            }

            _out.dec();
            _out << nl << "end";
        }

        //
        // read/write accessors for data members.
        //
        if(!members.empty())
        {
            bool prot = p->hasMetaData("protected");
            DataMemberList protectedMembers;

            _out << sp << nl << "attr_accessor ";
            for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
            {
                if(q != members.begin())
                {
                    _out << ", ";
                }
                _out << ":" << fixIdent((*q)->name(), IdentNormal);
                if(prot || (*q)->hasMetaData("protected"))
                {
                    protectedMembers.push_back(*q);
                }
            }

            if(!protectedMembers.empty())
            {
                _out << nl << "protected ";
                for(DataMemberList::iterator q = protectedMembers.begin(); q != protectedMembers.end(); ++q)
                {
                    if(q != protectedMembers.begin())
                    {
                        _out << ", ";
                    }
                    //
                    // We need to list the symbols of the reader and the writer (e.g., ":member" and ":member=").
                    //
                    _out << ":" << fixIdent((*q)->name(), IdentNormal) << ", :"
                        << fixIdent((*q)->name(), IdentNormal) << '=';
                }
            }
        }

        _out.dec();
        _out << nl << "end"; // End of class.
    }

    //
    // Generate proxy support. This includes a mix-in module for the proxy's
    // operations and a class for the proxy itself.
    //
    if(!p->isLocal() && isAbstract)
    {
        _out << nl << "module " << name << "Prx_mixin";
        _out.inc();
        for(ClassList::iterator cli = bases.begin(); cli != bases.end(); ++cli)
        {
            ClassDefPtr def = *cli;
            if(def->isInterface() || def->allOperations().size() > 0)
            {
                _out << nl << "include " << getAbsolute(*cli, IdentToUpper) << "Prx_mixin";
            }
        }
        for(OperationList::iterator oli = ops.begin(); oli != ops.end(); ++oli)
        {
            string fixedOpName = fixIdent((*oli)->name(), IdentNormal);
            if(fixedOpName == "checkedCast" || fixedOpName == "uncheckedCast")
            {
                fixedOpName.insert(0, "_");
            }
            TypePtr ret = (*oli)->returnType();
            ParamDeclList paramList = (*oli)->parameters();
            string inParams;

            for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
            {
                if(!(*q)->isOutParam())
                {
                    if(!inParams.empty())
                    {
                        inParams.append(", ");
                    }
                    inParams.append(fixIdent((*q)->name(), IdentToLower));
                }
            }

            _out << sp << nl << "def " << fixedOpName << "(";
            if(!inParams.empty())
            {
                _out << inParams << ", ";
            }
            const string contextParamName = getEscapedParamName(*oli, "context");
            _out << contextParamName << "=nil)";
            _out.inc();
            _out << nl << name << "Prx_mixin::OP_" << (*oli)->name() << ".invoke(self, [" << inParams;
            _out << "], " << contextParamName << ")";
            _out.dec();
            _out << nl << "end";
        }
        _out.dec();
        _out << nl << "end"; // End of mix-in module for proxy.

        _out << sp << nl << "class " << name << "Prx < ::Ice::ObjectPrx";
        _out.inc();
        _out << nl << "include ::Ice::Proxy_mixin";
        _out << nl << "include " << name << "Prx_mixin";
        _out.dec();
        _out << nl << "end"; // End of proxy class.
    }

    //
    // Emit type descriptions.
    //
    _out << sp << nl << "if not defined?(" << getAbsolute(p, IdentToUpper, "T_");
    if(isInterface)
    {
        _out << "Prx";
    }
    _out << ')';
    _out.inc();
    if(p->isLocal())
    {
        _out << nl << "T_" << name << " = ::Ice::__declareLocalClass('" << scoped << "')";
    }
    else
    {
        _out << nl << "T_" << name << " = ::Ice::__declareClass('" << scoped << "')";
        if(isAbstract)
        {
            _out << nl << "T_" << name << "Prx = ::Ice::__declareProxy('" << scoped << "')";
        }
    }
    _out.dec();
    _out << nl << "end";
    _classHistory.insert(scoped); // Avoid redundant declarations.

    const bool preserved = p->hasMetaData("preserve-slice") || p->inheritsMetaData("preserve-slice");

    _out << sp << nl << "T_" << name << ".defineClass("
         << (isInterface ? "::Ice::Value" : name) << ", "
         << p->compactId() << ", "
         << (preserved ? "true" : "false") << ", "
         << (isInterface ? "true" : "false") << ", ";
    if(!base)
    {
        _out << "nil";
    }
    else
    {
        _out << getAbsolute(base, IdentToUpper, "T_");
    }
    _out << ", ";
    //
    // Members
    //
    // Data members are represented as an array:
    //
    //   ['MemberName', MemberType, Optional, Tag]
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a constructed type.
    //
    _out << "[";
    if(members.size() > 1)
    {
        _out.inc();
        _out << nl;
    }
    {
        for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
        {
            if(q != members.begin())
            {
                _out << ',' << nl;
            }
            _out << "['" << fixIdent((*q)->name(), IdentNormal) << "', ";
            writeType((*q)->type());
            _out << ", " << ((*q)->optional() ? "true" : "false") << ", " << ((*q)->optional() ? (*q)->tag() : 0)
                 << ']';
        }
    }
    if(members.size() > 1)
    {
        _out.dec();
        _out << nl;
    }
    _out << "])";

    //
    // Define each operation. The arguments to __defineOperation are:
    //
    // 'opName', Mode, IsAmd, FormatType, [InParams], [OutParams], ReturnParam, [Exceptions]
    //
    // where InParams and OutParams are arrays of type descriptions, and Exceptions
    // is an array of exception types.
    //
    if(!p->isLocal() && isAbstract)
    {
        _out << sp << nl << "T_" << name << "Prx.defineProxy(" << name << "Prx, ";

        if(!base || (!base->isInterface() && base->allOperations().size() == 0))
        {
            _out << "nil";
        }
        else
        {
            _out << getAbsolute(base, IdentToUpper, "T_") << "Prx";
        }

        //
        // Interfaces
        //
        _out << ", [";
        {
            int interfaceCount = 0;
            for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
            {
                if((*q)->isInterface())
                {
                    if(interfaceCount > 0)
                    {
                        _out << ", ";
                    }
                    _out << getAbsolute(*q, IdentToUpper, "T_") << "Prx";
                    ++interfaceCount;
                }
            }
        }
        _out << "])";

        if(!ops.empty())
        {
            _out << sp;
        }
        for(OperationList::iterator s = ops.begin(); s != ops.end(); ++s)
        {
            ParamDeclList params = (*s)->parameters();
            ParamDeclList::iterator t;
            int count;
            string format;
            switch((*s)->format())
            {
            case DefaultFormat:
                format = "nil";
                break;
            case CompactFormat:
                format = "::Ice::FormatType::CompactFormat";
                break;
            case SlicedFormat:
                format = "::Ice::FormatType::SlicedFormat";
                break;
            }

            _out << nl << name << "Prx_mixin::OP_" << (*s)->name() << " = ::Ice::__defineOperation('"
                 << (*s)->name() << "', ";
            switch((*s)->mode())
            {
            case Operation::Normal:
                _out << "::Ice::OperationMode::Normal";
                break;
            case Operation::Nonmutating:
                _out << "::Ice::OperationMode::Nonmutating";
                break;
            case Operation::Idempotent:
                _out << "::Ice::OperationMode::Idempotent";
                break;
            }
            _out << ", ";
            switch((*s)->sendMode())
            {
            case Operation::Normal:
                _out << "::Ice::OperationMode::Normal";
                break;
            case Operation::Nonmutating:
                _out << "::Ice::OperationMode::Nonmutating";
                break;
            case Operation::Idempotent:
                _out << "::Ice::OperationMode::Idempotent";
                break;
            }
            _out << ", " << ((p->hasMetaData("amd") || (*s)->hasMetaData("amd")) ? "true" : "false") << ", " << format
                 << ", [";
            for(t = params.begin(), count = 0; t != params.end(); ++t)
            {
                if(!(*t)->isOutParam())
                {
                    if(count > 0)
                    {
                        _out << ", ";
                    }
                    _out << '[';
                    writeType((*t)->type());
                    _out << ", " << ((*t)->optional() ? "true" : "false") << ", "
                         << ((*t)->optional() ? (*t)->tag() : 0) << ']';
                    ++count;
                }
            }
            _out << "], [";
            for(t = params.begin(), count = 0; t != params.end(); ++t)
            {
                if((*t)->isOutParam())
                {
                    if(count > 0)
                    {
                        _out << ", ";
                    }
                    _out << '[';
                    writeType((*t)->type());
                    _out << ", " << ((*t)->optional() ? "true" : "false") << ", "
                         << ((*t)->optional() ? (*t)->tag() : 0) << ']';
                    ++count;
                }
            }
            _out << "], ";
            TypePtr returnType = (*s)->returnType();
            if(returnType)
            {
                //
                // The return type has the same format as an in/out parameter:
                //
                // Type, Optional?, OptionalTag
                //
                _out << '[';
                writeType(returnType);
                _out << ", " << ((*s)->returnIsOptional() ? "true" : "false") << ", "
                     << ((*s)->returnIsOptional() ? (*s)->returnTag() : 0) << ']';
            }
            else
            {
                _out << "nil";
            }
            _out << ", [";
            ExceptionList exceptions = (*s)->throws();
            for(ExceptionList::iterator u = exceptions.begin(); u != exceptions.end(); ++u)
            {
                if(u != exceptions.begin())
                {
                    _out << ", ";
                }
                _out << getAbsolute(*u, IdentToUpper, "T_");
            }
            _out << "])";

            string deprecateMetadata;
            if((*s)->findMetaData("deprecate", deprecateMetadata) || p->findMetaData("deprecate", deprecateMetadata))
            {
                string msg;
                string::size_type pos = deprecateMetadata.find(':');
                if(pos != string::npos && pos < deprecateMetadata.size() - 1)
                {
                    msg = deprecateMetadata.substr(pos + 1);
                }
                _out << nl << name << "Prx_mixin::OP_" << (*s)->name() << ".deprecate(\"" << msg << "\")";
            }
        }
    }

    _out.dec();
    _out << nl << "end"; // if not defined?()

    return false;
}

bool
Slice::Ruby::CodeVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string scoped = p->scoped();
    string name = fixIdent(p->name(), IdentToUpper);

    _out << sp << nl << "if not defined?(" << getAbsolute(p, IdentToUpper) << ')';
    _out.inc();
    _out << nl << "class " << name << " < ";
    ExceptionPtr base = p->base();
    string baseName;
    if(base)
    {
        baseName = getAbsolute(base, IdentToUpper);
        _out << baseName;
    }
    else if(p->isLocal())
    {
        _out << "Ice::LocalException";
    }
    else
    {
        _out << "Ice::UserException";
    }
    _out.inc();

    DataMemberList members = p->dataMembers();

    //
    // initialize
    //
    _out << nl << "def initialize";
    MemberInfoList allMembers;
    collectExceptionMembers(p, allMembers, false);
    bool inheritsMembers = false;
    if(!allMembers.empty())
    {
        _out << '(';
        writeConstructorParams(allMembers);
        _out << ')';
        for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
        {
            if(q->inherited)
            {
                inheritsMembers = true;
            }
        }
    }
    _out.inc();
    if(!allMembers.empty())
    {
        if(inheritsMembers)
        {
            _out << nl << "super" << spar;
            for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
            {
                if(q->inherited)
                {
                    _out << q->lowerName;
                }
            }
            _out << epar;
        }
        for(MemberInfoList::iterator q = allMembers.begin(); q != allMembers.end(); ++q)
        {
            if(!q->inherited)
            {
                _out << nl << '@' << q->fixedName << " = " << q->lowerName;
            }
        }
    }
    _out.dec();
    _out << nl << "end";

    //
    // to_s
    //
    _out << sp << nl << "def to_s";
    _out.inc();
    _out << nl << "'" << scoped << "'";
    _out.dec();
    _out << nl << "end";

    //
    // read/write accessors for data members.
    //
    if(!members.empty())
    {
        _out << sp << nl << "attr_accessor ";
        for(DataMemberList::iterator dmli = members.begin(); dmli != members.end(); ++dmli)
        {
            if(dmli != members.begin())
            {
                _out << ", ";
            }
            _out << ':' << fixIdent((*dmli)->name(), IdentNormal);
        }
    }

    _out.dec();
    _out << nl << "end"; // End of class.

    //
    // Emit the type information.
    //
    const bool preserved = p->hasMetaData("preserve-slice") || p->inheritsMetaData("preserve-slice");
    _out << sp << nl << "T_" << name << " = ::Ice::__defineException('" << scoped << "', " << name << ", "
         << (preserved ? "true" : "false") << ", ";
    if(!base)
    {
        _out << "nil";
    }
    else
    {
         _out << getAbsolute(base, IdentToUpper, "T_");
    }
    _out << ", [";
    if(members.size() > 1)
    {
        _out.inc();
        _out << nl;
    }
    //
    // Data members are represented as an array:
    //
    //   ['MemberName', MemberType, Optional, Tag]
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a constructed type.
    //
    for(DataMemberList::iterator dmli = members.begin(); dmli != members.end(); ++dmli)
    {
        if(dmli != members.begin())
        {
            _out << ',' << nl;
        }
        _out << "[\"" << fixIdent((*dmli)->name(), IdentNormal) << "\", ";
        writeType((*dmli)->type());
        _out << ", " << ((*dmli)->optional() ? "true" : "false") << ", " << ((*dmli)->optional() ? (*dmli)->tag() : 0)
             << ']';
    }
    if(members.size() > 1)
    {
        _out.dec();
        _out << nl;
    }
    _out << "])";

    _out.dec();
    _out << nl << "end"; // if not defined?()

    return false;
}

bool
Slice::Ruby::CodeVisitor::visitStructStart(const StructPtr& p)
{
    string scoped = p->scoped();
    string name = fixIdent(p->name(), IdentToUpper);
    MemberInfoList memberList;

    {
        DataMemberList members = p->dataMembers();
        for(DataMemberList::iterator q = members.begin(); q != members.end(); ++q)
        {
            memberList.push_back(MemberInfo());
            memberList.back().lowerName = fixIdent((*q)->name(), IdentToLower);
            memberList.back().fixedName = fixIdent((*q)->name(), IdentNormal);
            memberList.back().inherited = false;
            memberList.back().dataMember = *q;
        }
    }

    _out << sp << nl << "if not defined?(" << getAbsolute(p, IdentToUpper) << ')';
    _out.inc();
    _out << nl << "class " << name;
    _out.inc();
    _out << nl << "include ::Ice::Inspect_mixin";
    if(!memberList.empty())
    {
        _out << nl << "def initialize(";
        writeConstructorParams(memberList);
        _out << ")";
        _out.inc();
        for(MemberInfoList::iterator r = memberList.begin(); r != memberList.end(); ++r)
        {
            _out << nl << '@' << r->fixedName << " = " << r->lowerName;
        }
        _out.dec();
        _out << nl << "end";
    }

    //
    // hash
    //
    _out << sp << nl << "def hash";
    _out.inc();
    _out << nl << "_h = 0";
    int iter = 0;
    for(MemberInfoList::iterator r = memberList.begin(); r != memberList.end(); ++r)
    {
        writeHash("@" + r->fixedName, r->dataMember->type(), iter);
    }
    _out << nl << "_h % 0x7fffffff";
    _out.dec();
    _out << nl << "end";

    //
    // ==
    //
    _out << sp << nl << "def ==(other)";
    _out.inc();
    _out << nl << "return false if";
    _out.inc();
    _out << " !other.is_a? " << getAbsolute(p, IdentToUpper);
    for(MemberInfoList::iterator r = memberList.begin(); r != memberList.end(); ++r)
    {
        _out << " or" << nl << "@" << r->fixedName << " != other." << r->fixedName;
    }
    _out.dec();
    _out << nl << "true";
    _out.dec();
    _out << nl << "end";

    //
    // eql?
    //
    // This method is used to determine the equality of keys in a Hash object.
    //
    _out << sp << nl << "def eql?(other)";
    _out.inc();
    _out << nl << "return other.class == self.class && other == self";
    _out.dec();
    _out << nl << "end";

    //
    // read/write accessors for data members.
    //
    if(!memberList.empty())
    {
        _out << sp << nl << "attr_accessor ";
        for(MemberInfoList::iterator r = memberList.begin(); r != memberList.end(); ++r)
        {
            if(r != memberList.begin())
            {
                _out << ", ";
            }
            _out << ':' << r->fixedName;
        }
    }

    _out.dec();
    _out << nl << "end"; // End of class.

    //
    // Emit the type information.
    //
    _out << sp << nl << "T_" << name << " = ::Ice::__defineStruct('" << scoped << "', " << name << ", [";
    //
    // Data members are represented as an array:
    //
    //   ['MemberName', MemberType]
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a constructed type.
    //
    if(memberList.size() > 1)
    {
        _out.inc();
        _out << nl;
    }
    for(MemberInfoList::iterator r = memberList.begin(); r != memberList.end(); ++r)
    {
        if(r != memberList.begin())
        {
            _out << ',' << nl;
        }
        _out << "[\"" << r->fixedName << "\", ";
        writeType(r->dataMember->type());
        _out << ']';
    }
    if(memberList.size() > 1)
    {
        _out.dec();
        _out << nl;
    }
    _out << "])";

    _out.dec();
    _out << nl << "end"; // if not defined?()

    return false;
}

void
Slice::Ruby::CodeVisitor::visitSequence(const SequencePtr& p)
{
    //
    // Emit the type information.
    //
    string name = fixIdent(p->name(), IdentToUpper);
    string scoped = p->scoped();
    _out << sp << nl << "if not defined?(" << getAbsolute(p, IdentToUpper, "T_") << ')';
    _out.inc();
    _out << nl << "T_" << name << " = ::Ice::__defineSequence('" << scoped << "', ";
    writeType(p->type());
    _out << ")";
    _out.dec();
    _out << nl << "end"; // if not defined?()
}

void
Slice::Ruby::CodeVisitor::visitDictionary(const DictionaryPtr& p)
{
    //
    // Emit the type information.
    //
    string name = fixIdent(p->name(), IdentToUpper);
    string scoped = p->scoped();
    _out << sp << nl << "if not defined?(" << getAbsolute(p, IdentToUpper, "T_") << ')';
    _out.inc();
    _out << nl << "T_" << name << " = ::Ice::__defineDictionary('" << scoped << "', ";
    writeType(p->keyType());
    _out << ", ";
    writeType(p->valueType());
    _out << ")";
    _out.dec();
    _out << nl << "end"; // if not defined?()
}

void
Slice::Ruby::CodeVisitor::visitEnum(const EnumPtr& p)
{
    string scoped = p->scoped();
    string name = fixIdent(p->name(), IdentToUpper);
    EnumeratorList enums = p->enumerators();

    _out << sp << nl << "if not defined?(" << getAbsolute(p, IdentToUpper) << ')';
    _out.inc();
    _out << nl << "class " << name;
    _out.inc();
    _out << nl << "include Comparable";
    _out << sp << nl << "def initialize(name, value)";
    _out.inc();
    _out << nl << "@name = name";
    _out << nl << "@value = value";
    _out.dec();
    _out << nl << "end";

    //
    // from_int
    //
    {
        _out << sp << nl << "def " << name << ".from_int(val)";
        ostringstream sz;
        sz << enums.size() - 1;
        _out.inc();
        _out << nl << "@@_enumerators[val]"; // Evaluates to nil if the key is not found
        _out.dec();
        _out << nl << "end";
    }

    //
    // to_s
    //
    _out << sp << nl << "def to_s";
    _out.inc();
    _out << nl << "@name";
    _out.dec();
    _out << nl << "end";

    //
    // to_i
    //
    _out << sp << nl << "def to_i";
    _out.inc();
    _out << nl << "@value";
    _out.dec();
    _out << nl << "end";

    //
    // <=>
    //
    _out << sp << nl << "def <=>(other)";
    _out.inc();
    _out << nl << "other.is_a?(" << name << ") or raise ArgumentError, \"value must be a " << name << "\"";
    _out << nl << "@value <=> other.to_i";
    _out.dec();
    _out << nl << "end";

    //
    // hash
    //
    _out << sp << nl << "def hash";
    _out.inc();
    _out << nl << "@value.hash";
    _out.dec();
    _out << nl << "end";

    //
    // each
    //
    _out << sp << nl << "def " << name << ".each(&block)";
    _out.inc();
    _out << nl << "@@_enumerators.each_value(&block)";
    _out.dec();
    _out << nl << "end";

    //
    // Constant for each enumerator.
    //
    _out << sp;
    int i = 0;
    for(EnumeratorList::iterator q = enums.begin(); q != enums.end(); ++q, ++i)
    {
        ostringstream idx;
        idx << i;
        _out << nl << fixIdent((*q)->name(), IdentToUpper) << " = " << name << ".new(\"" << (*q)->name()
             << "\", " << (*q)->value() << ')';
    }

    _out << sp << nl << "@@_enumerators = {";
    for(EnumeratorList::iterator q = enums.begin(); q != enums.end(); ++q)
    {
        if(q != enums.begin())
        {
            _out << ", ";
        }
        _out << (*q)->value() << "=>" << fixIdent((*q)->name(), IdentToUpper);
    }
    _out << '}';

    _out << sp << nl << "def " << name << "._enumerators";
    _out.inc();
    _out << nl << "@@_enumerators";
    _out.dec();
    _out << nl << "end";

    _out << sp << nl << "private_class_method :new";

    _out.dec();
    _out << nl << "end"; // End of class.

    //
    // Emit the type information.
    //
    _out << sp << nl << "T_" << name << " = ::Ice::__defineEnum('" << scoped << "', " << name << ", " << name
         << "::_enumerators)";

    _out.dec();
    _out << nl << "end"; // if not defined?()
}

void
Slice::Ruby::CodeVisitor::visitConst(const ConstPtr& p)
{
    Slice::TypePtr type = p->type();
    string name = fixIdent(p->name(), IdentToUpper);

    _out << sp << nl << name << " = ";
    writeConstantValue(type, p->valueType(), p->value());
}

void
Slice::Ruby::CodeVisitor::writeType(const TypePtr& p)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindBool:
            {
                _out << "::Ice::T_bool";
                break;
            }
            case Builtin::KindByte:
            {
                _out << "::Ice::T_byte";
                break;
            }
            case Builtin::KindShort:
            {
                _out << "::Ice::T_short";
                break;
            }
            case Builtin::KindInt:
            {
                _out << "::Ice::T_int";
                break;
            }
            case Builtin::KindLong:
            {
                _out << "::Ice::T_long";
                break;
            }
            case Builtin::KindFloat:
            {
                _out << "::Ice::T_float";
                break;
            }
            case Builtin::KindDouble:
            {
                _out << "::Ice::T_double";
                break;
            }
            case Builtin::KindString:
            {
                _out << "::Ice::T_string";
                break;
            }
            case Builtin::KindValue:
            case Builtin::KindObject:
            {
                _out << "::Ice::T_Value";
                break;
            }
            case Builtin::KindObjectProxy:
            {
                _out << "::Ice::T_ObjectPrx";
                break;
            }
            case Builtin::KindLocalObject:
            {
                _out << "::Ice::T_LocalObject";
                break;
            }
        }
        return;
    }

    ProxyPtr prx = ProxyPtr::dynamicCast(p);
    if(prx)
    {
        ClassDefPtr def = prx->_class()->definition();
        if(def->isInterface() || def->allOperations().size() > 0)
        {
            _out << getAbsolute(prx->_class(), IdentToUpper, "T_") << "Prx";
        }
        else
        {
            _out << "::Ice::T_ObjectPrx";
        }
        return;
    }

    ContainedPtr cont = ContainedPtr::dynamicCast(p);
    assert(cont);
    _out << getAbsolute(cont, IdentToUpper, "T_");
}

string
Slice::Ruby::CodeVisitor::getInitializer(const DataMemberPtr& m)
{
    TypePtr p = m->type();
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindBool:
            {
                return "false";
            }
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindLong:
            {
                return "0";
            }
            case Builtin::KindFloat:
            case Builtin::KindDouble:
            {
                return "0.0";
            }
            case Builtin::KindString:
            {
                return "''";
            }
            case Builtin::KindValue:
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindLocalObject:
            {
                return "nil";
            }
        }
    }

    EnumPtr en = EnumPtr::dynamicCast(p);
    if(en)
    {
        EnumeratorList enums = en->enumerators();
        return getAbsolute(en, IdentToUpper) + "::" + fixIdent(enums.front()->name(), IdentToUpper);
    }

    StructPtr st = StructPtr::dynamicCast(p);
    if(st)
    {
        return getAbsolute(st, IdentToUpper) + ".new";
    }

    return "nil";
}

void
Slice::Ruby::CodeVisitor::writeHash(const string& name, const TypePtr&, int&)
{
    _out << nl << "_h = 5 * _h + " << name << ".hash";
}

void
Slice::Ruby::CodeVisitor::writeConstantValue(const TypePtr& type, const SyntaxTreeBasePtr& valueType,
                                             const string& value)
{
    ConstPtr constant = ConstPtr::dynamicCast(valueType);
    if(constant)
    {
        _out << fixIdent(constant->scoped(), IdentToUpper);
    }
    else
    {
        Slice::BuiltinPtr b = Slice::BuiltinPtr::dynamicCast(type);
        Slice::EnumPtr en = Slice::EnumPtr::dynamicCast(type);
        if(b)
        {
            switch(b->kind())
            {
            case Slice::Builtin::KindBool:
            case Slice::Builtin::KindByte:
            case Slice::Builtin::KindShort:
            case Slice::Builtin::KindInt:
            case Slice::Builtin::KindFloat:
            case Slice::Builtin::KindDouble:
            {
                _out << value;
                break;
            }
            case Slice::Builtin::KindLong:
            {
                IceUtil::Int64 l;
                IceUtilInternal::stringToInt64(value, l);
                _out << value;
                break;
            }
            case Slice::Builtin::KindString:
            {
                // RubyUCN available in Ruby 1.9 or greater
                _out << "\"" << toStringLiteral(value, "\a\b\f\n\r\t\v\x20\x1b", "", EC6UCN, 0) << "\"";
                break;
            }

            case Slice::Builtin::KindValue:
            case Slice::Builtin::KindObject:
            case Slice::Builtin::KindObjectProxy:
            case Slice::Builtin::KindLocalObject:
                assert(false);
            }
        }
        else if(en)
        {
            EnumeratorPtr lte = EnumeratorPtr::dynamicCast(valueType);
            assert(lte);
            _out << getAbsolute(lte, IdentToUpper);
        }
        else
        {
            assert(false); // Unknown const type.
        }
    }
}

void
Slice::Ruby::CodeVisitor::writeConstructorParams(const MemberInfoList& members)
{
    for(MemberInfoList::const_iterator p = members.begin(); p != members.end(); ++p)
    {
        if(p != members.begin())
        {
            _out << ", ";
        }
        _out << p->lowerName << "=";

        const DataMemberPtr member = p->dataMember;
        if(member->defaultValueType())
        {
            writeConstantValue(member->type(), member->defaultValueType(), member->defaultValue());
        }
        else if(member->optional())
        {
            _out << "::Ice::Unset";
        }
        else
        {
            _out << getInitializer(member);
        }
    }
}

void
Slice::Ruby::CodeVisitor::collectClassMembers(const ClassDefPtr& p, MemberInfoList& allMembers, bool inherited)
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
        m.lowerName = fixIdent((*q)->name(), IdentToLower);
        m.fixedName = fixIdent((*q)->name(), IdentNormal);
        m.inherited = inherited;
        m.dataMember = *q;
        allMembers.push_back(m);
    }
}

void
Slice::Ruby::CodeVisitor::collectExceptionMembers(const ExceptionPtr& p, MemberInfoList& allMembers, bool inherited)
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
        m.lowerName = fixIdent((*q)->name(), IdentToLower);
        m.fixedName = fixIdent((*q)->name(), IdentNormal);
        m.inherited = inherited;
        m.dataMember = *q;
        allMembers.push_back(m);
    }
}

void
Slice::Ruby::generate(const UnitPtr& un, bool all, bool checksum, const vector<string>& includePaths, Output& out)
{
    out << nl << "require 'Ice'";

    if(!all)
    {
        vector<string> paths = includePaths;
        for(vector<string>::iterator p = paths.begin(); p != paths.end(); ++p)
        {
            *p = fullPath(*p);
        }

        StringList includes = un->includeFiles();
        for(StringList::const_iterator q = includes.begin(); q != includes.end(); ++q)
        {
            string file = changeInclude(*q, paths);
            out << nl << "require '" << file << ".rb'";
        }
    }

    CodeVisitor codeVisitor(out);
    un->visit(&codeVisitor, false);

    if(checksum)
    {
        ChecksumMap checksums = createChecksums(un);
        if(!checksums.empty())
        {
            out << sp;
            for(ChecksumMap::const_iterator p = checksums.begin(); p != checksums.end(); ++p)
            {
                out << nl << "::Ice::SliceChecksums[\"" << p->first << "\"] = \"";
                ostringstream str;
                str.flags(ios_base::hex);
                str.fill('0');
                for(vector<unsigned char>::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
                {
                    str << static_cast<int>(*q);
                }
                out << str.str() << "\"";
            }
        }
    }

    out << nl; // Trailing newline.
}

string
Slice::Ruby::fixIdent(const string& ident, IdentStyle style)
{
    assert(!ident.empty());
    if(ident[0] != ':')
    {
        string id = ident;
        switch(style)
        {
        case IdentNormal:
            break;
        case IdentToUpper:
            // Special case BEGIN & END for class/module names.
            if(id == "BEGIN" || id == "END")
            {
                return id + "_";
            }
            if(id[0] >= 'a' && id[0] <= 'z')
            {
                id[0] += 'A' - 'a';
            }
            break;
        case IdentToLower:
            if(id[0] >= 'A' && id[0] <= 'Z')
            {
                id[0] += 'a' - 'A';
            }
            break;
        }
        return lookupKwd(id);
    }

    vector<string> ids = splitScopedName(ident);
    assert(!ids.empty());

    ostringstream result;

    for(vector<string>::size_type i = 0; i < ids.size() - 1; ++i)
    {
        //
        // We assume all intermediate names must be upper-case (i.e., they represent
        // the names of modules or classes).
        //
        result << "::" << fixIdent(ids[i], IdentToUpper);
    }

    result << "::" << fixIdent(ids[ids.size() - 1], style);

    //
    // Preserve trailing scope resolution operator if necessary.
    //
    if(ident.rfind("::") == ident.size() - 2)
    {
        result << "::";
    }

    return result.str();
}

string
Slice::Ruby::getAbsolute(const ContainedPtr& cont, IdentStyle style, const string& prefix)
{
    string scope = fixIdent(cont->scope(), IdentToUpper);

    if(prefix.empty())
    {
        return scope + fixIdent(cont->name(), style);
    }
    else
    {
        return scope + prefix + fixIdent(cont->name(), style);
    }
}

void
Slice::Ruby::printHeader(IceUtilInternal::Output& out)
{
    static const char* header =
"# **********************************************************************\n"
"#\n"
"# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.\n"
"#\n"
"# This copy of Ice is licensed to you under the terms described in the\n"
"# ICE_LICENSE file included in this distribution.\n"
"#\n"
"# **********************************************************************\n"
        ;

    out << header;
    out << "#\n";
    out << "# Ice version " << ICE_STRING_VERSION << "\n";
    out << "#\n";
}
