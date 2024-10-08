//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
//

#include "../Ice/OutputUtil.h"
#include "Ice/StringUtil.h"

#include "../Slice/Util.h"

#include "SwiftUtil.h"

#include <cassert>
#include <functional>

using namespace std;
using namespace Slice;
using namespace IceInternal;

namespace
{
    static string lookupKwd(const string& name)
    {
        //
        // Keyword list. *Must* be kept in alphabetical order.
        //
        static const string keywordList[] = {
            "Any",
            "Self",
            "Type",
            "as",
            "associatedtype",
            "associativity",
            "async",
            "await",
            "borrowing",
            "break",
            "case",
            "catch",
            "class",
            "consuming",
            "continue",
            "convenience",
            "default",
            "defer",
            "deinit",
            "didSet",
            "do",
            "dynamic",
            "else",
            "enum",
            "extension",
            "fallthrough",
            "false",
            "fileprivate",
            "final",
            "for",
            "func",
            "get",
            "guard",
            "if",
            "import",
            "in",
            "indirect",
            "infix",
            "init",
            "inout",
            "internal",
            "is",
            "lazy",
            "left",
            "let",
            "mutating",
            "nil",
            "none",
            "nonisolated",
            "nonmutating",
            "open",
            "operator",
            "optional",
            "override",
            "package",
            "postfix",
            "precedence",
            "precedencegroup",
            "prefix",
            "private",
            "protocol",
            "public",
            "repeat",
            "required",
            "rethrows",
            "return",
            "right",
            "self",
            "set",
            "some",
            "static",
            "struct",
            "subscript",
            "super",
            "switch",
            "throw",
            "throws",
            "true",
            "try",
            "typealias",
            "unowned",
            "var",
            "weak",
            "where",
            "while",
            "willSet"};
        bool found = binary_search(&keywordList[0], &keywordList[sizeof(keywordList) / sizeof(*keywordList)], name);
        if (found)
        {
            return "`" + name + "`";
        }

        return name;
    }

    string replace(string s, string patt, string val)
    {
        string r = s;
        string::size_type pos = r.find(patt);
        while (pos != string::npos)
        {
            r.replace(pos, patt.size(), val);
            pos += val.size();
            pos = r.find(patt, pos);
        }
        return r;
    }

    string opFormatTypeToString(const OperationPtr& op)
    {
        optional<FormatType> opFormat = op->format();
        if (opFormat)
        {
            switch (*opFormat)
            {
                case CompactFormat:
                    return ".CompactFormat";
                case SlicedFormat:
                    return ".SlicedFormat";
                default:
                    assert(false);
                    return "???";
            }
        }
        else
        {
            return "nil";
        }
    }
}

// Check the given identifier against Swift's list of reserved words. If it matches
// a reserved word, then an escaped version is returned with a leading underscore.
//
// The provided identifier must be a _Swift_ identifier (Ice.Foo) and not a Slice identifier (Ice::Foo).
string
Slice::fixIdent(const string& ident)
{
    assert(ident.find(':' == string::npos));

    // Swift namespaces are flat, so we don't need to check for multiple periods, there can be at most 1.
    string::size_type separator_pos = ident.find('.');
    if (separator_pos != string::npos)
    {
        // If this identifier is scoped, we break it up and escape each piece separately.
        auto scope = ident.substr(0, separator_pos);
        auto name = ident.substr(separator_pos + 1);

        ostringstream result;
        result << lookupKwd(scope) << "." << lookupKwd(name);
        return result.str();
    }
    else
    {
        // If this identifier isn't scoped, we directly escape it.
        return lookupKwd(ident);
    }
}

string
Slice::getSwiftModule(const ModulePtr& module, string& swiftPrefix)
{
    string swiftModule;
    if (auto argument = module->getMetadataArgs("swift:module"))
    {
        size_t pos = argument->find(':');
        if (pos != string::npos)
        {
            swiftModule = argument->substr(0, pos);
            swiftPrefix = argument->substr(pos + 1);
        }
        else
        {
            swiftModule = *argument;
            swiftPrefix = "";
        }
    }
    else
    {
        swiftModule = module->name();
        swiftPrefix = "";
    }
    return fixIdent(swiftModule);
}

string
Slice::getSwiftModule(const ModulePtr& module)
{
    string prefix;
    return getSwiftModule(module, prefix);
}

ModulePtr
Slice::getTopLevelModule(const ContainedPtr& cont)
{
    //
    // Traverse to the top-level module.
    //
    ModulePtr m;
    ContainedPtr p = cont;
    while (true)
    {
        if (dynamic_pointer_cast<Module>(p))
        {
            m = dynamic_pointer_cast<Module>(p);
        }

        ContainerPtr c = p->container();
        p = dynamic_pointer_cast<Contained>(c); // This cast fails for Unit.
        if (!p)
        {
            break;
        }
    }
    return m;
}

ModulePtr
Slice::getTopLevelModule(const TypePtr& type)
{
    assert(dynamic_pointer_cast<InterfaceDecl>(type) || dynamic_pointer_cast<Contained>(type));

    InterfaceDeclPtr proxy = dynamic_pointer_cast<InterfaceDecl>(type);
    return getTopLevelModule(proxy ? dynamic_pointer_cast<Contained>(proxy) : dynamic_pointer_cast<Contained>(type));
}

void
SwiftGenerator::trimLines(StringList& l)
{
    //
    // Remove empty trailing lines.
    //
    while (!l.empty() && l.back().empty())
    {
        l.pop_back();
    }
}

StringList
SwiftGenerator::splitComment(const string& c)
{
    string comment = c;

    //
    // TODO: this comment is suspicious. Does this apply to Swift docs as well?
    // Strip HTML markup and javadoc links -- MATLAB doesn't display them.
    //
    string::size_type pos = 0;
    do
    {
        pos = comment.find('<', pos);
        if (pos != string::npos)
        {
            string::size_type endpos = comment.find('>', pos);
            if (endpos == string::npos)
            {
                break;
            }
            comment.erase(pos, endpos - pos + 1);
        }
    } while (pos != string::npos);

    const string link = "{@link";
    pos = 0;
    do
    {
        pos = comment.find(link, pos);
        if (pos != string::npos)
        {
            comment.erase(pos, link.size() + 1); // Erase trailing white space too.
            string::size_type endpos = comment.find('}', pos);
            if (endpos != string::npos)
            {
                string ident = comment.substr(pos, endpos - pos);
                comment.erase(pos, endpos - pos + 1);

                //
                // Check for links of the form {@link Type#member}.
                //
                string::size_type hash = ident.find('#');
                string rest;
                if (hash != string::npos)
                {
                    rest = ident.substr(hash + 1);
                    ident = ident.substr(0, hash);
                    if (!ident.empty())
                    {
                        ident = fixIdent(ident);
                        if (!rest.empty())
                        {
                            ident += "." + fixIdent(rest);
                        }
                    }
                    else if (!rest.empty())
                    {
                        ident = fixIdent(rest);
                    }
                }
                else
                {
                    ident = fixIdent(ident);
                }

                comment.insert(pos, ident);
            }
        }
    } while (pos != string::npos);

    StringList result;

    pos = 0;
    string::size_type nextPos;
    while ((nextPos = comment.find_first_of('\n', pos)) != string::npos)
    {
        result.push_back(IceInternal::trim(string(comment, pos, nextPos - pos)));
        pos = nextPos + 1;
    }
    string lastLine = IceInternal::trim(string(comment, pos));
    if (!lastLine.empty())
    {
        result.push_back(lastLine);
    }

    trimLines(result);

    return result;
}

bool
SwiftGenerator::parseCommentLine(const string& l, const string& tag, bool namedTag, string& name, string& doc)
{
    doc.clear();

    if (l.find(tag) == 0)
    {
        const string ws = " \t";

        if (namedTag)
        {
            string::size_type n = l.find_first_not_of(ws, tag.size());
            if (n == string::npos)
            {
                return false; // Malformed line, ignore it.
            }
            string::size_type end = l.find_first_of(ws, n);
            if (end == string::npos)
            {
                return false; // Malformed line, ignore it.
            }
            name = l.substr(n, end - n);
            n = l.find_first_not_of(ws, end);
            if (n != string::npos)
            {
                doc = l.substr(n);
            }
        }
        else
        {
            name.clear();

            string::size_type n = l.find_first_not_of(ws, tag.size());
            if (n == string::npos)
            {
                return false; // Malformed line, ignore it.
            }
            doc = l.substr(n);
        }

        return true;
    }

    return false;
}

DocElements
SwiftGenerator::parseComment(const ContainedPtr& p)
{
    DocElements doc;

    doc.deprecated = p->isDeprecated();

    // First check metadata for a deprecated tag.
    if (auto reason = p->getDeprecationReason())
    {
        doc.deprecateReason.push_back(IceInternal::trim(*reason));
    }

    //
    // Split up the comment into lines.
    //
    StringList lines = splitComment(p->comment());

    StringList::const_iterator i;
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        const string l = *i;
        if (l[0] == '@')
        {
            break;
        }
        doc.overview.push_back(l);
    }

    enum State
    {
        StateMisc,
        StateParam,
        StateThrows,
        StateReturn,
        StateDeprecated
    };
    State state = StateMisc;
    string name;
    const string ws = " \t";
    const string paramTag = "@param";
    const string throwsTag = "@throws";
    const string exceptionTag = "@exception";
    const string returnTag = "@return";
    const string deprecatedTag = "@deprecated";
    const string seeTag = "@see";
    for (; i != lines.end(); ++i)
    {
        const string l = IceInternal::trim(*i);
        string line;
        if (parseCommentLine(l, paramTag, true, name, line))
        {
            if (!line.empty())
            {
                state = StateParam;
                StringList sl;
                sl.push_back(line); // The first line of the description.
                doc.params[name] = sl;
            }
        }
        else if (parseCommentLine(l, throwsTag, true, name, line))
        {
            if (!line.empty())
            {
                state = StateThrows;
                StringList sl;
                sl.push_back(line); // The first line of the description.
                doc.exceptions[name] = sl;
            }
        }
        else if (parseCommentLine(l, exceptionTag, true, name, line))
        {
            if (!line.empty())
            {
                state = StateThrows;
                StringList sl;
                sl.push_back(line); // The first line of the description.
                doc.exceptions[name] = sl;
            }
        }
        else if (parseCommentLine(l, seeTag, false, name, line))
        {
            if (!line.empty())
            {
                doc.seeAlso.push_back(line);
            }
        }
        else if (parseCommentLine(l, returnTag, false, name, line))
        {
            if (!line.empty())
            {
                state = StateReturn;
                doc.returns.push_back(line); // The first line of the description.
            }
        }
        else if (parseCommentLine(l, deprecatedTag, false, name, line))
        {
            doc.deprecated = true;
            if (!line.empty())
            {
                state = StateDeprecated;
                doc.deprecateReason.push_back(line); // The first line of the description.
            }
        }
        else if (!l.empty())
        {
            if (l[0] == '@')
            {
                //
                // Treat all other tags as miscellaneous comments.
                //
                state = StateMisc;
            }

            switch (state)
            {
                case StateMisc:
                {
                    doc.misc.push_back(l);
                    break;
                }
                case StateParam:
                {
                    assert(!name.empty());
                    StringList sl;
                    if (doc.params.find(name) != doc.params.end())
                    {
                        sl = doc.params[name];
                    }
                    sl.push_back(l);
                    doc.params[name] = sl;
                    break;
                }
                case StateThrows:
                {
                    assert(!name.empty());
                    StringList sl;
                    if (doc.exceptions.find(name) != doc.exceptions.end())
                    {
                        sl = doc.exceptions[name];
                    }
                    sl.push_back(l);
                    doc.exceptions[name] = sl;
                    break;
                }
                case StateReturn:
                {
                    doc.returns.push_back(l);
                    break;
                }
                case StateDeprecated:
                {
                    doc.deprecateReason.push_back(l);
                    break;
                }
            }
        }
    }

    trimLines(doc.overview);
    trimLines(doc.deprecateReason);
    trimLines(doc.misc);
    trimLines(doc.returns);

    return doc;
}

void
SwiftGenerator::writeDocLines(IceInternal::Output& out, const StringList& lines, bool commentFirst, const string& space)
{
    StringList l = lines;
    if (!commentFirst)
    {
        out << l.front();
        l.pop_front();
    }

    for (StringList::const_iterator i = l.begin(); i != l.end(); ++i)
    {
        out << nl << "///";
        if (!i->empty())
        {
            out << space << *i;
        }
    }
}

void
SwiftGenerator::writeDocSentence(IceInternal::Output& out, const StringList& lines)
{
    //
    // Write the first sentence.
    //
    for (StringList::const_iterator i = lines.begin(); i != lines.end(); ++i)
    {
        const string ws = " \t";

        if (i->empty())
        {
            break;
        }
        if (i != lines.begin() && i->find_first_not_of(ws) == 0)
        {
            out << " ";
        }
        string::size_type pos = i->find('.');
        if (pos == string::npos)
        {
            out << *i;
        }
        else if (pos == i->size() - 1)
        {
            out << *i;
            break;
        }
        else
        {
            //
            // Assume a period followed by whitespace indicates the end of the sentence.
            //
            while (pos != string::npos)
            {
                if (ws.find((*i)[pos + 1]) != string::npos)
                {
                    break;
                }
                pos = i->find('.', pos + 1);
            }
            if (pos != string::npos)
            {
                out << i->substr(0, pos + 1);
                break;
            }
            else
            {
                out << *i;
            }
        }
    }
}

void
SwiftGenerator::writeDocSummary(IceInternal::Output& out, const ContainedPtr& p)
{
    DocElements doc = parseComment(p);

    string n = fixIdent(p->name());

    //
    // No leading newline.
    //
    if (!doc.overview.empty())
    {
        writeDocLines(out, doc.overview);
    }

    if (!doc.misc.empty())
    {
        out << "///" << nl;
        writeDocLines(out, doc.misc);
    }

    if (doc.deprecated)
    {
        out << nl << "///";
        out << nl << "/// ## Deprecated";
        if (!doc.deprecateReason.empty())
        {
            writeDocLines(out, doc.deprecateReason);
        }
    }
}

void
SwiftGenerator::writeOpDocSummary(IceInternal::Output& out, const OperationPtr& p, bool dispatch)
{
    DocElements doc = parseComment(p);
    if (!doc.overview.empty())
    {
        writeDocLines(out, doc.overview);
    }

    if (doc.deprecated)
    {
        out << nl << "///";
        out << nl << "///  ## Deprecated";
        if (!doc.deprecateReason.empty())
        {
            writeDocLines(out, doc.deprecateReason);
        }
    }

    const ParamInfoList allInParams = getAllInParams(p);
    for (ParamInfoList::const_iterator q = allInParams.begin(); q != allInParams.end(); ++q)
    {
        out << nl << "///";
        out << nl << "/// - parameter " << (!dispatch && allInParams.size() == 1 ? "_" : q->name) << ": `" << q->typeStr
            << "`";
        map<string, StringList>::const_iterator r = doc.params.find(q->name);
        if (r != doc.params.end() && !r->second.empty())
        {
            out << " ";
            writeDocLines(out, r->second, false);
        }
    }

    out << nl << "///";
    if (dispatch)
    {
        out << nl << "/// - parameter current: `Ice.Current` - The Current object for the dispatch.";
    }
    else
    {
        out << nl << "/// - parameter context: `Ice.Context` - Optional request context.";
    }

    const ParamInfoList allOutParams = getAllOutParams(p);
    if (allOutParams.size() == 1)
    {
        ParamInfo ret = allOutParams.front();
        out << nl << "///";
        out << nl << "/// - returns: `" << ret.typeStr << "`";
        if (p->returnType())
        {
            if (!doc.returns.empty())
            {
                out << " - ";
                writeDocLines(out, doc.returns, false);
            }
        }
        else
        {
            map<string, StringList>::const_iterator r = doc.params.find(ret.name);
            if (r != doc.params.end() && !r->second.empty())
            {
                out << " - ";
                writeDocLines(out, r->second, false);
            }
        }
    }
    else if (allOutParams.size() > 1)
    {
        out << nl << "///";
        out << nl << "/// - returns: `" << operationReturnType(p) << "`:";
        if (p->returnType())
        {
            ParamInfo ret = allOutParams.back();
            out << nl << "///";
            out << nl << "///   - " << ret.name << ": `" << ret.typeStr << "`";
            if (!doc.returns.empty())
            {
                out << " - ";
                writeDocLines(out, doc.returns, false);
            }
        }

        for (ParamInfoList::const_iterator q = allOutParams.begin(); q != allOutParams.end(); ++q)
        {
            if (q->param != 0)
            {
                out << nl << "///";
                out << nl << "///   - " << q->name << ": `" << q->typeStr << "`";
                map<string, StringList>::const_iterator r = doc.params.find(q->name);
                if (r != doc.params.end() && !r->second.empty())
                {
                    out << " - ";
                    writeDocLines(out, r->second, false);
                }
            }
        }
    }

    if (!doc.exceptions.empty())
    {
        out << nl << "///";
        out << nl << "/// - throws:";
        for (map<string, StringList>::const_iterator q = doc.exceptions.begin(); q != doc.exceptions.end(); ++q)
        {
            out << nl << "///";
            out << nl << "///   - " << q->first;
            if (!q->second.empty())
            {
                out << " - ";
                writeDocLines(out, q->second, false, "     ");
            }
        }
    }

    if (!doc.misc.empty())
    {
        out << nl;
        writeDocLines(out, doc.misc, false);
    }
}

void
SwiftGenerator::writeProxyDocSummary(IceInternal::Output& out, const InterfaceDefPtr& p, const string& swiftModule)
{
    DocElements doc = parseComment(p);

    const string name = getRelativeTypeString(p, swiftModule);
    const string prx = name + "Prx";

    if (doc.overview.empty())
    {
        out << nl << "/// " << prx << " overview.";
    }
    else
    {
        writeDocLines(out, doc.overview);
    }

    const OperationList ops = p->operations();
    if (!ops.empty())
    {
        out << nl << "///";
        out << nl << "/// " << prx << " Methods:";
        for (OperationList::const_iterator q = ops.begin(); q != ops.end(); ++q)
        {
            OperationPtr op = *q;
            DocElements opdoc = parseComment(op);
            out << nl << "///";
            out << nl << "///  - " << fixIdent(op->name()) << ": ";
            if (!opdoc.overview.empty())
            {
                writeDocSentence(out, opdoc.overview);
            }

            out << nl << "///";
            out << nl << "///  - " << op->name() << "Async: ";
            if (!opdoc.overview.empty())
            {
                writeDocSentence(out, opdoc.overview);
            }
        }
    }

    if (!doc.misc.empty())
    {
        writeDocLines(out, doc.misc, false);
    }
}

void
SwiftGenerator::writeServantDocSummary(IceInternal::Output& out, const InterfaceDefPtr& p, const string& swiftModule)
{
    DocElements doc = parseComment(p);

    const string name = getRelativeTypeString(p, swiftModule);

    if (doc.overview.empty())
    {
        out << nl << "/// " << name << " overview.";
    }
    else
    {
        writeDocLines(out, doc.overview);
    }

    const OperationList ops = p->operations();
    if (!ops.empty())
    {
        out << nl << "///";
        out << nl << "/// " << name << " Methods:";
        for (OperationList::const_iterator q = ops.begin(); q != ops.end(); ++q)
        {
            OperationPtr op = *q;
            DocElements opdoc = parseComment(op);
            out << nl << "///";
            out << nl << "///  - " << fixIdent(op->name()) << ": ";
            if (!opdoc.overview.empty())
            {
                writeDocSentence(out, opdoc.overview);
            }
        }
    }

    if (!doc.misc.empty())
    {
        writeDocLines(out, doc.misc, false);
    }
}

void
SwiftGenerator::writeMemberDoc(IceInternal::Output& out, const DataMemberPtr& p)
{
    DocElements doc = parseComment(p);

    //
    // Skip if there are no doc comments.
    //
    if (doc.overview.empty() && doc.misc.empty() && doc.seeAlso.empty() && doc.deprecateReason.empty() &&
        !doc.deprecated)
    {
        return;
    }

    if (doc.overview.empty())
    {
        out << nl << "/// " << fixIdent(p->name());
    }
    else
    {
        writeDocLines(out, doc.overview);
    }

    if (!doc.misc.empty())
    {
        writeDocLines(out, doc.misc);
    }

    if (doc.deprecated)
    {
        out << nl << "/// ##Deprecated";
        if (!doc.deprecateReason.empty())
        {
            writeDocLines(out, doc.deprecateReason);
        }
    }
}

void
SwiftGenerator::validateMetadata(const UnitPtr& u)
{
    MetadataVisitor visitor;
    u->visit(&visitor);
}

string
SwiftGenerator::getRelativeTypeString(const ContainedPtr& contained, const string& currentModule)
{
    string typeString = contained->scoped();

    // Proxy types always end with "Prx".
    if (dynamic_pointer_cast<InterfaceDecl>(contained))
    {
        typeString += "Prx";
    }

    // Remove the leading '::' the Slice compiler always gives us.
    assert(typeString.find("::") == 0);
    typeString.erase(0, 2);
    // Replace the definition top-level module by the corresponding Swift module
    // and append the Swift prefix for the Slice module, then any remaining nested
    // modules become a Swift prefix
    size_t pos = typeString.find("::");
    if (pos != string::npos)
    {
        typeString = typeString.substr(pos + 2);
    }
    string swiftPrefix;
    string swiftModule = getSwiftModule(getTopLevelModule(contained), swiftPrefix);
    auto absoluteIdent = swiftModule + "." + swiftPrefix + replace(typeString, "::", "");
    return getUnqualified(absoluteIdent, currentModule);
}

string
SwiftGenerator::getValue(const string& swiftModule, const TypePtr& type)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindBool:
            {
                return "false";
            }
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindLong:
            {
                return "0";
            }
            case Builtin::KindFloat:
            case Builtin::KindDouble:
            {
                return "0.0";
            }
            case Builtin::KindString:
            {
                return "\"\"";
            }
            default:
            {
                return "nil";
            }
        }
    }

    EnumPtr en = dynamic_pointer_cast<Enum>(type);
    if (en)
    {
        return "." + fixIdent((*en->enumerators().begin())->name());
    }

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st)
    {
        return getRelativeTypeString(st, swiftModule) + "()";
    }

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        return getRelativeTypeString(seq, swiftModule) + "()";
    }

    DictionaryPtr dict = dynamic_pointer_cast<Dictionary>(type);
    if (dict)
    {
        return getRelativeTypeString(dict, swiftModule) + "()";
    }

    return "nil";
}

void
SwiftGenerator::writeConstantValue(
    IceInternal::Output& out,
    const TypePtr& type,
    const SyntaxTreeBasePtr& valueType,
    const string& value,
    const string& swiftModule,
    bool optional)
{
    ConstPtr constant = dynamic_pointer_cast<Const>(valueType);
    if (constant)
    {
        out << getRelativeTypeString(constant, swiftModule);
    }
    else
    {
        if (valueType)
        {
            BuiltinPtr bp = dynamic_pointer_cast<Builtin>(type);
            EnumPtr ep = dynamic_pointer_cast<Enum>(type);
            if (bp && bp->kind() == Builtin::KindString)
            {
                out << "\"";
                out << toStringLiteral(value, "\n\r\t", "", EC6UCN, 0);
                out << "\"";
            }
            else if (ep)
            {
                assert(valueType);
                EnumeratorPtr enumerator = dynamic_pointer_cast<Enumerator>(valueType);
                assert(enumerator);
                out << getRelativeTypeString(ep, swiftModule) << "." << enumerator->name();
            }
            else
            {
                out << value;
            }
        }
        else if (optional)
        {
            out << "nil";
        }
        else
        {
            out << getValue(swiftModule, type);
        }
    }
}

string
SwiftGenerator::typeToString(
    const TypePtr& type,
    const ContainedPtr& toplevel,
    const MetadataList& metadata,
    bool optional)
{
    static const char* builtinTable[] = {
        "Swift.UInt8",
        "Swift.Bool",
        "Swift.Int16",
        "Swift.Int32",
        "Swift.Int64",
        "Swift.Float",
        "Swift.Double",
        "Swift.String",
        "Ice.Disp",      // Object
        "Ice.ObjectPrx", // ObjectPrx
        "Ice.Value"      // Value
    };

    if (!type)
    {
        return "";
    }

    string t = "";
    //
    // The current module where the type is being used
    //
    string currentModule = getSwiftModule(getTopLevelModule(toplevel));
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);

    if (builtin)
    {
        if (builtin->kind() == Builtin::KindObject)
        {
            t = getUnqualified(builtinTable[Builtin::KindValue], currentModule);
        }
        else
        {
            t = getUnqualified(builtinTable[builtin->kind()], currentModule);
        }
    }

    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    InterfaceDeclPtr prx = dynamic_pointer_cast<InterfaceDecl>(type);
    ContainedPtr cont = dynamic_pointer_cast<Contained>(type);

    if (cl)
    {
        t += fixIdent(getRelativeTypeString(cl, currentModule));
    }
    else if (prx)
    {
        t = getRelativeTypeString(prx, currentModule);
    }
    else if (cont)
    {
        t = fixIdent(getRelativeTypeString(cont, currentModule));
    }

    bool nonnull = false;
    for (const auto& meta : metadata)
    {
        if (meta->directive() == "swift:nonnull")
        {
            nonnull = true;
            break;
        }
    }
    if (!nonnull && (optional || isNullableType(type)))
    {
        t += "?";
    }
    return t;
}

string
SwiftGenerator::getUnqualified(const string& type, const string& localModule)
{
    const string prefix = localModule + ".";
    return type.find(prefix) == 0 ? type.substr(prefix.size()) : type;
}

string
SwiftGenerator::modeToString(Operation::Mode opMode)
{
    string mode;
    switch (opMode)
    {
        case Operation::Normal:
        {
            mode = ".Normal";
            break;
        }
        case Operation::Idempotent:
        {
            mode = ".Idempotent";
            break;
        }
        default:
        {
            assert(false);
            break;
        }
    }
    return mode;
}

string
SwiftGenerator::getOptionalFormat(const TypePtr& type)
{
    BuiltinPtr bp = dynamic_pointer_cast<Builtin>(type);
    if (bp)
    {
        switch (bp->kind())
        {
            case Builtin::KindByte:
            case Builtin::KindBool:
            {
                return ".F1";
            }
            case Builtin::KindShort:
            {
                return ".F2";
            }
            case Builtin::KindInt:
            case Builtin::KindFloat:
            {
                return ".F4";
            }
            case Builtin::KindLong:
            case Builtin::KindDouble:
            {
                return ".F8";
            }
            case Builtin::KindString:
            {
                return ".VSize";
            }
            case Builtin::KindObjectProxy:
            {
                return ".FSize";
            }
            case Builtin::KindObject:
            case Builtin::KindValue:
            {
                return ".Class";
            }
        }
    }

    if (dynamic_pointer_cast<Enum>(type))
    {
        return ".Size";
    }

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        return seq->type()->isVariableLength() ? ".FSize" : ".VSize";
    }

    DictionaryPtr d = dynamic_pointer_cast<Dictionary>(type);
    if (d)
    {
        return (d->keyType()->isVariableLength() || d->valueType()->isVariableLength()) ? ".FSize" : ".VSize";
    }

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st)
    {
        return st->isVariableLength() ? ".FSize" : ".VSize";
    }

    if (dynamic_pointer_cast<InterfaceDecl>(type))
    {
        return ".FSize";
    }

    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    assert(cl);
    return ".Class";
}

bool
SwiftGenerator::isNullableType(const TypePtr& type)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindValue:
            {
                return true;
            }
            default:
            {
                return false;
            }
        }
    }

    return dynamic_pointer_cast<ClassDecl>(type) || dynamic_pointer_cast<InterfaceDecl>(type);
}

bool
SwiftGenerator::isProxyType(const TypePtr& p)
{
    const BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(p);
    return (builtin && builtin->kind() == Builtin::KindObjectProxy) || dynamic_pointer_cast<InterfaceDecl>(p);
}

void
SwiftGenerator::writeDefaultInitializer(IceInternal::Output& out, bool required, bool rootClass)
{
    out << sp;
    out << nl << "public ";
    if (required)
    {
        out << "required ";
    }
    if (rootClass)
    {
        out << "init() {}";
    }
    else
    {
        assert(required);
        out << "init()";
        out << sb;
        out << nl << "super.init()";
        out << eb;
    }
}

void
SwiftGenerator::writeMemberwiseInitializer(
    IceInternal::Output& out,
    const DataMemberList& members,
    const ContainedPtr& p)
{
    writeMemberwiseInitializer(out, members, DataMemberList(), members, p, true);
}

void
SwiftGenerator::writeMemberwiseInitializer(
    IceInternal::Output& out,
    const DataMemberList& members,
    const DataMemberList& baseMembers,
    const DataMemberList& allMembers,
    const ContainedPtr& p,
    bool rootClass)
{
    if (!members.empty())
    {
        out << sp;
        out << nl;
        out << "public init" << spar;
        for (DataMemberList::const_iterator i = allMembers.begin(); i != allMembers.end(); ++i)
        {
            DataMemberPtr m = *i;
            out << (fixIdent(m->name()) + ": " + typeToString(m->type(), p, m->getMetadata(), m->optional()));
        }
        out << epar;
        out << sb;
        for (DataMemberList::const_iterator i = members.begin(); i != members.end(); ++i)
        {
            DataMemberPtr m = *i;
            out << nl << "self." << fixIdent(m->name()) << " = " << fixIdent(m->name());
        }

        if (!rootClass)
        {
            out << nl << "super.init";
            out << spar;
            for (DataMemberList::const_iterator i = baseMembers.begin(); i != baseMembers.end(); ++i)
            {
                const string name = fixIdent((*i)->name());
                out << (name + ": " + name);
            }
            out << epar;
        }
        out << eb;
    }
}

void
SwiftGenerator::writeMembers(
    IceInternal::Output& out,
    const DataMemberList& members,
    const ContainedPtr& p,
    int typeCtx)
{
    string swiftModule = getSwiftModule(getTopLevelModule(p));
    bool protocol = (typeCtx & TypeContextProtocol) != 0;
    string access = protocol ? "" : "public ";
    for (DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;
        TypePtr type = member->type();
        const string defaultValue = member->defaultValue();

        const string memberName = fixIdent(member->name());
        string memberType = typeToString(type, p, member->getMetadata(), member->optional());

        //
        // If the member type is equal to the member name, create a local type alias
        // to avoid ambiguity.
        //
        string alias;
        if (!protocol && memberName == memberType &&
            (dynamic_pointer_cast<Struct>(type) || dynamic_pointer_cast<Sequence>(type) ||
             dynamic_pointer_cast<Dictionary>(type)))
        {
            ModulePtr m = getTopLevelModule(type);
            alias = m->name() + "_" + memberType;
            out << nl << "typealias " << alias << " = " << memberType;
        }

        writeMemberDoc(out, member);
        out << nl << access << "var " << memberName << ": " << memberType;
        if (protocol)
        {
            out << " { get set }";
        }
        else
        {
            out << " = ";
            if (alias.empty())
            {
                writeConstantValue(
                    out,
                    type,
                    member->defaultValueType(),
                    defaultValue,
                    swiftModule,
                    member->optional());
            }
            else
            {
                out << alias << "()";
            }
        }
    }
}

bool
SwiftGenerator::usesMarshalHelper(const TypePtr& type)
{
    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(seq->type());
        if (builtin)
        {
            return builtin->kind() > Builtin::KindString;
        }
        return true;
    }
    return dynamic_pointer_cast<Dictionary>(type) != nullptr;
}

void
SwiftGenerator::writeMarshalUnmarshalCode(
    Output& out,
    const TypePtr& type,
    const ContainedPtr& p,
    const string& param,
    bool marshal,
    int tag)
{
    assert(!(type->isClassType() && tag >= 0)); // Optional classes are disallowed by the parser.

    string swiftModule = getSwiftModule(getTopLevelModule(p));
    string stream = dynamic_pointer_cast<Struct>(p) ? "self" : marshal ? "ostr" : "istr";

    string args;
    if (tag >= 0)
    {
        args += "tag: " + std::to_string(tag);
        if (marshal)
        {
            args += ", ";
        }
    }

    if (marshal)
    {
        if (tag >= 0 || usesMarshalHelper(type))
        {
            args += "value: ";
        }
        args += param;
    }

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindByte:
            case Builtin::KindBool:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindLong:
            case Builtin::KindFloat:
            case Builtin::KindDouble:
            case Builtin::KindString:
            {
                if (marshal)
                {
                    out << nl << stream << ".write(" << args << ")";
                }
                else
                {
                    out << nl << param << " = try " << stream << ".read(" << args << ")";
                }
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if (marshal)
                {
                    out << nl << stream << ".write(" << args << ")";
                }
                else
                {
                    if (tag >= 0)
                    {
                        args += ", type: ";
                    }
                    args += getUnqualified("Ice.ObjectPrx", swiftModule) + ".self";

                    out << nl << param << " = try " << stream << ".read(" << args << ")";
                }
                break;
            }
            case Builtin::KindObject:
            case Builtin::KindValue:
            {
                if (marshal)
                {
                    out << nl << stream << ".write(" << args << ")";
                }
                else
                {
                    out << nl << "try " << stream << ".read(" << args << ") { " << param << " = $0 }";
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }

    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    if (cl)
    {
        if (marshal)
        {
            out << nl << stream << ".write(" << args << ")";
        }
        else
        {
            string memberType = getRelativeTypeString(cl, swiftModule);
            string memberName;
            const string memberPrefix = "self.";
            if (param.find(memberPrefix) == 0)
            {
                memberName = param.substr(memberPrefix.size());
            }

            string alias;
            //
            // If the member type is equal to the member name, create a local type alias
            // to avoid ambiguity.
            //
            if (memberType == memberName)
            {
                ModulePtr m = getTopLevelModule(type);
                alias = m->name() + "_" + memberType;
                out << nl << "typealias " << alias << " = " << memberType;
            }
            args += fixIdent(alias.empty() ? memberType : alias) + ".self";
            out << nl << "try " << stream << ".read(" << args << ") { " << param << " = $0 "
                << "}";
        }
        return;
    }

    EnumPtr en = dynamic_pointer_cast<Enum>(type);
    if (en)
    {
        if (marshal)
        {
            out << nl << stream << ".write(" << args << ")";
        }
        else
        {
            out << nl << param << " = try " << stream << ".read(" << args << ")";
        }
        return;
    }

    InterfaceDeclPtr prx = dynamic_pointer_cast<InterfaceDecl>(type);
    if (prx)
    {
        if (marshal)
        {
            out << nl << stream << ".write(" << args << ")";
        }
        else
        {
            if (tag >= 0)
            {
                args += ", type: ";
            }

            args += getRelativeTypeString(prx, swiftModule) + ".self";
            out << nl << param << " = try " << stream << ".read(" << args << ")";
        }
        return;
    }

    if (dynamic_pointer_cast<Struct>(type))
    {
        if (marshal)
        {
            out << nl << stream << ".write(" << args << ")";
        }
        else
        {
            out << nl << param << " = try " << stream << ".read(" << args << ")";
        }
        return;
    }

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        BuiltinPtr seqBuiltin = dynamic_pointer_cast<Builtin>(seq->type());
        if (seqBuiltin && seqBuiltin->kind() <= Builtin::KindString)
        {
            if (marshal)
            {
                out << nl << stream << ".write(" << args << ")";
            }
            else
            {
                out << nl << param << " = try " << stream << ".read(" << args << ")";
            }
        }
        else
        {
            string helper = getRelativeTypeString(seq, swiftModule) + "Helper";
            if (marshal)
            {
                out << nl << helper << ".write(to: " << stream << ", " << args << ")";
            }
            else
            {
                out << nl << param << " = try " << helper << ".read(from: " << stream;
                if (!args.empty())
                {
                    out << ", " << args;
                }
                out << ")";
            }
        }
        return;
    }

    DictionaryPtr dict = dynamic_pointer_cast<Dictionary>(type);
    if (dict)
    {
        string helper = getRelativeTypeString(dict, swiftModule) + "Helper";
        if (marshal)
        {
            out << nl << helper << ".write(to: " << stream << ", " << args << ")";
        }
        else
        {
            out << nl << param << " = try " << helper << ".read(from: " << stream;
            if (!args.empty())
            {
                out << ", " << args;
            }
            out << ")";
        }
        return;
    }
}

bool
SwiftGenerator::MetadataVisitor::visitModuleStart(const ModulePtr& p)
{
    if (dynamic_pointer_cast<Unit>(p->container()))
    {
        // top-level module
        const UnitPtr unit = p->unit();

        string swiftPrefix;
        string swiftModule = getSwiftModule(p, swiftPrefix);

        const string filename = p->definitionContext()->filename();
        ModuleMap::const_iterator current = _modules.find(filename);

        if (current == _modules.end())
        {
            _modules[filename] = swiftModule;
        }
        else if (current->second != swiftModule)
        {
            ostringstream os;
            os << "invalid module mapping:\n Slice module `" << p->scoped() << "' should map to Swift module `"
               << current->second << "'" << endl;
            unit->error(p->file(), p->line(), os.str());
        }

        ModulePrefix::iterator prefixes = _prefixes.find(swiftModule);
        if (prefixes == _prefixes.end())
        {
            ModuleMap mappings;
            mappings[p->name()] = swiftPrefix;
            _prefixes[swiftModule] = mappings;
        }
        else
        {
            current = prefixes->second.find(p->name());
            if (current == prefixes->second.end())
            {
                prefixes->second[p->name()] = swiftPrefix;
            }
            else if (current->second != swiftPrefix)
            {
                ostringstream os;
                os << "invalid module prefix:\n Slice module `" << p->scoped() << "' is already using";
                if (current->second.empty())
                {
                    os << " no prefix " << endl;
                }
                else
                {
                    os << " a different Swift module prefix `" << current->second << "'" << endl;
                }
                unit->error(p->file(), p->line(), os.str());
            }
        }
    }
    p->setMetadata(validate(p, p->getMetadata(), p->file(), p->line()));
    return true;
}

string
SwiftGenerator::paramLabel(const string& param, const ParamDeclList& params)
{
    string s = param;
    for (ParamDeclList::const_iterator q = params.begin(); q != params.end(); ++q)
    {
        if ((*q)->name() == param)
        {
            s = "_" + s;
            break;
        }
    }
    return s;
}

bool
SwiftGenerator::operationReturnIsTuple(const OperationPtr& op)
{
    ParamDeclList outParams = op->outParameters();
    return (op->returnType() && outParams.size() > 0) || outParams.size() > 1;
}

string
SwiftGenerator::operationReturnType(const OperationPtr& op)
{
    ostringstream os;
    bool returnIsTuple = operationReturnIsTuple(op);
    if (returnIsTuple)
    {
        os << "(";
    }

    ParamDeclList outParams = op->outParameters();
    TypePtr returnType = op->returnType();
    if (returnType)
    {
        if (returnIsTuple)
        {
            os << paramLabel("returnValue", outParams) << ": ";
        }
        os << typeToString(returnType, op, op->getMetadata(), op->returnIsOptional());
    }

    for (ParamDeclList::const_iterator q = outParams.begin(); q != outParams.end(); ++q)
    {
        if (returnType || q != outParams.begin())
        {
            os << ", ";
        }

        if (returnIsTuple)
        {
            os << (*q)->name() << ": ";
        }

        os << typeToString((*q)->type(), *q, (*q)->getMetadata(), (*q)->optional());
    }

    if (returnIsTuple)
    {
        os << ")";
    }

    return os.str();
}

std::string
SwiftGenerator::operationReturnDeclaration(const OperationPtr& op)
{
    ostringstream os;
    ParamDeclList outParams = op->outParameters();
    TypePtr returnType = op->returnType();
    bool returnIsTuple = operationReturnIsTuple(op);

    if (returnIsTuple)
    {
        os << "(";
    }

    if (returnType)
    {
        os << ("iceP_" + paramLabel("returnValue", outParams));
    }

    for (ParamDeclList::const_iterator q = outParams.begin(); q != outParams.end(); ++q)
    {
        if (returnType || q != outParams.begin())
        {
            os << ", ";
        }

        os << ("iceP_" + (*q)->name());
    }

    if (returnIsTuple)
    {
        os << ")";
    }

    return os.str();
}

string
SwiftGenerator::operationInParamsDeclaration(const OperationPtr& op)
{
    ostringstream os;

    ParamDeclList inParams = op->inParameters();
    const bool isTuple = inParams.size() > 1;

    if (!inParams.empty())
    {
        if (isTuple)
        {
            os << "(";
        }
        for (ParamDeclList::const_iterator q = inParams.begin(); q != inParams.end(); ++q)
        {
            if (q != inParams.begin())
            {
                os << ", ";
            }

            os << ("iceP_" + (*q)->name());
        }
        if (isTuple)
        {
            os << ")";
        }

        os << ": ";

        if (isTuple)
        {
            os << "(";
        }
        for (ParamDeclList::const_iterator q = inParams.begin(); q != inParams.end(); ++q)
        {
            if (q != inParams.begin())
            {
                os << ", ";
            }

            os << typeToString((*q)->type(), *q, (*q)->getMetadata(), (*q)->optional());
        }
        if (isTuple)
        {
            os << ")";
        }
    }

    return os.str();
}

ParamInfoList
SwiftGenerator::getAllInParams(const OperationPtr& op)
{
    const ParamDeclList l = op->inParameters();
    ParamInfoList r;
    for (ParamDeclList::const_iterator p = l.begin(); p != l.end(); ++p)
    {
        ParamInfo info;
        info.name = (*p)->name();
        info.type = (*p)->type();
        info.typeStr = typeToString(info.type, op, (*p)->getMetadata(), (*p)->optional());
        info.optional = (*p)->optional();
        info.tag = (*p)->tag();
        info.param = *p;
        r.push_back(info);
    }
    return r;
}

void
SwiftGenerator::getInParams(const OperationPtr& op, ParamInfoList& required, ParamInfoList& optional)
{
    const ParamInfoList params = getAllInParams(op);
    for (ParamInfoList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        if (p->optional)
        {
            optional.push_back(*p);
        }
        else
        {
            required.push_back(*p);
        }
    }

    //
    // Sort optional parameters by tag.
    //
    class SortFn
    {
    public:
        static bool compare(const ParamInfo& lhs, const ParamInfo& rhs) { return lhs.tag < rhs.tag; }
    };
    optional.sort(SortFn::compare);
}

ParamInfoList
SwiftGenerator::getAllOutParams(const OperationPtr& op)
{
    ParamDeclList params = op->outParameters();
    ParamInfoList l;

    for (ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        ParamInfo info;
        info.name = (*p)->name();
        info.type = (*p)->type();
        info.typeStr = typeToString(info.type, op, (*p)->getMetadata(), (*p)->optional());
        info.optional = (*p)->optional();
        info.tag = (*p)->tag();
        info.param = *p;
        l.push_back(info);
    }

    if (op->returnType())
    {
        ParamInfo info;
        info.name = paramLabel("returnValue", params);
        info.type = op->returnType();
        info.typeStr = typeToString(info.type, op, op->getMetadata(), op->returnIsOptional());
        info.optional = op->returnIsOptional();
        info.tag = op->returnTag();
        l.push_back(info);
    }

    return l;
}

void
SwiftGenerator::getOutParams(const OperationPtr& op, ParamInfoList& required, ParamInfoList& optional)
{
    const ParamInfoList params = getAllOutParams(op);
    for (ParamInfoList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        if (p->optional)
        {
            optional.push_back(*p);
        }
        else
        {
            required.push_back(*p);
        }
    }

    //
    // Sort optional parameters by tag.
    //
    class SortFn
    {
    public:
        static bool compare(const ParamInfo& lhs, const ParamInfo& rhs) { return lhs.tag < rhs.tag; }
    };
    optional.sort(SortFn::compare);
}

void
SwiftGenerator::writeMarshalInParams(::IceInternal::Output& out, const OperationPtr& op)
{
    ParamInfoList requiredInParams, optionalInParams;
    getInParams(op, requiredInParams, optionalInParams);

    out << "{ ostr in";
    out.inc();
    //
    // Marshal parameters
    // 1. required
    // 2. optional
    //

    for (ParamInfoList::const_iterator q = requiredInParams.begin(); q != requiredInParams.end(); ++q)
    {
        writeMarshalUnmarshalCode(out, q->type, op, "iceP_" + q->name, true);
    }

    for (ParamInfoList::const_iterator q = optionalInParams.begin(); q != optionalInParams.end(); ++q)
    {
        writeMarshalUnmarshalCode(out, q->type, op, "iceP_" + q->name, true, q->tag);
    }

    if (op->sendsClasses())
    {
        out << nl << "ostr.writePendingValues()";
    }
    out.dec();
    out << nl << "}";
}

void
SwiftGenerator::writeMarshalOutParams(::IceInternal::Output& out, const OperationPtr& op)
{
    ParamInfoList requiredOutParams, optionalOutParams;
    getOutParams(op, requiredOutParams, optionalOutParams);

    //
    // Marshal parameters
    // 1. required
    // 2. optional (including optional return)
    //

    for (ParamInfoList::const_iterator q = requiredOutParams.begin(); q != requiredOutParams.end(); ++q)
    {
        writeMarshalUnmarshalCode(out, q->type, op, "iceP_" + q->name, true);
    }

    for (ParamInfoList::const_iterator q = optionalOutParams.begin(); q != optionalOutParams.end(); ++q)
    {
        writeMarshalUnmarshalCode(out, q->type, op, "iceP_" + q->name, true, q->tag);
    }

    if (op->returnsClasses())
    {
        out << nl << "ostr.writePendingValues()";
    }
}

void
SwiftGenerator::writeMarshalAsyncOutParams(::IceInternal::Output& out, const OperationPtr& op)
{
    ParamInfoList requiredOutParams, optionalOutParams;
    getOutParams(op, requiredOutParams, optionalOutParams);

    out << nl << "let " << operationReturnDeclaration(op) << " = value";
    //
    // Marshal parameters
    // 1. required
    // 2. optional (including optional return)
    //

    for (ParamInfoList::const_iterator q = requiredOutParams.begin(); q != requiredOutParams.end(); ++q)
    {
        writeMarshalUnmarshalCode(out, q->type, op, "iceP_" + q->name, true);
    }

    for (ParamInfoList::const_iterator q = optionalOutParams.begin(); q != optionalOutParams.end(); ++q)
    {
        writeMarshalUnmarshalCode(out, q->type, op, "iceP_" + q->name, true, q->tag);
    }

    if (op->returnsClasses())
    {
        out << nl << "ostr.writePendingValues()";
    }
}

void
SwiftGenerator::writeUnmarshalOutParams(::IceInternal::Output& out, const OperationPtr& op)
{
    TypePtr returnType = op->returnType();

    ParamInfoList requiredOutParams, optionalOutParams;
    getOutParams(op, requiredOutParams, optionalOutParams);
    const ParamInfoList allOutParams = getAllOutParams(op);
    //
    // Unmarshal parameters
    // 1. required
    // 2. return
    // 3. optional (including optional return)
    //
    out << "{ istr in";
    out.inc();
    for (ParamInfoList::const_iterator q = requiredOutParams.begin(); q != requiredOutParams.end(); ++q)
    {
        string param;
        if (q->type->isClassType())
        {
            out << nl << "var iceP_" << q->name << ": " << q->typeStr;
            param = "iceP_" + q->name;
        }
        else
        {
            param = "let iceP_" + q->name + ": " + q->typeStr;
        }
        writeMarshalUnmarshalCode(out, q->type, op, param, false);
    }

    for (ParamInfoList::const_iterator q = optionalOutParams.begin(); q != optionalOutParams.end(); ++q)
    {
        string param;
        if (q->type->isClassType())
        {
            out << nl << "var iceP_" << q->name << ": " << q->typeStr;
            param = "iceP_" + q->name;
        }
        else
        {
            param = "let iceP_" + q->name + ": " + q->typeStr;
        }
        writeMarshalUnmarshalCode(out, q->type, op, param, false, q->tag);
    }

    if (op->returnsClasses())
    {
        out << nl << "try istr.readPendingValues()";
    }

    out << nl << "return ";
    if (allOutParams.size() > 1)
    {
        out << spar;
    }

    if (returnType)
    {
        out << ("iceP_" + paramLabel("returnValue", op->outParameters()));
    }

    for (ParamInfoList::const_iterator q = allOutParams.begin(); q != allOutParams.end(); ++q)
    {
        if (q->param)
        {
            out << ("iceP_" + q->name);
        }
    }

    if (allOutParams.size() > 1)
    {
        out << epar;
    }

    out.dec();
    out << nl << "}";
}

void
SwiftGenerator::writeUnmarshalInParams(::IceInternal::Output& out, const OperationPtr& op)
{
    ParamInfoList requiredInParams, optionalInParams;
    getInParams(op, requiredInParams, optionalInParams);
    const ParamInfoList allInParams = getAllInParams(op);
    //
    // Unmarshal parameters
    // 1. required
    // 3. optional
    //
    for (ParamInfoList::const_iterator q = requiredInParams.begin(); q != requiredInParams.end(); ++q)
    {
        if (q->param)
        {
            string param;
            if (q->type->isClassType())
            {
                out << nl << "var iceP_" << q->name << ": " << q->typeStr;
                param = "iceP_" + q->name;
            }
            else
            {
                param = "let iceP_" + q->name + ": " + q->typeStr;
            }
            writeMarshalUnmarshalCode(out, q->type, op, param, false);
        }
    }

    for (ParamInfoList::const_iterator q = optionalInParams.begin(); q != optionalInParams.end(); ++q)
    {
        string param;
        if (q->type->isClassType())
        {
            out << nl << "var iceP_" << q->name << ": " << q->typeStr;
            param = "iceP_" + q->name;
        }
        else
        {
            param = "let iceP_" + q->name + ": " + q->typeStr;
        }
        writeMarshalUnmarshalCode(out, q->type, op, param, false, q->tag);
    }

    if (op->sendsClasses())
    {
        out << nl << "try istr.readPendingValues()";
    }
}

void
SwiftGenerator::writeUnmarshalUserException(::IceInternal::Output& out, const OperationPtr& op)
{
    const string swiftModule = getSwiftModule(getTopLevelModule(dynamic_pointer_cast<Contained>(op)));
    ExceptionList throws = op->throws();
    throws.sort();
    throws.unique();

    out << "{ ex in";
    out.inc();
    out << nl << "do ";
    out << sb;
    out << nl << "throw ex";
    out << eb;

    for (const auto& thrown : throws)
    {
        out << " catch let error as " << fixIdent(getRelativeTypeString(thrown, swiftModule)) << sb;
        out << nl << "throw error";
        out << eb;
    }
    out << " catch is " << getUnqualified("Ice.UserException", swiftModule) << " {}";
    out.dec();
    out << nl << "}";
}

void
SwiftGenerator::writeSwiftAttributes(::IceInternal::Output& out, const MetadataList& metadata)
{
    for (const auto& meta : metadata)
    {
        if (meta->directive() == "swift:attribute")
        {
            out << nl << meta->arguments();
        }
    }
}

void
SwiftGenerator::writeProxyOperation(::IceInternal::Output& out, const OperationPtr& op)
{
    const string opName = fixIdent(op->name());

    const ParamInfoList allInParams = getAllInParams(op);
    const ParamInfoList allOutParams = getAllOutParams(op);
    const ExceptionList allExceptions = op->throws();

    const string swiftModule = getSwiftModule(getTopLevelModule(dynamic_pointer_cast<Contained>(op)));

    out << sp;
    writeOpDocSummary(out, op, false);
    out << nl << "func " << opName;
    out << spar;
    for (ParamInfoList::const_iterator q = allInParams.begin(); q != allInParams.end(); ++q)
    {
        if (allInParams.size() == 1)
        {
            out << ("_ iceP_" + q->name + ": " + q->typeStr + (q->optional ? " = nil" : ""));
        }
        else
        {
            out << (q->name + " iceP_" + q->name + ": " + q->typeStr + (q->optional ? " = nil" : ""));
        }
    }
    out << "context: " + getUnqualified("Ice.Context", swiftModule) + "? = nil";
    out << epar;
    out << " async throws -> ";
    if (allOutParams.empty())
    {
        out << "Swift.Void";
    }
    else
    {
        out << operationReturnType(op);
    }

    out << sb;

    //
    // Invoke
    //
    out << sp;
    out << nl << "return try await _impl._invoke(";

    out.useCurrentPosAsIndent();
    out << "operation: \"" << op->name() << "\",";
    out << nl << "mode: " << modeToString(op->mode()) << ",";

    if (op->format())
    {
        out << nl << "format: " << opFormatTypeToString(op);
        out << ",";
    }

    if (allInParams.size() > 0)
    {
        out << nl << "write: ";
        writeMarshalInParams(out, op);
        out << ",";
    }

    if (allOutParams.size() > 0)
    {
        out << nl << "read: ";
        writeUnmarshalOutParams(out, op);
        out << ",";
    }

    if (allExceptions.size() > 0)
    {
        out << nl << "userException:";
        writeUnmarshalUserException(out, op);
        out << ",";
    }

    out << nl << "context: context)";
    out.restoreIndent();

    out << eb;
}

void
SwiftGenerator::writeDispatchOperation(::IceInternal::Output& out, const OperationPtr& op)
{
    const string opName = op->name();

    const ParamInfoList inParams = getAllInParams(op);
    const ParamInfoList outParams = getAllOutParams(op);

    const string swiftModule = getSwiftModule(getTopLevelModule(dynamic_pointer_cast<Contained>(op)));

    out << sp;
    out << nl << "public func _iceD_" << opName
        << "(_ request: Ice.IncomingRequest) async throws -> Ice.OutgoingResponse";

    out << sb;
    out << nl;

    // TODO: check operation mode

    if (inParams.empty())
    {
        out << nl << "_ = try request.inputStream.skipEmptyEncapsulation()";
    }
    else
    {
        out << nl << "let istr = request.inputStream";
        out << nl << "_ = try istr.startEncapsulation()";
        writeUnmarshalInParams(out, op);
    }

    out << nl;
    if (!outParams.empty())
    {
        out << "let result = ";
    }

    out << "try await self." << fixIdent(opName);
    out << spar;
    for (const auto& q : inParams)
    {
        out << (q.name + ": iceP_" + q.name);
    }
    out << "current: request.current";
    out << epar;

    if (outParams.empty())
    {
        out << nl << "return request.current.makeEmptyOutgoingResponse()";
    }
    else
    {
        out << nl << "return request.current.makeOutgoingResponse(result, formatType: " << opFormatTypeToString(op)
            << ")";
        out << sb;
        out << " ostr, value in ";
        writeMarshalAsyncOutParams(out, op);
        out << eb;
    }

    out << eb;
}

bool
SwiftGenerator::MetadataVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    p->setMetadata(validate(p, p->getMetadata(), p->file(), p->line()));
    for (const auto& member : p->dataMembers())
    {
        // TODO we should probably be passing `member` instead of `member->type()`.
        // Otherwise I'm pretty sure we're just skipping the data-member metadata.
        member->setMetadata(validate(member->type(), member->getMetadata(), p->file(), member->line()));
    }
    return true;
}

bool
SwiftGenerator::MetadataVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    p->setMetadata(validate(p, p->getMetadata(), p->file(), p->line()));
    return true;
}

void
SwiftGenerator::MetadataVisitor::visitOperation(const OperationPtr& p)
{
    MetadataList metadata = p->getMetadata();

    const UnitPtr ut = p->unit();
    const DefinitionContextPtr dc = ut->findDefinitionContext(p->file());
    assert(dc);

    for (MetadataList::iterator q = metadata.begin(); q != metadata.end();)
    {
        MetadataPtr s = *q++;
        string_view directive = s->directive();
        if (directive == "swift:attribute" || directive == "swift:type" || directive == "swift:noexcept" ||
            directive == "swift:nonnull")
        {
            string message = "ignoring metadata '" + string(directive) + "' for non local operation";
            dc->warning(InvalidMetadata, p->file(), p->line(), message);
            metadata.remove(s);
        }
    }
    p->setMetadata(validate(p, metadata, p->file(), p->line()));
    for (const auto& param : p->parameters())
    {
        param->setMetadata(validate(param->type(), param->getMetadata(), param->file(), param->line()));
    }
}

bool
SwiftGenerator::MetadataVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    p->setMetadata(validate(p, p->getMetadata(), p->file(), p->line()));
    for (const auto& member : p->dataMembers())
    {
        member->setMetadata(validate(member->type(), member->getMetadata(), member->file(), member->line()));
    }
    return true;
}

bool
SwiftGenerator::MetadataVisitor::visitStructStart(const StructPtr& p)
{
    p->setMetadata(validate(p, p->getMetadata(), p->file(), p->line()));
    for (const auto& member : p->dataMembers())
    {
        member->setMetadata(validate(member->type(), member->getMetadata(), member->file(), member->line()));
    }
    return true;
}

void
SwiftGenerator::MetadataVisitor::visitSequence(const SequencePtr& p)
{
    p->setMetadata(validate(p, p->getMetadata(), p->file(), p->line()));
}

void
SwiftGenerator::MetadataVisitor::visitDictionary(const DictionaryPtr& p)
{
    const string prefix = "swift:";
    const DefinitionContextPtr dc = p->unit()->findDefinitionContext(p->file());
    assert(dc);

    StringList newMetadata = p->keyMetadata();
    for (StringList::const_iterator q = newMetadata.begin(); q != newMetadata.end();)
    {
        string s = *q++;
        if (s.find(prefix) != 0)
        {
            continue;
        }

        dc->warning(
            InvalidMetadata,
            p->file(),
            p->line(),
            "ignoring invalid metadata '" + s + "' for dictionary key type");
    }

    newMetadata = p->valueMetadata();
    TypePtr t = p->valueType();
    for (StringList::const_iterator q = newMetadata.begin(); q != newMetadata.end();)
    {
        string s = *q++;
        if (s.find(prefix) != 0)
        {
            continue;
        }

        dc->warning(
            InvalidMetadata,
            p->file(),
            p->line(),
            "ignoring invalid metadata '" + s + "' for dictionary value type");
    }

    p->setMetadata(validate(p, p->getMetadata(), p->file(), p->line()));
}

void
SwiftGenerator::MetadataVisitor::visitEnum(const EnumPtr& p)
{
    p->setMetadata(validate(p, p->getMetadata(), p->file(), p->line()));
}

void
SwiftGenerator::MetadataVisitor::visitConst(const ConstPtr& p)
{
    p->setMetadata(validate(p, p->getMetadata(), p->file(), p->line()));
}

StringList
SwiftGenerator::MetadataVisitor::validate(
    const SyntaxTreeBasePtr& cont,
    const StringList& metadata,
    const string& file,
    int line)
{
    StringList newMetadata = metadata;
    const string prefix = "swift:";
    const UnitPtr ut = cont->unit();
    const DefinitionContextPtr dc = ut->findDefinitionContext(file);
    assert(dc);
    for (StringList::const_iterator p = newMetadata.begin(); p != newMetadata.end();)
    {
        string s = *p++;
        if (s.find(prefix) != 0)
        {
            continue;
        }

        if (dynamic_pointer_cast<Module>(cont) && s.find("swift:module:") == 0)
        {
            continue;
        }

        if (dynamic_pointer_cast<InterfaceDef>(cont) && s.find("swift:inherits:") == 0)
        {
            continue;
        }

        if ((dynamic_pointer_cast<ClassDef>(cont) || dynamic_pointer_cast<InterfaceDef>(cont) ||
             dynamic_pointer_cast<Enum>(cont) || dynamic_pointer_cast<Exception>(cont) ||
             dynamic_pointer_cast<Operation>(cont)) &&
            s.find("swift:attribute:") == 0)
        {
            continue;
        }

        dc->warning(InvalidMetadata, file, line, "ignoring invalid metadata `" + s + "'");
        newMetadata.remove(s);
        continue;
    }
    return newMetadata;
}
