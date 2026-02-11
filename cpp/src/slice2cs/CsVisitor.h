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

    protected:
        void emitNonBrowsableAttribute();

        /// Generates C# attributes from any 'cs:attribute' metadata.
        void emitAttributes(const ContainedPtr&);

        void emitObsoleteAttribute(const ContainedPtr&);

        /// Writes a doc-comment for the given Slice element, using this element's doc-comment, if any.
        /// @param p The Slice element.
        /// @param generatedType The kind of mapped element, used for the remarks. For example, "skeleton interface".
        /// This function does not write any remarks when this argument is empty.
        /// @param notes Optional notes included at the end of the remarks.
        void
        writeDocComment(const ContainedPtr& p, const std::string& generatedType = "", const std::string& notes = "");

        /// Writes a doc-comment for a helper class generated for a Slice element.
        /// @param p The Slice element.
        /// @param comment The summary.
        /// @param generatedType The kind of mapped element, used for the remarks. Must not be empty.
        /// @param notes Optional notes included at the end of the remarks.
        void writeHelperDocComment(
            const ContainedPtr& p,
            const std::string& comment,
            const std::string& generatedType,
            const std::string& notes = "");

        void
        writeOpDocComment(const OperationPtr& operation, const std::vector<std::string>& extraParams, bool isAsync);
        void writeParameterDocComments(const DocComment&, const ParameterList&);

        void namespacePrefixStart(const ModulePtr&);
        void namespacePrefixEnd(const ModulePtr&);

        IceInternal::Output& _out;
    };
}

#endif
