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
Slice::Csharp::getUnqualified(const ContainedPtr& p, const string& ns, const string& prefix)
{
    string name = p->mappedName();
    if (!prefix.empty())
    {
        name = prefix + removeEscapePrefix(name);
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
