// Copyright (c) ZeroC, Inc.

#include "Gen.h"
#include "../Ice/ConsoleUtil.h"
#include "../Ice/FileUtil.h"
#include "../Slice/FileTracker.h"
#include "../Slice/Util.h"
#include "Ice/StringUtil.h"

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstring>
#include <regex>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#    include <direct.h>
#else
#    include <unistd.h>
#endif

using namespace std;
using namespace Slice;
using namespace IceInternal;

namespace
{
    void
    documentArgument(IceInternal::Output& out, const ParameterPtr& param, const string& argName, StringList docLines);
    void documentProperty(IceInternal::Output& out, const DataMemberPtr& field);

    void writeGeneratedFrom(IceInternal::Output& out, string_view file)
    {
        string::size_type pos = file.find_last_of('/');
        if (pos != string::npos)
        {
            file = file.substr(pos + 1);
        }
        out << nl << "%";
        out << nl << "%   Generated from " << file << " by slice2matlab version " << ICE_STRING_VERSION;
        out << sp;
    }

    string typeToString(const TypePtr& type)
    {
        static const char* builtinTable[] = {
            "uint8",
            "logical",
            "int16",
            "int32",
            "int64",
            "single",
            "double",
            "char",
            "Ice.ObjectPrx", // ObjectPrx
            "Ice.Value"      // Value
        };

        if (!type)
        {
            return "void";
        }

        if (auto builtin = dynamic_pointer_cast<Builtin>(type))
        {
            return builtinTable[builtin->kind()];
        }

        if (auto proxy = dynamic_pointer_cast<InterfaceDecl>(type))
        {
            return proxy->mappedScoped(".") + "Prx";
        }

        if (dynamic_pointer_cast<Dictionary>(type))
        {
            return "dictionary";
        }

        auto contained = dynamic_pointer_cast<Contained>(type);
        assert(contained); // every 'Type' is either 'Builtin', or 'Contained'
        return contained->mappedScoped(".");
    }

    // Type represents a dictionary value type or the type of a sequence element.
    // Returns true if when the mapped dictionary value type is a scalar, as opposed to a cell array. For sequences,
    // returns true when the sequence maps to a regular array as opposed to a cell array.
    bool isMappedToScalar(const TypePtr& type)
    {
        if (auto builtin = dynamic_pointer_cast<Builtin>(type))
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
                case Builtin::KindString:
                    return true;
                default:
                    return false;
            }
        }

        return dynamic_pointer_cast<Enum>(type) || dynamic_pointer_cast<Struct>(type);
    }

    string constantValue(const TypePtr& type, const SyntaxTreeBasePtr& valueType, const string& value)
    {
        ConstPtr constant = dynamic_pointer_cast<Const>(valueType);
        if (constant)
        {
            return constant->mappedScoped(".") + ".value";
        }
        else
        {
            BuiltinPtr bp;
            if ((bp = dynamic_pointer_cast<Builtin>(type)))
            {
                switch (bp->kind())
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
                    case Builtin::KindObjectProxy:
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
            else if (dynamic_pointer_cast<Enum>(type))
            {
                EnumeratorPtr e = dynamic_pointer_cast<Enumerator>(valueType);
                assert(e);
                return e->mappedScoped(".");
            }
            else
            {
                return value;
            }
        }
    }

    string defaultValue(const DataMemberPtr& m)
    {
        if (m->defaultValue()) // explicit default value
        {
            return constantValue(m->type(), m->defaultValueType(), *m->defaultValue());
        }
        else if (m->optional() && !isProxyType(m->type()))
        {
            // We don't distinguish between unset and "null" (empty) for proxy types. We always use empty for them.
            return "IceInternal.UnsetI.Instance";
        }
        else
        {
            BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(m->type());
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
                    case Builtin::KindString:
                        return ""; // use implicit default
                    case Builtin::KindObjectProxy:
                        return "Ice.ObjectPrx.empty";
                    case Builtin::KindValue:
                        return "Ice.UnknownSlicedValue.empty";
                }
            }

            if (auto dict = dynamic_pointer_cast<Dictionary>(m->type()))
            {
                // Generate configured empty dictionary.
                const TypePtr key = dict->keyType();
                const TypePtr value = dict->valueType();
                ostringstream ostr;
                ostr << "configureDictionary('" << typeToString(key) << "', '"
                     << (isMappedToScalar(value) ? typeToString(value) : "cell") << "')";
                return ostr.str();
            }

            if (auto seq = dynamic_pointer_cast<Sequence>(m->type()); seq && seq->type()->usesClasses())
            {
                // Property has no type, so we need to generate an empty cell array.
                return "{}";
            }

            if (dynamic_pointer_cast<InterfaceDecl>(m->type()) || dynamic_pointer_cast<Struct>(m->type()) ||
                m->type()->usesClasses())
            {
                // Use .empty for proxies, structs and untyped properties.
                return typeToString(m->type()) + ".empty";
            }

            return ""; // use implicit default
        }
    }

    bool isProxy(const TypePtr& type)
    {
        BuiltinPtr b = dynamic_pointer_cast<Builtin>(type);
        InterfaceDeclPtr p = dynamic_pointer_cast<InterfaceDecl>(type);
        return (b && b->kind() == Builtin::KindObjectProxy) || p;
    }

    bool needsConversion(const TypePtr& type) { return type->usesClasses() && !type->isClassType(); }

    void convertValueType(
        IceInternal::Output& out,
        const string& dest,
        const string& src,
        const TypePtr& type,
        bool optional)
    {
        assert(needsConversion(type));

        if (optional)
        {
            out << nl << "if " << src << " ~= Ice.Unset";
            out.inc();
        }

        SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
        if (seq)
        {
            out << nl << dest << " = " << seq->mappedScoped(".") << ".convert(" << src << ");";
        }

        DictionaryPtr d = dynamic_pointer_cast<Dictionary>(type);
        if (d)
        {
            out << nl << dest << " = " << d->mappedScoped(".") << ".convert(" << src << ");";
        }

        StructPtr st = dynamic_pointer_cast<Struct>(type);
        if (st)
        {
            out << nl << dest << " = " << src << ".ice_convert();";
        }

        if (optional)
        {
            out.dec();
            out << nl << "end";
        }
    }

    void writeDocLines(IceInternal::Output& out, const StringList& lines, size_t indentation = 0)
    {
        for (const auto& line : lines)
        {
            out << nl << "%";
            if (!line.empty())
            {
                out << " " << string(indentation, ' ') << line;
            }
        }
    }

    void writeSeeAlso(IceInternal::Output& out, const optional<DocComment>& doc, const ContainerPtr& container)
    {
        if (!doc)
        {
            return;
        }
        const StringList& seeAlso = doc->seeAlso();
        if (seeAlso.empty())
        {
            return;
        }

        //
        // All references must be on one line.
        //
        out << nl << "%";
        out << nl << "%   See also ";
        for (auto p = seeAlso.begin(); p != seeAlso.end(); ++p)
        {
            if (p != seeAlso.begin())
            {
                out << ", ";
            }

            string sym = *p;
            string rest;
            string::size_type pos = sym.find('#');
            if (pos != string::npos)
            {
                rest = sym.substr(pos + 1);
                sym = sym.substr(0, pos);
            }

            if (!sym.empty() || !rest.empty())
            {
                if (!sym.empty())
                {
                    ContainedList c = container->lookupContained(sym, false);
                    if (c.empty())
                    {
                        out << sym;
                    }
                    else
                    {
                        out << c.front()->mappedScoped(".");
                    }

                    if (!rest.empty())
                    {
                        out << ".";
                    }
                }

                if (!rest.empty())
                {
                    out << rest;
                }
            }
        }
    }

    void writeDeprecated(IceInternal::Output& out, const optional<DocComment>& doc, const ContainedPtr& p)
    {
        if (doc && doc->isDeprecated())
        {
            out << nl << "%";
            out << nl << "%   Deprecated";
            StringList docDeprecated = doc->deprecated();
            if (!docDeprecated.empty())
            {
                out << ": " << docDeprecated.front();
                docDeprecated.pop_front();
                if (!docDeprecated.empty())
                {
                    writeDocLines(out, docDeprecated, 4);
                }
            }
        }
        else if (p->isDeprecated())
        {
            out << nl << "%";
            out << nl << "%   Deprecated";
            optional<string> deprecationReason = p->getDeprecationReason();
            if (deprecationReason)
            {
                out << ": " << *deprecationReason;
            }
        }
    }

    void writeRemarks(IceInternal::Output& out, const optional<DocComment>& doc)
    {
        if (!doc)
        {
            return;
        }
        const StringList& remarks = doc->remarks();
        if (remarks.empty())
        {
            return;
        }

        out << nl << "%";
        out << nl << "%   Remarks";
        writeDocLines(out, remarks, 4);
    }

    void writeConstructorDoc(IceInternal::Output& out, const string& name, const DataMemberList& fields)
    {
        out << nl << "%";
        out << nl << "%   Creation";
        out << nl << "%     Syntax";
        out << nl << "%       obj = " << name << "()";
        if (!fields.empty())
        {
            out << nl << "%       obj = " << name << spar;
            for (const auto& field : fields)
            {
                out << field->mappedName();
            }
            out << epar;

            out << nl << "%";
            out << nl << "%     The input arguments correspond to the properties, in order.";
        }
    }

    template<class T>
    void writePropertiesSummary(IceInternal::Output& out, const string& name, const std::list<T>& list)
    {
        if (!list.empty())
        {
            // We keep the declaration order for this summary; sorting the properties in alphabetical order would be
            // confusing wrt the primary constructor.

            out << nl << "%";
            out << nl << "%   " << name << " Properties:";
            for (const auto& field : list)
            {
                out << nl << "%     " << field->mappedName();
                if (const auto& fieldDoc = field->docComment())
                {
                    const StringList& fieldOverview = fieldDoc->overview();
                    if (!fieldOverview.empty())
                    {
                        out << " - " << getFirstSentence(fieldOverview);
                    }
                }
            }
        }
    }

    // The main doc-comment for a class, struct, exception, enum, enumerator, and const.
    // Per MATLAB conventions, it's the first comment inside the class.
    void writeDocSummary(IceInternal::Output& out, const ContainedPtr& p)
    {
        const string name = p->mappedName();
        // No space and upper-case, per MATLAB conventions.
        out << nl << "%" << toUpper(name);

        const optional<DocComment>& doc = p->docComment();
        StringList docOverview;
        if (doc)
        {
            docOverview = doc->overview();
        }
        if (!docOverview.empty())
        {
            // Use the first line as the summary.
            out << " " << docOverview.front();
            docOverview.pop_front();

            if (!docOverview.empty())
            {
                writeDocLines(out, docOverview, 2); // indent with 2 spaces
            }
        }

        if (EnumPtr en = dynamic_pointer_cast<Enum>(p))
        {
            writePropertiesSummary(out, name, en->enumerators());
        }
        else if (StructPtr st = dynamic_pointer_cast<Struct>(p))
        {
            writeConstructorDoc(out, p->mappedScoped("."), st->dataMembers());
            writePropertiesSummary(out, name, st->dataMembers());
        }
        else if (ClassDefPtr cl = dynamic_pointer_cast<ClassDef>(p))
        {
            writeConstructorDoc(out, p->mappedScoped("."), cl->dataMembers());
            writePropertiesSummary(out, name, cl->dataMembers());
        }
        else if (ExceptionPtr ex = dynamic_pointer_cast<Exception>(p))
        {
            writePropertiesSummary(out, name, ex->dataMembers());
        }

        writeDeprecated(out, doc, p);
        writeSeeAlso(out, doc, p->container());
        writeRemarks(out, doc);
    }

    void writeOpDocSummary(IceInternal::Output& out, const OperationPtr& p, bool async)
    {
        out << nl << "%" << toUpper(p->mappedName() + (async ? "Async" : ""));

        const optional<DocComment>& doc = p->docComment();
        if (doc)
        {
            StringList docOverview = doc->overview();
            if (!docOverview.empty())
            {
                out << " ";
                // TODO: it would be much better to extract and print the first sentence, however, this is currently no
                // helper to remove this first sentence.
                out << docOverview.front();
                docOverview.pop_front();
                writeDocLines(out, docOverview, 2); // indent with 2 spaces
            }
        }

        writeDeprecated(out, doc, p);

        out << nl << "%";
        out << nl << "%   Input Arguments";
        const ParameterList inParams = p->inParameters();
        const ParameterList outParams = p->outParameters();

        const string contextParam = getEscapedParamName(inParams, "context");
        const string returnValueName = getEscapedParamName(outParams, "returnValue");

        for (const auto& inParam : inParams)
        {
            StringList docLines;
            if (doc)
            {
                auto r = doc->parameters().find(inParam->name());
                if (r != doc->parameters().end())
                {
                    docLines = r->second;
                }
            }
            documentArgument(out, inParam, inParam->mappedName(), docLines);
        }
        out << nl << "%     " << contextParam << " - The request context.";
        out << nl << "%       unconfigured dictionary (default) | dictionary(string, string) scalar";

        if (async)
        {
            out << nl << "%";
            out << nl << "%   Output Arguments";
            out << nl << "%     future - A future that will be completed with the result of the invocation.";
            out << nl << "%       Ice.Future scalar";
            out << nl << "%";
            out << nl << "%   See also " << p->mappedName() << ", Ice.Future.";
        }
        else
        {
            if (p->returnsAnyValues())
            {
                out << nl << "%";
                out << nl << "%   Output Arguments";

                // returnValueName was checked and escaped against the _mapped_ parameter names.
                ParameterPtr returnParam = p->returnParameter(returnValueName);
                if (returnParam)
                {
                    StringList docLines;
                    if (doc)
                    {
                        docLines = doc->returns();
                    }
                    documentArgument(out, returnParam, returnValueName, docLines);
                }
                for (const auto& outParam : p->outParameters())
                {
                    StringList docLines;
                    if (doc)
                    {
                        auto r = doc->parameters().find(outParam->name());
                        if (r != doc->parameters().end())
                        {
                            docLines = r->second;
                        }
                    }
                    documentArgument(out, outParam, outParam->mappedName(), docLines);
                }
            }

            ExceptionList exceptions = p->throws();
            if (!exceptions.empty())
            {
                out << nl << "%";
                out << nl << "%   Exceptions";
                for (const auto& exception : exceptions)
                {
                    out << nl << "%     " << exception->mappedScoped(".");
                    if (doc)
                    {
                        StringList docLines;
                        auto r = doc->exceptions().find(exception->name());
                        if (r != doc->exceptions().end())
                        {
                            docLines = r->second;
                        }
                        if (!docLines.empty())
                        {
                            out << " - " << docLines.front();
                            docLines.pop_front();
                            writeDocLines(out, docLines, 6);
                        }
                    }
                }
            }
        }

        writeSeeAlso(out, doc, p->container());
        writeRemarks(out, doc);

        out << nl;
    }

    void writeProxyDocSummary(IceInternal::Output& out, const InterfaceDefPtr& p)
    {
        const string name = p->mappedName() + "Prx";
        out << nl << "%" << toUpper(name);

        const optional<DocComment>& doc = p->docComment();
        StringList docOverview;
        if (doc)
        {
            docOverview = doc->overview();
        }
        if (!docOverview.empty())
        {
            // Use the first line as the summary.
            out << " " << docOverview.front();
            docOverview.pop_front();

            if (!docOverview.empty())
            {
                writeDocLines(out, docOverview, 2); // indent with 2 spaces
            }
        }

        // We document the inherited constructor here because:
        // - documenting the inherited constructor (Ice.ObjectPrx/ObjectPrx) inside Methods is rather confusing, and
        // - we don't want to generate a constructor for documentation purposes because it would introduce a lot of
        //   complexity with multiple inheritance.

        out << nl << "%";
        out << nl << "%   Creation";
        out << nl << "%     Syntax";
        out << nl << "%       prx = " << p->mappedScoped(".") << "Prx(communicator, proxyString)";
        out << nl << "%";
        out << nl << "%     Input Arguments";
        out << nl << "%       communicator - The associated communicator.";
        out << nl << "%         Ice.Communicator scalar";
        out << nl << "%       proxyString - A stringified proxy, such as 'name:tcp -p localhost -p 4061'.";
        out << nl << "%         character vector";
        out << nl << "%";
        out << nl << "%   " << name << " Methods:";
        OperationList ops = p->operations();
        if (!ops.empty())
        {
            // The summary is in alphabetical order.
            ops.sort([](const OperationPtr& a, const OperationPtr& b) { return a->mappedName() < b->mappedName(); });

            for (const auto& op : ops)
            {
                const string opName = op->mappedName();
                const optional<DocComment>& opdoc = op->docComment();
                out << nl << "%     " << opName;
                if (opdoc)
                {
                    const StringList& opdocOverview = opdoc->overview();
                    if (!opdocOverview.empty())
                    {
                        out << " - " << getFirstSentence(opdocOverview);
                    }
                }
                out << nl << "%     " << opName << "Async - An asynchronous " << opName << ".";
            }
        }
        out << nl << "%";
        out << nl << "%   " << name << " Static Methods:";
        out << nl << "%     checkedCast - Creates a new proxy from an existing proxy after checking the target's type.";
        out << nl << "%     uncheckedCast - Creates a new " << name << " from an existing proxy.";

        writeDeprecated(out, doc, p);
        writeSeeAlso(out, doc, p->container());
        writeRemarks(out, doc);
    }

    void declareArgument(IceInternal::Output& out, const ParameterPtr& param)
    {
        out << nl << param->mappedName();

        TypePtr type = param->type();

        // First the dimensions

        bool mustBeScalarOrEmpty = false;
        if (auto builtin = dynamic_pointer_cast<Builtin>(type))
        {
            switch (builtin->kind())
            {
                case Builtin::KindString:
                    out << " (1, :)";
                    break;
                case Builtin::KindObjectProxy:
                case Builtin::KindValue:
                    mustBeScalarOrEmpty = true;
                    break;
                default:
                    out << " (1, 1)";
                    break;
            }
        }
        else if (
            dynamic_pointer_cast<Enum>(type) || dynamic_pointer_cast<Dictionary>(type) ||
            dynamic_pointer_cast<Struct>(type))
        {
            out << " (1, 1)";
        }
        else if (dynamic_pointer_cast<Sequence>(type))
        {
            out << " (1, :)";
        }
        else // proxies and classes
        {
            mustBeScalarOrEmpty = true;
        }

        // We can't specify a type for optional parameter because we can't represent "not set" with the same MATLAB
        // type.
        if (!param->optional())
        {
            if (auto seq = dynamic_pointer_cast<Sequence>(type))
            {
                TypePtr seqType = seq->type();
                if (isMappedToScalar(seqType))
                {
                    // sequence<string> maps to array of string, not to array of char.
                    if (auto builtin = dynamic_pointer_cast<Builtin>(seqType);
                        builtin && builtin->kind() == Builtin::KindString)
                    {
                        out << " string";
                    }
                    else
                    {
                        out << " " << typeToString(seqType);
                    }
                }
                else
                {
                    out << " cell";
                }
            }
            else
            {
                out << " " << typeToString(type);
            }
        }

        if (mustBeScalarOrEmpty)
        {
            // Generate constraint.
            out << " {mustBeScalarOrEmpty}";
        }
    }

    // The implementation of documentArgument() and documentProperty().
    void documentArgumentOrProperty(
        IceInternal::Output& out,
        const string& name,
        const TypePtr& type,
        bool optional,
        StringList docLines,
        bool itemInList)
    {
        auto typeStr = typeToString(type);
        size_t indentation = 0;

        if (itemInList)
        {
            indentation = 4;
            out << nl << "% " << string(indentation, ' ') << name;
        }
        else
        {
            out << nl << "% " << toUpper(name);
        }

        if (!docLines.empty())
        {
            if (itemInList)
            {
                out << " - ";
            }
            else
            {
                out << " ";
            }
            // TODO: it would be much better to extract and print the first sentence, however, this is currently no
            // helper to remove this first sentence.
            out << docLines.front();
            docLines.pop_front();
            writeDocLines(out, docLines, indentation + 2);
        }
        // Always put type description on next line.
        out << nl << "% " << string(indentation + 2, ' ');

        if (auto builtin = dynamic_pointer_cast<Builtin>(type))
        {
            switch (builtin->kind())
            {
                case Builtin::KindString:
                    out << "character vector";
                    break;
                case Builtin::KindObjectProxy:
                case Builtin::KindValue:
                    out << typeStr << " scalar | empty array of " << typeStr;
                    break;
                default:
                    out << typeStr << " scalar";
                    break;
            }
        }
        else if (dynamic_pointer_cast<Enum>(type) || dynamic_pointer_cast<Struct>(type))
        {
            // Struct properties can be empty, but this is a temporary state.
            out << typeStr << " scalar";
        }
        else if (auto seq = dynamic_pointer_cast<Sequence>(type))
        {
            if (isMappedToScalar(seq->type()))
            {
                string elementStr = typeToString(seq->type());
                if (elementStr == "char")
                {
                    elementStr = "string";
                }
                out << elementStr << " vector";
            }
            else
            {
                out << "cell array";
            }
        }
        else if (auto dict = dynamic_pointer_cast<Dictionary>(type))
        {
            auto keyStr = typeToString(dict->keyType());
            if (keyStr == "char")
            {
                keyStr = "string";
            }

            out << keyStr << ", ";
            if (isMappedToScalar(dict->valueType()))
            {
                auto valueStr = typeToString(dict->valueType());
                if (valueStr == "char")
                {
                    valueStr = "string";
                }
                out << valueStr;
            }
            else
            {
                out << "cell";
            }
            out << ") scalar";
        }
        else // proxies and classes
        {
            out << typeStr << " scalar | empty array of " << typeStr;
        }

        if (optional)
        {
            out << " | Ice.Unset";
        }
    }

    void documentArgument(IceInternal::Output& out, const ParameterPtr& param, const string& name, StringList docLines)
    {
        documentArgumentOrProperty(out, name, param->type(), param->optional(), std::move(docLines), true);
    }

    void documentProperty(IceInternal::Output& out, const DataMemberPtr& field)
    {
        const optional<DocComment>& doc = field->docComment();
        documentArgumentOrProperty(
            out,
            toUpper(field->mappedName()),
            field->type(),
            field->optional(),
            doc ? doc->overview() : StringList{},
            false);

        writeDeprecated(out, doc, field);
        writeSeeAlso(out, doc, field->container());
        writeRemarks(out, doc);
    }

    void writeArguments(
        IceInternal::Output& out,
        const string& self,
        const string& proxyType,
        const ParameterList& inParams,
        const string& contextParam)
    {
        out << nl << "arguments";
        out.inc();
        out << nl << self << " (1, 1) " << proxyType;
        for (const auto& param : inParams)
        {
            declareArgument(out, param);
        }
        out << nl << contextParam << " (1, 1) dictionary = dictionary";
        out.dec();
        out << nl << "end";
    }
}

//
// CodeVisitor implementation.
//
CodeVisitor::CodeVisitor(string dir) : _dir(std::move(dir)) {}

bool
CodeVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    const string name = p->mappedName();
    const ClassDefPtr base = p->base();

    openClass(p->mappedScoped("."), _dir);
    IceInternal::Output& out = *_out;

    out << "classdef " << name;
    if (base)
    {
        out << " < " << base->mappedScoped(".");
    }
    else
    {
        out << " < Ice.Value";
    }

    out.inc();
    writeDocSummary(out, p);
    writeGeneratedFrom(out, p->file());

    if (!p->dataMembers().empty())
    {
        out << nl << "properties";
        out.inc();
    }

    return true;
}

void
CodeVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    IceInternal::Output& out = *_out;
    const string name = p->mappedName();
    const string scoped = p->scoped();
    const string self = "obj";
    const ClassDefPtr base = p->base();
    const DataMemberList members = p->dataMembers();
    const DataMemberList allMembers = p->allDataMembers();

    if (!members.empty())
    {
        out.dec();
        out << nl << "end";
    }

    out << nl << "methods";
    out.inc();

    //
    // Constructor
    //
    if (!members.empty()) // else no need to define a constructor: we inherit the base class constructor
    {
        const auto firstMember = *allMembers.begin();

        vector<string> allNames;
        for (const auto& member : allMembers)
        {
            allNames.push_back(member->mappedName());
        }
        out << nl << "function " << self << " = " << name << spar << allNames << epar;
        out.inc();
        if (base)
        {
            const DataMemberList baseMembers = base->allDataMembers();

            out << nl << "if nargin == 0";
            out.inc();
            out << nl << "superArgs = {};";
            out.dec();
            out << nl << "else";
            out.inc();
            out << nl << "assert(nargin == " << allMembers.size() << ", 'Invalid number of arguments');";
            out << nl << "superArgs = ";
            out.spar("{");
            for (const auto& member : baseMembers)
            {
                out << member->mappedName();
            }
            out.epar("};");
            out.dec();
            out << nl << "end";

            out << nl << self << " = " << self << "@" << base->mappedScoped(".") << "(superArgs{:});";

            if (!members.empty())
            {
                // Set this class properties.
                out << nl << "if nargin > 0";
                out.inc();
                for (const auto& member : members)
                {
                    const string memberName = member->mappedName();
                    out << nl << self << "." << memberName << " = " << memberName << ';';
                }
                out.dec();
                out << nl << "end";
            }
        }
        else
        {
            out << nl << "if nargin > 0";
            out.inc();
            out << nl << "assert(nargin == " << allMembers.size() << ", 'Invalid number of arguments');";
            for (const auto& member : allMembers)
            {
                const string memberName = member->mappedName();
                out << nl << self << "." << memberName << " = " << memberName << ';';
            }
            out.dec();
            out << nl << "end";
        }

        out.dec();
        out << nl << "end";
    }

    out << nl << "function id = ice_id(obj)";
    out.inc();
    out << nl << "id = obj.ice_staticId();";
    out.dec();
    out << nl << "end";

    out.dec();
    out << nl << "end";

    DataMemberList convertMembers;
    for (const auto& member : members)
    {
        if (needsConversion(member->type()))
        {
            convertMembers.push_back(member);
        }
    }

    if (!convertMembers.empty())
    {
        out << nl << "methods (Hidden)";
        out.inc();

        out << nl << "function r = iceDelayPostUnmarshal(~)";
        out.inc();
        out << nl << "r = true;";
        out.dec();
        out << nl << "end";
        out << nl << "function icePostUnmarshal(obj)";
        out.inc();
        for (const auto& convertMember : convertMembers)
        {
            string m = "obj." + convertMember->mappedName();
            convertValueType(out, m, m, convertMember->type(), convertMember->optional());
        }
        if (base)
        {
            out << nl << "icePostUnmarshal@" << base->mappedScoped(".") << "(obj);";
        }
        out.dec();
        out << nl << "end";

        out.dec();
        out << nl << "end";
    }

    out << nl << "methods (Access = protected)";
    out.inc();

    const DataMemberList optionalMembers = p->orderedOptionalDataMembers();

    out << nl << "function iceWriteImpl(obj, os)";
    out.inc();
    out << nl << "os.startSlice('" << scoped << "', " << p->compactId() << (!base ? ", true" : ", false") << ");";
    for (const auto& member : members)
    {
        if (!member->optional())
        {
            marshal(out, "os", "obj." + member->mappedName(), member->type(), false, 0);
        }
    }
    for (const auto& optionalMember : optionalMembers)
    {
        marshal(out, "os", "obj." + optionalMember->mappedName(), optionalMember->type(), true, optionalMember->tag());
    }
    out << nl << "os.endSlice();";
    if (base)
    {
        out << nl << "iceWriteImpl@" << base->mappedScoped(".") << "(obj, os);";
    }
    out.dec();
    out << nl << "end";
    out << nl << "function iceReadImpl(obj, is)";
    out.inc();
    out << nl << "is.startSlice();";
    for (const auto& dm : members)
    {
        if (!dm->optional())
        {
            if (dm->type()->isClassType())
            {
                unmarshal(out, "is", "@obj.iceSetProperty_" + dm->mappedName(), dm->type(), false, 0);
            }
            else
            {
                unmarshal(out, "is", "obj." + dm->mappedName(), dm->type(), false, 0);
            }
        }
    }
    for (const auto& dm : optionalMembers)
    {
        assert(!dm->type()->isClassType());
        unmarshal(out, "is", "obj." + dm->mappedName(), dm->type(), true, dm->tag());
    }
    out << nl << "is.endSlice();";
    if (base)
    {
        out << nl << "iceReadImpl@" << base->mappedScoped(".") << "(obj, is);";
    }
    out.dec();
    out << nl << "end";

    // Generate an iceSetProperty_name method for all class fields.
    for (const auto& classField : p->classDataMembers())
    {
        string m = classField->mappedName();
        out << nl << "function iceSetProperty_" << m << "(obj, v)";
        out.inc();
        out << nl << "obj." << m << " = v;";
        out.dec();
        out << nl << "end";
    }

    out.dec();
    out << nl << "end";

    out << nl << "methods (Static)";
    out.inc();
    out << nl << "function id = ice_staticId()";
    out.inc();
    out << nl << "id = '" << scoped << "';";
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";
    out << nl << "properties (Constant, Access = private)";
    out.inc();
    out << nl << "TypeId char = '" << scoped << "'";
    if (p->compactId() != -1)
    {
        out << nl << "CompactId char = '" << p->compactId() << "'";
    }
    out.dec();
    out << nl << "end";

    out.dec();
    out << nl << "end";
    out << nl;

    closeClass();
}

bool
CodeVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    //
    // Generate proxy class.
    //

    const InterfaceList bases = p->bases();
    const string prxName = p->mappedName() + "Prx";
    const string prxAbs = p->mappedScoped(".") + "Prx";

    openClass(prxAbs, _dir);
    IceInternal::Output& out = *_out;

    out << "classdef " << prxName << " < ";
    if (!bases.empty())
    {
        for (auto q = bases.begin(); q != bases.end(); ++q)
        {
            if (q != bases.begin())
            {
                out << " & ";
            }
            out << (*q)->mappedScoped(".") + "Prx";
        }
    }
    else
    {
        out << "Ice.ObjectPrx";
    }

    out.inc();
    writeProxyDocSummary(out, p);
    writeGeneratedFrom(out, p->file());

    if (!p->operations().empty())
    {
        out << nl << "methods";
        out.inc();
    }
    return true;
}

void
CodeVisitor::visitOperation(const OperationPtr& op)
{
    IceInternal::Output& out = *_out;
    const string prxAbs = op->interface()->mappedScoped(".") + "Prx";

    const ParameterList inParams = op->inParameters();
    const ParameterList sortedInParams = op->sortedInParameters();
    const ParameterList outParams = op->outParameters();
    const string returnValueName = getEscapedParamName(outParams, "returnValue");
    const bool returnsMultipleValues = op->returnsMultipleValues();
    const bool returnsAnyValues = op->returnsAnyValues();

    ParameterList returnAndOutParameters = op->outParameters();
    ParameterPtr returnParam = op->returnParameter(returnValueName);
    if (returnParam)
    {
        returnAndOutParameters.push_front(returnParam);
    }

    const bool twowayOnly = op->returnsData();
    const ExceptionList exceptions = op->throws();
    const string self = getEscapedParamName(inParams, "obj");
    const string contextParam = getEscapedParamName(inParams, "context");

    if (!isFirstElement(op))
    {
        out << sp;
    }

    //
    // Synchronous method.
    //
    out << nl << "function ";
    if (returnsMultipleValues)
    {
        out.spar("[");
        for (const auto& param : returnAndOutParameters)
        {
            out << param->mappedName();
        }
        out.epar("]");
        out << " = ";
    }
    else if (returnsAnyValues)
    {
        out << (*returnAndOutParameters.begin())->mappedName() << " = ";
    }
    out << op->mappedName() << spar;

    out << self;
    for (const auto& param : inParams)
    {
        out << param->mappedName();
    }
    out << contextParam;
    out << epar;
    out.inc();

    writeOpDocSummary(out, op, false);
    writeArguments(out, self, prxAbs, inParams, contextParam);

    if (!inParams.empty())
    {
        if (op->format())
        {
            out << nl << "os_ = " << self << ".iceStartWriteParams(" << getFormatType(*op->format()) << ");";
        }
        else
        {
            out << nl << "os_ = " << self << ".iceStartWriteParams([]);";
        }
        for (const auto& param : sortedInParams)
        {
            marshal(out, "os_", param->mappedName(), param->type(), param->optional(), param->tag());
        }
        if (op->sendsClasses())
        {
            out << nl << "os_.writePendingValues();";
        }
        out << nl << self << ".iceEndWriteParams(os_);";
    }

    out << nl;
    if (returnsAnyValues)
    {
        out << "is_ = ";
    }
    out << self << ".iceInvoke('" << op->name() << "', " << getOperationMode(op->mode()) << ", "
        << (twowayOnly ? "true" : "false") << ", " << (inParams.empty() ? "[]" : "os_") << ", "
        << (returnsAnyValues ? "true" : "false");
    if (exceptions.empty())
    {
        out << ", {}";
    }
    else
    {
        out << ", " << prxAbs << "." << op->mappedName() << "_ex_";
    }
    out << ", " << contextParam << ");";

    if (twowayOnly && returnsAnyValues)
    {
        out << nl << "is_.startEncapsulation();";
        //
        // To unmarshal results:
        //
        // * unmarshal all required out parameters
        // * unmarshal the required return value (if any)
        // * unmarshal all optional out parameters (this includes an optional return value)
        //
        ParameterList classParams;
        ParameterList convertParams;
        for (const auto& param : op->sortedReturnAndOutParameters(returnValueName))
        {
            const TypePtr paramType = param->type();
            const string paramName = param->mappedName();
            string paramString;

            if (paramType->isClassType())
            {
                out << nl << paramName << "_h_ = IceInternal.ValueHolder();";
                paramString = "@(v) " + paramName + "_h_.set(v)";
                classParams.push_back(param);
            }
            else
            {
                paramString = paramName;
            }
            unmarshal(out, "is_", paramString, paramType, param->optional(), param->tag());

            if (needsConversion(paramType))
            {
                convertParams.push_back(param);
            }
        }
        if (op->returnsClasses())
        {
            out << nl << "is_.readPendingValues();";
        }
        out << nl << "is_.endEncapsulation();";

        // After calling readPendingValues(), all callback functions have been invoked.
        // Now we need to collect the values.
        for (const auto& param : classParams)
        {
            const string paramName = param->mappedName();
            out << nl << paramName << " = " << paramName << "_h_.value;";
        }
        for (const auto& param : convertParams)
        {
            const string paramName = param->mappedName();
            convertValueType(out, paramName, paramName, param->type(), param->optional());
        }
    }

    out.dec();
    out << nl << "end";

    //
    // Asynchronous method.
    //
    out << sp;
    out << nl << "function future = " << op->mappedName() << "Async" << spar;
    out << self;
    for (const auto& param : inParams)
    {
        out << param->mappedName();
    }
    out << contextParam;
    out << epar;
    out.inc();

    writeOpDocSummary(out, op, true);
    writeArguments(out, self, prxAbs, inParams, contextParam);

    if (!inParams.empty())
    {
        if (op->format())
        {
            out << nl << "os_ = " << self << ".iceStartWriteParams(" << getFormatType(*op->format()) << ");";
        }
        else
        {
            out << nl << "os_ = " << self << ".iceStartWriteParams([]);";
        }
        for (const auto& param : sortedInParams)
        {
            marshal(out, "os_", param->mappedName(), param->type(), param->optional(), param->tag());
        }
        if (op->sendsClasses())
        {
            out << nl << "os_.writePendingValues();";
        }
        out << nl << self << ".iceEndWriteParams(os_);";
    }

    if (twowayOnly && returnsAnyValues)
    {
        out << nl << "function varargout = unmarshal(is_)";
        out.inc();
        out << nl << "is_.startEncapsulation();";
        //
        // To unmarshal results:
        //
        // * unmarshal all required out parameters
        // * unmarshal the required return value (if any)
        // * unmarshal all optional out parameters (this includes an optional return value)
        //
        for (const auto& param : op->sortedReturnAndOutParameters(returnValueName))
        {
            const TypePtr paramType = param->type();
            const string paramName = param->mappedName();
            string paramString;
            if (paramType->isClassType())
            {
                out << nl << paramName << " = IceInternal.ValueHolder();";
                paramString = "@(v) " + paramName + ".set(v)";
            }
            else
            {
                paramString = paramName;
            }
            unmarshal(out, "is_", paramString, paramType, param->optional(), param->tag());
        }
        if (op->returnsClasses())
        {
            out << nl << "is_.readPendingValues();";
        }
        out << nl << "is_.endEncapsulation();";
        int i = 1;
        for (const auto& param : returnAndOutParameters)
        {
            const string paramName = param->mappedName();
            if (param->type()->isClassType())
            {
                out << nl << "varargout{" << i << "} = " << paramName << ".value;";
            }
            else if (needsConversion(param->type()))
            {
                ostringstream dest;
                dest << "varargout{" << i << "}";
                convertValueType(out, dest.str(), paramName, param->type(), param->optional());
            }
            else
            {
                out << nl << "varargout{" << i << "} = " << paramName << ';';
            }
            i++;
        }
        out.dec();
        out << nl << "end";
    }

    out << nl << "future = " << self << ".iceInvokeAsync('" << op->name() << "', " << getOperationMode(op->mode())
        << ", " << (twowayOnly ? "true" : "false") << ", " << (inParams.empty() ? "[]" : "os_") << ", "
        << returnAndOutParameters.size() << ", " << (twowayOnly && returnsAnyValues ? "@unmarshal" : "[]");
    if (exceptions.empty())
    {
        out << ", {}";
    }
    else
    {
        out << ", " << prxAbs << "." << op->mappedName() << "_ex_";
    }
    out << ", " << contextParam << ");";

    out.dec();
    out << nl << "end";
}

void
CodeVisitor::visitInterfaceDefEnd(const InterfaceDefPtr& p)
{
    IceInternal::Output& out = *_out;
    const string prxAbs = p->mappedScoped(".") + "Prx";

    if (!p->operations().empty())
    {
        out.dec();
        out << nl << "end";
        out << sp;
    }

    out << nl << "methods (Static)";
    out.inc();
    out << nl << "function id = ice_staticId()";
    out.inc();
    out << nl << "id = '" << p->scoped() << "';";
    out.dec();
    out << nl << "end";
    out << sp;
    out << nl << "function r = ice_read(is)";
    out.inc();
    out << nl << "r = is.readProxy('" << prxAbs << "');";
    out.dec();
    out << nl << "end";
    out << sp;
    out << nl << "function r = checkedCast(p, varargin)";
    out.inc();
    out << nl
        << "%CHECKEDCAST Creates a new proxy from an existing proxy after confirming the target object implements "
           "Slice interface "
        << p->scoped() << ".";
    out << nl << "%";
    out << nl << "%   Input Arguments";
    out << nl << "%     p - The source proxy.";
    out << nl << "%       Ice.ObjectPrx scalar | empty array of Ice.ObjectPrx";
    out << nl << "%     facet - The desired facet (optional).";
    out << nl << "%       character vector";
    out << nl << "%     context - The request context (optional).";
    out << nl << "%       dictionary(string, string) scalar";
    out << nl << "%";
    out << nl << "%   Output Arguments";
    out << nl << "%     r - A new " << prxAbs << " scalar if the target object implements Slice interface ";
    out << nl << "%       " << p->scoped() << "; otherwise, an empty array of " << prxAbs << ".";
    out << nl << "%";
    out << nl << "arguments";
    out.inc();
    out << nl << "p Ice.ObjectPrx {mustBeScalarOrEmpty}";
    out.dec();
    out << nl << "end";
    out << nl << "arguments (Repeating)";
    out.inc();
    out << nl << "varargin % facet or context, or both, or neither";
    out.dec();
    out << nl << "end";
    out << nl << "r = Ice.ObjectPrx.iceCheckedCast(p, " << prxAbs << ".ice_staticId(), '" << prxAbs
        << "', varargin{:});";
    out.dec();
    out << nl << "end";
    out << sp;
    out << nl << "function r = uncheckedCast(p, varargin)";
    out.inc();
    out << nl << "%UNCHECKEDCAST Creates a new " << prxAbs << " from an existing proxy.";
    out << nl << "%";
    out << nl << "%   Input Arguments";
    out << nl << "%     p - The source proxy.";
    out << nl << "%       Ice.ObjectPrx scalar | empty array of Ice.ObjectPrx";
    out << nl << "%     facet - The desired facet (optional).";
    out << nl << "%       character vector";
    out << nl << "%";
    out << nl << "%   Output Arguments";
    out << nl << "%     r - A new " << prxAbs << " scalar, or an empty array when p is an empty array.";
    out << nl << "%";
    out << nl << "arguments";
    out.inc();
    out << nl << "p Ice.ObjectPrx {mustBeScalarOrEmpty}";
    out.dec();
    out << nl << "end";
    out << nl << "arguments (Repeating)";
    out.inc();
    out << nl << "varargin (1, :) char";
    out.dec();
    out << nl << "end";
    out << nl << "r = Ice.ObjectPrx.iceUncheckedCast(p, '" << prxAbs << "', varargin{:});";
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    // The constructor is inherited, even with multiple inheritance.

    bool hasExceptions = false;
    const OperationList ops = p->operations();
    for (const auto& op : ops)
    {
        if (!op->throws().empty())
        {
            hasExceptions = true;
            break;
        }
    }

    if (hasExceptions)
    {
        //
        // Generate a constant property for each operation that throws user exceptions. The property is
        // a cell array containing the class names of the exceptions.
        //
        out << sp;
        out << nl << "properties (Constant, Access = private)";
        out.inc();
        for (const auto& op : ops)
        {
            // Arrange exceptions into most-derived to least-derived order. If we don't
            // do this, a base exception handler can appear before a derived exception
            // handler, causing compiler warnings and resulting in the base exception
            // being marshaled instead of the derived exception.
            ExceptionList exceptions = op->throws();
            exceptions.sort(Slice::DerivedToBaseCompare());

            if (!exceptions.empty())
            {
                out << nl << op->mappedName() << "_ex_ = { ";
                for (auto e = exceptions.begin(); e != exceptions.end(); ++e)
                {
                    if (e != exceptions.begin())
                    {
                        out << ", ";
                    }
                    out << "'" + (*e)->mappedScoped(".") + "'";
                }
                out << " }";
            }
        }
        out.dec();
        out << nl << "end";
    }

    out.dec();
    out << nl << "end";
    out << nl;

    closeClass();
}

bool
CodeVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    const string name = p->mappedName();
    const string abs = p->mappedScoped(".");

    openClass(abs, _dir);
    IceInternal::Output& out = *_out;

    const ExceptionPtr base = p->base();

    out << "classdef " << name;
    if (base)
    {
        out << " < " << base->mappedScoped(".");
    }
    else
    {
        out << " < Ice.UserException";
    }

    out.inc();
    writeDocSummary(out, p);
    writeGeneratedFrom(out, p->file());

    if (!p->dataMembers().empty())
    {
        out << nl << "properties";
        out.inc();
    }

    return true;
}

void
CodeVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    const string name = p->mappedName();
    const string abs = p->mappedScoped(".");

    IceInternal::Output& out = *_out;

    const ExceptionPtr base = p->base();
    const DataMemberList members = p->dataMembers();

    DataMemberList convertMembers;
    for (const auto& member : members)
    {
        if (needsConversion(member->type()))
        {
            convertMembers.push_back(member);
        }
    }

    if (!members.empty())
    {
        out.dec();
        out << nl << "end";
    }

    out << nl << "methods";
    out.inc();

    const string self = "obj";

    //
    // Constructor
    //
    out << nl << "function " << self << " = " << name << spar << "errID" << "msg" << epar;
    out.inc();
    string errID = abs;
    const string& msg = abs;
    //
    // The ID argument must use colon separators.
    //
    string::size_type pos = errID.find('.');
    assert(pos != string::npos);
    while (pos != string::npos)
    {
        errID[pos] = ':';
        pos = errID.find('.', pos);
    }

    // DefaultSliceLoader always create a user exception with no argument. A derived exception class created with no
    // argument gives two arguments to its base class constructor. That's why the constructor needs to accept two
    // arguments as well.

    out << nl << "if nargin == 0";
    out.inc();
    out << nl << "errID = '" << errID << "';";
    out << nl << "msg = '" << msg << "';";
    out.dec();
    out << nl << "else";
    out.inc();
    out << nl << "assert(nargin == 2, 'Invalid number of arguments');";
    out.dec();
    out << nl << "end";

    if (!base)
    {
        out << nl << self << " = " << self << "@Ice.UserException" << spar << "errID" << "msg" << epar << ';';
    }
    else
    {
        out << nl << self << " = " << self << "@" << base->mappedScoped(".") << spar << "errID" << "msg" << epar << ';';
    }
    out.dec();
    out << nl << "end";

    out << nl << "function id = ice_id(~)";
    out.inc();
    out << nl << "id = '" << p->scoped() << "';";
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    const DataMemberList classMembers = p->classDataMembers();
    if (!classMembers.empty() || !convertMembers.empty())
    {
        out << nl << "methods (Hidden)";
        out.inc();
        out << nl << "function obj = icePostUnmarshal(obj)";
        out.inc();
        for (const auto& classMember : classMembers)
        {
            string m = classMember->mappedName();
            out << nl << "obj." << m << " = obj." << m << ".value;";
        }
        for (const auto& convertMember : convertMembers)
        {
            string m = "obj." + convertMember->mappedName();
            convertValueType(out, m, m, convertMember->type(), convertMember->optional());
        }
        if (base && base->usesClasses())
        {
            out << nl << "obj = icePostUnmarshal@" << base->mappedScoped(".") << "(obj);";
        }
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";
    }

    out << nl << "methods (Access = protected)";
    out.inc();

    out << nl << "function obj = iceReadImpl(obj, is)";
    out.inc();
    out << nl << "is.startSlice();";
    for (const auto& dm : members)
    {
        const string m = dm->mappedName();
        if (!dm->optional())
        {
            if (dm->type()->isClassType())
            {
                out << nl << "obj." << m << " = IceInternal.ValueHolder();";
                unmarshal(out, "is", "@(v) obj." + m + ".set(v)", dm->type(), false, 0);
            }
            else
            {
                unmarshal(out, "is", "obj." + m, dm->type(), false, 0);
            }
        }
    }
    const DataMemberList optionalMembers = p->orderedOptionalDataMembers();
    for (const auto& dm : optionalMembers)
    {
        const string m = dm->mappedName();
        assert(!dm->type()->isClassType()); // guaranteed by the parser.
        unmarshal(out, "is", "obj." + m, dm->type(), true, dm->tag());
    }
    out << nl << "is.endSlice();";
    if (base)
    {
        out << nl << "obj = iceReadImpl@" << base->mappedScoped(".") << "(obj, is);";
    }
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    out << nl << "properties (Constant, Access = private)";
    out.inc();
    out << nl << "TypeId char = '" << p->scoped() << "'";
    out.dec();
    out << nl << "end";

    out.dec();
    out << nl << "end";
    out << nl;

    closeClass();
}

bool
CodeVisitor::visitStructStart(const StructPtr& p)
{
    const string name = p->mappedName();
    const string abs = p->mappedScoped(".");

    openClass(abs, _dir);
    IceInternal::Output& out = *_out;

    out << "classdef (Sealed) " << name;

    out.inc();
    writeDocSummary(out, p);
    writeGeneratedFrom(out, p->file());

    out << nl << "properties";
    out.inc();
    return true;
}

void
CodeVisitor::visitStructEnd(const StructPtr& p)
{
    IceInternal::Output& out = *_out;
    const string name = p->mappedName();
    const string abs = p->mappedScoped(".");
    vector<string> memberNames;
    const DataMemberList members = p->dataMembers();
    const DataMemberList classMembers = p->classDataMembers();
    DataMemberList convertMembers;

    for (const auto& q : members)
    {
        memberNames.push_back(q->mappedName());

        if (needsConversion(q->type()))
        {
            convertMembers.push_back(q);
        }
    }

    out.dec();
    out << nl << "end";

    out << nl << "methods";
    out.inc();
    const string self = "obj";
    out << nl << "function " << self << " = " << name << spar << memberNames << epar;
    out.inc();
    // We rely on the default values when nargin is 0.
    out << nl << "if nargin > 0";
    out.inc();
    out << nl << "assert(nargin == " << members.size() << ", 'Invalid number of arguments');";
    for (const auto& memberName : memberNames)
    {
        out << nl << self << "." << memberName << " = " << memberName << ';';
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

    if (!convertMembers.empty() || !classMembers.empty())
    {
        out << nl << "function obj = ice_convert(obj)";
        out.inc();
        convertStruct(out, p, "obj");
        out.dec();
        out << nl << "end";
    }

    out.dec();
    out << nl << "end";

    out << nl << "methods (Static)";
    out.inc();
    out << nl << "function r = ice_read(is)";
    out.inc();
    out << nl << "r = " << abs << "();";
    unmarshalStruct(out, p, "r");
    out.dec();
    out << nl << "end";

    out << nl << "function ice_write(os, v)";
    out.inc();
    out << nl << "if isempty(v)";
    out.inc();
    out << nl << "v = " << abs << "();";
    out.dec();
    out << nl << "end";
    for (const auto& member : members)
    {
        marshal(out, "os", "v." + member->mappedName(), member->type(), false, 0);
    }
    out.dec();
    out << nl << "end";

    if (!p->usesClasses())
    {
        out << nl << "function r = ice_readOpt(is, tag)";
        out.inc();
        out << nl << "if is.readOptional(tag, " << getOptionalFormat(p) << ")";
        out.inc();
        if (p->isVariableLength())
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

        out << nl << "function ice_writeOpt(os, tag, v)";
        out.inc();
        out << nl << "if v ~= Ice.Unset && os.writeOptional(tag, " << getOptionalFormat(p) << ")";
        out.inc();
        if (p->isVariableLength())
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
    }

    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";
    out << nl;

    closeClass();
}

void
CodeVisitor::visitDataMember(const DataMemberPtr& p)
{
    IceInternal::Output& out = *_out;

    if (!isFirstElement(p))
    {
        out << nl; // blank line between properties
    }

    documentProperty(out, p);
    out << nl << p->mappedName();

    TypePtr type = p->type();

    // First the dimensions:
    // - (1, 1) for simple scalar types, enums and dictionaries.
    // - (1, :) for strings and sequences.
    // - no dimensions for proxies, classes, structs because we add later the {mustBeScalarOrEmpty} constraint.

    bool mustBeScalarOrEmpty = false;
    if (auto builtin = dynamic_pointer_cast<Builtin>(type))
    {
        if (builtin->kind() < Builtin::KindString)
        {
            out << " (1, 1)";
        }
        else if (builtin->kind() == Builtin::KindString)
        {
            out << " (1, :)";
        }
        else
        {
            mustBeScalarOrEmpty = true;
        }
    }
    else if (dynamic_pointer_cast<Enum>(type) || dynamic_pointer_cast<Dictionary>(type))
    {
        out << " (1, 1)";
    }
    else if (dynamic_pointer_cast<Sequence>(type))
    {
        out << " (1, :)";
    }
    else // proxies, classes, structs
    {
        mustBeScalarOrEmpty = true;
    }

    // We can't specify a type for optional fields because we can't represent "not set" with the same MATLAB type.
    // For some types, we could use an empty array, but this does not work for sequences mapped to arrays or cells,
    // nor does it work for dictionaries.
    //
    // We also can't specify a type for class fields (in structs and exceptions, because we convert them in place;
    // we do the same for class fields in classes for consistency). Likewise, we can't specify a type for fields
    // that use classes (but are not classes), since we convert them in place.
    if (!p->optional() && !type->usesClasses())
    {
        if (auto seq = dynamic_pointer_cast<Sequence>(type))
        {
            if (isMappedToScalar(seq->type()))
            {
                out << " " << typeToString(seq->type());
            }
            else
            {
                out << " cell";
            }
        }
        else
        {
            out << " " << typeToString(type);
        }
    }

    if (mustBeScalarOrEmpty)
    {
        // Generate constraint.
        out << " {mustBeScalarOrEmpty}";
    }

    // Specify the default value.
    string defaultValueStr = defaultValue(p);
    if (!defaultValueStr.empty())
    {
        out << " = " << defaultValueStr;
    }
}

void
CodeVisitor::visitSequence(const SequencePtr& p)
{
    const TypePtr content = p->type();

    const BuiltinPtr b = dynamic_pointer_cast<Builtin>(content);
    if (b)
    {
        switch (b->kind())
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
            case Builtin::KindObjectProxy:
            case Builtin::KindValue:
            {
                break;
            }
        }
    }

    EnumPtr enumContent = dynamic_pointer_cast<Enum>(content);
    SequencePtr seqContent = dynamic_pointer_cast<Sequence>(content);
    StructPtr structContent = dynamic_pointer_cast<Struct>(content);
    DictionaryPtr dictContent = dynamic_pointer_cast<Dictionary>(content);

    const string abs = p->mappedScoped(".");
    const bool cls = content->isClassType();
    const bool proxy = isProxy(content);
    const bool convert = needsConversion(content);

    openClass(abs, _dir);
    IceInternal::Output& out = *_out;

    out << nl << "classdef (Hidden) " << p->mappedName();
    out.inc();
    out << nl << "%" << toUpper(p->mappedName()) << " Marshaling and unmarshaling support code for sequence<"
        << getTypeScopedName(content) << ">.";
    writeGeneratedFrom(out, p->file());
    out << nl << "methods (Static)";
    out.inc();

    out << nl << "function write(os, seq)";
    out.inc();
    out << nl << "sz = length(seq);";
    out << nl << "os.writeSize(sz);";
    out << nl << "for i = 1:sz";
    out.inc();
    //
    // Aside from the primitive types, only enum and struct sequences are mapped to arrays. The rest are mapped
    // to cell arrays. We can't use the same subscript syntax for both.
    //
    if (enumContent || structContent)
    {
        marshal(out, "os", "seq(i)", content, false, 0);
    }
    else
    {
        marshal(out, "os", "seq{i}", content, false, 0);
    }
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    out << nl << "function r = read(is)";
    out.inc();
    out << nl << "sz = is.readSize();";
    if (cls)
    {
        //
        // For a sequence<class>, read() returns an instance of IceInternal.CellArrayHandle that we later replace
        // with the cell array. See convert().
        //
        out << nl << "if sz == 0";
        out.inc();
        out << nl << "r = {};";
        out.dec();
        out << nl << "else";
        out.inc();
        out << nl << "r = IceInternal.CellArrayHandle();";
        out << nl << "r.array = cell(1, sz);";
        out << nl << "for i = 1:sz";
        out.inc();
        //
        // Ice.CellArrayHandle defines a set() method that we call from the lambda.
        //
        unmarshal(out, "is", "@(v) r.set(i, v)", content, false, 0);
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";
    }
    else if (dictContent || seqContent || proxy)
    {
        //
        // These types require a cell array.
        //
        out << nl << "if sz == 0";
        out.inc();
        out << nl << "r = {};";
        out.dec();
        out << nl << "else";
        out.inc();
        out << nl << "r = cell(1, sz);";
        out << nl << "for i = 1:sz";
        out.inc();
        unmarshal(out, "is", "r{i}", content, false, 0);
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";
    }
    else if (enumContent)
    {
        const EnumeratorList enumerators = enumContent->enumerators();
        out << nl << "r = " << enumContent->mappedScoped(".") << ".empty();";
        out << nl << "if sz > 0";
        out.inc();
        out << nl << "r(1, sz) = " << (*enumerators.begin())->mappedScoped(".") << ";";
        out << nl << "for i = 1:sz";
        out.inc();
        unmarshal(out, "is", "r(i)", content, false, 0);
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";
    }
    else if (structContent)
    {
        //
        // The most efficient way to build a sequence of structs is to pre-allocate the array using the
        // syntax "arr(1, sz) = Type()". Additionally, we also have to inline the unmarshaling code for
        // the struct members.
        //
        out << nl << "r = " << structContent->mappedScoped(".") << ".empty();";
        out << nl << "if sz > 0";
        out.inc();
        out << nl << "r(1, sz) = " << structContent->mappedScoped(".") << "();";
        out << nl << "for i = 1:sz";
        out.inc();
        unmarshalStruct(out, structContent, "r(i)");
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";
    }
    else
    {
        assert(false);
    }
    out.dec();
    out << nl << "end";

    if (!p->type()->usesClasses())
    {
        out << nl << "function writeOpt(os, tag, seq)";
        out.inc();
        out << nl << "if seq ~= Ice.Unset && os.writeOptional(tag, " << getOptionalFormat(p) << ")";
        out.inc();
        if (p->type()->isVariableLength())
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
            if (sz > 1)
            {
                out << nl << "len = length(seq);";
                out << nl << "if len > 254";
                out.inc();
                out << nl << "os.writeSize(len * " << sz << " + 5);";
                out.dec();
                out << nl << "else";
                out.inc();
                out << nl << "os.writeSize(len * " << sz << " + 1);";
                out.dec();
                out << nl << "end";
            }
            out << nl << abs << ".write(os, seq);";
        }
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";

        out << nl << "function r = readOpt(is, tag)";
        out.inc();
        out << nl << "if is.readOptional(tag, " << getOptionalFormat(p) << ")";
        out.inc();
        if (p->type()->isVariableLength())
        {
            out << nl << "is.skip(4);";
        }
        else if (p->type()->minWireSize() > 1)
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
    }

    if (cls || convert)
    {
        out << nl << "function r = convert(seq)";
        out.inc();
        if (cls)
        {
            out << nl << "if isempty(seq)";
            out.inc();
            out << nl << "r = seq;";
            out.dec();
            out << nl << "else";
            out.inc();
            out << nl << "r = seq.array;";
            out.dec();
            out << nl << "end";
        }
        else
        {
            assert(structContent || seqContent || dictContent);
            if (structContent)
            {
                //
                // Inline the conversion.
                //
                out << nl << "r = seq;";
                out << nl << "for i = 1:length(seq)";
                out.inc();
                convertStruct(out, structContent, "r(i)");
                out.dec();
                out << nl << "end";
            }
            else
            {
                out << nl << "sz = length(seq);";
                out << nl << "if sz > 0";
                out.inc();
                out << nl << "r = cell(1, sz);";
                out << nl << "for i = 1:sz";
                out.inc();
                convertValueType(out, "r{i}", "seq{i}", content, false);
                out << nl << "end";
                out.dec();
                out.dec();
                out << nl << "else";
                out.inc();
                out << nl << "r = seq;";
                out.dec();
                out << nl << "end";
            }
        }
        out.dec();
        out << nl << "end";
    }

    out.dec();
    out << nl << "end";

    out.dec();
    out << nl << "end";
    out << nl;

    closeClass();
}

void
CodeVisitor::visitDictionary(const DictionaryPtr& p)
{
    const TypePtr key = p->keyType();
    const TypePtr value = p->valueType();
    const bool cls = value->isClassType();
    const bool scalarValue = isMappedToScalar(value);
    const bool convert = needsConversion(value);

    const string name = p->mappedName();
    const string abs = p->mappedScoped(".");
    const string self = "obj";

    openClass(abs, _dir);
    IceInternal::Output& out = *_out;

    out << nl << "classdef (Hidden) " << name;
    out.inc();
    out << nl << "%" << toUpper(p->mappedName()) << " Marshaling and unmarshaling support code for dictionary<"
        << getTypeScopedName(key) << ", " << getTypeScopedName(value) << ">.";
    writeGeneratedFrom(out, p->file());
    out << nl << "methods (Access = private)";
    out.inc();
    //
    // Declare a private constructor so that programs can't instantiate this type. They need to use new().
    //
    out << nl << "function " << self << " = " << name << "()";
    out.inc();
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";
    out << nl << "methods (Static)";
    out.inc();

    out << nl << "function write(os, d)";
    out.inc();
    out << nl << "if isempty(d)";
    out.inc();
    out << nl << "os.writeSize(0);";
    out.dec();
    out << nl << "else";
    out.inc();

    out << nl << "sz = d.numEntries;";
    out << nl << "os.writeSize(sz);";
    out << nl << "entries = d.entries;";
    out << nl << "for i = 1:sz";
    out.inc();

    marshal(out, "os", "entries{i, 1}", key, false, 0);

    if (scalarValue)
    {
        marshal(out, "os", "entries{i, 2}", value, false, 0);
    }
    else
    {
        marshal(out, "os", "entries{i, 2}{1, 1}", value, false, 0); // "unwrap" the cell array
    }

    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    out << nl << "function r = read(is)";
    out.inc();
    out << nl << "sz = is.readSize();";

    string valueType = scalarValue ? typeToString(value) : "cell";

    // Can be a temporary dictionary that needs to be converted later.
    out << nl << "r = configureDictionary('" << typeToString(key) << "', '" << valueType << "');";
    out << nl << "for i = 1:sz";
    out.inc();

    unmarshal(out, "is", "k", key, false, 0);

    if (cls)
    {
        out << nl << "v = IceInternal.ValueHolder();";
        unmarshal(out, "is", "@(v_) v.set(v_)", value, false, 0);
    }
    else
    {
        unmarshal(out, "is", "v", value, false, 0);
    }

    if (scalarValue)
    {
        out << nl << "r(k) = v;";
    }
    else
    {
        out << nl << "r{k} = v;"; // cell array value
    }

    out.dec();
    out << nl << "end";
    out.dec();
    out << nl << "end";

    if (!p->valueType()->usesClasses())
    {
        out << nl << "function writeOpt(os, tag, d)";
        out.inc();
        // We can't compare against Ice.Unset because dictionary doesn't implement equality.
        out << nl << "if isa(d, 'dictionary') && os.writeOptional(tag, " << getOptionalFormat(p) << ")";
        out.inc();
        if (key->isVariableLength() || value->isVariableLength())
        {
            out << nl << "pos = os.startSize();";
            out << nl << abs << ".write(os, d);";
            out << nl << "os.endSize(pos);";
        }
        else
        {
            const size_t sz = key->minWireSize() + value->minWireSize();
            out << nl << "len = d.numEntries;";
            out << nl << "if len > 254";
            out.inc();
            out << nl << "os.writeSize(len * " << sz << " + 5);";
            out.dec();
            out << nl << "else";
            out.inc();
            out << nl << "os.writeSize(len * " << sz << " + 1);";
            out.dec();
            out << nl << "end";
            out << nl << abs << ".write(os, d);";
        }
        out.dec();
        out << nl << "end";
        out.dec();
        out << nl << "end";

        out << nl << "function r = readOpt(is, tag)";
        out.inc();
        out << nl << "if is.readOptional(tag, " << getOptionalFormat(p) << ")";
        out.inc();
        if (key->isVariableLength() || value->isVariableLength())
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
    }

    if (cls || convert)
    {
        out << nl << "function r = convert(d)";
        out.inc();
        out << nl << "r = d;"; // rewrite input argument
        out << nl << "keys = d.keys;";
        out << nl << "values = d.values;";
        out << nl << "for i = 1:d.numEntries";
        out.inc();
        out << nl << "k = keys(i);";

        if (scalarValue)
        {
            out << nl << "v = values(i);";
        }
        else
        {
            out << nl << "v = values{i};";
        }

        if (cls)
        {
            // Each entry has a temporary ValueHolder that we need to replace with the actual value wrapped in a cell.
            out << nl << "r{k} = v.value;";
        }
        else if (scalarValue)
        {
            convertValueType(out, "r(k)", "v", value, false);
        }
        else
        {
            convertValueType(out, "r{k}", "v", value, false); // cell value syntax
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

    closeClass();
}

void
CodeVisitor::visitEnum(const EnumPtr& p)
{
    const string abs = p->mappedScoped(".");
    const EnumeratorList enumerators = p->enumerators();

    openClass(abs, _dir);
    IceInternal::Output& out = *_out;

    out << "classdef " << p->mappedName();
    if (p->maxValue() <= 255)
    {
        out << " < uint8";
    }
    else
    {
        out << " < int32";
    }

    out.inc();
    writeDocSummary(out, p);
    writeGeneratedFrom(out, p->file());

    out << nl << "enumeration";
    out.inc();
    for (const auto& enumerator : enumerators)
    {
        if (!isFirstElement(enumerator))
        {
            out << nl; // blank line between enumerators
        }

        writeDocSummary(out, enumerator);
        out << nl << enumerator->mappedName() << " (" << enumerator->value() << ")";
    }
    out.dec();
    out << nl << "end";

    out << nl << "methods (Static)";
    out.inc();

    out << nl << "function ice_write(os, v)";
    out.inc();
    out << nl << "if isempty(v)";
    out.inc();
    const string firstEnum = enumerators.front()->mappedName();
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

    out << nl << "function ice_writeOpt(os, tag, v)";
    out.inc();
    out << nl << "if v ~= Ice.Unset && os.writeOptional(tag, " << getOptionalFormat(p) << ")";
    out.inc();
    out << nl << abs << ".ice_write(os, v);";
    out.dec();
    out << nl << "end";
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

    out << nl << "function r = ice_getValue(v)";
    out.inc();
    out << nl << "switch v";
    out.inc();
    for (const auto& enumerator : enumerators)
    {
        out << nl << "case " << enumerator->value();
        out.inc();
        out << nl << "r = " << abs << "." << enumerator->mappedName() << ";";
        out.dec();
    }
    out << nl << "otherwise";
    out.inc();
    out << nl << "throw(Ice.MarshalException(sprintf('enumerator value %d is out of range', v)));";
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
    closeClass();
}

void
CodeVisitor::visitConst(const ConstPtr& p)
{
    openClass(p->mappedScoped("."), _dir);
    IceInternal::Output& out = *_out;

    out << "classdef " << p->mappedName();
    out.inc();
    writeDocSummary(out, p);
    writeGeneratedFrom(out, p->file());

    out << nl << "properties (Constant)";
    out.inc();
    out << nl << "value " << typeToString(p->type()) << " = " << constantValue(p->type(), p->valueType(), p->value());
    out.dec();
    out << nl << "end";

    out.dec();
    out << nl << "end";
    out << nl;
    closeClass();
}

void
CodeVisitor::openClass(const string& cls, const string& prefix)
{
    string::size_type pos = cls.rfind('.');
    // The generated classes are always in a package corresponding to the Slice module.
    assert(pos != string::npos);
    string package = cls.substr(0, pos);
    string file = cls.substr(pos + 1) + ".m";

    vector<string> packageParts;
    IceInternal::splitString(string_view{package}, ".", packageParts);

    // The package path is of the form "+part1/+part2/+part3/..." where each part is a package name
    // corresponding to a Slice module.
    string packagePath;
    for (const auto& part : packageParts)
    {
        packagePath += '+' + part + '/';
    }
    createPackagePath(packagePath, prefix);

    string path = (prefix.empty() ? "" : prefix + "/") + packagePath + '/' + file;

    _out = make_unique<IceInternal::Output>();
    _out->open(path);
    if (!_out->isOpen())
    {
        ostringstream os;
        os << "cannot open file '" << path << "': " << IceInternal::lastErrorToString();
        throw FileException(os.str());
    }
    FileTracker::instance()->addFile(path);
}

void
CodeVisitor::closeClass()
{
    if (_out)
    {
        _out->close();
        _out.reset();
    }
}

string
CodeVisitor::getOperationMode(Slice::Operation::Mode mode)
{
    switch (mode)
    {
        case Operation::Normal:
            return "0";
        case Operation::Idempotent:
            return "2";
        default:
            return "???";
    }
}

string
CodeVisitor::getOptionalFormat(const TypePtr& type)
{
    return "Ice.OptionalFormat." + type->getOptionalFormat();
}

string
CodeVisitor::getFormatType(FormatType type)
{
    switch (type)
    {
        case CompactFormat:
            return "Ice.FormatType.CompactFormat";
        case SlicedFormat:
            return "Ice.FormatType.SlicedFormat";
        default:
            assert(false);
            return "???";
    }
}

void
CodeVisitor::marshal(
    IceInternal::Output& out,
    const string& stream,
    const string& v,
    const TypePtr& type,
    bool optional,
    int32_t tag)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindByte:
            {
                if (optional)
                {
                    out << nl << stream << ".writeByteOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeByte(" << v << ");";
                }
                return;
            }
            case Builtin::KindBool:
            {
                if (optional)
                {
                    out << nl << stream << ".writeBoolOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeBool(" << v << ");";
                }
                return;
            }
            case Builtin::KindShort:
            {
                if (optional)
                {
                    out << nl << stream << ".writeShortOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeShort(" << v << ");";
                }
                return;
            }
            case Builtin::KindInt:
            {
                if (optional)
                {
                    out << nl << stream << ".writeIntOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeInt(" << v << ");";
                }
                return;
            }
            case Builtin::KindLong:
            {
                if (optional)
                {
                    out << nl << stream << ".writeLongOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeLong(" << v << ");";
                }
                return;
            }
            case Builtin::KindFloat:
            {
                if (optional)
                {
                    out << nl << stream << ".writeFloatOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeFloat(" << v << ");";
                }
                return;
            }
            case Builtin::KindDouble:
            {
                if (optional)
                {
                    out << nl << stream << ".writeDoubleOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeDouble(" << v << ");";
                }
                return;
            }
            case Builtin::KindString:
            {
                if (optional)
                {
                    out << nl << stream << ".writeStringOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeString(" << v << ");";
                }
                return;
            }
            case Builtin::KindValue:
            {
                // Handled by isClassType below.
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if (optional)
                {
                    out << nl << stream << ".writeProxyOpt(" << tag << ", " << v << ");";
                }
                else
                {
                    out << nl << stream << ".writeProxy(" << v << ");";
                }
                return;
            }
        }
    }

    InterfaceDeclPtr prx = dynamic_pointer_cast<InterfaceDecl>(type);
    if (prx)
    {
        if (optional)
        {
            out << nl << stream << ".writeProxyOpt(" << tag << ", " << v << ");";
        }
        else
        {
            out << nl << stream << ".writeProxy(" << v << ");";
        }
        return;
    }

    if (type->isClassType())
    {
        assert(!optional); // Optional classes are disallowed by the parser.
        out << nl << stream << ".writeValue(" << v << ");";
        return;
    }

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st)
    {
        const string typeS = st->mappedScoped(".");
        if (optional)
        {
            out << nl << typeS << ".ice_writeOpt(" << stream << ", " << tag << ", " << v << ");";
        }
        else
        {
            out << nl << typeS << ".ice_write(" << stream << ", " << v << ");";
        }
        return;
    }

    EnumPtr en = dynamic_pointer_cast<Enum>(type);
    if (en)
    {
        const string typeS = en->mappedScoped(".");
        if (optional)
        {
            out << nl << typeS << ".ice_writeOpt(" << stream << ", " << tag << ", " << v << ");";
        }
        else
        {
            out << nl << typeS << ".ice_write(" << stream << ", " << v << ");";
        }
        return;
    }

    DictionaryPtr dict = dynamic_pointer_cast<Dictionary>(type);
    if (dict)
    {
        const string typeS = dict->mappedScoped(".");
        if (optional)
        {
            out << nl << typeS << ".writeOpt(" << stream << ", " << tag << ", " << v << ");";
        }
        else
        {
            out << nl << typeS << ".write(" << stream << ", " << v << ");";
        }
        return;
    }

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        const TypePtr content = seq->type();
        const BuiltinPtr b = dynamic_pointer_cast<Builtin>(content);

        if (b && b->kind() <= Builtin::KindString)
        {
            static const char* builtinTable[] = {"Byte", "Bool", "Short", "Int", "Long", "Float", "Double", "String"};
            out << nl << stream << ".write" << builtinTable[b->kind()] << "Seq";
            if (optional)
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

        const string typeS = seq->mappedScoped(".");
        if (optional)
        {
            out << nl << typeS << ".writeOpt(" << stream << ", " << tag << ", " << v << ");";
        }
        else
        {
            out << nl << typeS << ".write(" << stream << ", " << v << ");";
        }
        return;
    }

    assert(false);
}

void
CodeVisitor::unmarshal(
    IceInternal::Output& out,
    const string& stream,
    const string& v,
    const TypePtr& type,
    bool optional,
    int32_t tag)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindByte:
            {
                if (optional)
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
                if (optional)
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
                if (optional)
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
                if (optional)
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
                if (optional)
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
                if (optional)
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
                if (optional)
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
                if (optional)
                {
                    out << nl << v << " = " << stream << ".readStringOpt(" << tag << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readString();";
                }
                break;
            }
            case Builtin::KindValue:
            {
                assert(!optional); // Optional classes are disallowed by the parser.
                out << nl << stream << ".readValue(" << v << ", 'Ice.Value');";
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if (optional)
                {
                    out << nl << v << " = " << stream << ".readProxyOpt(" << tag << ");";
                }
                else
                {
                    out << nl << v << " = " << stream << ".readProxy();";
                }
                break;
            }
        }
        return;
    }

    InterfaceDeclPtr prx = dynamic_pointer_cast<InterfaceDecl>(type);
    if (prx)
    {
        const string typeS = prx->mappedScoped(".") + "Prx";
        if (optional)
        {
            out << nl << v << " = " << stream << ".readProxyOpt(" << tag << ", '" << typeS << "');";
        }
        else
        {
            out << nl << v << " = " << typeS << ".ice_read(" << stream << ");";
        }

        return;
    }

    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    if (cl)
    {
        assert(!optional); // Optional classes are disallowed by the parser.
        const string cls = cl->mappedScoped(".");
        out << nl << stream << ".readValue(" << v << ", '" << cls << "');";
        return;
    }

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st)
    {
        const string typeS = st->mappedScoped(".");
        if (optional)
        {
            out << nl << v << " = " << typeS << ".ice_readOpt(" << stream << ", " << tag << ");";
        }
        else
        {
            out << nl << v << " = " << typeS << ".ice_read(" << stream << ");";
        }
        return;
    }

    EnumPtr en = dynamic_pointer_cast<Enum>(type);
    if (en)
    {
        const string typeS = en->mappedScoped(".");
        if (optional)
        {
            out << nl << v << " = " << typeS << ".ice_readOpt(" << stream << ", " << tag << ");";
        }
        else
        {
            out << nl << v << " = " << typeS << ".ice_read(" << stream << ");";
        }
        return;
    }

    DictionaryPtr dict = dynamic_pointer_cast<Dictionary>(type);
    if (dict)
    {
        const string typeS = dict->mappedScoped(".");
        if (optional)
        {
            out << nl << v << " = " << typeS << ".readOpt(" << stream << ", " << tag << ");";
        }
        else
        {
            out << nl << v << " = " << typeS << ".read(" << stream << ");";
        }
        return;
    }

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        const TypePtr content = seq->type();
        const BuiltinPtr b = dynamic_pointer_cast<Builtin>(content);

        if (b && b->kind() <= Builtin::KindString)
        {
            static const char* builtinTable[] = {"Byte", "Bool", "Short", "Int", "Long", "Float", "Double", "String"};
            out << nl << v << " = " << stream << ".read" << builtinTable[b->kind()] << "Seq";
            if (optional)
            {
                out << "Opt(" << tag << ");";
            }
            else
            {
                out << "();";
            }
            return;
        }

        const string typeS = seq->mappedScoped(".");
        if (optional)
        {
            out << nl << v << " = " << typeS << ".readOpt(" << stream << ", " << tag << ");";
        }
        else
        {
            out << nl << v << " = " << typeS << ".read(" << stream << ");";
        }
        return;
    }

    assert(false);
}

void
CodeVisitor::unmarshalStruct(IceInternal::Output& out, const StructPtr& p, const string& v)
{
    for (const auto& dm : p->dataMembers())
    {
        const string m = dm->mappedName();
        if (dm->type()->isClassType())
        {
            out << nl << m << "_ = IceInternal.ValueHolder();";
            out << nl << v << "." << m << " = " << m << "_;";
            unmarshal(out, "is", "@(v_) " + m + "_.set(v_)", dm->type(), false, 0);
        }
        else
        {
            unmarshal(out, "is", v + "." + m, dm->type(), false, 0);
        }
    }
}

void
CodeVisitor::convertStruct(IceInternal::Output& out, const StructPtr& p, const string& v)
{
    for (const auto& dm : p->dataMembers())
    {
        const string m = dm->mappedName();
        if (needsConversion(dm->type()))
        {
            convertValueType(out, v + "." + m, v + "." + m, dm->type(), false);
        }
        else if (dm->type()->isClassType())
        {
            out << nl << v << "." << m << " = " << v << "." << m << ".value;";
        }
    }
}

string
Slice::matlabLinkFormatter(const string& rawLink, const ContainedPtr&, const SyntaxTreeBasePtr& target)
{
    string displayText; // The hyperlink text that will be visible to the user.
    string linkText;    // The fully-scoped name of a MATLAB element, which the link resolves to when clicked.

    if (auto contained = dynamic_pointer_cast<Contained>(target))
    {
        if (dynamic_pointer_cast<DataMember>(contained) || dynamic_pointer_cast<Enumerator>(contained))
        {
            // Data member & enumerator links should be of the form "<ScopedType>/<propertyName>".
            auto container = dynamic_pointer_cast<Contained>(contained->container());
            assert(container);
            displayText = container->mappedName() + "/" + contained->mappedName();
            linkText = container->mappedScoped(".") + "/" + contained->mappedName();
        }
        else if (auto opTarget = dynamic_pointer_cast<Operation>(target))
        {
            // Operation links should be of the form "<ScopedProxy>/<functionName>".
            auto interfaceDef = opTarget->interface();
            displayText = opTarget->mappedName();
            linkText = interfaceDef->mappedScoped(".") + "Prx" + "/" + displayText;
        }
        else if (auto interfaceTarget = dynamic_pointer_cast<InterfaceDecl>(target))
        {
            displayText = contained->mappedName() + "Prx";
            linkText = contained->mappedScoped(".") + "Prx";
        }
        else
        {
            displayText = contained->mappedName();
            linkText = contained->mappedScoped(".");
        }
    }
    else if (auto builtin = dynamic_pointer_cast<Builtin>(target))
    {
        displayText = linkText = typeToString(builtin);
    }
    else // we couldn't resolve the link target and make a best-effort attempt to map the link.
    {
        // All we can do is replace any doxygen separators with the MATLAB separator ('.').
        // Note that '.' can always be used in place of '/', it's just convention to use '/' in certain cases.
        static const std::regex separatorRegex{"::|#"};
        displayText = linkText = std::regex_replace(rawLink, separatorRegex, ".");
    }

    // MATLAB allows you to run arbitrary commands inside an 'href', using the 'matlab:' command prefix.
    return "<a href=\"matlab:help " + linkText + " -displayBanner\">" + displayText + "</a>";
}
