//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
//

#include <IceUtil/OutputUtil.h>
#include <IceUtil/StringUtil.h>

#include <Slice/Util.h>

#include <SwiftUtil.h>

using namespace std;
using namespace Slice;
using namespace IceUtilInternal;

namespace
{

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
// Split a scoped name into its components and return the components as a list of (unscoped) identifiers.
//
StringList
Slice::splitScopedName(const string& scoped)
{
    assert(scoped[0] == ':');
    StringList ids;
    string::size_type next = 0;
    string::size_type pos;
    while((pos = scoped.find("::", next)) != string::npos)
    {
        pos += 2;
        if(pos != scoped.size())
        {
            string::size_type endpos = scoped.find("::", pos);
            if(endpos != string::npos)
            {
                ids.push_back(scoped.substr(pos, endpos - pos));
            }
        }
        next = pos;
    }
    if(next != scoped.size())
    {
        ids.push_back(scoped.substr(next));
    }
    else
    {
        ids.push_back("");
    }

    return ids;
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
    StringList ids = splitScopedName(ident);
    transform(ids.begin(), ids.end(), ids.begin(), ptr_fun(lookupKwd));
    ostringstream result;
    for(StringList::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        result << "::" + *i;
    }
    return result.str();
}

string
Slice::getSwiftModule(const ModulePtr& module, string& swiftPrefix)
{
    const string modulePrefix = "swift:module:";

    string swiftModule;

    if(module->findMetaData(modulePrefix, swiftModule))
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
    assert(ProxyPtr::dynamicCast(type) || ContainedPtr::dynamicCast(type));

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    return getTopLevelModule(proxy ? ContainedPtr::dynamicCast(proxy->_class()) :
                                     ContainedPtr::dynamicCast(type));
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
    if(p->findMetaData("deprecate", deprecateMetadata))
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
                                  bool dispatch,
                                  bool local)
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

    int typeCtx = TypeContextInParam;
    if(local)
    {
        typeCtx |= TypeContextLocal;
    }

    const ParamInfoList allInParams = getAllInParams(p, typeCtx);
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

    if(!local)
    {
        out << nl << "///";
        if(dispatch)
        {
            out << nl << "/// - parameter current: `Ice.Current` - The Current object for the dispatch.";
        }
        else
        {
            out << nl << "/// - parameter context: `Ice.Context` - Optional request context.";
        }
    }

    typeCtx = local ? TypeContextLocal : 0;

    if(async)
    {
        if(!dispatch)
        {
            out << nl << "///";
            out << nl << "/// - parameter sentOn: `Dispatch.DispatchQueue?` - Optional dispatch queue used to";
            out << nl << "///   dispatch sent callback, the default is to use `PromiseKit.conf.Q.return` queue.";
            out << nl << "///";
            out << nl << "/// - parameter sentFlags: `Dispatch.DispatchWorkItemFlags?` - Optional dispatch flags used";
            out << nl << "///   to dispatch sent callback";
            out << nl << "///";
            out << nl << "/// - parameter sent: `((Swift.Bool) -> Swift.Void)` - Optional sent callback.";
        }

        out << nl << "///";
        out << nl << "/// - returns: `PromiseKit.Promise<" << operationReturnType(p, typeCtx)
            << ">` - The result of the operation";
    }
    else
    {
        const ParamInfoList allOutParams = getAllOutParams(p, typeCtx);
        if(allOutParams.size() == 1)
        {
            ParamInfo ret = allOutParams.front();
            out << nl << "///";
            out << nl << "/// - returns: `" << ret.typeStr << "`";
            if(p->returnType())
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
            out << nl << "/// - returns: `" << operationReturnType(p, typeCtx) << "`:";
            if(p->returnType())
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
SwiftGenerator::writeProxyDocSummary(IceUtilInternal::Output& out, const ClassDefPtr& p, const string& swiftModule)
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
SwiftGenerator::writeServantDocSummary(IceUtilInternal::Output& out, const ClassDefPtr& p, const string& swiftModule)
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
SwiftGenerator::writeMemberDoc(IceUtilInternal::Output& out, const DataMemberPtr& p)
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
SwiftGenerator::validateMetaData(const UnitPtr& u)
{
    MetaDataVisitor visitor;
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
SwiftGenerator::writeConstantValue(IceUtilInternal::Output& out, const TypePtr& type,
                                   const SyntaxTreeBasePtr& valueType, const string& value,
                                   const StringList&, const string& swiftModule, bool optional)
{
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
SwiftGenerator::typeToString(const TypePtr& type,
                             const ContainedPtr& toplevel,
                             const StringList& metadata,
                             bool optional,
                             int typeCtx)
{
    static const char* builtinTable[] =
    {
        "Swift.UInt8",
        "Swift.Bool",
        "Swift.Int16",
        "Swift.Int32",
        "Swift.Int64",
        "Swift.Float",
        "Swift.Double",
        "Swift.String",
        "Ice.Disp",         // Object
        "Ice.ObjectPrx",    // ObjectPrx
        "Swift.AnyObject",  // LocalObject
        "Ice.Value"         // Value
    };

    if(!type)
    {
        return "";
    }

    bool local = (typeCtx & TypeContextLocal) != 0;
    if(local)
    {
        for(StringList::const_iterator i = metadata.begin(); i != metadata.end(); ++i)
        {
            const string swiftType = "swift:type:";
            const string meta = *i;
            if(meta.find(swiftType) == 0)
            {
                return meta.substr(swiftType.size());
            }
        }
    }

    string t = "";
    //
    // The current module were the type is being used
    //
    string currentModule = getSwiftModule(getTopLevelModule(toplevel));
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    bool nonnull = find(metadata.begin(), metadata.end(), "swift:nonnull") != metadata.end();
    bool inparam = typeCtx & TypeContextInParam;

    if(builtin)
    {
        if(builtin->kind() == Builtin::KindObject && !(typeCtx & TypeContextLocal))
        {
            t = getUnqualified(builtinTable[Builtin::KindValue], currentModule);
        }
        else
        {
            t = getUnqualified(builtinTable[builtin->kind()], currentModule);
        }
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    ProxyPtr prx = ProxyPtr::dynamicCast(type);
    ContainedPtr cont = ContainedPtr::dynamicCast(type);

    if(cl)
    {
        if(cl->isInterface() && !cl->isLocal())
        {
            t = fixIdent(getUnqualified(builtinTable[Builtin::KindValue], currentModule));
        }
        else
        {
            //
            // Annotate nonnull closure as @escaping, Swift optional closure parameters are always
            // @escaping see https://www.jessesquires.com/blog/why-optional-swift-closures-are-escaping/
            //
            if(cl->isLocal() && cl->definition() && cl->definition()->isDelegate() && inparam && nonnull)
            {
                t = "@escaping ";
            }
            t += fixIdent(getUnqualified(getAbsoluteImpl(cl), currentModule));
        }
    }
    else if(prx)
    {
        ClassDefPtr def = prx->_class()->definition();
        if(!def || def->isAbstract())
        {
            t = getUnqualified(getAbsoluteImpl(prx->_class(), "", "Prx"), currentModule);
        }
        else
        {
            t = getUnqualified("Ice.ObjectPrx", currentModule);
        }
    }
    else if(cont)
    {
        t = fixIdent(getUnqualified(getAbsoluteImpl(cont), currentModule));
    }

    if(!nonnull && (optional || isNullableType(type)))
    {
        t += "?";
    }
    return t;
}

string
SwiftGenerator::getAbsolute(const TypePtr& type)
{
    static const char* builtinTable[] =
    {
        "Swift.UInt8",
        "Swift.Bool",
        "Swift.Int16",
        "Swift.Int32",
        "Swift.Int64",
        "Swift.Float",
        "Swift.Double",
        "Swift.String",
        "Ice.Disp",         // Object
        "Ice.ObjectPrx",    // ObjectPrx
        "Swift.AnyObject",  // LocalObject
        "Ice.Value"         // Value
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        return builtinTable[builtin->kind()];
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        return getAbsoluteImpl(proxy->_class(), "", "Prx");
    }

    ContainedPtr cont = ContainedPtr::dynamicCast(type);
    if(cont)
    {
        return getAbsoluteImpl(cont);
    }

    assert(false);
    return "???";
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
SwiftGenerator::getAbsolute(const ProxyPtr& prx)
{
    return getAbsoluteImpl(prx->_class(), "", "Prx");
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
SwiftGenerator::getOptionalFormat(const TypePtr& type)
{
    BuiltinPtr bp = BuiltinPtr::dynamicCast(type);
    if(bp)
    {
        switch(bp->kind())
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
        case Builtin::KindObject:
        {
            return ".Class";
        }
        case Builtin::KindObjectProxy:
        {
            return ".FSize";
        }
        case Builtin::KindLocalObject:
        {
            assert(false);
            break;
        }
        case Builtin::KindValue:
        {
            return ".Class";
        }
        }
    }

    if(EnumPtr::dynamicCast(type))
    {
        return ".Size";
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        return seq->type()->isVariableLength() ? ".FSize" : ".VSize";
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    if(d)
    {
        return (d->keyType()->isVariableLength() || d->valueType()->isVariableLength()) ?
            ".FSize" : ".VSize";
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        return st->isVariableLength() ? ".FSize" : ".VSize";
    }

    if(ProxyPtr::dynamicCast(type))
    {
        return ".FSize";
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    assert(cl);
    return ".Class";
}

bool
SwiftGenerator::isNullableType(const TypePtr& type)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindLocalObject:
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

    return ClassDeclPtr::dynamicCast(type) || ProxyPtr::dynamicCast(type);
}

bool
SwiftGenerator::isProxyType(const TypePtr& p)
{
    const BuiltinPtr builtin = BuiltinPtr::dynamicCast(p);
    return (builtin && builtin->kind() == Builtin::KindObjectProxy) || ProxyPtr::dynamicCast(p);
}

bool
SwiftGenerator::isClassType(const TypePtr& p)
{
    const BuiltinPtr builtin = BuiltinPtr::dynamicCast(p);
    return (builtin && (builtin->kind() == Builtin::KindObject ||
                        builtin->kind() == Builtin::KindValue)) ||
        ClassDeclPtr::dynamicCast(p);
}

bool
SwiftGenerator::containsClassMembers(const StructPtr& s)
{
    DataMemberList dm = s->dataMembers();
    for(DataMemberList::const_iterator i = dm.begin(); i != dm.end(); ++i)
    {
        if(isClassType((*i)->type()))
        {
            return true;
        }

        StructPtr st = StructPtr::dynamicCast((*i)->type());
        if(st && containsClassMembers(st))
        {
            return true;
        }

        SequencePtr seq = SequencePtr::dynamicCast((*i)->type());
        if(seq)
        {
            st = StructPtr::dynamicCast(seq->type());
            if(isClassType(seq->type()) || (st && containsClassMembers(st)))
            {
                return true;
            }
        }

        DictionaryPtr dict = DictionaryPtr::dynamicCast((*i)->type());
        if(dict)
        {
            st = StructPtr::dynamicCast(dict->valueType());
            if(isClassType(dict->valueType()) || (st && containsClassMembers(st)))
            {
                return true;
            }
        }
    }
    return false;
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
                                           const DataMemberList& members,
                                           const ContainedPtr& p)
{
    writeMemberwiseInitializer(out, members, DataMemberList(), members, p, false, true);
}

void
SwiftGenerator::writeMemberwiseInitializer(IceUtilInternal::Output& out,
                                           const DataMemberList& members,
                                           const DataMemberList& baseMembers,
                                           const DataMemberList& allMembers,
                                           const ContainedPtr& p,
                                           bool local,
                                           bool rootClass,
                                           const StringPairList& extraParams)
{
    if(!members.empty())
    {
        out << sp;
        out << nl;
        int typeCtx = TypeContextInParam;
        if(local)
        {
            typeCtx |= TypeContextLocal;
        }
        out << "public init" << spar;
        for(DataMemberList::const_iterator i = allMembers.begin(); i != allMembers.end(); ++i)
        {
            DataMemberPtr m = *i;
            out << (fixIdent(m->name()) + ": " +
                    typeToString(m->type(), p, m->getMetaData(), m->optional(), TypeContextInParam));
        }
        for(StringPairList::const_iterator q = extraParams.begin(); q != extraParams.end(); ++q)
        {
            out << (q->first + ": " + q->second);
        }
        out << epar;
        out << sb;
        for(DataMemberList::const_iterator i = members.begin(); i != members.end(); ++i)
        {
            DataMemberPtr m = *i;
            out << nl << "self." << fixIdent(m->name()) << " = " << fixIdent(m->name());
        }

        if(!rootClass)
        {
            out << nl << "super.init";
            out << spar;
            for(DataMemberList::const_iterator i = baseMembers.begin(); i != baseMembers.end(); ++i)
            {
                const string name = fixIdent((*i)->name());
                out << (name + ": " + name);
            }
            for(StringPairList::const_iterator q = extraParams.begin(); q != extraParams.end(); ++q)
            {
                out << (q->first + ": " + q->first);
            }
            out << epar;
        }
        out << eb;
    }
}

void
SwiftGenerator::writeMembers(IceUtilInternal::Output& out,
                             const DataMemberList& members,
                             const ContainedPtr& p,
                             int typeCtx)
{
    string swiftModule = getSwiftModule(getTopLevelModule(p));
    bool protocol = (typeCtx & TypeContextProtocol);
    string access = protocol ? "" : "public ";
    for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;
        TypePtr type = member->type();
        string defaultValue = member->defaultValue();
        writeMemberDoc(out, member);
        out << nl << access << "var " << fixIdent(member->name()) << ": "
            << typeToString(type, p, member->getMetaData(), member->optional(), typeCtx);
        if(protocol)
        {
            out << " { get set }";
        }
        else
        {
            out << " = ";
            writeConstantValue(out, type, member->defaultValueType(), defaultValue, p->getMetaData(), swiftModule,
                               member->optional());
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
SwiftGenerator::writeMarshalUnmarshalCode(Output &out,
                                          const TypePtr& type,
                                          const ContainedPtr& p,
                                          const string& param,
                                          bool marshal,
                                          int tag)
{
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
            case Builtin::KindByte:
            case Builtin::KindBool:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindLong:
            case Builtin::KindFloat:
            case Builtin::KindDouble:
            case Builtin::KindString:
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
            case Builtin::KindObjectProxy:
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
            case Builtin::KindObject:
            case Builtin::KindValue:
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
            case Builtin::KindLocalObject:
            {
                assert(false);
                break;
            }
            default:
            {
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
            if(cl->isInterface())
            {
                out << nl << "try " << stream << ".read(" << args << ") { " << param << " = $0 }";
            }
            else
            {
                if(tag >= 0)
                {
                    args += ", value: ";
                }
                args += getUnqualified(getAbsolute(type), swiftModule) + ".self";
                out << nl << "try " << stream << ".read(" << args << ") { " << param << " = $0 " << "}";
            }
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

    ProxyPtr prx = ProxyPtr::dynamicCast(type);
    if(prx)
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

            ClassDefPtr def = prx->_class()->definition();
            if(!def || def->isAbstract())
            {
                args += getUnqualified(getAbsolute(type), swiftModule) + ".self";
            }
            else
            {
                args += getUnqualified("Ice.ObjectPrx", swiftModule) + ".self";
            }
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
SwiftGenerator::MetaDataVisitor::visitModuleStart(const ModulePtr& p)
{
    if(UnitPtr::dynamicCast(p->container()))
    {
        const UnitPtr ut = p->unit();
        const DefinitionContextPtr dc = ut->findDefinitionContext(p->file());
        assert(dc);

        // top-level module
        ModulePtr m = ModulePtr::dynamicCast(p);
        const string modulePrefix = "swift:module:";

        string swiftModule;
        string swiftPrefix;

        if(m->findMetaData(modulePrefix, swiftModule))
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
            swiftModule = m->name();
        }

        const string filename = m->definitionContext()->filename();
        ModuleMap::const_iterator current = _modules.find(filename);

        if(current == _modules.end())
        {
            _modules[filename] = swiftModule;
        }
        else if(current->second != swiftModule)
        {
            ostringstream os;
            os << "invalid module mapping:\n Slice module `" << m->scoped() << "' should be map to Swift module `"
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
                os << "invalid module prefix:\n Slice module `" << m->scoped() << "' is already using";
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
    return true;
}

string
SwiftGenerator::paramLabel(const string& param, const ParamDeclList& params)
{
    string s = param;
    for(ParamDeclList::const_iterator q = params.begin(); q != params.end(); ++q)
    {
        if((*q)->name() == param)
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
SwiftGenerator::operationReturnType(const OperationPtr& op, int typeCtx)
{
    ostringstream os;
    bool returnIsTuple = operationReturnIsTuple(op);
    if(returnIsTuple)
    {
        os << "(";
    }

    ParamDeclList outParams = op->outParameters();
    TypePtr returnType = op->returnType();
    if(returnType)
    {
        if(returnIsTuple)
        {
            os << paramLabel("returnValue", outParams) << ": ";
        }
        os << typeToString(returnType, op, op->getMetaData(), op->returnIsOptional(), typeCtx);
    }

    for(ParamDeclList::const_iterator q = outParams.begin(); q != outParams.end(); ++q)
    {
        if(returnType || q != outParams.begin())
        {
            os << ", ";
        }

        if(returnIsTuple)
        {
            os << (*q)->name() << ": ";
        }

        os << typeToString((*q)->type(), *q, (*q)->getMetaData(), (*q)->optional(), typeCtx);
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
    ParamDeclList outParams = op->outParameters();
    TypePtr returnType = op->returnType();
    bool returnIsTuple = operationReturnIsTuple(op);

    if(returnIsTuple)
    {
        os << "(";
    }

    if(returnType)
    {
        os << ("iceP_" + paramLabel("returnValue", outParams));
    }

    for(ParamDeclList::const_iterator q = outParams.begin(); q != outParams.end(); ++q)
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

    ParamDeclList inParams = op->inParameters();
    const bool isTuple = inParams.size() > 1;

    if(!inParams.empty())
    {
        if(isTuple)
        {
            os << "(";
        }
        for(ParamDeclList::const_iterator q = inParams.begin(); q != inParams.end(); ++q)
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
        for(ParamDeclList::const_iterator q = inParams.begin(); q != inParams.end(); ++q)
        {
            if(q != inParams.begin())
            {
                os << ", ";
            }

            os << typeToString((*q)->type(), *q, (*q)->getMetaData(), (*q)->optional());
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
    const ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());
    return cl->hasMetaData("amd") || op->hasMetaData("amd");
}

ParamInfoList
SwiftGenerator::getAllInParams(const OperationPtr& op, int typeCtx)
{
    const ParamDeclList l = op->inParameters();
    ParamInfoList r;
    for(ParamDeclList::const_iterator p = l.begin(); p != l.end(); ++p)
    {
        ParamInfo info;
        info.name = (*p)->name();
        info.type = (*p)->type();
        info.typeStr = typeToString(info.type, op, (*p)->getMetaData(), (*p)->optional(), typeCtx);
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
    for(ParamInfoList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        if(p->optional)
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
        static bool compare(const ParamInfo& lhs, const ParamInfo& rhs)
        {
            return lhs.tag < rhs.tag;
        }
    };
    optional.sort(SortFn::compare);
}

ParamInfoList
SwiftGenerator::getAllOutParams(const OperationPtr& op, int typeCtx)
{
    ParamDeclList params = op->outParameters();
    ParamInfoList l;

    for(ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        ParamInfo info;
        info.name = (*p)->name();
        info.type = (*p)->type();
        info.typeStr = typeToString(info.type, op, (*p)->getMetaData(), (*p)->optional(), typeCtx);
        info.optional = (*p)->optional();
        info.tag = (*p)->tag();
        info.param = *p;
        l.push_back(info);
    }

    if(op->returnType())
    {
        ParamInfo info;
        info.name = paramLabel("returnValue", params);
        info.type = op->returnType();
        info.typeStr = typeToString(info.type, op, op->getMetaData(), op->returnIsOptional(), typeCtx);
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
    for(ParamInfoList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        if(p->optional)
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
        static bool compare(const ParamInfo& lhs, const ParamInfo& rhs)
        {
            return lhs.tag < rhs.tag;
        }
    };
    optional.sort(SortFn::compare);
}

void
SwiftGenerator::writeMarshalInParams(::IceUtilInternal::Output& out, const OperationPtr& op)
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

    for(ParamInfoList::const_iterator q = requiredInParams.begin(); q != requiredInParams.end(); ++q)
    {
        writeMarshalUnmarshalCode(out, q->type, op, "iceP_" + q->name, true);
    }

    for(ParamInfoList::const_iterator q = optionalInParams.begin(); q != optionalInParams.end(); ++q)
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
    ParamInfoList requiredOutParams, optionalOutParams;
    getOutParams(op, requiredOutParams, optionalOutParams);

    out << "{ ostr in";
    out.inc();
    //
    // Marshal parameters
    // 1. required
    // 2. optional (including optional return)
    //

    for(ParamInfoList::const_iterator q = requiredOutParams.begin(); q != requiredOutParams.end(); ++q)
    {
        writeMarshalUnmarshalCode(out, q->type, op, "iceP_" + q->name, true);
    }

    for(ParamInfoList::const_iterator q = optionalOutParams.begin(); q != optionalOutParams.end(); ++q)
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
    ParamInfoList requiredOutParams, optionalOutParams;
    getOutParams(op, requiredOutParams, optionalOutParams);

    out << sb << " (ostr, retVals) in";
    out << nl << "let " << operationReturnDeclaration(op) << " = retVals";
    //
    // Marshal parameters
    // 1. required
    // 2. optional (including optional return)
    //

    for(ParamInfoList::const_iterator q = requiredOutParams.begin(); q != requiredOutParams.end(); ++q)
    {
        writeMarshalUnmarshalCode(out, q->type, op, "iceP_" + q->name, true);
    }

    for(ParamInfoList::const_iterator q = optionalOutParams.begin(); q != optionalOutParams.end(); ++q)
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
    for(ParamInfoList::const_iterator q = requiredOutParams.begin(); q != requiredOutParams.end(); ++q)
    {
        string param;
        if(isClassType(q->type))
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

    for(ParamInfoList::const_iterator q = optionalOutParams.begin(); q != optionalOutParams.end(); ++q)
    {
        string param;
        if(isClassType(q->type))
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
    ParamInfoList requiredInParams, optionalInParams;
    getInParams(op, requiredInParams, optionalInParams);
    const ParamInfoList allInParams = getAllInParams(op);
    //
    // Unmarshal parameters
    // 1. required
    // 3. optional
    //
    out << "{ istr in";
    out.inc();
    for(ParamInfoList::const_iterator q = requiredInParams.begin(); q != requiredInParams.end(); ++q)
    {
        if(q->param)
        {
            string param;
            if(isClassType(q->type))
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

    for(ParamInfoList::const_iterator q = optionalInParams.begin(); q != optionalInParams.end(); ++q)
    {
        string param;
        if(isClassType(q->type))
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
            out << ("_ iceP_" + q->name + ": " + q->typeStr + (q->optional ? " = nil" : ""));
        }
        else
        {
            out << (q->name + " iceP_" + q->name + ": " + q->typeStr + (q->optional ? " = nil" : ""));
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
            out << ("_ iceP_" + q->name + ": " + q->typeStr + (q->optional ? " = nil" : ""));
        }
        else
        {
            out << (q->name + " iceP_" + q->name + ": " + q->typeStr + (q->optional ? " = nil" : ""));
        }
    }
    out << "context: " + getUnqualified("Ice.Context", swiftModule) + "? = nil";
    out << "sentOn: Dispatch.DispatchQueue? = PromiseKit.conf.Q.return";
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
SwiftGenerator::MetaDataVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    validate(p);
    return true;
}

bool
SwiftGenerator::MetaDataVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    validate(p);
    return true;
}

bool
SwiftGenerator::MetaDataVisitor::visitStructStart(const StructPtr& p)
{
    validate(p);
    return true;
}

void
SwiftGenerator::MetaDataVisitor::visitSequence(const SequencePtr& p)
{
    validate(p);
}

void
SwiftGenerator::MetaDataVisitor::visitDictionary(const DictionaryPtr& p)
{
    validate(p);
}

void
SwiftGenerator::MetaDataVisitor::visitEnum(const EnumPtr& p)
{
    validate(p);
}

void
SwiftGenerator::MetaDataVisitor::visitConst(const ConstPtr& p)
{
    validate(p);
}

void
SwiftGenerator::MetaDataVisitor::validate(const ContainedPtr&)
{
}
