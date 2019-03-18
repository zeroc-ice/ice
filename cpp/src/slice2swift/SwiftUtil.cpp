
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
            t = getUnqualified(builtinTable[Builtin::KindValue], currentModule);
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
            t += getUnqualified(getAbsoluteImpl(cl), currentModule);
        }
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
            return "Ice.OptionalFormat.F1";
        }
        case Builtin::KindShort:
        {
            return "Ice.OptionalFormat.F2";
        }
        case Builtin::KindInt:
        case Builtin::KindFloat:
        {
            return "Ice.OptionalFormat.F4";
        }
        case Builtin::KindLong:
        case Builtin::KindDouble:
        {
            return "Ice.OptionalFormat.F8";
        }
        case Builtin::KindString:
        {
            return "Ice.OptionalFormat.VSize";
        }
        case Builtin::KindObject:
        {
            return "Ice.OptionalFormat.Class";
        }
        case Builtin::KindObjectProxy:
        {
            return "Ice.OptionalFormat.FSize";
        }
        case Builtin::KindLocalObject:
        {
            assert(false);
            break;
        }
        case Builtin::KindValue:
        {
            return "Ice.OptionalFormat.Class";
        }
        }
    }

    if(EnumPtr::dynamicCast(type))
    {
        return "Ice.OptionalFormat.Size";
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        return seq->type()->isVariableLength() ? "Ice.OptionalFormat.FSize" : "Ice.OptionalFormat.VSize";
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    if(d)
    {
        return (d->keyType()->isVariableLength() || d->valueType()->isVariableLength()) ?
            "Ice.OptionalFormat.FSize" : "Ice.OptionalFormat.VSize";
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        return st->isVariableLength() ? "Ice.OptionalFormat.FSize" : "Ice.OptionalFormat.VSize";
    }

    if(ProxyPtr::dynamicCast(type))
    {
        return "Ice.OptionalFormat.FSize";
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    assert(cl);
    return "Ice.OptionalFormat.Class";
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
                                           bool rootClass,
                                           const StringPairList& extraParams)
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
SwiftGenerator::writeMarshalUnmarshalCode(Output &out,
                                          const DataMemberPtr& member,
                                          const ContainedPtr& topLevel,
                                          bool insideStream,
                                          bool declareParam,
                                          bool marshal,
                                          int tag)
{
    TypePtr type = member->type();
    string typeStr = typeToString(type, topLevel, member->getMetaData(), member->optional());
    string param = member->name();

    writeMarshalUnmarshalCode(out, type, typeStr, param, topLevel, insideStream, declareParam, marshal, tag);
}

void
SwiftGenerator::writeMarshalUnmarshalCode(Output &out,
                                          const ParamDeclPtr& param,
                                          bool insideStream,
                                          bool declareParam,
                                          bool marshal,
                                          int tag)
{
    ContainedPtr topLevel = ContainedPtr::dynamicCast(param->container());
    TypePtr type = param->type();
    string typeStr = typeToString(type, topLevel, param->getMetaData(), param->optional());
    string name = param->name();

    writeMarshalUnmarshalCode(out, type, typeStr, name, topLevel, insideStream, declareParam, marshal, tag);
}

void
SwiftGenerator::writeMarshalUnmarshalCode(Output &out,
                                          const TypePtr& type,
                                          const string& typeStr,
                                          const string& param,
                                          const ContainedPtr& topLevel,
                                          bool insideStream,
                                          bool declareParam,
                                          bool marshal,
                                          int tag)
{
    string streamName = marshal ? "ostr" : "istr";
    string assign = declareParam ? ("let " + param + ": " + typeStr) : param;
    string marshalParam = insideStream ? ("v." + param) : param;
    string unmarshalParam;
    string stream = insideStream ? "" : (streamName + ".");

    string swiftModule = getSwiftModule(getTopLevelModule(topLevel));

    if(tag >= 0)
    {
        marshalParam = "tag: " + int64ToString(tag) + ", value: " + marshalParam;
        unmarshalParam = "tag: " + int64ToString(tag);
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
                    out << nl << stream << "write(" << marshalParam << ")";
                }
                else
                {
                    out << nl << assign << " = try " << stream << "read(" << unmarshalParam << ")";
                }
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if(marshal)
                {
                    out << nl << stream << "write(" << marshalParam << ")";
                }
                else
                {
                    out << nl << assign << " = try " << stream << "read(" << unmarshalParam << ") as _ObjectPrxI?";
                }
                break;
            }
            case Builtin::KindObject:
            case Builtin::KindValue:
            {
                if(marshal)
                {
                    out << nl << stream << "write(" << marshalParam << ")";
                }
                else
                {
                    if(declareParam)
                    {
                        out << nl << "var " << param << ": " << typeStr;
                    }
                    out << nl << "try " << stream << "read(" << unmarshalParam << ") { ";
                    if(!declareParam)
                    {
                        out << "self.";
                    }
                    out << param << " = $0 }";
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

            }
        }
    }

    if(ClassDeclPtr::dynamicCast(type))
    {
        ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
        if(marshal)
        {
            out << nl << stream << "write(" << marshalParam << ")";
        }
        else
        {
            if(declareParam)
            {
                out << nl << "var " << param << ": " << typeStr;
            }
            if(cl->isInterface())
            {
                out << nl << "try " << stream << "read(" << unmarshalParam << ") { ";
                if(!declareParam)
                {
                    out << "self.";
                }
                out << param << " = $0 }";
            }
            else
            {
                out << nl << "try " << stream << "read";
                out << spar;
                if(!unmarshalParam.empty())
                {
                    out << unmarshalParam;
                }
                const string className = getUnqualified(getAbsolute(type), swiftModule);
                out << ("value: " + className + ".self");
                out << epar;
                out << sb;
                out << nl;
                if(!declareParam)
                {
                    out << "self.";
                }
                out << param << " = $0";
                out << eb;
            }
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        if(marshal)
        {
            out << nl << stream << "write(" << marshalParam << ")";
        }
        else
        {
            out << nl << assign << " = try " << stream << "read(" << unmarshalParam << ")";
        }
        return;
    }
    ProxyPtr prx = ProxyPtr::dynamicCast(type);
    if(prx)
    {
        if(marshal)
        {
            out << nl << stream << "write(" << marshalParam << ")";
        }
        else
        {
            const string prxI = "_" + getUnqualified(getAbsolute(type), swiftModule) + "I?";
            out << nl << assign << " = try " << stream << "read(" << unmarshalParam << ") as " << prxI ;
        }
        return;
    }

    if(StructPtr::dynamicCast(type))
    {
        if(marshal)
        {
            out << nl << stream << "write(" << marshalParam << ")";
        }
        else
        {
            out << nl << assign << " = try " << stream << "read(" << unmarshalParam << ")";
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
                out << nl << stream << "write(" << marshalParam << ")";
            }
            else
            {
                out << nl << assign << " = try " << stream << "read(" << unmarshalParam << ")";
            }
        }
        else
        {
            string helper = typeStr + "Helper";
            if(marshal)
            {
                out << nl << helper <<".write";
                out << spar;
                out << ("to: " + streamName);
                out << ("value: " + marshalParam);
                out << epar;
            }
            else
            {
                out << nl << assign << " = try " << helper << ".read";
                out << spar;
                out << ("from: " + streamName);
                if(!unmarshalParam.empty())
                {
                    out << unmarshalParam;
                }
                out << epar;
            }
        }
        return;
    }

    if(DictionaryPtr::dynamicCast(type))
    {
        string helper = typeStr + "Helper";
        if(marshal)
        {
            out << nl << helper << ".write";
            out << spar;
            out << ("to: " + streamName);
            out << ("value: " + marshalParam);
            out << epar;
        }
        else
        {
            out << nl << assign << " = try " << helper << ".read";
            out << spar;
            out << ("from: " + streamName);
            if(!unmarshalParam.empty())
            {
                out << unmarshalParam;
            }
            out << epar;
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

void
SwiftGenerator::writeProxyOperation(::IceUtilInternal::Output& out, const OperationPtr& op, bool async)
{
    const string opName = fixIdent(op->name()) + (async ? "Async" : "");
    TypePtr returnType = op->returnType();

    ParamDeclList paramList = op->parameters();
    ParamDeclList inParams = op->inParameters();
    ParamDeclList outParams = op->outParameters();

    ParamDeclList requiredInParams, optionalInParams;
    ParamDeclList requiredOutParams, optionalOutParams;
    op->inParameters(requiredInParams, optionalInParams);
    op->outParameters(requiredOutParams, optionalOutParams);

    ExceptionList throws = op->throws();
    throws.sort();
    throws.unique();

    const string swiftModule = getSwiftModule(getTopLevelModule(ContainedPtr::dynamicCast(op)));

    const bool twowayOnly = op->returnsData();
    const bool useInputStream = !op->outParameters().empty() || returnType;

    out << sp;
    out << nl << "func " << opName;
    out << spar;
    const string omitLabel = inParams.size() == 1 ? "_ " : "";
    for(ParamDeclList::const_iterator q = inParams.begin(); q != inParams.end(); ++q)
    {
        ParamDeclPtr param = *q;
        out << (omitLabel + param->name() + ": " + typeToString(param->type(), param));
    }
    // context
    out << "context: Context? = nil";
    if(async)
    {
        out << "sent: ((Bool) -> Void)? = nil";
    }
    out << epar;

    out << " throws";
    if(useInputStream || async)
    {
        out << " -> ";
        // Async operations always return a Promise
        if(async)
        {
            out << "PromiseKit.Promise<";
        }
    }

    if(useInputStream)
    {
        StringList returnTypes;
        if(returnType) // return parameter first
        {
            returnTypes.push_back(typeToString(returnType, op));
        }
        for(ParamDeclList::const_iterator q = outParams.begin(); q != outParams.end(); ++q)
        {
            ParamDeclPtr param = *q;
            returnTypes.push_back(typeToString(param->type(), param, param->getMetaData(), param->optional()));
        }

        if(returnTypes.size() == 1)
        {
            out << returnTypes.front();
        }
        else
        {
            out << spar;
            if(returnType)
            {
                out << typeToString(returnType, op);
            }
            for(ParamDeclList::const_iterator q = outParams.begin(); q != outParams.end(); ++q)
            {
                ParamDeclPtr param = *q;
                out << typeToString(param->type(), param, param->getMetaData(), param->optional());
            }
            out << epar;
        }

        if(async)
        {
            out << ">";
        }
    }
    else if(async)
    {
        out << "Void>";
    }

    out << sb;

    //
    // Marshal parameters
    // 1. required
    // 2. optional
    //
    out << nl << "let ostr = impl._createOutputStream()";
    out << nl << "ostr.startEncapsulation()";
    for(ParamDeclList::const_iterator q = requiredInParams.begin(); q != requiredInParams.end(); ++q)
    {
        ParamDeclPtr param = *q;
        writeMarshalUnmarshalCode(out, param, false, false, true);
    }
    for(ParamDeclList::const_iterator q = optionalInParams.begin(); q != optionalInParams.end(); ++q)
    {
        ParamDeclPtr param = *q;
        assert(param->optional());
        writeMarshalUnmarshalCode(out, param, false, false, true, param->tag());
    }
    out << nl << "ostr.endEncapsulation()";

    //
    // Invoke
    //
    out << sp;
    out << nl;

    if(async)
    {
         out << "return impl._invokeAsync(";
    }
    else
    {
        out << "let " << (useInputStream ? "istr " : "_ ");
        out << "= try impl._invoke(";
    }

    out.useCurrentPosAsIndent();
    out << "operation: \"" << fixIdent(op->name()) << "\",";
    out << nl << "mode: " << modeToString(op->mode()) << ",";
    out << nl << "twowayOnly: " << (twowayOnly ? "true" : "false") << ",";
    out << nl << "inParams: ostr,";
    out << nl << "hasOutParams: " << (useInputStream ? "true" : "false") << ",";
    out << nl << "exceptions: ";
    out << "[";
    for(ExceptionList::const_iterator q = throws.begin(); q != throws.end(); ++q)
    {
        ExceptionPtr ex = *q;
        out << getUnqualified(getAbsolute(ex), swiftModule) << ".self";
        if(q != throws.end())
        {
            out << ',';
        }
    }
    out << "],";
    out << nl << "context: context";
    out <<  ")";
    out.restoreIndent();

    //
    // Unmarshal parameters
    // 1. required
    // 2. return
    // 3. optional (including optional return)
    //
    if(async && !useInputStream)
    {
        out << sb;
        out << " _ in";
        out << eb;
    }
    if(useInputStream)
    {
        if(async)
        {
            out << sb;
            out << " istr in";
        }
        out << sp;
        out << nl << "try istr.startEncapsulation()";
        StringList returnVals;
        for(ParamDeclList::const_iterator q = requiredOutParams.begin(); q != requiredOutParams.end(); ++q)
        {
            ParamDeclPtr param = *q;
            writeMarshalUnmarshalCode(out, param, false, true, false);
            returnVals.push_back((*q)->name());
        }

        // If the return type is optional we unmarshal it with the rest of the optional params (in order)
        if(returnType && !op->returnIsOptional())
        {
            writeMarshalUnmarshalCode(out, returnType, typeToString(returnType, op), "ret",
                                      ContainedPtr::dynamicCast(op->container()), false,
                                      true, false);
            returnVals.push_front("ret");
        }

        bool optReturnUnmarshaled = false;
        for(ParamDeclList::const_iterator q = optionalOutParams.begin(); q != optionalOutParams.end(); ++q)
        {
            ParamDeclPtr param = *q;
            assert(param->optional());

            if(returnType && op->returnIsOptional() && !optReturnUnmarshaled && (op->returnTag() < param->tag()))
            {
                writeMarshalUnmarshalCode(out, returnType, typeToString(returnType, op), "ret",
                                          ContainedPtr::dynamicCast(op->container()), false,
                                          true, false, op->returnTag());
                returnVals.push_front("ret");
                optReturnUnmarshaled = true;
            }
            writeMarshalUnmarshalCode(out, param, false, true, false, param->tag());
            returnVals.push_back((*q)->name());
        }
        out << nl << "try istr.endEncapsulation()";

        out << sp;
        out << nl << "return ";
        if(returnVals.size() == 1)
        {
            out << returnVals.front();
        }
        else
        {
            out << spar;
            for(StringList::const_iterator q = returnVals.begin(); q != returnVals.end(); ++q)
            {
                out << *q;
            }
            out << epar;
        }

        if(async)
        {
            out << eb;
        }
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
