// Copyright (c) ZeroC, Inc.

#include "RubyUtil.h"
#include "../Ice/FileUtil.h"
#include "../Slice/MetadataValidation.h"
#include "../Slice/Util.h"

#include <algorithm>
#include <cassert>
#include <iterator>

using namespace std;
using namespace Slice;
using namespace IceInternal;

namespace
{
    string getEscapedParamName(const OperationPtr& p, const string& name)
    {
        ParameterList params = p->parameters();

        for (const auto& param : params)
        {
            if (param->name() == name)
            {
                return name + "_";
            }
        }
        return name;
    }
}

namespace Slice::Ruby
{
    //
    // CodeVisitor generates the Ruby mapping for a translation unit.
    //
    class CodeVisitor final : public ParserVisitor
    {
    public:
        CodeVisitor(IceInternal::Output&);

        bool visitModuleStart(const ModulePtr&) final;
        void visitModuleEnd(const ModulePtr&) final;
        void visitClassDecl(const ClassDeclPtr&) final;
        bool visitClassDefStart(const ClassDefPtr&) final;
        void visitInterfaceDecl(const InterfaceDeclPtr&) final;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
        bool visitExceptionStart(const ExceptionPtr&) final;
        bool visitStructStart(const StructPtr&) final;
        void visitSequence(const SequencePtr&) final;
        void visitDictionary(const DictionaryPtr&) final;
        void visitEnum(const EnumPtr&) final;
        void visitConst(const ConstPtr&) final;

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
            string lowerName; // Mapped name beginning with a lower-case letter for use as the name of a local
                              // variable.
            string fixedName;
            bool inherited = false;
            DataMemberPtr dataMember;
        };
        using MemberInfoList = list<MemberInfo>;

        //
        // Write constructor parameters with default values.
        //
        void writeConstructorParams(const MemberInfoList&);

        void collectClassMembers(const ClassDefPtr&, MemberInfoList&, bool);
        void collectExceptionMembers(const ExceptionPtr&, MemberInfoList&, bool);

        void outputElementSp();

        Output& _out;
        set<string> _classHistory;

        bool _firstElement{true};
    };
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
    static const string keywordList[] = {
        "BEGIN",
        "END",
        "alias",
        "and",
        "begin",
        "break",
        "case",
        "class",
        "clone",
        "def",
        "display",
        "do",
        "dup",
        "else",
        "elsif",
        "end",
        "ensure",
        "extend",
        "false",
        "for",
        "freeze",
        "hash",
        "if",
        "in",
        "initialize_copy",
        "inspect",
        "instance_eval",
        "instance_variable_get",
        "instance_variable_set",
        "instance_variables",
        "method",
        "method_missing",
        "methods",
        "module",
        "new",
        "next",
        "nil",
        "not",
        "object_id",
        "or",
        "private_methods",
        "protected_methods",
        "public_methods",
        "redo",
        "rescue",
        "retry",
        "return",
        "self",
        "send",
        "singleton_methods",
        "super",
        "taint",
        "then",
        "to_a",
        "to_s",
        "true",
        "undef",
        "unless",
        "untaint",
        "until",
        "when",
        "while",
        "yield"};
    bool found = binary_search(&keywordList[0], &keywordList[sizeof(keywordList) / sizeof(*keywordList)], name);
    return found ? "_" + name : name;
}

//
// CodeVisitor implementation.
//
Slice::Ruby::CodeVisitor::CodeVisitor(Output& out) : _out(out) {}

bool
Slice::Ruby::CodeVisitor::visitModuleStart(const ModulePtr& p)
{
    _out << sp << nl << "module ";
    // Ensure that Slice top-level modules are defined as top level modules in Ruby
    if (p->isTopLevel())
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
    if (_classHistory.count(scoped) == 0)
    {
        outputElementSp();

        string name = "T_" + fixIdent(p->name(), IdentToUpper);
        _out << nl << "if not defined?(" << getAbsolute(p, IdentToUpper, "T_") << ')';
        _out.inc();
        _out << nl << name << " = Ice::__declareClass('" << scoped << "')";
        _out.dec();
        _out << nl << "end";
        _classHistory.insert(scoped); // Avoid redundant declarations.
    }
}

void
Slice::Ruby::CodeVisitor::visitInterfaceDecl(const InterfaceDeclPtr& p)
{
    //
    // Emit forward declarations.
    //
    string scoped = p->scoped();
    if (_classHistory.count(scoped) == 0)
    {
        outputElementSp();

        string name = "T_" + fixIdent(p->name(), IdentToUpper);
        _out << nl << "if not defined?(" << getAbsolute(p, IdentToUpper, "T_") << "Prx)";
        _out.inc();
        _out << nl << name << "Prx = Ice::__declareProxy('" << scoped << "')";
        _out.dec();
        _out << nl << "end";
        _classHistory.insert(scoped); // Avoid redundant declarations.
    }
}

bool
Slice::Ruby::CodeVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    outputElementSp();
    _out << nl << "if not defined?(" << getAbsolute(p, IdentToUpper) << ')';
    _out.inc();

    string scoped = p->scoped();
    string name = fixIdent(p->name(), IdentToUpper);
    ClassDefPtr base = p->base();
    DataMemberList members = p->dataMembers();

    _out << nl << "class " << name;
    if (base)
    {
        _out << " < " << getAbsolute(base, IdentToUpper);
    }
    else
    {
        _out << " < Ice::Value";
    }
    _out.inc();

    //
    // initialize
    //
    MemberInfoList allMembers;
    collectClassMembers(p, allMembers, false);
    if (!allMembers.empty())
    {
        _out << sp << nl << "def initialize(";
        writeConstructorParams(allMembers);
        _out << ')';
        _out.inc();

        bool inheritsMembers = false;
        for (const auto& allMember : allMembers)
        {
            if (allMember.inherited)
            {
                inheritsMembers = true;
                break;
            }
        }

        if (inheritsMembers)
        {
            _out << nl << "super" << spar;
            for (const auto& allMember : allMembers)
            {
                if (allMember.inherited)
                {
                    _out << allMember.lowerName;
                }
            }
            _out << epar;
        }

        for (const auto& allMember : allMembers)
        {
            if (!allMember.inherited)
            {
                _out << nl << '@' << allMember.fixedName << " = " << allMember.lowerName;
            }
        }

        _out.dec();
        _out << nl << "end";
    }

    //
    // read/write accessors for data members.
    //
    if (!members.empty())
    {
        _out << sp << nl << "attr_accessor ";
        for (auto q = members.begin(); q != members.end(); ++q)
        {
            if (q != members.begin())
            {
                _out << ", ";
            }
            _out << ":" << fixIdent((*q)->name(), IdentNormal);
        }
    }

    _out.dec();
    _out << nl << "end"; // End of class.

    _out << sp << nl << "T_" << name << ".defineClass(" << name << ", " << p->compactId() << ", "
         << "false, ";
    if (!base)
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
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a user-defined type.
    //
    _out << "[";
    if (members.size() > 1)
    {
        _out.inc();
        _out << nl;
    }
    {
        for (auto q = members.begin(); q != members.end(); ++q)
        {
            if (q != members.begin())
            {
                _out << ',' << nl;
            }
            _out << "['" << fixIdent((*q)->name(), IdentNormal) << "', ";
            writeType((*q)->type());
            _out << ", " << ((*q)->optional() ? "true" : "false") << ", " << ((*q)->optional() ? (*q)->tag() : 0)
                 << ']';
        }
    }
    if (members.size() > 1)
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
Slice::Ruby::CodeVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    outputElementSp();
    _out << nl << "if not defined?(" << getAbsolute(p, IdentToUpper) << "Prx)";
    _out.inc();

    string scoped = p->scoped();
    string name = fixIdent(p->name(), IdentToUpper);
    InterfaceList bases = p->bases();
    OperationList ops = p->operations();

    //
    // Generate proxy support. This includes a mix-in module for the proxy's
    // operations and a class for the proxy itself.
    //

    _out << nl << "module " << name << "Prx_mixin";
    _out.inc();
    for (const auto& base : bases)
    {
        InterfaceDefPtr def = base;
        _out << nl << "include " << getAbsolute(base, IdentToUpper) << "Prx_mixin";
    }
    for (const auto& op : ops)
    {
        string fixedOpName = fixIdent(op->name(), IdentNormal);
        if (fixedOpName == "checkedCast" || fixedOpName == "uncheckedCast")
        {
            fixedOpName.insert(0, "_");
        }
        TypePtr ret = op->returnType();
        ParameterList paramList = op->parameters();
        string inParams;

        for (const auto& q : paramList)
        {
            if (!q->isOutParam())
            {
                if (!inParams.empty())
                {
                    inParams.append(", ");
                }
                inParams.append(fixIdent(q->name(), IdentToLower));
            }
        }

        if (!isFirstOperation(op))
        {
            _out << sp;
        }

        _out << nl << "def " << fixedOpName << "(";
        if (!inParams.empty())
        {
            _out << inParams << ", ";
        }
        const string contextParamName = getEscapedParamName(op, "context");
        _out << contextParamName << "=nil)";
        _out.inc();
        _out << nl << name << "Prx_mixin::OP_" << op->name() << ".invoke(self, [" << inParams;
        _out << "], " << contextParamName << ")";
        _out.dec();
        _out << nl << "end";
    }
    _out.dec();
    _out << nl << "end"; // End of mix-in module for proxy.

    _out << sp << nl << "class " << name << "Prx < Ice::ObjectPrx";
    _out.inc();
    _out << nl << "include Ice::Proxy_mixin";
    _out << nl << "include " << name << "Prx_mixin";
    _out.dec();
    _out << nl << "end"; // End of proxy class.

    //
    // Define each operation. The arguments to __defineOperation are:
    //
    // 'opName', Mode, FormatType, [InParams], [OutParams], ReturnParam, [Exceptions]
    //
    // where InParams and OutParams are arrays of type descriptions, and Exceptions
    // is an array of exception types.
    //
    _out << sp << nl << "T_" << name << "Prx.defineProxy(" << name << "Prx, ";
    _out << "nil";

    //
    // Interfaces
    //
    _out << ", [";
    {
        int interfaceCount = 0;
        for (const auto& base : bases)
        {
            if (interfaceCount > 0)
            {
                _out << ", ";
            }
            _out << getAbsolute(base, IdentToUpper, "T_") << "Prx";
            ++interfaceCount;
        }
    }
    _out << "])";

    if (!ops.empty())
    {
        _out << sp;
    }
    for (const auto& op : ops)
    {
        ParameterList params = op->parameters();
        ParameterList::const_iterator t;
        int count;
        string format;
        optional<FormatType> opFormat = op->format();
        if (opFormat)
        {
            switch (*opFormat)
            {
                case CompactFormat:
                    format = "Ice::FormatType::CompactFormat";
                    break;
                case SlicedFormat:
                    format = "Ice::FormatType::SlicedFormat";
                    break;
                default:
                    assert(false);
            }
        }
        else
        {
            format = "nil";
        }

        _out << nl << name << "Prx_mixin::OP_" << op->name() << " = Ice::__defineOperation('" << op->name() << "', ";
        switch (op->mode())
        {
            case Operation::Normal:
                _out << "Ice::OperationMode::Normal";
                break;
            case Operation::Idempotent:
                _out << "Ice::OperationMode::Idempotent";
                break;
        }
        _out << ", " << format << ", [";
        for (t = params.begin(), count = 0; t != params.end(); ++t)
        {
            if (!(*t)->isOutParam())
            {
                if (count > 0)
                {
                    _out << ", ";
                }
                _out << '[';
                writeType((*t)->type());
                _out << ", " << ((*t)->optional() ? "true" : "false") << ", " << ((*t)->optional() ? (*t)->tag() : 0)
                     << ']';
                ++count;
            }
        }
        _out << "], [";
        for (t = params.begin(), count = 0; t != params.end(); ++t)
        {
            if ((*t)->isOutParam())
            {
                if (count > 0)
                {
                    _out << ", ";
                }
                _out << '[';
                writeType((*t)->type());
                _out << ", " << ((*t)->optional() ? "true" : "false") << ", " << ((*t)->optional() ? (*t)->tag() : 0)
                     << ']';
                ++count;
            }
        }
        _out << "], ";
        TypePtr returnType = op->returnType();
        if (returnType)
        {
            //
            // The return type has the same format as an in/out parameter:
            //
            // Type, Optional?, OptionalTag
            //
            _out << '[';
            writeType(returnType);
            _out << ", " << (op->returnIsOptional() ? "true" : "false") << ", "
                 << (op->returnIsOptional() ? op->returnTag() : 0) << ']';
        }
        else
        {
            _out << "nil";
        }
        _out << ", [";
        ExceptionList exceptions = op->throws();
        for (auto u = exceptions.begin(); u != exceptions.end(); ++u)
        {
            if (u != exceptions.begin())
            {
                _out << ", ";
            }
            _out << getAbsolute(*u, IdentToUpper, "T_");
        }
        _out << "])";

        if (op->isDeprecated())
        {
            // Get the deprecation reason if present, or default to an empty string.
            string reason = op->getDeprecationReason().value_or("");
            _out << nl << name << "Prx_mixin::OP_" << op->name() << ".deprecate(\"" << reason << "\")";
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

    outputElementSp();
    _out << nl << "if not defined?(" << getAbsolute(p, IdentToUpper) << ')';
    _out.inc();
    _out << nl << "class " << name << " < ";
    ExceptionPtr base = p->base();
    string baseName;
    if (base)
    {
        baseName = getAbsolute(base, IdentToUpper);
        _out << baseName;
    }
    else
    {
        _out << "Ice::UserException";
    }
    _out.inc();

    //
    // to_s
    //
    _out << nl << "def to_s";
    _out.inc();
    _out << nl << "'" << scoped << "'";
    _out.dec();
    _out << nl << "end";

    //
    // read/write accessors for data members.
    //
    DataMemberList members = p->dataMembers();
    if (!members.empty())
    {
        _out << sp << nl << "attr_accessor ";
        for (auto dmli = members.begin(); dmli != members.end(); ++dmli)
        {
            if (dmli != members.begin())
            {
                _out << ", ";
            }
            _out << ':' << fixIdent((*dmli)->name(), IdentNormal);
        }
    }

    _out.dec();
    _out << nl << "end"; // End of exception class.

    //
    // Emit the type information.
    //
    _out << sp << nl << "T_" << name << " = Ice::__defineException('" << scoped << "', " << name << ", ";
    if (!base)
    {
        _out << "nil";
    }
    else
    {
        _out << getAbsolute(base, IdentToUpper, "T_");
    }
    _out << ", [";
    if (members.size() > 1)
    {
        _out.inc();
        _out << nl;
    }
    //
    // Data members are represented as an array:
    //
    //   ['MemberName', MemberType, Optional, Tag]
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a user-defined type.
    //
    for (auto dmli = members.begin(); dmli != members.end(); ++dmli)
    {
        if (dmli != members.begin())
        {
            _out << ',' << nl;
        }
        _out << "[\"" << fixIdent((*dmli)->name(), IdentNormal) << "\", ";
        writeType((*dmli)->type());
        _out << ", " << ((*dmli)->optional() ? "true" : "false") << ", " << ((*dmli)->optional() ? (*dmli)->tag() : 0)
             << ']';
    }
    if (members.size() > 1)
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
        for (const auto& member : members)
        {
            memberList.emplace_back();
            memberList.back().lowerName = fixIdent(member->name(), IdentToLower);
            memberList.back().fixedName = fixIdent(member->name(), IdentNormal);
            memberList.back().inherited = false;
            memberList.back().dataMember = member;
        }
    }

    outputElementSp();
    _out << nl << "if not defined?(" << getAbsolute(p, IdentToUpper) << ')';
    _out.inc();
    _out << nl << "class " << name;
    _out.inc();
    _out << nl << "include Ice::Inspect_mixin";
    if (!memberList.empty())
    {
        _out << nl << "def initialize(";
        writeConstructorParams(memberList);
        _out << ")";
        _out.inc();
        for (const auto& r : memberList)
        {
            _out << nl << '@' << r.fixedName << " = " << r.lowerName;
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
    for (const auto& r : memberList)
    {
        writeHash("@" + r.fixedName, r.dataMember->type(), iter);
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
    for (const auto& r : memberList)
    {
        _out << " or" << nl << "@" << r.fixedName << " != other." << r.fixedName;
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
    if (!memberList.empty())
    {
        _out << sp << nl << "attr_accessor ";
        for (auto r = memberList.begin(); r != memberList.end(); ++r)
        {
            if (r != memberList.begin())
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
    _out << sp << nl << "T_" << name << " = Ice::__defineStruct('" << scoped << "', " << name << ", [";
    //
    // Data members are represented as an array:
    //
    //   ['MemberName', MemberType]
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a user-defined type.
    //
    if (memberList.size() > 1)
    {
        _out.inc();
        _out << nl;
    }
    for (auto r = memberList.begin(); r != memberList.end(); ++r)
    {
        if (r != memberList.begin())
        {
            _out << ',' << nl;
        }
        _out << "[\"" << r->fixedName << "\", ";
        writeType(r->dataMember->type());
        _out << ']';
    }
    if (memberList.size() > 1)
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
    outputElementSp();
    _out << nl << "if not defined?(" << getAbsolute(p, IdentToUpper, "T_") << ')';
    _out.inc();
    _out << nl << "T_" << name << " = Ice::__defineSequence('" << scoped << "', ";
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
    outputElementSp();
    _out << nl << "if not defined?(" << getAbsolute(p, IdentToUpper, "T_") << ')';
    _out.inc();
    _out << nl << "T_" << name << " = Ice::__defineDictionary('" << scoped << "', ";
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
    EnumeratorList enumerators = p->enumerators();

    outputElementSp();
    _out << nl << "if not defined?(" << getAbsolute(p, IdentToUpper) << ')';
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
        sz << enumerators.size() - 1;
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
    for (auto q = enumerators.begin(); q != enumerators.end(); ++q, ++i)
    {
        ostringstream idx;
        idx << i;
        _out << nl << fixIdent((*q)->name(), IdentToUpper) << " = " << name << ".new(\"" << (*q)->name() << "\", "
             << (*q)->value() << ')';
    }

    _out << sp << nl << "@@_enumerators = {";
    for (auto q = enumerators.begin(); q != enumerators.end(); ++q)
    {
        if (q != enumerators.begin())
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
    _out << sp << nl << "T_" << name << " = Ice::__defineEnum('" << scoped << "', " << name << ", " << name
         << "::_enumerators)";

    _out.dec();
    _out << nl << "end"; // if not defined?()
}

void
Slice::Ruby::CodeVisitor::visitConst(const ConstPtr& p)
{
    Slice::TypePtr type = p->type();
    string name = fixIdent(p->name(), IdentToUpper);

    outputElementSp();
    _out << nl << name << " = ";
    writeConstantValue(type, p->valueType(), p->value());
}

void
Slice::Ruby::CodeVisitor::writeType(const TypePtr& p)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(p);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindBool:
            {
                _out << "Ice::T_bool";
                break;
            }
            case Builtin::KindByte:
            {
                _out << "Ice::T_byte";
                break;
            }
            case Builtin::KindShort:
            {
                _out << "Ice::T_short";
                break;
            }
            case Builtin::KindInt:
            {
                _out << "Ice::T_int";
                break;
            }
            case Builtin::KindLong:
            {
                _out << "Ice::T_long";
                break;
            }
            case Builtin::KindFloat:
            {
                _out << "Ice::T_float";
                break;
            }
            case Builtin::KindDouble:
            {
                _out << "Ice::T_double";
                break;
            }
            case Builtin::KindString:
            {
                _out << "Ice::T_string";
                break;
            }
            case Builtin::KindValue:
            case Builtin::KindObject:
            {
                _out << "Ice::T_Value";
                break;
            }
            case Builtin::KindObjectProxy:
            {
                _out << "Ice::T_ObjectPrx";
                break;
            }
        }
        return;
    }

    InterfaceDeclPtr prx = dynamic_pointer_cast<InterfaceDecl>(p);
    if (prx)
    {
        _out << getAbsolute(prx, IdentToUpper, "T_") << "Prx";
        return;
    }

    ContainedPtr cont = dynamic_pointer_cast<Contained>(p);
    assert(cont);
    _out << getAbsolute(cont, IdentToUpper, "T_");
}

string
Slice::Ruby::CodeVisitor::getInitializer(const DataMemberPtr& m)
{
    TypePtr p = m->type();
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(p);
    if (builtin)
    {
        switch (builtin->kind())
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
            {
                return "nil";
            }
        }
    }

    EnumPtr en = dynamic_pointer_cast<Enum>(p);
    if (en)
    {
        string firstEnumerator = en->enumerators().front()->name();
        return getAbsolute(en, IdentToUpper) + "::" + fixIdent(firstEnumerator, IdentToUpper);
    }

    StructPtr st = dynamic_pointer_cast<Struct>(p);
    if (st)
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
Slice::Ruby::CodeVisitor::writeConstantValue(
    const TypePtr& type,
    const SyntaxTreeBasePtr& valueType,
    const string& value)
{
    ConstPtr constant = dynamic_pointer_cast<Const>(valueType);
    if (constant)
    {
        _out << fixIdent(constant->scoped(), IdentToUpper);
    }
    else
    {
        Slice::BuiltinPtr b = dynamic_pointer_cast<Slice::Builtin>(type);
        Slice::EnumPtr en = dynamic_pointer_cast<Slice::Enum>(type);
        if (b)
        {
            switch (b->kind())
            {
                case Slice::Builtin::KindBool:
                case Slice::Builtin::KindByte:
                case Slice::Builtin::KindShort:
                case Slice::Builtin::KindInt:
                case Slice::Builtin::KindFloat:
                case Slice::Builtin::KindDouble:
                case Slice::Builtin::KindLong:
                {
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
                    assert(false);
            }
        }
        else if (en)
        {
            EnumeratorPtr lte = dynamic_pointer_cast<Enumerator>(valueType);
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
    for (auto p = members.begin(); p != members.end(); ++p)
    {
        if (p != members.begin())
        {
            _out << ", ";
        }
        _out << p->lowerName << "=";

        const DataMemberPtr member = p->dataMember;
        if (member->defaultValueType())
        {
            writeConstantValue(member->type(), member->defaultValueType(), member->defaultValue());
        }
        else if (member->optional())
        {
            _out << "Ice::Unset";
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
    ClassDefPtr base = p->base();
    if (base)
    {
        collectClassMembers(base, allMembers, true);
    }

    DataMemberList members = p->dataMembers();

    for (const auto& member : members)
    {
        MemberInfo m;
        m.lowerName = fixIdent(member->name(), IdentToLower);
        m.fixedName = fixIdent(member->name(), IdentNormal);
        m.inherited = inherited;
        m.dataMember = member;
        allMembers.push_back(m);
    }
}

void
Slice::Ruby::CodeVisitor::collectExceptionMembers(const ExceptionPtr& p, MemberInfoList& allMembers, bool inherited)
{
    ExceptionPtr base = p->base();
    if (base)
    {
        collectExceptionMembers(base, allMembers, true);
    }

    DataMemberList members = p->dataMembers();

    for (const auto& member : members)
    {
        MemberInfo m;
        m.lowerName = fixIdent(member->name(), IdentToLower);
        m.fixedName = fixIdent(member->name(), IdentNormal);
        m.inherited = inherited;
        m.dataMember = member;
        allMembers.push_back(m);
    }
}

void
Slice::Ruby::CodeVisitor::outputElementSp()
{
    if (_firstElement)
    {
        _firstElement = false;
    }
    else
    {
        _out << sp;
    }
}

void
Slice::Ruby::generate(const UnitPtr& un, bool all, const vector<string>& includePaths, Output& out)
{
    out << nl << "require 'Ice'";

    // 'slice2rb' doesn't have any language-specific metadata, so we call `validateMetadata` with an empty list.
    // This ensures that the validation still runs, and will reject any 'rb' metadata the user might think exists.
    Slice::validateMetadata(un, "ruby", {});

    if (!all)
    {
        vector<string> paths = includePaths;
        for (auto& path : paths)
        {
            path = fullPath(path);
        }

        for (string file : un->includeFiles())
        {
            if (isAbsolutePath(file))
            {
                file = changeInclude(file, paths);
                out << nl << "require '" << file << ".rb'";
            }
            else
            {
                file = removeExtension(file);
                out << nl << "require_relative '" << file << ".rb'";
            }
        }
    }

    CodeVisitor codeVisitor(out);
    un->visit(&codeVisitor);

    out << nl; // Trailing newline.
}

string
Slice::Ruby::fixIdent(const string& ident, IdentStyle style)
{
    assert(!ident.empty());
    if (ident[0] != ':')
    {
        string id = ident;
        switch (style)
        {
            case IdentNormal:
                break;
            case IdentToUpper:
                // Special case BEGIN & END for class/module names.
                if (id == "BEGIN" || id == "END")
                {
                    return id + "_";
                }
                if (id[0] >= 'a' && id[0] <= 'z')
                {
                    id[0] += 'A' - 'a';
                }
                break;
            case IdentToLower:
                if (id[0] >= 'A' && id[0] <= 'Z')
                {
                    id[0] += 'a' - 'A';
                }
                break;
        }
        return lookupKwd(id);
    }

    vector<string> ids = splitScopedName(ident, false);
    assert(!ids.empty());

    ostringstream result;

    for (vector<string>::size_type i = 0; i < ids.size() - 1; ++i)
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
    if (ident.rfind("::") == ident.size() - 2)
    {
        result << "::";
    }

    return result.str();
}

string
Slice::Ruby::getAbsolute(const ContainedPtr& cont, IdentStyle style, const string& prefix)
{
    string scope = fixIdent(cont->scope(), IdentToUpper);

    if (prefix.empty())
    {
        return scope + fixIdent(cont->name(), style);
    }
    else
    {
        return scope + prefix + fixIdent(cont->name(), style);
    }
}

void
Slice::Ruby::printHeader(IceInternal::Output& out)
{
    out << "# Copyright (c) ZeroC, Inc.\n";
    out << "#\n";
    out << "# Ice version " << ICE_STRING_VERSION << "\n";
    out << "#\n";
}
