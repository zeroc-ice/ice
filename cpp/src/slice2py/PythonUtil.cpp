// Copyright (c) ZeroC, Inc.

#include "PythonUtil.h"
#include "../Slice/Util.h"
#include "Ice/StringUtil.h"

#include <algorithm>
#include <cassert>
#include <climits>
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

    const char* const tripleQuotes = "\"\"\"";

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
            return Slice::Python::scopedToName(cl->scoped());
        }

        StructPtr st = dynamic_pointer_cast<Struct>(type);
        if (st)
        {
            return Slice::Python::scopedToName(st->scoped());
        }

        InterfaceDeclPtr proxy = dynamic_pointer_cast<InterfaceDecl>(type);
        if (proxy)
        {
            ostringstream os;
            os << "(";
            os << Slice::Python::scopedToName(proxy->scoped() + "Prx");
            os << " or None)";
            return os.str();
        }

        EnumPtr en = dynamic_pointer_cast<Enum>(type);
        if (en)
        {
            return Slice::Python::scopedToName(en->scoped());
        }

        SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
        if (seq)
        {
            return typeToDocstring(seq->type(), false) + "[]";
        }

        DictionaryPtr dict = dynamic_pointer_cast<Dictionary>(type);
        if (dict)
        {
            ostringstream os;
            os << "dict where keys are " << typeToDocstring(dict->keyType(), false) << " and values are "
               << typeToDocstring(dict->valueType(), false);
            return os.str();
        }

        return "???";
    }

    /// Returns a DocString formatted link to the provided Slice identifier.
    string pyLinkFormatter(string identifier, string memberComponent)
    {
        ostringstream os;
        os << "`";
        if (!identifier.empty())
        {
            os << Slice::Python::fixIdent(identifier);
            if (!memberComponent.empty())
            {
                os << ".";
            }
        }
        os << Slice::Python::fixIdent(memberComponent);
        os << "`";
        return os.str();
    }
}

namespace Slice
{
    namespace Python
    {
        class MetadataVisitor final : public ParserVisitor
        {
        public:
            bool visitUnitStart(const UnitPtr&) final;
            bool visitModuleStart(const ModulePtr&) final;
            void visitClassDecl(const ClassDeclPtr&) final;
            void visitInterfaceDecl(const InterfaceDeclPtr&) final;
            bool visitExceptionStart(const ExceptionPtr&) final;
            bool visitStructStart(const StructPtr&) final;
            void visitOperation(const OperationPtr&) final;
            void visitDataMember(const DataMemberPtr&) final;
            void visitSequence(const SequencePtr&) final;
            void visitDictionary(const DictionaryPtr&) final;
            void visitEnum(const EnumPtr&) final;
            void visitConst(const ConstPtr&) final;

        private:
            /// Validates sequence metadata.
            MetadataList validateSequence(const ContainedPtr&, const TypePtr&);

            /// Checks a definition that doesn't currently support Python metadata.
            void reject(const ContainedPtr&);
        };

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
            // Return a Python symbol for the given parser element.
            //
            string getSymbol(const ContainedPtr&, const string& = "", const string& = "");

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

            struct MemberInfo
            {
                string fixedName;
                bool inherited;
                DataMemberPtr dataMember;
            };
            using MemberInfoList = list<MemberInfo>;

            //
            // Write a member assignment statement for a constructor.
            //
            void writeAssign(const MemberInfo&);

            //
            // Write a constant value.
            //
            void writeConstantValue(const TypePtr&, const SyntaxTreeBasePtr&, const string&);

            //
            // Write constructor parameters with default values.
            //
            void writeConstructorParams(const MemberInfoList&);

            void collectClassMembers(const ClassDefPtr&, MemberInfoList&, bool);
            void collectExceptionMembers(const ExceptionPtr&, MemberInfoList&, bool);

            void writeDocstring(const DocCommentPtr&, const string& = "");
            void writeDocstring(const DocCommentPtr&, const DataMemberList&);
            void writeDocstring(const DocCommentPtr&, const EnumeratorList&);

            enum DocstringMode
            {
                DocSync,
                DocAsync,
                DocDispatch,
                DocAsyncDispatch
            };

            void writeDocstring(const OperationPtr&, DocstringMode);

            Output& _out;
            set<string>& _moduleHistory;
            list<string> _moduleStack;
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
    static const string keywordList[] = {
        "False", "None",     "True",  "and",    "as",     "assert", "async",  "await", "break",    "case",
        "class", "continue", "def",   "del",    "elif",   "else",   "except", "exec",  "finally",  "for",
        "from",  "global",   "if",    "import", "in",     "is",     "lambda", "match", "nonlocal", "not",
        "or",    "pass",     "print", "raise",  "return", "try",    "type",   "while", "with",     "yield"};
    bool found = binary_search(&keywordList[0], &keywordList[sizeof(keywordList) / sizeof(*keywordList)], name);
    return found ? "_" + name : name;
}

static string
getDictLookup(const ContainedPtr& cont, const string& suffix = "", const string& prefix = "")
{
    string scope = Slice::Python::scopedToName(cont->scope());
    assert(!scope.empty());

    string package = Slice::Python::getPackageMetadata(cont);
    if (!package.empty())
    {
        scope = package + "." + scope;
    }

    return "'" + suffix + Slice::Python::fixIdent(cont->name() + prefix) + "' not in _M_" + scope + "__dict__";
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
            //
            // If this is a top-level module, then we check if it has package metadata.
            // If so, we need to emit statements to open each of the modules in the
            // package before we can open this module.
            //
            if (dynamic_pointer_cast<Unit>(p->container()))
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
        //
        // If this is a top-level module, then we check if it has package metadata.
        // If so, we need to emit statements to open each of the modules in the
        // package before we can open this module.
        //
        if (dynamic_pointer_cast<Unit>(p->container()))
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

    writeDocstring(p->parseDocComment(pyLinkFormatter, true), "_M_" + abs + ".__doc__ = ");

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
    //
    // Emit forward declarations.
    //
    string scoped = p->scoped();
    if (_classHistory.count(scoped) == 0)
    {
        _out << sp << nl << "if " << getDictLookup(p) << ':';
        _out.inc();
        _out << nl << "_M_" << getAbsolute(p, "_t_") << " = IcePy.declareValue('" << scoped << "')";
        _out.dec();
        _classHistory.insert(scoped); // Avoid redundant declarations.
    }
}

void
Slice::Python::CodeVisitor::visitInterfaceDecl(const InterfaceDeclPtr& p)
{
    //
    // Emit forward declarations.
    //
    string scoped = p->scoped();
    if (_classHistory.count(scoped) == 0)
    {
        _out << sp << nl << "if " << getDictLookup(p) << ':';
        _out.inc();
        _out << nl << "_M_" << getAbsolute(p, "_t_", "Prx") << " = IcePy.declareProxy('" << scoped << "')";
        _out.dec();
        _classHistory.insert(scoped); // Avoid redundant declarations.
    }
}

void
Slice::Python::CodeVisitor::writeOperations(const InterfaceDefPtr& p)
{
    OperationList operations = p->operations();
    //
    // Emit a placeholder for each operation.
    //
    for (const auto& operation : operations)
    {
        string fixedOpName = fixIdent(operation->name());

        if (operation->hasMarshaledResult())
        {
            string name = operation->name();
            name[0] = static_cast<char>(toupper(static_cast<unsigned char>(name[0])));
            _out << sp;
            _out << nl << "@staticmethod";
            _out << nl << "def " << name << "MarshaledResult(result, current):";
            _out.inc();
            _out << nl << tripleQuotes;
            _out << nl << "Immediately marshals the result of an invocation of " << name;
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
            _out << nl << "return IcePy.MarshaledResult(result, _M_" << getAbsolute(p) << "._op_" << fixedOpName
                 << ", current.adapter.getCommunicator()._getImpl(), current.encoding)";
            _out.dec();
        }

        _out << sp << nl << "def " << fixedOpName << "(self";

        for (const auto& param : operation->parameters())
        {
            if (!param->isOutParam())
            {
                _out << ", " << fixIdent(param->name());
            }
        }

        const string currentParamName = getEscapedParamName(operation, "current");
        _out << ", " << currentParamName << "=None";
        _out << "):";
        _out.inc();

        writeDocstring(operation, DocAsyncDispatch);

        _out << nl << "raise NotImplementedError(\"servant method '" << fixedOpName << "' not implemented\")";
        _out.dec();
    }
}

bool
Slice::Python::CodeVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string scoped = p->scoped();
    string type = getAbsolute(p, "_t_");
    string abs = getAbsolute(p);
    string valueName = fixIdent(p->name());
    ClassDefPtr base = p->base();

    _out << sp << nl << "if " << getDictLookup(p) << ':';
    _out.inc();
    _out << nl << "_M_" << abs << " = None";
    _out << nl << "class " << valueName << '(';
    if (!base)
    {
        _out << "Ice.Value";
    }
    else
    {
        _out << getSymbol(base);
    }
    _out << "):";

    _out.inc();

    writeDocstring(p->parseDocComment(pyLinkFormatter, true), p->dataMembers());

    //
    // __init__
    //
    _out << nl << "def __init__(self";
    MemberInfoList allMembers;
    collectClassMembers(p, allMembers, false);
    writeConstructorParams(allMembers);
    _out << "):";
    _out.inc();
    if (!base && p->dataMembers().empty())
    {
        _out << nl << "pass";
    }
    else
    {
        if (base)
        {
            _out << nl << getSymbol(base) << ".__init__(self";
            for (const auto& allMember : allMembers)
            {
                if (allMember.inherited)
                {
                    _out << ", " << allMember.fixedName;
                }
            }
            _out << ')';
        }
        for (const auto& allMember : allMembers)
        {
            if (!allMember.inherited)
            {
                writeAssign(allMember);
            }
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

    //
    // __str__
    //
    _out << sp << nl << "def __str__(self):";
    _out.inc();
    _out << nl << "return IcePy.stringify(self, _M_" << type << ")";
    _out.dec();
    _out << sp << nl << "__repr__ = __str__";

    _out.dec();

    DataMemberList members = p->dataMembers();
    _out << sp << nl << "_M_" << type << " = IcePy.defineValue('" << scoped << "', " << valueName << ", "
         << p->compactId() << ", ";
    writeMetadata(p->getMetadata());
    _out << ", False, ";
    if (!base)
    {
        _out << "None";
    }
    else
    {
        _out << "_M_" << getAbsolute(base, "_t_");
    }
    _out << ", (";
    //
    // Members
    //
    // Data members are represented as a tuple:
    //
    //   ('MemberName', MemberMetadata, MemberType, Optional, Tag)
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a constructed type.
    //
    if (members.size() > 1)
    {
        _out.inc();
        _out << nl;
    }
    bool isProtected = p->hasMetadata("protected");
    for (auto r = members.begin(); r != members.end(); ++r)
    {
        if (r != members.begin())
        {
            _out << ',' << nl;
        }
        _out << "('";
        if (isProtected || (*r)->hasMetadata("protected"))
        {
            _out << '_';
        }
        _out << fixIdent((*r)->name()) << "', ";
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
    _out << nl << valueName << "._ice_type = _M_" << type;

    registerName(valueName);

    _out.dec();

    return false;
}

bool
Slice::Python::CodeVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    string scoped = p->scoped();
    string classType = getAbsolute(p, "_t_", "Disp");
    string abs = getAbsolute(p);
    string className = fixIdent(p->name());
    string classAbs = getAbsolute(p);
    string prxAbs = getAbsolute(p, "", "Prx");
    string prxName = fixIdent(p->name() + "Prx");
    string prxType = getAbsolute(p, "_t_", "Prx");
    InterfaceList bases = p->bases();

    _out << sp << nl << "if " << getDictLookup(p, "", "Prx") << ':';
    _out.inc();

    // Define the proxy class
    _out << nl << "_M_" << prxAbs << " = None";
    _out << nl << "class " << prxName << '(';

    {
        vector<string> baseClasses;
        for (const auto& base : bases)
        {
            InterfaceDefPtr d = base;
            baseClasses.push_back(getSymbol(base, "", "Prx"));
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
        string fixedOpName = fixIdent(operation->name());
        if (fixedOpName == "checkedCast" || fixedOpName == "uncheckedCast")
        {
            fixedOpName.insert(0, "_");
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
                string param = fixIdent(q->name());
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
        _out << nl << "def " << fixedOpName << "(self";
        if (!inParamsDecl.empty())
        {
            _out << ", " << inParamsDecl;
        }
        const string contextParamName = getEscapedParamName(operation, "context");
        _out << ", " << contextParamName << "=None):";
        _out.inc();
        writeDocstring(operation, DocSync);
        _out << nl << "return _M_" << classAbs << "._op_" << operation->name() << ".invoke(self, ((" << inParams;
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
        _out << nl << "def " << operation->name() << "Async(self";
        if (!inParams.empty())
        {
            _out << ", " << inParams;
        }
        _out << ", " << contextParamName << "=None):";
        _out.inc();
        writeDocstring(operation, DocAsync);
        _out << nl << "return _M_" << classAbs << "._op_" << operation->name() << ".invokeAsync(self, ((" << inParams;
        if (!inParams.empty() && inParams.find(',') == string::npos)
        {
            _out << ", ";
        }
        _out << "), " << contextParamName << "))";
        _out.dec();
    }

    _out << sp << nl << "@staticmethod";
    _out << nl << "def checkedCast(proxy, facetOrContext=None, context=None):";
    _out.inc();
    _out << nl << "return _M_" << prxAbs << ".ice_checkedCast(proxy, '" << scoped << "', facetOrContext, context)";
    _out.dec();

    _out << sp << nl << "@staticmethod";
    _out << nl << "def uncheckedCast(proxy, facet=None):";
    _out.inc();
    _out << nl << "return _M_" << prxAbs << ".ice_uncheckedCast(proxy, facet)";
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

    _out << nl << "_M_" << prxType << " = IcePy.defineProxy('" << scoped << "', " << prxName << ")";

    registerName(prxName);

    // Define the servant class
    _out << sp << nl << "_M_" << classAbs << " = None";
    _out << nl << "class " << className << '(';
    {
        vector<string> baseClasses;
        for (const auto& base : bases)
        {
            InterfaceDefPtr d = base;
            baseClasses.push_back(getSymbol(base, "", ""));
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
    _out << sp << nl << "def ice_ids(self, current=None):";
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
    _out << sp << nl << "def ice_id(self, current=None):";
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

    //
    // __str__
    //
    _out << sp << nl << "def __str__(self):";
    _out.inc();
    _out << nl << "return IcePy.stringify(self, _M_" << classType << ")";
    _out.dec();
    _out << sp << nl << "__repr__ = __str__";

    _out.dec();

    //
    // Define each operation. The arguments to the IcePy.Operation constructor are:
    //
    // 'opName', Mode, AMD, Format, Metadata, (InParams), (OutParams), ReturnParam, (Exceptions)
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

        _out << nl << className << "._op_" << operation->name() << " = IcePy.Operation('" << operation->name() << "', "
             << getOperationMode(operation->mode()) << ", "
             << ((p->hasMetadata("amd") || operation->hasMetadata("amd")) ? "True" : "False") << ", " << format << ", ";
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
            _out << "_M_" << getAbsolute(*u, "_t_");
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
            _out << nl << className << "._op_" << operation->name() << ".deprecate(\"" << reason << "\")";
        }
    }

    registerName(className);
    _out.dec();

    return false;
}

bool
Slice::Python::CodeVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string scoped = p->scoped();
    string abs = getAbsolute(p);
    string name = fixIdent(p->name());

    _out << sp << nl << "if " << getDictLookup(p) << ':';
    _out.inc();
    _out << nl << "_M_" << abs << " = None";
    _out << nl << "class " << name << '(';
    ExceptionPtr base = p->base();
    string baseName;
    if (base)
    {
        baseName = getSymbol(base);
        _out << baseName;
    }
    else
    {
        _out << "Ice.UserException";
    }
    _out << "):";
    _out.inc();

    DataMemberList members = p->dataMembers();

    writeDocstring(p->parseDocComment(pyLinkFormatter, true), members);

    //
    // __init__
    //
    _out << nl << "def __init__(self";
    MemberInfoList allMembers;
    collectExceptionMembers(p, allMembers, false);
    writeConstructorParams(allMembers);
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
            for (const auto& allMember : allMembers)
            {
                if (allMember.inherited)
                {
                    _out << ", " << allMember.fixedName;
                }
            }
            _out << ')';
        }
        for (const auto& allMember : allMembers)
        {
            if (!allMember.inherited)
            {
                writeAssign(allMember);
            }
        }
    }
    _out.dec();

    //
    // __str__
    //
    _out << sp << nl << "def __str__(self):";
    _out.inc();
    _out << nl << "return IcePy.stringifyException(self)";
    _out.dec();
    _out << sp << nl << "__repr__ = __str__";

    //
    // _ice_id
    //
    _out << sp << nl << "_ice_id = '" << scoped << "'";

    _out.dec();

    //
    // Emit the type information.
    //
    string type = getAbsolute(p, "_t_");
    _out << sp << nl << "_M_" << type << " = IcePy.defineException('" << scoped << "', " << name << ", ";
    writeMetadata(p->getMetadata());
    _out << ", ";
    if (!base)
    {
        _out << "None";
    }
    else
    {
        _out << "_M_" << getAbsolute(base, "_t_");
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
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a constructed type.
    //
    for (auto dmli = members.begin(); dmli != members.end(); ++dmli)
    {
        if (dmli != members.begin())
        {
            _out << ',' << nl;
        }
        _out << "('" << fixIdent((*dmli)->name()) << "', ";
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
    _out << nl << name << "._ice_type = _M_" << type;

    registerName(name);

    _out.dec();

    return false;
}

bool
Slice::Python::CodeVisitor::visitStructStart(const StructPtr& p)
{
    string scoped = p->scoped();
    string abs = getAbsolute(p);
    string name = fixIdent(p->name());
    DataMemberList members = p->dataMembers();
    MemberInfoList memberList;

    {
        for (const auto& member : members)
        {
            memberList.emplace_back();
            memberList.back().fixedName = fixIdent(member->name());
            memberList.back().inherited = false;
            memberList.back().dataMember = member;
        }
    }

    _out << sp << nl << "if " << getDictLookup(p) << ':';
    _out.inc();
    _out << nl << "_M_" << abs << " = None";
    _out << nl << "class " << name << "(object):";
    _out.inc();

    writeDocstring(p->parseDocComment(pyLinkFormatter, true), members);

    _out << nl << "def __init__(self";
    writeConstructorParams(memberList);
    _out << "):";
    _out.inc();
    for (const auto& r : memberList)
    {
        writeAssign(r);
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
        for (const auto& r : memberList)
        {
            string s = "self." + r.fixedName;
            writeHash(s, r.dataMember->type(), iter);
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
        _out << nl << "elif not isinstance(other, _M_" << abs << "):";
        _out.inc();
        _out << nl << "return NotImplemented";
        _out.dec();
        _out << nl << "else:";
        _out.inc();
        for (const auto& r : memberList)
        {
            //
            // The None value is not orderable in Python 3.
            //
            _out << nl << "if self." << r.fixedName << " is None or other." << r.fixedName << " is None:";
            _out.inc();
            _out << nl << "if self." << r.fixedName << " != other." << r.fixedName << ':';
            _out.inc();
            _out << nl << "return (-1 if self." << r.fixedName << " is None else 1)";
            _out.dec();
            _out.dec();
            _out << nl << "else:";
            _out.inc();
            _out << nl << "if self." << r.fixedName << " < other." << r.fixedName << ':';
            _out.inc();
            _out << nl << "return -1";
            _out.dec();
            _out << nl << "elif self." << r.fixedName << " > other." << r.fixedName << ':';
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
        _out << nl << "elif not isinstance(other, _M_" << abs << "):";
        _out.inc();
        _out << nl << "return NotImplemented";
        _out.dec();
        _out << nl << "else:";
        _out.inc();
        for (const auto& r : memberList)
        {
            //
            // The None value is not orderable in Python 3.
            //
            _out << nl << "if self." << r.fixedName << " != other." << r.fixedName << ':';
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

    //
    // __str__
    //
    _out << sp << nl << "def __str__(self):";
    _out.inc();
    _out << nl << "return IcePy.stringify(self, _M_" << getAbsolute(p, "_t_") << ")";
    _out.dec();
    _out << sp << nl << "__repr__ = __str__";

    _out.dec();

    //
    // Emit the type information.
    //
    _out << sp << nl << "_M_" << getAbsolute(p, "_t_") << " = IcePy.defineStruct('" << scoped << "', " << name << ", ";
    writeMetadata(p->getMetadata());
    _out << ", (";
    //
    // Data members are represented as a tuple:
    //
    //   ('MemberName', MemberMetadata, MemberType)
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a constructed type.
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
        _out << "('" << r->fixedName << "', ";
        writeMetadata(r->dataMember->getMetadata());
        _out << ", ";
        writeType(r->dataMember->type());
        _out << ')';
    }
    if (memberList.size() == 1)
    {
        _out << ',';
    }
    else if (memberList.size() > 1)
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
    string scoped = p->scoped();
    _out << sp << nl << "if " << getDictLookup(p, "_t_") << ':';
    _out.inc();
    _out << nl << "_M_" << getAbsolute(p, "_t_") << " = IcePy.defineSequence('" << scoped << "', ";
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
    string scoped = p->scoped();
    _out << sp << nl << "if " << getDictLookup(p, "_t_") << ':';
    _out.inc();
    _out << nl << "_M_" << getAbsolute(p, "_t_") << " = IcePy.defineDictionary('" << scoped << "', ";
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
    string abs = getAbsolute(p);
    string name = fixIdent(p->name());
    EnumeratorList enumerators = p->enumerators();

    _out << sp << nl << "if " << getDictLookup(p) << ':';
    _out.inc();
    _out << nl << "_M_" << abs << " = None";
    _out << nl << "class " << name << "(Ice.EnumBase):";
    _out.inc();

    writeDocstring(p->parseDocComment(pyLinkFormatter, true), enumerators);

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

    _out.dec();

    _out << sp;
    for (const auto& enumerator : enumerators)
    {
        string fixedEnum = fixIdent(enumerator->name());
        _out << nl << name << '.' << fixedEnum << " = " << name << "(\"" << enumerator->name() << "\", "
             << enumerator->value() << ')';
    }
    _out << nl << name << "._enumerators = { ";
    for (auto q = enumerators.begin(); q != enumerators.end(); ++q)
    {
        if (q != enumerators.begin())
        {
            _out << ", ";
        }
        string fixedEnum = fixIdent((*q)->name());
        _out << (*q)->value() << ':' << name << '.' << fixedEnum;
    }
    _out << " }";

    //
    // Emit the type information.
    //
    _out << sp << nl << "_M_" << getAbsolute(p, "_t_") << " = IcePy.defineEnum('" << scoped << "', " << name << ", ";
    writeMetadata(p->getMetadata());
    _out << ", " << name << "._enumerators)";

    registerName(name);

    _out.dec();
}

void
Slice::Python::CodeVisitor::visitConst(const ConstPtr& p)
{
    Slice::TypePtr type = p->type();
    string name = fixIdent(p->name());

    _out << sp << nl << "_M_" << getAbsolute(p) << " = ";
    writeConstantValue(type, p->valueType(), p->value());
}

string
Slice::Python::CodeVisitor::getSymbol(const ContainedPtr& p, const string& prefix, const string& suffix)
{
    //
    // An explicit reference to another type must always be prefixed with "_M_".
    //
    return "_M_" + getAbsolute(p, prefix, suffix);
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
        _out << "_M_" << getAbsolute(prx, "_t_", "Prx");
        return;
    }

    ContainedPtr cont = dynamic_pointer_cast<Contained>(p);
    assert(cont);
    _out << "_M_" << getAbsolute(cont, "_t_");
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
        string firstEnumerator = en->enumerators().front()->name();
        _out << getSymbol(en) << "." << fixIdent(firstEnumerator);
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
Slice::Python::CodeVisitor::writeAssign(const MemberInfo& info)
{
    string paramName = info.fixedName;
    string memberName = info.fixedName;

    //
    // Structures are treated differently (see bug 3676).
    //
    StructPtr st = dynamic_pointer_cast<Struct>(info.dataMember->type());
    if (st && !info.dataMember->optional())
    {
        _out << nl << "self." << memberName << " = " << paramName << " if " << paramName << " is not None else "
             << getSymbol(st) << "()";
    }
    else
    {
        _out << nl << "self." << memberName << " = " << paramName;
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
        _out << "_M_" << getAbsolute(constant);
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
            _out << getSymbol(lte);
        }
        else
        {
            assert(false); // Unknown const type.
        }
    }
}

void
Slice::Python::CodeVisitor::writeConstructorParams(const MemberInfoList& members)
{
    for (const auto& p : members)
    {
        _out << ", " << p.fixedName << "=";

        const DataMemberPtr member = p.dataMember;
        if (member->defaultValueType())
        {
            writeConstantValue(member->type(), member->defaultValueType(), member->defaultValue());
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
Slice::Python::CodeVisitor::collectClassMembers(const ClassDefPtr& p, MemberInfoList& allMembers, bool inherited)
{
    ClassDefPtr base = p->base();
    if (base)
    {
        collectClassMembers(base, allMembers, true);
    }

    for (const auto& member : p->dataMembers())
    {
        MemberInfo m;
        if (p->hasMetadata("protected") || member->hasMetadata("protected"))
        {
            m.fixedName = "_" + fixIdent(member->name());
        }
        else
        {
            m.fixedName = fixIdent(member->name());
        }
        m.inherited = inherited;
        m.dataMember = member;
        allMembers.push_back(m);
    }
}

void
Slice::Python::CodeVisitor::collectExceptionMembers(const ExceptionPtr& p, MemberInfoList& allMembers, bool inherited)
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
        m.fixedName = fixIdent(member->name());
        m.inherited = inherited;
        m.dataMember = member;
        allMembers.push_back(m);
    }
}

void
Slice::Python::CodeVisitor::writeDocstring(const DocCommentPtr& comment, const string& prefix)
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
Slice::Python::CodeVisitor::writeDocstring(const DocCommentPtr& comment, const DataMemberList& members)
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
        auto memeberDoc = member->parseDocComment(pyLinkFormatter, true);
        auto memberOverview = memeberDoc ? memeberDoc->overview() : StringList{};
        if (!memberOverview.empty())
        {
            docs[member->name()] = memberOverview;
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
            _out << nl << fixIdent(member->name()) << " : " << typeToDocstring(member->type(), member->optional());
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
Slice::Python::CodeVisitor::writeDocstring(const DocCommentPtr& comment, const EnumeratorList& enumerators)
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
        auto enumeratorDoc = enumerator->parseDocComment(pyLinkFormatter, true);
        auto enumeratorOverview = enumeratorDoc ? enumeratorDoc->overview() : StringList{};
        if (!enumeratorOverview.empty())
        {
            docs[enumerator->name()] = enumeratorOverview;
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
            _out << nl << fixIdent(enumerator->name()) << " -- ";
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
    DocCommentPtr comment = op->parseDocComment(pyLinkFormatter, true);
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
        else if (mode == DocAsyncDispatch && inParams.empty() && exceptionsDoc.empty())
        {
            return;
        }
    }

    //
    // Emit the general description.
    //
    _out << nl << "\"\"\"";
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
        case DocAsyncDispatch:
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
            string fixed = fixIdent(param->name());
            _out << nl << fixed << " : " << typeToDocstring(param->type(), param->optional());
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
            _out << nl << contextParamName << " : Ice.Context";
            _out << nl << "    The request context for the invocation.";
        }

        if (mode == DocDispatch || mode == DocAsyncDispatch)
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
    if (mode == DocAsync || mode == DocAsyncDispatch)
    {
        hasReturnValue = true;
        if (!overview.empty() || needArgs)
        {
            _out << nl;
        }
        _out << nl << "Returns";
        _out << nl << "-------";
        _out << nl << "Ice.Future";
        _out << nl << "    A future object that is completed with the result of "
             << (mode == DocAsync ? "the invocation." : "the dispatch.");
    }

    if ((mode == DocSync || mode == DocDispatch) && (returnType || !outParams.empty()))
    {
        hasReturnValue = true;
        if (!overview.empty() || needArgs)
        {
            _out << nl;
        }
        _out << nl << "Returns";
        _out << nl << "-------";
        if ((outParams.size() + (returnType ? 1 : 0)) > 1)
        {
            _out << nl << "Returns a tuple of (";
            if (returnType)
            {
                _out << typeToDocstring(returnType, op->returnIsOptional());
                _out << ", ";
            }

            for (const auto& param : outParams)
            {
                _out << typeToDocstring(param->type(), param->optional());
                if (param != outParams.back())
                {
                    _out << ", ";
                }
            }
            _out << ")";

            _out << nl << "    A tuple containing:";
            if (returnType)
            {
                _out << nl << "    - " << typeToDocstring(returnType, op->returnIsOptional());
                for (const string& line : returnsDoc)
                {
                    _out << nl << "        " << line;
                }
            }

            for (const auto& param : outParams)
            {
                _out << nl << "    - " << typeToDocstring(param->type(), param->optional());
                const auto r = parametersDoc.find(param->name());
                if (r != parametersDoc.end())
                {
                    for (const string& line : r->second)
                    {
                        _out << nl << "        " << line;
                    }
                }
            }
        }
        else if (returnType)
        {
            _out << nl << typeToDocstring(returnType, op->returnIsOptional());
            for (const string& line : returnsDoc)
            {
                _out << nl << "    " << line;
            }
        }
        else if (!outParams.empty())
        {
            assert(outParams.size() == 1);
            auto param = outParams.front();
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
    if ((mode == DocSync || mode == DocDispatch || mode == DocAsyncDispatch) && !exceptionsDoc.empty())
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
    _out << nl << "\"\"\"";
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
            pkgdir += fixIdent(*p);
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
Slice::Python::generate(const UnitPtr& un, bool all, const vector<string>& includePaths, Output& out)
{
    Slice::Python::MetadataVisitor visitor;
    un->visit(&visitor);

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

        StringList includes = un->includeFiles();
        for (const auto& include : includes)
        {
            out << nl << "import " << getImportFileName(include, un, paths);
        }
    }

    set<string> moduleHistory;

    ModuleVisitor moduleVisitor(out, moduleHistory);
    un->visit(&moduleVisitor);

    CodeVisitor codeVisitor(out, moduleHistory);
    un->visit(&codeVisitor);

    out << nl; // Trailing newline.
}

string
Slice::Python::scopedToName(const string& scoped)
{
    string result = fixIdent(scoped);
    if (result.find("::") == 0)
    {
        result.erase(0, 2);
    }

    string::size_type pos;
    while ((pos = result.find("::")) != string::npos)
    {
        result.replace(pos, 2, ".");
    }

    return result;
}

string
Slice::Python::fixIdent(const string& ident)
{
    if (ident[0] != ':')
    {
        return lookupKwd(ident);
    }
    vector<string> ids = splitScopedName(ident);
    transform(ids.begin(), ids.end(), ids.begin(), [](const string& id) -> string { return lookupKwd(id); });
    stringstream result;
    for (const auto& id : ids)
    {
        result << "::" + id;
    }
    return result.str();
}

string
Slice::Python::getPackageMetadata(const ContainedPtr& cont)
{
    //
    // Traverse to the top-level module.
    //
    ModulePtr m;
    ContainedPtr p = cont;
    while (true)
    {
        if (dynamic_pointer_cast<Module>(p))
        {
            m = dynamic_pointer_cast<Module>(p);
        }

        ContainerPtr c = p->container();
        p = dynamic_pointer_cast<Contained>(c); // This cast fails for Unit.
        if (!p)
        {
            break;
        }
    }

    assert(m);

    // The python:package metadata can be defined as file metadata or applied to a top-level module.
    // We check for the metadata at the top-level module first and then fall back to the global scope.
    static const string directive = "python:package";
    if (auto packageMetadata = m->getMetadataArgs(directive))
    {
        return *packageMetadata;
    }

    string file = cont->file();
    DefinitionContextPtr dc = cont->unit()->findDefinitionContext(file);
    assert(dc);
    return dc->getMetadataArgs(directive).value_or("");
}

string
Slice::Python::getAbsolute(const ContainedPtr& cont, const string& suffix, const string& nameSuffix)
{
    string scope = scopedToName(cont->scope());

    string package = getPackageMetadata(cont);
    if (!package.empty())
    {
        if (!scope.empty())
        {
            scope = package + "." + scope;
        }
        else
        {
            scope = package + ".";
        }
    }

    return scope + suffix + fixIdent(cont->name() + nameSuffix);
}

void
Slice::Python::printHeader(IceInternal::Output& out)
{
    static const char* header = "#\n"
                                "# Copyright (c) ZeroC, Inc.\n"
                                "#\n";

    out << header;
    out << "#\n";
    out << "# Ice version " << ICE_STRING_VERSION << "\n";
    out << "#\n";
}

bool
Slice::Python::MetadataVisitor::visitUnitStart(const UnitPtr& unit)
{
    // Validate file metadata in the top-level file and all included files.
    for (const auto& file : unit->allFiles())
    {
        DefinitionContextPtr dc = unit->findDefinitionContext(file);
        MetadataList fileMetadata = dc->getMetadata();
        for (auto r = fileMetadata.begin(); r != fileMetadata.end();)
        {
            MetadataPtr meta = *r++;
            string_view directive = meta->directive();
            string_view arguments = meta->arguments();

            if (directive.find("python:") == 0)
            {
                if (directive == "python:package" && !arguments.empty())
                {
                    continue;
                }
                if (directive == "python:pkgdir" && !arguments.empty())
                {
                    continue;
                }

                ostringstream msg;
                msg << "ignoring invalid file metadata '" << *meta << "'";
                unit->warning(meta->file(), meta->line(), InvalidMetadata, msg.str());
                fileMetadata.remove(meta);
            }
        }
        dc->setMetadata(fileMetadata);
    }
    return true;
}

bool
Slice::Python::MetadataVisitor::visitModuleStart(const ModulePtr& p)
{
    MetadataList metadata = p->getMetadata();
    for (auto r = metadata.begin(); r != metadata.end();)
    {
        MetadataPtr meta = *r++;
        string_view directive = meta->directive();

        if (directive.find("python:") == 0)
        {
            // Must be a top-level module.
            if (dynamic_pointer_cast<Unit>(p->container()) && directive == "python:package")
            {
                continue;
            }

            ostringstream msg;
            msg << "ignoring invalid file metadata '" << *meta << "'";
            p->unit()->warning(meta->file(), meta->line(), InvalidMetadata, msg.str());
            metadata.remove(meta);
        }
    }

    p->setMetadata(std::move(metadata));
    return true;
}

void
Slice::Python::MetadataVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    reject(p);
}

void
Slice::Python::MetadataVisitor::visitInterfaceDecl(const InterfaceDeclPtr& p)
{
    reject(p);
}

bool
Slice::Python::MetadataVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    reject(p);
    return true;
}

bool
Slice::Python::MetadataVisitor::visitStructStart(const StructPtr& p)
{
    reject(p);
    return true;
}

void
Slice::Python::MetadataVisitor::visitOperation(const OperationPtr& p)
{
    TypePtr ret = p->returnType();
    if (ret)
    {
        validateSequence(p, ret);
    }

    for (const auto& param : p->parameters())
    {
        validateSequence(param, param->type());
    }
}

void
Slice::Python::MetadataVisitor::visitDataMember(const DataMemberPtr& p)
{
    validateSequence(p, p->type());
}

void
Slice::Python::MetadataVisitor::visitSequence(const SequencePtr& p)
{
    p->setMetadata(validateSequence(p, p));
}

void
Slice::Python::MetadataVisitor::visitDictionary(const DictionaryPtr& p)
{
    reject(p);
}

void
Slice::Python::MetadataVisitor::visitEnum(const EnumPtr& p)
{
    reject(p);
}

void
Slice::Python::MetadataVisitor::visitConst(const ConstPtr& p)
{
    reject(p);
}

MetadataList
Slice::Python::MetadataVisitor::validateSequence(const ContainedPtr& cont, const TypePtr& type)
{
    static const string prefix = "python:";
    MetadataList newMetadata = cont->getMetadata();
    for (auto p = newMetadata.begin(); p != newMetadata.end();)
    {
        MetadataPtr s = *p++;
        string_view directive = s->directive();
        string_view arguments = s->arguments();

        if (directive.find(prefix) == 0)
        {
            SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
            if (seq)
            {
                if (directive == "python:seq")
                {
                    if (arguments == "tuple" || arguments == "list" || arguments == "default")
                    {
                        continue;
                    }
                }
                else if (directive.size() > prefix.size())
                {
                    string_view subArg = directive.substr(prefix.size());
                    if (subArg == "tuple" || subArg == "list" || subArg == "default")
                    {
                        continue;
                    }
                    else if (subArg == "array.array" || subArg == "numpy.ndarray" || subArg.find("memoryview") == 0)
                    {
                        // The memoryview sequence metadata is only valid for integral builtin
                        // types excluding strings.
                        BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(seq->type());
                        if (builtin)
                        {
                            switch (builtin->kind())
                            {
                                case Builtin::KindBool:
                                case Builtin::KindByte:
                                case Builtin::KindShort:
                                case Builtin::KindInt:
                                case Builtin::KindLong:
                                case Builtin::KindFloat:
                                case Builtin::KindDouble:
                                {
                                    continue;
                                }
                                default:
                                {
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            ostringstream msg;
            msg << "ignoring invalid metadata '" << *s << "'";
            type->unit()->warning(s->file(), s->line(), InvalidMetadata, msg.str());
            newMetadata.remove(s);
        }
    }
    return newMetadata;
}

void
Slice::Python::MetadataVisitor::reject(const ContainedPtr& cont)
{
    MetadataList localMetadata = cont->getMetadata();

    for (auto p = localMetadata.begin(); p != localMetadata.end();)
    {
        MetadataPtr s = *p++;
        if (s->directive().find("python:") == 0)
        {
            ostringstream msg;
            msg << "ignoring invalid metadata '" << *s << "'";
            cont->unit()->warning(s->file(), s->line(), InvalidMetadata, msg.str());
            localMetadata.remove(s);
        }
    }
    cont->setMetadata(std::move(localMetadata));
}
