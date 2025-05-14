// Copyright (c) ZeroC, Inc.

#include "PythonUtil.h"
#include "../Slice/MetadataValidation.h"
#include "../Slice/Util.h"
#include "Ice/StringUtil.h"

#include <algorithm>
#include <cassert>
#include <climits>
#include <iterator>
#include <sstream>

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
            if (param->mappedName() == name)
            {
                return name + "_";
            }
        }
        return name;
    }

    const char* const tripleQuotes = R"(""")";

    string typeToDocstring(const TypePtr& type, bool optional)
    {
        assert(type);

        if (optional)
        {
            if (isProxyType(type))
            {
                // We map optional proxies like regular proxies, as XxxPrx or None.
                return typeToDocstring(type, false);
            }
            else
            {
                ostringstream os;
                os << "(";
                os << typeToDocstring(type, false);
                os << " or None)";
                return os.str();
            }
        }

        static constexpr string_view builtinTable[] = {
            "int",
            "bool",
            "int",
            "int",
            "int",
            "float",
            "float",
            "str",
            "Ice.Value",
            "(Ice.ObjectPrx or None)",
            "Ice.Value"};

        BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
        if (builtin)
        {
            if (builtin->kind() == Builtin::KindObject)
            {
                return string{builtinTable[Builtin::KindValue]};
            }
            else
            {
                return string{builtinTable[builtin->kind()]};
            }
        }

        ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
        if (cl)
        {
            return cl->mappedScoped(".").substr(1);
        }

        StructPtr st = dynamic_pointer_cast<Struct>(type);
        if (st)
        {
            return st->mappedScoped(".").substr(1);
        }

        InterfaceDeclPtr proxy = dynamic_pointer_cast<InterfaceDecl>(type);
        if (proxy)
        {
            ostringstream os;
            os << "(";
            os << proxy->mappedScoped(".").substr(1) + "Prx";
            os << " or None)";
            return os.str();
        }

        EnumPtr en = dynamic_pointer_cast<Enum>(type);
        if (en)
        {
            return en->mappedScoped(".").substr(1);
        }

        SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
        if (seq)
        {
            return "list[" + typeToDocstring(seq->type(), false) + "]";
        }

        DictionaryPtr dict = dynamic_pointer_cast<Dictionary>(type);
        if (dict)
        {
            ostringstream os;
            os << "dict[" << typeToDocstring(dict->keyType(), false) << ", "
               << typeToDocstring(dict->valueType(), false) << "]";
            return os.str();
        }

        return "???";
    }

    enum DocstringMode
    {
        DocSync,
        DocAsync,
        DocDispatch
    };

    string docReturnType(const OperationPtr& op, DocstringMode mode)
    {
        ostringstream os;

        if (!op->returnsAnyValues())
        {
            if (mode == DocDispatch)
            {
                os << "None or awaitable";
            }
            else if (mode == DocAsync)
            {
                os << "awaitable";
            }
        }
        else if (mode == DocDispatch)
        {
            os << docReturnType(op, DocSync) << " or " << docReturnType(op, DocAsync);
        }
        else if (mode == DocAsync)
        {
            os << "awaitable of " << docReturnType(op, DocSync);
        }
        else
        {
            if (op->returnsMultipleValues())
            {
                os << "tuple of (";
                if (op->returnType())
                {
                    os << typeToDocstring(op->returnType(), op->returnIsOptional());
                    os << ", ";
                }
                for (const auto& param : op->outParameters())
                {
                    os << typeToDocstring(param->type(), param->optional());
                    if (param != op->outParameters().back())
                    {
                        os << ", ";
                    }
                }
                os << ")";
            }
            else if (op->returnType())
            {
                os << typeToDocstring(op->returnType(), op->returnIsOptional());
            }
        }

        return os.str();
    }

    /// Returns a DocString formatted link to the provided Slice identifier.
    string pyLinkFormatter(const string& rawLink, const ContainedPtr&, const SyntaxTreeBasePtr& target)
    {
        ostringstream result;
        if (target)
        {
            if (auto builtinTarget = dynamic_pointer_cast<Builtin>(target))
            {
                if (builtinTarget->kind() == Builtin::KindObject)
                {
                    result << ":class:`Ice.Object`";
                }
                else if (builtinTarget->kind() == Builtin::KindValue)
                {
                    result << ":class:`Ice.Value`";
                }
                else if (builtinTarget->kind() == Builtin::KindObjectProxy)
                {
                    result << ":class:`Ice.ObjectPrx`";
                }
                else
                {
                    result << "``" << typeToDocstring(builtinTarget, false) << "``";
                }
            }
            else if (auto operationTarget = dynamic_pointer_cast<Operation>(target))
            {
                string targetScoped = operationTarget->interface()->mappedScoped(".").substr(1);

                // link to the method on the proxy interface
                result << ":meth:`" << targetScoped << "Prx." << operationTarget->mappedName() << "`";
            }
            else
            {
                string targetScoped = dynamic_pointer_cast<Contained>(target)->mappedScoped(".").substr(1);
                result << ":class:`" << targetScoped;
                if (auto interfaceTarget = dynamic_pointer_cast<InterfaceDecl>(target))
                {
                    // link to the proxy interface
                    result << "Prx";
                }
                result << "`";
            }
        }
        else
        {
            result << "``";

            auto hashPos = rawLink.find('#');
            if (hashPos != string::npos)
            {
                if (hashPos != 0)
                {
                    result << rawLink.substr(0, hashPos) << ".";
                }
                result << rawLink.substr(hashPos + 1);
            }
            else
            {
                result << rawLink;
            }

            result << "``";
        }
        return result.str();
    }

    string formatFields(const DataMemberList& members)
    {
        if (members.empty())
        {
            return "";
        }

        ostringstream os;
        bool first = true;
        os << "{Ice.Util.format_fields(";
        for (const auto& dataMember : members)
        {
            if (!first)
            {
                os << ", ";
            }
            first = false;
            os << dataMember->mappedName() << "=self." << dataMember->mappedName();
        }
        os << ")}";
        return os.str();
    }
}

namespace Slice::Python
{
    //
    // ModuleVisitor finds all of the Slice modules whose include level is greater
    // than 0 and emits a statement of the following form:
    //
    // _M_Foo = Ice.openModule('Foo')
    //
    // This statement allows the code generated for this translation unit to refer
    // to types residing in those included modules.
    //
    class ModuleVisitor final : public ParserVisitor
    {
    public:
        ModuleVisitor(Output&, set<string>&);

        bool visitModuleStart(const ModulePtr&) final;

        [[nodiscard]] bool shouldVisitIncludedDefinitions() const final { return true; }

    private:
        Output& _out;
        set<string>& _history;
    };

    //
    // CodeVisitor generates the Python mapping for a translation unit.
    //
    class CodeVisitor final : public ParserVisitor
    {
    public:
        CodeVisitor(IceInternal::Output&, set<string>&);

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
        // Emit Python code for operations
        //
        void writeOperations(const InterfaceDefPtr&);

        //
        // Emit Python code to assign the given symbol in the current module.
        //
        void registerName(const string&);

        //
        // Emit the tuple for a Slice type.
        //
        void writeType(const TypePtr&);

        //
        // Write an initializer value for a given type.
        //
        void writeInitializer(const DataMemberPtr&);

        //
        // Add a value to a hash code.
        //
        void writeHash(const string&, const TypePtr&, int&);

        //
        // Write Python metadata as a tuple.
        //
        void writeMetadata(const MetadataList&);

        //
        // Convert an operation mode into a string.
        //
        string getOperationMode(Slice::Operation::Mode);

        //
        // Write a member assignment statement for a constructor.
        //
        void writeAssign(const DataMemberPtr& member);

        //
        // Write a constant value.
        //
        void writeConstantValue(const TypePtr&, const SyntaxTreeBasePtr&, const string&);

        /// Write constructor parameters with default values.
        void writeConstructorParams(const DataMemberList& members);

        void writeDocstring(const optional<DocComment>&, const string& = "");
        void writeDocstring(const optional<DocComment>&, const DataMemberList&);
        void writeDocstring(const optional<DocComment>&, const EnumeratorList&);

        void writeDocstring(const OperationPtr&, DocstringMode);

        Output& _out;
        set<string>& _moduleHistory;
        list<string> _moduleStack;
        set<string> _classHistory;
    };
}

static void
writeModuleHasDefinitionCheck(Output& out, const ContainedPtr& cont, const string& name)
{
    string scope = cont->mappedScope(".").substr(1);
    assert(!scope.empty());

    string package = Slice::Python::getPackageMetadata(cont);
    if (!package.empty())
    {
        scope = package + "." + scope;
    }

    out << sp << nl << "if '" << name << "' not in _M_" << scope << "__dict__:";
}

//
// ModuleVisitor implementation.
//
Slice::Python::ModuleVisitor::ModuleVisitor(Output& out, set<string>& history) : _out(out), _history(history) {}

bool
Slice::Python::ModuleVisitor::visitModuleStart(const ModulePtr& p)
{
    if (p->includeLevel() > 0)
    {
        string abs = getAbsolute(p);
        if (_history.count(abs) == 0)
        {
            // If this is a top-level module, then we check if it has package metadata.
            // If so, we need to emit statements to open each of the modules in the
            // package before we can open this module.
            if (p->isTopLevel())
            {
                string pkg = getPackageMetadata(p);
                if (!pkg.empty())
                {
                    vector<string> v;
                    splitString(pkg, ".", v);
                    string mod;
                    for (const auto& q : v)
                    {
                        mod = mod.empty() ? q : mod + "." + q;
                        if (_history.count(mod) == 0)
                        {
                            _out << nl << "_M_" << mod << " = Ice.openModule('" << mod << "')";
                            _history.insert(mod);
                        }
                    }
                }
            }

            _out << sp << nl << "# Included module " << abs;
            _out << nl << "_M_" << abs << " = Ice.openModule('" << abs << "')";
            _history.insert(abs);
        }
    }

    return true;
}

// CodeVisitor implementation.
Slice::Python::CodeVisitor::CodeVisitor(Output& out, set<string>& moduleHistory)
    : _out(out),
      _moduleHistory(moduleHistory)
{
}

bool
Slice::Python::CodeVisitor::visitModuleStart(const ModulePtr& p)
{
    //
    // As each module is opened, we emit the statement
    //
    // __name__ = 'Foo'
    //
    // This renames the current module to 'Foo' so that subsequent
    // type definitions have the proper fully-qualified name.
    //
    // We also emit the statement
    //
    // _M_Foo = Ice.openModule('Foo')
    //
    // This allows us to create types in the module Foo.
    //
    string abs = getAbsolute(p);
    _out << sp << nl << "# Start of module " << abs;
    if (_moduleHistory.count(abs) == 0) // Don't emit this more than once for each module.
    {
        // If this is a top-level module, then we check if it has package metadata.
        // If so, we need to emit statements to open each of the modules in the
        // package before we can open this module.
        if (p->isTopLevel())
        {
            string pkg = getPackageMetadata(p);
            if (!pkg.empty())
            {
                vector<string> v;
                splitString(pkg, ".", v);
                string mod;
                for (const auto& q : v)
                {
                    mod = mod.empty() ? q : mod + "." + q;
                    if (_moduleHistory.count(mod) == 0) // Don't emit this more than once for each module.
                    {
                        _out << nl << "_M_" << mod << " = Ice.openModule('" << mod << "')";
                        _moduleHistory.insert(mod);
                    }
                }
            }
        }
        _out << nl << "_M_" << abs << " = Ice.openModule('" << abs << "')";
        _moduleHistory.insert(abs);
    }
    _out << nl << "__name__ = '" << abs << "'";

    writeDocstring(DocComment::parseFrom(p, pyLinkFormatter, true), "_M_" + abs + ".__doc__ = ");

    _moduleStack.push_front(abs);
    return true;
}

void
Slice::Python::CodeVisitor::visitModuleEnd(const ModulePtr&)
{
    assert(!_moduleStack.empty());
    _out << sp << nl << "# End of module " << _moduleStack.front();
    _moduleStack.pop_front();

    if (!_moduleStack.empty())
    {
        _out << sp << nl << "__name__ = '" << _moduleStack.front() << "'";
    }
}

void
Slice::Python::CodeVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    // Emit forward declarations.
    string scoped = p->scoped();
    if (_classHistory.count(scoped) == 0)
    {
        writeModuleHasDefinitionCheck(_out, p, p->mappedName());
        _out.inc();
        _out << nl << getMetaTypeReference(p) << " = IcePy.declareValue('" << scoped << "')";
        _out.dec();
        _classHistory.insert(scoped); // Avoid redundant declarations.
    }
}

void
Slice::Python::CodeVisitor::visitInterfaceDecl(const InterfaceDeclPtr& p)
{
    // Emit forward declarations.
    string scoped = p->scoped();
    if (_classHistory.count(scoped) == 0)
    {
        writeModuleHasDefinitionCheck(_out, p, p->mappedName());
        _out.inc();
        _out << nl << getMetaTypeReference(p) << "Prx" << " = IcePy.declareProxy('" << scoped << "')";
        _out.dec();
        _classHistory.insert(scoped); // Avoid redundant declarations.
    }
}

void
Slice::Python::CodeVisitor::writeOperations(const InterfaceDefPtr& p)
{
    // Emit a placeholder for each operation.
    for (const auto& operation : p->operations())
    {
        const string sliceName = operation->name();
        const string mappedName = operation->mappedName();

        if (operation->hasMarshaledResult())
        {
            string capName = sliceName;
            capName[0] = static_cast<char>(toupper(static_cast<unsigned char>(capName[0])));
            _out << sp;
            _out << nl << "@staticmethod";
            _out << nl << "def " << capName << "MarshaledResult(result, current):";
            _out.inc();
            _out << nl << tripleQuotes;
            _out << nl << "Immediately marshals the result of an invocation of " << sliceName;
            _out << nl << "and returns an object that the servant implementation must return";
            _out << nl << "as its result.";
            _out << nl;
            _out << nl << "Args:";
            _out << nl << "  result: The result (or result tuple) of the invocation.";
            _out << nl << "  current: The Current object passed to the invocation.";
            _out << nl;
            _out << nl << "Returns";
            _out << nl << "  An object containing the marshaled result.";
            _out << nl << tripleQuotes;
            _out << nl << "return IcePy.MarshaledResult(result, " << getTypeReference(p) << "._op_" << sliceName
                 << ", current.adapter.getCommunicator()._getImpl(), current.encoding)";
            _out.dec();
        }

        _out << sp << nl << "def " << mappedName << "(self";

        for (const auto& param : operation->parameters())
        {
            if (!param->isOutParam())
            {
                _out << ", " << param->mappedName();
            }
        }

        const string currentParamName = getEscapedParamName(operation, "current");
        _out << ", " << currentParamName;
        _out << "):";
        _out.inc();

        writeDocstring(operation, DocDispatch);

        _out << nl << "raise NotImplementedError(\"servant method '" << mappedName << "' not implemented\")";
        _out.dec();
    }
}

bool
Slice::Python::CodeVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    const string scoped = p->scoped();
    const string type = getMetaTypeReference(p);
    const string valueName = p->mappedName();
    const ClassDefPtr base = p->base();
    const DataMemberList members = p->dataMembers();

    writeModuleHasDefinitionCheck(_out, p, valueName);
    _out.inc();
    _out << nl << getTypeReference(p) << " = None";
    _out << nl << "class " << valueName << '(';
    if (!base)
    {
        _out << "Ice.Value";
    }
    else
    {
        _out << getTypeReference(base);
    }
    _out << "):";

    _out.inc();

    writeDocstring(DocComment::parseFrom(p, pyLinkFormatter, true), members);

    //
    // __init__
    //
    _out << nl << "def __init__(self";
    writeConstructorParams(p->allDataMembers());
    _out << "):";
    _out.inc();
    if (!base && members.empty())
    {
        _out << nl << "pass";
    }
    else
    {
        if (base)
        {
            _out << nl << getTypeReference(base) << ".__init__(self";
            for (const auto& member : base->allDataMembers())
            {
                _out << ", " << member->mappedName();
            }
            _out << ')';
        }
        for (const auto& member : members)
        {
            writeAssign(member);
        }
    }
    _out.dec();

    //
    // ice_id
    //
    _out << sp << nl << "def ice_id(self):";
    _out.inc();
    _out << nl << "return '" << scoped << "'";
    _out.dec();

    //
    // ice_staticId
    //
    _out << sp << nl << "@staticmethod";
    _out << nl << "def ice_staticId():";
    _out.inc();
    _out << nl << "return '" << scoped << "'";
    _out.dec();

    // Generate the __repr__ method for this Value class.
    // The default __str__ method inherited from Ice.Value calls __repr__().
    _out << sp << nl << "def __repr__(self):";
    _out.inc();
    _out << nl << "return f\"" << getAbsolute(p) << "(" << formatFields(p->allDataMembers()) << ")\"";
    _out.dec();

    _out.dec();

    _out << sp << nl << type << " = IcePy.defineValue('" << scoped << "', " << valueName << ", " << p->compactId()
         << ", ";
    writeMetadata(p->getMetadata());
    _out << ", False, ";
    if (!base)
    {
        _out << "None";
    }
    else
    {
        _out << getMetaTypeReference(base);
    }
    _out << ", (";
    //
    // Members
    //
    // Data members are represented as a tuple:
    //
    //   ('MemberName', MemberMetadata, MemberType, Optional, Tag)
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
        _out << "('";
        _out << (*r)->mappedName() << "', ";
        writeMetadata((*r)->getMetadata());
        _out << ", ";
        writeType((*r)->type());
        _out << ", " << ((*r)->optional() ? "True" : "False") << ", " << ((*r)->optional() ? (*r)->tag() : 0) << ')';
    }
    if (members.size() == 1)
    {
        _out << ',';
    }
    else if (members.size() > 1)
    {
        _out.dec();
        _out << nl;
    }
    _out << "))";
    _out << nl << valueName << "._ice_type = " << type;

    registerName(valueName);

    _out.dec();

    return false;
}

bool
Slice::Python::CodeVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    string scoped = p->scoped();
    string className = p->mappedName();
    string classAbs = getTypeReference(p);
    string prxName = className + "Prx";
    string prxAbs = classAbs + "Prx";
    InterfaceList bases = p->bases();

    writeModuleHasDefinitionCheck(_out, p, prxName);
    _out.inc();

    // Define the proxy class
    _out << nl << prxAbs << " = None";
    _out << nl << "class " << prxName << '(';

    {
        vector<string> baseClasses;
        for (const auto& base : bases)
        {
            InterfaceDefPtr d = base;
            baseClasses.push_back(getTypeReference(base) + "Prx");
        }

        if (baseClasses.empty())
        {
            _out << "Ice.ObjectPrx";
        }
        else
        {
            auto q = baseClasses.begin();
            while (q != baseClasses.end())
            {
                _out << *q;

                if (++q != baseClasses.end())
                {
                    _out << ", ";
                }
            }
        }
    }
    _out << "):";
    _out.inc();

    _out << sp;
    _out << nl << "def __init__(self, communicator, proxyString):";
    _out.inc();
    _out << nl << tripleQuotes;
    _out << nl << "Creates a new " << prxName << " proxy";
    _out << nl;
    _out << nl << "Parameters";
    _out << nl << "----------";
    _out << nl << "communicator : Ice.Communicator";
    _out << nl << "    The communicator of the new proxy.";
    _out << nl << "proxyString : str";
    _out << nl << "    The string representation of the proxy.";
    _out << nl;
    _out << nl << "Raises";
    _out << nl << "------";
    _out << nl << "ParseException";
    _out << nl << "    Thrown when proxyString is not a valid proxy string.";
    _out << nl << tripleQuotes;
    _out << nl << "super().__init__(communicator, proxyString)";
    _out.dec();

    OperationList operations = p->operations();
    for (const auto& operation : operations)
    {
        const string opName = operation->name();
        string mappedOpName = operation->mappedName();
        if (mappedOpName == "checkedCast" || mappedOpName == "uncheckedCast")
        {
            mappedOpName.insert(0, "_");
        }
        TypePtr ret = operation->returnType();
        ParameterList paramList = operation->parameters();
        string inParams;
        string inParamsDecl;

        // Find the last required parameter, all optional parameters after the last required parameter will use
        // None as the default.
        ParameterPtr lastRequiredParameter;
        for (const auto& q : paramList)
        {
            if (!q->isOutParam() && !q->optional())
            {
                lastRequiredParameter = q;
            }
        }

        bool afterLastRequiredParameter = lastRequiredParameter == nullptr;
        for (const auto& q : paramList)
        {
            if (!q->isOutParam())
            {
                if (!inParams.empty())
                {
                    inParams.append(", ");
                    inParamsDecl.append(", ");
                }
                string param = q->mappedName();
                inParams.append(param);
                if (afterLastRequiredParameter)
                {
                    param += "=None";
                }
                inParamsDecl.append(param);

                if (q == lastRequiredParameter)
                {
                    afterLastRequiredParameter = true;
                }
            }
        }

        _out << sp;
        _out << nl << "def " << mappedOpName << "(self";
        if (!inParamsDecl.empty())
        {
            _out << ", " << inParamsDecl;
        }
        const string contextParamName = getEscapedParamName(operation, "context");
        _out << ", " << contextParamName << "=None):";
        _out.inc();
        writeDocstring(operation, DocSync);
        _out << nl << "return " << classAbs << "._op_" << opName << ".invoke(self, ((" << inParams;
        if (!inParams.empty() && inParams.find(',') == string::npos)
        {
            _out << ", ";
        }
        _out << "), " << contextParamName << "))";
        _out.dec();

        //
        // Async operations.
        //
        _out << sp;
        _out << nl << "def " << mappedOpName << "Async(self";
        if (!inParams.empty())
        {
            _out << ", " << inParams;
        }
        _out << ", " << contextParamName << "=None):";
        _out.inc();
        writeDocstring(operation, DocAsync);
        _out << nl << "return " << classAbs << "._op_" << opName << ".invokeAsync(self, ((" << inParams;
        if (!inParams.empty() && inParams.find(',') == string::npos)
        {
            _out << ", ";
        }
        _out << "), " << contextParamName << "))";
        _out.dec();
    }

    _out << sp << nl << "@staticmethod";
    _out << nl << "def checkedCast(proxy, facet=None, context=None):";
    _out.inc();
    _out << nl << "return Ice.checkedCast(" << prxAbs << ", proxy, facet, context)";
    _out.dec();

    _out << sp << nl << "@staticmethod";
    _out << nl << "def checkedCastAsync(proxy, facet=None, context=None):";
    _out.inc();
    _out << nl << "return Ice.checkedCastAsync(" << prxAbs << ", proxy, facet, context)";
    _out.dec();

    _out << sp << nl << "@staticmethod";
    _out << nl << "def uncheckedCast(proxy, facet=None):";
    _out.inc();
    _out << nl << "return Ice.uncheckedCast(" << prxAbs << ", proxy, facet)";
    _out.dec();

    //
    // ice_staticId
    //
    _out << sp << nl << "@staticmethod";
    _out << nl << "def ice_staticId():";
    _out.inc();
    _out << nl << "return '" << scoped << "'";
    _out.dec();

    _out.dec(); // end prx class

    _out << nl << getMetaTypeReference(p) << "Prx" << " = IcePy.defineProxy('" << scoped << "', " << prxName << ")";

    registerName(prxName);

    // Define the servant class
    _out << sp << nl << classAbs << " = None";
    _out << nl << "class " << className << '(';
    {
        vector<string> baseClasses;
        for (const auto& base : bases)
        {
            InterfaceDefPtr d = base;
            baseClasses.push_back(getTypeReference(base));
        }

        if (baseClasses.empty())
        {
            _out << "Ice.Object";
        }
        else
        {
            auto q = baseClasses.begin();
            while (q != baseClasses.end())
            {
                _out << *q;

                if (++q != baseClasses.end())
                {
                    _out << ", ";
                }
            }
        }
    }
    _out << "):";

    _out.inc();

    //
    // ice_ids
    //
    StringList ids = p->ids();
    _out << sp << nl << "def ice_ids(self, current):";
    _out.inc();
    _out << nl << "return (";
    for (auto q = ids.begin(); q != ids.end(); ++q)
    {
        if (q != ids.begin())
        {
            _out << ", ";
        }
        _out << "'" << *q << "'";
    }
    _out << ')';
    _out.dec();

    //
    // ice_id
    //
    _out << sp << nl << "def ice_id(self, current):";
    _out.inc();
    _out << nl << "return '" << scoped << "'";
    _out.dec();

    //
    // ice_staticId
    //
    _out << sp << nl << "@staticmethod";
    _out << nl << "def ice_staticId():";
    _out.inc();
    _out << nl << "return '" << scoped << "'";
    _out.dec();

    writeOperations(p);

    _out.dec();

    //
    // Define each operation. The arguments to the IcePy.Operation constructor are:
    //
    // 'sliceOpName', 'mappedOpName', Mode, AMD, Format, Metadata, (InParams), (OutParams), ReturnParam, (Exceptions)
    //
    // where InParams and OutParams are tuples of type descriptions, and Exceptions
    // is a tuple of exception type ids.
    //
    if (!operations.empty())
    {
        _out << sp;
    }
    for (const auto& operation : operations)
    {
        ParameterList params = operation->parameters();
        ParameterList::iterator t;
        int count;
        string format;
        optional<FormatType> opFormat = operation->format();
        if (opFormat)
        {
            switch (*opFormat)
            {
                case CompactFormat:
                    format = "Ice.FormatType.CompactFormat";
                    break;
                case SlicedFormat:
                    format = "Ice.FormatType.SlicedFormat";
                    break;
                default:
                    assert(false);
            }
        }
        else
        {
            format = "None";
        }

        const string sliceName = operation->name();

        _out << nl << className << "._op_" << sliceName << " = IcePy.Operation('" << sliceName << "', '"
             << operation->mappedName() << "', " << getOperationMode(operation->mode()) << ", " << format << ", ";
        writeMetadata(operation->getMetadata());
        _out << ", (";
        for (t = params.begin(), count = 0; t != params.end(); ++t)
        {
            if (!(*t)->isOutParam())
            {
                if (count > 0)
                {
                    _out << ", ";
                }
                _out << '(';
                writeMetadata((*t)->getMetadata());
                _out << ", ";
                writeType((*t)->type());
                _out << ", " << ((*t)->optional() ? "True" : "False") << ", " << ((*t)->optional() ? (*t)->tag() : 0)
                     << ')';
                ++count;
            }
        }
        if (count == 1)
        {
            _out << ',';
        }
        _out << "), (";
        for (t = params.begin(), count = 0; t != params.end(); ++t)
        {
            if ((*t)->isOutParam())
            {
                if (count > 0)
                {
                    _out << ", ";
                }
                _out << '(';
                writeMetadata((*t)->getMetadata());
                _out << ", ";
                writeType((*t)->type());
                _out << ", " << ((*t)->optional() ? "True" : "False") << ", " << ((*t)->optional() ? (*t)->tag() : 0)
                     << ')';
                ++count;
            }
        }
        if (count == 1)
        {
            _out << ',';
        }
        _out << "), ";
        TypePtr returnType = operation->returnType();
        if (returnType)
        {
            //
            // The return type has the same format as an in/out parameter:
            //
            // Metadata, Type, Optional?, OptionalTag
            //
            _out << "((), ";
            writeType(returnType);
            _out << ", " << (operation->returnIsOptional() ? "True" : "False") << ", "
                 << (operation->returnIsOptional() ? operation->returnTag() : 0) << ')';
        }
        else
        {
            _out << "None";
        }
        _out << ", (";
        ExceptionList exceptions = operation->throws();
        for (auto u = exceptions.begin(); u != exceptions.end(); ++u)
        {
            if (u != exceptions.begin())
            {
                _out << ", ";
            }
            _out << getMetaTypeReference(*u);
        }
        if (exceptions.size() == 1)
        {
            _out << ',';
        }
        _out << "))";

        if (operation->isDeprecated())
        {
            // Get the deprecation reason if present, or default to an empty string.
            string reason = operation->getDeprecationReason().value_or("");
            _out << nl << className << "._op_" << sliceName << ".deprecate(\"" << reason << "\")";
        }
    }

    registerName(className);
    _out.dec();

    return false;
}

bool
Slice::Python::CodeVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    const string scoped = p->scoped();
    const string name = p->mappedName();

    const ExceptionPtr base = p->base();
    string baseName;

    const DataMemberList members = p->dataMembers();

    writeModuleHasDefinitionCheck(_out, p, name);
    _out.inc();
    _out << nl << getTypeReference(p) << " = None";
    _out << nl << "class " << name << '(';
    if (base)
    {
        baseName = getTypeReference(base);
        _out << baseName;
    }
    else
    {
        _out << "Ice.UserException";
    }
    _out << "):";
    _out.inc();

    writeDocstring(DocComment::parseFrom(p, pyLinkFormatter, true), members);

    //
    // __init__
    //
    _out << nl << "def __init__(self";
    writeConstructorParams(p->allDataMembers());
    _out << "):";
    _out.inc();
    if (!base && members.empty())
    {
        _out << nl << "pass";
    }
    else
    {
        if (base)
        {
            _out << nl << baseName << ".__init__(self";
            for (const auto& member : base->allDataMembers())
            {
                _out << ", " << member->mappedName();
            }
            _out << ')';
        }
        for (const auto& member : members)
        {
            writeAssign(member);
        }
    }
    _out.dec();

    // Generate the __repr__ method for this Exception class.
    // The default __str__ method inherited from Ice.UserException calls __repr__().
    _out << sp << nl << "def __repr__(self):";
    _out.inc();
    _out << nl << "return f\"" << getAbsolute(p) << "(" << formatFields(p->allDataMembers()) << ")\"";
    _out.dec();

    //
    // _ice_id
    //
    _out << sp << nl << "_ice_id = '" << scoped << "'";

    _out.dec();

    //
    // Emit the type information.
    //
    string type = getMetaTypeReference(p);
    _out << sp << nl << type << " = IcePy.defineException('" << scoped << "', " << name << ", ";
    writeMetadata(p->getMetadata());
    _out << ", ";
    if (!base)
    {
        _out << "None";
    }
    else
    {
        _out << getMetaTypeReference(base);
    }
    _out << ", (";
    if (members.size() > 1)
    {
        _out.inc();
        _out << nl;
    }
    //
    // Data members are represented as a tuple:
    //
    //   ('MemberName', MemberMetadata, MemberType, Optional, Tag)
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a user-defined type.
    //
    for (auto dmli = members.begin(); dmli != members.end(); ++dmli)
    {
        if (dmli != members.begin())
        {
            _out << ',' << nl;
        }
        _out << "('" << (*dmli)->mappedName() << "', ";
        writeMetadata((*dmli)->getMetadata());
        _out << ", ";
        writeType((*dmli)->type());
        _out << ", " << ((*dmli)->optional() ? "True" : "False") << ", " << ((*dmli)->optional() ? (*dmli)->tag() : 0)
             << ')';
    }
    if (members.size() == 1)
    {
        _out << ',';
    }
    else if (members.size() > 1)
    {
        _out.dec();
        _out << nl;
    }
    _out << "))";
    _out << nl << name << "._ice_type = " << type;

    registerName(name);

    _out.dec();

    return false;
}

bool
Slice::Python::CodeVisitor::visitStructStart(const StructPtr& p)
{
    const string scoped = p->scoped();
    const string abs = getTypeReference(p);
    const string name = p->mappedName();
    const DataMemberList members = p->dataMembers();

    writeModuleHasDefinitionCheck(_out, p, name);
    _out.inc();
    _out << nl << abs << " = None";
    _out << nl << "class " << name << "(object):";
    _out.inc();

    writeDocstring(DocComment::parseFrom(p, pyLinkFormatter, true), members);

    _out << nl << "def __init__(self";
    writeConstructorParams(p->dataMembers());
    _out << "):";
    _out.inc();
    for (const auto& member : members)
    {
        writeAssign(member);
    }
    _out.dec();

    //
    // Only generate __hash__ and the comparison operators if this structure type is a legal dictionary key type.
    //
    if (Dictionary::isLegalKeyType(p))
    {
        _out << sp << nl << "def __hash__(self):";
        _out.inc();
        _out << nl << "_h = 0";
        int iter = 0;
        for (const auto& member : members)
        {
            string s = "self." + member->mappedName();
            writeHash(s, member->type(), iter);
        }
        _out << nl << "return _h % 0x7fffffff";
        _out.dec();

        //
        // Rich operators.  __lt__, __le__, __eq__, __ne__, __gt__, __ge__
        //

        _out << sp << nl << "def __compare(self, other):";
        _out.inc();
        _out << nl << "if other is None:";
        _out.inc();
        _out << nl << "return 1";
        _out.dec();
        _out << nl << "elif not isinstance(other, " << abs << "):";
        _out.inc();
        _out << nl << "return NotImplemented";
        _out.dec();
        _out << nl << "else:";
        _out.inc();
        for (const auto& member : members)
        {
            const string memberName = member->mappedName();

            //
            // The None value is not orderable in Python 3.
            //
            _out << nl << "if self." << memberName << " is None or other." << memberName << " is None:";
            _out.inc();
            _out << nl << "if self." << memberName << " != other." << memberName << ':';
            _out.inc();
            _out << nl << "return (-1 if self." << memberName << " is None else 1)";
            _out.dec();
            _out.dec();
            _out << nl << "else:";
            _out.inc();
            _out << nl << "if self." << memberName << " < other." << memberName << ':';
            _out.inc();
            _out << nl << "return -1";
            _out.dec();
            _out << nl << "elif self." << memberName << " > other." << memberName << ':';
            _out.inc();
            _out << nl << "return 1";
            _out.dec();
            _out.dec();
        }
        _out << nl << "return 0";
        _out.dec();
        _out.dec();

        _out << sp << nl << "def __lt__(self, other):";
        _out.inc();
        _out << nl << "r = self.__compare(other)";
        _out << nl << "if r is NotImplemented:";
        _out.inc();
        _out << nl << "return r";
        _out.dec();
        _out << nl << "else:";
        _out.inc();
        _out << nl << "return r < 0";
        _out.dec();
        _out.dec();

        _out << sp << nl << "def __le__(self, other):";
        _out.inc();
        _out << nl << "r = self.__compare(other)";
        _out << nl << "if r is NotImplemented:";
        _out.inc();
        _out << nl << "return r";
        _out.dec();
        _out << nl << "else:";
        _out.inc();
        _out << nl << "return r <= 0";
        _out.dec();
        _out.dec();

        _out << sp << nl << "def __gt__(self, other):";
        _out.inc();
        _out << nl << "r = self.__compare(other)";
        _out << nl << "if r is NotImplemented:";
        _out.inc();
        _out << nl << "return r";
        _out.dec();
        _out << nl << "else:";
        _out.inc();
        _out << nl << "return r > 0";
        _out.dec();
        _out.dec();

        _out << sp << nl << "def __ge__(self, other):";
        _out.inc();
        _out << nl << "r = self.__compare(other)";
        _out << nl << "if r is NotImplemented:";
        _out.inc();
        _out << nl << "return r";
        _out.dec();
        _out << nl << "else:";
        _out.inc();
        _out << nl << "return r >= 0";
        _out.dec();
        _out.dec();

        _out << sp << nl << "def __eq__(self, other):";
        _out.inc();
        _out << nl << "r = self.__compare(other)";
        _out << nl << "if r is NotImplemented:";
        _out.inc();
        _out << nl << "return r";
        _out.dec();
        _out << nl << "else:";
        _out.inc();
        _out << nl << "return r == 0";
        _out.dec();
        _out.dec();

        _out << sp << nl << "def __ne__(self, other):";
        _out.inc();
        _out << nl << "r = self.__compare(other)";
        _out << nl << "if r is NotImplemented:";
        _out.inc();
        _out << nl << "return r";
        _out.dec();
        _out << nl << "else:";
        _out.inc();
        _out << nl << "return r != 0";
        _out.dec();
        _out.dec();
    }
    else
    {
        //
        // Only generate the rich equality operators __eq__ and __ne__.
        //

        _out << sp << nl << "def __eq__(self, other):";
        _out.inc();
        _out << nl << "if other is None:";
        _out.inc();
        _out << nl << "return False";
        _out.dec();
        _out << nl << "elif not isinstance(other, " << abs << "):";
        _out.inc();
        _out << nl << "return NotImplemented";
        _out.dec();
        _out << nl << "else:";
        _out.inc();
        for (const auto& member : members)
        {
            const string memberName = member->mappedName();

            //
            // The None value is not orderable in Python 3.
            //
            _out << nl << "if self." << memberName << " != other." << memberName << ':';
            _out.inc();
            _out << nl << "return False";
            _out.dec();
        }
        _out << nl << "return True";
        _out.dec();
        _out.dec();

        _out << sp << nl << "def __ne__(self, other):";
        _out.inc();
        _out << nl << "return not self.__eq__(other)";
        _out.dec();
    }

    // Generate the __repr__ method for this struct class.
    _out << sp << nl << "def __repr__(self):";
    _out.inc();
    _out << nl << "return f\"" << getAbsolute(p) << "(" << formatFields(members) << ")\"";
    _out.dec();

    _out << sp << nl << "def __str__(self):";
    _out.inc();
    _out << nl << "return repr(self)";
    _out.dec();

    _out.dec();

    //
    // Emit the type information.
    //
    _out << sp << nl << getMetaTypeReference(p) << " = IcePy.defineStruct('" << scoped << "', " << name << ", ";
    writeMetadata(p->getMetadata());
    _out << ", (";
    //
    // Data members are represented as a tuple:
    //
    //   ('MemberName', MemberMetadata, MemberType)
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
        _out << "('" << (*r)->mappedName() << "', ";
        writeMetadata((*r)->getMetadata());
        _out << ", ";
        writeType((*r)->type());
        _out << ')';
    }
    if (members.size() == 1)
    {
        _out << ',';
    }
    else if (members.size() > 1)
    {
        _out.dec();
        _out << nl;
    }
    _out << "))";

    registerName(name);

    _out.dec();

    return false;
}

void
Slice::Python::CodeVisitor::visitSequence(const SequencePtr& p)
{
    // Emit the type information.
    writeModuleHasDefinitionCheck(_out, p, "_t_" + p->mappedName());
    _out.inc();
    _out << nl << getMetaTypeReference(p) << " = IcePy.defineSequence('" << p->scoped() << "', ";
    writeMetadata(p->getMetadata());
    _out << ", ";
    writeType(p->type());
    _out << ")";
    _out.dec();
}

void
Slice::Python::CodeVisitor::visitDictionary(const DictionaryPtr& p)
{
    // Emit the type information.
    writeModuleHasDefinitionCheck(_out, p, "_t_" + p->mappedName());
    _out.inc();
    _out << nl << getMetaTypeReference(p) << " = IcePy.defineDictionary('" << p->scoped() << "', ";
    writeMetadata(p->getMetadata());
    _out << ", ";
    writeType(p->keyType());
    _out << ", ";
    writeType(p->valueType());
    _out << ")";
    _out.dec();
}

void
Slice::Python::CodeVisitor::visitEnum(const EnumPtr& p)
{
    string scoped = p->scoped();
    string name = p->mappedName();
    EnumeratorList enumerators = p->enumerators();

    writeModuleHasDefinitionCheck(_out, p, name);
    _out.inc();
    _out << nl << getTypeReference(p) << " = None";
    _out << nl << "class " << name << "(Ice.EnumBase):";
    _out.inc();

    writeDocstring(DocComment::parseFrom(p, pyLinkFormatter, true), enumerators);

    _out << sp << nl << "def __init__(self, _n, _v):";
    _out.inc();
    _out << nl << "Ice.EnumBase.__init__(self, _n, _v)";
    _out.dec();

    _out << sp << nl << "def valueOf(self, _n):";
    _out.inc();
    _out << nl << "if _n in self._enumerators:";
    _out.inc();
    _out << nl << "return self._enumerators[_n]";
    _out.dec();
    _out << nl << "return None";
    _out.dec();
    _out << nl << "valueOf = classmethod(valueOf)";

    _out << sp << nl << "def __repr__(self):";
    _out.inc();

    _out << nl << "if self._value in _M_" << _moduleStack.front() << '.' << name << "._enumerators:";
    _out.inc();
    _out << nl << "return f\"" << getAbsolute(p) << ".{self._name}\"";
    _out.dec();

    _out << nl << "else:";
    _out.inc();
    _out << nl << "return f\"" << getAbsolute(p) << "({self._name!r}, {self._value!r})\"";
    _out.dec();

    _out.dec();

    _out.dec();

    _out << sp;
    for (const auto& enumerator : enumerators)
    {
        _out << nl << name << '.' << enumerator->mappedName() << " = " << name << "(\"" << enumerator->name() << "\", "
             << enumerator->value() << ')';
    }
    _out << nl << name << "._enumerators = { ";
    for (auto q = enumerators.begin(); q != enumerators.end(); ++q)
    {
        if (q != enumerators.begin())
        {
            _out << ", ";
        }
        _out << (*q)->value() << ':' << name << '.' << (*q)->mappedName();
    }
    _out << " }";

    //
    // Emit the type information.
    //
    _out << sp << nl << getMetaTypeReference(p) << " = IcePy.defineEnum('" << scoped << "', " << name << ", ";
    writeMetadata(p->getMetadata());
    _out << ", " << name << "._enumerators)";

    registerName(name);

    _out.dec();
}

void
Slice::Python::CodeVisitor::visitConst(const ConstPtr& p)
{
    _out << sp << nl << getTypeReference(p) << " = ";
    writeConstantValue(p->type(), p->valueType(), p->value());
}

void
Slice::Python::CodeVisitor::registerName(const string& name)
{
    assert(!_moduleStack.empty());
    _out << sp << nl << "_M_" << _moduleStack.front() << '.' << name << " = " << name;
    _out << nl << "del " << name;
}

void
Slice::Python::CodeVisitor::writeType(const TypePtr& p)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(p);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindBool:
            {
                _out << "IcePy._t_bool";
                break;
            }
            case Builtin::KindByte:
            {
                _out << "IcePy._t_byte";
                break;
            }
            case Builtin::KindShort:
            {
                _out << "IcePy._t_short";
                break;
            }
            case Builtin::KindInt:
            {
                _out << "IcePy._t_int";
                break;
            }
            case Builtin::KindLong:
            {
                _out << "IcePy._t_long";
                break;
            }
            case Builtin::KindFloat:
            {
                _out << "IcePy._t_float";
                break;
            }
            case Builtin::KindDouble:
            {
                _out << "IcePy._t_double";
                break;
            }
            case Builtin::KindString:
            {
                _out << "IcePy._t_string";
                break;
            }
            case Builtin::KindObject:
            case Builtin::KindValue:
            {
                _out << "IcePy._t_Value";
                break;
            }
            case Builtin::KindObjectProxy:
            {
                _out << "IcePy._t_ObjectPrx";
                break;
            }
        }
        return;
    }

    InterfaceDeclPtr prx = dynamic_pointer_cast<InterfaceDecl>(p);
    if (prx)
    {
        _out << getMetaTypeReference(prx) + "Prx";
        return;
    }

    ContainedPtr cont = dynamic_pointer_cast<Contained>(p);
    assert(cont);
    _out << getMetaTypeReference(cont);
}

void
Slice::Python::CodeVisitor::writeInitializer(const DataMemberPtr& m)
{
    TypePtr p = m->type();
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(p);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindBool:
            {
                _out << "False";
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
            case Builtin::KindValue:
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            {
                _out << "None";
                break;
            }
        }
        return;
    }

    EnumPtr en = dynamic_pointer_cast<Enum>(p);
    if (en)
    {
        _out << getTypeReference(en->enumerators().front());
        return;
    }

    _out << "None";
}

void
Slice::Python::CodeVisitor::writeHash(const string& name, const TypePtr& p, int& iter)
{
    SequencePtr seq = dynamic_pointer_cast<Sequence>(p);
    if (seq)
    {
        _out << nl << "if " << name << ':';
        _out.inc();
        _out << nl << "for _i" << iter << " in " << name << ':';
        _out.inc();
        ostringstream elem;
        elem << "_i" << iter;
        iter++;
        writeHash(elem.str(), seq->type(), iter);
        _out.dec();
        _out.dec();
        return;
    }

    DictionaryPtr dict = dynamic_pointer_cast<Dictionary>(p);
    if (dict)
    {
        _out << nl << "if " << name << ':';
        _out.inc();
        _out << nl << "for _i" << iter << " in " << name << ':';
        _out.inc();
        ostringstream key;
        key << "_i" << iter;
        ostringstream value;
        value << name << '[' << key.str() << ']';
        iter++;
        writeHash(key.str(), dict->keyType(), iter);
        writeHash(value.str(), dict->valueType(), iter);
        _out.dec();
        _out.dec();
        return;
    }

    _out << nl << "_h = 5 * _h + _builtins.hash(" << name << ")";
}

void
Slice::Python::CodeVisitor::writeMetadata(const MetadataList& metadata)
{
    int i = 0;
    _out << '(';
    for (const auto& meta : metadata)
    {
        if (meta->directive().find("python:") == 0)
        {
            if (i > 0)
            {
                _out << ", ";
            }
            _out << "'" << *meta << "'";
            ++i;
        }
    }
    if (i == 1)
    {
        _out << ',';
    }
    _out << ')';
}

void
Slice::Python::CodeVisitor::writeAssign(const DataMemberPtr& member)
{
    const string memberName = member->mappedName();

    // Structures are treated differently (see bug 3676).
    StructPtr st = dynamic_pointer_cast<Struct>(member->type());
    if (st && !member->optional())
    {
        _out << nl << "self." << memberName << " = " << memberName << " if " << memberName << " is not None else "
             << getTypeReference(st) << "()";
    }
    else
    {
        _out << nl << "self." << memberName << " = " << memberName;
    }
}

void
Slice::Python::CodeVisitor::writeConstantValue(
    const TypePtr& type,
    const SyntaxTreeBasePtr& valueType,
    const string& value)
{
    ConstPtr constant = dynamic_pointer_cast<Const>(valueType);
    if (constant)
    {
        _out << getTypeReference(constant);
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
                {
                    _out << (value == "true" ? "True" : "False");
                    break;
                }
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
                    const string controlChars = "\a\b\f\n\r\t\v";
                    const unsigned char cutOff = 0;

                    _out << "\"" << toStringLiteral(value, controlChars, "", UCN, cutOff) << "\"";
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
            _out << getTypeReference(lte);
        }
        else
        {
            assert(false); // Unknown const type.
        }
    }
}

void
Slice::Python::CodeVisitor::writeConstructorParams(const DataMemberList& members)
{
    for (const auto& member : members)
    {
        // Function signatures always start with a 'self' parameter, so we always need a comma separator.
        _out << ", " << member->mappedName() << "=";
        if (member->defaultValue())
        {
            writeConstantValue(member->type(), member->defaultValueType(), *member->defaultValue());
        }
        else if (member->optional())
        {
            _out << "None";
        }
        else
        {
            writeInitializer(member);
        }
    }
}

string
Slice::Python::CodeVisitor::getOperationMode(Slice::Operation::Mode mode)
{
    string result;
    switch (mode)
    {
        case Operation::Normal:
            result = "Ice.OperationMode.Normal";
            break;
        case Operation::Idempotent:
            result = "Ice.OperationMode.Idempotent";
            break;
    }
    return result;
}

void
Slice::Python::CodeVisitor::writeDocstring(const optional<DocComment>& comment, const string& prefix)
{
    if (comment)
    {
        auto overview = comment->overview();
        if (!overview.empty())
        {
            _out << nl << prefix << tripleQuotes;
            for (const auto& line : overview)
            {
                _out << nl << line;
            }
            _out << nl << tripleQuotes;
        }
    }
}

void
Slice::Python::CodeVisitor::writeDocstring(const optional<DocComment>& comment, const DataMemberList& members)
{
    if (!comment)
    {
        return;
    }

    auto overview = comment->overview();

    // Collect docstrings (if any) for the members.
    map<string, list<string>> docs;
    for (const auto& member : members)
    {
        if (auto memberDoc = DocComment::parseFrom(member, pyLinkFormatter, true))
        {
            auto memberOverview = memberDoc->overview();
            if (!memberOverview.empty())
            {
                docs[member->name()] = memberOverview;
            }
        }
    }

    if (overview.empty() && docs.empty())
    {
        return;
    }

    _out << nl << tripleQuotes;

    for (const auto& line : overview)
    {
        _out << nl << line;
    }

    // Only emit members if there's a docstring for at least one member.
    if (!docs.empty())
    {
        if (!overview.empty())
        {
            _out << nl;
        }
        _out << nl << "Attributes";
        _out << nl << "----------";
        for (const auto& member : members)
        {
            _out << nl << member->mappedName() << " : " << typeToDocstring(member->type(), member->optional());
            auto p = docs.find(member->name());
            if (p != docs.end())
            {
                for (const auto& line : p->second)
                {
                    _out << nl << "    " << line;
                }
            }
        }
    }

    _out << nl << tripleQuotes;
}

void
Slice::Python::CodeVisitor::writeDocstring(const optional<DocComment>& comment, const EnumeratorList& enumerators)
{
    if (!comment)
    {
        return;
    }

    auto overview = comment->overview();

    // Collect docstrings (if any) for the enumerators.
    map<string, list<string>> docs;
    for (const auto& enumerator : enumerators)
    {
        if (auto enumeratorDoc = DocComment::parseFrom(enumerator, pyLinkFormatter, true))
        {
            auto enumeratorOverview = enumeratorDoc->overview();
            if (!enumeratorOverview.empty())
            {
                docs[enumerator->name()] = enumeratorOverview;
            }
        }
    }

    if (overview.empty() && docs.empty())
    {
        return;
    }

    _out << nl << tripleQuotes;

    for (const auto& line : overview)
    {
        _out << nl << line;
    }

    // Only emit enumerators if there's a docstring for at least one enumerator.
    if (!docs.empty())
    {
        if (!overview.empty())
        {
            _out << nl;
        }
        _out << nl << "Enumerators:";
        for (const auto& enumerator : enumerators)
        {
            _out << nl << enumerator->mappedName() << " -- ";
            auto p = docs.find(enumerator->name());
            if (p != docs.end())
            {
                for (auto q = p->second.begin(); q != p->second.end(); ++q)
                {
                    if (q != p->second.begin())
                    {
                        _out << nl;
                    }
                    _out << *q;
                }
            }
        }
    }

    _out << nl << tripleQuotes;
}

void
Slice::Python::CodeVisitor::writeDocstring(const OperationPtr& op, DocstringMode mode)
{
    optional<DocComment> comment = DocComment::parseFrom(op, pyLinkFormatter, true);
    if (!comment)
    {
        return;
    }

    TypePtr returnType = op->returnType();
    ParameterList params = op->parameters();
    ParameterList inParams = op->inParameters();
    ParameterList outParams = op->outParameters();

    auto overview = comment->overview();
    auto returnsDoc = comment->returns();
    auto parametersDoc = comment->parameters();
    auto exceptionsDoc = comment->exceptions();

    if (overview.empty())
    {
        if ((mode == DocSync || mode == DocDispatch) && parametersDoc.empty() && exceptionsDoc.empty() &&
            returnsDoc.empty())
        {
            return;
        }
        else if (mode == DocAsync && inParams.empty())
        {
            return;
        }
        else if (mode == DocDispatch && inParams.empty() && exceptionsDoc.empty())
        {
            return;
        }
    }

    //
    // Emit the general description.
    //
    _out << nl << tripleQuotes;
    for (const string& line : overview)
    {
        _out << nl << line;
    }

    //
    // Emit arguments.
    //
    bool needArgs = false;
    switch (mode)
    {
        case DocSync:
        case DocAsync:
        case DocDispatch:
            needArgs = true;
            break;
    }

    if (needArgs)
    {
        if (!overview.empty())
        {
            _out << nl;
        }

        _out << nl << "Parameters";
        _out << nl << "----------";
        for (const auto& param : inParams)
        {
            _out << nl << param->mappedName() << " : " << typeToDocstring(param->type(), param->optional());
            const auto r = parametersDoc.find(param->name());
            if (r != parametersDoc.end())
            {
                for (const auto& line : r->second)
                {
                    _out << nl << "    " << line;
                }
            }
        }

        if (mode == DocSync || mode == DocAsync)
        {
            const string contextParamName = getEscapedParamName(op, "context");
            _out << nl << contextParamName << " : dict[str, str]";
            _out << nl << "    The request context for the invocation.";
        }

        if (mode == DocDispatch)
        {
            const string currentParamName = getEscapedParamName(op, "current");
            _out << nl << currentParamName << " : Ice.Current";
            _out << nl << "    The Current object for the dispatch.";
        }
    }

    //
    // Emit return value(s).
    //
    bool hasReturnValue = false;
    if (!op->returnsAnyValues() && (mode == DocAsync || mode == DocDispatch))
    {
        hasReturnValue = true;
        if (!overview.empty() || needArgs)
        {
            _out << nl;
        }
        _out << nl << "Returns";
        _out << nl << "-------";
        _out << nl << docReturnType(op, mode);
        if (mode == DocAsync)
        {
            _out << nl << "    An awaitable that is completed when the invocation completes.";
        }
        else if (mode == DocDispatch)
        {
            _out << nl << "    None or an awaitable that completes when the dispatch completes.";
        }
    }
    else if (op->returnsAnyValues())
    {
        hasReturnValue = true;
        if (!overview.empty() || needArgs)
        {
            _out << nl;
        }
        _out << nl << "Returns";
        _out << nl << "-------";
        _out << nl << docReturnType(op, mode);

        if (op->returnsMultipleValues())
        {
            _out << nl;
            _out << nl << "    A tuple containing:";
            if (returnType)
            {
                _out << nl << "        - " << typeToDocstring(returnType, op->returnIsOptional());
                bool firstLine = true;
                for (const string& line : returnsDoc)
                {
                    if (firstLine)
                    {
                        firstLine = false;
                        _out << " " << line;
                    }
                    else
                    {
                        _out << nl << "          " << line;
                    }
                }
            }

            for (const auto& param : outParams)
            {
                _out << nl << "        - " << typeToDocstring(param->type(), param->optional());
                const auto r = parametersDoc.find(param->name());
                if (r != parametersDoc.end())
                {
                    bool firstLine = true;
                    for (const string& line : r->second)
                    {
                        if (firstLine)
                        {
                            firstLine = false;
                            _out << " " << line;
                        }
                        else
                        {
                            _out << nl << "          " << line;
                        }
                    }
                }
            }
        }
        else if (returnType)
        {
            for (const string& line : returnsDoc)
            {
                _out << nl << "    " << line;
            }
        }
        else if (!outParams.empty())
        {
            assert(outParams.size() == 1);
            const auto& param = outParams.front();
            _out << nl << typeToDocstring(param->type(), param->optional());
            const auto r = parametersDoc.find(param->name());
            if (r != parametersDoc.end())
            {
                for (const string& line : r->second)
                {
                    _out << nl << "    " << line;
                }
            }
        }
    }

    //
    // Emit exceptions.
    //
    if ((mode == DocSync || mode == DocDispatch) && !exceptionsDoc.empty())
    {
        if (!overview.empty() || needArgs || hasReturnValue)
        {
            _out << nl;
        }
        _out << nl << "Raises";
        _out << nl << "------";
        for (const auto& [exception, exceptionDescription] : exceptionsDoc)
        {
            _out << nl << exception;
            for (const auto& line : exceptionDescription)
            {
                _out << nl << "    " << line;
            }
        }
    }
    _out << nl << tripleQuotes;
}

string
Slice::Python::getPackageDirectory(const string& file, const UnitPtr& ut)
{
    // file must be a fully-qualified path name.

    // Check if the file contains the python:pkgdir file metadata.
    // If the metadata is present, then the generated file was placed in the specified directory.
    DefinitionContextPtr dc = ut->findDefinitionContext(file);
    assert(dc);
    return dc->getMetadataArgs("python:pkgdir").value_or("");
}

string
Slice::Python::getImportFileName(const string& file, const UnitPtr& ut, const vector<string>& includePaths)
{
    //
    // The file and includePaths arguments must be fully-qualified path names.
    //

    //
    // Check if the file contains the python:pkgdir file metadata.
    //
    string pkgdir = getPackageDirectory(file, ut);
    if (!pkgdir.empty())
    {
        //
        // The metadata is present, so the generated file was placed in the specified directory.
        //
        vector<string> names;
        IceInternal::splitString(pkgdir, "/", names);
        assert(!names.empty());
        pkgdir = "";
        for (auto p = names.begin(); p != names.end(); ++p)
        {
            if (p != names.begin())
            {
                pkgdir += ".";
            }
            pkgdir += *p;
        }
        string name = file;
        string::size_type pos = name.rfind('/');
        if (pos != string::npos)
        {
            name = name.substr(pos + 1); // Get the name of the file without the leading path.
        }
        assert(!name.empty());
        replace(name.begin(), name.end(), '.', '_'); // Convert .ice to _ice
        return pkgdir + "." + name;
    }
    else
    {
        //
        // The metadata is not present, so we transform the file name using the include paths (-I)
        // given to the compiler.
        //
        string name = changeInclude(file, includePaths);
        replace(name.begin(), name.end(), '/', '_');
        return name + "_ice";
    }
}

void
Slice::Python::generate(const UnitPtr& unit, bool all, const vector<string>& includePaths, Output& out)
{
    validateMetadata(unit);

    out << nl << "import Ice";
    out << nl << "import IcePy";
    out << nl << "import builtins as _builtins";

    if (!all)
    {
        vector<string> paths = includePaths;
        for (auto& path : paths)
        {
            path = fullPath(path);
        }

        StringList includes = unit->includeFiles();
        for (const auto& include : includes)
        {
            out << nl << "import " << getImportFileName(include, unit, paths);
        }
    }

    set<string> moduleHistory;

    ModuleVisitor moduleVisitor(out, moduleHistory);
    unit->visit(&moduleVisitor);

    CodeVisitor codeVisitor(out, moduleHistory);
    unit->visit(&codeVisitor);

    out << nl; // Trailing newline.
}

string
Slice::Python::getPackageMetadata(const ContainedPtr& cont)
{
    // Traverse to the top-level module.
    ContainedPtr p = cont;
    while (!p->isTopLevel())
    {
        p = dynamic_pointer_cast<Contained>(p->container());
        assert(p);
    }
    assert(dynamic_pointer_cast<Module>(p));

    // The python:package metadata can be defined as file metadata or applied to a top-level module.
    // We check for the metadata at the top-level module first and then fall back to the global scope.
    static const string directive = "python:package";
    if (auto packageMetadata = p->getMetadataArgs(directive))
    {
        return *packageMetadata;
    }

    string file = cont->file();
    DefinitionContextPtr dc = cont->unit()->findDefinitionContext(file);
    assert(dc);
    return dc->getMetadataArgs(directive).value_or("");
}

string
Slice::Python::getAbsolute(const ContainedPtr& p)
{
    const string package = getPackageMetadata(p);
    const string packagePrefix = package + (package.empty() ? "" : ".");
    return packagePrefix + p->mappedScoped(".").substr(1);
}

string
Slice::Python::getTypeReference(const ContainedPtr& p)
{
    return "_M_" + getAbsolute(p);
}

string
Slice::Python::getMetaTypeReference(const ContainedPtr& p)
{
    string absoluteName = getTypeReference(p);

    // Append a "_t_" in front of the last name segment.
    auto pos = absoluteName.rfind('.');
    pos = (pos == string::npos ? 0 : pos + 1);
    absoluteName.insert(pos, "_t_");

    return absoluteName;
}

void
Slice::Python::printHeader(IceInternal::Output& out)
{
    out << "# Copyright (c) ZeroC, Inc.";
    out << sp;
    out << nl << "# slice2py version " << ICE_STRING_VERSION;
}

void
Slice::Python::validateMetadata(const UnitPtr& unit)
{
    auto pythonArrayTypeValidationFunc = [](const MetadataPtr& m, const SyntaxTreeBasePtr& p) -> optional<string>
    {
        if (auto sequence = dynamic_pointer_cast<Sequence>(p))
        {
            BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(sequence->type());
            if (!builtin || !(builtin->isNumericType() || builtin->kind() == Builtin::KindBool))
            {
                return "the '" + m->directive() +
                       "' metadata can only be applied to sequences of bools, bytes, shorts, ints, longs, floats, "
                       "or doubles";
            }
        }
        return nullopt;
    };

    map<string, MetadataInfo> knownMetadata;

    // "python:<array-type>"
    MetadataInfo arrayTypeInfo = {
        .validOn = {typeid(Sequence)},
        .acceptedArgumentKind = MetadataArgumentKind::NoArguments,
        .acceptedContext = MetadataApplicationContext::DefinitionsAndTypeReferences,
        .extraValidation = pythonArrayTypeValidationFunc,
    };
    knownMetadata.emplace("python:array.array", arrayTypeInfo);
    knownMetadata.emplace("python:numpy.ndarray", std::move(arrayTypeInfo));

    // "python:identifier"
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
    knownMetadata.emplace("python:identifier", std::move(identifierInfo));

    // "python:memoryview"
    MetadataInfo memoryViewInfo = {
        .validOn = {typeid(Sequence)},
        .acceptedArgumentKind = MetadataArgumentKind::RequiredTextArgument,
        .acceptedContext = MetadataApplicationContext::DefinitionsAndTypeReferences,
        .extraValidation = pythonArrayTypeValidationFunc,
    };
    knownMetadata.emplace("python:memoryview", std::move(memoryViewInfo));

    // "python:package"
    MetadataInfo packageInfo = {
        .validOn = {typeid(Module), typeid(Unit)},
        .acceptedArgumentKind = MetadataArgumentKind::SingleArgument,
        .extraValidation = [](const MetadataPtr& metadata, const SyntaxTreeBasePtr& p) -> optional<string>
        {
            const string msg = "'python:package' is deprecated; use 'python:identifier' to remap modules instead";
            p->unit()->warning(metadata->file(), metadata->line(), Deprecated, msg);

            if (auto cont = dynamic_pointer_cast<Contained>(p); cont && cont->hasMetadata("python:identifier"))
            {
                return "A Slice element can only have one of 'python:package' and 'python:identifier' applied to it";
            }

            // If 'python:package' is applied to a module, it must be a top-level module.
            // Top-level modules are contained by the 'Unit'. Non-top-level modules are contained in 'Module's.
            if (auto mod = dynamic_pointer_cast<Module>(p); mod && !mod->isTopLevel())
            {
                return "the 'python:package' metadata can only be applied at the file level or to top-level modules";
            }
            return nullopt;
        },
    };
    knownMetadata.emplace("python:package", std::move(packageInfo));

    // "python:pkgdir"
    MetadataInfo pkgdirInfo = {
        .validOn = {typeid(Unit)},
        .acceptedArgumentKind = MetadataArgumentKind::RequiredTextArgument,
    };
    knownMetadata.emplace("python:pkgdir", std::move(pkgdirInfo));

    // "python:seq"
    // We support 3 arguments to this metadata: "default", "list", and "tuple".
    // We also allow users to omit the "seq" in the middle, ie. "python:seq:list" and "python:list" are equivalent.
    MetadataInfo seqInfo = {
        .validOn = {typeid(Sequence)},
        .acceptedArgumentKind = MetadataArgumentKind::SingleArgument,
        .validArgumentValues = {{"default", "list", "tuple"}},
        .acceptedContext = MetadataApplicationContext::DefinitionsAndTypeReferences,
    };
    knownMetadata.emplace("python:seq", std::move(seqInfo));
    MetadataInfo unqualifiedSeqInfo = {
        .validOn = {typeid(Sequence)},
        .acceptedArgumentKind = MetadataArgumentKind::NoArguments,
        .acceptedContext = MetadataApplicationContext::DefinitionsAndTypeReferences,
    };
    knownMetadata.emplace("python:default", unqualifiedSeqInfo);
    knownMetadata.emplace("python:list", unqualifiedSeqInfo);
    knownMetadata.emplace("python:tuple", std::move(unqualifiedSeqInfo));

    // Pass this information off to the parser's metadata validation logic.
    Slice::validateMetadata(unit, "python", std::move(knownMetadata));
}
