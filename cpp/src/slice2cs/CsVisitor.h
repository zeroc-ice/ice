// Copyright (c) ZeroC, Inc.

#ifndef CS_VISITOR_H
#define CS_VISITOR_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"

namespace Slice
{
    /// Base visitor class for C# code generation.
    class CsVisitor : public ParserVisitor
    {
    public:
        CsVisitor(IceInternal::Output&);

        bool visitModuleStart(const ModulePtr&) override;
        void visitModuleEnd(const ModulePtr&) override;

    protected:
        void emitNonBrowsableAttribute();

        /// Generates C# attributes from any 'cs:attribute' metadata.
        void emitAttributes(const ContainedPtr&);

        void emitObsoleteAttribute(const ContainedPtr&);

        IceInternal::Output& _out;

    private:
        void namespacePrefixStart(const ModulePtr&);
        void namespacePrefixEnd(const ModulePtr&);
    };
}

#endif
