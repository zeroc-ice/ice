//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
//

#include <IceUtil/OutputUtil.h>
#include <IceUtil/StringUtil.h>

#include <Slice/Util.h>

#include <SwiftUtil.h>

#include <functional>

using namespace std;
using namespace Slice;
using namespace IceUtilInternal;

namespace
{

const std::array<std::string, 17> builtinTable =
{
    "Swift.Bool",
    "Swift.UInt8",
    "Swift.Int16",
    "Swift.UInt16",
    "Swift.Int32",
    "Swift.UInt32",
    "Swift.Int32",
    "Swift.UInt32",
    "Swift.Int64",
    "Swift.UInt64",
    "Swift.Int64",
    "Swift.UInt64",
    "Swift.Float",
    "Swift.Double",
    "Swift.String",
    "Ice.ObjectPrx",    // Object
    "Ice.Value"         // Value
};

static string
lookupKwd(const string& name)
{
    //
    // Keyword list. *Must* be kept in alphabetical order.
    //
    static const string keywordList[] =
    {
        "Any", "as", "associatedtype", "associativity", "break",
        "case", "catch", "class", "continue", "convenience", "default", "defer", "deinit",
        "didSet", "do", "dynamic", "else", "enum", "extension", "fallthrough", "false",
        "fileprivate", "final", "for", "func", "get", "guard", "if", "import", "in", "indirect",
        "infix", "init", "inout", "internal", "is", "lazy", "left", "let", "mutating", "nil",
        "none", "nonmutating", "open", "operator", "optional", "override", "postfix", "precedence",
        "prefix", "private", "protocol", "public", "repeat", "required", "rethrows", "return",
        "right", "self", "set", "static", "struct", "subscript", "super", "switch", "throw", "throws",
        "true", "try", "Type", "typealias", "unowned", "var", "weak", "where", "while", "willSet"
    };
    bool found = binary_search(&keywordList[0],
                               &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
                               name,
                               Slice::CICompare());
    if(found)
    {
        return "`" + name + "`";
    }

    return name;
}

string
replace(string s, string patt, string val)
{
    string r = s;
    string::size_type pos = r.find(patt);
    while(pos != string::npos)
    {
        r.replace(pos, patt.size(), val);
        pos += val.size();
        pos = r.find(patt, pos);
    }
    return r;
}

string
opFormatTypeToString(const OperationPtr& op )
{
    switch(op->format())
    {
        case DefaultFormat:
        {
            return ".DefaultFormat";
        }
        case CompactFormat:
        {
            return ".CompactFormat";
        }
        case SlicedFormat:
        {
            return ".SlicedFormat";
        }
        default:
        {
            assert(false);
        }
    }
    return "???";
}

}

//
// Check the given identifier against Swift's list of reserved words. If it matches
// a reserved word, then an escaped version is returned with a leading underscore.
//
string
Slice::fixIdent(const string& ident)
{
    if(ident[0] != ':')
    {
        return lookupKwd(ident);
    }
    auto ids = splitScopedName(ident);
    transform(ids.begin(), ids.end(), ids.begin(), lookupKwd);
    ostringstream result;
    for(const auto& id : ids)
    {
        result << "::" << id;
    }
    return result.str();
}

string
Slice::getSwiftModule(const ModulePtr& module, string& swiftPrefix)
{
    const string modulePrefix = "swift:module:";

    string swiftModule;

    if(module->findMetadata(modulePrefix, swiftModule))
    {
        swiftModule = swiftModule.substr(modulePrefix.size());

        size_t pos = swiftModule.find(':');
        if(pos != string::npos)
        {
            swiftPrefix = swiftModule.substr(pos + 1);
            swiftModule = swiftModule.substr(0, pos);
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
    while(true)
    {
        if(ModulePtr::dynamicCast(p))
        {
            m = ModulePtr::dynamicCast(p);
        }

        ContainerPtr c = p->container();
        p = ContainedPtr::dynamicCast(c); // This cast fails for Unit.
        if(!p)
        {
            break;
        }
    }
    return m;
}

ModulePtr
Slice::getTopLevelModule(const TypePtr& type)
{
    auto contained = ContainedPtr::dynamicCast(unwrapIfOptional(type));
    assert(contained);
    return getTopLevelModule(contained);
}

void
SwiftGenerator::trimLines(StringList& l)
{
    //
    // Remove empty trailing lines.
    //
    while(!l.empty() && l.back().empty())
    {
        l.pop_back();
    }
}

StringList
SwiftGenerator::splitComment(const string& c)
{
    string comment = c;

    //
    // Strip HTML markup and javadoc links -- MATLAB doesn't display them.
    //
    string::size_type pos = 0;
    do
    {
        pos = comment.find('<', pos);
        if(pos != string::npos)
        {
            string::size_type endpos = comment.find('>', pos);
            if(endpos == string::npos)
            {
                break;
            }
            comment.erase(pos, endpos - pos + 1);
        }
    }
    while(pos != string::npos);

    const string link = "{@link";
    pos = 0;
    do
    {
        pos = comment.find(link, pos);
        if(pos != string::npos)
        {
            comment.erase(pos, link.size() + 1); // Erase trailing white space too.
            string::size_type endpos = comment.find('}', pos);
            if(endpos != string::npos)
            {
                string ident = comment.substr(pos, endpos - pos);
                comment.erase(pos, endpos - pos + 1);

                //
                // Check for links of the form {@link Type#member}.
                //
                string::size_type hash = ident.find('#');
                string rest;
                if(hash != string::npos)
                {
                    rest = ident.substr(hash + 1);
                    ident = ident.substr(0, hash);
                    if(!ident.empty())
                    {
                        ident = fixIdent(ident);
                        if(!rest.empty())
                        {
                            ident += "." + fixIdent(rest);
                        }
                    }
                    else if(!rest.empty())
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
    }
    while(pos != string::npos);

    StringList result;

    pos = 0;
    string::size_type nextPos;
    while((nextPos = comment.find_first_of('\n', pos)) != string::npos)
    {
        result.push_back(IceUtilInternal::trim(string(comment, pos, nextPos - pos)));
        pos = nextPos + 1;
    }
    string lastLine = IceUtilInternal::trim(string(comment, pos));
    if(!lastLine.empty())
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

    if(l.find(tag) == 0)
    {
        const string ws = " \t";

        if(namedTag)
        {
            string::size_type n = l.find_first_not_of(ws, tag.size());
            if(n == string::npos)
            {
                return false; // Malformed line, ignore it.
            }
            string::size_type end = l.find_first_of(ws, n);
            if(end == string::npos)
            {
                return false; // Malformed line, ignore it.
            }
            name = l.substr(n, end - n);
            n = l.find_first_not_of(ws, end);
            if(n != string::npos)
            {
                doc = l.substr(n);
            }
        }
        else
        {
            name.clear();

            string::size_type n = l.find_first_not_of(ws, tag.size());
            if(n == string::npos)
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

    doc.deprecated = false;

    //
    // First check metadata for a deprecated tag.
    //
    string deprecateMetadata;
    if(p->findMetadata("deprecate", deprecateMetadata))
    {
        doc.deprecated = true;
        if(deprecateMetadata.find("deprecate:") == 0 && deprecateMetadata.size() > 10)
        {
            doc.deprecateReason.push_back(IceUtilInternal::trim(deprecateMetadata.substr(10)));
        }
    }

    //
    // Split up the comment into lines.
    //
    StringList lines = splitComment(p->comment());

    StringList::const_iterator i;
    for(i = lines.begin(); i != lines.end(); ++i)
    {
        const string l = *i;
        if(l[0] == '@')
        {
            break;
        }
        doc.overview.push_back(l);
    }

    enum State { StateMisc, StateParam, StateThrows, StateReturn, StateDeprecated };
    State state = StateMisc;
    string name;
    const string ws = " \t";
    const string paramTag = "@param";
    const string throwsTag = "@throws";
    const string exceptionTag = "@exception";
    const string returnTag = "@return";
    const string deprecatedTag = "@deprecated";
    const string seeTag = "@see";
    for(; i != lines.end(); ++i)
    {
        const string l = IceUtilInternal::trim(*i);
        string line;
        if(parseCommentLine(l, paramTag, true, name, line))
        {
            if(!line.empty())
            {
                state = StateParam;
                StringList sl;
                sl.push_back(line); // The first line of the description.
                doc.params[name] = sl;
            }
        }
        else if(parseCommentLine(l, throwsTag, true, name, line))
        {
            if(!line.empty())
            {
                state = StateThrows;
                StringList sl;
                sl.push_back(line); // The first line of the description.
                doc.exceptions[name] = sl;
            }
        }
        else if(parseCommentLine(l, exceptionTag, true, name, line))
        {
            if(!line.empty())
            {
                state = StateThrows;
                StringList sl;
                sl.push_back(line); // The first line of the description.
                doc.exceptions[name] = sl;
            }
        }
        else if(parseCommentLine(l, seeTag, false, name, line))
        {
            if(!line.empty())
            {
                doc.seeAlso.push_back(line);
            }
        }
        else if(parseCommentLine(l, returnTag, false, name, line))
        {
            if(!line.empty())
            {
                state = StateReturn;
                doc.returns.push_back(line); // The first line of the description.
            }
        }
        else if(parseCommentLine(l, deprecatedTag, false, name, line))
        {
            doc.deprecated = true;
            if(!line.empty())
            {
                state = StateDeprecated;
                doc.deprecateReason.push_back(line); // The first line of the description.
            }
        }
        else if(!l.empty())
        {
            if(l[0] == '@')
            {
                //
                // Treat all other tags as miscellaneous comments.
                //
                state = StateMisc;
            }

            switch(state)
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
                    if(doc.params.find(name) != doc.params.end())
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
                    if(doc.exceptions.find(name) != doc.exceptions.end())
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
SwiftGenerator::writeDocLines(IceUtilInternal::Output& out, const StringList& lines, bool commentFirst,
                              const string& space)
{
    StringList l = lines;
    if(!commentFirst)
    {
        out << l.front();
        l.pop_front();
    }

    for(StringList::const_iterator i = l.begin(); i != l.end(); ++i)
    {
        out << nl << "///";
        if(!i->empty())
        {
            out << space << *i;
        }
    }
}

void
SwiftGenerator::writeDocSentence(IceUtilInternal::Output& out, const StringList& lines)
{
    //
    // Write the first sentence.
    //
    for(StringList::const_iterator i = lines.begin(); i != lines.end(); ++i)
    {
        const string ws = " \t";

        if(i->empty())
        {
            break;
        }
        if(i != lines.begin() && i->find_first_not_of(ws) == 0)
        {
            out << " ";
        }
        string::size_type pos = i->find('.');
        if(pos == string::npos)
        {
            out << *i;
        }
        else if(pos == i->size() - 1)
        {
            out << *i;
            break;
        }
        else
        {
            //
            // Assume a period followed by whitespace indicates the end of the sentence.
            //
            while(pos != string::npos)
            {
                if(ws.find((*i)[pos + 1]) != string::npos)
                {
                    break;
                }
                pos = i->find('.', pos + 1);
            }
            if(pos != string::npos)
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
SwiftGenerator::writeDocSummary(IceUtilInternal::Output& out, const ContainedPtr& p)
{
    DocElements doc = parseComment(p);

    string n = fixIdent(p->name());

    //
    // No leading newline.
    //
    if(!doc.overview.empty())
    {
        writeDocLines(out, doc.overview);
    }

    if(!doc.misc.empty())
    {
        out << "///" << nl;
        writeDocLines(out, doc.misc);
    }

    if(doc.deprecated)
    {
        out << nl << "///";
        out << nl << "/// ## Deprecated";
        if(!doc.deprecateReason.empty())
        {
            writeDocLines(out, doc.deprecateReason);
        }
    }
}

void
SwiftGenerator::writeOpDocSummary(IceUtilInternal::Output& out,
                                  const OperationPtr& p,
                                  bool async,
                                  bool dispatch)
{
    DocElements doc = parseComment(p);
    if(!doc.overview.empty())
    {
        writeDocLines(out, doc.overview);
    }

    if(doc.deprecated)
    {
        out << nl << "///";
        out << nl << "///  ## Deprecated";
        if(!doc.deprecateReason.empty())
        {
            writeDocLines(out, doc.deprecateReason);
        }
    }

    const ParamInfoList allInParams = getAllInParams(p);
    for(ParamInfoList::const_iterator q = allInParams.begin(); q != allInParams.end(); ++q)
    {
        out << nl << "///";
        out << nl << "/// - parameter " << (!dispatch && allInParams.size() == 1 ? "_" : q->name)
            << ": `" << q->typeStr << "`";
        map<string, StringList>::const_iterator r = doc.params.find(q->name);
        if(r != doc.params.end() && !r->second.empty())
        {
            out << " ";
            writeDocLines(out, r->second, false);
        }
    }

    out << nl << "///";
    if(dispatch)
    {
        out << nl << "/// - parameter current: `Ice.Current` - The Current object for the dispatch.";
    }
    else
    {
        out << nl << "/// - parameter context: `Ice.Context` - Optional request context.";
    }

    if(async)
    {
        if(!dispatch)
        {
            out << nl << "///";
            out << nl << "/// - parameter sentOn: `Dispatch.DispatchQueue?` - Optional dispatch queue used to";
            out << nl << "///   dispatch the sent callback.";
            out << nl << "///";
            out << nl << "/// - parameter sentFlags: `Dispatch.DispatchWorkItemFlags?` - Optional dispatch flags used";
            out << nl << "///   to dispatch the sent callback";
            out << nl << "///";
            out << nl << "/// - parameter sent: `((Swift.Bool) -> Swift.Void)` - Optional sent callback.";
        }

        out << nl << "///";
        out << nl << "/// - returns: `PromiseKit.Promise<" << operationReturnType(p)
            << ">` - The result of the operation";
    }
    else
    {
        const ParamInfoList allOutParams = getAllOutParams(p);
        if(allOutParams.size() == 1)
        {
            ParamInfo ret = allOutParams.front();
            out << nl << "///";
            out << nl << "/// - returns: `" << ret.typeStr << "`";
            if(p->deprecatedReturnType())
            {
                if(!doc.returns.empty())
                {
                    out << " - ";
                    writeDocLines(out, doc.returns, false);
                }
            }
            else
            {
                map<string, StringList>::const_iterator r = doc.params.find(ret.name);
                if(r != doc.params.end() && !r->second.empty())
                {
                    out << " - ";
                    writeDocLines(out, r->second, false);
                }
            }
        }
        else if(allOutParams.size() > 1)
        {
            out << nl << "///";
            out << nl << "/// - returns: `" << operationReturnType(p) << "`:";
            if(p->deprecatedReturnType())
            {
                ParamInfo ret = allOutParams.back();
                out << nl << "///";
                out << nl << "///   - " << ret.name << ": `" << ret.typeStr << "`";
                if(!doc.returns.empty())
                {
                    out << " - ";
                    writeDocLines(out, doc.returns, false);
                }
            }

            for(ParamInfoList::const_iterator q = allOutParams.begin(); q != allOutParams.end(); ++q)
            {
                if(q->param != 0)
                {
                    out << nl << "///";
                    out << nl << "///   - " << q->name << ": `" << q->typeStr << "`";
                    map<string, StringList>::const_iterator r = doc.params.find(q->name);
                    if(r != doc.params.end() && !r->second.empty())
                    {
                        out << " - ";
                        writeDocLines(out, r->second, false);
                    }
                }
            }
        }
    }

    if(!doc.exceptions.empty() && !async)
    {
        out << nl << "///";
        out << nl << "/// - throws:";
        for(map<string, StringList>::const_iterator q = doc.exceptions.begin(); q != doc.exceptions.end(); ++q)
        {
            out << nl << "///";
            out << nl << "///   - "  << q->first;
            if(!q->second.empty())
            {
                out << " - ";
                writeDocLines(out, q->second, false, "     ");
            }
        }
    }

    if(!doc.misc.empty())
    {
        out << nl;
        writeDocLines(out, doc.misc, false);
    }
}

void
SwiftGenerator::writeProxyDocSummary(IceUtilInternal::Output& out, const InterfaceDefPtr& p, const string& swiftModule)
{
    DocElements doc = parseComment(p);

    const string name = getUnqualified(getAbsolute(p), swiftModule);
    const string prx = name + "Prx";

    if(doc.overview.empty())
    {
        out << nl << "/// " << prx << " overview.";
    }
    else
    {
        writeDocLines(out, doc.overview);
    }

    const OperationList ops = p->operations();
    if(!ops.empty())
    {
        out << nl << "///";
        out << nl << "/// " << prx << " Methods:";
        for(OperationList::const_iterator q = ops.begin(); q != ops.end(); ++q)
        {
            OperationPtr op = *q;
            DocElements opdoc = parseComment(op);
            out << nl << "///";
            out << nl << "///  - " << fixIdent(op->name()) << ": ";
            if(!opdoc.overview.empty())
            {
                writeDocSentence(out, opdoc.overview);
            }

            out << nl << "///";
            out << nl << "///  - " << op->name() << "Async: ";
            if(!opdoc.overview.empty())
            {
                writeDocSentence(out, opdoc.overview);
            }
        }
    }

    if(!doc.misc.empty())
    {
        writeDocLines(out, doc.misc, false);
    }
}

void
SwiftGenerator::writeServantDocSummary(IceUtilInternal::Output& out, const InterfaceDefPtr& p, const string& swiftModule)
{
    DocElements doc = parseComment(p);

    const string name = getUnqualified(getAbsolute(p), swiftModule);

    if(doc.overview.empty())
    {
        out << nl << "/// " << name << " overview.";
    }
    else
    {
        writeDocLines(out, doc.overview);
    }

    const OperationList ops = p->operations();
    if(!ops.empty())
    {
        out << nl << "///";
        out << nl << "/// " << name << " Methods:";
        for(OperationList::const_iterator q = ops.begin(); q != ops.end(); ++q)
        {
            OperationPtr op = *q;
            DocElements opdoc = parseComment(op);
            out << nl << "///";
            out << nl << "///  - " << fixIdent(op->name()) << ": ";
            if(!opdoc.overview.empty())
            {
                writeDocSentence(out, opdoc.overview);
            }
        }
    }

    if(!doc.misc.empty())
    {
        writeDocLines(out, doc.misc, false);
    }
}

void
SwiftGenerator::writeMemberDoc(IceUtilInternal::Output& out, const MemberPtr& p)
{
    DocElements doc = parseComment(p);

    //
    // Skip if there are no doc comments.
    //
    if(doc.overview.empty() && doc.misc.empty() && doc.seeAlso.empty() && doc.deprecateReason.empty() &&
       !doc.deprecated)
    {
        return;
    }

    if(doc.overview.empty())
    {
        out << nl << "/// " << fixIdent(p->name());
    }
    else
    {
        writeDocLines(out, doc.overview);
    }

    if(!doc.misc.empty())
    {
        writeDocLines(out, doc.misc);
    }

    if(doc.deprecated)
    {
        out << nl << "/// ##Deprecated";
        if(!doc.deprecateReason.empty())
        {
            writeDocLines(out, doc.deprecateReason);
        }
    }
}

void
SwiftGenerator::validateMetadata(const UnitPtr& u)
{
    MetadataVisitor visitor;
    u->visit(&visitor, true);
}

//
// Get the fully-qualified name of the given definition. If a suffix is provided,
// it is prepended to the definition's unqualified name. If the nameSuffix
// is provided, it is appended to the container's name.
//
namespace
{

string
getAbsoluteImpl(const ContainedPtr& cont, const string& prefix = "", const string& suffix = "")
{
    string swiftPrefix;
    string swiftModule = getSwiftModule(getTopLevelModule(cont), swiftPrefix);

    string str = cont->scope() + prefix + cont->name() + suffix;
    if(str.find("::") == 0)
    {
        str.erase(0, 2);
    }

    size_t pos = str.find("::");
    //
    // Replace the definition top-level module by the corresponding Swift module
    // and append the Swift prefix for the Slice module, then any remaining nested
    // modules become a Swift prefix
    //
    if(pos != string::npos)
    {
        str = str.substr(pos + 2);
    }
    return swiftModule + "." + swiftPrefix + replace(str, "::", "");
}

}

string
SwiftGenerator::getValue(const string& swiftModule, const TypePtr& type)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindBool:
            {
                return "false";
            }
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindUShort:
            case Builtin::KindInt:
            case Builtin::KindUInt:
            case Builtin::KindVarInt:
            case Builtin::KindVarUInt:
            case Builtin::KindLong:
            case Builtin::KindULong:
            case Builtin::KindVarLong:
            case Builtin::KindVarULong:
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

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        return "." + fixIdent((*en->enumerators().begin())->name());
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        return getUnqualified(getAbsolute(type), swiftModule) + "()";
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        return getUnqualified(getAbsolute(type), swiftModule) + "()";
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        return getUnqualified(getAbsolute(type), swiftModule) + "()";
    }

    return "nil";
}

void
SwiftGenerator::writeConstantValue(IceUtilInternal::Output& out, const TypePtr& constType,
                                   const SyntaxTreeBasePtr& valueType, const string& value,
                                   const StringList&, const string& swiftModule, bool optional)
{
    TypePtr type = unwrapIfOptional(constType);

    ConstPtr constant = ConstPtr::dynamicCast(valueType);
    if(constant)
    {
        out << getUnqualified(getAbsolute(constant), swiftModule);
    }
    else
    {
        if(valueType)
        {
            BuiltinPtr bp = BuiltinPtr::dynamicCast(type);
            EnumPtr ep = EnumPtr::dynamicCast(type);
            if(bp && bp->kind() == Builtin::KindString)
            {
                out << "\"";
                out << toStringLiteral(value, "\n\r\t", "", EC6UCN, 0);
                out << "\"";
            }
            else if(ep)
            {
                assert(valueType);
                EnumeratorPtr enumerator = EnumeratorPtr::dynamicCast(valueType);
                assert(enumerator);
                out << getUnqualified(getAbsolute(ep), swiftModule) << "." << enumerator->name();
            }
            else
            {
                out << value;
            }
        }
        else if(optional)
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
SwiftGenerator::typeToString(const TypePtr& type, const ContainedPtr& toplevel, const StringList& metadata)
{
    if(!type)
    {
        return "";
    }

    if (auto optional = OptionalPtr::dynamicCast(type))
    {
        auto underlying = optional->underlying();
        // For interface type, we currently always generate "?", so we don't want a double ?.
        return typeToString(underlying, toplevel, metadata) + (underlying->isInterfaceType() ? "" : "?");
    }

    //
    // The current module where the type is being used
    //
    string currentModule = getSwiftModule(getTopLevelModule(toplevel));
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);

    if(builtin)
    {
        if (builtin->isInterfaceType())
        {
            return getUnqualified(builtinTable[builtin->kind()], currentModule) + "?";
        }
        else
        {
            return getUnqualified(builtinTable[builtin->kind()], currentModule);
        }
    }

    InterfaceDeclPtr proxy = InterfaceDeclPtr::dynamicCast(type);
    ContainedPtr cont = ContainedPtr::dynamicCast(type);

    if (proxy)
    {
        return getUnqualified(getAbsoluteImpl(proxy, "", "Prx?"), currentModule);
    }

    assert(cont);
    return fixIdent(getUnqualified(getAbsoluteImpl(cont), currentModule));
}

string
SwiftGenerator::getAbsolute(const TypePtr& type)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        return builtinTable[builtin->kind()];
    }

    InterfaceDeclPtr proxy = InterfaceDeclPtr::dynamicCast(type);
    if(proxy)
    {
        return getAbsoluteImpl(proxy, "", "Prx");
    }

    ContainedPtr cont = ContainedPtr::dynamicCast(type);
    assert(cont);
    return getAbsoluteImpl(cont);
}

string
SwiftGenerator::getAbsolute(const ClassDeclPtr& cl)
{
    return getAbsoluteImpl(cl);
}

string
SwiftGenerator::getAbsolute(const ClassDefPtr& cl)
{
    return getAbsoluteImpl(cl);
}

string
SwiftGenerator::getAbsolute(const InterfaceDeclPtr& proxy)
{
    return getAbsoluteImpl(proxy, "", "Prx");
}

string
SwiftGenerator::getAbsolute(const InterfaceDefPtr& interface)
{
    return getAbsoluteImpl(interface);
}

string
SwiftGenerator::getAbsolute(const StructPtr& st)
{
    return getAbsoluteImpl(st);
}

string
SwiftGenerator::getAbsolute(const ExceptionPtr& ex)
{
    return getAbsoluteImpl(ex);
}

string
SwiftGenerator::getAbsolute(const EnumPtr& en)
{
    return getAbsoluteImpl(en);
}

string
SwiftGenerator::getAbsolute(const ConstPtr& en)
{
    return getAbsoluteImpl(en);
}

string
SwiftGenerator::getAbsolute(const SequencePtr& en)
{
    return getAbsoluteImpl(en);
}

string
SwiftGenerator::getAbsolute(const DictionaryPtr& en)
{
    return getAbsoluteImpl(en);
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
    switch(opMode)
    {
        case Operation::Normal:
        {
            mode = ".Normal";
            break;
        }
        case Operation::Nonmutating:
        {
            mode = ".Nonmutating";
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
SwiftGenerator::getTagFormat(const TypePtr& type)
{
    return "." + type->getTagFormat();
}

void
SwiftGenerator::writeDefaultInitializer(IceUtilInternal::Output& out,
                                        bool required,
                                        bool rootClass)
{
     out << sp;
     out << nl << "public ";
     if(required)
     {
         out << "required ";
     }
     if(rootClass)
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
SwiftGenerator::writeMemberwiseInitializer(IceUtilInternal::Output& out,
                                           const MemberList& members,
                                           const ContainedPtr& p)
{
    writeMemberwiseInitializer(out, members, MemberList(), members, p, true);
}

void
SwiftGenerator::writeMemberwiseInitializer(IceUtilInternal::Output& out,
                                           const MemberList& members,
                                           const MemberList& baseMembers,
                                           const MemberList& allMembers,
                                           const ContainedPtr& p,
                                           bool rootClass)
{
    if(!members.empty())
    {
        out << sp;
        out << nl;
        out << "public init" << spar;
        for (const auto& member : allMembers)
        {
            out << (fixIdent(member->name()) + ": " + typeToString(member->type(), p, member->getAllMetadata()));
        }

        out << epar;
        out << sb;
        for (const auto& member : members)
        {
            out << nl << "self." << fixIdent(member->name()) << " = " << fixIdent(member->name());
        }

        if(!rootClass)
        {
            out << nl << "super.init";
            out << spar;
            for (const auto& member : baseMembers)
            {
                const string name = fixIdent(member->name());
                out << (name + ": " + name);
            }
            out << epar;
        }
        out << eb;
    }
}

void
SwiftGenerator::writeMembers(IceUtilInternal::Output& out,
                             const MemberList& members,
                             const ContainedPtr& p)
{
    string swiftModule = getSwiftModule(getTopLevelModule(p));
    for (const auto& member : members)
    {
        TypePtr type = member->type();
        const string defaultValue = member->defaultValue();

        const string memberName = fixIdent(member->name());
        string memberType = typeToString(type, p, member->getAllMetadata());

        //
        // If the member type is equal to the member name, create a local type alias
        // to avoid ambiguity.
        //
        string alias;
        if(memberName == memberType && (StructPtr::dynamicCast(type) ||
                                        SequencePtr::dynamicCast(type) ||
                                        DictionaryPtr::dynamicCast(type)))
        {
            ModulePtr m = getTopLevelModule(type);
            alias =  m->name() + "_" + memberType;
            out << nl << "typealias " << alias << " = " << memberType;
        }

        writeMemberDoc(out, member);
        out << nl << "public var " << memberName << ": " << memberType;
        out << " = ";
        if(alias.empty())
        {
            writeConstantValue(out, type, member->defaultValueType(), defaultValue, p->getAllMetadata(), swiftModule,
                               member->tagged());
        }
        else
        {
            out << alias << "()";
        }
    }
}

bool
SwiftGenerator::usesMarshalHelper(const TypePtr& type)
{
    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
        if(builtin)
        {
            return builtin->kind() > Builtin::KindString;
        }
        return true;
    }
    return DictionaryPtr::dynamicCast(type);
}

void
SwiftGenerator::writeMarshalUnmarshalCode(Output& out,
                                          const TypePtr& constType,
                                          const ContainedPtr& p,
                                          const string& param,
                                          bool marshal,
                                          int tag)
{
    // TODO: rework this code
    TypePtr type = unwrapIfOptional(constType);

    string swiftModule = getSwiftModule(getTopLevelModule(p));
    string stream = StructPtr::dynamicCast(p) ? "self" : marshal ? "ostr" : "istr";

    string args;
    if(tag >= 0)
    {
        args += "tag: " + int64ToString(tag);
        if(marshal)
        {
            args += ", ";
        }
    }

    if(marshal)
    {
        if(tag >= 0 || usesMarshalHelper(type))
        {
            args += "value: ";
        }
        args += param;
    }

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindObject:
            {
                if(marshal)
                {
                    out << nl << stream << ".write(" << args << ")";
                }
                else
                {
                    if(tag >= 0)
                    {
                        args += ", type: ";
                    }
                    args += getUnqualified(getAbsolute(type), swiftModule) + ".self";

                    out << nl << param << " = try " << stream << ".read(" << args << ")";
                }
                break;
            }
            case Builtin::KindAnyClass:
            {
                if(marshal)
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
                if(marshal)
                {
                    out << nl << stream << ".write(" << args << ")";
                }
                else
                {
                    out << nl << param << " = try " << stream << ".read(" << args << ")";
                }
                break;
            }
        }
    }

    if(ClassDeclPtr::dynamicCast(type))
    {
        ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
        if(marshal)
        {
            out << nl << stream << ".write(" << args << ")";
        }
        else
        {
            if (tag >= 0)
            {
                args += ", value: ";
            }
            string memberType = getUnqualified(getAbsolute(type), swiftModule);
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
            args += (alias.empty() ? memberType : alias) + ".self";
            out << nl << "try " << stream << ".read(" << args << ") { " << param << " = $0 "
                << "}";
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        if(marshal)
        {
            out << nl << stream << ".write(" << args << ")";
        }
        else
        {
            out << nl << param << " = try " << stream << ".read(" << args << ")";
        }
        return;
    }

    if(InterfaceDeclPtr::dynamicCast(type))
    {
        if(marshal)
        {
            out << nl << stream << ".write(" << args << ")";
        }
        else
        {
            if(tag >= 0)
            {
                args += ", type: ";
            }

            args += getUnqualified(getAbsolute(type), swiftModule) + ".self";
            out << nl << param << " = try " << stream << ".read(" << args << ")";
        }
        return;
    }

    if(StructPtr::dynamicCast(type))
    {
        if(marshal)
        {
            out << nl << stream << ".write(" << args << ")";
        }
        else
        {
            out << nl << param << " = try " << stream << ".read(" << args << ")";
        }
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        BuiltinPtr seqBuiltin = BuiltinPtr::dynamicCast(seq->type());
        if(seqBuiltin && seqBuiltin->kind() <= Builtin::KindString)
        {
            if(marshal)
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
            string helper = getUnqualified(getAbsoluteImpl(ContainedPtr::dynamicCast(type)), swiftModule) + "Helper";
            if(marshal)
            {
                out << nl << helper <<".write(to: " << stream << ", " << args << ")";
            }
            else
            {
                out << nl << param << " = try " << helper << ".read(from: " << stream;
                if(!args.empty())
                {
                    out << ", " << args;
                }
                out << ")";
            }
        }
        return;
    }

    if(DictionaryPtr::dynamicCast(type))
    {
        string helper = getUnqualified(getAbsoluteImpl(ContainedPtr::dynamicCast(type)), swiftModule) + "Helper";
        if(marshal)
        {
            out << nl << helper <<".write(to: " << stream << ", " << args << ")";
        }
        else
        {
            out << nl << param << " = try " << helper << ".read(from: " << stream;
            if(!args.empty())
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
    if(UnitPtr::dynamicCast(p->container()))
    {
        // top-level module
        const UnitPtr ut = p->unit();
        const DefinitionContextPtr dc = ut->findDefinitionContext(p->file());
        assert(dc);

        const string modulePrefix = "swift:module:";

        string swiftModule;
        string swiftPrefix;

        if(p->findMetadata(modulePrefix, swiftModule))
        {
            swiftModule = swiftModule.substr(modulePrefix.size());

            size_t pos = swiftModule.find(':');
            if(pos != string::npos)
            {
                swiftPrefix = swiftModule.substr(pos + 1);
                swiftModule = swiftModule.substr(0, pos);
            }
        }
        else
        {
            swiftModule = p->name();
        }

        const string filename = p->definitionContext()->filename();
        ModuleMap::const_iterator current = _modules.find(filename);

        if(current == _modules.end())
        {
            _modules[filename] = swiftModule;
        }
        else if(current->second != swiftModule)
        {
            ostringstream os;
            os << "invalid module mapping:\n Slice module `" << p->scoped() << "' should be map to Swift module `"
               << current->second << "'" << endl;
            dc->error(p->file(), p->line(), os.str());
        }

        ModulePrefix::iterator prefixes = _prefixes.find(swiftModule);
        if(prefixes == _prefixes.end())
        {
            ModuleMap mappings;
            mappings[p->name()] = swiftPrefix;
            _prefixes[swiftModule] = mappings;
        }
        else
        {
            current = prefixes->second.find(p->name());
            if(current == prefixes->second.end())
            {
                prefixes->second[p->name()] = swiftPrefix;
            }
            else if(current->second != swiftPrefix)
            {
                ostringstream os;
                os << "invalid module prefix:\n Slice module `" << p->scoped() << "' is already using";
                if(current->second.empty())
                {
                    os << " no prefix " << endl;
                }
                else
                {
                   os << " a different Swift module prefix `" << current->second << "'" << endl;
                }
                dc->error(p->file(), p->line(), os.str());
            }
        }
    }
    p->setMetadata(validate(p, p->getAllMetadata(), p->file(), p->line()));
    return true;
}

string
SwiftGenerator::paramLabel(const string& name, const MemberList& params)
{
    for (const auto& param : params)
    {
        if(param->name() == name)
        {
            return "_" + name;
        }
    }
    return name;
}

bool
SwiftGenerator::operationReturnIsTuple(const OperationPtr& op)
{
    MemberList outParams = op->outParameters();
    return (op->deprecatedReturnType() && outParams.size() > 0) || outParams.size() > 1;
}

string
SwiftGenerator::operationReturnType(const OperationPtr& op)
{
    ostringstream os;
    bool returnIsTuple = operationReturnIsTuple(op);
    if(returnIsTuple)
    {
        os << "(";
    }

    MemberList outParams = op->outParameters();
    TypePtr returnType = op->deprecatedReturnType();
    if(returnType)
    {
        if(returnIsTuple)
        {
            os << paramLabel("returnValue", outParams) << ": ";
        }
        os << typeToString(returnType, op, op->getAllMetadata());
    }

    for (auto q = outParams.begin(); q != outParams.end(); ++q)
    {
        if(returnType || q != outParams.begin())
        {
            os << ", ";
        }

        if(returnIsTuple)
        {
            os << (*q)->name() << ": ";
        }

        os << typeToString((*q)->type(), *q, (*q)->getAllMetadata());
    }

    if(returnIsTuple)
    {
        os << ")";
    }

    return os.str();
}

std::string
SwiftGenerator::operationReturnDeclaration(const OperationPtr& op)
{
    ostringstream os;
    MemberList outParams = op->outParameters();
    TypePtr returnType = op->deprecatedReturnType();
    bool returnIsTuple = operationReturnIsTuple(op);

    if(returnIsTuple)
    {
        os << "(";
    }

    if(returnType)
    {
        os << ("iceP_" + paramLabel("returnValue", outParams));
    }

    for (auto q = outParams.begin(); q != outParams.end(); ++q)
    {
        if(returnType || q != outParams.begin())
        {
            os << ", ";
        }

        os << ("iceP_" + (*q)->name());
    }

    if(returnIsTuple)
    {
        os << ")";
    }

    return os.str();
}

string
SwiftGenerator::operationInParamsDeclaration(const OperationPtr& op)
{
    ostringstream os;

    MemberList inParams = op->params();
    const bool isTuple = inParams.size() > 1;

    if(!inParams.empty())
    {
        if(isTuple)
        {
            os << "(";
        }
        for (auto q = inParams.begin(); q != inParams.end(); ++q)
        {
            if(q != inParams.begin())
            {
                os << ", ";
            }

            os << ("iceP_" + (*q)->name());
        }
        if(isTuple)
        {
            os << ")";
        }

        os << ": ";

        if(isTuple)
        {
            os << "(";
        }
        for (auto q = inParams.begin(); q != inParams.end(); ++q)
        {
            if(q != inParams.begin())
            {
                os << ", ";
            }

            os << typeToString((*q)->type(), *q, (*q)->getAllMetadata());
        }
        if(isTuple)
        {
            os << ")";
        }
    }

    return os.str();
}

bool
SwiftGenerator::operationIsAmd(const OperationPtr& op)
{
    const InterfaceDefPtr def = op->interface();
    return def->hasMetadata("amd") || op->hasMetadata("amd");
}

ParamInfoList
SwiftGenerator::getAllInParams(const OperationPtr& op)
{
    ParamInfoList r;
    for (const auto& param : op->params())
    {
        ParamInfo info;
        info.name = param->name();
        info.type = param->type();
        info.typeStr = typeToString(info.type, op, param->getAllMetadata());
        info.isTagged = param->tagged();
        info.tag = param->tag();
        info.param = param;
        r.push_back(info);
    }
    return r;
}

void
SwiftGenerator::getInParams(const OperationPtr& op, ParamInfoList& requiredParams, ParamInfoList& taggedParams)
{
    const ParamInfoList params = getAllInParams(op);
    for(ParamInfoList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        if(p->isTagged)
        {
            taggedParams.push_back(*p);
        }
        else
        {
            requiredParams.push_back(*p);
        }
    }

    //
    // Sort tagged parameters by tag.
    //
    class SortFn
    {
    public:
        static bool compare(const ParamInfo& lhs, const ParamInfo& rhs)
        {
            return lhs.tag < rhs.tag;
        }
    };
    taggedParams.sort(SortFn::compare);
}

ParamInfoList
SwiftGenerator::getAllOutParams(const OperationPtr& op)
{
    MemberList params = op->outParameters();
    ParamInfoList l;

    for (const auto& param : params)
    {
        ParamInfo info;
        info.name = param->name();
        info.type = param->type();
        info.typeStr = typeToString(info.type, op, param->getAllMetadata());
        info.isTagged = param->tagged();
        info.tag = param->tag();
        info.param = param;
        l.push_back(info);
    }

    if(op->deprecatedReturnType())
    {
        ParamInfo info;
        info.name = paramLabel("returnValue", params);
        info.type = op->deprecatedReturnType();
        info.typeStr = typeToString(info.type, op, op->getAllMetadata());
        info.isTagged = op->returnIsTagged();
        info.tag = op->returnTag();
        l.push_back(info);
    }

    return l;
}

void
SwiftGenerator::getOutParams(const OperationPtr& op, ParamInfoList& requiredParams, ParamInfoList& taggedParams)
{
    const ParamInfoList params = getAllOutParams(op);
    for(ParamInfoList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        if(p->isTagged)
        {
            taggedParams.push_back(*p);
        }
        else
        {
            requiredParams.push_back(*p);
        }
    }

    //
    // Sort tagged parameters by tag.
    //
    class SortFn
    {
    public:
        static bool compare(const ParamInfo& lhs, const ParamInfo& rhs)
        {
            return lhs.tag < rhs.tag;
        }
    };
    taggedParams.sort(SortFn::compare);
}

void
SwiftGenerator::writeMarshalInParams(::IceUtilInternal::Output& out, const OperationPtr& op)
{
    ParamInfoList requiredInParams, taggedInParams;
    getInParams(op, requiredInParams, taggedInParams);

    out << "{ ostr in";
    out.inc();
    //
    // Marshal parameters
    // 1. required
    // 2. tagged
    //

    for(ParamInfoList::const_iterator q = requiredInParams.begin(); q != requiredInParams.end(); ++q)
    {
        writeMarshalUnmarshalCode(out, q->type, op, "iceP_" + q->name, true);
    }

    for(ParamInfoList::const_iterator q = taggedInParams.begin(); q != taggedInParams.end(); ++q)
    {
        writeMarshalUnmarshalCode(out, q->type, op, "iceP_" + q->name, true, q->tag);
    }

    if(op->sendsClasses(false))
    {
        out << nl << "ostr.writePendingValues()";
    }
    out.dec();
    out << nl << "}";
}

void
SwiftGenerator::writeMarshalOutParams(::IceUtilInternal::Output& out, const OperationPtr& op)
{
    ParamInfoList requiredOutParams, taggedOutParams;
    getOutParams(op, requiredOutParams, taggedOutParams);

    out << "{ ostr in";
    out.inc();
    //
    // Marshal parameters
    // 1. required
    // 2. tagged (including tagged return)
    //

    for(ParamInfoList::const_iterator q = requiredOutParams.begin(); q != requiredOutParams.end(); ++q)
    {
        writeMarshalUnmarshalCode(out, q->type, op, "iceP_" + q->name, true);
    }

    for(ParamInfoList::const_iterator q = taggedOutParams.begin(); q != taggedOutParams.end(); ++q)
    {
        writeMarshalUnmarshalCode(out, q->type, op, "iceP_" + q->name, true, q->tag);
    }

    if(op->returnsClasses(false))
    {
        out << nl << "ostr.writePendingValues()";
    }

    out.dec();
    out << nl << "}";
}

void
SwiftGenerator::writeMarshalAsyncOutParams(::IceUtilInternal::Output& out, const OperationPtr& op)
{
    ParamInfoList requiredOutParams, taggedOutParams;
    getOutParams(op, requiredOutParams, taggedOutParams);

    out << sb << " (ostr, retVals) in";
    out << nl << "let " << operationReturnDeclaration(op) << " = retVals";
    //
    // Marshal parameters
    // 1. required
    // 2. tagged (including tagged return)
    //

    for(ParamInfoList::const_iterator q = requiredOutParams.begin(); q != requiredOutParams.end(); ++q)
    {
        writeMarshalUnmarshalCode(out, q->type, op, "iceP_" + q->name, true);
    }

    for(ParamInfoList::const_iterator q = taggedOutParams.begin(); q != taggedOutParams.end(); ++q)
    {
        writeMarshalUnmarshalCode(out, q->type, op, "iceP_" + q->name, true, q->tag);
    }

    if(op->returnsClasses(false))
    {
        out << nl << "ostr.writePendingValues()";
    }

    out << eb;
}

void
SwiftGenerator::writeUnmarshalOutParams(::IceUtilInternal::Output& out, const OperationPtr& op)
{
    TypePtr returnType = op->deprecatedReturnType();

    ParamInfoList requiredOutParams, taggedOutParams;
    getOutParams(op, requiredOutParams, taggedOutParams);
    const ParamInfoList allOutParams = getAllOutParams(op);
    //
    // Unmarshal parameters
    // 1. required
    // 2. return
    // 3. tagged (including tagged return)
    //
    out << "{ istr in";
    out.inc();
    for(ParamInfoList::const_iterator q = requiredOutParams.begin(); q != requiredOutParams.end(); ++q)
    {
        string param;
        if(q->type->isClassType())
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

    for(ParamInfoList::const_iterator q = taggedOutParams.begin(); q != taggedOutParams.end(); ++q)
    {
        string param;
        if(q->type->isClassType())
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

    if(op->returnsClasses(false))
    {
        out << nl << "try istr.readPendingValues()";
    }

    out << nl << "return ";
    if(allOutParams.size() > 1)
    {
        out << spar;
    }

    if(returnType)
    {
        out << ("iceP_" + paramLabel("returnValue", op->outParameters()));
    }

    for(ParamInfoList::const_iterator q = allOutParams.begin(); q != allOutParams.end(); ++q)
    {
        if(q->param)
        {
            out << ("iceP_" + q->name);
        }
    }

    if(allOutParams.size() > 1)
    {
        out << epar;
    }

    out.dec();
    out << nl << "}";
}

void
SwiftGenerator::writeUnmarshalInParams(::IceUtilInternal::Output& out, const OperationPtr& op)
{
    ParamInfoList requiredInParams, taggedInParams;
    getInParams(op, requiredInParams, taggedInParams);
    const ParamInfoList allInParams = getAllInParams(op);
    //
    // Unmarshal parameters
    // 1. required
    // 3. tagged
    //
    out << "{ istr in";
    out.inc();
    for(ParamInfoList::const_iterator q = requiredInParams.begin(); q != requiredInParams.end(); ++q)
    {
        if(q->param)
        {
            string param;
            if(q->type->isClassType())
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

    for(ParamInfoList::const_iterator q = taggedInParams.begin(); q != taggedInParams.end(); ++q)
    {
        string param;
        if(q->type->isClassType())
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

    if(op->sendsClasses(false))
    {
        out << nl << "try istr.readPendingValues()";
    }

    out << nl << "return ";
    if(allInParams.size() > 1)
    {
        out << spar;
    }

    for(ParamInfoList::const_iterator q = allInParams.begin(); q != allInParams.end(); ++q)
    {
        out << ("iceP_" + q->name);
    }

    if(allInParams.size() > 1)
    {
        out << epar;
    }

    out.dec();
    out << nl << "}";
}

void
SwiftGenerator::writeUnmarshalUserException(::IceUtilInternal::Output& out, const OperationPtr& op)
{
    const string swiftModule = getSwiftModule(getTopLevelModule(ContainedPtr::dynamicCast(op)));
    ExceptionList throws = op->throws();
    throws.sort();
    throws.unique();

    out << "{ ex in";
    out.inc();
    out << nl << "do ";
    out << sb;
    out << nl << "throw ex";
    out << eb;
    for(ExceptionList::const_iterator q = throws.begin(); q != throws.end(); ++q)
    {
        out << " catch let error as " << getUnqualified(getAbsolute(*q), swiftModule) << sb;
        out << nl << "throw error";
        out << eb;
    }
    out << " catch is " << getUnqualified("Ice.UserException", swiftModule) << " {}";
    out.dec();
    out << nl << "}";
}

void
SwiftGenerator::writeSwiftAttributes(::IceUtilInternal::Output& out, const StringList& metadata)
{
    static const string prefix = "swift:attribute:";
    for(StringList::const_iterator q = metadata.begin(); q != metadata.end(); ++q)
    {
        if(q->find(prefix) == 0 && q->size() > prefix.size())
        {
            out << nl << q->substr(prefix.size());
        }
    }
}

void
SwiftGenerator::writeProxyOperation(::IceUtilInternal::Output& out, const OperationPtr& op)
{
    const string opName = fixIdent(op->name());

    const ParamInfoList allInParams = getAllInParams(op);
    const ParamInfoList allOutParams = getAllOutParams(op);
    const ExceptionList allExceptions = op->throws();

    const string swiftModule = getSwiftModule(getTopLevelModule(ContainedPtr::dynamicCast(op)));

    out << sp;
    writeOpDocSummary(out, op, false, false);
    out << nl << "func " << opName;
    out << spar;
    for(ParamInfoList::const_iterator q = allInParams.begin(); q != allInParams.end(); ++q)
    {
        if(allInParams.size() == 1)
        {
            out << ("_ iceP_" + q->name + ": " + q->typeStr + (q->isTagged ? " = nil" : ""));
        }
        else
        {
            out << (q->name + " iceP_" + q->name + ": " + q->typeStr + (q->isTagged ? " = nil" : ""));
        }
    }
    out << ("context: " + getUnqualified("Ice.Context", swiftModule) + "? = nil");

    out << epar;
    out << " throws";

    if(allOutParams.size() > 0)
    {
        out << " -> " << operationReturnType(op);
    }

    out << sb;

    //
    // Invoke
    //
    out << sp;
    out << nl;
    if(allOutParams.size() > 0)
    {
        out << "return ";
    }
    out << "try _impl._invoke(";

    out.useCurrentPosAsIndent();
    out << "operation: \"" << op->name() << "\",";
    out << nl << "mode: " << modeToString(op->sendMode()) << ",";

    if(op->format() != DefaultFormat)
    {
        out << nl << "format: " << opFormatTypeToString(op);
        out << ",";
    }

    if(allInParams.size() > 0)
    {
        out << nl << "write: ";
        writeMarshalInParams(out, op);
        out << ",";
    }

    if(allOutParams.size() > 0)
    {
        out << nl << "read: ";
        writeUnmarshalOutParams(out, op);
        out << ",";
    }

    if(allExceptions.size() > 0)
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
SwiftGenerator::writeProxyAsyncOperation(::IceUtilInternal::Output& out, const OperationPtr& op)
{
    const string opName = fixIdent(op->name() + "Async");

    const ParamInfoList allInParams = getAllInParams(op);
    const ParamInfoList allOutParams = getAllOutParams(op);
    const ExceptionList allExceptions = op->throws();

    const string swiftModule = getSwiftModule(getTopLevelModule(ContainedPtr::dynamicCast(op)));

    out << sp;
    writeOpDocSummary(out, op, true, false);
    out << nl << "func " << opName;
    out << spar;
    for(ParamInfoList::const_iterator q = allInParams.begin(); q != allInParams.end(); ++q)
    {
        if(allInParams.size() == 1)
        {
            out << ("_ iceP_" + q->name + ": " + q->typeStr + (q->isTagged ? " = nil" : ""));
        }
        else
        {
            out << (q->name + " iceP_" + q->name + ": " + q->typeStr + (q->isTagged ? " = nil" : ""));
        }
    }
    out << "context: " + getUnqualified("Ice.Context", swiftModule) + "? = nil";
    out << "sentOn: Dispatch.DispatchQueue? = nil";
    out << "sentFlags: Dispatch.DispatchWorkItemFlags? = nil";
    out << "sent: ((Swift.Bool) -> Swift.Void)? = nil";

    out << epar;
    out << " -> PromiseKit.Promise<";
    if(allOutParams.empty())
    {
        out << "Swift.Void";
    }
    else
    {
        out << operationReturnType(op);
    }
    out << ">";

    out << sb;

    //
    // Invoke
    //
    out << sp;
    out << nl << "return _impl._invokeAsync(";

    out.useCurrentPosAsIndent();
    out << "operation: \"" << op->name() << "\",";
    out << nl << "mode: " << modeToString(op->sendMode()) << ",";

    if(op->format() != DefaultFormat)
    {
        out << nl << "format: " << opFormatTypeToString(op);
        out << ",";
    }

    if(allInParams.size() > 0)
    {
        out << nl << "write: ";
        writeMarshalInParams(out, op);
        out << ",";
    }

    if(allOutParams.size() > 0)
    {
        out << nl << "read: ";
        writeUnmarshalOutParams(out, op);
        out << ",";
    }

    if(allExceptions.size() > 0)
    {
        out << nl << "userException:";
        writeUnmarshalUserException(out, op);
        out << ",";
    }

    out << nl << "context: context,";
    out << nl << "sentOn: sentOn,";
    out << nl << "sentFlags: sentFlags,";
    out << nl << "sent: sent)";
    out.restoreIndent();

    out << eb;
}

void
SwiftGenerator::writeDispatchOperation(::IceUtilInternal::Output& out, const OperationPtr& op)
{
    const string opName = op->name();

    const ParamInfoList allInParams = getAllInParams(op);
    const ParamInfoList allOutParams = getAllOutParams(op);
    const ExceptionList allExceptions = op->throws();

    const string swiftModule = getSwiftModule(getTopLevelModule(ContainedPtr::dynamicCast(op)));

    out << sp;
    out << nl << "func _iceD_" << opName;
    out << spar;
    out << ("incoming inS: " + getUnqualified("Ice.Incoming", swiftModule));
    out << ("current: " + getUnqualified("Ice.Current", swiftModule));
    out << epar;

    out << " throws -> PromiseKit.Promise<" << getUnqualified("Ice.OutputStream", swiftModule) << ">?";

    out << sb;
    if(allInParams.empty())
    {
        out << nl << "try inS.readEmptyParams()";
    }
    else
    {
        out << nl << "let " << operationInParamsDeclaration(op) << " = try inS.read ";
        writeUnmarshalInParams(out, op);
    }

    if(op->format() != DefaultFormat)
    {
        out << nl << "inS.setFormat(" << opFormatTypeToString(op) << ")";
    }

    out << sp;
    out << nl;
    if(!allOutParams.empty())
    {
        out << "let " << operationReturnDeclaration(op) << " = ";
    }
    out << "try self." << fixIdent(opName);

    out << spar;
    for(ParamInfoList::const_iterator q = allInParams.begin(); q != allInParams.end(); ++q)
    {
        out << (q->name + ": iceP_" + q->name);
    }
    out << "current: current";
    out << epar;

    out << sp << nl;
    out << "return inS.setResult";
    if (allOutParams.empty())
    {
        out << "()";
    }
    else
    {
        writeMarshalOutParams(out, op);
    }
    out << eb;
}

void
SwiftGenerator::writeDispatchAsyncOperation(::IceUtilInternal::Output& out, const OperationPtr& op)
{
    const ParamInfoList allInParams = getAllInParams(op);
    const ParamInfoList allOutParams = getAllOutParams(op);

    const string swiftModule = getSwiftModule(getTopLevelModule(ContainedPtr::dynamicCast(op)));

    out << sp;
    out << nl << "func _iceD_" << op->name();
    out << spar;
    out << ("incoming inS: " + getUnqualified("Ice.Incoming", swiftModule));
    out << ("current: " + getUnqualified("Ice.Current", swiftModule));
    out << epar;

    out << " throws -> PromiseKit.Promise<" << getUnqualified("Ice.OutputStream", swiftModule) << ">?";
    out << sb;
    if(allInParams.empty())
    {
        out << nl << "try inS.readEmptyParams()";
    }
    else
    {
        out << nl << "let " << operationInParamsDeclaration(op) << " = try inS.read ";
        writeUnmarshalInParams(out, op);
    }

    if(op->format() != DefaultFormat)
    {
        out << nl << "inS.setFormat(" << opFormatTypeToString(op) << ")";
    }

    out << sp << nl;
    out << "return inS.setResultPromise(" << fixIdent(op->name() + (operationIsAmd(op) ? "Async" : "")) << spar;
    for(ParamInfoList::const_iterator q = allInParams.begin(); q != allInParams.end(); ++q)
    {
        out << (q->name + ": iceP_" + q->name);
    }
    out << "current: current" << epar;
    out << ")";
    if (!allOutParams.empty())
    {
        writeMarshalAsyncOutParams(out, op);
    }
    out << eb;
}

bool
SwiftGenerator::MetadataVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    p->setMetadata(validate(p, p->getAllMetadata(), p->file(), p->line()));
    for (auto& member : p->dataMembers())
    {
        member->setMetadata(validate(member->type(), member->getAllMetadata(), p->file(), member->line()));
    }
    return true;
}

bool
SwiftGenerator::MetadataVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    p->setMetadata(validate(p, p->getAllMetadata(), p->file(), p->line()));
    return true;
}

void
SwiftGenerator::MetadataVisitor::visitOperation(const OperationPtr& p)
{
    InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(p->container());
    StringList metadata = p->getAllMetadata();

    const UnitPtr ut = p->unit();
    const DefinitionContextPtr dc = ut->findDefinitionContext(p->file());
    assert(dc);

    p->setMetadata(validate(p, metadata, p->file(), p->line()));
    for (auto& param : p->allMembers())
    {
        param->setMetadata(validate(param->type(), param->getAllMetadata(), p->file(), param->line()));
    }
}

bool
SwiftGenerator::MetadataVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    p->setMetadata(validate(p, p->getAllMetadata(), p->file(), p->line()));
    for (auto& member : p->dataMembers())
    {
        member->setMetadata(validate(member->type(), member->getAllMetadata(), p->file(), member->line()));
    }
    return true;
}

bool
SwiftGenerator::MetadataVisitor::visitStructStart(const StructPtr& p)
{
    p->setMetadata(validate(p, p->getAllMetadata(), p->file(), p->line()));
    for (auto& member : p->dataMembers())
    {
        member->setMetadata(validate(member->type(), member->getAllMetadata(), p->file(), member->line()));
    }
    return true;
}

void
SwiftGenerator::MetadataVisitor::visitSequence(const SequencePtr& p)
{
    p->setMetadata(validate(p, p->getAllMetadata(), p->file(), p->line()));
}

void
SwiftGenerator::MetadataVisitor::visitDictionary(const DictionaryPtr& p)
{
    const string prefix = "swift:";
    const DefinitionContextPtr dc = p->unit()->findDefinitionContext(p->file());
    assert(dc);

    StringList newMetadata = p->keyMetadata();
    for(StringList::const_iterator q = newMetadata.begin(); q != newMetadata.end();)
    {
        string s = *q++;
        if(s.find(prefix) != 0)
        {
            continue;
        }

        dc->error(p->file(), p->line(), "invalid metadata `" + s + "' for dictionary key type");
    }

    newMetadata = p->valueMetadata();
    TypePtr t = p->valueType();
    for(StringList::const_iterator q = newMetadata.begin(); q != newMetadata.end();)
    {
        string s = *q++;
        if(s.find(prefix) != 0)
        {
            continue;
        }
        dc->error(p->file(), p->line(), "error invalid metadata `" + s + "' for dictionary value type");
    }

    p->setMetadata(validate(p, p->getAllMetadata(), p->file(), p->line()));
}

void
SwiftGenerator::MetadataVisitor::visitEnum(const EnumPtr& p)
{
    p->setMetadata(validate(p, p->getAllMetadata(), p->file(), p->line()));
}

void
SwiftGenerator::MetadataVisitor::visitConst(const ConstPtr& p)
{
    p->setMetadata(validate(p, p->getAllMetadata(), p->file(), p->line()));
}

StringList
SwiftGenerator::MetadataVisitor::validate(const SyntaxTreeBasePtr& cont, const StringList& metadata,
                                          const string& file, int line)
{
    StringList newMetadata = metadata;
    const string prefix = "swift:";
    const UnitPtr ut = cont->unit();
    const DefinitionContextPtr dc = ut->findDefinitionContext(file);
    assert(dc);
    for(StringList::const_iterator p = newMetadata.begin(); p != newMetadata.end();)
    {
        string s = *p++;
        if(s.find(prefix) != 0)
        {
            continue;
        }

        if(ModulePtr::dynamicCast(cont) && s.find("swift:module:") == 0)
        {
            continue;
        }

        if(ClassDefPtr::dynamicCast(cont) && s.find("swift:inherits:") == 0)
        {
            continue;
        }

        if(InterfaceDefPtr::dynamicCast(cont) && s.find("swift:inherits:") == 0)
        {
            continue;
        }

        if((ClassDefPtr::dynamicCast(cont) ||
            InterfaceDefPtr::dynamicCast(cont) ||
            EnumPtr::dynamicCast(cont) ||
            ExceptionPtr::dynamicCast(cont) ||
            OperationPtr::dynamicCast(cont)) && s.find("swift:attribute:") == 0)
        {
            continue;
        }

        dc->warning(InvalidMetadata, file, line, "ignoring invalid metadata `" + s + "'");
        newMetadata.remove(s);
        continue;
    }
    return newMetadata;
}
