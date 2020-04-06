//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <CsUtil.h>
#include <Slice/Util.h>
#include <IceUtil/Functional.h>
#include <IceUtil/StringUtil.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#  include <direct.h>
#else
#  include <unistd.h>
#endif

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

bool
Slice::normalizeCase(const ContainedPtr& c)
{
    auto fileMetaData = c->unit()->findDefinitionContext(c->file())->getMetaData();
    if(find(begin(fileMetaData), end(fileMetaData), "normalize-case") != end(fileMetaData) ||
       find(begin(fileMetaData), end(fileMetaData), "cs:normalize-case") != end(fileMetaData))
    {
        return true;
    }
    return false;
}
std::string
Slice::operationName(const OperationPtr& op)
{
    return normalizeCase(op) ? pascalCase(op->name()) : op->name();
}

std::string
Slice::paramName(const ParamInfo& info)
{
    return normalizeCase(info.operation) ? camelCase(info.name) : info.name;
}

std::string
Slice::interfaceName(const ClassDefPtr& c)
{
    string name = normalizeCase(c) ? pascalCase(c->name()) : c->name();
    return name.find("II") == 0 ? name : "I" + name;
}

std::string
Slice::interfaceName(const ProxyPtr& p)
{
    string name = normalizeCase(p->_class()) ? pascalCase(p->_class()->name()) : p->_class()->name();
    return name.find("II") == 0 ? name : "I" + name;
}

std::string
Slice::dataMemberName(const ParamInfo& info)
{
    return normalizeCase(info.operation) ? pascalCase(info.name) : info.name;
}

std::string
Slice::dataMemberName(const DataMemberPtr& p)
{
    return normalizeCase(p) ? pascalCase(p->name()) : p->name();
}

std::string
Slice::helperName(const TypePtr& type, const string& scope)
{
    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    assert(contained);
    return getUnqualified(contained, scope, "", "Helper");
}

bool
Slice::isNullable(const TypePtr& type)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin && (builtin->usesClasses() || builtin->kind() == Builtin::KindObjectProxy))
    {
        return true;
    }
    return ClassDeclPtr::dynamicCast(type) || ProxyPtr::dynamicCast(type);
}

namespace
{

const std::array<std::string, 18> builtinSuffixTable =
{
    "Bool",
    "Byte",
    "Short",
    "UShort",
    "Int",
    "UInt",
    "VarInt",
    "VarUInt",
    "Long",
    "ULong",
    "VarLong",
    "VarULong",
    "Float",
    "Double",
    "String",
    "Class",
    "Proxy",
    "Class"
};

string
mangleName(const string& name, unsigned int baseTypes)
{
    static const char* ObjectNames[] = { "Equals", "Finalize", "GetHashCode", "GetType", "MemberwiseClone",
                                         "ReferenceEquals", "ToString", 0 };

    static const char* ExceptionNames[] = { "Data", "GetBaseException", "GetObjectData", "HelpLink", "HResult",
                                            "InnerException", "Message", "Source", "StackTrace", "TargetSite", 0 };
    string mangled = name;

    if((baseTypes & ExceptionType) == ExceptionType)
    {
        for(int i = 0; ExceptionNames[i] != 0; ++i)
        {
            if(ciequals(name, ExceptionNames[i]))
            {
                return "Ice" + name;
            }
        }
        baseTypes |= ObjectType; // Exception is an Object
    }

    if((baseTypes & ObjectType) == ObjectType)
    {
        for(int i = 0; ObjectNames[i] != 0; ++i)
        {
            if(ciequals(name, ObjectNames[i]))
            {
                return "Ice" + name;
            }
        }
    }

    return mangled;
}

string
lookupKwd(const string& name, unsigned int baseTypes)
{
    //
    // Keyword list. *Must* be kept in alphabetical order.
    //
    static const string keywordList[] =
    {
        "abstract", "as", "async", "await", "base", "bool", "break", "byte", "case", "catch", "char", "checked",
        "class", "const", "continue", "decimal", "default", "delegate", "do", "double", "else", "enum", "event",
        "explicit", "extern", "false", "finally", "fixed", "float", "for", "foreach", "goto", "if", "implicit",
        "in", "int", "interface", "internal", "is", "lock", "long", "namespace", "new", "null", "object", "operator",
        "out", "override", "params", "private", "protected", "public", "readonly", "ref", "return", "sbyte", "sealed",
        "short", "sizeof", "stackalloc", "static", "string", "struct", "switch", "this", "throw", "true", "try",
        "typeof", "uint", "ulong", "unchecked", "unsafe", "ushort", "using", "virtual", "void", "volatile", "while"
    };
    bool found = binary_search(&keywordList[0],
                               &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
                               name,
                               Slice::CICompare());
    if(found)
    {
        return "@" + name;
    }
    return mangleName(name, baseTypes);
}

}

string
Slice::getNamespacePrefix(const ContainedPtr& cont)
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

    assert(m);

    static const string prefix = "cs:namespace:";

    string q;
    if(m->findMetaData(prefix, q))
    {
        q = q.substr(prefix.size());
    }
    return q;
}

string
Slice::CsGenerator::getCustomTypeIdNamespace(const UnitPtr& ut)
{
    DefinitionContextPtr dc = ut->findDefinitionContext(ut->topLevelFile());
    assert(dc);

    static const string typeIdNsPrefix = "cs:typeid-namespace:";
    string result = dc->findMetaData(typeIdNsPrefix);
    if(!result.empty())
    {
        result = result.substr(typeIdNsPrefix.size());
    }
    return result;
}

string
Slice::getNamespace(const ContainedPtr& cont)
{
    string scope = fixId(cont->scope());
    if(scope.rfind(".") == scope.size() - 1)
    {
        scope = scope.substr(0, scope.size() - 1);
    }
    string prefix = getNamespacePrefix(cont);
    if(!prefix.empty())
    {
        if(!scope.empty())
        {
            return prefix + "." + scope;
        }
        else
        {
            return prefix;
        }
    }

    return scope;
}

string
Slice::getUnqualified(const string& type, const string& scope, bool builtin)
{
    if(type.find(".") != string::npos && type.find(scope) == 0 && type.find(".", scope.size() + 1) == string::npos)
    {
        return type.substr(scope.size() + 1);
    }
    else if(builtin)
    {
        return type.find(".") == string::npos ? type : "global::" + type;
    }
    else
    {
        return "global::" + type;
    }
}

string
Slice::getUnqualified(const ContainedPtr& p, const string& package, const string& prefix, const string& suffix)
{
    string name = fixId(prefix + p->name() + suffix);
    string contPkg = getNamespace(p);
    if(contPkg == package || contPkg.empty())
    {
        return name;
    }
    else
    {
        return "global::" + contPkg + "." + name;
    }
}

//
// If the passed name is a scoped name, return the identical scoped name,
// but with all components that are C# keywords replaced by
// their "@"-prefixed version; otherwise, if the passed name is
// not scoped, but a C# keyword, return the "@"-prefixed name;
// otherwise, check if the name is one of the method names of baseTypes;
// if so, prefix it with ice_; otherwise, return the name unchanged.
//
string
Slice::fixId(const string& name, unsigned int baseTypes)
{
    if(name.empty())
    {
        return name;
    }
    if(name[0] != ':')
    {
        return lookupKwd(name, baseTypes);
    }
    vector<string> ids = splitScopedName(name);
    transform(begin(ids), end(ids), begin(ids), [baseTypes](const std::string& i)
                                                {
                                                    return lookupKwd(i, baseTypes);
                                                });
    ostringstream os;
    for(vector<string>::const_iterator i = ids.begin(); i != ids.end();)
    {
        os << *i;
        if(++i != ids.end())
        {
            os << ".";
        }
    }
    return os.str();
}

string
Slice::CsGenerator::getTagFormat(const TypePtr& type, const string& scope)
{
    return getUnqualified("Ice.OptionalFormat", scope) + "." + type->getTagFormat();
}

string
Slice::CsGenerator::typeToString(const TypePtr& type, const string& package, bool optional, bool readOnly)
{
    if(!type)
    {
        return "void";
    }

    if(optional)
    {
        return typeToString(type, package) + "?";
    }

    static const std::array<std::string, 18> builtinTable =
    {
        "bool",
        "byte",
        "short",
        "ushort",
        "int",
        "uint",
        "int",
        "uint",
        "long",
        "ulong",
        "long",
        "ulong",
        "float",
        "double",
        "string",
        "Ice.IObject",
        "Ice.IObjectPrx",
        "Ice.AnyClass"
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        if(builtin->kind() == Builtin::KindObject)
        {
            return getUnqualified(builtinTable[Builtin::KindValue], package, true);
        }
        else
        {
            return getUnqualified(builtinTable[builtin->kind()], package, true);
        }
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(cl->isInterface())
        {
            return getUnqualified("Ice.AnyClass", package);
        }
        else
        {
            return getUnqualified(cl, package);
        }
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        ClassDefPtr def = proxy->_class()->definition();
        if(!def || def->isAbstract())
        {
            return getUnqualified(getNamespace(proxy->_class()) + "." +
                                  interfaceName(proxy) + "Prx", package);
        }
        else
        {
            return getUnqualified("Ice.IObjectPrx", package);
        }
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        string customType = seq->findMetaDataWithPrefix("cs:generic:");
        string serializableType = seq->findMetaDataWithPrefix("cs:serializable:");
        if(!customType.empty())
        {
            ostringstream out;
            out << "global::";
            if(customType == "List" || customType == "LinkedList" || customType == "Queue" || customType == "Stack")
            {
                out << "System.Collections.Generic.";
            }
            out << customType << "<" << typeToString(seq->type(), package, isNullable(seq->type())) << ">";
            return out.str();
        }
        else if(!serializableType.empty())
        {
            return "global::" + serializableType;
        }
        else
        {
            return typeToString(seq->type(), package, isNullable(seq->type())) + "[]";
        }
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    if(d)
    {
        string prefix = "cs:generic:";
        string meta;
        string typeName;
        if(d->findMetaData(prefix, meta))
        {
            typeName = meta.substr(prefix.size());
        }
        else
        {
            typeName = readOnly ? "IReadOnlyDictionary" : "Dictionary";
        }
        return "global::System.Collections.Generic." + typeName + "<" +
            typeToString(d->keyType(), package) + ", " +
            typeToString(d->valueType(), package, isNullable(d->valueType())) + ">";
    }

    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
        return getUnqualified(contained, package);
    }

    return "???";
}

string
Slice::returnValueName(const ParamDeclList& outParams)
{
    for(ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end(); ++i)
    {
        if((*i)->name() == "ReturnValue")
        {
            return "ReturnValue_";
        }
    }
    return "ReturnValue";
}

string
Slice::resultType(const OperationPtr& op, const string& scope, bool dispatch)
{
    ClassDefPtr cls = ClassDefPtr::dynamicCast(op->container());
    list<ParamInfo> outParams = getAllOutParams(op, "", true);
    if(outParams.size() == 0)
    {
        return "void";
    }
    else if(dispatch && op->hasMarshaledResult())
    {
        string name = getNamespace(cls) + "." + interfaceName(cls);
        return getUnqualified(name, scope) + "." + pascalCase(op->name()) + "MarshaledReturnValue";
    }
    else if(outParams.size() > 1)
    {
        return toTupleType(outParams);
    }
    else
    {
        string t = outParams.front().typeStr;
        if(outParams.front().nullable && !outParams.front().tagged)
        {
            t += "?";
        }
        return t;
    }
}

string
Slice::resultTask(const OperationPtr& op, const string& ns, bool dispatch)
{
    string t = resultType(op, ns, dispatch);
    if(t == "void")
    {
        if (dispatch)
        {
            return "global::System.Threading.Tasks.ValueTask";
        }
        else
        {
            return "global::System.Threading.Tasks.Task";
        }
    }
    else if (dispatch)
    {
        return "global::System.Threading.Tasks.ValueTask<" + t + '>';
    }
    else
    {
        return "global::System.Threading.Tasks.Task<" + t + '>';
    }
}

bool
Slice::isClassType(const TypePtr& type)
{
    if(ClassDeclPtr::dynamicCast(type))
    {
        return true;
    }
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    return builtin && builtin->usesClasses();
}

bool
Slice::isProxyType(const TypePtr& type)
{
    if(ProxyPtr::dynamicCast(type))
    {
        return true;
    }
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    return builtin && builtin->kind() == Builtin::KindObjectProxy;
}

bool
Slice::isCollectionType(const TypePtr& type)
{
    return SequencePtr::dynamicCast(type) || DictionaryPtr::dynamicCast(type);
}

bool
Slice::isReferenceType(const TypePtr& type)
{
    return !isValueType(type);
}

bool
Slice::isValueType(const TypePtr& type)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindString:
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            case Builtin::KindValue:
            {
                return false;
            }
            default:
            {
                return true;
            }
        }
    }

    if(EnumPtr::dynamicCast(type))
    {
        return true;
    }

    return StructPtr::dynamicCast(type);
}

Slice::ParamInfo::ParamInfo(const OperationPtr& pOperation,
                            const string& pName,
                            const TypePtr& pType,
                            bool pTagged,
                            int pTag,
                            const string& pPrefix)
{
    this->operation = pOperation;
    this->name = fixId(pPrefix + pName);
    this->type = pType;
    this->typeStr = CsGenerator::typeToString(pType, getNamespace(ClassDefPtr::dynamicCast(operation->container())),
                                              pTagged);
    this->nullable = isNullable(pType);
    this->tagged = pTagged;
    this->tag = pTag;
    this->param = 0;
}

Slice::ParamInfo::ParamInfo(const ParamDeclPtr& pParam, const string& pPrefix)
{
    this->operation = OperationPtr::dynamicCast(pParam->container());
    this->name = fixId(pPrefix + pParam->name());
    this->type = pParam->type();
    this->typeStr = CsGenerator::typeToString(type, getNamespace(ClassDefPtr::dynamicCast(operation->container())),
                                              pParam->tagged());
    this->nullable = isNullable(type);
    this->tagged = pParam->tagged();
    this->tag = pParam->tag();
    this->param = pParam;
}

list<ParamInfo>
Slice::getAllInParams(const OperationPtr& op, const string& prefix)
{
    list<ParamInfo> inParams;
    for(const auto& p : op->inParameters())
    {
        inParams.push_back(ParamInfo(p, prefix));
    }
    return inParams;
}
void
Slice::getInParams(const OperationPtr& op, list<ParamInfo>& requiredParams, list<ParamInfo>& taggedParams,
                   const string&  prefix)
{
    requiredParams.clear();
    taggedParams.clear();
    for(const auto& p : getAllInParams(op, prefix))
    {
        if(p.tagged)
        {
            taggedParams.push_back(p);
        }
        else
        {
            requiredParams.push_back(p);
        }
    }

    //
    // Sort tagged parameters by tag.
    //
    taggedParams.sort([](const auto& lhs, const auto& rhs)
                      {
                          return lhs.tag < rhs.tag;
                      });
}

list<ParamInfo>
Slice::getAllOutParams(const OperationPtr& op, const string& prefix, bool returnTypeIsFirst)
{
    list<ParamInfo> outParams;

    for(const auto& p : op->outParameters())
    {
        outParams.push_back(ParamInfo(p, prefix));
    }

    if(op->returnType())
    {
        auto ret = ParamInfo(op,
                             returnValueName(op->outParameters()),
                             op->returnType(),
                             op->returnIsTagged(),
                             op->returnTag(),
                             prefix);

        if(returnTypeIsFirst)
        {
            outParams.push_front(ret);
        }
        else
        {
            outParams.push_back(ret);
        }
    }

    return outParams;
}

void
Slice::getOutParams(const OperationPtr& op, list<ParamInfo>& requiredParams, list<ParamInfo>& taggedParams,
                    const string& prefix)
{
    requiredParams.clear();
    taggedParams.clear();

    for(const auto& p : getAllOutParams(op, prefix))
    {
        if(p.tagged)
        {
            taggedParams.push_back(p);
        }
        else
        {
            requiredParams.push_back(p);
        }
    }

    //
    // Sort tagged parameters by tag.
    //
    taggedParams.sort([](const auto& lhs, const auto& rhs)
                      {
                          return lhs.tag < rhs.tag;
                      });
}

vector<string>
Slice::getNames(const list<ParamInfo>& params, string prefix)
{
    return getNames(params, [p = move(prefix)](const auto& item)
                            {
                                return p + item.name;
                            });
}

std::string
Slice::toTuple(const list<ParamInfo>& params, const string& paramPrefix)
{
    if(params.size() == 1)
    {
        auto p = params.front();
        return p.param ?  fixId(paramPrefix + p.param->name()) : fixId(paramPrefix + p.name);
    }
    else
    {
        ostringstream os;
        os << "(";
        for(list<ParamInfo>::const_iterator it = params.begin(); it != params.end();)
        {
            os << (it->param ? fixId(paramPrefix + it->param->name()) : fixId(paramPrefix + it->name));
            if(++it != params.end())
            {
                os << ", ";
            }
        }
        os << ")";
        return os.str();
    }
}

std::string
Slice::toTupleType(const list<ParamInfo>& params, const string& prefix)
{
    if(params.size() == 1)
    {
        auto param = params.front();
        return param.typeStr + (param.nullable && !param.tagged ? "?" : "");
    }
    else
    {
        ostringstream os;
        os << "(";
        for(list<ParamInfo>::const_iterator it = params.begin(); it != params.end();)
        {
            os << it->typeStr;
            if(it->nullable && !it->tagged)
            {
                os << "?";
            }
            os << " " << (it->param ? fixId(prefix + it->param->name()) : fixId(prefix + it->name));
            if(++it != params.end())
            {
                os << ", ";
            }
        }
        os << ")";
        return os.str();
    }
}

vector<string>
Slice::getNames(const list<ParamInfo>& params, function<string (const ParamInfo&)> fn)
{
    return mapfn<ParamInfo>(params, move(fn));
}

string
Slice::CsGenerator::outputStreamWriter(const TypePtr& type, const string& scope)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    ostringstream out;
    if(builtin && !builtin->usesClasses() && builtin->kind() != Builtin::KindObjectProxy)
    {
        out << "Ice.OutputStream.IceWriterFrom" << builtinSuffixTable[builtin->kind()];
    }
    else if(DictionaryPtr::dynamicCast(type) || EnumPtr::dynamicCast(type) || SequencePtr::dynamicCast(type))
    {
        out << helperName(type, scope) << ".IceWriter";
    }
    else
    {
        out << typeToString(type, scope) << ".IceWriter";
    }
    return out.str();
}

void
Slice::CsGenerator::writeMarshalCode(Output& out,
                                     const TypePtr& type,
                                     const string& scope,
                                     const string& param,
                                     const string& stream)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    SequencePtr seq = SequencePtr::dynamicCast(type);
    StructPtr st = StructPtr::dynamicCast(type);

    if(builtin || isProxyType(type) || isClassType(type))
    {
        auto kind = builtin ? builtin->kind() : isProxyType(type) ? Builtin::KindObjectProxy : Builtin::KindValue;
        out << nl << stream << ".Write" << builtinSuffixTable[kind] << "(" << param << ");";
    }
    else if(st)
    {
        out << nl << param << ".IceWrite(" << stream << ");";
    }
    else
    {
        out << nl << helperName(type, scope) << ".Write(" << stream << ", " << param << ");";
    }
}

string
Slice::CsGenerator::inputStreamReader(const TypePtr& type, const string& scope)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    ostringstream out;
    if(builtin && !builtin->usesClasses() && builtin->kind() != Builtin::KindObjectProxy)
    {
        out << "Ice.InputStream.IceReaderInto" << builtinSuffixTable[builtin->kind()];
    }
    else if(DictionaryPtr::dynamicCast(type) || EnumPtr::dynamicCast(type) || SequencePtr::dynamicCast(type))
    {
        out << helperName(type, scope) << ".IceReader";
    }
    else
    {
        out << typeToString(type, scope) << ".IceReader";
    }
    return out.str();
}

void
Slice::CsGenerator::writeUnmarshalCode(Output &out,
                                       const TypePtr& type,
                                       const string& scope,
                                       const string& param,
                                       const string& stream)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    StructPtr st = StructPtr::dynamicCast(type);
    SequencePtr seq = SequencePtr::dynamicCast(type);

    out << nl << param << " = ";
    if(isClassType(type))
    {
        out << stream << ".ReadClass<" << typeToString(type, scope) << ">();";
    }
    else if(isProxyType(type))
    {
        out << stream << ".ReadProxy(" << typeToString(type, scope) << ".Factory);";
    }
    else if(builtin)
    {
        out << stream << ".Read" << builtinSuffixTable[builtin->kind()] << "();";
    }
    else if(st)
    {
        out << "new " << getUnqualified(st, scope) << "(" << stream << ");";
    }
    else
    {
        ConstructedPtr constructed = ConstructedPtr::dynamicCast(type);
        assert(constructed);
        out << helperName(type, scope) << ".Read" << constructed->name() << "(" << stream << ");";
    }
}

void
Slice::CsGenerator::writeTaggedMarshalCode(Output &out,
                                           const TypePtr& type,
                                           const string& scope,
                                           const string& param,
                                           int tag,
                                           const string& stream)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    StructPtr st = StructPtr::dynamicCast(type);
    EnumPtr en = EnumPtr::dynamicCast(type);
    SequencePtr seq = SequencePtr::dynamicCast(type);

    if(builtin || isProxyType(type) || isClassType(type))
    {
        auto kind = builtin ? builtin->kind() : isProxyType(type) ? Builtin::KindObjectProxy : Builtin::KindValue;
        out << nl << stream << ".Write" << builtinSuffixTable[kind] << "(" << tag << ", " << param << ");";
    }
    else if(st)
    {
        out << nl << "if(" << param << " is " << typeToString(st, scope);
        out << " && " << stream << ".WriteOptional(" << tag << ", " << getTagFormat(st, scope) << "))";
        out << sb;
        if(st->isVariableLength())
        {
            out << nl << "var pos = " << stream << ".StartSize();";
        }
        else
        {
            out << nl << stream << ".WriteSize(" << st->minWireSize() << ");";
        }
        writeMarshalCode(out, type, scope, param + ".Value", stream);
        if(st->isVariableLength())
        {
            out << nl << stream << ".EndSize(pos);";
        }
        out << eb;
    }
    else if(en)
    {
        out << nl << "if(" << param << " is " << typeToString(en, scope) << ")";
        out << sb;
        out << nl << stream << ".WriteEnum(" << tag << ", (int)" << param << ".Value);";
        out << eb;
    }
    else if(seq)
    {
        writeTaggedSequenceMarshalUnmarshalCode(out, seq, scope, param, tag, true, stream);
    }
    else
    {
        DictionaryPtr d = DictionaryPtr::dynamicCast(type);
        assert(d);
        TypePtr keyType = d->keyType();
        TypePtr valueType = d->valueType();
        out << nl << "if(" << param << " != null && " << stream << ".WriteOptional(" << tag << ", "
            << getTagFormat(d, scope) << "))";
        out << sb;
        if(keyType->isVariableLength() || valueType->isVariableLength())
        {
            out << nl << "var pos = " << stream << ".StartSize();";
        }
        else
        {
            out << nl << stream << ".WriteSize(" << param << " == null ? 1 : " << param << ".Count * "
                << (keyType->minWireSize() + valueType->minWireSize()) << " + (" << param
                << ".Count > 254 ? 5 : 1));";
        }
        writeMarshalCode(out, type, scope, param, stream);
        if(keyType->isVariableLength() || valueType->isVariableLength())
        {
            out << nl << stream << ".EndSize(pos);";
        }
        out << eb;
    }
}

void
Slice::CsGenerator::writeTaggedUnmarshalCode(Output &out,
                                             const TypePtr& type,
                                             const string& scope,
                                             const string& param,
                                             int tag,
                                             const string& customStream)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    StructPtr st = StructPtr::dynamicCast(type);
    EnumPtr en = EnumPtr::dynamicCast(type);
    SequencePtr seq = SequencePtr::dynamicCast(type);

    const string stream = customStream.empty() ? "istr" : customStream;

    if(isClassType(type))
    {
        out << nl << param << " = " << stream << ".ReadClass<" << typeToString(type, scope) << ">(" << tag << ");";
    }
    else if(isProxyType(type))
    {
        out << nl << param << " = " << stream << ".ReadProxy(" << tag << ", " << typeToString(type, scope)
            << ".Factory);";
    }
    else if(builtin)
    {
        out << nl << param << " = " << stream << ".Read" << builtinSuffixTable[builtin->kind()] << "(" << tag << ");";
    }
    else if(st)
    {
        out << nl << "if(" << stream << ".ReadOptional(" << tag << ", " << getTagFormat(st, scope) << "))";
        out << sb;
        out << nl << stream << (st->isVariableLength() ? ".Skip(4);" : ".SkipSize();");
        out << nl << param << " = new " << typeToString(type, scope) << "(" << stream << ");";
        out << eb;
    }
    else if(en)
    {
        out << nl << "if(" << stream << ".ReadOptional(" << tag << ", " << getUnqualified("Ice.OptionalFormat", scope)
            << ".Size))";
        out << sb;
        writeUnmarshalCode(out, type, scope, param, stream);
        out << eb;
    }
    else if(seq)
    {
        writeTaggedSequenceMarshalUnmarshalCode(out, seq, scope, param, tag, false, stream);
    }
    else
    {
        DictionaryPtr d = DictionaryPtr::dynamicCast(type);
        assert(d);
        TypePtr keyType = d->keyType();
        TypePtr valueType = d->valueType();

        out << nl << "if(" << stream << ".ReadOptional(" << tag << ", " << getTagFormat(d, scope) << "))";
        out << sb;
        if(keyType->isVariableLength() || valueType->isVariableLength())
        {
            out << nl << stream << ".Skip(4);";
        }
        else
        {
            out << nl << stream << ".SkipSize();";
        }
        writeUnmarshalCode(out, type, scope, param, stream);
        out << eb;
    }
}

string
Slice::CsGenerator::sequenceMarshalCode(const SequencePtr& seq, const string& scope, const string& param,
                                        const string& stream)
{
    TypePtr type = seq->type();
    StructPtr st = StructPtr::dynamicCast(type);
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    ostringstream out;
    if(seq->hasMetaDataWithPrefix("cs:serializable:"))
    {
        out << stream << ".WriteSerializable(" << param << ")";
    }
    else if(builtin && !builtin->usesClasses() && builtin->kind() != Builtin::KindObjectProxy)
    {
        out << stream << ".Write" << builtinSuffixTable[builtin->kind()] << "Seq(" << param << ")";
    }
    else
    {
        out << stream << ".WriteSeq(" << param << ", " << outputStreamWriter(type, scope) << ")";
    }
    return out.str();
}

string
Slice::CsGenerator::sequenceUnmarshalCode(const SequencePtr& seq, const string& scope, const string& stream)
{
    string generic = seq->findMetaDataWithPrefix("cs:generic:");
    string serializable = seq->findMetaDataWithPrefix("cs:serializable:");

    TypePtr type = seq->type();
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);

    ostringstream out;
    if(!serializable.empty())
    {
        out << "(" << serializable << ") " << stream << ".ReadSerializable()";
    }
    else if(generic.empty())
    {
        if(builtin && !builtin->usesClasses() && builtin->kind() != Builtin::KindObjectProxy)
        {
            out << stream << ".Read" << builtinSuffixTable[builtin->kind()] << "Array()";
        }
        else
        {
            out << stream << ".ReadArray(" << inputStreamReader(type, scope)
                << ", " << type->minWireSize() << ")";
        }
    }
    else
    {
        if(builtin && !builtin->usesClasses() && builtin->kind() != Builtin::KindObjectProxy)
        {
            out << stream << ".Read" << builtinSuffixTable[builtin->kind()] << "Array()";
        }
        else
        {
            out << stream << ".ReadCollection(" << inputStreamReader(type, scope)
                << ", " << type->minWireSize() << ")";
        }

        string reader = generic == "Stack" ? ("System.Linq.Enumerable.Reverse(" + out.str() + ")") : out.str();
        out = ostringstream();
        out << "new " << typeToString(seq, scope) << "(" << reader << ")";
    }
    return out.str();
}

void
Slice::CsGenerator::writeTaggedSequenceMarshalUnmarshalCode(Output& out,
                                                            const SequencePtr& seq,
                                                            const string& scope,
                                                            const string& param,
                                                            int tag,
                                                            bool marshal,
                                                            const string& customStream)
{
    string stream = customStream;
    if(stream.empty())
    {
        stream = marshal ? "ostr" : "istr";
    }

    const TypePtr type = seq->type();
    const string typeS = typeToString(type, scope);
    const string seqS = typeToString(seq, scope);

    string meta;
    const bool isArray = !seq->findMetaData("cs:generic:", meta);
    const string length = isArray ? param + ".Length" : param + ".Count";

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    auto kind = builtin ? builtin->kind() : Builtin::KindObjectProxy;

    if(builtin || proxy)
    {
        if(builtin->usesClasses() || kind == Builtin::KindObjectProxy)
        {
            if(marshal)
            {
                out << nl << "if(" << param << " != null && " << stream << ".WriteOptional(" << tag << ", "
                    << getTagFormat(seq, scope) << "))";
                out << sb;
                out << nl << "var pos = " << stream << ".StartSize();";
                writeMarshalCode(out, seq, scope, param, stream);
                out << nl << stream << ".EndSize(pos);";
                out << eb;
            }
            else
            {
                out << nl << "if(" << stream << ".ReadOptional(" << tag << ", " << getTagFormat(seq, scope) << "))";
                out << sb;
                out << nl << stream << ".Skip(4);";
                string tmp = "tmpVal";
                out << nl << seqS << ' ' << tmp << ';';
                writeUnmarshalCode(out, seq, scope, tmp, stream);
                if(isArray)
                {
                    out << nl << param << " = " << tmp << ";";
                }
                else
                {
                    out << nl << param << " = new " << seqS << "(" << tmp << ");";
                }
                out << eb;
                out << nl << "else";
                out << sb;
                out << nl << param << " = null;";
                out << eb;
            }
        }
        else
        {
            string func = typeS;
            func[0] = static_cast<char>(toupper(static_cast<unsigned char>(typeS[0])));
            const bool isSerializable = seq->findMetaData("cs:serializable:", meta);

            if(marshal)
            {
                if(isSerializable)
                {
                    out << nl << "if(" << param << " != null && " << stream << ".WriteOptional(" << tag
                        << ", " << getUnqualified("Ice.OptionalFormat", scope) << ".VSize))";
                    out << sb;
                    out << nl << stream << ".WriteSerializable(" << param << ");";
                    out << eb;
                }
                else if(isArray)
                {
                    out << nl << stream << ".Write" << func << "Seq(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << "if(" << param << " != null)";
                    out << sb;
                    out << nl << stream << ".Write" << func << "Seq(" << tag << ", " << param << " == null ? 0 : "
                        << param << ".Count, " << param << ");";
                    out << eb;
                }
            }
            else
            {
                out << nl << "if(" << stream << ".ReadOptional(" << tag << ", " << getTagFormat(seq, scope) << "))";
                out << sb;
                if(builtin->isVariableLength())
                {
                    out << nl << stream << ".Skip(4);";
                }
                else if(kind != Builtin::KindByte && kind != Builtin::KindBool)
                {
                    out << nl << stream << ".SkipSize();";
                }
                string tmp = "tmpVal";
                out << nl << seqS << ' ' << tmp << ';';
                writeUnmarshalCode(out, seq, scope, tmp, stream);
                if(isArray)
                {
                    out << nl << param << " = " << tmp << ";";
                }
                else
                {
                    out << nl << param << " = new " << seqS << "(" << tmp << ");";
                }
                out << eb;
                out << nl << "else";
                out << sb;
                out << nl << param << " = null;";
                out << eb;
            }
        }
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(marshal)
        {
            out << nl << "if(" << param << " != null && " << stream << ".WriteOptional(" << tag << ", "
                << getTagFormat(seq, scope) << "))";
            out << sb;
            if(st->isVariableLength())
            {
                out << nl << "var pos = " << stream << ".StartSize();";
            }
            else if(st->minWireSize() > 1)
            {
                out << nl << stream << ".WriteSize(" << param << " == null ? 1 : " << length << " * "
                    << st->minWireSize() << " + (" << length << " > 254 ? 5 : 1));";
            }
            writeMarshalCode(out, seq, scope, param, stream);
            if(st->isVariableLength())
            {
                out << nl << stream << ".EndSize(pos);";
            }
            out << eb;
        }
        else
        {
            out << nl << "if(" << stream << ".ReadOptional(" << tag << ", " << getTagFormat(seq, scope) << "))";
            out << sb;
            if(st->isVariableLength())
            {
                out << nl << stream << ".Skip(4);";
            }
            else if(st->minWireSize() > 1)
            {
                out << nl << stream << ".SkipSize();";
            }
            string tmp = "tmpVal";
            out << nl << seqS << ' ' << tmp << ';';
            writeUnmarshalCode(out, seq, scope, tmp, stream);
            if(isArray)
            {
                out << nl << param << " = " << tmp << ";";
            }
            else
            {
                out << nl << param << " = new " << seqS << "(" << tmp << ");";
            }
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << param << " = null;";
            out << eb;
        }
        return;
    }

    //
    // At this point, all remaining element types have variable size.
    //
    if(marshal)
    {
        out << nl << "if(" << param << " != null && " << stream << ".WriteOptional(" << tag << ", "
            << getTagFormat(seq, scope) << "))";
        out << sb;
        out << nl << "var pos = " << stream << ".StartSize();";
        writeMarshalCode(out, seq, scope, param, stream);
        out << nl << stream << ".EndSize(pos);";
        out << eb;
    }
    else
    {
        out << nl << "if(" << stream << ".ReadOptional(" << tag << ", " << getTagFormat(seq, scope) << "))";
        out << sb;
        out << nl << stream << ".Skip(4);";
        string tmp = "tmpVal";
        out << nl << seqS << ' ' << tmp << ';';
        writeUnmarshalCode(out, seq, scope, tmp, stream);
        if(isArray)
        {
            out << nl << param << " = " << tmp << ";";
        }
        else
        {
            out << nl << param << " = new " << seqS << "(" << tmp << ");";
        }
        out << eb;
        out << nl << "else";
        out << sb;
        out << nl << param << " = null;";
        out << eb;
    }
}

void
Slice::CsGenerator::validateMetaData(const UnitPtr& u)
{
    MetaDataVisitor visitor;
    u->visit(&visitor, true);
}

bool
Slice::CsGenerator::MetaDataVisitor::visitUnitStart(const UnitPtr& p)
{
    //
    // Validate global metadata in the top-level file and all included files.
    //
    StringList files = p->allFiles();
    for(StringList::iterator q = files.begin(); q != files.end(); ++q)
    {
        string file = *q;
        DefinitionContextPtr dc = p->findDefinitionContext(file);
        assert(dc);
        StringList globalMetaData = dc->getMetaData();
        StringList newGlobalMetaData;
        static const string csPrefix = "cs:";
        static const string clrPrefix = "clr:";

        for(StringList::iterator r = globalMetaData.begin(); r != globalMetaData.end(); ++r)
        {
            string& s = *r;
            string oldS = s;

            if(s.find(clrPrefix) == 0)
            {
                s.replace(0, clrPrefix.size(), csPrefix);
            }

            if(s.find(csPrefix) == 0)
            {
                static const string csAttributePrefix = csPrefix + "attribute:";
                static const string csTypeIdNsPrefix = csPrefix + "typeid-namespace:";
                if(!(s.find(csTypeIdNsPrefix) == 0 && s.size() > csTypeIdNsPrefix.size()) &&
                   !(s.find(csAttributePrefix) == 0 && s.size() > csAttributePrefix.size()))
                {
                    dc->warning(InvalidMetaData, file, -1, "ignoring invalid global metadata `" + oldS + "'");
                    continue;
                }
            }
            newGlobalMetaData.push_back(oldS);
        }

        dc->setMetaData(newGlobalMetaData);
    }
    return true;
}

bool
Slice::CsGenerator::MetaDataVisitor::visitModuleStart(const ModulePtr& p)
{
    validate(p);
    return true;
}

void
Slice::CsGenerator::MetaDataVisitor::visitModuleEnd(const ModulePtr&)
{
}

void
Slice::CsGenerator::MetaDataVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    validate(p);
}

bool
Slice::CsGenerator::MetaDataVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    validate(p);
    return true;
}

void
Slice::CsGenerator::MetaDataVisitor::visitClassDefEnd(const ClassDefPtr&)
{
}

bool
Slice::CsGenerator::MetaDataVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    validate(p);
    return true;
}

void
Slice::CsGenerator::MetaDataVisitor::visitExceptionEnd(const ExceptionPtr&)
{
}

bool
Slice::CsGenerator::MetaDataVisitor::visitStructStart(const StructPtr& p)
{
    validate(p);
    return true;
}

void
Slice::CsGenerator::MetaDataVisitor::visitStructEnd(const StructPtr&)
{
}

void
Slice::CsGenerator::MetaDataVisitor::visitOperation(const OperationPtr& p)
{
    validate(p);

    ParamDeclList params = p->parameters();
    for(ParamDeclList::const_iterator i = params.begin(); i != params.end(); ++i)
    {
        visitParamDecl(*i);
    }
}

void
Slice::CsGenerator::MetaDataVisitor::visitParamDecl(const ParamDeclPtr& p)
{
    validate(p);
}

void
Slice::CsGenerator::MetaDataVisitor::visitDataMember(const DataMemberPtr& p)
{
    validate(p);
}

void
Slice::CsGenerator::MetaDataVisitor::visitSequence(const SequencePtr& p)
{
    validate(p);
}

void
Slice::CsGenerator::MetaDataVisitor::visitDictionary(const DictionaryPtr& p)
{
    validate(p);
}

void
Slice::CsGenerator::MetaDataVisitor::visitEnum(const EnumPtr& p)
{
    validate(p);
}

void
Slice::CsGenerator::MetaDataVisitor::visitConst(const ConstPtr& p)
{
    validate(p);
}

void
Slice::CsGenerator::MetaDataVisitor::validate(const ContainedPtr& cont)
{
    const string msg = "ignoring invalid metadata";

    StringList localMetaData = cont->getMetaData();
    StringList newLocalMetaData;

    const UnitPtr ut = cont->unit();
    const DefinitionContextPtr dc = ut->findDefinitionContext(cont->file());
    assert(dc);

    for(StringList::iterator p = localMetaData.begin(); p != localMetaData.end(); ++p)
    {
        string& s = *p;
        string oldS = s;

        const string csPrefix = "cs:";
        const string clrPrefix = "clr:";

        if(s.find(clrPrefix) == 0)
        {
            s.replace(0, clrPrefix.size(), csPrefix);
        }

        if(s.find(csPrefix) == 0)
        {
            SequencePtr seq = SequencePtr::dynamicCast(cont);
            if(seq)
            {
                static const string csGenericPrefix = csPrefix + "generic:";
                if(s.find(csGenericPrefix) == 0)
                {
                    string type = s.substr(csGenericPrefix.size());
                    if(!type.empty())
                    {
                        newLocalMetaData.push_back(s);
                        continue; // Custom type or List<T>
                    }
                }
                static const string csSerializablePrefix = csPrefix + "serializable:";
                if(s.find(csSerializablePrefix) == 0)
                {
                    string meta;
                    if(cont->findMetaData(csPrefix + "generic:", meta))
                    {
                        dc->warning(InvalidMetaData, cont->file(), cont->line(), msg + " `" + meta + "':\n" +
                                    "serialization can only be used with the array mapping for byte sequences");
                        continue;
                    }
                    string type = s.substr(csSerializablePrefix.size());
                    BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
                    if(!type.empty() && builtin && builtin->kind() == Builtin::KindByte)
                    {
                        newLocalMetaData.push_back(s);
                        continue;
                    }
                }
            }
            else if(StructPtr::dynamicCast(cont))
            {
                if(s.substr(csPrefix.size()) == "property")
                {
                    newLocalMetaData.push_back(s);
                    continue;
                }
                if(s.substr(csPrefix.size()) == "readonly")
                {
                    newLocalMetaData.push_back(s);
                    continue;
                }
            }
            else if(ClassDefPtr::dynamicCast(cont))
            {
                if(s.substr(csPrefix.size()) == "property")
                {
                    newLocalMetaData.push_back(s);
                    continue;
                }
            }
            else if(DictionaryPtr::dynamicCast(cont))
            {
                static const string csGenericPrefix = csPrefix + "generic:";
                if(s.find(csGenericPrefix) == 0)
                {
                    string type = s.substr(csGenericPrefix.size());
                    if(type == "SortedDictionary" ||  type == "SortedList")
                    {
                        newLocalMetaData.push_back(s);
                        continue;
                    }
                }
            }
            else if(ModulePtr::dynamicCast(cont))
            {
                static const string csNamespacePrefix = csPrefix + "namespace:";
                if(s.find(csNamespacePrefix) == 0 && s.size() > csNamespacePrefix.size())
                {
                    newLocalMetaData.push_back(s);
                    continue;
                }
            }

            static const string csAttributePrefix = csPrefix + "attribute:";
            if(s.find(csAttributePrefix) == 0 && s.size() > csAttributePrefix.size())
            {
                newLocalMetaData.push_back(s);
                continue;
            }

            dc->warning(InvalidMetaData, cont->file(), cont->line(), msg + " `" + oldS + "'");
            continue;
        }
        newLocalMetaData.push_back(s);
    }

    cont->setMetaData(newLocalMetaData);
}
