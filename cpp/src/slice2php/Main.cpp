// Copyright (c) ZeroC, Inc.

#include "../Ice/ConsoleUtil.h"
#include "../Ice/Options.h"
#include "../Ice/OutputUtil.h"
#include "../Slice/FileTracker.h"
#include "../Slice/MetadataValidation.h"
#include "../Slice/Parser.h"
#include "../Slice/Preprocessor.h"
#include "../Slice/Util.h"
#include "Ice/CtrlCHandler.h"
#include "Ice/StringUtil.h"

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstring>
#include <mutex>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#    include <direct.h>
#else
#    include <unistd.h>
#endif

// TODO: fix this warning!
#if defined(_MSC_VER)
#    pragma warning(disable : 4456) // shadow
#    pragma warning(disable : 4457) // shadow
#    pragma warning(disable : 4459) // shadow
#elif defined(__clang__)
#    pragma clang diagnostic ignored "-Wshadow"
#elif defined(__GNUC__)
#    pragma GCC diagnostic ignored "-Wshadow"
#endif

using namespace std;
using namespace Slice;
using namespace IceInternal;

namespace
{
    void validateMetadata(const UnitPtr& unit)
    {
        map<string, MetadataInfo> knownMetadata;

        // "php:identifier"
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
        knownMetadata.emplace("php:identifier", std::move(identifierInfo));

        // Pass this information off to the parser's metadata validation logic.
        Slice::validateMetadata(unit, "php", std::move(knownMetadata));
    }
}

// CodeVisitor generates the PHP mapping for a translation unit.
class CodeVisitor final : public ParserVisitor
{
public:
    CodeVisitor(IceInternal::Output&);

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
    void startNamespace(const ContainedPtr&);
    void endNamespace();

    // Return the PHP variable for the given object's type.
    string getTypeVar(const ContainedPtr&);

    string getType(const TypePtr&);

    // Write a default value for a given type.
    void writeDefaultValue(const DataMemberPtr&);

    // Write a member assignment statement for a constructor.
    void writeAssign(const DataMemberPtr& member);

    // Write constant value.
    void writeConstantValue(const TypePtr&, const SyntaxTreeBasePtr&, const string&);

    /// Write constructor parameters with default values.
    void writeConstructorParams(const DataMemberList& members);

    Output& _out;
    set<string> _classHistory;
};

// CodeVisitor implementation.
CodeVisitor::CodeVisitor(Output& out) : _out(out) {}

void
CodeVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    // Handle forward declarations.
    const string scoped = p->scoped();
    if (_classHistory.count(scoped) == 0)
    {
        startNamespace(p);

        const string type = getTypeVar(p);
        _out << sp << nl << "global " << type << ';';
        _out << nl << type << " = IcePHP_declareClass('" << scoped << "');";

        endNamespace();

        _classHistory.insert(scoped); // Avoid redundant declarations.
    }
}

void
CodeVisitor::visitInterfaceDecl(const InterfaceDeclPtr& p)
{
    // Handle forward declarations.
    const string scoped = p->scoped();
    if (_classHistory.count(scoped) == 0)
    {
        startNamespace(p);

        const string type = getTypeVar(p);
        _out << sp << nl << "global " << type << ';';
        _out << nl << "global " << type << "Prx;";
        _out << nl << type << "Prx = IcePHP_declareProxy('" << scoped << "');";

        endNamespace();

        _classHistory.insert(scoped); // Avoid redundant declarations.
    }
}

bool
CodeVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    const string scoped = p->scoped();
    const string name = p->mappedName();
    const string type = getTypeVar(p);
    const ClassDefPtr base = p->base();
    const DataMemberList members = p->dataMembers();

    startNamespace(p);

    _out << sp << nl << "global " << type << ';';

    _out << nl;
    _out << "class " << name;
    if (base)
    {
        _out << " extends " << base->mappedScoped("\\");
    }
    else
    {
        _out << " extends \\Ice\\Value";
    }

    _out << sb;

    // __construct
    _out << nl << "public function __construct(";
    writeConstructorParams(p->allDataMembers());
    _out << ")";
    _out << sb;
    if (base)
    {
        _out << nl << "parent::__construct(";
        int count = 0;
        for (const auto& member : base->allDataMembers())
        {
            if (count)
            {
                _out << ", ";
            }
            _out << '$' << member->mappedName();
            ++count;
        }
        _out << ");";
    }
    {
        for (const auto& member : members)
        {
            writeAssign(member);
        }
    }
    _out << eb;

    // ice_ice
    _out << sp << nl << "public function ice_id()";
    _out << sb;
    _out << nl << "return '" << scoped << "';";
    _out << eb;

    // ice_staticId
    _out << sp << nl << "public static function ice_staticId()";
    _out << sb;
    _out << nl << "return '" << scoped << "';";
    _out << eb;

    // __toString
    _out << sp << nl << "public function __toString(): string";

    _out << sb;
    _out << nl << "global " << type << ';';
    _out << nl << "return IcePHP_stringify($this, " << type << ");";
    _out << eb;

    if (!members.empty())
    {
        _out << sp;
        for (const auto& member : members)
        {
            _out << nl << "public " << "$" << member->mappedName() << ";";
        }
    }

    _out << eb; // End of class.

    {
        string type;
        vector<string> seenType;
        _out << sp << nl << "global ";
        if (!base)
        {
            type = "$Ice__t_Value";
        }
        else
        {
            type = getTypeVar(base);
        }
        _out << type << ";";
        seenType.push_back(type);

        for (const auto& member : members)
        {
            string type = getType(member->type());
            if (find(seenType.begin(), seenType.end(), type) == seenType.end())
            {
                seenType.push_back(type);
                _out << nl << "global " << type << ";";
            }
        }
    }

    // Emit the type information.
    const string abs = p->mappedScoped(R"(\\)");
    _out << nl << type << " = IcePHP_defineClass('" << scoped << "', '" << abs << "', " << p->compactId() << ", ";
    if (!base)
    {
        _out << "$Ice__t_Value";
    }
    else
    {
        _out << getTypeVar(base);
    }
    _out << ", ";
    // Members
    //
    // Data members are represented as an array:
    //
    //   ('MemberName', MemberType, Optional, Tag)
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a user-defined type.
    if (!members.empty())
    {
        _out << "array(";
        for (auto q = members.begin(); q != members.end(); ++q)
        {
            if (q != members.begin())
            {
                _out << ',';
            }
            _out.inc();
            _out << nl << "array('" << (*q)->mappedName() << "', ";
            _out << getType((*q)->type());
            _out << ", " << ((*q)->optional() ? "true" : "false") << ", " << ((*q)->optional() ? (*q)->tag() : 0)
                 << ')';
            _out.dec();
        }
        _out << ')';
    }
    else
    {
        _out << "null";
    }
    _out << ");";

    endNamespace();

    return false;
}

bool
CodeVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    const string scoped = p->scoped();
    const string type = getTypeVar(p);
    const string prxName = p->mappedName() + "Prx";
    const string prxType = type + "Prx";
    const OperationList ops = p->operations();

    startNamespace(p);

    _out << sp << nl << "global " << type << ';';
    _out << nl << "global " << prxType << ';';

    // Define the proxy class.
    _out << sp << nl << "class " << prxName << "Helper";
    _out << sb;

    _out << sp << nl << "public static function createProxy($communicator, $proxyString)";
    _out << sb;
    _out << nl << "return  $communicator->stringToProxy($proxyString, '" << scoped << "');";
    _out << eb;

    _out << sp << nl << "public static function checkedCast($proxy, ...$args)";
    _out << sb;
    _out << nl << "return $proxy->ice_checkedCast('" << scoped << "', ...$args);";
    _out << eb;

    _out << sp << nl << "public static function uncheckedCast($proxy, $facet=null)";
    _out << sb;
    _out << nl << "return $proxy->ice_uncheckedCast('" << scoped << "', $facet);";
    _out << eb;

    _out << sp << nl << "public static function ice_staticId()";
    _out << sb;
    _out << nl << "return '" << scoped << "';";
    _out << eb;

    _out << eb;

    _out << sp;
    _out << nl << "global $Ice__t_ObjectPrx;";
    _out << nl << prxType << " = IcePHP_defineProxy('" << scoped << "', " << "$Ice__t_ObjectPrx" << ", ";

    // Interfaces
    const InterfaceList bases = p->bases();
    if (!bases.empty())
    {
        _out << "array" << spar;
        for (const auto& base : bases)
        {
            _out << getTypeVar(base) + "Prx";
        }
        _out << epar;
    }
    else
    {
        _out << "null";
    }
    _out << ");";

    // Define each operation. The arguments to IcePHP_defineOperation are:
    //
    // $ClassType, 'sliceOpName', 'mappedOpName', Mode, FormatType, (InParams), (OutParams), ReturnParam, (Exceptions)
    //
    // where InParams and OutParams are arrays of type descriptions, and Exceptions is an array of exception type ids.

    if (!ops.empty())
    {
        _out << sp;
        vector<string> seenTypes;
        for (const auto& op : ops)
        {
            for (const auto& param : op->parameters())
            {
                const string type = getType(param->type());
                if (find(seenTypes.begin(), seenTypes.end(), type) == seenTypes.end())
                {
                    seenTypes.push_back(type);
                    _out << nl << "global " << type << ";";
                }
            }

            if (op->returnType())
            {
                const string type = getType(op->returnType());
                if (find(seenTypes.begin(), seenTypes.end(), type) == seenTypes.end())
                {
                    seenTypes.push_back(type);
                    _out << nl << "global " << type << ";";
                }
            }
        }

        for (const auto& op : ops)
        {
            ParameterList params = op->parameters();
            ParameterList::iterator t;
            int count;

            // We encode nullopt as -1.
            optional<FormatType> opFormat = op->format();
            int phpFormat = -1;
            if (opFormat)
            {
                phpFormat = static_cast<int>(*opFormat);
            }

            _out << nl << "IcePHP_defineOperation(" << prxType << ", '" << op->name() << "', '" << op->mappedName()
                 << "', " << static_cast<int>(op->mode()) << ", " << phpFormat << ", ";
            for (t = params.begin(), count = 0; t != params.end(); ++t)
            {
                if (!(*t)->isOutParam())
                {
                    if (count == 0)
                    {
                        _out << "array(";
                    }
                    else if (count > 0)
                    {
                        _out << ", ";
                    }
                    _out << "array(";
                    _out << getType((*t)->type());
                    if ((*t)->optional())
                    {
                        _out << ", " << (*t)->tag();
                    }
                    _out << ')';
                    ++count;
                }
            }
            if (count > 0)
            {
                _out << ')';
            }
            else
            {
                _out << "null";
            }
            _out << ", ";
            for (t = params.begin(), count = 0; t != params.end(); ++t)
            {
                if ((*t)->isOutParam())
                {
                    if (count == 0)
                    {
                        _out << "array(";
                    }
                    else if (count > 0)
                    {
                        _out << ", ";
                    }
                    _out << "array(";
                    _out << getType((*t)->type());
                    if ((*t)->optional())
                    {
                        _out << ", " << (*t)->tag();
                    }
                    _out << ')';
                    ++count;
                }
            }
            if (count > 0)
            {
                _out << ')';
            }
            else
            {
                _out << "null";
            }
            _out << ", ";
            TypePtr returnType = op->returnType();
            if (returnType)
            {
                // The return type has the same format as an in/out parameter:
                //
                // Type, Optional?, OptionalTag
                _out << "array(";
                _out << getType(returnType);
                if (op->returnIsOptional())
                {
                    _out << ", " << op->returnTag();
                }
                _out << ')';
            }
            else
            {
                _out << "null";
            }
            _out << ", ";

            ExceptionList exceptions = op->throws();
            if (!exceptions.empty())
            {
                _out << "array(";
                for (auto u = exceptions.begin(); u != exceptions.end(); ++u)
                {
                    if (u != exceptions.begin())
                    {
                        _out << ", ";
                    }
                    _out << getTypeVar(*u);
                }
                _out << ')';
            }
            else
            {
                _out << "null";
            }
            _out << ");";
        }
    }

    endNamespace();

    return false;
}

bool
CodeVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    const string scoped = p->scoped();
    const string name = p->mappedName();
    const string type = getTypeVar(p);
    const ExceptionPtr base = p->base();

    startNamespace(p);

    _out << sp << nl << "global " << type << ';';
    _out << nl << "class " << name << " extends ";
    if (base)
    {
        _out << base->mappedScoped("\\");
    }
    else
    {
        _out << "\\Ice\\UserException";
    }
    _out << sb;

    // ice_id
    _out << sp << nl << "public function ice_id()";
    _out << sb;
    _out << nl << "return '" << scoped << "';";
    _out << eb;

    // __toString
    _out << sp << nl << "public function __toString(): string";

    _out << sb;
    _out << nl << "global " << type << ';';
    _out << nl << "return IcePHP_stringifyException($this, " << type << ");";
    _out << eb;

    DataMemberList members = p->dataMembers();

    if (!members.empty())
    {
        _out << sp;
        for (const auto& member : members)
        {
            _out << nl << "public $" << member->mappedName() << ";";
        }
    }

    _out << eb;

    vector<string> seenType;
    for (const auto& member : members)
    {
        string type = getType(member->type());
        if (find(seenType.begin(), seenType.end(), type) == seenType.end())
        {
            seenType.push_back(type);
            _out << nl << "global " << type << ";";
        }
    }

    // Emit the type information.
    const string abs = p->mappedScoped("\\\\");
    _out << sp << nl << type << " = IcePHP_defineException('" << scoped << "', '" << abs << "', ";
    if (!base)
    {
        _out << "null";
    }
    else
    {
        _out << getTypeVar(base);
    }
    _out << ", ";
    // Data members are represented as an array:
    //
    //   ('MemberName', MemberType, Optional, Tag)
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a user-defined type.
    if (!members.empty())
    {
        _out << "array(";
        for (auto dmli = members.begin(); dmli != members.end(); ++dmli)
        {
            if (dmli != members.begin())
            {
                _out << ',';
            }
            _out.inc();
            _out << nl << "array('" << (*dmli)->mappedName() << "', ";
            _out << getType((*dmli)->type());
            _out << ", " << ((*dmli)->optional() ? "true" : "false") << ", "
                 << ((*dmli)->optional() ? (*dmli)->tag() : 0) << ')';
            _out.dec();
        }
        _out << ')';
    }
    else
    {
        _out << "null";
    }
    _out << ");";

    endNamespace();

    return false;
}

bool
CodeVisitor::visitStructStart(const StructPtr& p)
{
    const string scoped = p->scoped();
    const string name = p->mappedName();
    const string type = getTypeVar(p);
    const DataMemberList members = p->dataMembers();

    startNamespace(p);

    _out << sp << nl << "global " << type << ';';

    _out << nl << "class " << name;
    _out << sb;
    _out << nl << "public function __construct(";
    writeConstructorParams(members);
    _out << ")";
    _out << sb;
    for (const auto& member : members)
    {
        writeAssign(member);
    }
    _out << eb;

    // __toString
    _out << sp << nl << "public function __toString(): string";

    _out << sb;
    _out << nl << "global " << type << ';';
    _out << nl << "return IcePHP_stringify($this, " << type << ");";
    _out << eb;

    if (!members.empty())
    {
        _out << sp;
        for (const auto& member : members)
        {
            _out << nl << "public $" << member->mappedName() << ';';
        }
    }

    _out << eb;

    _out << sp;
    vector<string> seenType;
    for (const auto& member : members)
    {
        string type = getType(member->type());
        if (find(seenType.begin(), seenType.end(), type) == seenType.end())
        {
            seenType.push_back(type);
            _out << nl << "global " << type << ";";
        }
    }

    // Emit the type information.
    const string abs = p->mappedScoped("\\\\");
    _out << nl << type << " = IcePHP_defineStruct('" << scoped << "', '" << abs << "', array(";

    // Data members are represented as an array:
    //
    //   ('MemberName', MemberType)
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a user-defined type.
    for (auto r = members.begin(); r != members.end(); ++r)
    {
        if (r != members.begin())
        {
            _out << ",";
        }
        _out.inc();
        _out << nl << "array('" << (*r)->mappedName() << "', " << getType((*r)->type()) << ')';
        _out.dec();
    }
    _out << "));";

    endNamespace();

    return false;
}

void
CodeVisitor::visitSequence(const SequencePtr& p)
{
    const string type = getTypeVar(p);
    const TypePtr content = p->type();

    startNamespace(p);

    // Emit the type information.
    _out << sp << nl << "global " << type << ';';
    _out << sp << nl << "if(!isset(" << type << "))";
    _out << sb;
    _out << nl << "global " << getType(content) << ";";
    _out << nl << type << " = IcePHP_defineSequence('" << p->scoped() << "', ";
    _out << getType(content);
    _out << ");";
    _out << eb;

    endNamespace();
}

void
CodeVisitor::visitDictionary(const DictionaryPtr& p)
{
    const string type = getTypeVar(p);
    const TypePtr keyType = p->keyType();
    const TypePtr valueType = p->valueType();

    if (!dynamic_pointer_cast<Builtin>(keyType) && !dynamic_pointer_cast<Enum>(keyType))
    {
        // See https://github.com/zeroc-ice/ice/issues/254
        p->unit()->warning(p->file(), p->line(), All, "dictionary key type not supported in PHP");
    }

    startNamespace(p);

    // Emit the type information.
    _out << sp << nl << "global " << type << ';';
    _out << sp << nl << "if(!isset(" << type << "))";
    _out << sb;
    _out << nl << "global " << getType(keyType) << ";";
    _out << nl << "global " << getType(valueType) << ";";
    _out << nl << type << " = IcePHP_defineDictionary('" << p->scoped() << "', ";
    _out << getType(keyType);
    _out << ", ";
    _out << getType(valueType);
    _out << ");";
    _out << eb;

    endNamespace();
}

void
CodeVisitor::visitEnum(const EnumPtr& p)
{
    const string type = getTypeVar(p);
    const EnumeratorList enumerators = p->enumerators();

    startNamespace(p);

    _out << sp << nl << "global " << type << ';';
    _out << nl << "class " << p->mappedName();
    _out << sb;

    for (const auto& enumerator : enumerators)
    {
        _out << nl << "const " << enumerator->mappedName() << " = " << enumerator->value() << ';';
    }

    _out << eb;

    // Emit the type information.
    _out << sp << nl << type << " = IcePHP_defineEnum('" << p->scoped() << "', array" << spar;
    for (auto q = enumerators.begin(); q != enumerators.end(); ++q)
    {
        _out << "'" + (*q)->mappedName() + "'";
        _out << (*q)->value();
    }
    _out << epar << ");";

    endNamespace();
}

void
CodeVisitor::visitConst(const ConstPtr& p)
{
    startNamespace(p);

    _out << sp << nl << "if(!defined('" << p->mappedScoped("\\\\") << "'))";
    _out << sb;
    _out << sp << nl << "define(__NAMESPACE__ . '\\\\" << p->mappedName() << "', ";
    writeConstantValue(p->type(), p->valueType(), p->value());

    _out << ");";
    _out << eb;

    endNamespace();
}

void
CodeVisitor::startNamespace(const ContainedPtr& p)
{
    // This function should only be called on module level elements.
    ModulePtr container = dynamic_pointer_cast<Module>(p->container());
    assert(container);
    _out << sp << nl << "namespace " << container->mappedScoped("\\").substr(1);
    _out << sb;
}

void
CodeVisitor::endNamespace()
{
    _out << eb;
}

string
CodeVisitor::getTypeVar(const ContainedPtr& p)
{
    return "$" + p->mappedScope("_").substr(1) + "_t_" + p->mappedName();
}

string
CodeVisitor::getType(const TypePtr& p)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(p);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindBool:
            {
                return "$IcePHP__t_bool";
            }
            case Builtin::KindByte:
            {
                return "$IcePHP__t_byte";
            }
            case Builtin::KindShort:
            {
                return "$IcePHP__t_short";
            }
            case Builtin::KindInt:
            {
                return "$IcePHP__t_int";
            }
            case Builtin::KindLong:
            {
                return "$IcePHP__t_long";
            }
            case Builtin::KindFloat:
            {
                return "$IcePHP__t_float";
            }
            case Builtin::KindDouble:
            {
                return "$IcePHP__t_double";
            }
            case Builtin::KindString:
            {
                return "$IcePHP__t_string";
            }
            case Builtin::KindObject:
            case Builtin::KindValue:
            {
                return "$Ice__t_Value";
            }
            case Builtin::KindObjectProxy:
            {
                return "$Ice__t_ObjectPrx";
            }
        }
    }

    InterfaceDeclPtr prx = dynamic_pointer_cast<InterfaceDecl>(p);
    if (prx)
    {
        return getTypeVar(prx) + "Prx";
    }

    ContainedPtr cont = dynamic_pointer_cast<Contained>(p);
    assert(cont);
    return getTypeVar(cont);
}

void
CodeVisitor::writeDefaultValue(const DataMemberPtr& m)
{
    TypePtr p = m->type();
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(p);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindBool:
            {
                _out << "false";
                break;
            }
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindLong:
            {
                _out << "0";
                break;
            }
            case Builtin::KindFloat:
            case Builtin::KindDouble:
            {
                _out << "0.0";
                break;
            }
            case Builtin::KindString:
            {
                _out << "''";
                break;
            }
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindValue:
            {
                _out << "null";
                break;
            }
        }
        return;
    }

    EnumPtr en = dynamic_pointer_cast<Enum>(p);
    if (en)
    {
        string firstEnumerator = en->enumerators().front()->mappedName();
        _out << en->mappedScoped("\\") << "::" << firstEnumerator;
        return;
    }

    // PHP does not allow the following construct:
    //
    // function foo($theStruct=new MyStructType)
    //
    // Instead we use null as the default value and allocate an instance in the constructor.
    if (dynamic_pointer_cast<Struct>(p))
    {
        _out << "null";
        return;
    }

    _out << "null";
}

void
CodeVisitor::writeAssign(const DataMemberPtr& member)
{
    const string memberName = member->mappedName();
    if (StructPtr st = dynamic_pointer_cast<Struct>(member->type()))
    {
        _out << nl << "$this->" << memberName << " = is_null($" << memberName << ") ? new " << st->mappedScoped("\\")
             << " : $" << memberName << ';';
    }
    else
    {
        _out << nl << "$this->" << memberName << " = $" << memberName << ';';
    }
}

void
CodeVisitor::writeConstantValue(const TypePtr& type, const SyntaxTreeBasePtr& valueType, const string& value)
{
    ConstPtr constant = dynamic_pointer_cast<Const>(valueType);
    if (constant)
    {
        _out << constant->mappedScoped("\\");
    }
    else
    {
        Slice::BuiltinPtr b = dynamic_pointer_cast<Builtin>(type);
        Slice::EnumPtr en = dynamic_pointer_cast<Enum>(type);
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
                {
                    _out << value;
                    break;
                }
                case Slice::Builtin::KindLong:
                {
                    int64_t l = std::stoll(value, nullptr, 0); // NOLINT(clang-analyzer-deadcode.DeadStores)
                    // The platform's 'long' type may not be 64 bits, so we store 64-bit values as a string.
                    if (sizeof(int64_t) > sizeof(long) && (l < INT32_MIN || l > INT32_MAX))
                    {
                        _out << "'" << value << "'";
                    }
                    else
                    {
                        _out << value;
                    }
                    break;
                }
                case Slice::Builtin::KindString:
                {
                    // PHP 7.x also supports an EC6UCN-like notation, see: https://wiki.php.net/rfc/unicode_escape
                    _out << "\"" << toStringLiteral(value, "\f\n\r\t\v\x1b", "$", Octal, 0) << "\"";
                    break;
                }
                case Slice::Builtin::KindObject:
                case Slice::Builtin::KindObjectProxy:
                case Slice::Builtin::KindValue:
                    assert(false);
            }
        }
        else if (en)
        {
            EnumeratorPtr lte = dynamic_pointer_cast<Enumerator>(valueType);
            assert(lte);
            _out << en->mappedScoped("\\") << "::" << lte->mappedName();
        }
        else
        {
            assert(false); // Unknown const type.
        }
    }
}

void
CodeVisitor::writeConstructorParams(const DataMemberList& members)
{
    bool isFirst = true;
    for (const auto& member : members)
    {
        if (!isFirst)
        {
            _out << ", ";
        }
        isFirst = false;

        _out << '$' << member->mappedName() << "=";
        if (member->defaultValue())
        {
            writeConstantValue(member->type(), member->defaultValueType(), *member->defaultValue());
        }
        else if (member->optional())
        {
            _out << "\\Ice\\None";
        }
        else
        {
            writeDefaultValue(member);
        }
    }
}

static void
generate(const UnitPtr& un, bool all, const vector<string>& includePaths, Output& out)
{
    if (!all)
    {
        vector<string> paths = includePaths;
        for (auto& path : paths)
        {
            path = fullPath(path);
        }

        StringList includes = un->includeFiles();
        if (!includes.empty())
        {
            out << sp;
            out << nl << "namespace";
            out << sb;
            for (const auto& include : includes)
            {
                string file = changeInclude(include, paths);
                out << nl << "require_once '" << file << ".php';";
            }
            out << eb;
        }
    }

    validateMetadata(un);

    CodeVisitor codeVisitor(out);
    un->visit(&codeVisitor);

    out << nl; // Trailing newline.
}

static void
printHeader(IceInternal::Output& out)
{
    out << "// Copyright (c) ZeroC, Inc.";
    out << sp;
    out << nl << "// slice2php version " << ICE_STRING_VERSION;
}

namespace
{
    mutex globalMutex;
    bool interrupted = false;
}

static void
interruptedCallback(int /*signal*/)
{
    lock_guard lock(globalMutex);
    interrupted = true;
}

static void
usage(const string& n)
{
    consoleErr << "Usage: " << n << " [options] slice-files...\n";
    consoleErr << "Options:\n"
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
                  "--all                    Generate code for Slice definitions in included files.\n";
}

int
compile(const vector<string>& argv)
{
    IceInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("", "validate");
    opts.addOpt("D", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("U", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("I", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("E");
    opts.addOpt("", "output-dir", IceInternal::Options::NeedArg);
    opts.addOpt("", "depend");
    opts.addOpt("", "depend-xml");
    opts.addOpt("", "depend-file", IceInternal::Options::NeedArg, "");
    opts.addOpt("d", "debug");
    opts.addOpt("", "all");

    bool validate = find(argv.begin(), argv.end(), "--validate") != argv.end();

    vector<string> args;
    try
    {
        args = opts.parse(argv);
    }
    catch (const IceInternal::BadOptException& e)
    {
        consoleErr << argv[0] << ": error: " << e.what() << endl;
        if (!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if (opts.isSet("help"))
    {
        usage(argv[0]);
        return EXIT_SUCCESS;
    }

    if (opts.isSet("version"))
    {
        consoleErr << ICE_STRING_VERSION << endl;
        return EXIT_SUCCESS;
    }

    vector<string> cppArgs;
    vector<string> optargs = opts.argVec("D");
    cppArgs.reserve(optargs.size()); // keep clang-tidy happy
    for (const auto& optarg : optargs)
    {
        cppArgs.push_back("-D" + optarg);
    }

    optargs = opts.argVec("U");
    for (const auto& optarg : optargs)
    {
        cppArgs.push_back("-U" + optarg);
    }

    vector<string> includePaths = opts.argVec("I");
    for (const auto& includePath : includePaths)
    {
        cppArgs.push_back("-I" + Preprocessor::normalizeIncludePath(includePath));
    }

    bool preprocess = opts.isSet("E");

    string output = opts.optArg("output-dir");

    bool depend = opts.isSet("depend");

    bool dependxml = opts.isSet("depend-xml");

    string dependFile = opts.optArg("depend-file");

    bool debug = opts.isSet("debug");

    bool all = opts.isSet("all");

    if (args.empty())
    {
        consoleErr << argv[0] << ": error: no input file" << endl;
        if (!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if (depend && dependxml)
    {
        consoleErr << argv[0] << ": error: cannot specify both --depend and --depend-xml" << endl;
        if (!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if (validate)
    {
        return EXIT_SUCCESS;
    }

    int status = EXIT_SUCCESS;

    Ice::CtrlCHandler ctrlCHandler;
    ctrlCHandler.setCallback(interruptedCallback);

    ostringstream os;
    if (dependxml)
    {
        os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dependencies>" << endl;
    }

    for (auto i = args.begin(); i != args.end(); ++i)
    {
        // Ignore duplicates.
        auto p = find(args.begin(), args.end(), *i);
        if (p != i)
        {
            continue;
        }

        if (depend || dependxml)
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2PHP__");

            if (cppHandle == nullptr)
            {
                return EXIT_FAILURE;
            }

            UnitPtr u = Unit::createUnit("php", false);
            int parseStatus = u->parse(*i, cppHandle, debug);
            u->destroy();

            if (parseStatus == EXIT_FAILURE)
            {
                return EXIT_FAILURE;
            }

            if (!icecpp->printMakefileDependencies(
                    os,
                    depend ? Preprocessor::PHP : Preprocessor::SliceXML,
                    includePaths,
                    "-D__SLICE2PHP__"))
            {
                return EXIT_FAILURE;
            }

            if (!icecpp->close())
            {
                return EXIT_FAILURE;
            }
        }
        else
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2PHP__");

            if (cppHandle == nullptr)
            {
                return EXIT_FAILURE;
            }

            if (preprocess)
            {
                char buf[4096];
                while (fgets(buf, static_cast<int>(sizeof(buf)), cppHandle) != nullptr)
                {
                    if (fputs(buf, stdout) == EOF)
                    {
                        return EXIT_FAILURE;
                    }
                }
                if (!icecpp->close())
                {
                    return EXIT_FAILURE;
                }
            }
            else
            {
                UnitPtr u = Unit::createUnit("php", all);
                int parseStatus = u->parse(*i, cppHandle, debug);

                if (!icecpp->close())
                {
                    u->destroy();
                    return EXIT_FAILURE;
                }

                if (parseStatus == EXIT_FAILURE)
                {
                    status = EXIT_FAILURE;
                }
                else
                {
                    string base = icecpp->getBaseName();
                    string::size_type pos = base.find_last_of("/\\");
                    if (pos != string::npos)
                    {
                        base.erase(0, pos + 1);
                    }

                    string file = base + ".php";
                    if (!output.empty())
                    {
                        file = output + '/' + file;
                    }

                    try
                    {
                        IceInternal::Output out;
                        out.open(file.c_str());
                        if (!out)
                        {
                            ostringstream os;
                            os << "cannot open '" << file << "': " << IceInternal::errorToString(errno);
                            throw FileException(os.str());
                        }
                        FileTracker::instance()->addFile(file);

                        out << "<?php\n";
                        printHeader(out);
                        printGeneratedHeader(out, base + ".ice");

                        // Generate the PHP mapping.
                        generate(u, all, includePaths, out);
                        out.close();
                    }
                    catch (const Slice::FileException& ex)
                    {
                        // If a file could not be created, then cleanup any  created files.
                        FileTracker::instance()->cleanup();
                        u->destroy();
                        consoleErr << argv[0] << ": error: " << ex.what() << endl;
                        return EXIT_FAILURE;
                    }
                    catch (const exception& ex)
                    {
                        FileTracker::instance()->cleanup();
                        consoleErr << argv[0] << ": error: " << ex.what() << endl;
                        status = EXIT_FAILURE;
                    }
                }

                status |= u->getStatus();
                u->destroy();
            }
        }

        {
            lock_guard lock(globalMutex);
            if (interrupted)
            {
                FileTracker::instance()->cleanup();
                return EXIT_FAILURE;
            }
        }
    }

    if (dependxml)
    {
        os << "</dependencies>\n";
    }

    if (depend || dependxml)
    {
        writeDependencies(os.str(), dependFile);
    }

    return status;
}

#ifdef _WIN32
int
wmain(int argc, wchar_t* argv[])
#else
int
main(int argc, char* argv[])
#endif
{
    vector<string> args = Slice::argvToArgs(argc, argv);
    try
    {
        return compile(args);
    }
    catch (const std::exception& ex)
    {
        consoleErr << args[0] << ": error:" << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        consoleErr << args[0] << ": error:"
                   << "unknown exception" << endl;
        return EXIT_FAILURE;
    }
}
