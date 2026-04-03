// Copyright (c) ZeroC, Inc.

#include "CsUtil.h"
#include "../Slice/MetadataValidation.h"
#include "../Slice/Util.h"
#include "Ice/StringUtil.h"

#include <algorithm>
#include <cassert>

using namespace std;
using namespace Slice;
using namespace IceInternal;

string
Slice::Csharp::getNamespacePrefix(const ContainedPtr& p)
{
    if (auto topLevel = p->getTopLevelModule())
    {
        return topLevel->getMetadataArgs("cs:namespace").value_or("");
    }
    return "";
}

string
Slice::Csharp::getNamespace(const ContainedPtr& p)
{
    assert(!dynamic_pointer_cast<Module>(p));

    string ns = p->mappedScope(".");
    ns.pop_back(); // Remove the trailing '.' on the ns.
    string prefix = getNamespacePrefix(p);
    return (prefix.empty() ? ns : prefix + "." + ns);
}

string
Slice::Csharp::getUnqualified(const ContainedPtr& p, const string& ns, const string& prefix, const string& suffix)
{
    string name = p->mappedName();
    if (!prefix.empty() || !suffix.empty())
    {
        name = prefix + removeEscapePrefix(name) + suffix;
    }

    // If contained is an operation, a field, or an enumerator, we use the enclosing type.
    if (dynamic_pointer_cast<Operation>(p) || dynamic_pointer_cast<DataMember>(p) ||
        dynamic_pointer_cast<Enumerator>(p))
    {
        return getUnqualified(dynamic_pointer_cast<Contained>(p->container()), ns) + "." + name;
    }
    else
    {
        string pNamespace = getNamespace(p);
        if (pNamespace == ns || pNamespace.empty())
        {
            return name;
        }
        else
        {
            return "global::" + pNamespace + "." + name;
        }
    }
}

string
Slice::Csharp::removeEscapePrefix(const string& identifier)
{
    return identifier.find('@') == 0 ? identifier.substr(1) : identifier;
}

void
Slice::Csharp::writeConstantValue(
    Output& out,
    const TypePtr& type,
    const SyntaxTreeBasePtr& valueType,
    const string& value,
    const string& ns,
    const string& fieldName)
{
    ConstPtr constant = dynamic_pointer_cast<Const>(valueType);
    if (constant)
    {
        out << getUnqualified(constant, ns) << "." << fieldName;
    }
    else
    {
        BuiltinPtr bp = dynamic_pointer_cast<Builtin>(type);
        if (bp && bp->kind() == Builtin::KindString)
        {
            out << "\"" << toStringLiteral(value, "\a\b\f\n\r\t\v\0", "", UCN, 0) << "\"";
        }
        else if (bp && bp->kind() == Builtin::KindLong)
        {
            out << value << "L";
        }
        else if (bp && bp->kind() == Builtin::KindFloat)
        {
            out << value << "F";
        }
        else if (bp && bp->kind() == Builtin::KindDouble)
        {
            out << value << "D";
        }
        else if (dynamic_pointer_cast<Enum>(type))
        {
            EnumeratorPtr lte = dynamic_pointer_cast<Enumerator>(valueType);
            assert(lte);
            out << getUnqualified(lte, ns);
        }
        else
        {
            out << value;
        }
    }
}

void
Slice::Csharp::writeDocLine(
    Output& out,
    const string& openTag,
    const string& comment,
    const std::optional<string>& closeTag)
{
    if (comment.empty())
    {
        return;
    }

    out << nl << "/// <" << openTag << ">" << comment << "</" << closeTag.value_or(openTag) << ">";
}

void
Slice::Csharp::writeDocLines(
    Output& out,
    const string& openTag,
    const StringList& lines,
    const optional<string>& closeTag)
{
    // If there is a single line, write the doc-comment as a single line. Otherwise, write the doc-comment on
    // multiple lines.

    if (lines.size() == 1)
    {
        writeDocLine(out, openTag, lines.front(), closeTag);
    }
    else if (lines.size() > 1)
    {
        bool firstLine = true;

        for (const auto& line : lines)
        {
            if (firstLine)
            {
                firstLine = false;
                out << nl << "/// <" << openTag << ">";
            }
            else
            {
                out << nl << "///";
                if (!line.empty())
                {
                    out << ' ';
                }
            }
            out << line;
        }
        out << "</" << closeTag.value_or(openTag) << ">";
    }
}

void
Slice::Csharp::writeParameterDocComments(Output& out, const DocComment& comment, const ParameterList& parameters)
{
    const auto& commentParameters = comment.parameters();
    for (const auto& param : parameters)
    {
        auto q = commentParameters.find(param->name());
        if (q != commentParameters.end())
        {
            ostringstream openTag;
            openTag << "param name=\"" << removeEscapePrefix(param->mappedName()) << "\"";
            writeDocLines(out, openTag.str(), q->second, "param");
        }
    }
}

void
Slice::Csharp::writeSeeAlso(Output& out, const StringList& seeAlso)
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

Slice::Csharp::CsharpDocCommentFormatter::CsharpDocCommentFormatter(
    function<pair<bool, string>(const string&, const ContainedPtr&, const SyntaxTreeBasePtr&)> linkFormatter)
    : _linkFormatter(std::move(linkFormatter))
{
}

void
Slice::Csharp::CsharpDocCommentFormatter::preprocess(StringList& rawComment)
{
    for (auto& line : rawComment)
    {
        // Escape any XML special characters in the comment.
        string::size_type pos = 0;
        while ((pos = line.find_first_of("&<>", pos)) != string::npos)
        {
            switch (line[pos])
            {
                case '&':
                    line.replace(pos, 1, "&amp;");
                    break;
                case '<':
                    line.replace(pos, 1, "&lt;");
                    break;
                case '>':
                    line.replace(pos, 1, "&gt;");
                    break;
            }
            // Skip over the leading '&' character to avoid 'find'ing it again.
            pos += 1;
        }
    }
}

string
Slice::Csharp::CsharpDocCommentFormatter::formatCode(const string& rawText)
{
    return "<c>" + rawText + "</c>";
}

string
Slice::Csharp::CsharpDocCommentFormatter::formatParamRef(const string& param)
{
    return "<paramref name=\"" + param + "\" />";
}

string
Slice::Csharp::CsharpDocCommentFormatter::formatLink(
    const string& rawLink,
    const ContainedPtr& source,
    const SyntaxTreeBasePtr& target)
{
    auto [mapToLink, qualifiedName] = _linkFormatter(rawLink, source, target);
    if (mapToLink)
    {
        return "<see " + qualifiedName + " />";
    }
    else
    {
        return "<c>" + qualifiedName + "</c>";
    }
}

string
Slice::Csharp::CsharpDocCommentFormatter::formatSeeAlso(
    const string& rawLink,
    const ContainedPtr& source,
    const SyntaxTreeBasePtr& target)
{
    auto [mapToLink, qualifiedName] = _linkFormatter(rawLink, source, target);
    if (mapToLink)
    {
        return "<seealso " + qualifiedName + " />";
    }
    else
    {
        return "";
    }
}
