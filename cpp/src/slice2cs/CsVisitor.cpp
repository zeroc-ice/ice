// Copyright (c) ZeroC, Inc.

#include "CsVisitor.h"
#include "CsUtil.h"

#include <cassert>

using namespace std;
using namespace Slice;
using namespace Slice::Csharp;
using namespace IceInternal;

namespace
{
    void writeSeeAlso(Output& out, const StringList& seeAlso)
    {
        for (const auto& line : seeAlso)
        {
            // An empty line means that the see-also was referencing a Slice element which isn't mapped in C#.
            // There's nothing we can do about this in C#, so we just skip it.
            if (!line.empty())
            {
                out << nl << "/// " << line;
            }
        }
    }
}

Slice::CsVisitor::CsVisitor(Output& out) : _out(out) {}

void
Slice::CsVisitor::emitNonBrowsableAttribute()
{
    _out << nl
         << "[global::System.ComponentModel.EditorBrowsable(global::System.ComponentModel.EditorBrowsableState.Never)]";
}

void
Slice::CsVisitor::emitAttributes(const ContainedPtr& p)
{
    assert(
        dynamic_pointer_cast<Enum>(p) || dynamic_pointer_cast<Enumerator>(p) || dynamic_pointer_cast<DataMember>(p) ||
        dynamic_pointer_cast<Const>(p));

    for (const auto& metadata : p->getMetadata())
    {
        if (metadata->directive() == "cs:attribute")
        {
            _out << nl << '[' << metadata->arguments() << ']';
        }
    }
}

void
Slice::CsVisitor::emitObsoleteAttribute(const ContainedPtr& p)
{
    if (p->isDeprecated())
    {
        if (auto reason = p->getDeprecationReason())
        {
            _out << nl << "[global::System.Obsolete(\"" << *reason << "\")]";
        }
        else
        {
            _out << nl << "[global::System.Obsolete]";
        }
    }
}

void
Slice::CsVisitor::writeDocComment(const ContainedPtr& p, const string& generatedType, const string& notes)
{
    const optional<DocComment>& comment = p->docComment();
    StringList remarks;
    if (comment)
    {
        writeDocLines(_out, "summary", comment->overview());
        remarks = comment->remarks();
    }

    if (!generatedType.empty())
    {
        // If there's user-provided remarks, and a generated-type message, we introduce a paragraph between them.
        if (!remarks.empty())
        {
            remarks.emplace_back("<para />");
        }

        remarks.push_back(
            "The Slice compiler generated this " + generatedType + " from Slice " + p->kindOf() + " <c>" + p->scoped() +
            "</c>.");
        if (!notes.empty())
        {
            remarks.push_back(notes);
        }
    }

    if (!remarks.empty())
    {
        writeDocLines(_out, "remarks", remarks);
    }

    if (comment)
    {
        writeSeeAlso(_out, comment->seeAlso());
    }
}

void
Slice::CsVisitor::writeHelperDocComment(
    const ContainedPtr& p,
    const string& comment,
    const string& generatedType,
    const string& notes)
{
    // Called only for module-level types.
    assert(dynamic_pointer_cast<Module>(p->container()));
    assert(!generatedType.empty());

    writeDocLine(_out, "summary", comment);
    _out << nl << "/// <remarks>" << "The Slice compiler generated this " << generatedType << " from Slice "
         << p->kindOf() << " <c>" << p->scoped() << "</c>.";
    if (!notes.empty())
    {
        _out << nl << "/// " << notes;
    }
    _out << "</remarks>";
}

void
Slice::CsVisitor::writeOpDocComment(const OperationPtr& op, const vector<string>& extraParams, bool isAsync)
{
    const optional<DocComment>& comment = op->docComment();
    if (!comment)
    {
        return;
    }

    writeDocLines(_out, "summary", comment->overview());

    writeParameterDocComments(*comment, isAsync ? op->inParameters() : op->parameters());

    for (const auto& extraParam : extraParams)
    {
        _out << nl << "/// " << extraParam;
    }

    if (isAsync)
    {
        _out << nl << "/// <returns>A task that represents the asynchronous operation.</returns>";
    }
    else if (op->returnType())
    {
        writeDocLines(_out, "returns", comment->returns());
    }

    for (const auto& [exceptionName, exceptionLines] : comment->exceptions())
    {
        string name = exceptionName;
        ExceptionPtr ex = op->container()->lookupException(exceptionName, false);
        if (ex)
        {
            name = ex->mappedScoped(".");
        }

        ostringstream openTag;
        openTag << "exception cref=\"" << name << "\"";

        writeDocLines(_out, openTag.str(), exceptionLines, "exception");
    }

    writeDocLines(_out, "remarks", comment->remarks());

    writeSeeAlso(_out, comment->seeAlso());
}

void
Slice::CsVisitor::writeParameterDocComments(const DocComment& comment, const ParameterList& parameters)
{
    const auto& commentParameters = comment.parameters();
    for (const auto& param : parameters)
    {
        auto q = commentParameters.find(param->name());
        if (q != commentParameters.end())
        {
            ostringstream openTag;
            openTag << "param name=\"" << removeEscapePrefix(param->mappedName()) << "\"";
            writeDocLines(_out, openTag.str(), q->second, "param");
        }
    }
}

void
Slice::CsVisitor::namespacePrefixStart(const ModulePtr& p)
{
    string ns = getNamespacePrefix(p);
    if (!ns.empty())
    {
        _out << sp;
        _out << nl << "namespace " << ns;
        _out << sb;
    }
}

void
Slice::CsVisitor::namespacePrefixEnd(const ModulePtr& p)
{
    if (!getNamespacePrefix(p).empty())
    {
        _out << eb;
    }
}
