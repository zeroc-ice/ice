// Copyright (c) ZeroC, Inc.

#include "IceRpcCsUtil.h"
#include "../Slice/Util.h"
#include "CsUtil.h"

#include <cassert>

using namespace std;
using namespace Slice;
using namespace IceInternal;

namespace
{
    [[nodiscard]] bool hasFixedSizeBuiltinElements(const SequencePtr& seq)
    {
        BuiltinPtr elementTypeBuiltin = dynamic_pointer_cast<Builtin>(seq->type());
        return elementTypeBuiltin && !elementTypeBuiltin->isVariableLength();
    }

    [[nodiscard]] bool isBool(const TypePtr& type)
    {
        BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
        return builtin && builtin->kind() == Builtin::KindBool;
    }

    [[nodiscard]] bool isString(const TypePtr& type)
    {
        BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
        return builtin && builtin->kind() == Builtin::KindString;
    }

    // The TagFormat is the OptionalFormat + the OptimizedVSize enumerator.
    [[nodiscard]] string getTagFormat(const TypePtr& type)
    {
        SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
        string tagFormat = type->getOptionalFormat();
        if (tagFormat == "VSize")
        {
            if (isString(type) || (seq && seq->type()->minWireSize() == 1))
            {
                tagFormat = "OptimizedVSize";
            }
        }
        return tagFormat;
    }

    void castToNestedFieldType(Output& out, const TypePtr& type, const string& ns)
    {
        if (dynamic_pointer_cast<Sequence>(type) || dynamic_pointer_cast<Dictionary>(type))
        {
            out << "(" << Slice::Csharp::csFieldType(type, ns) << ")";
        }
        // else no need to cast
    }
}

bool
Slice::Csharp::isCsValueType(const TypePtr& type)
{
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
                return true;
            default:
                return false;
        }
    }

    return dynamic_pointer_cast<Enum>(type) || dynamic_pointer_cast<Struct>(type) ||
           dynamic_pointer_cast<InterfaceDecl>(type);
}

string
Slice::Csharp::csFieldType(const TypePtr& type, const string& ns, bool optional)
{
    assert(type);

    if (optional && !isProxyType(type))
    {
        // Proxy types are mapped the same way for optional and non-optional types.
        return csFieldType(type, ns) + "?";
    }
    // else, just use the regular mapping.

    static const char* builtinTable[] =
        {"byte", "bool", "short", "int", "long", "float", "double", "string", "IceRpc.ServiceAddress?", "SliceClass?"};

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        return builtinTable[builtin->kind()];
    }

    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    if (cl)
    {
        assert(!optional); // Optional classes are disallowed by the parser.
        return getUnqualified(cl, ns) + "?";
    }

    InterfaceDeclPtr proxy = dynamic_pointer_cast<InterfaceDecl>(type);
    if (proxy)
    {
        return getUnqualified(proxy, ns) + "Proxy?";
    }

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        return "global::System.Collections.Generic.IList<" + csFieldType(seq->type(), ns) + ">";
    }

    DictionaryPtr d = dynamic_pointer_cast<Dictionary>(type);
    if (d)
    {
        return "global::System.Collections.Generic.IDictionary<" + csFieldType(d->keyType(), ns) + ", " +
               csFieldType(d->valueType(), ns) + ">";
    }

    // Struct, Enum
    ContainedPtr contained = dynamic_pointer_cast<Contained>(type);
    if (contained)
    {
        return getUnqualified(contained, ns);
    }

    assert(false);
    return "???";
}

string
Slice::Csharp::csIncomingParamType(const TypePtr& type, const string& ns, bool optional)
{
    if (optional && !isProxyType(type))
    {
        // Proxy types are mapped the same way for optional and non-optional types.
        return csIncomingParamType(type, ns) + "?";
    }

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        if (auto metadata = seq->getMetadataArgs("cs:generic"))
        {
            const string& customType = *metadata;
            if (customType == "List" || customType == "LinkedList" || customType == "Queue" || customType == "Stack")
            {
                return "global::System.Collections.Generic." + customType + "<" + csFieldType(seq->type(), ns) + ">";
            }
            else
            {
                return "global::" + customType + "<" + csFieldType(seq->type(), ns) + ">";
            }
        }
        return csFieldType(seq->type(), ns) + "[]";
    }

    DictionaryPtr d = dynamic_pointer_cast<Dictionary>(type);
    if (d)
    {
        string typeName = d->getMetadataArgs("cs:generic").value_or("Dictionary");
        return "global::System.Collections.Generic." + typeName + "<" + csFieldType(d->keyType(), ns) + ", " +
               csFieldType(d->valueType(), ns) + ">";
    }

    return csFieldType(type, ns);
}

string
Slice::Csharp::csOutgoingParamType(const TypePtr& type, const string& ns, bool optional)
{
    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        bool hasGenericMetadata = seq->hasMetadata("cs:generic");

        BuiltinPtr elementTypeBuiltin = dynamic_pointer_cast<Builtin>(seq->type());
        string elementTypeStr = csFieldType(seq->type(), ns);

        if (elementTypeBuiltin && !elementTypeBuiltin->isVariableLength() && !hasGenericMetadata)
        {
            // If the underlying type is a fixed-size primitive, we map to `ReadOnlyMemory` instead,
            // and the mapping is the same for optional and non-optional types.
            return "global::System.ReadOnlyMemory<" + elementTypeStr + ">";
        }
        else
        {
            return "global::System.Collections.Generic.IEnumerable<" + elementTypeStr + ">" + (optional ? "?" : "");
        }
    }

    DictionaryPtr d = dynamic_pointer_cast<Dictionary>(type);
    if (d)
    {
        return "global::System.Collections.Generic.IEnumerable<global::System.Collections.Generic.KeyValuePair<" +
               csFieldType(d->keyType(), ns) + ", " + csFieldType(d->valueType(), ns) + ">>" + (optional ? "?" : "");
    }

    return csFieldType(type, ns, optional);
}

string
Slice::Csharp::csType(const TypePtr& type, const string& ns, TypeContext context, bool optional)
{
    switch (context)
    {
        case TypeContext::Field:
            return csFieldType(type, ns, optional);
        case TypeContext::IncomingParam:
            return csIncomingParamType(type, ns, optional);
        case TypeContext::OutgoingParam:
            return csOutgoingParamType(type, ns, optional);
        default:
            assert(false);
            return "???";
    }
}

bool
Slice::Csharp::csRequired(const DataMemberPtr& field)
{
    if (field->optional())
    {
        return false;
    }

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(field->type());
    if (builtin)
    {
        return builtin->kind() == Builtin::KindString;
    }

    return dynamic_pointer_cast<Sequence>(field->type()) || dynamic_pointer_cast<Dictionary>(field->type());
}

void
Slice::Csharp::encodeField(
    Output& out,
    const string& fieldName,
    const TypePtr& type,
    const string& ns,
    TypeContext context,
    const string& encoderName)
{
    assert(type);

    static const char* builtinTable[] = {
        "UInt8",
        "Bool",
        "Int16",
        "Int32",
        "Int64",
        "Float",
        "Double",
        "String",
        "NullableServiceAddress",
        "NullableClass"};

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        out << encoderName << ".Encode" << builtinTable[builtin->kind()] << "(" << fieldName << ")";
        return;
    }

    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    if (cl)
    {
        out << encoderName << ".EncodeNullableClass(" << fieldName << ")";
        return;
    }

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st)
    {
        out << fieldName << ".Encode(ref " << encoderName << ")";
        return;
    }

    InterfaceDeclPtr proxy = dynamic_pointer_cast<InterfaceDecl>(type);
    if (proxy)
    {
        out << getUnqualified(proxy, ns) << "ProxySliceEncoderExtensions.EncodeNullable"
            << removeEscapePrefix(proxy->mappedName()) << "Proxy(ref " << encoderName << ", " << fieldName << ")";
        return;
    }

    EnumPtr en = dynamic_pointer_cast<Enum>(type);
    if (en)
    {
        out << getUnqualified(en, ns) << "SliceEncoderExtensions.Encode" << removeEscapePrefix(en->mappedName())
            << "(ref " << encoderName << ", " << fieldName << ")";
        return;
    }

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        if (hasFixedSizeBuiltinElements(seq) && !seq->hasMetadata("cs:generic"))
        {
            if (context == TypeContext::OutgoingParam)
            {
                out << encoderName << ".EncodeSpan(" << fieldName << ".Span)";
            }
            else
            {
                out << encoderName << ".EncodeSequence(" << fieldName << ")";
            }
        }
        else
        {
            TypePtr elementType = seq->type();

            out << encoderName << ".EncodeSequence(";
            out.inc();
            out << nl << fieldName << ",";
            out << nl << "(ref SliceEncoder encoder, " << csFieldType(elementType, ns) << " value) =>";
            out << nl;
            out.inc();
            encodeField(out, "value", elementType, ns, TypeContext::Field, "encoder");
            out.dec();
            out << ")";
            out.dec();
        }
        return;
    }

    DictionaryPtr dict = dynamic_pointer_cast<Dictionary>(type);
    if (dict)
    {
        TypePtr keyType = dict->keyType();
        TypePtr valueType = dict->valueType();
        out << encoderName << ".EncodeDictionary(";
        out.inc();
        out << nl << fieldName << ",";
        out << nl << "(ref SliceEncoder encoder, " << csFieldType(keyType, ns) << " key) =>";
        out << nl;
        out.inc();
        encodeField(out, "key", keyType, ns, TypeContext::Field, "encoder");
        out.dec();
        out << ',';
        out << nl << "(ref SliceEncoder encoder, " << csFieldType(valueType, ns) << " value) =>";
        out << nl;
        out.inc();
        encodeField(out, "value", valueType, ns, TypeContext::Field, "encoder");
        out.dec();
        out << ")";
        out.dec();
        return;
    }

    assert(false);
}

void
Slice::Csharp::encodeOptionalField(
    Output& out,
    int tag,
    const string& fieldName,
    const TypePtr& type,
    const string& ns,
    TypeContext context,
    const string& encoderName)
{
    assert(type);

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    bool readOnlyMemory = seq && hasFixedSizeBuiltinElements(seq) && !seq->hasMetadata("cs:generic") &&
                          context == TypeContext::OutgoingParam;

    string tagFormat = getTagFormat(type);

    if (readOnlyMemory)
    {
        out << nl << "if (" << fieldName << ".Span != null)";
    }
    else if (isCsValueType(type))
    {
        out << nl << "if (" << fieldName << ".HasValue)";
    }
    else
    {
        out << nl << "if (" << fieldName << " is not null)";
    }
    out << sb;

    if (tagFormat == "VSize" && ((seq && !readOnlyMemory) || dynamic_pointer_cast<Dictionary>(type)))
    {
        out << nl << "int count_ = " << fieldName << ".Count();"; // may be slow
        out << sp;
    }

    out << nl << encoderName << ".EncodeTagged(";
    out.inc();
    out << nl << "tag: " << tag << ",";

    if (tagFormat == "VSize")
    {
        // "size" overload
        out << nl << "size: ";
        if (auto st = dynamic_pointer_cast<Struct>(type))
        {
            out << st->minWireSize();
        }
        else if (readOnlyMemory)
        {
            out << encoderName << ".GetSizeLength(" << fieldName << ".Length) + " << seq->type()->minWireSize() << " * "
                << fieldName << ".Length";
        }
        else if (seq)
        {
            out << encoderName << ".GetSizeLength(count_) + " << seq->type()->minWireSize() << " * count_";
        }
        else if (auto dict = dynamic_pointer_cast<Dictionary>(type))
        {
            out << encoderName << ".GetSizeLength(count_) + "
                << (dict->keyType()->minWireSize() + dict->valueType()->minWireSize()) << " * count_";
        }
        else
        {
            // No other VSize.
            assert(false);
        }
        out << ",";
    }
    else
    {
        // TagFormat overload
        out << nl << "TagFormat." << tagFormat << ",";
    }

    out << nl << fieldName << (isCsValueType(type) ? ".Value" : "") << ",";
    out << nl << "(ref SliceEncoder encoder, " << csType(type, ns, context) << " value) => ";
    out.inc();
    encodeField(out, "value", type, ns, context, "encoder");
    out.dec();
    out << ");";
    out.dec();

    out << eb;
    // else, don't encode anything for null.
}

void
Slice::Csharp::decodeField(Output& out, const TypePtr& type, const string& ns, TypeContext)
{
    assert(type);

    static const char* builtinTable[] = {
        "UInt8",
        "Bool",
        "Int16",
        "Int32",
        "Int64",
        "Float",
        "Double",
        "String",
        "NullableServiceAddress",
        "NullableClass<SliceClass>"};

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        out << "decoder.Decode" << builtinTable[builtin->kind()] << "()";
        return;
    }

    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    if (cl)
    {
        out << "decoder.DecodeNullableClass<" << getUnqualified(cl, ns) << ">()";
        return;
    }

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st)
    {
        out << "new " << getUnqualified(st, ns) << "(ref decoder)";
        return;
    }

    InterfaceDeclPtr proxy = dynamic_pointer_cast<InterfaceDecl>(type);
    if (proxy)
    {
        out << getUnqualified(proxy, ns) << "ProxySliceDecoderExtensions.DecodeNullable"
            << removeEscapePrefix(proxy->mappedName()) << "Proxy(ref decoder)";

        return;
    }

    EnumPtr en = dynamic_pointer_cast<Enum>(type);
    if (en)
    {
        out << getUnqualified(en, ns) << "SliceDecoderExtensions.Decode" << removeEscapePrefix(en->mappedName())
            << "(ref decoder)";
        return;
    }

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        bool hasGenericMetadata = seq->hasMetadata("cs:generic");

        // The concrete type we create.
        string csSeq = csIncomingParamType(seq, ns);

        if (hasFixedSizeBuiltinElements(seq))
        {
            if (hasGenericMetadata)
            {
                out << "new " << csSeq << "(";
            }

            out << "decoder.DecodeSequence<" << Slice::Csharp::csFieldType(seq->type(), ns) << ">(";
            if (isBool(seq->type()))
            {
                out << "checkElement: SliceDecoder.CheckBoolValue";
            }
            out << ")";

            if (hasGenericMetadata)
            {
                out << ")";
            }
        }
        else
        {
            out << "decoder.DecodeSequence(";
            out.inc();
            if (hasGenericMetadata)
            {
                out << nl << "sequenceFactory: size => new " << csSeq << "(size),";
            }
            out << nl << "(ref SliceDecoder decoder) => ";
            castToNestedFieldType(out, seq->type(), ns);
            decodeField(out, seq->type(), ns, TypeContext::Field);
            out << ")";
            out.dec();
        }
        return;
    }

    DictionaryPtr dict = dynamic_pointer_cast<Dictionary>(type);
    if (dict)
    {
        TypePtr keyType = dict->keyType();
        TypePtr valueType = dict->valueType();

        // The concrete type we create.
        string csDict = csIncomingParamType(dict, ns);

        out << "decoder.DecodeDictionary(";
        out.inc();
        out << nl << "size => new " << csDict << "(size),";
        out << nl << "(ref SliceDecoder decoder) => ";
        decodeField(out, keyType, ns, TypeContext::Field);
        out << ",";
        out << nl << "(ref SliceDecoder decoder) => ";
        castToNestedFieldType(out, valueType, ns);
        decodeField(out, valueType, ns, TypeContext::Field);
        out << ")";
        out.dec();
        return;
    }

    assert(false);
}

void
Slice::Csharp::decodeOptionalField(Output& out, int tag, const TypePtr& type, const string& ns, TypeContext context)
{
    out << "decoder.DecodeTagged(";
    out.inc();
    out << nl << "tag: " << tag << ",";
    out << nl << "TagFormat." << getTagFormat(type) << ",";
    out << nl << "(ref SliceDecoder decoder) => ";
    // We need to cast to the optional type. This is especially important for value types.
    out << "(" << csType(type, ns, context) << "?)";
    decodeField(out, type, ns, context);
    out << ",";
    out << nl << "useTagEndMarker: " << (context == TypeContext::Field ? "true" : "false");
    out << ")";
    out.dec();
}

std::pair<bool, string>
Slice::Csharp::icerpcLinkFormatter(const string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target)
{
    ostringstream result;

    if (auto builtinTarget = dynamic_pointer_cast<Builtin>(target))
    {
        string typeS = csFieldType(builtinTarget, "");
        if (builtinTarget->kind() == Builtin::KindObjectProxy || builtinTarget->kind() == Builtin::KindValue)
        {
            // Remove trailing '?':
            typeS.pop_back();
            result << "cref=\"" << typeS << "\"";
        }
        else
        {
            // All other builtin types correspond to C# language keywords.
            result << "langword=\"" << typeS << "\"";
        }
    }
    else if (auto contained = dynamic_pointer_cast<Contained>(target))
    {
        string sourceScope = source->mappedScope(".");
        sourceScope.pop_back(); // Remove the trailing '.' ns separator.

        if (dynamic_pointer_cast<Sequence>(contained) || dynamic_pointer_cast<Dictionary>(contained))
        {
            // slice2cs doesn't generate C# types for sequences or dictionaries, so there's nothing to link to.
            // So, we return 'false' to signal this, and just output the sequence or dictionary name.
            return {false, getUnqualified(contained, sourceScope)};
        }

        result << "cref=\"";
        if (auto operationTarget = dynamic_pointer_cast<Operation>(target))
        {
            // link to the method on the proxy interface
            result << getUnqualified(operationTarget->interface(), sourceScope, "", "Proxy") << "."
                   << operationTarget->mappedName() << "Async";
        }
        else if (auto interfaceTarget = dynamic_pointer_cast<InterfaceDecl>(target))
        {
            // link to the proxy interface
            result << getUnqualified(interfaceTarget, sourceScope, "", "Proxy");
        }
        else
        {
            result << getUnqualified(contained, sourceScope);
        }
        result << "\"";
    }
    else
    {
        // Replace "::" by "." in the raw link. This is for the situation where the user passes a Slice type
        // reference but (a) the source Slice file does not include this type and (b) there is no cs:identifier or
        // other identifier renaming.
        string targetS = rawLink;
        // Replace any "::" ns separators with '.'s.
        auto pos = targetS.find("::");
        while (pos != string::npos)
        {
            targetS.replace(pos, 2, ".");
            pos = targetS.find("::", pos);
        }
        // Replace any '#' ns separators with '.'s.
        replace(targetS.begin(), targetS.end(), '#', '.');
        // Remove any leading ns separators.
        if (targetS.find('.') == 0)
        {
            targetS.erase(0, 1);
        }
        result << "cref=\"" << targetS << "\"";
    }

    return {true, result.str()};
}
