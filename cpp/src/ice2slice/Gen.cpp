//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Gen.h"

#include <cassert>

using namespace std;
using namespace Slice;
using namespace IceInternal;

namespace
{
    struct ParamInfo
    {
        std::string name;
        TypePtr type;
        bool optional;
        int tag;
        ParamDeclPtr param; // 0 == return value
    };

    typedef std::list<ParamInfo> ParamInfoList;

    ParamInfoList getAllInParams(const OperationPtr& op)
    {
        const ParamDeclList l = op->inParameters();
        ParamInfoList r;
        for (ParamDeclList::const_iterator p = l.begin(); p != l.end(); ++p)
        {
            ParamInfo info;
            info.name = (*p)->name();
            info.type = (*p)->type();
            info.optional = (*p)->optional();
            info.tag = (*p)->tag();
            info.param = *p;
            r.push_back(info);
        }
        return r;
    }

    ParamInfoList getAllOutParams(const OperationPtr& op)
    {
        ParamDeclList params = op->outParameters();
        ParamInfoList l;

        for (ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
        {
            ParamInfo info;
            info.name = (*p)->name();
            info.type = (*p)->type();
            info.optional = (*p)->optional();
            info.tag = (*p)->tag();
            info.param = *p;
            l.push_back(info);
        }

        if (op->returnType())
        {
            ParamInfo info;
            info.name = "returnValue";
            info.type = op->returnType();
            info.optional = op->returnIsOptional();
            info.tag = op->returnTag();
            l.push_back(info);
        }

        return l;
    }

    static string getCSharpNamespace(const ContainedPtr& cont, bool& hasCSharpNamespaceAttribute)
    {
        // Traverse to the top-level module.
        ModulePtr m;
        ContainedPtr p = cont;
        string csharpNamespace;
        while (true)
        {
            if (dynamic_pointer_cast<Module>(p))
            {
                m = dynamic_pointer_cast<Module>(p);
                if (csharpNamespace.empty())
                {
                    csharpNamespace = m->name();
                }
                else
                {
                    csharpNamespace = m->name() + "." + csharpNamespace;
                }
            }

            ContainerPtr c = p->container();
            p = dynamic_pointer_cast<Contained>(c); // This cast fails for Unit.
            if (!p)
            {
                break;
            }
        }

        assert(m);

        if (auto metadata = m->getMetadataArgs("cs:namespace"))
        {
            hasCSharpNamespaceAttribute = true;
            return *metadata + "." + csharpNamespace;
        }
        else
        {
            hasCSharpNamespaceAttribute = false;
            return csharpNamespace;
        }
    }

    static string getOutputName(const string& fileBase, const string& scoped, bool includeNamespace)
    {
        ostringstream os;
        os << fileBase;
        if (includeNamespace)
        {
            assert(scoped[0] == ':');
            string::size_type next = 0;
            string::size_type pos;
            while ((pos = scoped.find("::", next)) != string::npos)
            {
                pos += 2;
                if (pos != scoped.size())
                {
                    string::size_type endpos = scoped.find("::", pos);
                    if (endpos != string::npos && endpos > pos)
                    {
                        os << "_" << scoped.substr(pos, endpos - pos);
                    }
                }
                next = pos;
            }

            if (next != scoped.size())
            {
                os << "_" << scoped.substr(next);
            }
        }
        os << ".slice";
        return os.str();
    }

    string getUnqualified(const ContainedPtr& contained, const string& moduleName)
    {
        const string scopedName = contained->scoped();
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
            "???",                       // KindLocalObject
            "AnyClass?"                  // KindValue
        };

        BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
        if (builtin)
        {
            os << builtinTable[builtin->kind()];
        }

        ContainedPtr contained = dynamic_pointer_cast<Contained>(type);
        if (contained)
        {
            os << getUnqualified(contained, scope);
        }

        if (optional)
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
            "???",                             // KindObject
            "IceRpc.Slice.Ice.IceObjectProxy", // KindObjectProxy
            "???",                             // KindLocalObject
            "???"                              // KindValue
        };

        BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
        if (builtin)
        {
            os << builtinTable[builtin->kind()];
        }

        ContainedPtr contained = dynamic_pointer_cast<Contained>(type);
        if (contained)
        {
            bool hasCSharpNamespaceAttribute;
            string csharpNamespace = getCSharpNamespace(contained, hasCSharpNamespaceAttribute);

            os << csharpNamespace << "." << contained->name();
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

    string paramToString(ParamInfo param, string scope)
    {
        ostringstream os;
        if (param.optional)
        {
            os << "tag(" << param.tag << ") ";
        }
        os << param.name << ": " << typeToString(param.type, scope, param.optional);
        return os.str();
    }

    string getParamList(const ParamInfoList& params, string scope)
    {
        ostringstream os;
        os << "(";
        for (ParamInfoList::const_iterator q = params.begin(); q != params.end();)
        {
            os << paramToString(*q, scope);
            q++;
            if (q != params.end())
            {
                os << ", ";
            }
        }
        os << ")";
        return os.str();
    }

    string slice2LinkFormatter(string identifier)
    {
        // Replace links of the form `{@link Type#member}` with `{@link Type.member}`.
        std::replace(identifier.begin(), identifier.end(), '#', '.');
        return "{@link " + identifier + "}";
    }

    void writeComment(const ContainedPtr& contained, Output& out)
    {
        CommentPtr comment = contained->parseComment(slice2LinkFormatter, false, true);
        if (!comment)
        {
            return;
        }
        StringList overview = comment->overview();
        for (StringList::const_iterator it = overview.begin(); it != overview.end(); it++)
        {
            out << nl << "/// " << (*it);
        }

        OperationPtr operation = dynamic_pointer_cast<Operation>(contained);
        if (operation)
        {
            std::map<std::string, StringList> parameterDocs = comment->parameters();
            ParamDeclList parameters = operation->parameters();
            for (ParamDeclList::const_iterator it = parameters.begin(); it != parameters.end(); it++)
            {
                ParamDeclPtr param = *it;
                if (!param->isOutParam())
                {
                    std::map<std::string, StringList>::const_iterator q = parameterDocs.find(param->name());
                    if (q != parameterDocs.end())
                    {
                        out << nl << "/// @param " << param->name() << ": ";
                        for (StringList::const_iterator r = q->second.begin(); r != q->second.end();)
                        {
                            if (r != q->second.begin())
                            {
                                out << nl;
                            }
                            out << (*r);
                            r++;
                        }
                    }
                }
            }
        }
    }

    void writeDataMembers(Output& out, DataMemberList dataMembers, string scope)
    {
        for (DataMemberList::const_iterator i = dataMembers.begin(); i != dataMembers.end(); ++i)
        {
            DataMemberPtr member = *i;
            writeComment(member, out);
            out << nl;
            if (member->optional())
            {
                out << "tag(" << member->tag() << ") ";
            }
            out << member->name() << ": " << typeToString(member->type(), scope, member->optional());
        }
    }
}

Gen::Gen(const std::string& fileBase) : _fileBase(fileBase) {}

void
Gen::generate(const UnitPtr& p)
{
    OutputVisitor outputVisitor;
    p->visit(&outputVisitor);

    TypesVisitor typesVisitor(_fileBase, outputVisitor.modules());
    p->visit(&typesVisitor);
}

bool
Gen::OutputVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    _modules.insert(p->scope());
    return false;
}

bool
Gen::OutputVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    _modules.insert(p->scope());
    return false;
}

bool
Gen::OutputVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    _modules.insert(p->scope());
    return false;
}

bool
Gen::OutputVisitor::visitStructStart(const StructPtr& p)
{
    _modules.insert(p->scope());
    return false;
}

void
Gen::OutputVisitor::visitSequence(const SequencePtr& p)
{
    _modules.insert(p->scope());
}

void
Gen::OutputVisitor::visitDictionary(const DictionaryPtr& p)
{
    _modules.insert(p->scope());
}

void
Gen::OutputVisitor::visitEnum(const EnumPtr& p)
{
    _modules.insert(p->scope());
}

set<string>
Gen::OutputVisitor::modules() const
{
    return _modules;
}

Gen::TypesVisitor::TypesVisitor(const std::string& fileBase, const std::set<std::string>& modules)
    : _fileBase(fileBase),
      _modules(modules)
{
}

bool
Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    ClassDefPtr base = p->base();
    const string scope = p->scope();
    Output& out = getOutput(p);

    writeComment(p, out);

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
    out << nl;
    return false;
}

bool
Gen::TypesVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    InterfaceList bases = p->bases();
    const string scope = p->scope();
    Output& out = getOutput(p);

    writeComment(p, out);
    out << nl << "interface " << p->name();
    if (bases.size() > 0)
    {
        out << " :";
        for (InterfaceList::const_iterator q = bases.begin(); q != bases.end();)
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
    for (OperationList::const_iterator q = operations.begin(); q != operations.end();)
    {
        OperationPtr op = *q;
        writeComment(op, out);
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
        ParamInfoList inParams = getAllInParams(op);
        out << getParamList(inParams, scope);
        ParamInfoList outParams = getAllOutParams(op);
        if (outParams.size() > 1)
        {
            out << " -> " << getParamList(outParams, scope);
        }
        else if (outParams.size() > 0)
        {
            out << " -> " << paramToString(outParams.front(), scope);
        }

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();
        if (throws.size() == 1)
        {
            out << " throws " << getUnqualified(throws.front(), scope);
        }
        else if (throws.size() > 1)
        {
            out << " throws (";
            for (ExceptionList::const_iterator r = throws.begin(); r != throws.end();)
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
    out << nl;
    return false;
}

bool
Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    ExceptionPtr base = p->base();
    const string scope = p->scope();
    Output& out = getOutput(p);
    writeComment(p, out);
    out << nl << "exception " << p->name();
    if (base)
    {
        out << " : " << getUnqualified(base, scope);
    }
    out << " {";
    out.inc();

    writeDataMembers(out, p->dataMembers(), scope);

    out.dec();
    out << nl << "}";
    out << nl;
    return false;
}

bool
Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    const string scope = p->scope();
    Output& out = getOutput(p);
    writeComment(p, out);
    out << nl << "compact struct " << p->name() << " {";
    out.inc();

    writeDataMembers(out, p->dataMembers(), scope);

    out.dec();
    out << nl << "}";
    out << nl;
    return false;
}

void
Gen::TypesVisitor::visitSequence(const SequencePtr& p)
{
    const string scope = p->scope();
    Output& out = getOutput(p);

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
                << typeToString(p->type(), p->scope(), false) << ">\")]";
            break;
        }
    }
    out << " Sequence<" << typeToString(p->type(), p->scope(), false) << ">";
    out << nl;
}

void
Gen::TypesVisitor::visitDictionary(const DictionaryPtr& p)
{
    const string scope = p->scope();
    Output& out = getOutput(p);

    out << nl << "typealias " << p->name() << " = ";

    for (const auto& metadata : p->getMetadata())
    {
        if (metadata->directive() == "cs:generic" && metadata->arguments() == "SortedDictionary")
        {
            out << "[cs::type(\"[System.Collections.Generic.SortedDictionary<"
                // TODO the generic arguments must be the IceRPC C# mapped types
                << typeToString(p->keyType(), p->scope(), false) << ", "
                << typeToString(p->valueType(), p->scope(), false) << ">\")]";
            break;
        }
    }
    out << " Dictionary<" << typeToString(p->keyType(), p->scope(), false) << ", "
        << typeToString(p->valueType(), p->scope(), false) << ">";
    out << nl;
}

void
Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    const string scope = p->scope();
    Output& out = getOutput(p);

    writeComment(p, out);
    out << nl << "enum " << p->name() << " {";
    out.inc();

    EnumeratorList enumerators = p->enumerators();
    const bool hasExplicitValues = p->hasExplicitValues();
    for (EnumeratorList::const_iterator q = enumerators.begin(); q != enumerators.end(); ++q)
    {
        EnumeratorPtr en = *q;
        out << nl << en->name();
        if (hasExplicitValues)
        {
            out << " = " << en->value();
        }
    }
    out.dec();
    out << nl << "}";
    out << nl;
}

IceInternal::Output&
Gen::TypesVisitor::getOutput(const ContainedPtr& contained)
{
    string scopedName = contained->scope();
    map<string, Output*>::const_iterator it = _outputs.find(scopedName);
    if (it == _outputs.end())
    {
        string outputName = getOutputName(_fileBase, scopedName, _modules.size() > 1);
        Output* out = new Output(outputName.c_str());
        *out << "// Use Slice1 mode for compatibility with ZeroC Ice.";
        *out << nl << "mode = Slice1";
        *out << nl;

        bool hasCSharpNamespaceAttribute;
        string csharpNamespace = getCSharpNamespace(contained, hasCSharpNamespaceAttribute);
        if (hasCSharpNamespaceAttribute)
        {
            *out << nl << "[cs::namespace(\"" << csharpNamespace << "\")]";
        }

        // The module name is the scoped named without the start and end scope operator '::'
        assert(scopedName.find("::") == 0);
        assert(scopedName.rfind("::") == scopedName.size() - 2);
        string moduleName = scopedName.substr(2).substr(0, scopedName.size() - 4);

        *out << nl << "module " << moduleName;
        *out << nl;
        _outputs[scopedName] = out;
        return *out;
    }
    else
    {
        return *(it->second);
    }
}
