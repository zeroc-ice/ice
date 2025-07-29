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
    string getEscapedRubyParamName(const OperationPtr& p, const string& name)
    {
        for (const auto& param : p->parameters())
        {
            if (Slice::Ruby::getMappedName(param) == name)
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
        // Emit the array that describes a Slice type.
        //
        void writeType(const TypePtr&);

        //
        // Get an initializer value for a given type.
        //
        string getInitializer(const DataMemberPtr&);

        //
        // Write a constant value.
        //
        void writeConstantValue(const TypePtr&, const SyntaxTreeBasePtr&, const string&);

        /// Write constructor parameters with default values.
        void writeConstructorParams(const DataMemberList&);

        void outputElementSp();

        Output& _out;
        set<string> _classHistory;

        bool _firstElement{true};
    };
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
    _out << getMappedName(p, IdentToUpper);
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
    // Emit forward declarations.
    const string scoped = p->scoped();
    if (_classHistory.count(scoped) == 0)
    {
        outputElementSp();

        _out << nl << "if not defined?(" << getMetaTypeReference(p) << ')';
        _out.inc();
        _out << nl << getMetaTypeName(p) << " = Ice::__declareClass('" << scoped << "')";
        _out.dec();
        _out << nl << "end";
        _classHistory.insert(scoped); // Avoid redundant declarations.
    }
}

void
Slice::Ruby::CodeVisitor::visitInterfaceDecl(const InterfaceDeclPtr& p)
{
    // Emit forward declarations.
    const string scoped = p->scoped();
    if (_classHistory.count(scoped) == 0)
    {
        outputElementSp();

        _out << nl << "if not defined?(" << getMetaTypeReference(p) << "Prx)";
        _out.inc();
        _out << nl << getMetaTypeName(p) << "Prx = Ice::__declareProxy('" << scoped << "')";
        _out.dec();
        _out << nl << "end";
        _classHistory.insert(scoped); // Avoid redundant declarations.
    }
}

bool
Slice::Ruby::CodeVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    outputElementSp();
    _out << nl << "if not defined?(" << getAbsolute(p) << ')';
    _out.inc();

    const string name = getMappedName(p, IdentToUpper);
    const ClassDefPtr base = p->base();
    const DataMemberList members = p->dataMembers();
    const DataMemberList baseMembers = (base ? base->allDataMembers() : DataMemberList{});
    const DataMemberList allMembers = p->allDataMembers();

    _out << nl << "class " << name;
    if (base)
    {
        _out << " < " << getAbsolute(base);
    }
    else
    {
        _out << " < Ice::Value";
    }
    _out.inc();

    //
    // initialize
    //
    if (!allMembers.empty())
    {
        _out << sp << nl << "def initialize(";
        writeConstructorParams(allMembers);
        _out << ')';
        _out.inc();

        if (!baseMembers.empty())
        {
            _out << nl << "super" << spar;
            for (const auto& member : baseMembers)
            {
                _out << getMappedName(member, IdentToLower);
            }
            _out << epar;
        }

        for (const auto& member : members)
        {
            _out << nl << '@' << getMappedName(member) << " = " << getMappedName(member, IdentToLower);
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
            _out << ":" << getMappedName(*q);
        }
    }

    _out.dec();
    _out << nl << "end"; // End of class.

    _out << sp << nl << getMetaTypeName(p) << ".defineClass(" << name << ", " << p->compactId() << ", "
         << "false, ";
    if (!base)
    {
        _out << "nil";
    }
    else
    {
        _out << getMetaTypeReference(base);
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
    for (auto q = members.begin(); q != members.end(); ++q)
    {
        if (q != members.begin())
        {
            _out << ',' << nl;
        }
        _out << "['" << getMappedName(*q) << "', ";
        writeType((*q)->type());
        _out << ", " << ((*q)->optional() ? "true" : "false") << ", " << ((*q)->optional() ? (*q)->tag() : 0) << ']';
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
    _out << nl << "if not defined?(" << getAbsolute(p) << "Prx)";
    _out.inc();

    string proxyName = getMappedName(p, IdentToUpper) + "Prx";
    InterfaceList bases = p->bases();
    OperationList ops = p->operations();

    //
    // Generate proxy support. This includes a mix-in module for the proxy's
    // operations and a class for the proxy itself.
    //

    _out << nl << "module " << proxyName << "_mixin";
    _out.inc();
    for (const auto& base : bases)
    {
        InterfaceDefPtr def = base;
        _out << nl << "include " << getAbsolute(base) << "Prx_mixin";
    }
    for (const auto& op : ops)
    {
        string fixedOpName = getMappedName(op);
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
                inParams.append(getMappedName(q, IdentToLower));
            }
        }

        if (!isFirstElement(op))
        {
            _out << sp;
        }

        _out << nl << "def " << fixedOpName << "(";
        if (!inParams.empty())
        {
            _out << inParams << ", ";
        }
        const string contextParamName = getEscapedRubyParamName(op, "context");
        _out << contextParamName << "=nil)";
        _out.inc();
        _out << nl << proxyName << "_mixin::OP_" << op->name() << ".invoke(self, [" << inParams;
        _out << "], " << contextParamName << ")";
        _out.dec();
        _out << nl << "end";
    }
    _out.dec();
    _out << nl << "end"; // End of mix-in module for proxy.

    _out << sp << nl << "class " << proxyName << " < Ice::ObjectPrx";
    _out.inc();
    _out << nl << "include " << proxyName << "_mixin";
    _out.dec();
    _out << nl << "end"; // End of proxy class.

    _out << sp << nl << getMetaTypeName(p) << "Prx.defineProxy(" << proxyName << ", ";
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
            _out << getMetaTypeReference(base) << "Prx";
            ++interfaceCount;
        }
    }
    _out << "])";

    //
    // Define each operation. The arguments to __defineOperation are:
    //
    // 'sliceOpName', 'mappedOpName', Mode, FormatType, [InParams], [OutParams], ReturnParam, [Exceptions]
    //
    // where InParams and OutParams are arrays of type descriptions, and Exceptions
    // is an array of exception types.
    //
    if (!ops.empty())
    {
        _out << sp;
    }
    for (const auto& op : ops)
    {
        const string opName = op->name();
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

        _out << nl << proxyName << "_mixin::OP_" << opName << " = Ice::__defineOperation('" << opName << "', '"
             << op->mappedName() << "', ";
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
        bool isFirst = true;
        for (const auto& param : op->inParameters())
        {
            if (!isFirst)
            {
                _out << ", ";
            }
            isFirst = false;
            _out << '[';
            writeType(param->type());
            _out << ", " << (param->optional() ? "true" : "false") << ", " << (param->optional() ? param->tag() : 0)
                 << ']';
        }
        _out << "], [";
        isFirst = true;
        for (const auto& param : op->outParameters())
        {
            if (!isFirst)
            {
                _out << ", ";
            }
            isFirst = false;
            _out << '[';
            writeType(param->type());
            _out << ", " << (param->optional() ? "true" : "false") << ", " << (param->optional() ? param->tag() : 0)
                 << ']';
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
        _out << ", ";
        _out.spar("[");
        for (const auto& ex : op->throws())
        {
            _out << getMetaTypeReference(ex);
        }
        _out.epar("]");
        _out << ")";

        if (op->isDeprecated())
        {
            // Get the deprecation reason if present, or default to an empty string.
            string reason = op->getDeprecationReason().value_or("");
            _out << nl << proxyName << "_mixin::OP_" << opName << ".deprecate(\"" << reason << "\")";
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
    string name = getMappedName(p, IdentToUpper);

    outputElementSp();
    _out << nl << "if not defined?(" << getAbsolute(p) << ')';
    _out.inc();
    _out << nl << "class " << name << " < ";
    ExceptionPtr base = p->base();
    if (base)
    {
        _out << getAbsolute(base);
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
        _out.spar("");
        for (const auto& member : members)
        {
            _out << ":" + getMappedName(member);
        }
        _out.epar("");
    }

    _out.dec();
    _out << nl << "end"; // End of exception class.

    //
    // Emit the type information.
    //
    _out << sp << nl << getMetaTypeName(p) << " = Ice::__defineException('" << scoped << "', " << name << ", ";
    if (!base)
    {
        _out << "nil";
    }
    else
    {
        _out << getMetaTypeReference(base);
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
        _out << "[\"" << getMappedName(*dmli) << "\", ";
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
    const string scoped = p->scoped();
    const string name = getMappedName(p, IdentToUpper);
    const DataMemberList members = p->dataMembers();

    outputElementSp();
    _out << nl << "if not defined?(" << getAbsolute(p) << ')';
    _out.inc();
    _out << nl << "class " << name;
    _out.inc();
    _out << nl << "include Ice::Inspect_mixin";
    if (!members.empty())
    {
        _out << nl << "def initialize(";
        writeConstructorParams(members);
        _out << ")";
        _out.inc();
        for (const auto& member : members)
        {
            _out << nl << '@' << getMappedName(member) << " = " << getMappedName(member, IdentToLower);
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
    for (const auto& member : members)
    {
        const string memberName = "@" + getMappedName(member);
        _out << nl << "_h = 5 * _h + " << memberName << ".hash";
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
    _out << " !other.is_a? " << getAbsolute(p);
    for (const auto& member : members)
    {
        const string memberName = getMappedName(member);
        _out << " or" << nl << "@" << memberName << " != other." << memberName;
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
    if (!members.empty())
    {
        _out << sp << nl << "attr_accessor ";
        _out.spar("");
        for (const auto& member : members)
        {
            _out << (":" + getMappedName(member));
        }
        _out.epar("");
    }

    _out.dec();
    _out << nl << "end"; // End of class.

    //
    // Emit the type information.
    //
    _out << sp << nl << getMetaTypeName(p) << " = Ice::__defineStruct('" << scoped << "', " << name << ", [";
    //
    // Data members are represented as an array:
    //
    //   ['MemberName', MemberType]
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a user-defined type.
    //
    if (members.size() > 1)
    {
        _out.inc();
        _out << nl;
    }
    for (auto r = members.begin(); r != members.end(); ++r)
    {
        if (r != members.begin())
        {
            _out << ',' << nl;
        }
        _out << "[\"" << getMappedName(*r) << "\", ";
        writeType((*r)->type());
        _out << ']';
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

void
Slice::Ruby::CodeVisitor::visitSequence(const SequencePtr& p)
{
    // Emit the type information.
    outputElementSp();
    _out << nl << "if not defined?(" << getMetaTypeReference(p) << ')';
    _out.inc();
    _out << nl << getMetaTypeName(p) << " = Ice::__defineSequence('" << p->scoped() << "', ";
    writeType(p->type());
    _out << ")";
    _out.dec();
    _out << nl << "end"; // if not defined?()
}

void
Slice::Ruby::CodeVisitor::visitDictionary(const DictionaryPtr& p)
{
    // Emit the type information.
    outputElementSp();
    _out << nl << "if not defined?(" << getMetaTypeReference(p) << ')';
    _out.inc();
    _out << nl << getMetaTypeName(p) << " = Ice::__defineDictionary('" << p->scoped() << "', ";
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
    string name = getMappedName(p, IdentToUpper);

    outputElementSp();
    _out << nl << "if not defined?(" << getAbsolute(p) << ')';
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
    _out << sp << nl << "def " << name << ".from_int(val)";
    _out.inc();
    _out << nl << "@@_enumerators[val]"; // Evaluates to nil if the key is not found
    _out.dec();
    _out << nl << "end";

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
    _out << nl << "other.is_a?(" << name << ") or raise ArgumentError, \"value must be " << getArticleFor(name) << ' '
         << name << "\"";
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
    EnumeratorList enumerators = p->enumerators();
    for (const auto& enumerator : enumerators)
    {
        _out << nl << getMappedName(enumerator, IdentToUpper) << " = " << name << ".new(\"" << enumerator->name()
             << "\", " << enumerator->value() << ')';
    }
    _out << sp << nl << "@@_enumerators = ";
    _out.spar("{");
    for (const auto& enumerator : enumerators)
    {
        _out << std::to_string(enumerator->value()) + "=>" + getMappedName(enumerator, IdentToUpper);
    }
    _out.epar("}");

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
    _out << sp << nl << getMetaTypeName(p) << " = Ice::__defineEnum('" << p->scoped() << "', " << name << ", " << name
         << "::_enumerators)";

    _out.dec();
    _out << nl << "end"; // if not defined?()
}

void
Slice::Ruby::CodeVisitor::visitConst(const ConstPtr& p)
{
    outputElementSp();
    _out << nl << getMappedName(p, IdentToUpper) << " = ";
    writeConstantValue(p->type(), p->valueType(), p->value());
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
        _out << getMetaTypeReference(prx) << "Prx";
        return;
    }

    ContainedPtr cont = dynamic_pointer_cast<Contained>(p);
    assert(cont);
    _out << getMetaTypeReference(cont);
}

string
Slice::Ruby::CodeVisitor::getInitializer(const DataMemberPtr& m)
{
    TypePtr p = m->type();

    if (BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(p))
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
            case Builtin::KindObjectProxy:
            {
                return "nil";
            }
        }
    }

    if (EnumPtr en = dynamic_pointer_cast<Enum>(p))
    {
        const EnumeratorPtr firstEnumerator = en->enumerators().front();
        return getAbsolute(firstEnumerator);
    }

    if (StructPtr st = dynamic_pointer_cast<Struct>(p))
    {
        return getAbsolute(st) + ".new";
    }

    return "nil";
}

void
Slice::Ruby::CodeVisitor::writeConstantValue(
    const TypePtr& type,
    const SyntaxTreeBasePtr& valueType,
    const string& value)
{
    if (ConstPtr constant = dynamic_pointer_cast<Const>(valueType))
    {
        _out << getAbsolute(constant);
    }
    else if (BuiltinPtr b = dynamic_pointer_cast<Builtin>(type))
    {
        switch (b->kind())
        {
            case Builtin::KindBool:
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindFloat:
            case Builtin::KindDouble:
            case Builtin::KindLong:
            {
                _out << value;
                break;
            }
            case Builtin::KindString:
            {
                // RubyUCN available in Ruby 1.9 or greater
                _out << "\"" << toStringLiteral(value, "\a\b\f\n\r\t\v\x20\x1b", "", EC6UCN, 0) << "\"";
                break;
            }

            case Builtin::KindValue:
            case Builtin::KindObjectProxy:
                assert(false);
        }
    }
    else if (EnumPtr en = dynamic_pointer_cast<Enum>(type))
    {
        EnumeratorPtr lte = dynamic_pointer_cast<Enumerator>(valueType);
        assert(lte);
        _out << getAbsolute(lte);
    }
    else
    {
        assert(false); // Unknown const type.
    }
}

void
Slice::Ruby::CodeVisitor::writeConstructorParams(const DataMemberList& members)
{
    bool isFirst = true;
    for (const auto& member : members)
    {
        if (!isFirst)
        {
            _out << ", ";
        }
        isFirst = false;

        _out << getMappedName(member, IdentToLower) << "=";
        if (member->defaultValue())
        {
            writeConstantValue(member->type(), member->defaultValueType(), *member->defaultValue());
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
Slice::Ruby::generate(const UnitPtr& unit, bool all, const vector<string>& includePaths, Output& out)
{
    out << nl << "require 'Ice'";

    validateRubyMetadata(unit);

    if (!all)
    {
        vector<string> paths = includePaths;
        for (auto& path : paths)
        {
            path = fullPath(path);
        }

        for (string file : unit->includeFiles())
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
    unit->visit(&codeVisitor);

    out << nl; // Trailing newline.
}

string
Slice::Ruby::getMappedName(const ContainedPtr& p, IdentStyle style)
{
    // If the user provided a specific identifier through metadata, we use it without any changes.
    if (auto customName = p->getMetadataArgs("ruby:identifier"))
    {
        return *customName;
    }

    // Otherwise, we get the element's name and apply any necessary case-correction to it.
    string ident = p->name();
    switch (style)
    {
        case Slice::Ruby::IdentNormal:
            break;
        case Slice::Ruby::IdentToUpper:
            ident[0] = static_cast<char>(toupper(ident[0]));
            break;
        case Slice::Ruby::IdentToLower:
            ident[0] = static_cast<char>(tolower(ident[0]));
            break;
    }
    return ident;
}

string
Slice::Ruby::getAbsolute(const ContainedPtr& p)
{
    // We construct the absolutely scoped name from the inside-out. Starting with the element,
    // and working up through its parents, we continually insert "::name" to the front of a string.
    string result;
    ContainedPtr currentElement = p;
    while (currentElement)
    {
        // Insert the element's name at the beginning of the 'result' string (with a leading "::").
        result.insert(0, "::" + getMappedName(currentElement, IdentToUpper));
        // Attempt to navigate to the parent element. If it exists, the while loop will continue.
        currentElement = dynamic_pointer_cast<Contained>(currentElement->container());
    }
    return result;
}

string
Slice::Ruby::getMetaTypeName(const ContainedPtr& p)
{
    return "T_" + getMappedName(p, IdentToUpper);
}

string
Slice::Ruby::getMetaTypeReference(const ContainedPtr& p)
{
    string absolute = getAbsolute(p);

    // Append a "T_" in front of the last name segment.
    auto pos = absolute.rfind(':');
    pos = (pos == string::npos ? 0 : pos + 1);
    absolute.insert(pos, "T_");

    return absolute;
}

void
Slice::Ruby::printHeader(IceInternal::Output& out)
{
    out << "# Copyright (c) ZeroC, Inc.";
    out << sp;
    out << nl << "# slice2rb version " << ICE_STRING_VERSION;
}

void
Slice::Ruby::validateRubyMetadata(const UnitPtr& unit)
{
    map<string, MetadataInfo> knownMetadata;

    // "ruby:identifier"
    MetadataInfo identifierInfo = {
        .validOn =
            {typeid(Module),
             typeid(InterfaceDecl),
             typeid(Operation),
             typeid(ClassDecl),
             typeid(Slice::Exception),
             typeid(Struct),
             typeid(Sequence),
             typeid(Dictionary),
             typeid(Enum),
             typeid(Enumerator),
             typeid(Const),
             typeid(Parameter),
             typeid(DataMember)},
        .acceptedArgumentKind = MetadataArgumentKind::SingleArgument,
    };
    knownMetadata.emplace("ruby:identifier", std::move(identifierInfo));

    // Pass this information off to the parser's metadata validation logic.
    Slice::validateMetadata(unit, "ruby", std::move(knownMetadata));
}
