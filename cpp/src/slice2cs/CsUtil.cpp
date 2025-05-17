// Copyright (c) ZeroC, Inc.

#include "CsUtil.h"
#include "../Slice/MetadataValidation.h"
#include "../Slice/Util.h"
#include "Ice/StringUtil.h"

#include <algorithm>
#include <cassert>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#    include <direct.h>
#else
#    include <unistd.h>
#endif

using namespace std;
using namespace Slice;
using namespace IceInternal;

string
Slice::CsGenerator::getNamespacePrefix(const ContainedPtr& cont)
{
    // Traverse to the top-level module.
    ContainedPtr p = cont;
    while (!p->isTopLevel())
    {
        p = dynamic_pointer_cast<Contained>(p->container());
        assert(p);
    }
    assert(dynamic_pointer_cast<Module>(p));

    return p->getMetadataArgs("cs:namespace").value_or("");
}

string
Slice::CsGenerator::getNamespace(const ContainedPtr& cont)
{
    assert(!dynamic_pointer_cast<Module>(cont));

    string scope = cont->mappedScope(".").substr(1);
    scope.pop_back(); // Remove the trailing '.' on the scope.
    string prefix = getNamespacePrefix(cont);
    return (prefix.empty() ? scope : prefix + "." + scope);
}

string
Slice::CsGenerator::getUnqualified(const ContainedPtr& p, const string& package)
{
    // If contained is an operation, a field, or an enumerator, we use the enclosing type.
    if (dynamic_pointer_cast<Operation>(p) || dynamic_pointer_cast<DataMember>(p) ||
        dynamic_pointer_cast<Enumerator>(p))
    {
        return getUnqualified(dynamic_pointer_cast<Contained>(p->container()), package) + "." + p->mappedName();
    }
    else
    {
        string pNamespace = getNamespace(p);
        if (pNamespace == package || pNamespace.empty())
        {
            return p->mappedName();
        }
        else
        {
            return "global::" + pNamespace + "." + p->mappedName();
        }
    }
}

string
Slice::CsGenerator::removeEscapePrefix(const string& identifier)
{
    return identifier.find('@') == 0 ? identifier.substr(1) : identifier;
}

string
Slice::CsGenerator::getOptionalFormat(const TypePtr& type)
{
    return "Ice.OptionalFormat." + type->getOptionalFormat();
}

string
Slice::CsGenerator::getStaticId(const TypePtr& type)
{
    BuiltinPtr b = dynamic_pointer_cast<Builtin>(type);
    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);

    assert((b && b->usesClasses()) || cl);

    if (b)
    {
        return "Ice.Value.ice_staticId()";
    }
    else
    {
        return getUnqualified(cl, "") + ".ice_staticId()";
    }
}

string
Slice::CsGenerator::typeToString(const TypePtr& type, const string& package, bool optional)
{
    if (!type)
    {
        return "void";
    }

    if (optional && !isProxyType(type))
    {
        // Proxy types are mapped the same way for optional and non-optional types.
        return typeToString(type, package) + "?";
    }
    // else, just use the regular mapping. null represents "not set".

    static const char* builtinTable[] = {
        "byte",
        "bool",
        "short",
        "int",
        "long",
        "float",
        "double",
        "string",
        "Ice.Object", // not used anymore
        "Ice.ObjectPrx?",
        "Ice.Value?"};

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        if (builtin->kind() == Builtin::KindObject)
        {
            return builtinTable[Builtin::KindValue];
        }
        else
        {
            return builtinTable[builtin->kind()];
        }
    }

    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    if (cl)
    {
        return getUnqualified(cl, package) + "?";
    }

    InterfaceDeclPtr proxy = dynamic_pointer_cast<InterfaceDecl>(type);
    if (proxy)
    {
        return getUnqualified(proxy, package) + "Prx?";
    }

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        if (auto metadata = seq->getMetadataArgs("cs:generic"))
        {
            const string& customType = *metadata;
            if (customType == "List" || customType == "LinkedList" || customType == "Queue" || customType == "Stack")
            {
                return "global::System.Collections.Generic." + customType + "<" + typeToString(seq->type(), package) +
                       ">";
            }
            else
            {
                return "global::" + customType + "<" + typeToString(seq->type(), package) + ">";
            }
        }

        return typeToString(seq->type(), package) + "[]";
    }

    DictionaryPtr d = dynamic_pointer_cast<Dictionary>(type);
    if (d)
    {
        string typeName = d->getMetadataArgs("cs:generic").value_or("Dictionary");
        return "global::System.Collections.Generic." + typeName + "<" + typeToString(d->keyType(), package) + ", " +
               typeToString(d->valueType(), package) + ">";
    }

    ContainedPtr contained = dynamic_pointer_cast<Contained>(type);
    if (contained)
    {
        return getUnqualified(contained, package);
    }

    return "???";
}

string
Slice::CsGenerator::resultStructName(const string& className, const string& opName, bool marshaledResult)
{
    ostringstream s;
    string fixedOpName = removeEscapePrefix(opName);
    s << className << "_" << IceInternal::toUpper(fixedOpName.substr(0, 1)) << fixedOpName.substr(1)
      << (marshaledResult ? "MarshaledResult" : "Result");
    return s.str();
}

string
Slice::CsGenerator::resultType(const OperationPtr& op, const string& package, bool dispatch)
{
    InterfaceDefPtr interface = op->interface();
    if (dispatch && op->hasMarshaledResult())
    {
        return getUnqualified(interface, package) + resultStructName("", op->mappedName(), true);
    }

    string t;
    ParameterList outParams = op->outParameters();
    if (op->returnsAnyValues())
    {
        if (outParams.empty())
        {
            t = typeToString(op->returnType(), package, op->returnIsOptional());
        }
        else if (op->returnType() || outParams.size() > 1)
        {
            t = getUnqualified(interface, package) + resultStructName("", op->mappedName());
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
    if (t.empty())
    {
        return "global::System.Threading.Tasks.Task";
    }
    else
    {
        return "global::System.Threading.Tasks.Task<" + t + '>';
    }
}

bool
Slice::CsGenerator::isValueType(const TypePtr& type)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindString:
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
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
    StructPtr s = dynamic_pointer_cast<Struct>(type);
    if (s)
    {
        if (s->hasMetadata("cs:class"))
        {
            return false;
        }
        DataMemberList dm = s->dataMembers();
        for (const auto& i : dm)
        {
            if (!isValueType(i->type()) || i->defaultValue())
            {
                return false;
            }
        }
        return true;
    }
    if (dynamic_pointer_cast<Enum>(type))
    {
        return true;
    }
    return false;
}

bool
Slice::CsGenerator::isMappedToNonNullableReference(const DataMemberPtr& p)
{
    if (p->optional())
    {
        return false;
    }

    TypePtr type = p->type();

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        return builtin->kind() == Builtin::KindString;
    }

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st)
    {
        return isMappedToClass(st);
    }

    return dynamic_pointer_cast<Sequence>(type) || dynamic_pointer_cast<Dictionary>(type);
}

bool
Slice::CsGenerator::isMappedToRequiredField(const DataMemberPtr& p)
{
    if (p->optional())
    {
        return false;
    }

    // String fields get a "" default.

    TypePtr type = p->type();

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st)
    {
        return isMappedToClass(st);
    }

    return dynamic_pointer_cast<Sequence>(type) || dynamic_pointer_cast<Dictionary>(type);
}

void
Slice::CsGenerator::writeMarshalUnmarshalCode(
    Output& out,
    const TypePtr& type,
    const string& package,
    const string& param,
    bool marshal,
    const string& customStream)
{
    string stream = customStream;
    if (stream.empty())
    {
        stream = marshal ? "ostr" : "istr";
    }

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindByte:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeByte(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readByte()" << ';';
                }
                return;
            }
            case Builtin::KindBool:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeBool(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readBool()" << ';';
                }
                return;
            }
            case Builtin::KindShort:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeShort(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readShort()" << ';';
                }
                return;
            }
            case Builtin::KindInt:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeInt(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readInt()" << ';';
                }
                return;
            }
            case Builtin::KindLong:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeLong(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readLong()" << ';';
                }
                return;
            }
            case Builtin::KindFloat:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeFloat(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readFloat()" << ';';
                }
                return;
            }
            case Builtin::KindDouble:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeDouble(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readDouble()" << ';';
                }
                return;
            }
            case Builtin::KindString:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeString(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readString()" << ';';
                }
                return;
            }
            case Builtin::KindObject:
            case Builtin::KindValue:
            {
                // Handled by isClassType below.
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeProxy(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readProxy()" << ';';
                }
                return;
            }
        }
    }

    InterfaceDeclPtr prx = dynamic_pointer_cast<InterfaceDecl>(type);
    if (prx)
    {
        string typeS = typeToString(type, package);
        string helperName = typeS.substr(0, typeS.size() - 1) + "Helper"; // remove the trailing '?'
        if (marshal)
        {
            out << nl << helperName << ".write(" << stream << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = " << helperName << ".read(" << stream << ");";
        }
        return;
    }

    if (type->isClassType())
    {
        if (marshal)
        {
            out << nl << stream << ".writeValue(" << param << ");";
        }
        else
        {
            out << nl << stream << ".readValue(" << param << ");";
        }
        return;
    }

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st)
    {
        if (marshal)
        {
            out << nl << typeToString(st, package) << ".ice_write(" << stream << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = new " << typeToString(type, package) << "(" << stream << ");";
        }
        return;
    }

    EnumPtr en = dynamic_pointer_cast<Enum>(type);
    if (en)
    {
        if (marshal)
        {
            out << nl << stream << ".writeEnum((int)" << param << ", " << en->maxValue() << ");";
        }
        else
        {
            out << nl << param << " = (" << typeToString(type, package) << ')' << stream << ".readEnum("
                << en->maxValue() << ");";
        }
        return;
    }

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        writeSequenceMarshalUnmarshalCode(out, seq, package, param, marshal, true, stream);
        return;
    }

    assert(dynamic_pointer_cast<Contained>(type));
    string helperName;
    DictionaryPtr d = dynamic_pointer_cast<Dictionary>(type);
    if (d)
    {
        helperName = getUnqualified(d, package) + "Helper";
    }
    else
    {
        helperName = typeToString(type, package) + "Helper";
    }

    if (marshal)
    {
        out << nl << helperName << ".write(" << stream << ", " << param << ");";
    }
    else
    {
        out << nl << param << " = " << helperName << ".read(" << stream << ')' << ';';
    }
}

void
Slice::CsGenerator::writeOptionalMarshalUnmarshalCode(
    Output& out,
    const TypePtr& type,
    const string& scope,
    const string& param,
    int tag,
    bool marshal,
    const string& customStream)
{
    assert(!type->isClassType()); // Optional classes are disallowed by the parser.

    string stream = customStream;
    if (stream.empty())
    {
        stream = marshal ? "ostr" : "istr";
    }

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindByte:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeByte(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readByte(" << tag << ");";
                }
                break;
            }
            case Builtin::KindBool:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeBool(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readBool(" << tag << ");";
                }
                break;
            }
            case Builtin::KindShort:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeShort(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readShort(" << tag << ");";
                }
                break;
            }
            case Builtin::KindInt:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeInt(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readInt(" << tag << ");";
                }
                break;
            }
            case Builtin::KindLong:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeLong(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readLong(" << tag << ");";
                }
                break;
            }
            case Builtin::KindFloat:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeFloat(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readFloat(" << tag << ");";
                }
                break;
            }
            case Builtin::KindDouble:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeDouble(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readDouble(" << tag << ");";
                }
                break;
            }
            case Builtin::KindString:
            {
                if (marshal)
                {
                    out << nl << stream << ".writeString(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readString(" << tag << ");";
                }
                break;
            }
            case Builtin::KindObjectProxy:
            {
                string typeS = typeToString(type, scope);
                if (marshal)
                {
                    out << nl << stream << ".writeProxy(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readProxy(" << tag << ");";
                }
                break;
            }
            case Builtin::KindObject:
            case Builtin::KindValue:
            {
                // Unreachable because we reject all class types at the start of the function.
                assert(false);
            }
        }
        return;
    }

    InterfaceDeclPtr prx = dynamic_pointer_cast<InterfaceDecl>(type);
    if (prx)
    {
        if (marshal)
        {
            out << nl << stream << ".writeProxy(" << tag << ", " << param << ");";
        }
        else
        {
            out << nl << "if (" << stream << ".readOptional(" << tag << ", Ice.OptionalFormat.FSize))";
            out << sb;
            out << nl << stream << ".skip(4);";
            writeMarshalUnmarshalCode(out, type, scope, param, marshal, customStream);
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << param << " = null;";
            out << eb;
        }
        return;
    }

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st)
    {
        if (marshal)
        {
            out << nl << "if (" << param << " is not null && " << stream << ".writeOptional(" << tag << ", "
                << getOptionalFormat(st) << "))";
            out << sb;
            if (st->isVariableLength())
            {
                out << nl << "int pos = " << stream << ".startSize();";
            }
            else
            {
                out << nl << stream << ".writeSize(" << st->minWireSize() << ");";
            }

            writeMarshalUnmarshalCode(
                out,
                type,
                scope,
                isMappedToClass(st) ? param : param + ".Value",
                marshal,
                customStream);
            if (st->isVariableLength())
            {
                out << nl << stream << ".endSize(pos);";
            }
            out << eb;
        }
        else
        {
            out << nl << "if (" << stream << ".readOptional(" << tag << ", " << getOptionalFormat(st) << "))";
            out << sb;
            if (st->isVariableLength())
            {
                out << nl << stream << ".skip(4);";
            }
            else
            {
                out << nl << stream << ".skipSize();";
            }
            string typeS = typeToString(type, scope);
            string tmp = "tmpVal";
            out << nl << typeS << ' ' << tmp << ";";
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

    EnumPtr en = dynamic_pointer_cast<Enum>(type);
    if (en)
    {
        size_t sz = en->enumerators().size();
        if (marshal)
        {
            out << nl << "if (" << param << " is not null)";
            out << sb;
            out << nl << stream << ".writeEnum(" << tag << ", (int)" << param << ".Value, " << sz << ");";
            out << eb;
        }
        else
        {
            out << nl << "if (" << stream << ".readOptional(" << tag << ", Ice.OptionalFormat.Size))";
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

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        writeOptionalSequenceMarshalUnmarshalCode(out, seq, scope, param, tag, marshal, stream);
        return;
    }

    DictionaryPtr d = dynamic_pointer_cast<Dictionary>(type);
    assert(d);
    TypePtr keyType = d->keyType();
    TypePtr valueType = d->valueType();
    if (marshal)
    {
        out << nl << "if (" << param << " is not null && " << stream << ".writeOptional(" << tag << ", "
            << getOptionalFormat(d) << "))";
        out << sb;
        if (keyType->isVariableLength() || valueType->isVariableLength())
        {
            out << nl << "int pos = " << stream << ".startSize();";
        }
        else
        {
            out << nl << stream << ".writeSize(" << param << ".Count * "
                << (keyType->minWireSize() + valueType->minWireSize()) << " + (" << param << ".Count > 254 ? 5 : 1));";
        }
        writeMarshalUnmarshalCode(out, type, scope, param, marshal, customStream);
        if (keyType->isVariableLength() || valueType->isVariableLength())
        {
            out << nl << stream << ".endSize(pos);";
        }
        out << eb;
    }
    else
    {
        out << nl << "if (" << stream << ".readOptional(" << tag << ", " << getOptionalFormat(d) << "))";
        out << sb;
        if (keyType->isVariableLength() || valueType->isVariableLength())
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
Slice::CsGenerator::writeSequenceMarshalUnmarshalCode(
    Output& out,
    const SequencePtr& seq,
    const string& scope,
    const string& param,
    bool marshal,
    bool useHelper,
    const string& customStream)
{
    string stream = customStream;
    if (stream.empty())
    {
        stream = marshal ? "ostr" : "istr";
    }

    ContainedPtr cont = dynamic_pointer_cast<Contained>(seq->container());
    assert(cont);
    if (useHelper)
    {
        string helperName = getUnqualified(seq, scope) + "Helper";
        if (marshal)
        {
            out << nl << helperName << ".write(" << stream << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = " << helperName << ".read(" << stream << ");";
        }
        return;
    }

    TypePtr type = seq->type();
    string typeS = typeToString(type, scope);

    string genericType;
    string addMethod = "Add";
    bool isGeneric = false;
    bool isStack = false;
    bool isList = false;
    bool isLinkedList = false;
    bool isCustom = false;
    if (auto metadata = seq->getMetadataArgs("cs:generic"))
    {
        isGeneric = true;
        genericType = *metadata;
        if (genericType == "LinkedList")
        {
            addMethod = "AddLast";
            isLinkedList = true;
        }
        else if (genericType == "Queue")
        {
            addMethod = "Enqueue";
        }
        else if (genericType == "Stack")
        {
            addMethod = "Push";
            isStack = true;
        }
        else if (genericType == "List")
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

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);

    if (builtin)
    {
        Builtin::Kind kind = builtin->kind();
        switch (kind)
        {
            case Builtin::KindValue:
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            {
                if (marshal)
                {
                    out << nl << "if (" << param << " is null)";
                    out << sb;
                    out << nl << stream << ".writeSize(0);";
                    out << eb;
                    out << nl << "else";
                    out << sb;
                    out << nl << stream << ".writeSize(" << param << '.' << limitID << ");";
                    if (isGeneric && !isList)
                    {
                        if (isStack)
                        {
                            //
                            // If the collection is a stack, write in top-to-bottom order. Stacks
                            // cannot contain Ice.Value.
                            //
                            out << nl << "Ice.ObjectPrx?[] " << param << "_tmp = " << param << ".ToArray();";
                            out << nl << "for (int ix = 0; ix < " << param << "_tmp.Length; ++ix)";
                            out << sb;
                            out << nl << stream << ".writeProxy(" << param << "_tmp[ix]);";
                            out << eb;
                        }
                        else
                        {
                            out << nl << "global::System.Collections.Generic.IEnumerator<" << typeS << "> e = " << param
                                << ".GetEnumerator();";
                            out << nl << "while (e.MoveNext())";
                            out << sb;
                            string func = (kind == Builtin::KindObject || kind == Builtin::KindValue) ? "writeValue"
                                                                                                      : "writeProxy";
                            out << nl << stream << '.' << func << "(e.Current);";
                            out << eb;
                        }
                    }
                    else
                    {
                        out << nl << "for (int ix = 0; ix < " << param << '.' << limitID << "; ++ix)";
                        out << sb;
                        string func =
                            (kind == Builtin::KindObject || kind == Builtin::KindValue) ? "writeValue" : "writeProxy";
                        out << nl << stream << '.' << func << '(' << param << "[ix]);";
                        out << eb;
                    }
                    out << eb;
                }
                else
                {
                    out << nl << "int " << param << "_lenx = " << stream << ".readAndCheckSeqSize("
                        << static_cast<unsigned>(type->minWireSize()) << ");";
                    if (!isStack)
                    {
                        out << nl << param << " = new ";
                    }
                    if ((kind == Builtin::KindObject || kind == Builtin::KindValue))
                    {
                        string patcherName;
                        if (isArray)
                        {
                            patcherName = "Ice.Internal.Patcher.arrayReadValue";
                            out << "Ice.Value?[" << param << "_lenx];";
                        }
                        else if (isCustom)
                        {
                            patcherName = "Ice.Internal.Patcher.customSeqReadValue";
                            out << "global::" << genericType << "<Ice.Value?>();";
                        }
                        else
                        {
                            patcherName = "Ice.Internal.Patcher.listReadValue";
                            out << "global::System.Collections.Generic." << genericType << "<Ice.Value?>(" << param
                                << "_lenx);";
                        }
                        out << nl << "for (int ix = 0; ix < " << param << "_lenx; ++ix)";
                        out << sb;
                        out << nl << stream << ".readValue(" << patcherName << "<Ice.Value>(" << param << ", ix));";
                    }
                    else
                    {
                        if (isStack)
                        {
                            out << nl << "Ice.ObjectPrx?[] " << param << "_tmp = new Ice.ObjectPrx?[" << param
                                << "_lenx];";
                        }
                        else if (isArray)
                        {
                            out << "Ice.ObjectPrx?[" << param << "_lenx];";
                        }
                        else if (isCustom)
                        {
                            out << "global::" << genericType << "<Ice.ObjectPrx?>();";
                        }
                        else
                        {
                            out << "global::System.Collections.Generic." << genericType << "<Ice.ObjectPrx?>(";
                            if (!isLinkedList)
                            {
                                out << param << "_lenx";
                            }
                            out << ");";
                        }

                        out << nl << "for (int ix = 0; ix < " << param << "_lenx; ++ix)";
                        out << sb;
                        if (isArray || isStack)
                        {
                            string v = isArray ? param : param + "_tmp";
                            out << nl << v << "[ix] = " << stream << ".readProxy();";
                        }
                        else
                        {
                            out << nl << "Ice.ObjectPrx? val = " << stream << ".readProxy();";
                            out << nl << param << "." << addMethod << "(val);";
                        }
                    }
                    out << eb;

                    if (isStack)
                    {
                        out << nl << "global::System.Array.Reverse(" << param << "_tmp);";
                        out << nl << param << " = new global::System.Collections.Generic." << genericType << "<"
                            << typeS << ">(" << param << "_tmp);";
                    }
                }
                break;
            }
            default:
            {
                string func = typeS;
                func[0] = static_cast<char>(toupper(static_cast<unsigned char>(typeS[0])));
                if (marshal)
                {
                    // TODO: we have to pass "param!" because the comparison with null suggests it can be null.
                    // Note that the write method called deals accepts nulls too even though its signature is
                    // non-nullable.
                    if (isArray)
                    {
                        out << nl << stream << ".write" << func << "Seq(" << param << ");";
                    }
                    else if (isCustom)
                    {
                        out << nl << stream << ".write" << func << "Seq(" << param << " == null ? 0 : " << param
                            << ".Count, " << param << "!);";
                    }
                    else
                    {
                        assert(isGeneric);
                        out << nl << stream << ".write" << func << "Seq(" << param << " == null ? 0 : " << param
                            << ".Count, " << param << "!);";
                    }
                }
                else
                {
                    if (isArray)
                    {
                        out << nl << param << " = " << stream << ".read" << func << "Seq();";
                    }
                    else if (isCustom)
                    {
                        out << sb;
                        out << nl << param << " = new "
                            << "global::" << genericType << "<" << typeToString(type, scope) << ">();";
                        out << nl << "int szx = " << stream << ".readSize();";
                        out << nl << "for (int ix = 0; ix < szx; ++ix)";
                        out << sb;
                        out << nl << param << ".Add(" << stream << ".read" << func << "());";
                        out << eb;
                        out << eb;
                    }
                    else
                    {
                        assert(isGeneric);
                        out << nl << stream << ".read" << func << "Seq(out " << param << ");";
                    }
                }
                break;
            }
        }
        return;
    }

    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    if (cl)
    {
        if (marshal)
        {
            out << nl << "if (" << param << " is null)";
            out << sb;
            out << nl << stream << ".writeSize(0);";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << stream << ".writeSize(" << param << '.' << limitID << ");";
            if (isGeneric && !isList)
            {
                //
                // Stacks cannot contain class instances, so there is no need to marshal a
                // stack bottom-up here.
                //
                out << nl << "global::System.Collections.Generic.IEnumerator<" << typeS << "> e = " << param
                    << ".GetEnumerator();";
                out << nl << "while (e.MoveNext())";
                out << sb;
                out << nl << stream << ".writeValue(e.Current);";
                out << eb;
            }
            else
            {
                out << nl << "for (int ix = 0; ix < " << param << '.' << limitID << "; ++ix)";
                out << sb;
                out << nl << stream << ".writeValue(" << param << "[ix]);";
                out << eb;
            }
            out << eb;
        }
        else
        {
            out << sb;
            out << nl << "int szx = " << stream << ".readAndCheckSeqSize(" << static_cast<unsigned>(type->minWireSize())
                << ");";
            out << nl << param << " = new ";
            string patcherName;
            if (isArray)
            {
                patcherName = "Ice.Internal.Patcher.arrayReadValue";
                out << toArrayAlloc(typeS + "[]", "szx") << ";";
            }
            else if (isCustom)
            {
                patcherName = "Ice.Internal.Patcher.customSeqReadValue";
                out << "global::" << genericType << "<" << typeS << ">();";
            }
            else
            {
                patcherName = "Ice.Internal.Patcher.listReadValue";
                out << "global::System.Collections.Generic." << genericType << "<" << typeS << ">(szx);";
            }
            out << nl << "for (int ix = 0; ix < szx; ++ix)";
            out << sb;
            // Remove trailing '?'
            string nonNullableTypeS = typeS.substr(0, typeS.size() - 1);
            out << nl << stream << ".readValue(" << patcherName << '<' << nonNullableTypeS << ">(" << param
                << ", ix));";
            out << eb;
            out << eb;
        }
        return;
    }

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st)
    {
        if (marshal)
        {
            out << nl << "if (" << param << " is null)";
            out << sb;
            out << nl << stream << ".writeSize(0);";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << stream << ".writeSize(" << param << '.' << limitID << ");";
            if (isGeneric && !isList)
            {
                //
                // Stacks are marshaled top-down.
                //
                if (isStack)
                {
                    out << nl << typeS << "[] " << param << "_tmp = " << param << ".ToArray();";
                    out << nl << "for (int ix = 0; ix < " << param << "_tmp.Length; ++ix)";
                }
                else
                {
                    out << nl << "global::System.Collections.Generic.IEnumerator<" << typeS << "> e = " << param
                        << ".GetEnumerator();";
                    out << nl << "while (e.MoveNext())";
                }
            }
            else
            {
                out << nl << "for (int ix = 0; ix < " << param << '.' << limitID << "; ++ix)";
            }
            out << sb;

            out << nl << typeS << ".ice_write(" << stream << ", ";

            if (isGeneric && !isList && !isStack)
            {
                out << "e.Current";
            }
            else
            {
                out << param;
                if (isStack)
                {
                    out << "_tmp";
                }
                out << "[ix]";
            }
            out << ");";
            out << eb;
            out << eb;
        }
        else
        {
            out << sb;
            out << nl << "int szx = " << stream << ".readAndCheckSeqSize(" << static_cast<unsigned>(type->minWireSize())
                << ");";
            if (isArray)
            {
                out << nl << param << " = new " << toArrayAlloc(typeS + "[]", "szx") << ";";
            }
            else if (isCustom)
            {
                out << nl << param << " = new global::" << genericType << "<" << typeS << ">();";
            }
            else if (isStack)
            {
                out << nl << typeS << "[] " << param << "_tmp = new " << toArrayAlloc(typeS + "[]", "szx") << ";";
            }
            else
            {
                out << nl << param << " = new global::System.Collections.Generic." << genericType << "<" << typeS
                    << ">(";
                if (!isLinkedList)
                {
                    out << "szx";
                }
                out << ");";
            }
            out << nl << "for (int ix = 0; ix < szx; ++ix)";
            out << sb;
            if (isArray || isStack)
            {
                string v = isArray ? param : param + "_tmp";
                out << nl << v << "[ix] = new " << typeS << "(istr);";
            }
            else
            {
                out << nl << typeS << " val = new " << typeS << "(istr);";
                out << nl << param << "." << addMethod << "(val);";
            }
            out << eb;
            if (isStack)
            {
                out << nl << "global::System.Array.Reverse(" << param << "_tmp);";
                out << nl << param << " = new global::System.Collections.Generic." << genericType << "<" << typeS
                    << ">(" << param << "_tmp);";
            }
            out << eb;
        }
        return;
    }

    EnumPtr en = dynamic_pointer_cast<Enum>(type);
    if (en)
    {
        if (marshal)
        {
            out << nl << "if (" << param << " is null)";
            out << sb;
            out << nl << stream << ".writeSize(0);";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << stream << ".writeSize(" << param << '.' << limitID << ");";
            if (isGeneric && !isList)
            {
                //
                // Stacks are marshaled top-down.
                //
                if (isStack)
                {
                    out << nl << typeS << "[] " << param << "_tmp = " << param << ".ToArray();";
                    out << nl << "for (int ix = 0; ix < " << param << "_tmp.Length; ++ix)";
                    out << sb;
                    out << nl << stream << ".writeEnum((int)" << param << "_tmp[ix], " << en->maxValue() << ");";
                    out << eb;
                }
                else
                {
                    out << nl << "global::System.Collections.Generic.IEnumerator<" << typeS << "> e = " << param
                        << ".GetEnumerator();";
                    out << nl << "while (e.MoveNext())";
                    out << sb;
                    out << nl << stream << ".writeEnum((int)e.Current, " << en->maxValue() << ");";
                    out << eb;
                }
            }
            else
            {
                out << nl << "for (int ix = 0; ix < " << param << '.' << limitID << "; ++ix)";
                out << sb;
                out << nl << stream << ".writeEnum((int)" << param << "[ix], " << en->maxValue() << ");";
                out << eb;
            }
            out << eb;
        }
        else
        {
            out << sb;
            out << nl << "int szx = " << stream << ".readAndCheckSeqSize(" << static_cast<unsigned>(type->minWireSize())
                << ");";
            if (isArray)
            {
                out << nl << param << " = new " << toArrayAlloc(typeS + "[]", "szx") << ";";
            }
            else if (isCustom)
            {
                out << nl << param << " = new global::" << genericType << "<" << typeS << ">();";
            }
            else if (isStack)
            {
                out << nl << typeS << "[] " << param << "_tmp = new " << toArrayAlloc(typeS + "[]", "szx") << ";";
            }
            else
            {
                out << nl << param << " = new global::System.Collections.Generic." << genericType << "<" << typeS
                    << ">(";
                if (!isLinkedList)
                {
                    out << "szx";
                }
                out << ");";
            }
            out << nl << "for (int ix = 0; ix < szx; ++ix)";
            out << sb;
            if (isArray || isStack)
            {
                string v = isArray ? param : param + "_tmp";
                out << nl << v << "[ix] = (" << typeS << ')' << stream << ".readEnum(" << en->maxValue() << ");";
            }
            else
            {
                out << nl << param << "." << addMethod << "((" << typeS << ')' << stream << ".readEnum("
                    << en->maxValue() << "));";
            }
            out << eb;
            if (isStack)
            {
                out << nl << "global::System.Array.Reverse(" << param << "_tmp);";
                out << nl << param << " = new global::System.Collections.Generic." << genericType << "<" << typeS
                    << ">(" << param << "_tmp);";
            }
            out << eb;
        }
        return;
    }

    string helperName;
    if (dynamic_pointer_cast<InterfaceDecl>(type))
    {
        helperName = getUnqualified(dynamic_pointer_cast<InterfaceDecl>(type), scope) + "PrxHelper";
    }
    else
    {
        helperName = getUnqualified(dynamic_pointer_cast<Contained>(type), scope) + "Helper";
    }

    string func;
    if (marshal)
    {
        func = "write";
        out << nl << "if (" << param << " is null)";
        out << sb;
        out << nl << stream << ".writeSize(0);";
        out << eb;
        out << nl << "else";
        out << sb;
        out << nl << stream << ".writeSize(" << param << '.' << limitID << ");";
        if (isGeneric && !isList)
        {
            //
            // Stacks are marshaled top-down.
            //
            if (isStack)
            {
                out << nl << typeS << "[] " << param << "_tmp = " << param << ".ToArray();";
                out << nl << "for (int ix = 0; ix < " << param << "_tmp.Length; ++ix)";
                out << sb;
                out << nl << helperName << '.' << func << '(' << stream << ", " << param << "_tmp[ix]);";
                out << eb;
            }
            else
            {
                out << nl << "global::System.Collections.Generic.IEnumerator<" << typeS << "> e = " << param
                    << ".GetEnumerator();";
                out << nl << "while (e.MoveNext())";
                out << sb;
                out << nl << helperName << '.' << func << '(' << stream << ", e.Current);";
                out << eb;
            }
        }
        else
        {
            out << nl << "for (int ix = 0; ix < " << param << '.' << limitID << "; ++ix)";
            out << sb;
            out << nl << helperName << '.' << func << '(' << stream << ", " << param << "[ix]);";
            out << eb;
        }
        out << eb;
    }
    else
    {
        func = "read";
        out << sb;
        out << nl << "int szx = " << stream << ".readAndCheckSeqSize(" << static_cast<unsigned>(type->minWireSize())
            << ");";
        if (isArray)
        {
            out << nl << param << " = new " << toArrayAlloc(typeS + "[]", "szx") << ";";
        }
        else if (isCustom)
        {
            out << nl << param << " = new global::" << genericType << "<" << typeS << ">();";
        }
        else if (isStack)
        {
            out << nl << typeS << "[] " << param << "_tmp = new " << toArrayAlloc(typeS + "[]", "szx") << ";";
        }
        else
        {
            out << nl << param << " = new global::System.Collections.Generic." << genericType << "<" << typeS << ">();";
        }
        out << nl << "for (int ix = 0; ix < szx; ++ix)";
        out << sb;
        if (isArray || isStack)
        {
            string v = isArray ? param : param + "_tmp";
            out << nl << v << "[ix] = " << helperName << '.' << func << '(' << stream << ");";
        }
        else
        {
            out << nl << param << "." << addMethod << "(" << helperName << '.' << func << '(' << stream << "));";
        }
        out << eb;
        if (isStack)
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
Slice::CsGenerator::writeOptionalSequenceMarshalUnmarshalCode(
    Output& out,
    const SequencePtr& seq,
    const string& scope,
    const string& param,
    int tag,
    bool marshal,
    const string& customStream)
{
    string stream = customStream;
    if (stream.empty())
    {
        stream = marshal ? "ostr" : "istr";
    }

    const TypePtr type = seq->type();
    const string typeS = typeToString(type, scope);
    const string seqS = typeToString(seq, scope);

    const bool isArray = !seq->hasMetadata("cs:generic");
    const string length = isArray ? param + ".Length" : param + ".Count";

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        switch (builtin->kind())
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

                if (marshal)
                {
                    if (isArray)
                    {
                        out << nl << stream << ".write" << func << "Seq(" << tag << ", " << param << ");";
                    }
                    else
                    {
                        out << nl << "if (" << param << " is not null)";
                        out << sb;
                        out << nl << stream << ".write" << func << "Seq(" << tag << ", " << param << ".Count, " << param
                            << ");";
                        out << eb;
                    }
                }
                else
                {
                    if (isArray)
                    {
                        out << nl << param << " = " << stream << ".read" << func << "Seq(" << tag << ");";
                    }
                    else
                    {
                        out << nl << "if (" << stream << ".readOptional(" << tag << ", " << getOptionalFormat(seq)
                            << "))";
                        out << sb;
                        if (builtin->isVariableLength())
                        {
                            out << nl << stream << ".skip(4);";
                        }
                        else if (builtin->kind() != Builtin::KindByte && builtin->kind() != Builtin::KindBool)
                        {
                            out << nl << stream << ".skipSize();";
                        }
                        writeSequenceMarshalUnmarshalCode(out, seq, scope, param, marshal, true, stream);
                        out << eb;
                        out << nl << "else";
                        out << sb;
                        out << nl << param << " = null;";
                        out << eb;
                    }
                }
                break;
            }

            case Builtin::KindValue:
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            {
                if (marshal)
                {
                    out << nl << "if (" << param << " is not null && " << stream << ".writeOptional(" << tag << ", "
                        << getOptionalFormat(seq) << "))";
                    out << sb;
                    out << nl << "int pos = " << stream << ".startSize();";
                    writeSequenceMarshalUnmarshalCode(out, seq, scope, param, marshal, true, stream);
                    out << nl << stream << ".endSize(pos);";
                    out << eb;
                }
                else
                {
                    out << nl << "if (" << stream << ".readOptional(" << tag << ", " << getOptionalFormat(seq) << "))";
                    out << sb;
                    out << nl << stream << ".skip(4);";
                    string tmp = "tmpVal";
                    out << nl << seqS << ' ' << tmp << ';';
                    writeSequenceMarshalUnmarshalCode(out, seq, scope, tmp, marshal, true, stream);
                    out << nl << param << " = " << tmp << ";";
                    out << eb;
                    out << nl << "else";
                    out << sb;
                    out << nl << param << " = null;";
                    out << eb;
                }
                break;
            }
        }

        return;
    }

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st)
    {
        if (marshal)
        {
            out << nl << "if (" << param << " is not null && " << stream << ".writeOptional(" << tag << ", "
                << getOptionalFormat(seq) << "))";
            out << sb;
            if (st->isVariableLength())
            {
                out << nl << "int pos = " << stream << ".startSize();";
            }
            else if (st->minWireSize() > 1)
            {
                out << nl << stream << ".writeSize(" << length << " * " << st->minWireSize() << " + (" << length
                    << " > 254 ? 5 : 1));";
            }
            writeSequenceMarshalUnmarshalCode(out, seq, scope, param, marshal, true, stream);
            if (st->isVariableLength())
            {
                out << nl << stream << ".endSize(pos);";
            }
            out << eb;
        }
        else
        {
            out << nl << "if (" << stream << ".readOptional(" << tag << ", " << getOptionalFormat(seq) << "))";
            out << sb;
            if (st->isVariableLength())
            {
                out << nl << stream << ".skip(4);";
            }
            else if (st->minWireSize() > 1)
            {
                out << nl << stream << ".skipSize();";
            }
            string tmp = "tmpVal";
            out << nl << seqS << ' ' << tmp << ';';
            writeSequenceMarshalUnmarshalCode(out, seq, scope, tmp, marshal, true, stream);
            out << nl << param << " = " << tmp << ";";
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
    if (marshal)
    {
        out << nl << "if (" << param << " is not null && " << stream << ".writeOptional(" << tag << ", "
            << getOptionalFormat(seq) << "))";
        out << sb;
        out << nl << "int pos = " << stream << ".startSize();";
        writeSequenceMarshalUnmarshalCode(out, seq, scope, param, marshal, true, stream);
        out << nl << stream << ".endSize(pos);";
        out << eb;
    }
    else
    {
        out << nl << "if (" << stream << ".readOptional(" << tag << ", " << getOptionalFormat(seq) << "))";
        out << sb;
        out << nl << stream << ".skip(4);";
        string tmp = "tmpVal";
        out << nl << seqS << ' ' << tmp << ';';
        writeSequenceMarshalUnmarshalCode(out, seq, scope, tmp, marshal, true, stream);
        out << nl << param << " = " << tmp << ";";
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
    string::size_type pos = decl.size();
    while (pos > 1 && decl.substr(pos - 2, 2) == "[]")
    {
        pos -= 2;
    }

    ostringstream o;
    o << decl.substr(0, pos) << '[' << sz << ']' << decl.substr(pos + 2);
    return o.str();
}
