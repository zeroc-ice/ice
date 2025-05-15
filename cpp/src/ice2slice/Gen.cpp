// Copyright (c) ZeroC, Inc.

#include "Gen.h"
#include "../Slice/FileTracker.h"
#include "../Slice/Util.h"

#include <cassert>

using namespace std;
using namespace Slice;
using namespace IceInternal;

namespace
{
    static void writeCsIdentifier(const ContainedPtr& cont, Output& out)
    {
        // Modules are not allowed to have a cs::identifier attribute in IceRPC Slice.
        // they are handled separately in the getOutput function.
        assert(!dynamic_pointer_cast<Module>(cont));

        if (auto metadata = cont->getMetadataArgs("cs:identifier"))
        {
            out << nl << "[cs::identifier(\"" << *metadata << "\")]";
        }
    }

    // We cannot use mappedName because it depends on the language name used to create the Unit.
    static string getCsMappedName(const ContainedPtr& cont)
    {
        return cont->getMetadataArgs("cs:identifier").value_or(cont->name());
    }

    static string getCsMappedScoped(const ContainedPtr& cont);

    // We cannot use mappedScope because it depends on the language name used to create the Unit.
    static string getCsMappedScope(const ContainedPtr& cont)
    {
        string scoped;
        auto container = dynamic_pointer_cast<Contained>(cont->container());
        if (container)
        {
            scoped = getCsMappedScoped(container);
        }
        return scoped + '.';
    }

    // We cannot use mappedScoped because it depends on the language name used to create the Unit.
    static string getCsMappedScoped(const ContainedPtr& cont) { return getCsMappedScope(cont) + getCsMappedName(cont); }

    static string getCsMappedNamespace(const ContainedPtr& cont)
    {
        ModulePtr topLevelModule = cont->getTopLevelModule();
        string csMappedScope = getCsMappedScope(cont).substr(1);
        csMappedScope = csMappedScope.substr(0, csMappedScope.size() - 1); // Remove the trailing "."

        if (auto csNamespacePrefix = topLevelModule->getMetadataArgs("cs:namespace"))
        {
            return *csNamespacePrefix + "." + csMappedScope;
        }
        else
        {
            return csMappedScope;
        }
    }

    // Return the output file name for a given file and module scope. The mapped file name is the base file name with
    // the module scope appended and "::" replaced by "_" and the .slice extension. For .ice files with a single module
    // we always pass an empty scope.
    static string getOutputName(const string& fileBase, string scope)
    {
        scope = scope.substr(0, scope.size() - 2); // Remove the trailing "::"
        for (size_t pos = scope.find("::"); pos != std::string::npos; pos = scope.find("::"))
        {
            assert(pos + 2 <= scope.size());
            scope.replace(pos, 2, "_");
        }
        return fileBase + scope + ".slice";
    }

    string getUnqualified(const ContainedPtr& contained, const string& moduleName)
    {
        string scopedName = contained->scoped();
        if (scopedName.find("::") != string::npos && scopedName.find(moduleName) == 0 &&
            scopedName.find("::", moduleName.size()) == string::npos)
        {
            return scopedName.substr(moduleName.size());
        }
        else
        {
            return scopedName;
        }
    }

    string typeToString(const TypePtr& type, const string& scope, bool optional)
    {
        ostringstream os;

        static const char* builtinTable[] = {
            "uint8",                     // KindByte
            "bool",                      // KindBool
            "int16",                     // KindShort
            "int32",                     // KindInt
            "int64",                     // KindLong
            "float32",                   // KindFloat
            "float64",                   // KindDouble
            "string",                    // KindString
            "AnyClass?",                 // KindObject
            "::IceRpc::ServiceAddress?", // KindObjectProxy
            "AnyClass?"                  // KindValue
        };

        if (BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type))
        {
            os << builtinTable[builtin->kind()];
        }
        else if (auto interface = dynamic_pointer_cast<InterfaceDecl>(type))
        {
            // Proxys are always mapped to nullable types.
            os << getUnqualified(interface, scope) << "Proxy?";
        }
        else if (ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type))
        {
            // Classes are always mapped to nullable types.
            os << getUnqualified(cl, scope) << "?";
        }
        else if (ContainedPtr contained = dynamic_pointer_cast<Contained>(type))
        {
            os << getUnqualified(contained, scope);
        }

        if (optional && !type->isClassType() && !isProxyType(type))
        {
            os << "?";
        }

        return os.str();
    }

    string typeToCsString(const TypePtr& type, bool optional)
    {
        ostringstream os;

        static const char* builtinTable[] = {
            "byte",                            // KindByte
            "bool",                            // KindBool
            "short",                           // KindShort
            "int",                             // KindInt
            "long",                            // KindLong
            "float",                           // KindFloat
            "double",                          // KindDouble
            "string",                          // KindString
            "AnyClass",                        // KindObject
            "IceRpc.Slice.Ice.IceObjectProxy", // KindObjectProxy
            "AnyClass"                         // KindValue
        };

        BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
        if (builtin)
        {
            os << builtinTable[builtin->kind()];
        }

        ContainedPtr contained = dynamic_pointer_cast<Contained>(type);
        if (contained)
        {
            os << getCsMappedNamespace(contained) << "." << getCsMappedName(contained);
            InterfaceDeclPtr interface = dynamic_pointer_cast<InterfaceDecl>(contained);
            if (interface)
            {
                os << "Proxy";
            }
        }

        if (optional)
        {
            os << "?";
        }

        return os.str();
    }

    string paramToString(const ParameterPtr& param, const string& scope, bool includeParamName = true)
    {
        ostringstream os;
        if (auto identifier = param->getMetadataArgs("cs:identifier"))
        {
            os << "[cs::identifier(\"" << *identifier << "\")] ";
        }

        if (param->optional())
        {
            os << "tag(" << param->tag() << ") ";
        }

        if (includeParamName)
        {
            os << param->name() << ": ";
        }
        os << typeToString(param->type(), scope, param->optional());
        return os.str();
    }

    void writeCommentLines(IceInternal::Output& out, const StringList& lines, bool startInNewLine)
    {
        bool first = true;
        for (const auto& line : lines)
        {
            if (startInNewLine || !first)
            {
                out << nl << "///" << (line.empty() ? "" : " ");
            }
            first = false;
            out << line;
        }
    }

    string slice2LinkFormatter(const string& rawLink, const ContainedPtr&, const SyntaxTreeBasePtr&)
    {
        // The only difference with '@link' between the 'Ice' and 'Slice' syntaxes
        // is that the 'Ice' syntax uses '#' whereas the 'Slice' syntax uses '::'.
        string formattedLink;
        auto separatorPos = rawLink.find('#');
        if (separatorPos == 0)
        {
            // We want to avoid converting the relative link '#member' into the global link '::member'.
            // Instead we simply convert it to 'member' with no prefix.
            formattedLink = rawLink.substr(1);
        }
        else if (separatorPos != string::npos)
        {
            formattedLink = rawLink;
            formattedLink.replace(separatorPos, 1, "::");
        }

        return "{@link " + formattedLink + "}";
    }

    void writeDocComment(const ContainedPtr& contained, Output& out)
    {
        optional<DocComment> comment = DocComment::parseFrom(contained, slice2LinkFormatter, true);
        if (!comment)
        {
            return;
        }

        writeCommentLines(out, comment->overview(), true);

        OperationPtr operation = dynamic_pointer_cast<Operation>(contained);
        if (operation)
        {
            std::map<std::string, StringList> parameterDocs = comment->parameters();

            // Input parameters
            for (const auto& param : operation->inParameters())
            {
                auto q = parameterDocs.find(param->name());
                if (q != parameterDocs.end())
                {
                    out << nl << "/// @param " << param->name() << ": ";
                    writeCommentLines(out, q->second, false);
                }
            }

            // Output parameters
            for (const auto& param : operation->outParameters())
            {
                auto q = parameterDocs.find(param->name());
                if (q != parameterDocs.end())
                {
                    out << nl << "/// @returns";
                    if (operation->returnsMultipleValues())
                    {
                        out << " " << param->name();
                    }
                    out << ": ";

                    writeCommentLines(out, q->second, false);
                }
            }

            // Return value
            StringList returnDocs = comment->returns();
            if (!returnDocs.empty())
            {
                out << nl << "/// @returns";
                if (operation->returnsMultipleValues())
                {
                    out << " returnValue";
                }
                out << ": ";

                writeCommentLines(out, returnDocs, false);
            }

            // Exceptions
            for (const auto& [name, docs] : comment->exceptions())
            {
                out << nl << "/// @throws " << name << ": ";
                writeCommentLines(out, docs, false);
            }
        }

        for (string ident : comment->seeAlso())
        {
            string memberComponent = "";
            string::size_type hashPos = ident.find('#');
            if (hashPos != string::npos)
            {
                memberComponent = ident.substr(hashPos + 1);
                ident.erase(hashPos);
            }

            size_t pos = ident.find('.');
            while (pos != string::npos)
            {
                ident.replace(pos, 1, "::");
                pos = ident.find('.', pos + 2); // Move past the newly inserted "::"
            }

            if (!memberComponent.empty())
            {
                ident += "::" + memberComponent;
            }

            out << nl << "/// @see " << ident;
        }
    }

    void writeDataMembers(Output& out, const DataMemberList& dataMembers, const string& scope)
    {
        for (const auto& member : dataMembers)
        {
            writeDocComment(member, out);
            writeCsIdentifier(member, out);
            out << nl;
            if (member->optional())
            {
                out << "tag(" << member->tag() << ") ";
            }
            out << member->name() << ": " << typeToString(member->type(), scope, member->optional());
        }
    }
}

Gen::Gen(std::string fileBase) : _fileBase(std::move(fileBase)) {}

void
Gen::generate(const UnitPtr& p)
{
    OutputModulesVisitor outputModulesVisitor;
    p->visit(&outputModulesVisitor);

    TypesVisitor typesVisitor(_fileBase, outputModulesVisitor.modules());
    p->visit(&typesVisitor);
}

bool
Gen::OutputModulesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    _modules.insert(p->scope());
    return false;
}

bool
Gen::OutputModulesVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    _modules.insert(p->scope());
    return false;
}

bool
Gen::OutputModulesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    _modules.insert(p->scope());
    return false;
}

bool
Gen::OutputModulesVisitor::visitStructStart(const StructPtr& p)
{
    _modules.insert(p->scope());
    return false;
}

void
Gen::OutputModulesVisitor::visitSequence(const SequencePtr& p)
{
    _modules.insert(p->scope());
}

void
Gen::OutputModulesVisitor::visitDictionary(const DictionaryPtr& p)
{
    _modules.insert(p->scope());
}

void
Gen::OutputModulesVisitor::visitEnum(const EnumPtr& p)
{
    _modules.insert(p->scope());
}

set<string>
Gen::OutputModulesVisitor::modules() const
{
    return _modules;
}

Gen::TypesVisitor::TypesVisitor(std::string fileBase, const std::set<std::string>& modules)
    : _fileBase(std::move(fileBase)),
      _modules(modules)
{
}

void
Gen::TypesVisitor::visitUnitEnd(const UnitPtr&)
{
    // Append a newline to each generated file to ensure it ends properly.
    for (const auto& [_, output] : _outputs)
    {
        *output << nl;
    }
}

bool
Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    ClassDefPtr base = p->base();
    const string scope = p->scope();
    Output& out = getOutput(p);

    out << sp;
    writeDocComment(p, out);

    out << nl << "class " << p->name();
    if (base)
    {
        out << " : " << getUnqualified(base, scope);
    }
    out << " {";
    out.inc();

    writeDataMembers(out, p->dataMembers(), scope);

    out.dec();
    out << nl << "}";
    return false;
}

bool
Gen::TypesVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    InterfaceList bases = p->bases();
    const string scope = p->scope();
    Output& out = getOutput(p);

    out << sp;
    writeDocComment(p, out);
    writeCsIdentifier(p, out);
    out << nl << "interface " << p->name();
    if (bases.size() > 0)
    {
        out << " :";
        for (auto q = bases.begin(); q != bases.end();)
        {
            InterfaceDefPtr base = *q;
            out << " " << getUnqualified(base, scope);
            q++;
            if (q != bases.end())
            {
                out << ",";
            }
        }
    }
    out << " {";
    out.inc();
    OperationList operations = p->operations();
    for (auto q = operations.begin(); q != operations.end();)
    {
        OperationPtr op = *q;
        writeDocComment(op, out);
        writeCsIdentifier(op, out);
        if (op->hasMetadata("marshaled-result"))
        {
            out << nl << "[cs::encodedReturn]";
        }
        out << nl;
        if (op->mode() == Operation::Idempotent)
        {
            out << "idempotent ";
        }
        out << op->name();

        // Write the operation's parameters.
        out << spar;
        for (const auto& param : op->inParameters())
        {
            out << paramToString(param, scope);
        }
        out << epar;

        // Write the operation's return type.
        const ParameterList returnAndOutParams = op->returnAndOutParameters("returnValue");
        if (returnAndOutParams.size() > 1)
        {
            out << " -> ";
            out << spar;
            for (const auto& param : returnAndOutParams)
            {
                out << paramToString(param, scope);
            }
            out << epar;
        }
        else if (returnAndOutParams.size() > 0)
        {
            out << " -> " << paramToString(returnAndOutParams.front(), scope, false);
        }

        ExceptionList throws = op->throws();
        if (throws.size() == 1)
        {
            out << " throws " << getUnqualified(throws.front(), scope);
        }
        else if (throws.size() > 1)
        {
            out << " throws (";
            for (auto r = throws.begin(); r != throws.end();)
            {
                ExceptionPtr ex = *r;
                out << getUnqualified(ex, scope);
                r++;
                if (r != throws.end())
                {
                    out << ", ";
                }
            }
            out << ")";
        }

        q++;
        if (q != operations.end())
        {
            out << sp;
        }
    }
    out.dec();
    out << nl << "}";

    out << sp;
    out << nl << "[cs::type(\"" << typeToCsString(p->declaration(), false) << "\")]";
    out << nl << "custom " << p->name() << "Proxy";
    return false;
}

bool
Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    const string scope = p->scope();
    Output& out = getOutput(p);

    out << sp;
    writeDocComment(p, out);
    writeCsIdentifier(p, out);
    out << nl << "exception " << p->name();
    if (ExceptionPtr base = p->base())
    {
        out << " : " << getUnqualified(base, scope);
    }
    out << " {";
    out.inc();

    writeDataMembers(out, p->dataMembers(), scope);

    out.dec();
    out << nl << "}";
    return false;
}

bool
Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    const string scope = p->scope();
    Output& out = getOutput(p);

    out << sp;
    writeDocComment(p, out);
    writeCsIdentifier(p, out);
    out << nl << "compact struct " << p->name() << " {";
    out.inc();

    writeDataMembers(out, p->dataMembers(), scope);

    out.dec();
    out << nl << "}";
    return false;
}

void
Gen::TypesVisitor::visitSequence(const SequencePtr& p)
{
    const string scope = p->scope();
    Output& out = getOutput(p);

    out << sp;
    writeDocComment(p, out);
    out << nl << "typealias " << p->name() << " = ";

    for (const auto& metadata : p->getMetadata())
    {
        if (metadata->directive() == "cs:generic")
        {
            string_view type = metadata->arguments();
            if ((type == "LinkedList" || type == "Queue" || type == "Stack") && p->type()->isClassType())
            {
                continue; // Ignored for objects
            }

            out << "[cs::type(\"";

            if (type == "List" || type == "LinkedList" || type == "Queue" || type == "Stack")
            {
                out << "System.Collections.Generic." << type;
            }
            else
            {
                out << type;
            }
            out << "<"
                // TODO the generic argument must be the IceRPC C# mapped type
                << typeToString(p->type(), scope, false) << ">\")]";
            break;
        }
    }
    out << " Sequence<" << typeToString(p->type(), scope, false) << ">";
}

void
Gen::TypesVisitor::visitDictionary(const DictionaryPtr& p)
{
    const string scope = p->scope();
    Output& out = getOutput(p);

    out << sp;
    writeDocComment(p, out);
    out << nl << "typealias " << p->name() << " = ";

    for (const auto& metadata : p->getMetadata())
    {
        if (metadata->directive() == "cs:generic" && metadata->arguments() == "SortedDictionary")
        {
            out << "[cs::type(\"[System.Collections.Generic.SortedDictionary<"
                // TODO the generic arguments must be the IceRPC C# mapped types
                << typeToString(p->keyType(), scope, false) << ", " << typeToString(p->valueType(), scope, false)
                << ">\")]";
            break;
        }
    }
    out << " Dictionary<" << typeToString(p->keyType(), scope, false) << ", "
        << typeToString(p->valueType(), scope, false) << ">";
}

void
Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    Output& out = getOutput(p);

    out << sp;
    writeDocComment(p, out);
    writeCsIdentifier(p, out);
    out << nl << "enum " << p->name() << " {";
    out.inc();

    for (const auto& en : p->enumerators())
    {
        out << nl << en->name();
        if (p->hasExplicitValues())
        {
            out << " = " << en->value();
        }
    }
    out.dec();
    out << nl << "}";
}

void
Slice::Gen::TypesVisitor::visitConst(const ConstPtr& p)
{
    string typeString;
    if (auto builtin = dynamic_pointer_cast<Builtin>(p->type()))
    {
        typeString = builtin->kindAsString();
    }
    else
    {
        auto contained = dynamic_pointer_cast<Contained>(p->type());
        assert(contained);
        typeString = contained->scoped();
    }

    Output& out = getOutput(p);
    out << sp;
    out << nl << "// ice2slice could not convert:";
    out << nl << "// const " << typeString << " " << p->name() << " = " << p->value();

    p->unit()
        ->warning(p->file(), p->line(), WarningCategory::All, "ice2slice could not convert constant: " + p->name());
}

// Get the output stream where to write the mapped Slice construct, creating a new output stream if necessary. The
// mapping accounts for the fact that .slice files contain a single module, while .ice files can contain multiple
// modules. See comment in getOutputName for more details.
IceInternal::Output&
Gen::TypesVisitor::getOutput(const ContainedPtr& contained)
{
    const string scope = contained->scope();
    if (auto it = _outputs.find(scope); it != _outputs.end())
    {
        return *(it->second);
    }
    else
    {
        string outputName = getOutputName(_fileBase, _modules.size() > 1 ? scope : "");
        unique_ptr<Output> out = make_unique<Output>(outputName.c_str());
        FileTracker::instance()->addFile(outputName);
        *out << "// This file was generated by ice2slice (" << ICE_STRING_VERSION << ") from: " << contained->file();
        *out << nl;

        *out << nl << "// Use Slice1 mode for compatibility with ZeroC Ice.";
        *out << nl << "mode = Slice1";
        *out << nl;

        // The module name is the scoped named without the start and end scope operator '::'
        assert(scope.find("::") == 0);
        assert(scope.rfind("::") == scope.size() - 2);
        string moduleName = scope.substr(2).substr(0, scope.size() - 4);

        string csNamespace = moduleName;
        string::size_type pos = string::npos;
        while ((pos = csNamespace.find("::")) != string::npos)
        {
            csNamespace.replace(pos, 2, ".");
        }
        string csMappedNamespace = getCsMappedNamespace(contained);

        // If the mapped C# namespace is different than the default C# namespace, we need to add the
        // cs::namespace attribute to the module.
        if (csMappedNamespace != csNamespace)
        {
            *out << nl << "[cs::namespace(\"" << csMappedNamespace << "\")]";
        }

        *out << nl << "module " << moduleName;
        auto inserted = _outputs.emplace(scope, std::move(out));
        return *(inserted.first->second);
    }
}
