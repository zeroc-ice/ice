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
Slice::isNullable(const TypePtr& type)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        if(builtin->kind() == Builtin::KindObject || builtin->kind() == Builtin::KindObjectProxy)
        {
            return true;
        }
    }
    return ClassDeclPtr::dynamicCast(type) ||
        ProxyPtr::dynamicCast(type) ||
        SequencePtr::dynamicCast(type) ||
        DictionaryPtr::dynamicCast(type);
}

namespace
{

string
mangleName(const string& name, int baseTypes)
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
Slice::CsGenerator::getNamespacePrefix(const ContainedPtr& cont)
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
Slice::CsGenerator::getNamespace(const ContainedPtr& cont)
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
Slice::CsGenerator::getUnqualified(const string& type, const string& scope, bool builtin)
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
Slice::CsGenerator::getUnqualified(const ContainedPtr& p, const string& package, const string& prefix,
                                   const string& suffix)
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
Slice::CsGenerator::fixId(const string& name, unsigned int baseTypes)
{
    if(name.empty())
    {
        return name;
    }
    if(name[0] != ':')
    {
        return lookupKwd(name, baseTypes);
    }
    auto ids = splitScopedName(name);
    transform(begin(ids), end(ids), begin(ids), [baseTypes](const std::string& i)
                                                {
                                                    return lookupKwd(i, baseTypes);
                                                });

    stringstream result;
    for(vector<string>::const_iterator j = ids.begin(); j != ids.end(); ++j)
    {
        if(j != ids.begin())
        {
            result << '.';
        }
        result << *j;
    }
    return result.str();
}

string
Slice::CsGenerator::getOptionalFormat(const TypePtr& type, const string& scope)
{
    BuiltinPtr bp = BuiltinPtr::dynamicCast(type);
    string prefix = getUnqualified("Ice.OptionalFormat", scope);
    if(bp)
    {
        switch(bp->kind())
        {
        case Builtin::KindByte:
        case Builtin::KindBool:
        {
            return prefix + ".F1";
        }
        case Builtin::KindShort:
        {
            return prefix + ".F2";
        }
        case Builtin::KindInt:
        case Builtin::KindFloat:
        {
            return prefix + ".F4";
        }
        case Builtin::KindLong:
        case Builtin::KindDouble:
        {
            return prefix + ".F8";
        }
        case Builtin::KindString:
        {
            return prefix + ".VSize";
        }
        case Builtin::KindObject:
        {
            return prefix + ".Class";
        }
        case Builtin::KindObjectProxy:
        {
            return prefix + ".FSize";
        }
        case Builtin::KindValue:
        {
            return prefix + ".Class";
        }
        default:
        {
            assert(false);
            break;
        }
        }
    }

    if(EnumPtr::dynamicCast(type))
    {
        return prefix + ".Size";
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        if(seq->type()->isVariableLength())
        {
            return prefix + ".FSize";
        }
        else
        {
            return prefix + ".VSize";
        }
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    if(d)
    {
        if(d->keyType()->isVariableLength() || d->valueType()->isVariableLength())
        {
            return prefix + ".FSize";
        }
        else
        {
            return prefix + ".VSize";
        }
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(st->isVariableLength())
        {
            return prefix + ".FSize";
        }
        else
        {
            return prefix + ".VSize";
        }
    }

    if(ProxyPtr::dynamicCast(type))
    {
        return prefix + ".FSize";
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    assert(cl);
    return prefix + ".Class";
}

string
Slice::CsGenerator::getStaticId(const TypePtr& type)
{
    BuiltinPtr b = BuiltinPtr::dynamicCast(type);
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);

    assert(isClassType(type));

    if(b)
    {
        return "Ice.Value.ice_staticId()";
    }
    else if(cl->isInterface())
    {
        ContainedPtr cont = ContainedPtr::dynamicCast(cl->container());
        assert(cont);
        return getUnqualified(cont) + "." + cl->name() + "Disp_.ice_staticId()";
    }
    else
    {
        return getUnqualified(cl) + ".ice_staticId()";
    }
}

string
Slice::CsGenerator::typeToString(const TypePtr& type, const string& package, bool optional)
{
    if(!type)
    {
        return "void";
    }

    if(optional)
    {
        return typeToString(type, package) + "?";
    }

    static const char* builtinTable[] =
    {
        "byte",
        "bool",
        "short",
        "int",
        "long",
        "float",
        "double",
        "string",
        "Ice.IObject",
        "Ice.IObjectPrx",
        "Ice.Value"
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
            return getUnqualified("Ice.Value", package);
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
            return getUnqualified(proxy->_class(), package, "", "Prx");
        }
        else
        {
            return getUnqualified("Ice.IObjectPrx", package);
        }
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        string prefix = "cs:generic:";
        string meta;
        if(seq->findMetaData(prefix, meta))
        {
            string customType = meta.substr(prefix.size());
            if(customType == "List" || customType == "LinkedList" || customType == "Queue" || customType == "Stack")
            {
                return "global::System.Collections.Generic." + customType + "<" +
                    typeToString(seq->type(), package, optional) + ">";
            }
            else
            {
                return "global::" + customType + "<" + typeToString(seq->type(), package, optional) + ">";
            }
        }

        prefix = "cs:serializable:";
        if(seq->findMetaData(prefix, meta))
        {
            string customType = meta.substr(prefix.size());
            return "global::" + customType;
        }

        return typeToString(seq->type(), package, optional) + "[]";
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
            typeName = "Dictionary";
        }
        return "global::System.Collections.Generic." + typeName + "<" +
            typeToString(d->keyType(), package, optional) + ", " +
            typeToString(d->valueType(), package, optional) + ">";
    }

    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
        return getUnqualified(contained, package);
    }

    return "???";
}

string
Slice::CsGenerator::resultStructName(const string& className, const string& opName, bool marshaledResult)
{
    ostringstream s;
    s << className
      << "_"
      << IceUtilInternal::toUpper(opName.substr(0, 1))
      << opName.substr(1)
      << (marshaledResult ? "MarshaledResult" : "Result");
    return s.str();
}

string
Slice::CsGenerator::resultType(const OperationPtr& op, const string& package, bool dispatch)
{
    ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container()); // Get the class containing the op.
    if(dispatch && op->hasMarshaledResult())
    {
        return getUnqualified(cl, package, "", resultStructName("", op->name(), true));
    }

    string t;
    ParamDeclList outParams = op->outParameters();
    if(op->returnType() || !outParams.empty())
    {
        if(outParams.empty())
        {
            t = typeToString(op->returnType(), package, op->returnIsOptional());
        }
        else if(op->returnType() || outParams.size() > 1)
        {
            t = getUnqualified(cl, package, "", resultStructName("", op->name()));
        }
        else
        {
            t = typeToString(outParams.front()->type(), package, outParams.front()->optional());
        }
    }
    return t;
}

string
Slice::CsGenerator::taskResultType(const OperationPtr& op, const string& scope, bool dispatch)
{
    string t = resultType(op, scope, dispatch);
    if(t.empty())
    {
        return "global::System.Threading.Tasks.Task";
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
    return builtin && (builtin->kind() == Builtin::KindObject || builtin->kind() == Builtin::KindValue);
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
Slice::isImmutableType(const TypePtr& type)
{
    if(ProxyPtr::dynamicCast(type))
    {
        return true;
    }

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindObject:
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

    if(EnumPtr::dynamicCast(type))
    {
        return true;
    }

    StructPtr s = StructPtr::dynamicCast(type);
    if(s)
    {
        if(s->hasMetaData("cs:class"))
        {
            return false;
        }

        DataMemberList dm = s->dataMembers();
        for(DataMemberList::const_iterator i = dm.begin(); i != dm.end(); ++i)
        {
            if(!isImmutableType((*i)->type()) || (*i)->defaultValueType())
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool
Slice::isValueType(const TypePtr& type)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindObject:
            case Builtin::KindValue:
            case Builtin::KindString:
            case Builtin::KindObjectProxy:
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

    if(EnumPtr::dynamicCast(type))
    {
        return true;
    }

    StructPtr s = StructPtr::dynamicCast(type);
    return s && isImmutableType(s);
}

void
Slice::CsGenerator::writeMarshalUnmarshalCode(Output &out,
                                              const TypePtr& type,
                                              const string& package,
                                              const string& param,
                                              bool marshal,
                                              const string& customStream)
{
    string stream = customStream;
    if(stream.empty())
    {
        stream = marshal ? "ostr" : "istr";
    }

    static const char* builtinTable[] =
    {
        "Byte",
        "Bool",
        "Short",
        "Int",
        "Long",
        "Float",
        "Double",
        "String",
        "Value",
        "Proxy",
        "Value"
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        if(marshal)
        {
            out << nl << stream << ".Write" << builtinTable[builtin->kind()] << "(" << param << ");";
        }
        else
        {
            if(builtin->kind() == Builtin::KindValue || builtin->kind() == Builtin::KindObject)
            {
                out << nl << stream << ".ReadValue(" << param << ");";
            }
            else
            {
                out << nl << param << " = " << stream << ".Read" << builtinTable[builtin->kind()] << "(";
                if(builtin->kind() == Builtin::KindObjectProxy)
                {
                    out << getUnqualified("Ice.IObjectPrx", package) << ".Factory";
                }
                out << ");";
            }
        }
        return;
    }

    ProxyPtr prx = ProxyPtr::dynamicCast(type);
    if(prx)
    {
        ClassDefPtr def = prx->_class()->definition();
        if(!def || def->isAbstract())
        {
            string typeS = typeToString(type, package);
            if(marshal)
            {
                out << nl << stream << ".WriteProxy(" << param << ");";
            }
            else
            {
                out << nl << param << " = " << stream << ".ReadProxy(" << typeS  << ".Factory);";
            }
        }
        else
        {
            if(marshal)
            {
                out << nl << stream << ".WriteProxy(" << param << ");";
            }
            else
            {
                out << nl << param << " = " << stream << ".ReadProxy(" << getUnqualified("Ice.IObjectPrx", package)
                    << ".Factory)" << ';';
            }
        }
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(marshal)
        {
            out << nl << stream << ".WriteValue(" << param << ");";
        }
        else
        {
            out << nl << stream << ".ReadValue(" << param << ");";
        }
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(marshal)
        {
            if(!isImmutableType(st))
            {
                out << nl << typeToString(st, package) << ".ice_write(" << stream << ", " << param << ");";
            }
            else
            {
                out << nl << param << ".ice_writeMembers(" << stream << ");";
            }
        }
        else
        {
            if(!isImmutableType(st))
            {
                out << nl << param << " = " << typeToString(type, package) << ".ice_read(" << stream << ");";
            }
            else
            {
                out << nl << param << ".ice_readMembers(" << stream << ");";
            }
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        if(marshal)
        {
            out << nl << stream << ".WriteEnum((int)" << param << ", " << en->maxValue() << ");";
        }
        else
        {
            out << nl << param << " = (" << typeToString(type, package) << ')' << stream << ".ReadEnum("
                << en->maxValue() << ");";
        }
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        writeSequenceMarshalUnmarshalCode(out, seq, package, param, marshal, true, stream);
        return;
    }

    assert(ConstructedPtr::dynamicCast(type));
    string helperName;
    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    if(d)
    {
        helperName = getUnqualified(d, package, "", "Helper");
    }
    else
    {
        helperName = typeToString(type, package) + "Helper";
    }

    if(marshal)
    {
        out << nl << helperName << ".Write(" << stream << ", " << param << ");";
    }
    else
    {
        out << nl << param << " = " << helperName << ".Read(" << stream << ')' << ';';
    }
}

void
Slice::CsGenerator::writeOptionalMarshalUnmarshalCode(Output &out,
                                                      const TypePtr& type,
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

    static const char* builtinTable[] =
    {
        "Byte",
        "Bool",
        "Short",
        "Int",
        "Long",
        "Float",
        "Double",
        "String",
        "Value",
        "Proxy",
        "Value"
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        if(marshal)
        {
            out << nl << stream << ".Write" << builtinTable[builtin->kind()] << "(" << tag << ", " << param << ");";
        }
        else
        {
            if(builtin->kind() == Builtin::KindValue || builtin->kind() == Builtin::KindObject)
            {
                 out << nl << stream << ".ReadValue(" << tag << ", " << param << ");";
            }
            else
            {
                out << nl << param << " = " << stream << ".Read" << builtinTable[builtin->kind()] << "(" << tag;
                if(builtin->kind() == Builtin::KindObjectProxy)
                {
                    string typeS = typeToString(type, scope);
                    out << ", " << typeS << ".Factory";
                }
                out << ");";
            }
        }
        return;
    }

    ProxyPtr prx = ProxyPtr::dynamicCast(type);
    if(prx)
    {
        if(marshal)
        {
            out << nl << "if(" << param << " != null && " << stream << ".WriteOptional(" << tag
                << ", " << getUnqualified("Ice.OptionalFormat", scope) << ".FSize))";
            out << sb;
            out << nl << "int pos = " << stream << ".StartSize();";
            writeMarshalUnmarshalCode(out, type, scope, param, marshal, customStream);
            out << nl << stream << ".EndSize(pos);";
            out << eb;
        }
        else
        {
            out << nl << "if(" << stream << ".ReadOptional(" << tag << ", " << getUnqualified("Ice.OptionalFormat", scope)
                << ".FSize))";
            out << sb;
            out << nl << stream << ".skip(4);";
            string tmp = "tmpVal";
            string typeS = typeToString(type, scope);
            out << nl << typeS << ' ' << tmp << ';';
            writeMarshalUnmarshalCode(out, type, scope, tmp, marshal, customStream);
            out << nl << param << " = " << tmp << ";";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << param << " = null;";
            out << eb;
        }
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(marshal)
        {
            out << nl << stream << ".WriteValue(" << tag << ", " << param << ");";
        }
        else
        {
            out << nl << stream << ".ReadValue(" << tag << ", " << param << ");";
        }
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(marshal)
        {
            out << nl << "if(" << param;
            if(isImmutableType(st))
            {
                out << " is " << typeToString(st, scope);
            }
            else
            {
                out << " != null";
            }
            out << " && " << stream << ".WriteOptional(" << tag << ", " << getOptionalFormat(st, scope) << "))";
            out << sb;
            if(st->isVariableLength())
            {
                out << nl << "int pos = " << stream << ".StartSize();";
            }
            else
            {
                out << nl << stream << ".WriteSize(" << st->minWireSize() << ");";
            }
            writeMarshalUnmarshalCode(out, type, scope, param + (isImmutableType(st) ? ".Value" : ""), marshal,
                                      customStream);
            if(st->isVariableLength())
            {
                out << nl << stream << ".EndSize(pos);";
            }
            out << eb;
        }
        else
        {
            out << nl << "if(" << stream << ".ReadOptional(" << tag << ", " << getOptionalFormat(st, scope) << "))";
            out << sb;
            if(st->isVariableLength())
            {
                out << nl << stream << ".skip(4);";
            }
            else
            {
                out << nl << stream << ".skipSize();";
            }
            string typeS = typeToString(type, scope);
            string tmp = "tmpVal";
            out << nl << typeS << " tmpVal = default(" << typeS << ");";

            writeMarshalUnmarshalCode(out, type, scope, "tmpVal", marshal, customStream);
            out << nl << param << " = " << tmp << ";";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << param << " = null;";
            out << eb;
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        size_t sz = en->enumerators().size();
        if(marshal)
        {
            out << nl << "if(" << param << " != null)";
            out << sb;
            out << nl << stream << ".WriteEnum(" << tag << ", (int)" << param << ".Value, " << sz << ");";
            out << eb;
        }
        else
        {
            out << nl << "if(" << stream << ".ReadOptional(" << tag << ", " << getUnqualified("Ice.OptionalFormat", scope)
                << ".Size))";
            out << sb;
            string typeS = typeToString(type, scope);
            string tmp = "tmpVal";
            out << nl << typeS << ' ' << tmp << ';';
            writeMarshalUnmarshalCode(out, type, scope, tmp, marshal, customStream);
            out << nl << param << " = " << tmp << ";";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << param << " = null;";
            out << eb;
        }
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        writeOptionalSequenceMarshalUnmarshalCode(out, seq, scope, param, tag, marshal, stream);
        return;
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    assert(d);
    TypePtr keyType = d->keyType();
    TypePtr valueType = d->valueType();
    if(marshal)
    {
        out << nl << "if(" << param << " != null && " << stream << ".WriteOptional(" << tag << ", "
            << getOptionalFormat(d, scope) << "))";
        out << sb;
        if(keyType->isVariableLength() || valueType->isVariableLength())
        {
            out << nl << "int pos = " << stream << ".StartSize();";
        }
        else
        {
            out << nl << stream << ".WriteSize(" << param << " == null ? 1 : " << param << ".Count * "
                << (keyType->minWireSize() + valueType->minWireSize()) << " + (" << param
                << ".Count > 254 ? 5 : 1));";
        }
        writeMarshalUnmarshalCode(out, type, scope, param, marshal, customStream);
        if(keyType->isVariableLength() || valueType->isVariableLength())
        {
            out << nl << stream << ".EndSize(pos);";
        }
        out << eb;
    }
    else
    {
        out << nl << "if(" << stream << ".ReadOptional(" << tag << ", " << getOptionalFormat(d, scope) << "))";
        out << sb;
        if(keyType->isVariableLength() || valueType->isVariableLength())
        {
            out << nl << stream << ".skip(4);";
        }
        else
        {
            out << nl << stream << ".skipSize();";
        }
        string typeS = typeToString(type, scope);
        string tmp = "tmpVal";
        out << nl << typeS << ' ' << tmp << " = new " << typeS << "();";
        writeMarshalUnmarshalCode(out, type, scope, tmp, marshal, customStream);
        out << nl << param << " = " << tmp << ";";
        out << eb;
        out << nl << "else";
        out << sb;
        out << nl << param << " = null;";
        out << eb;
    }
}

void
Slice::CsGenerator::writeSequenceMarshalUnmarshalCode(Output& out,
                                                      const SequencePtr& seq,
                                                      const string& scope,
                                                      const string& param,
                                                      bool marshal,
                                                      bool useHelper,
                                                      const string& customStream)
{
    string stream = customStream;
    if(stream.empty())
    {
        stream = marshal ? "ostr" : "istr";
    }

    ContainedPtr cont = ContainedPtr::dynamicCast(seq->container());
    assert(cont);
    if(useHelper)
    {
        string helperName = getUnqualified(getNamespace(seq) + "." + seq->name() + "Helper", scope);
        if(marshal)
        {
            out << nl << helperName << ".Write(" << stream << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = " << helperName << ".Read(" << stream << ");";
        }
        return;
    }

    TypePtr type = seq->type();
    string typeS = typeToString(type, scope);

    const string genericPrefix = "cs:generic:";
    string genericType;
    string addMethod = "Add";
    const bool isGeneric = seq->findMetaData(genericPrefix, genericType);
    bool isStack = false;
    bool isList = false;
    bool isLinkedList = false;
    bool isCustom = false;
    if(isGeneric)
    {
        genericType = genericType.substr(genericPrefix.size());
        if(genericType == "LinkedList")
        {
            addMethod = "AddLast";
            isLinkedList = true;
        }
        else if(genericType == "Queue")
        {
            addMethod = "Enqueue";
        }
        else if(genericType == "Stack")
        {
            addMethod = "Push";
            isStack = true;
        }
        else if(genericType == "List")
        {
            isList = true;
        }
        else
        {
            isCustom = true;
        }
    }

    const bool isArray = !isGeneric;
    const string limitID = isArray ? "Length" : "Count";

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    Builtin::Kind kind = builtin ? builtin->kind() : Builtin::KindObjectProxy;

    if(builtin || proxy)
    {
        switch(kind)
        {
            case Builtin::KindValue:
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            {
                if(marshal)
                {
                    out << nl << "if(" << param << " == null)";
                    out << sb;
                    out << nl << stream << ".WriteSize(0);";
                    out << eb;
                    out << nl << "else";
                    out << sb;
                    out << nl << stream << ".WriteSize(" << param << '.' << limitID << ");";
                    if(isGeneric && !isList)
                    {
                        if(isStack)
                        {
                            //
                            // If the collection is a stack, write in top-to-bottom order. Stacks
                            // cannot contain Ice.Object.
                            //
                            out << nl << typeS << "[] " << param << "_tmp = " << param
                                << ".ToArray();";
                            out << nl << "for(int ix = 0; ix < " << param << "_tmp.Length; ++ix)";
                            out << sb;
                            out << nl << stream << ".WriteProxy(" << param << "_tmp[ix]);";
                            out << eb;
                        }
                        else
                        {
                            out << nl << "global::System.Collections.Generic.IEnumerator<" << typeS
                                << "> e = " << param << ".GetEnumerator();";
                            out << nl << "while(e.MoveNext())";
                            out << sb;
                            string func = (kind == Builtin::KindObject ||
                                           kind == Builtin::KindValue) ? "WriteValue" : "WriteProxy";
                            out << nl << stream << '.' << func << "(e.Current);";
                            out << eb;
                        }
                    }
                    else
                    {
                        out << nl << "for(int ix = 0; ix < " << param << '.' << limitID << "; ++ix)";
                        out << sb;
                        string func = (kind == Builtin::KindObject ||
                                       kind == Builtin::KindValue) ? "WriteValue" : "WriteProxy";
                        out << nl << stream << '.' << func << '(' << param << "[ix]);";
                        out << eb;
                    }
                    out << eb;
                }
                else
                {
                    out << nl << "int " << param << "_lenx = " << stream << ".ReadAndCheckSeqSize("
                        << static_cast<unsigned>(type->minWireSize()) << ");";
                    if(!isStack)
                    {
                        out << nl << param << " = new ";
                    }
                    if((kind == Builtin::KindObject || kind == Builtin::KindValue))
                    {
                        string patcherName;
                        if(isArray)
                        {
                            patcherName = "global::IceInternal.Patcher.arrayReadValue";
                            out << getUnqualified("Ice.Value", scope) << "[" << param << "_lenx];";
                        }
                        else if(isCustom)
                        {
                            patcherName = "global::IceInternal.Patcher.customSeqReadValue";
                            out << "global::" << genericType << "<" << getUnqualified("Ice.Value", scope) << ">();";
                        }
                        else
                        {
                            patcherName = "global::IceInternal.Patcher.listReadValue";
                            out << "global::System.Collections.Generic." << genericType << "<"
                                << getUnqualified("Ice.Value", scope) << ">(" << param << "_lenx);";
                        }
                        out << nl << "for(int ix = 0; ix < " << param << "_lenx; ++ix)";
                        out << sb;
                        out << nl << stream << ".ReadValue(" << patcherName << "<"
                            << getUnqualified("Ice.Value", scope) << ">(" << param << ", ix));";
                    }
                    else
                    {
                        if(isStack)
                        {
                            out << nl << typeS << "[] " << param << "_tmp = new " << typeS << "[" << param << "_lenx];";
                        }
                        else if(isArray)
                        {
                            out << typeS << "[" << param << "_lenx];";
                        }
                        else if(isCustom)
                        {
                            out << "global::" << genericType << "<" << typeS << ">();";
                        }
                        else
                        {
                            out << "global::System.Collections.Generic." << genericType << "<" << typeS << ">(";
                            if(!isLinkedList)
                            {
                                out << param << "_lenx";
                            }
                            out << ");";
                        }

                        out << nl << "for(int ix = 0; ix < " << param << "_lenx; ++ix)";
                        out << sb;
                        if(isArray || isStack)
                        {
                            string v = isArray ? param : param + "_tmp";
                            out << nl << v << "[ix] = " << stream << ".ReadProxy(" << typeS << ".Factory);";
                        }
                        else
                        {
                            out << nl << param << "." << addMethod << "(" << stream << ".ReadProxy(" << typeS
                                << ".Factory));";
                        }
                    }
                    out << eb;

                    if(isStack)
                    {
                        out << nl << "global::System.Array.Reverse(" << param << "_tmp);";
                        out << nl << param << " = new global::System.Collections.Generic." << genericType << "<"
                            << typeS << ">("
                            << param << "_tmp);";
                    }
                }
                break;
            }
            default:
            {
                string prefix = "cs:serializable:";
                string meta;
                if(seq->findMetaData(prefix, meta))
                {
                    if(marshal)
                    {
                        out << nl << stream << ".WriteSerializable(" << param << ");";
                    }
                    else
                    {
                        out << nl << param << " = (" << typeToString(seq, scope) << ")" << stream
                            << ".ReadSerializable();";
                    }
                    break;
                }

                string func = typeS;
                func[0] = static_cast<char>(toupper(static_cast<unsigned char>(typeS[0])));
                if(marshal)
                {
                    if(isArray)
                    {
                        out << nl << stream << ".Write" << func << "Seq(" << param << ");";
                    }
                    else if(isCustom)
                    {
                        out << nl << stream << ".Write" << func << "Seq(" << param << " == null ? 0 : "
                            << param << ".Count, " << param << ");";
                    }
                    else
                    {
                        assert(isGeneric);
                        out << nl << stream << ".Write" << func << "Seq(" << param << " == null ? 0 : "
                            << param << ".Count, " << param << ");";
                    }
                }
                else
                {
                    if(isArray)
                    {
                        out << nl << param << " = " << stream << ".Read" << func << "Seq();";
                    }
                    else if(isCustom)
                    {
                        out << sb;
                        out << nl << param << " = new " << "global::" << genericType << "<"
                            << typeToString(type, scope) << ">();";
                        out << nl << "int szx = " << stream << ".ReadSize();";
                        out << nl << "for(int ix = 0; ix < szx; ++ix)";
                        out << sb;
                        out << nl << param << ".Add(" << stream << ".Read" << func << "());";
                        out << eb;
                        out << eb;
                    }
                    else
                    {
                        assert(isGeneric);
                        out << nl << stream << ".Read" << func << "Seq(out " << param << ");";
                    }
                }
                break;
            }
        }
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(marshal)
        {
            out << nl << "if(" << param << " == null)";
            out << sb;
            out << nl << stream << ".WriteSize(0);";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << stream << ".WriteSize(" << param << '.' << limitID << ");";
            if(isGeneric && !isList)
            {
                //
                // Stacks cannot contain class instances, so there is no need to marshal a
                // stack bottom-up here.
                //
                out << nl << "global::System.Collections.Generic.IEnumerator<" << typeS
                    << "> e = " << param << ".GetEnumerator();";
                out << nl << "while(e.MoveNext())";
                out << sb;
                out << nl << stream << ".WriteValue(e.Current);";
                out << eb;
            }
            else
            {
                out << nl << "for(int ix = 0; ix < " << param << '.' << limitID << "; ++ix)";
                out << sb;
                out << nl << stream << ".WriteValue(" << param << "[ix]);";
                out << eb;
            }
            out << eb;
        }
        else
        {
            out << sb;
            out << nl << "int szx = " << stream << ".ReadAndCheckSeqSize("
                << static_cast<unsigned>(type->minWireSize()) << ");";
            out << nl << param << " = new ";
            string patcherName;
            if(isArray)
            {
                patcherName = "global::IceInternal.Patcher.arrayReadValue";
                out << toArrayAlloc(typeS + "[]", "szx") << ";";
            }
            else if(isCustom)
            {
                patcherName = "global::IceInternal.Patcher.customSeqReadValue";
                out << "global::" << genericType << "<" << typeS << ">();";
            }
            else
            {
                patcherName = "global::IceInternal.Patcher.listReadValue";
                out << "global::System.Collections.Generic." << genericType << "<" << typeS << ">(szx);";
            }
            out << nl << "for(int ix = 0; ix < szx; ++ix)";
            out << sb;
            string scoped = ContainedPtr::dynamicCast(type)->scoped();
            out << nl << stream << ".ReadValue(" << patcherName << '<' << typeS << ">(" << param << ", ix));";
            out << eb;
            out << eb;
        }
        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(marshal)
        {
            out << nl << "if(" << param << " == null)";
            out << sb;
            out << nl << stream << ".WriteSize(0);";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << stream << ".WriteSize(" << param << '.' << limitID << ");";
            if(isGeneric && !isList)
            {
                //
                // Stacks are marshaled top-down.
                //
                if(isStack)
                {
                    out << nl << typeS << "[] " << param << "_tmp = " << param << ".ToArray();";
                    out << nl << "for(int ix = 0; ix < " << param << "_tmp.Length; ++ix)";
                }
                else
                {
                    out << nl << "global::System.Collections.Generic.IEnumerator<" << typeS
                        << "> e = " << param << ".GetEnumerator();";
                    out << nl << "while(e.MoveNext())";
                }
            }
            else
            {
                out << nl << "for(int ix = 0; ix < " << param << '.' << limitID << "; ++ix)";
            }
            out << sb;
            string call;
            if(isGeneric && !isList && !isStack)
            {
                if(isImmutableType(type))
                {
                    call = "e.Current";
                }
                else
                {
                    call = "(e.Current == null ? new ";
                    call += typeS + "() : e.Current)";
                }
            }
            else
            {
                if(isImmutableType(type))
                {
                    call = param;
                    if(isStack)
                    {
                        call += "_tmp";
                    }
                }
                else
                {
                    call = "(";
                    call += param;
                    if(isStack)
                    {
                        call += "_tmp";
                    }
                    call += "[ix] == null ? new " + typeS + "() : " + param;
                    if(isStack)
                    {
                        call += "_tmp";
                    }
                }
                call += "[ix]";
                if(!isImmutableType(type))
                {
                    call += ")";
                }
            }
            call += ".";
            call += "ice_writeMembers";
            call += "(" + stream + ");";
            out << nl << call;
            out << eb;
            out << eb;
        }
        else
        {
            out << sb;
            out << nl << "int szx = " << stream << ".ReadAndCheckSeqSize("
                << static_cast<unsigned>(type->minWireSize()) << ");";
            if(isArray)
            {
                out << nl << param << " = new " << toArrayAlloc(typeS + "[]", "szx") << ";";
            }
            else if(isCustom)
            {
                out << nl << param << " = new global::" << genericType << "<" << typeS << ">();";
            }
            else if(isStack)
            {
                out << nl << typeS << "[] " << param << "_tmp = new " << toArrayAlloc(typeS + "[]", "szx") << ";";
            }
            else
            {
                out << nl << param << " = new global::System.Collections.Generic." << genericType << "<" << typeS << ">(";
                if(!isLinkedList)
                {
                    out << "szx";
                }
                out << ");";
            }
            out << nl << "for(int ix = 0; ix < szx; ++ix)";
            out << sb;
            if(isArray || isStack)
            {
                string v = isArray ? param : param + "_tmp";
                if(!isImmutableType(st))
                {
                    out << nl << v << "[ix] = new " << typeS << "();";
                }
                out << nl << v << "[ix].ice_readMembers(" << stream << ");";
            }
            else
            {
                out << nl << typeS << " val = new " << typeS << "();";
                out << nl << "val.ice_readMembers(" << stream << ");";
                out << nl << param << "." << addMethod << "(val);";
            }
            out << eb;
            if(isStack)
            {
                out << nl << "global::System.Array.Reverse(" << param << "_tmp);";
                out << nl << param << " = new global::System.Collections.Generic." << genericType << "<" << typeS << ">("
                    << param << "_tmp);";
            }
            out << eb;
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        if(marshal)
        {
            out << nl << "if(" << param << " == null)";
            out << sb;
            out << nl << stream << ".WriteSize(0);";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << stream << ".WriteSize(" << param << '.'<< limitID << ");";
            if(isGeneric && !isList)
            {
                //
                // Stacks are marshaled top-down.
                //
                if(isStack)
                {
                    out << nl << typeS << "[] " << param << "_tmp = " << param << ".ToArray();";
                    out << nl << "for(int ix = 0; ix < " << param << "_tmp.Length; ++ix)";
                    out << sb;
                    out << nl << stream << ".WriteEnum((int)" << param << "_tmp[ix], " << en->maxValue() << ");";
                    out << eb;
                }
                else
                {
                    out << nl << "global::System.Collections.Generic.IEnumerator<" << typeS
                        << "> e = " << param << ".GetEnumerator();";
                    out << nl << "while(e.MoveNext())";
                    out << sb;
                    out << nl << stream << ".WriteEnum((int)e.Current, " << en->maxValue() << ");";
                    out << eb;
                }
            }
            else
            {
                out << nl << "for(int ix = 0; ix < " << param << '.' << limitID << "; ++ix)";
                out << sb;
                out << nl << stream << ".WriteEnum((int)" << param << "[ix], " << en->maxValue() << ");";
                out << eb;
            }
            out << eb;
        }
        else
        {
            out << sb;
            out << nl << "int szx = " << stream << ".ReadAndCheckSeqSize(" <<
                static_cast<unsigned>(type->minWireSize()) << ");";
            if(isArray)
            {
                out << nl << param << " = new " << toArrayAlloc(typeS + "[]", "szx") << ";";
            }
            else if(isCustom)
            {
                out << nl << param << " = new global::" << genericType << "<" << typeS << ">();";
            }
            else if(isStack)
            {
                out << nl << typeS << "[] " << param << "_tmp = new " << toArrayAlloc(typeS + "[]", "szx") << ";";
            }
            else
            {
                out << nl << param << " = new global::System.Collections.Generic." << genericType << "<" << typeS << ">(";
                if(!isLinkedList)
                {
                    out << "szx";
                }
                out << ");";
            }
            out << nl << "for(int ix = 0; ix < szx; ++ix)";
            out << sb;
            if(isArray || isStack)
            {
                string v = isArray ? param : param + "_tmp";
                out << nl << v << "[ix] = (" << typeS << ')' << stream << ".ReadEnum(" << en->maxValue() << ");";
            }
            else
            {
                out << nl << param << "." << addMethod << "((" << typeS << ')' << stream << ".ReadEnum("
                    << en->maxValue() << "));";
            }
            out << eb;
            if(isStack)
            {
                out << nl << "global::System.Array.Reverse(" << param << "_tmp);";
                out << nl << param << " = new global::System.Collections.Generic." << genericType << "<" << typeS << ">("
                    << param << "_tmp);";
            }
            out << eb;
        }
        return;
    }

    string helperName = getUnqualified(ContainedPtr::dynamicCast(type), scope, "", "Helper");

    string func;
    if(marshal)
    {
        func = "Write";
        out << nl << "if(" << param << " == null)";
        out << sb;
        out << nl << stream << ".WriteSize(0);";
        out << eb;
        out << nl << "else";
        out << sb;
        out << nl << stream << ".WriteSize(" << param << '.' << limitID << ");";
        if(isGeneric && !isList)
        {
            //
            // Stacks are marshaled top-down.
            //
            if(isStack)
            {
                out << nl << typeS << "[] " << param << "_tmp = " << param << ".ToArray();";
                out << nl << "for(int ix = 0; ix < " << param << "_tmp.Length; ++ix)";
                out << sb;
                out << nl << helperName << '.' << func << '(' << stream << ", " << param << "_tmp[ix]);";
                out << eb;
            }
            else
            {
                out << nl << "global::System.Collections.Generic.IEnumerator<" << typeS
                    << "> e = " << param << ".GetEnumerator();";
                out << nl << "while(e.MoveNext())";
                out << sb;
                out << nl << helperName << '.' << func << '(' << stream << ", e.Current);";
                out << eb;
            }
        }
        else
        {
            out << nl << "for(int ix = 0; ix < " << param << '.' << limitID << "; ++ix)";
            out << sb;
            out << nl << helperName << '.' << func << '(' << stream << ", " << param << "[ix]);";
            out << eb;
        }
        out << eb;
    }
    else
    {
        func = "Read";
        out << sb;
        out << nl << "int szx = " << stream << ".ReadAndCheckSeqSize("
            << static_cast<unsigned>(type->minWireSize()) << ");";
        if(isArray)
        {
            out << nl << param << " = new " << toArrayAlloc(typeS + "[]", "szx") << ";";
        }
        else if(isCustom)
        {
            out << nl << param << " = new global::" << genericType << "<" << typeS << ">();";
        }
        else if(isStack)
        {
            out << nl << typeS << "[] " << param << "_tmp = new " << toArrayAlloc(typeS + "[]", "szx") << ";";
        }
        else
        {
            out << nl << param << " = new global::System.Collections.Generic." << genericType << "<" << typeS << ">();";
        }
        out << nl << "for(int ix = 0; ix < szx; ++ix)";
        out << sb;
        if(isArray || isStack)
        {
            string v = isArray ? param : param + "_tmp";
            out << nl << v << "[ix] = " << helperName << '.' << func << '(' << stream << ");";
        }
        else
        {
            out << nl << param << "." << addMethod << "(" << helperName << '.' << func << '(' << stream << "));";
        }
        out << eb;
        if(isStack)
        {
            out << nl << "global::System.Array.Reverse(" << param << "_tmp);";
            out << nl << param << " = new global::System.Collections.Generic." << genericType << "<" << typeS << ">("
                << param << "_tmp);";
        }
        out << eb;
    }

    return;
}

void
Slice::CsGenerator::writeOptionalSequenceMarshalUnmarshalCode(Output& out,
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
    Builtin::Kind kind = builtin ? builtin->kind() : Builtin::KindObjectProxy;

    if(builtin || proxy)
    {
        switch(kind)
        {
        case Builtin::KindByte:
        case Builtin::KindBool:
        case Builtin::KindShort:
        case Builtin::KindInt:
        case Builtin::KindFloat:
        case Builtin::KindLong:
        case Builtin::KindDouble:
        case Builtin::KindString:
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
                out << nl << "if(" << stream << ".ReadOptional(" << tag << ", " << getOptionalFormat(seq, scope) << "))";
                out << sb;
                if(builtin->isVariableLength())
                {
                    out << nl << stream << ".skip(4);";
                }
                else if(builtin->kind() != Builtin::KindByte && builtin->kind() != Builtin::KindBool)
                {
                    out << nl << stream << ".skipSize();";
                }
                string tmp = "tmpVal";
                out << nl << seqS << ' ' << tmp << ';';
                writeSequenceMarshalUnmarshalCode(out, seq, scope, tmp, marshal, true, stream);
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
            break;
        }

        case Builtin::KindValue:
        case Builtin::KindObject:
        case Builtin::KindObjectProxy:
        {
            if(marshal)
            {
                out << nl << "if(" << param << " != null && " << stream << ".WriteOptional(" << tag << ", "
                    << getOptionalFormat(seq, scope) << "))";
                out << sb;
                out << nl << "int pos = " << stream << ".StartSize();";
                writeSequenceMarshalUnmarshalCode(out, seq, scope, param, marshal, true, stream);
                out << nl << stream << ".EndSize(pos);";
                out << eb;
            }
            else
            {
                out << nl << "if(" << stream << ".ReadOptional(" << tag << ", " << getOptionalFormat(seq, scope) << "))";
                out << sb;
                out << nl << stream << ".skip(4);";
                string tmp = "tmpVal";
                out << nl << seqS << ' ' << tmp << ';';
                writeSequenceMarshalUnmarshalCode(out, seq, scope, tmp, marshal, true, stream);
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
            break;
        }

        default:
            assert(false);
        }

        return;
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(marshal)
        {
            out << nl << "if(" << param << " != null && " << stream << ".WriteOptional(" << tag << ", "
                << getOptionalFormat(seq, scope) << "))";
            out << sb;
            if(st->isVariableLength())
            {
                out << nl << "int pos = " << stream << ".StartSize();";
            }
            else if(st->minWireSize() > 1)
            {
                out << nl << stream << ".WriteSize(" << param << " == null ? 1 : " << length << " * "
                    << st->minWireSize() << " + (" << length << " > 254 ? 5 : 1));";
            }
            writeSequenceMarshalUnmarshalCode(out, seq, scope, param, marshal, true, stream);
            if(st->isVariableLength())
            {
                out << nl << stream << ".EndSize(pos);";
            }
            out << eb;
        }
        else
        {
            out << nl << "if(" << stream << ".ReadOptional(" << tag << ", " << getOptionalFormat(seq, scope) << "))";
            out << sb;
            if(st->isVariableLength())
            {
                out << nl << stream << ".skip(4);";
            }
            else if(st->minWireSize() > 1)
            {
                out << nl << stream << ".skipSize();";
            }
            string tmp = "tmpVal";
            out << nl << seqS << ' ' << tmp << ';';
            writeSequenceMarshalUnmarshalCode(out, seq, scope, tmp, marshal, true, stream);
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
            << getOptionalFormat(seq, scope) << "))";
        out << sb;
        out << nl << "int pos = " << stream << ".StartSize();";
        writeSequenceMarshalUnmarshalCode(out, seq, scope, param, marshal, true, stream);
        out << nl << stream << ".EndSize(pos);";
        out << eb;
    }
    else
    {
        out << nl << "if(" << stream << ".ReadOptional(" << tag << ", " << getOptionalFormat(seq, scope) << "))";
        out << sb;
        out << nl << stream << ".skip(4);";
        string tmp = "tmpVal";
        out << nl << seqS << ' ' << tmp << ';';
        writeSequenceMarshalUnmarshalCode(out, seq, scope, tmp, marshal, true, stream);
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

string
Slice::CsGenerator::toArrayAlloc(const string& decl, const string& sz)
{
    int count = 0;
    string::size_type pos = decl.size();
    while(pos > 1 && decl.substr(pos - 2, 2) == "[]")
    {
        ++count;
        pos -= 2;
    }
    assert(count > 0);

    ostringstream o;
    o << decl.substr(0, pos) << '[' << sz << ']' << decl.substr(pos + 2);
    return o.str();
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
                    if(type == "LinkedList" || type == "Queue" || type == "Stack")
                    {
                        if(!isClassType(seq->type()))
                        {
                            newLocalMetaData.push_back(s);
                            continue;
                        }
                    }
                    else if(!type.empty())
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
                if(s.substr(csPrefix.size()) == "class")
                {
                    newLocalMetaData.push_back(s);
                    continue;
                }
                if(s.substr(csPrefix.size()) == "property")
                {
                    newLocalMetaData.push_back(s);
                    continue;
                }
                static const string csImplementsPrefix = csPrefix + "implements:";
                if(s.find(csImplementsPrefix) == 0)
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
                static const string csImplementsPrefix = csPrefix + "implements:";
                if(s.find(csImplementsPrefix) == 0)
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
            static const string csTie = csPrefix + "tie";
            if(s.find(csAttributePrefix) == 0 && s.size() > csAttributePrefix.size())
            {
                newLocalMetaData.push_back(s);
                continue;
            }
            else if(s.find(csTie) == 0 && s.size() == csTie.size())
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
