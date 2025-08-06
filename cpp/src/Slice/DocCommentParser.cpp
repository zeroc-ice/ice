// Copyright (c) ZeroC, Inc.

#include "DocCommentParser.h"
#include "Ice/StringUtil.h"

#include <algorithm>
#include <cassert>

using namespace std;
using namespace Slice;

namespace Slice
{
    class DocCommentParser final : public ParserVisitor
    {
    public:
        DocCommentParser(DocLinkFormatter linkFormatter) : _linkFormatter(linkFormatter) {}

        bool visitModuleStart(const ModulePtr& p) final;
        void visitClassDecl(const ClassDeclPtr& p) final;
        bool visitClassDefStart(const ClassDefPtr& p) final;
        void visitInterfaceDecl(const InterfaceDeclPtr& p) final;
        bool visitInterfaceDefStart(const InterfaceDefPtr& p) final;
        bool visitExceptionStart(const ExceptionPtr& p) final;
        bool visitStructStart(const StructPtr& p) final;
        void visitOperation(const OperationPtr& p) final;
        void visitDataMember(const DataMemberPtr& p) final;
        void visitSequence(const SequencePtr& p) final;
        void visitDictionary(const DictionaryPtr& p) final;
        void visitEnum(const EnumPtr& p) final;
        void visitConst(const ConstPtr& p) final;

        [[nodiscard]] bool shouldVisitIncludedDefinitions() const final { return true; }

    private:
        /// Parses the doc-comment for @p p in-place.
        /// @remark This function must be called on a doc-comment before it's usable for code-gen purposes.
        void parseDocCommentFor(const ContainedPtr& p);

        DocLinkFormatter _linkFormatter;
    };
}

void
Slice::parseAllDocComments(const UnitPtr& unit, DocLinkFormatter linkFormatter)
{
    DocCommentParser visitor{linkFormatter};
    unit->visit(&visitor);
}

bool
DocCommentParser::visitModuleStart(const ModulePtr& p)
{
    parseDocCommentFor(p);
    return true;
}

void
DocCommentParser::visitClassDecl(const ClassDeclPtr& p)
{
    parseDocCommentFor(p);
}

bool
DocCommentParser::visitClassDefStart(const ClassDefPtr& p)
{
    parseDocCommentFor(p);
    return true;
}

void
DocCommentParser::visitInterfaceDecl(const InterfaceDeclPtr& p)
{
    parseDocCommentFor(p);
}

bool
DocCommentParser::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    parseDocCommentFor(p);
    return true;
}

bool
DocCommentParser::visitExceptionStart(const ExceptionPtr& p)
{
    parseDocCommentFor(p);
    return true;
}

bool
DocCommentParser::visitStructStart(const StructPtr& p)
{
    parseDocCommentFor(p);
    return true;
}

void
DocCommentParser::visitOperation(const OperationPtr& p)
{
    parseDocCommentFor(p);
    for (const auto& param : p->parameters())
    {
        parseDocCommentFor(param);
    }
}

void
DocCommentParser::visitDataMember(const DataMemberPtr& p)
{
    parseDocCommentFor(p);
}

void
DocCommentParser::visitSequence(const SequencePtr& p)
{
    parseDocCommentFor(p);
}

void
DocCommentParser::visitDictionary(const DictionaryPtr& p)
{
    parseDocCommentFor(p);
}

void
DocCommentParser::visitEnum(const EnumPtr& p)
{
    parseDocCommentFor(p);
    for (const auto& enumerator : p->enumerators())
    {
        parseDocCommentFor(enumerator);
    }
}

void
DocCommentParser::visitConst(const ConstPtr& p)
{
    parseDocCommentFor(p);
}

namespace
{
    void trimLines(StringList& lines)
    {
        // Remove empty trailing lines.
        while (!lines.empty() && lines.back().empty())
        {
            lines.pop_back();
        }
    }

    bool parseCommentLine(string_view line, string_view tag, string& doc)
    {
        const string ws = " \t";
        const auto tagLength = tag.size();

        // If the line doesn't start with the provided tag, we immediately return false.
        // Note that any leading whitespace has already been stripped off here.
        if (line.find(tag) != 0)
        {
            return false;
        }

        // The tag must be immediately followed by whitespace, or be the entire line (for multiline tags).
        if (line.find_first_of(ws, tagLength) != tagLength && line.length() != tagLength)
        {
            return false;
        }

        // Find the first non-whitespace character after the tag. This marks the start of the `doc` string.
        const auto docSplitPos = line.find_first_not_of(ws, tagLength);
        if (docSplitPos != string::npos)
        {
            doc = line.substr(docSplitPos);
        }
        return true;
    }

    bool parseNamedCommentLine(string_view line, string_view tag, string& name, string& doc)
    {
        const string ws = " \t";

        // First we check for the tag and parse the doc-comment normally.
        if (parseCommentLine(line, tag, doc))
        {
            // Then we perform additional parsing to extract the name...

            auto nameStart = line.find_first_not_of(ws, tag.size());
            if (nameStart == string::npos)
            {
                return false; // Malformed line, ignore it.
            }

            auto nameEnd = line.find_first_of(ws, nameStart);
            if (nameEnd == string::npos)
            {
                return false; // Malformed line, ignore it.
            }
            name = line.substr(nameStart, nameEnd - nameStart);

            // Store whatever remains of the doc comment in the `doc` string.
            auto docSplitPos = line.find_first_not_of(ws, nameEnd);
            if (docSplitPos != string::npos)
            {
                doc = line.substr(docSplitPos);
            }

            return true;
        }
        return false;
    }

    /// Returns a pointer to the Slice element referenced by `linkText`, relative to the scope of `source`.
    /// If the link cannot be resolved, `nullptr` is returned instead.
    SyntaxTreeBasePtr resolveDocLink(string linkText, const ContainedPtr& source)
    {
        // First we check if the link is to a builtin type.
        if (auto kind = Builtin::kindFromString(linkText))
        {
            return source->unit()->createBuiltin(kind.value());
        }

        // Then, before checking for user-defined types, we determine which scope we'll be searching relative to.
        ContainerPtr linkSourceScope = dynamic_pointer_cast<Container>(source);
        if (!linkSourceScope || dynamic_pointer_cast<Operation>(source))
        {
            linkSourceScope = source->container();
        }

        // Perform the actual lookup.
        auto separatorPos = linkText.find('#');
        if (separatorPos == 0)
        {
            // If the link starts with '#', it is explicitly relative to the `linkSourceScope` container.
            ContainedList results = source->unit()->findContents(linkSourceScope->thisScope() + linkText.substr(1));
            return (results.empty() ? nullptr : results.front());
        }
        else if (separatorPos != string::npos)
        {
            // If the link has a '#' anywhere else, convert it to '::' so we can look it up.
            linkText.replace(separatorPos, 1, "::");
        }
        ContainedList results = linkSourceScope->lookupContained(linkText, false);
        return (results.empty() ? nullptr : results.front());
    }
}

void
DocCommentParser::parseDocCommentFor(const ContainedPtr& p)
{
    optional<DocComment>& docComment = p->_docComment;
    if (!docComment)
    {
        // If 'p' didn't have a doc-comment, there's nothing to parse. Return immediately.
        return;
    }

    // TODO: this is a temporary hack since only "csharp" happens to set 'escapeXml'.
    // If true, escapes all XML special characters in the parsed comment. Defaults to false.
    const bool escapeXml = (p->unit()->languageName() == "cs");

    // Split the comment's raw text up into lines.
    StringList lines = docComment->_rawDocCommentLines;

    // Escape any XML entities if necessary.
    if (escapeXml)
    {
        const string amp = "&amp;";
        const string lt = "&lt;";
        const string gt = "&gt;";

        for (auto& line : lines)
        {
            string::size_type pos = 0;
            while ((pos = line.find_first_of("&<>", pos)) != string::npos)
            {
                switch (line[pos])
                {
                    case '&':
                        line.replace(pos, 1, amp);
                        pos += amp.size();
                        break;
                    case '<':
                        line.replace(pos, 1, lt);
                        pos += lt.size();
                        break;
                    case '>':
                        line.replace(pos, 1, gt);
                        pos += gt.size();
                        break;
                    default:
                        assert(false);
                        break;
                }
            }
        }
    }

    // Fix any link tags using the provided link formatter.
    const string link = "{@link ";
    for (auto& line : lines)
    {
        auto pos = line.find(link);
        while (pos != string::npos)
        {
            auto endpos = line.find('}', pos);
            if (endpos != string::npos)
            {
                // Extract the linked-to identifier.
                auto identStart = line.find_first_not_of(" \t", pos + link.size());
                auto identEnd = line.find_last_not_of(" \t", endpos);
                string linkText = line.substr(identStart, identEnd - identStart);

                // Then erase the entire '{@link foo}' tag from the comment.
                line.erase(pos, endpos - pos + 1);

                // Attempt to resolve the link, and issue a warning if the link is invalid.
                SyntaxTreeBasePtr linkTarget = resolveDocLink(linkText, p);
                if (!linkTarget)
                {
                    string msg = "no Slice element with identifier '" + linkText + "' could be found in this context";
                    p->unit()->warning(p->file(), p->line(), InvalidComment, msg);
                }
                if (dynamic_pointer_cast<Parameter>(linkTarget))
                {
                    // We don't support linking to parameters with '@link' tags.
                    // Parameter links must be done with '@p' tags, and can only appear on the enclosing operation.
                    string msg = "cannot link parameter '" + linkText + "'; parameters can only be referenced with @p";
                    p->unit()->warning(p->file(), p->line(), InvalidComment, msg);
                    linkTarget = nullptr;
                }

                // Finally, insert a correctly formatted link where the '{@link foo}' used to be.
                string formattedLink = (*_linkFormatter)(linkText, p, linkTarget);
                line.insert(pos, formattedLink);
                pos += formattedLink.length();
            }
            pos = line.find(link, pos);
        }
    }

    // Some tags are only valid if they're applied to an operation.
    // And we need a reference to the operation to make sure any names used in the tag match the names in the operation.
    OperationPtr operationTarget = dynamic_pointer_cast<Operation>(p);

    const string ws = " \t";
    const string paramTag = "@param";
    const string throwsTag = "@throws";
    const string exceptionTag = "@exception";
    const string remarkTag = "@remark";
    const string remarksTag = "@remarks";
    const string seeTag = "@see";
    const string returnTag = "@return";
    const string deprecatedTag = "@deprecated";

    StringList* currentSection = &docComment->_overview;
    string lineText;
    string name;

    // Parse the comment's text.
    for (const auto& line : lines)
    {
        lineText.clear();

        if (parseNamedCommentLine(line, paramTag, name, lineText))
        {
            if (!operationTarget)
            {
                // If '@param' was put on anything other than an operation, ignore it and issue a warning.
                const string msg = "the '" + paramTag + "' tag is only valid on operations";
                p->unit()->warning(p->file(), p->line(), InvalidComment, msg);
                currentSection = nullptr;
            }
            else
            {
                // Check that the '@param <name>' corresponds to an actual parameter in the operation.
                const ParameterList params = operationTarget->parameters();
                const auto paramNameCheck = [&name](const ParameterPtr& param) { return param->name() == name; };
                if (std::none_of(params.begin(), params.end(), paramNameCheck))
                {
                    const string msg = "'" + paramTag + " " + name +
                                       "' does not correspond to any parameter in operation '" + p->name() + "'";
                    p->unit()->warning(p->file(), p->line(), InvalidComment, msg);
                }

                // Check if this is a duplicate tag. If it is, ignore it and issue a warning.
                if (docComment->_parameters.count(name) != 0)
                {
                    const string msg = "ignoring duplicate doc-comment tag: '" + paramTag + " " + name + "'";
                    p->unit()->warning(p->file(), p->line(), InvalidComment, msg);
                    currentSection = nullptr;
                }
                else
                {
                    docComment->_parameters[name] = {};
                    currentSection = &docComment->_parameters[name];
                }
            }
        }
        else if (
            parseNamedCommentLine(line, throwsTag, name, lineText) ||
            parseNamedCommentLine(line, exceptionTag, name, lineText))
        {
            // '@throws' and '@exception' are equivalent. But we want to use the correct one in our warning messages.
            const string actualTag = (line.find(throwsTag) == 0) ? throwsTag : exceptionTag;
            if (!operationTarget)
            {
                // If '@throws'/'@exception' was put on anything other than an operation, ignore it and issue a warning.
                const string msg = "the '" + actualTag + "' tag is only valid on operations";
                p->unit()->warning(p->file(), p->line(), InvalidComment, msg);
                currentSection = nullptr;
            }
            else
            {
                // Check if the exception exists...
                const ExceptionPtr exceptionTarget = operationTarget->lookupException(name, false);
                if (!exceptionTarget)
                {
                    const string msg = "'" + actualTag + " " + name +
                                       "': no exception with this name could be found from the current scope";
                    p->unit()->warning(p->file(), p->line(), InvalidComment, msg);
                }
                else
                {
                    // ... and matches one of the exceptions in the operation's specification.
                    const ExceptionList exceptionSpec = operationTarget->throws();
                    const auto exceptionCheck = [&exceptionTarget](const ExceptionPtr& ex)
                    { return ex->scoped() == exceptionTarget->scoped(); };
                    if (std::none_of(exceptionSpec.begin(), exceptionSpec.end(), exceptionCheck))
                    {
                        const string msg = "'" + actualTag + " " + name + "': this exception is not listed in the " +
                                           "exception specification of '" + operationTarget->name() + "'";
                        p->unit()->warning(p->file(), p->line(), InvalidComment, msg);
                    }

                    // Check if this is a duplicate tag. If it is, ignore it and issue a warning.
                    if (docComment->_exceptions.count(name) != 0)
                    {
                        const string msg = "ignoring duplicate doc-comment tag: '" + actualTag + " " + name + "'";
                        p->unit()->warning(p->file(), p->line(), InvalidComment, msg);
                        currentSection = nullptr;
                    }
                    else
                    {
                        docComment->_exceptions[name] = {};
                        currentSection = &docComment->_exceptions[name];
                    }
                }
            }
        }
        else if (parseCommentLine(line, remarkTag, lineText) || parseCommentLine(line, remarksTag, lineText))
        {
            currentSection = &docComment->_remarks;
        }
        else if (parseCommentLine(line, seeTag, lineText))
        {
            currentSection = &docComment->_seeAlso;

            // Remove any leading and trailing whitespace from the line.
            // There's no concern of losing formatting for `@see` due to its simplicity.
            lineText = IceInternal::trim(lineText);
            if (lineText.empty())
            {
                const string msg = "missing link target after '" + seeTag + "' tag";
                p->unit()->warning(p->file(), p->line(), InvalidComment, msg);
            }
            else if (lineText.back() == '.')
            {
                // '@see' tags aren't allowed to end with periods.
                // They do not take sentences, and the trailing period will trip up some language's doc-comments.
                const string msg = "ignoring trailing '.' character in '" + seeTag + "' tag";
                p->unit()->warning(p->file(), p->line(), InvalidComment, msg);
                lineText.pop_back();
            }
        }
        else if (parseCommentLine(line, returnTag, lineText))
        {
            if (!operationTarget)
            {
                // If '@return' was put on anything other than an operation, ignore it and issue a warning.
                const string msg = "the '" + returnTag + "' tag is only valid on operations";
                p->unit()->warning(p->file(), p->line(), InvalidComment, msg);
                currentSection = nullptr;
            }
            else
            {
                if (!operationTarget->returnType())
                {
                    // If '@return' was applied to a void operation (one without a return-type), issue a warning.
                    const string msg = "'" + returnTag + "' is only valid on operations with non-void return types";
                    p->unit()->warning(p->file(), p->line(), InvalidComment, msg);
                }

                // Check if this is a duplicate tag. If it is, ignore it and issue a warning.
                if (!docComment->_returns.empty())
                {
                    const string msg = "ignoring duplicate doc-comment tag: '" + returnTag + "'";
                    p->unit()->warning(p->file(), p->line(), InvalidComment, msg);
                    currentSection = nullptr;
                }
                else
                {
                    currentSection = &docComment->_returns;
                }
            }
        }
        else if (parseCommentLine(line, deprecatedTag, lineText))
        {
            // Check if this is a duplicate tag (ie. multiple '@deprecated'). If it is, ignore it and issue a warning.
            if (docComment->_isDeprecated)
            {
                const string msg = "ignoring duplicate doc-comment tag: '" + deprecatedTag + "'";
                p->unit()->warning(p->file(), p->line(), InvalidComment, msg);
                currentSection = nullptr;
            }
            else
            {
                docComment->_isDeprecated = true;
                currentSection = &docComment->_deprecated;
            }
        }
        else // This line didn't introduce a new tag. Either we're in the overview or a tag whose content is multi-line.
        {
            if (!line.empty())
            {
                // We've encountered an unknown doc tag.
                if (line[0] == '@')
                {
                    auto unknownTag = line.substr(0, line.find_first_of(" \t"));
                    const string msg = "ignoring unknown doc tag '" + unknownTag + "' in comment";
                    p->unit()->warning(p->file(), p->line(), InvalidComment, msg);
                    currentSection = nullptr;
                }

                // '@see' tags are not allowed to span multiple lines.
                if (currentSection == &docComment->_seeAlso)
                {
                    string msg = "'@see' tags cannot span multiple lines and must be of the form: '@see identifier'";
                    p->unit()->warning(p->file(), p->line(), InvalidComment, msg);
                    currentSection = nullptr;
                }
            }

            // Here we allow empty lines, since they could be used for formatting to separate lines.
            if (currentSection)
            {
                currentSection->push_back(line);
            }
            continue;
        }

        // Reaching here means that this line introduced a new tag. We reject empty lines to handle comments which
        // are formatted like: `@param myVeryCoolParam\nvery long explanation that\nspans multiple lines`.
        // We don't want an empty line at the top just because the user's content didn't start until the next line.
        if (currentSection && !lineText.empty())
        {
            currentSection->push_back(lineText);
        }
    }

    trimLines(docComment->_overview);
    trimLines(docComment->_remarks);
    trimLines(docComment->_deprecated);
    trimLines(docComment->_returns);
}
