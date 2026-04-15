// Copyright (c) ZeroC, Inc.

#include "CsVisitor.h"
#include "CsUtil.h"

#include <cassert>

using namespace std;
using namespace Slice;
using namespace Slice::Csharp;
using namespace IceInternal;

Slice::CsVisitor::CsVisitor(Output& out) : _out(out) {}

bool
Slice::CsVisitor::visitModuleStart(const ModulePtr& p)
{
    namespacePrefixStart(p);
    _out << sp;
    _out << nl << "namespace " << p->mappedName();
    _out << sb;

    return true;
}

void
Slice::CsVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
    namespacePrefixEnd(p);
}

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
