// Copyright (c) ZeroC, Inc.

#ifndef DOC_COMMENT_PARSER_H
#define DOC_COMMENT_PARSER_H

#include "Parser.h"

namespace Slice
{
    class DocCommentFormatter
    {
    public:
        /// Gives the formatter a chance to preprocess a comment before it's parsed.
        /// @param rawComment The raw lines of the doc-comment (as it's written in the Slice file) minus any formatting
        /// characters like leading '///' and '*' characters or leading whitespace.
        //
        // By default we perform no preprocessing.
        virtual void preprocess(StringList& rawComment);

        /// This function is called by the doc-comment parser to map code-spans (`<rawText>`) into each language's
        /// syntax.
        /// @param rawText The raw text contained within the backticks (the backticks are NOT included).
        /// @return A properly formatted code span in the target language. The doc-comment parser will replace the
        /// entire "`...`" string with the returned value.
        //
        // By default we just re-emit the text with the original number of backticks around it.
        [[nodiscard]] virtual std::string formatCode(const std::string& rawText);

        /// This function is called by the doc-comment parser to map '@p' tags into each language's syntax.
        /// @param param The mapped name of the parameter that is being referenced.
        /// @return A properly formatted parameters reference in the target language. The doc-comment parser will
        /// replace the entire "@p <rawParamName>" string with the returned value.
        //
        // By default we just emit the parameter's name in code formatting.
        [[nodiscard]] virtual std::string formatParamRef(const std::string& param);

        /// This function is called by the doc-comment parser to map doc-links ('{@link <rawLink>}') into each
        /// language's syntax.
        /// @param rawLink The link's raw text, taken verbatim from the doc-comment.
        /// @param source A pointer to the Slice element that the doc-comment (and link) are written on.
        /// @param target A pointer to the Slice element that is being linked to, or `nullptr` if it doesn't exist.
        /// @return A properly formatted doc-link in the target language. The doc-comment parser will replace the
        /// entire "{@link <rawLink>}" string with the returned value.
        [[nodiscard]] virtual std::string
        formatLink(const std::string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target) = 0;
    };

    /// Parses all doc-comments within the provided unit (in-place).
    void parseAllDocComments(const UnitPtr& unit, DocCommentFormatter& formatter);
}
#endif
