
// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
        "Any", "as", "associatedtype", "break", "case", "catch", "class", "continue", "default", "defer", "deinit",
        "do", "else", "enum", "extension", "fallthrough", "false", "fileprivate", "for", "func", "guard", "if",
        "import", "in", "init", "inout", "internal", "is", "let", "nil", "open", "operator", "private", "protocol",
        "public", "repeat", "rethrows", "return", "self", "Self", "static", "struct", "subscript", "super", "Swift",
        "switch", "throw", "throws", "true", "try", "typealias", "var", "where", "while"
    };
    bool found = binary_search(&keywordList[0],
                               &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
                               name,
                               Slice::CICompare());
    if(found)
    {
        return "_" + name;
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

//
// Split a scoped name into its components and return the components as a list of (unscoped) identifiers.
//
static StringList
splitScopedName(const string& scoped)
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
    return getTopLevelModule(proxy ? ContainedPtr::dynamicCast(proxy->_class()->definition()) :
                                     ContainedPtr::dynamicCast(type));
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
    return swiftModule + "." + fixIdent(swiftPrefix + replace(str, "::", ""));
}

}


void
SwiftGenerator::writeConstantValue(IceUtilInternal::Output& out, const TypePtr& type,
                                   const SyntaxTreeBasePtr& valueType, const string& value,
                                   const StringList&, const string& swiftModule)
{
    ConstPtr constant = ConstPtr::dynamicCast(valueType);
    if(constant)
    {
        out << getUnqualified(getAbsolute(constant), swiftModule);
    }
    else
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
}

string
SwiftGenerator::typeToString(const TypePtr& type, const ContainedPtr& toplevel,
                             const StringList& metadata, bool optional,
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
        "Ice.Object",       // Object
        "Ice.ObjectPrx",    // ObjectPrx
        "Swift.AnyObject",  // LocalObject
        "Ice.Value"         // Value
    };

    if(!type)
    {
        return "";
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
        t = getUnqualified(builtinTable[builtin->kind()], currentModule);
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    ProxyPtr prx = ProxyPtr::dynamicCast(type);
    ContainedPtr cont = ContainedPtr::dynamicCast(type);

    if(cl)
    {
        //
        // Annotate nonnull closure as @escaping, Swift optional closure parameters are always
        // @escaping see https://www.jessesquires.com/blog/why-optional-swift-closures-are-escaping/
        //
        if(cl->isLocal() && cl->definition() && cl->definition()->isDelegate() && inparam && nonnull)
        {
            t = "@escaping ";
        }
        t += getUnqualified(getAbsoluteImpl(cl), currentModule);
    }
    else if(prx)
    {
        t = getUnqualified(getAbsoluteImpl(prx->_class(), "", "Prx"), currentModule);
    }
    else if(cont)
    {
        t = getUnqualified(getAbsoluteImpl(cont), currentModule);
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
        "Ice.Object",       // Object
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
            mode = "Ice.OperationMode.Normal";
            break;
        }
        case Operation::Nonmutating:
        {
            mode = "Ice.OperationMode.Nonmutating";
            break;
        }
        case Operation::Idempotent:
        {
            mode = "Ice.OperationMode.Idempotent";
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
SwiftGenerator::isObjcRepresentable(const TypePtr& type)
{
    return BuiltinPtr::dynamicCast(type);
}

bool
SwiftGenerator::isObjcRepresentable(const DataMemberList& members)
{
    for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
    {
        if(!isObjcRepresentable((*q)->type()))
        {
            return false;
        }
    }
    return true;
}

bool
SwiftGenerator::isProxyType(const TypePtr& p)
{
    const BuiltinPtr builtin = BuiltinPtr::dynamicCast(p);
    return (builtin && builtin->kind() == Builtin::KindObjectProxy) || ProxyPtr::dynamicCast(p);
}

void
SwiftGenerator::writeDefaultInitializer(IceUtilInternal::Output& out,
                                        const DataMemberList& members,
                                        const ContainedPtr& p,
                                        bool rootClass,
                                        bool required)
{
    out << sp;
    out << nl << "public ";
    if(required)
    {
        out << "required ";
    }
    else if(!rootClass)
    {
        out << "override ";
    }
    out << "init()";
    out << sb;
    for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
    {
        DataMemberPtr member = *q;
        TypePtr type = member->type();
        if(!member->defaultValueType())
        {
            out << nl << "self." << fixIdent(member->name()) << " = ";
            if(isNullableType(type))
            {
                out << "nil";
            }
            else
            {
                out << typeToString(type, p) << "()";
            }
        }
    }
    if(!rootClass)
    {
        out << nl << "super.init()";
    }
    out << eb;
}

void
SwiftGenerator::writeMemberwiseInitializer(IceUtilInternal::Output& out,
                                           const DataMemberList& members,
                                           const ContainedPtr& p)
{
    writeMemberwiseInitializer(out, members, DataMemberList(), members, p, true);
}

void
SwiftGenerator::writeMemberwiseInitializer(IceUtilInternal::Output& out,
                                           const DataMemberList& members,
                                           const DataMemberList& baseMembers,
                                           const DataMemberList& allMembers,
                                           const ContainedPtr& p,
                                           bool rootClass)
{
    if(allMembers.size() > 0)
    {
        out << sp;
        out << nl;
        out << "public ";
        if(members.size() == 0)
        {
            out << "override ";
        }
        out << "init" << spar;
        for(DataMemberList::const_iterator i = allMembers.begin(); i != allMembers.end(); ++i)
        {
            DataMemberPtr m = *i;
            out << (fixIdent(m->name()) + ": " +
                    typeToString(m->type(), p, m->getMetaData(), m->optional(), TypeContextInParam));
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
        out << nl << access << "var " << fixIdent(member->name()) << ": "
            << typeToString(type, p, member->getMetaData(), member->optional());
        if(protocol)
        {
            out << " { get }";
        }
        else if(!defaultValue.empty())
        {
            out << " = ";
            writeConstantValue(out, type, member->defaultValueType(), defaultValue, p->getMetaData(), swiftModule);
        }
    }
}

void
SwiftGenerator::writeMarshalUnmarshalCode(IceUtilInternal::Output&, const ClassDefPtr&, const OperationPtr&)
{
    // TODO
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
