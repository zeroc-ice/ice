
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
        "public", "repeat", "rethrows", "return", "self", "Self", "static", "struct", "subscript", "super", "switch",
        "throw", "throws", "true", "try", "typealias", "var", "where", "while"
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

static StringList
fixIds(const StringList& ids)
{
    StringList newIds;
    for(StringList::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        newIds.push_back(lookupKwd(*i));
    }
    return newIds;
}

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
    return swiftModule;
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

string
SwiftGenerator::getLocalScope(const string& scope, const string& separator)
{
    assert(!scope.empty());

    //
    // Remove trailing "::" if present.
    //
    string fixedScope;
    if(scope[scope.size() - 1] == ':')
    {
        assert(scope[scope.size() - 2] == ':');
        fixedScope = scope.substr(0, scope.size() - 2);
    }
    else
    {
        fixedScope = scope;
    }
    if(fixedScope.empty())
    {
        return "";
    }
    const StringList ids = fixIds(splitScopedName(fixedScope));

    //
    // Return local scope for "::A::B::C" as A.B.C
    //
    stringstream result;
    for(StringList::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        if(i != ids.begin())
        {
            result << separator;
        }
        result << *i;
    }
    return result.str();

}

string
SwiftGenerator::typeToString(const TypePtr& type, const ContainedPtr& c)
{
    static const char* builtinTable[] =
    {
        "UInt8",
        "Bool",
        "Int16",
        "Int32",
        "Int64",
        "Float",
        "Double",
        "String",
        "Ice.Object",    // Object
        "Ice.ObjectPrx", // ObjectPrx
        "AnyObject",     // LocalObject
        "Ice.Value"      // Value
    };

    if(!type)
    {
        return "";
    }

    const bool hasNonOptionalMetadata = c && c->hasMetaData("swift:non-optional");
    const ParamDeclPtr p = ParamDeclPtr::dynamicCast(c);
    const bool isOptionalParam = p && p->optional();

    string str;
    bool optional = isOptionalParam || (!isValueType(type) && !hasNonOptionalMetadata);

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        str = builtinTable[builtin->kind()];
        return optional ? str + "?" : str;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        str = getAbsolute(cl);

        if(cl->isLocal())
        {
            const ClassDefPtr def = cl->definition();
            if(p && def && def->isDelegate())
            {

                // Swift closures are @escaping by default
                if(p->hasMetaData("swift:non-optional"))
                {
                    return "@escaping " + str;
                }
                else
                {
                    return str + "?";
                }
            }
        }
        return optional ? str + "?" : str;
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        str = getAbsolute(proxy->_class(), "", "Prx");
        return optional ? str + "?" : str;
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        str = getAbsolute(dict);
        return optional ? str + "?" : str;
    }

    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
        str = getAbsolute(contained);
        return optional ? str + "?" : str;
    }

    return "???";
}

std::string
SwiftGenerator::typeToProxyImpl(const TypePtr& type)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindObjectProxy:
            {
                return "_ObjectPrxI";
                break;
            }
            default:
            {
                assert(false);
                break;
            }
        }

    }
    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        return getAbsolute(proxy->_class(), "_", "PrxI");
    }

    return "???";
}

//
// Check the given identifier against Swift's list of reserved words. If it matches
// a reserved word, then an escaped version is returned with a leading underscore.
//
std::string
SwiftGenerator::fixIdent(const std::string& ident)
{
    if(ident[0] != ':')
    {
        return lookupKwd(ident);
    }
    StringList ids = splitScopedName(ident);
    transform(ids.begin(), ids.end(), ids.begin(), ptr_fun(lookupKwd));
    stringstream result;
    for(StringList::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        result << "::" + *i;
    }
    return result.str();
}

std::string
SwiftGenerator::fixName(const ContainedPtr& cont)
{
    return cont->name();
}

//
// Get the fully-qualified name of the given definition. If a suffix is provided,
// it is prepended to the definition's unqualified name. If the nameSuffix
// is provided, it is appended to the container's name.
//
std::string
SwiftGenerator::getAbsolute(const ContainedPtr& cont,
                            const string& pfx,
                            const string& suffix)
{
    string str = fixIdent(cont->scope() + pfx + cont->name() + suffix);
    if(str.find("::") == 0)
    {
        str.erase(0, 2);
    }

    return replace(str, "::", ".");
}

std::string
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
SwiftGenerator::isValueType(const TypePtr& type)
{
    if(!type)
    {
        return true;
    }
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
                return false;
                break;
            }
            default:
            {
                return true;
                break;
            }
        }
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        return false;
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        return false;
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        return true;
    }

    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
        return true;
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
SwiftGenerator::writeTuple(IceUtilInternal::Output& out, const StringList& tuple)
{
    if(tuple.size() > 1)
    {
        out << "(";
    }

    for(StringList::const_iterator q = tuple.begin(); q != tuple.end(); ++q)
    {
        if(q != tuple.begin())
        {
            out << ", ";
        }
        out << (*q);
    }
    if(tuple.size() > 1)
    {
        out << ")";
    }
}

void
SwiftGenerator::writeDataMembers(IceUtilInternal::Output& out, const DataMemberList& members, bool writeGetter)
{
    for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
    {
        const TypePtr type = (*q)->type();
        out << nl;

        // not a protocol
        if(!writeGetter)
        {
            out << "public ";
        }

        out << "var " << (*q)->name() << ": " << typeToString(type, (*q));

        // protocol
        if(writeGetter)
        {
            out << " { get }";
        }
    }
}

void
SwiftGenerator::writeInitializer(IceUtilInternal::Output& out,
                                 const DataMemberList& members,
                                 const DataMemberList& allMembers)
{

    DataMemberList baseMembers;

    for(DataMemberList::const_iterator q = allMembers.begin(); q != allMembers.end(); ++q)
    {
        if(find(members.begin(), members.end(), *q) == members.end())
        {
            baseMembers.push_back(*q);
        }
    }

    // initializer
    out << nl << "public init";
    writeInitializerMembers(out, allMembers.empty() ? members : allMembers);
    out << sb;

    for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
    {
        const string n = (*q)->name();
        out << nl << "self." << n << " = " << n;
    }

    // call base class init
    if(!baseMembers.empty())
    {
        out << nl << "super.init";
        writeInitializerMembers(out, baseMembers, false);
    }

    out << eb;
}

void
SwiftGenerator::writeInitializerMembers(IceUtilInternal::Output& out, const DataMemberList& members, bool useType)
{
    out << "(";
    for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
    {
         if(q != members.begin())
        {
            out << ", ";
        }
        out << (*q)->name();

        if(useType)
        {
            out << ": " << typeToString((*q)->type(), *q);
        }
        else
        {
            out << ": " << (*q)->name();
        }
    }
    out << ")";
}

void
SwiftGenerator::writeOperation(IceUtilInternal::Output& out, const OperationPtr& op, bool)
{
    const string opName = op->name();
    StringList metaData = op->getMetaData();
    ExceptionList throws = op->throws();
    const ParamDeclList outParams = op->outParameters();
    StringList returns;
    throws.sort();
    throws.unique();

    out << nl << "func " << opName;
    writeOperationsParameters(out, op->parameters());
    //TODO: remove cpp:noexcept once swift:nothrow is everywhere
    if(!op->hasMetaData("cpp:noexcept") && !op->hasMetaData("swift:nothrow"))
    {
        out << " throws";
    }

    string ret = typeToString(op->returnType(), op);
    if(!ret.empty())
    {
        returns.push_back(ret);
    }

    for(ParamDeclList::const_iterator p = outParams.begin(); p != outParams.end(); ++p)
    {
        returns.push_back(typeToString((*p)->type()));
    }

    if(!returns.empty())
    {
        out << " -> ";
        writeTuple(out, returns);
    }
}

void
SwiftGenerator::writeOperationsParameters(IceUtilInternal::Output& out, const ParamDeclList& parameters)
{
    out << "(";
    for(ParamDeclList::const_iterator q = parameters.begin(); q != parameters.end() && !(*q)->isOutParam(); ++q)
    {

         if(q != parameters.begin())
        {
            out << ", ";
        }

        out << (*q)->name() << ": " << typeToString((*q)->type(), *q);
    }
    out << ")";
}

void
SwiftGenerator::writeCastFuncs(IceUtilInternal::Output& out, const ClassDefPtr& p)
{
    const string prx = fixName(p) + "Prx";
    const string prxImpl = "_" + prx + "I";
    out << nl << "public func checkedCast(prx: ObjectPrx, type: " << prx << ".Protocol, ";
    out << "facet: String? = nil, context: Context? = nil) throws -> " << prx << "?";
    out << sb << nl;
    out << "return try " << prxImpl << ".checkedCast(prx: prx, facet: facet, context: context) as " << prxImpl << "?";
    out << eb << nl;

    out << nl << "public func uncheckedCast(prx: ObjectPrx, type: " << prx << ".Protocol, ";
    out << "facet: String? = nil, context: Context? = nil) -> " << prx << "?";
    out << sb << nl;
    out << "return " << prxImpl << ".uncheckedCast(prx: prx, facet: facet, context: context) as " << prxImpl << "?";
    out << eb << nl;

    out << nl << "public extension Ice.InputStream";
    out << sb << nl;
    out << "func read(proxy: " << prx << ".Protocol) throws -> " << prx << "?";
    out << sb << nl;
    out << "return try " << prxImpl << ".ice_read(from: self)";
    out << eb << nl;

    out << "func read(proxyArray: " << prx << ".Protocol) throws -> [" << prx << "?]";
    out << sb << nl;
    // out << "return try " << prxImpl << ".ice_read(from: self)";
    out << "#warning(\"add generated proxy arrays\")" << nl;
    out << "preconditionFailure(\"TODO\")";
    out << eb << nl;
    out << eb << nl;
}

void
SwiftGenerator::writeStaticId(IceUtilInternal::Output& out, const ClassDefPtr& p)
{
    const string prx = fixName(p) + "Prx";
    const string prxImpl = "_" + prx + "I";

    out << nl << "public func ice_staticId(_: " << prx << ".Protocol) -> String";
    out << sb << nl;
    out << "return " << prxImpl << ".ice_staticId()";
    out << eb << nl;
}

void
SwiftGenerator::writeMarshalUnmarshalCode(IceUtilInternal::Output& out, const ClassDefPtr& p, const OperationPtr& op)
{
    const string prx = fixName(p) + "Prx";
    const string proxyImpl = "_" + prx + "I";
    const string returnType = typeToString(op->returnType());
    const ParamDeclList inParams = op->inParameters();
    const ParamDeclList outParams = op->outParameters();
    const bool returnsData = op->returnsData();
    const bool returnsInputStream = !op->outParameters().empty() || op->returnType();
    StringList returnTuple;

    out << "let impl = self as! " << proxyImpl;
    out << nl << "let os = impl._createOutputStream()";
    out << nl << "os.startEncapsulation()";

    for(ParamDeclList::const_iterator q = inParams.begin(); q != inParams.end(); ++q)
    {
        const SequencePtr sequence = SequencePtr::dynamicCast(op->returnType());
        if(isProxyType((*q)->type()))
        {
            out << nl << "os.write(proxy: " << (*q)->name() << ")";
        }
        else if(sequence && isProxyType(sequence->type()))
        {
            out << nl << "os.write(proxyArray: " << (*q)->name() << ")";
        }
        else
        {
            out << nl << (*q)->name() << ".ice_write(to: os)";

        }
    }
    out << nl << "os.endEncapsulation()";

    out << nl << "let " << (returnsInputStream ? "ins  " : "_ ");
    out << "= try impl._invoke(";

    out.useCurrentPosAsIndent();
    out << "operation: \"" << op->name() << "\",";
    out << nl << "mode: " << modeToString(op->mode()) << ",";
    out << nl << "twowayOnly: " << (returnsData ? "true" : "false") << ",";
    out << nl << "inParams: os,";
    out << nl << "hasOutParams: " << (op->outParameters().empty() ? "false" : "true");
    out <<  ")";
    out.restoreIndent();
    out << nl;

    //
    // TODO: Sequence and optioanl read (eg. optional tags and array min size)
    //

    for(ParamDeclList::const_iterator q = outParams.begin(); q != outParams.end(); ++q)
    {
        out << "let " << (*q)->name() << " = try " << typeToString((*q)->type()) << "(from: ins)";
        returnTuple.push_back((*q)->name());
    }

    if(!returnType.empty())
    {
        const SequencePtr sequence = SequencePtr::dynamicCast(op->returnType());
        if(isProxyType(op->returnType()))
        {
            const ProxyPtr proxy = ProxyPtr::dynamicCast(op->returnType());
            if(proxy)
            {
                const string retPrx = getAbsolute(proxy->_class(), "", "Prx");
                out << nl << "let ret = try ins.read(proxy: " << retPrx << ".self)";
            }
            else
            {
                out << nl << "let ret = try ins.read(proxy: ObjectPrx.self)";
            }
            returnTuple.push_front("ret");
        }
        else if(sequence && isProxyType(sequence->type()))
        {
            const ProxyPtr proxy = ProxyPtr::dynamicCast(op->returnType());
            if(proxy)
            {
                const string retPrx = getAbsolute(proxy->_class(), "", "Prx");
                out << nl << "let ret = try ins.read(proxyArray: " << retPrx << ".self)";
            }
            else
            {
                out << nl << "let ret = try ins.read(proxyArray: ObjectPrx.self)";
            }
            returnTuple.push_front("ret");
        }
        else if(op->returnIsOptional())
        {
            out << nl << "var ret: " << returnType;
            out << nl << "try " << "ret.ice_read(from: ins)";
            returnTuple.push_front("ret");
        }
        else
        {
            out << nl << "var ret = " << returnType << "()";
            out << nl << "try " << "ret.ice_read(from: ins)";
            returnTuple.push_front("ret");
        }
    }

    if(!returnTuple.empty())
    {
        out << nl << "return ";
        writeTuple(out, returnTuple);
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
            else
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
