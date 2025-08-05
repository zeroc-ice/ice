// Copyright (c) ZeroC, Inc.

#include "Gen.h"
#include "../Slice/Util.h"
#include "Ice/StringUtil.h"
#include <cstring>

#include <algorithm>
#include <cassert>
#include <limits>

#include <iostream>

using namespace std;
using namespace Slice;
using namespace Slice::Java;
using namespace IceInternal;

namespace
{
    static const char* builtinNameTable[] = {"Byte", "Bool", "Short", "Int", "Long", "Float", "Double", "String"};

    string sliceModeToIceMode(Operation::Mode opMode)
    {
        string mode = "com.zeroc.Ice.OperationMode.";
        switch (opMode)
        {
            case Operation::Normal:
                mode += "Normal";
                break;
            case Operation::Idempotent:
                mode += "Idempotent";
                break;
            default:
                assert(false);
                break;
        }
        return mode;
    }

    string opFormatTypeToString(const OperationPtr& op)
    {
        optional<FormatType> opFormat = op->format();
        if (opFormat)
        {
            switch (*opFormat)
            {
                case CompactFormat:
                    return "com.zeroc.Ice.FormatType.CompactFormat";
                case SlicedFormat:
                    return "com.zeroc.Ice.FormatType.SlicedFormat";
                default:
                    assert(false);
                    return "???";
            }
        }
        else
        {
            return "null";
        }
    }

    // Returns java.util.OptionalXXX.ofYYY depending on the type
    string ofFactory(const TypePtr& type)
    {
        const BuiltinPtr b = dynamic_pointer_cast<Builtin>(type);

        if (b)
        {
            if (b->kind() == Builtin::KindInt)
            {
                return "java.util.OptionalInt.of";
            }
            else if (b->kind() == Builtin::KindLong)
            {
                return "java.util.OptionalLong.of";
            }
            else if (b->kind() == Builtin::KindDouble)
            {
                return "java.util.OptionalDouble.of";
            }
            else if (b->kind() < Builtin::KindString)
            {
                return "java.util.Optional.of";
            }
        }

        return "java.util.Optional.ofNullable";
    }
}

Slice::JavaVisitor::JavaVisitor(const string& dir) : JavaGenerator(dir) {}

Slice::JavaVisitor::~JavaVisitor() = default;

void
Slice::JavaVisitor::writeMarshalUnmarshalCode(
    Output& out,
    const string& package,
    const TypePtr& type,
    bool isOptional,
    bool optionalMapping,
    int32_t tag,
    const string& param,
    bool marshal,
    int& iter,
    const string& customStream,
    const MetadataList& metadata,
    const string& patchParams)
{
    string stream = customStream;
    if (stream.empty())
    {
        stream = marshal ? "ostr" : "istr";
    }

    string typeS = typeToString(type, TypeModeIn, package, metadata);

    if (type->isClassType())
    {
        assert(!isOptional); // Optional classes are disallowed by the parser.
        if (marshal)
        {
            out << nl << stream << ".writeValue(" << param << ");";
        }
        else
        {
            assert(!patchParams.empty());
            out << nl << stream << ".readValue(" << patchParams << ");";
        }
    }
    else if (auto builtin = dynamic_pointer_cast<Builtin>(type))
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
                string s = builtinNameTable[builtin->kind()];
                if (marshal)
                {
                    if (isOptional)
                    {
                        out << nl << stream << ".write" << s << "(" << tag << ", " << param << ");";
                    }
                    else
                    {
                        out << nl << stream << ".write" << s << "(" << param << ");";
                    }
                }
                else
                {
                    if (isOptional)
                    {
                        out << nl << param << " = " << stream << ".read" << s << "(" << tag << ");";
                    }
                    else
                    {
                        out << nl << param << " = " << stream << ".read" << s << "();";
                    }
                }
                return;
            }
            case Builtin::KindValue:
            {
                assert(false); // already handled by 'isClassType' check above.
                return;
            }
            case Builtin::KindObjectProxy:
            {
                if (marshal)
                {
                    if (isOptional)
                    {
                        out << nl << stream << ".writeProxy(" << tag << ", " << param << ");";
                    }
                    else
                    {
                        out << nl << stream << ".writeProxy(" << param << ");";
                    }
                }
                else
                {
                    if (isOptional)
                    {
                        out << nl << param << " = " << stream << ".readProxy(" << tag << ");";
                    }
                    else
                    {
                        out << nl << param << " = " << stream << ".readProxy();";
                    }
                }
                return;
            }
        }
    }
    else if (auto prx = dynamic_pointer_cast<InterfaceDecl>(type))
    {
        if (marshal)
        {
            if (isOptional)
            {
                out << nl << stream << ".writeProxy(" << tag << ", " << param << ");";
            }
            else
            {
                out << nl << stream << ".writeProxy(" << param << ");";
            }
        }
        else
        {
            if (isOptional)
            {
                out << nl << param << " = " << stream << ".readProxy(" << tag << ", " << typeS << "::uncheckedCast);";
            }
            else
            {
                out << nl << param << " = " << typeS << ".uncheckedCast(" << stream << ".readProxy());";
            }
        }
        return;
    }
    else if (auto dict = dynamic_pointer_cast<Dictionary>(type))
    {
        writeDictionaryMarshalUnmarshalCode(
            out,
            package,
            dict,
            param,
            marshal,
            iter,
            true,
            customStream,
            metadata,
            isOptional,
            optionalMapping,
            tag);
    }
    else if (auto seq = dynamic_pointer_cast<Sequence>(type))
    {
        if (isOptional)
        {
            TypePtr elemType = seq->type();
            BuiltinPtr eltBltin = dynamic_pointer_cast<Builtin>(elemType);
            if (!hasTypeMetadata(seq, metadata) && mapsToJavaBuiltinType(eltBltin))
            {
                string bs = builtinNameTable[eltBltin->kind()];
                if (marshal)
                {
                    out << nl << stream << ".write" << bs << "Seq(" << tag << ", " << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".read" << bs << "Seq(" << tag << ");";
                }
                return;
            }
            else if (seq->hasMetadata("java:serializable"))
            {
                if (marshal)
                {
                    out << nl << stream << ".writeSerializable" << spar << tag << param << epar << ";";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readSerializable" << spar << tag << typeS + ".class"
                        << epar << ";";
                }
                return;
            }
            // Check if either 1) No type metadata was applied to this sequence at all or 2) 'java:type' was applied to
            // where the sequence is used (which overrides any metadata on the definition) or 3) 'java:type' was applied
            // to the sequence definition, and there is no metadata overriding it where the sequence is used.
            else if (
                !hasTypeMetadata(seq, metadata) || hasMetadata("java:type", metadata) ||
                (seq->hasMetadata("java:type") && !hasMetadata("java:buffer", metadata)))
            {
                string instanceType, formalType, origInstanceType, origFormalType;
                getSequenceTypes(seq, "", metadata, instanceType, formalType);
                getSequenceTypes(seq, "", MetadataList(), origInstanceType, origFormalType);
                if (formalType == origFormalType && (marshal || instanceType == origInstanceType))
                {
                    string helper = getUnqualified(seq, package) + "Helper";
                    if (marshal)
                    {
                        out << nl << helper << ".write" << spar << stream << tag << param << epar << ";";
                    }
                    else
                    {
                        out << nl << param << " = " << helper << ".read" << spar << stream << tag << epar << ";";
                    }
                }
                return;
            }

            if (marshal)
            {
                out << nl;
                if (optionalMapping)
                {
                    out << "if (" << param << " != null && " << param << ".isPresent() && " << stream
                        << ".writeOptional(" << tag << ", " << getOptionalFormat(type) << "))";
                }
                else
                {
                    out << "if (" << stream << ".writeOptional(" << tag << ", " << getOptionalFormat(type) << "))";
                }
                out << sb;

                if (elemType->isVariableLength())
                {
                    string s = optionalMapping ? param + ".get()" : param;
                    out << nl << "int pos = " << stream << ".startSize();";
                    writeSequenceMarshalUnmarshalCode(out, package, seq, s, true, iter, true, customStream, metadata);
                    out << nl << stream << ".endSize(pos);";
                }
                else
                {
                    const size_t sz = elemType->minWireSize();
                    string s = optionalMapping ? param + ".get()" : param;
                    if (sz > 1)
                    {
                        out << nl << "final int optSize = " << s << " == null ? 0 : ";

                        // Check the local metadata before checking metadata on the sequence definition.
                        if (hasMetadata("java:type", metadata))
                        {
                            out << s << ".size();";
                        }
                        else if (hasMetadata("java:buffer", metadata) || seq->hasMetadata("java:buffer"))
                        {
                            out << s << ".remaining() / " << sz << ";";
                        }
                        else if (seq->hasMetadata("java:type"))
                        {
                            out << s << ".size();";
                        }
                        else
                        {
                            out << s << ".length;";
                        }
                        out << nl << stream << ".writeSize(optSize > 254 ? optSize * " << sz << " + 5 : optSize * "
                            << sz << " + 1);";
                    }
                    writeSequenceMarshalUnmarshalCode(out, package, seq, s, true, iter, true, customStream, metadata);
                }

                out << eb;
            }
            else
            {
                const string s = "optSeq";
                out << nl << "if (" << stream << ".readOptional(" << tag << ", " << getOptionalFormat(type) << "))";
                out << sb;
                out << nl << typeS << ' ' << s << ';';

                if (elemType->isVariableLength())
                {
                    out << nl << stream << ".skip(4);";
                }
                else if (elemType->minWireSize() > 1)
                {
                    out << nl << stream << ".skipSize();";
                }
                writeSequenceMarshalUnmarshalCode(out, package, seq, s, false, iter, true, customStream, metadata);

                out << nl << param << " = java.util.Optional.of(" << s << ");";
                out << eb << " else";
                out << sb;
                out << nl << param << " = java.util.Optional.empty();";
                out << eb;
            }
        }
        else
        {
            writeSequenceMarshalUnmarshalCode(out, package, seq, param, marshal, iter, true, customStream, metadata);
        }
    }
    else
    {
        assert(dynamic_pointer_cast<Contained>(type));
        if (marshal)
        {
            out << nl << typeS << ".ice_write(" << stream << ", ";
            if (isOptional)
            {
                out << tag << ", ";
            }
            out << param << ");";
        }
        else
        {
            if (isOptional)
            {
                out << nl << param << " = " << typeS << ".ice_read(" << stream << ", " << tag << ");";
            }
            else
            {
                out << nl << param << " = " << typeS << ".ice_read(" << stream << ");";
            }
        }
    }
}

void
Slice::JavaVisitor::writeDictionaryMarshalUnmarshalCode(
    Output& out,
    const string& package,
    const DictionaryPtr& dict,
    const string& param,
    bool marshal,
    int& iter,
    bool useHelper,
    const string& customStream,
    const MetadataList& metadata,
    bool isOptional,
    bool optionalMapping,
    int32_t tag)
{
    string stream = customStream;
    if (stream.empty())
    {
        stream = marshal ? "ostr" : "istr";
    }

    // We have to determine whether it's possible to use the type's generated helper class for this marshal/unmarshal
    // task. Since the user may have specified a custom type in metadata, it's possible that the helper class is not
    // compatible and therefore we'll need to generate the code in-line instead.
    //
    // Specifically, there may be "local" metadata (i.e., from a data member or parameter definition) that overrides the
    // original type. We'll compare the mapped types with and without local metadata to determine whether we can use
    // the helper.
    string instanceType, formalType, origInstanceType, origFormalType;
    getDictionaryTypes(dict, "", metadata, instanceType, formalType);
    getDictionaryTypes(dict, "", MetadataList(), origInstanceType, origFormalType);
    if (useHelper && formalType == origFormalType && (marshal || instanceType == origInstanceType))
    {
        // If we can use the helper, it's easy.
        const string helper = getUnqualified(dict, package) + "Helper";
        if (marshal)
        {
            out << nl << helper << ".write" << spar << stream;
            if (isOptional)
            {
                out << tag;
            }
            out << param << epar << ";";
        }
        else
        {
            out << nl << param << " = " << helper << ".read" << spar << stream;
            if (isOptional)
            {
                out << tag;
            }
            out << epar << ";";
        }
        return;
    }

    const TypePtr keyT = dict->keyType();
    const TypePtr valueT = dict->valueType();
    const bool usesVariableLengthTypes = keyT->isVariableLength() || valueT->isVariableLength();
    if (isOptional)
    {
        if (marshal)
        {
            out << nl << "if (";
            if (optionalMapping)
            {
                out << param << " != null && " << param << ".isPresent() && ";
            }
            out << stream << ".writeOptional(" << tag << ", " << getOptionalFormat(dict) << "))" << sb;

            const string d = param + (optionalMapping ? ".get()" : "");
            if (usesVariableLengthTypes)
            {
                out << nl << "int pos = " << stream << ".startSize();";
            }
            else
            {
                const size_t sz = keyT->minWireSize() + valueT->minWireSize();
                out << nl << "final int optSize = " << d << " == null ? 0 : " << d << ".size();";
                out << nl << stream << ".writeSize((optSize * " << sz << ") + (optSize > 254 ? 5 : 1));";
            }
            writeDictionaryMarshalUnmarshalCode(out, package, dict, d, marshal, iter, true, customStream, metadata);
            if (usesVariableLengthTypes)
            {
                out << nl << stream << ".endSize(pos);";
            }

            out << eb;
        }
        else
        {
            const string d = "optDict";
            out << nl << "if (" << stream << ".readOptional(" << tag << ", " << getOptionalFormat(dict) << "))";
            out << sb;
            out << nl << typeToString(dict, TypeModeIn, package, metadata) << ' ' << d << ';';

            if (usesVariableLengthTypes)
            {
                out << nl << stream << ".skip(4);";
            }
            else
            {
                out << nl << stream << ".skipSize();";
            }
            writeDictionaryMarshalUnmarshalCode(out, package, dict, d, marshal, iter, true, customStream, metadata);

            out << nl << param << " = java.util.Optional.of(" << d << ");";
            out << eb << " else";
            out << sb;
            out << nl << param << " = java.util.Optional.empty();";
            out << eb;
        }
    }
    else
    {
        string iterS = std::to_string(iter);
        iter++;

        if (marshal)
        {
            const string keyS = typeToObjectString(keyT, TypeModeIn, package);
            const string valueS = typeToObjectString(valueT, TypeModeIn, package);

            out << nl << "if (" << param << " == null)";
            out << sb;
            out << nl << "ostr.writeSize(0);";
            out << eb << " else";
            out << sb;
            out << nl << "ostr.writeSize(" << param << ".size());";
            out << nl;
            out << "for (java.util.Map.Entry<" << keyS << ", " << valueS << "> e : " << param << ".entrySet())";
            out << sb;
            writeMarshalUnmarshalCode(out, package, keyT, false, false, 0, "e.getKey()", true, iter, customStream);
            writeMarshalUnmarshalCode(out, package, valueT, false, false, 0, "e.getValue()", true, iter, customStream);
            out << eb;
            out << eb;
        }
        else
        {
            string keyS = typeToString(keyT, TypeModeIn, package);
            string valueS = typeToString(valueT, TypeModeIn, package);

            out << nl << param << " = new " << instanceType << "();";
            out << nl << "int sz" << iterS << " = " << stream << ".readSize();";
            out << nl << "for (int i" << iterS << " = 0; i" << iterS << " < sz" << iterS << "; i" << iterS << "++)";
            out << sb;

            if (valueT->isClassType())
            {
                out << nl << "final " << keyS << " key;";
                writeMarshalUnmarshalCode(out, package, keyT, false, false, 0, "key", false, iter, customStream);

                valueS = typeToObjectString(valueT, TypeModeIn, package);
                ostringstream patchParams;
                patchParams << "value -> " << param << ".put(key, value), " << valueS << ".class";
                writeMarshalUnmarshalCode(
                    out,
                    package,
                    valueT,
                    false,
                    false,
                    0,
                    "value",
                    false,
                    iter,
                    customStream,
                    MetadataList(),
                    patchParams.str());
            }
            else
            {
                out << nl << keyS << " key;";
                writeMarshalUnmarshalCode(out, package, keyT, false, false, 0, "key", false, iter, customStream);

                out << nl << valueS << " value;";
                writeMarshalUnmarshalCode(out, package, valueT, false, false, 0, "value", false, iter, customStream);

                out << nl << "" << param << ".put(key, value);";
            }
            out << eb;
        }
    }
}

void
Slice::JavaVisitor::writeSequenceMarshalUnmarshalCode(
    Output& out,
    const string& package,
    const SequencePtr& seq,
    const string& param,
    bool marshal,
    int& iter,
    bool useHelper,
    const string& customStream,
    const MetadataList& metadata)
{
    string stream = customStream;
    if (stream.empty())
    {
        stream = marshal ? "ostr" : "istr";
    }

    string typeS = typeToString(seq, TypeModeIn, package);

    // Check for the serializable metadata to get rid of this case first.
    if (seq->hasMetadata("java:serializable"))
    {
        if (marshal)
        {
            out << nl << stream << ".writeSerializable(" << param << ");";
        }
        else
        {
            out << nl << param << " = " << stream << ".readSerializable(" << typeS << ".class);";
        }
        return;
    }

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(seq->type());
    static const string bytebuffer = "java:buffer";
    if ((seq->hasMetadata(bytebuffer) || hasMetadata(bytebuffer, metadata)) && !hasMetadata("java:type", metadata))
    {
        if (marshal)
        {
            out << nl << stream << ".write" << builtinNameTable[builtin->kind()] << "Buffer(" << param << ");";
        }
        else
        {
            out << nl << param << " = " << stream << ".read" << builtinNameTable[builtin->kind()] << "Buffer();";
        }
        return;
    }

    if (!hasTypeMetadata(seq, metadata) && mapsToJavaBuiltinType(builtin))
    {
        if (marshal)
        {
            out << nl << stream << ".write" << builtinNameTable[builtin->kind()] << "Seq(" << param << ");";
        }
        else
        {
            out << nl << param << " = " << stream << ".read" << builtinNameTable[builtin->kind()] << "Seq();";
        }
        return;
    }

    //
    // We have to determine whether it's possible to use the
    // type's generated helper class for this marshal/unmarshal
    // task. Since the user may have specified a custom type in
    // metadata, it's possible that the helper class is not
    // compatible and therefore we'll need to generate the code
    // in-line instead.
    //
    // Specifically, there may be "local" metadata (i.e., from
    // a data member or parameter definition) that overrides the
    // original type. We'll compare the mapped types with and
    // without local metadata to determine whether we can use
    // the helper.
    //
    string instanceType, formalType, origInstanceType, origFormalType;
    bool customType = getSequenceTypes(seq, "", metadata, instanceType, formalType);
    getSequenceTypes(seq, "", MetadataList(), origInstanceType, origFormalType);
    if (useHelper && formalType == origFormalType && (marshal || instanceType == origInstanceType))
    {
        //
        // If we can use the helper, it's easy.
        //
        string helper = getUnqualified(seq, package) + "Helper";
        if (marshal)
        {
            out << nl << helper << ".write" << spar << stream << param << epar << ";";
        }
        else
        {
            out << nl << param << " = " << helper << ".read" << spar << stream << epar << ";";
        }
        return;
    }

    //
    // Determine sequence depth.
    //
    int depth = 0;
    TypePtr origContent = seq->type();
    SequencePtr s = dynamic_pointer_cast<Sequence>(origContent);
    while (s)
    {
        //
        // Stop if the inner sequence type has a custom, or serializable type.
        //
        if (hasTypeMetadata(s))
        {
            break;
        }
        depth++;
        origContent = s->type();
        s = dynamic_pointer_cast<Sequence>(origContent);
    }
    string origContentS = typeToString(origContent, TypeModeIn, package);

    TypePtr type = seq->type();

    if (customType)
    {
        //
        // Marshal/unmarshal a custom sequence type.
        //
        BuiltinPtr b = dynamic_pointer_cast<Builtin>(type);
        typeS = getUnqualified(seq, package);
        ostringstream o;
        o << origContentS;
        int d = depth;
        while (d--)
        {
            o << "[]";
        }
        string cont = o.str();
        if (marshal)
        {
            out << nl << "if (" << param << " == null)";
            out << sb;
            out << nl << stream << ".writeSize(0);";
            out << eb << " else";
            out << sb;
            out << nl << stream << ".writeSize(" << param << ".size());";
            string ctypeS = typeToString(type, TypeModeIn, package);
            out << nl << "for (" << ctypeS << " elem : " << param << ')';
            out << sb;
            writeMarshalUnmarshalCode(out, package, type, false, false, 0, "elem", true, iter, customStream);
            out << eb;
            out << eb; // else
        }
        else
        {
            out << nl << param << " = new " << instanceType << "();";
            out << nl << "final int len" << iter << " = " << stream << ".readAndCheckSeqSize(" << type->minWireSize()
                << ");";
            out << nl << "for (int i" << iter << " = 0; i" << iter << " < len" << iter << "; i" << iter << "++)";
            out << sb;
            if (type->isClassType())
            {
                //
                // Add a null value to the list as a placeholder for the element.
                //
                out << nl << param << ".add(null);";
                ostringstream patchParams;
                out << nl << "final int fi" << iter << " = i" << iter << ";";
                patchParams << "value -> " << param << ".set(fi" << iter << ", value), " << origContentS << ".class";

                writeMarshalUnmarshalCode(
                    out,
                    package,
                    type,
                    false,
                    false,
                    0,
                    "elem",
                    false,
                    iter,
                    customStream,
                    MetadataList(),
                    patchParams.str());
            }
            else
            {
                out << nl << cont << " elem;";
                writeMarshalUnmarshalCode(out, package, type, false, false, 0, "elem", false, iter, customStream);
                out << nl << param << ".add(elem);";
            }
            out << eb;
            iter++;
        }
    }
    else
    {
        BuiltinPtr b = dynamic_pointer_cast<Builtin>(type);
        if (mapsToJavaBuiltinType(b))
        {
            string_view kindName = builtinNameTable[b->kind()];
            if (marshal)
            {
                out << nl << stream << ".write" << kindName << "Seq(" << param << ");";
            }
            else
            {
                out << nl << param << " = " << stream << ".read" << kindName << "Seq();";
            }
        }
        else
        {
            if (marshal)
            {
                out << nl << "if (" << param << " == null)";
                out << sb;
                out << nl << stream << ".writeSize(0);";
                out << eb << " else";
                out << sb;
                out << nl << stream << ".writeSize(" << param << ".length);";
                out << nl << "for (int i" << iter << " = 0; i" << iter << " < " << param << ".length; i" << iter
                    << "++)";
                out << sb;
                ostringstream o;
                o << param << "[i" << iter << "]";
                iter++;
                writeMarshalUnmarshalCode(out, package, type, false, false, 0, o.str(), true, iter, customStream);
                out << eb;
                out << eb;
            }
            else
            {
                bool isObject = false;
                ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(origContent);
                if ((b && b->usesClasses()) || cl)
                {
                    isObject = true;
                }
                out << nl << "final int len" << iter << " = " << stream << ".readAndCheckSeqSize("
                    << type->minWireSize() << ");";
                //
                // We cannot allocate an array of a generic type, such as
                //
                // arr = new Map<String, String>[sz];
                //
                // Attempting to compile this code results in a "generic array creation" error
                // message. This problem can occur when the sequence's element type is a
                // dictionary, or when the element type is a nested sequence that uses a custom
                // mapping.
                //
                // The solution is to rewrite the code as follows:
                //
                // arr = (Map<String, String>[])new Map[sz];
                //
                // Unfortunately, this produces an unchecked warning during compilation.
                //
                // A simple test is to look for a "<" character in the content type, which
                // indicates the use of a generic type.
                //
                string::size_type pos = origContentS.find('<');
                if (pos != string::npos)
                {
                    string nonGenericType = origContentS.substr(0, pos);
                    out << nl << param << " = (" << origContentS << "[]";
                    int d = depth;
                    while (d--)
                    {
                        out << "[]";
                    }
                    out << ")new " << nonGenericType << "[len" << iter << "]";
                }
                else
                {
                    out << nl << param << " = new " << origContentS << "[len" << iter << "]";
                }
                int d = depth;
                while (d--)
                {
                    out << "[]";
                }
                out << ';';
                out << nl << "for (int i" << iter << " = 0; i" << iter << " < len" << iter << "; i" << iter << "++)";
                out << sb;
                ostringstream o;
                o << param << "[i" << iter << "]";
                if (isObject)
                {
                    ostringstream patchParams;
                    out << nl << "final int fi" << iter << " = i" << iter << ";";
                    patchParams << "value -> " << param << "[fi" << iter << "] = value, " << origContentS << ".class";
                    writeMarshalUnmarshalCode(
                        out,
                        package,
                        type,
                        false,
                        false,
                        0,
                        o.str(),
                        false,
                        iter,
                        customStream,
                        MetadataList(),
                        patchParams.str());
                }
                else
                {
                    writeMarshalUnmarshalCode(out, package, type, false, false, 0, o.str(), false, iter, customStream);
                }
                out << eb;
                iter++;
            }
        }
    }
}

void
Slice::JavaVisitor::writeResultTypeMarshalUnmarshalCode(
    Output& out,
    const OperationPtr& op,
    const string& package,
    const string& streamName,
    const string& paramPrefix,
    bool isMarshalling)
{
    int iter = 0;

    const string retval = getEscapedParamName(op->outParameters(), "returnValue");

    for (const auto& param : op->sortedReturnAndOutParameters(retval))
    {
        const bool isOptional = param->optional();
        const string name = paramPrefix + param->mappedName();
        const string patchParams = isMarshalling ? getPatcher(param->type(), package, name) : "";

        writeMarshalUnmarshalCode(
            out,
            package,
            param->type(),
            isOptional,
            isOptional,
            param->tag(),
            name,
            !isMarshalling,
            iter,
            streamName,
            param->getMetadata(),
            patchParams);
    }
}

void
Slice::JavaVisitor::writeResultType(
    Output& out,
    const OperationPtr& op,
    const string& package,
    const optional<DocComment>& comment)
{
    string opName = op->mappedName();
    opName[0] = static_cast<char>(toupper(static_cast<unsigned char>(opName[0])));

    out << sp;
    writeDocComment(out, "Holds the result of operation " + op->mappedName() + ".");
    out << nl << "public static class " << opName << "Result";
    out << sb;

    // Make sure none of the out parameters are named "returnValue".
    string retval = getEscapedParamName(op->outParameters(), "returnValue");

    const ParameterList outParams = op->outParameters();
    const TypePtr ret = op->returnType();

    //
    // Fields.
    //
    if (ret)
    {
        if (comment)
        {
            const StringList& returns = comment->returns();
            if (!returns.empty())
            {
                out << nl << "/**";
                out << nl << " * ";
                writeDocCommentLines(out, returns);
                out << nl << " */";
            }
        }
        out << nl << "public "
            << typeToString(ret, TypeModeIn, package, op->getMetadata(), true, op->returnIsOptional()) << ' ' << retval
            << ';';
        out << sp;
    }

    for (const auto& outParam : outParams)
    {
        if (comment)
        {
            const map<string, StringList>& paramDocs = comment->parameters();
            auto q = paramDocs.find(outParam->name());
            if (q != paramDocs.end() && !q->second.empty())
            {
                out << nl << "/**";
                out << nl << " * ";
                writeDocCommentLines(out, q->second);
                out << nl << " */";
            }
        }
        out << nl << "public "
            << typeToString(outParam->type(), TypeModeIn, package, outParam->getMetadata(), true, outParam->optional())
            << ' ' << outParam->mappedName() << ';';
        out << sp;
    }

    //
    // Default constructor.
    //
    writeDocComment(out, "Default constructor.");
    out << nl << "public " << opName << "Result()";
    out << sb;
    out << eb;

    //
    // One-shot constructor.
    //

    bool needMandatoryOnly = false;
    bool generateMandatoryOnly = false;

    do
    {
        if (needMandatoryOnly)
        {
            generateMandatoryOnly = true;
        }
        needMandatoryOnly = !generateMandatoryOnly && op->receivesOptionals();

        out << sp;

        if (comment)
        {
            //
            // Emit a doc comment for the constructor if necessary.
            //
            out << nl << "/**";
            out << nl << " * This constructor makes shallow copies of the results for operation " << op->mappedName();
            if (generateMandatoryOnly)
            {
                out << " (overload without Optional parameters).";
            }
            else
            {
                out << '.';
            }

            const StringList& returns = comment->returns();
            if (ret && !returns.empty())
            {
                out << nl << " * @param " << retval << ' ';
                writeDocCommentLines(out, returns);
            }
            const map<string, StringList>& paramDocs = comment->parameters();
            for (const auto& outParam : outParams)
            {
                auto q = paramDocs.find(outParam->name());
                if (q != paramDocs.end() && !q->second.empty())
                {
                    out << nl << " * @param " << outParam->mappedName() << ' ';
                    writeDocCommentLines(out, q->second);
                }
            }
            out << nl << " */";
        }

        out << nl << "public " << opName << "Result" << spar;

        if (ret)
        {
            out
                << (typeToString(
                        ret,
                        TypeModeIn,
                        package,
                        op->getMetadata(),
                        true,
                        !generateMandatoryOnly && op->returnIsOptional()) +
                    " " + retval);
        }
        for (const auto& outParam : outParams)
        {
            out
                << (typeToString(
                        outParam->type(),
                        TypeModeIn,
                        package,
                        outParam->getMetadata(),
                        true,
                        !generateMandatoryOnly && outParam->optional()) +
                    " " + outParam->mappedName());
        }
        out << epar;
        out << sb;
        if (ret)
        {
            out << nl << "this." << retval << " = ";
            if (op->returnIsOptional() && generateMandatoryOnly)
            {
                out << ofFactory(ret) << "(" << retval << ");";
            }
            else
            {
                out << retval << ';';
            }
        }
        for (const auto& outParam : outParams)
        {
            const string name = outParam->mappedName();
            out << nl << "this." << name << " = ";
            if (outParam->optional() && generateMandatoryOnly)
            {
                out << ofFactory(outParam->type()) << "(" << name << ");";
            }
            else
            {
                out << name << ';';
            }
        }
        out << eb;
    } while (needMandatoryOnly);

    out << sp;
    out << nl << "/**";
    out << nl << " * Marshals the result for the " << op->mappedName() << " operation.";
    out << nl << " * @param ostr the output stream to write this result to";
    out << nl << " */";
    out << nl << "public void write(com.zeroc.Ice.OutputStream ostr)";
    out << sb;
    writeResultTypeMarshalUnmarshalCode(out, op, package, "", "this.", false);
    out << eb;

    out << sp;
    out << nl << "/**";
    out << nl << " * Unmarshals the result of the " << op->mappedName() << " operation.";
    out << nl << " * @param istr the input stream to read this result from";
    out << nl << " */";
    out << nl << "public void read(com.zeroc.Ice.InputStream istr)";
    out << sb;
    writeResultTypeMarshalUnmarshalCode(out, op, package, "", "this.", true);
    out << eb;

    out << eb;
}

void
Slice::JavaVisitor::writeMarshaledResultType(
    Output& out,
    const OperationPtr& op,
    const string& package,
    const optional<DocComment>& comment)
{
    string opName = op->mappedName();
    const TypePtr ret = op->returnType();
    opName[0] = static_cast<char>(toupper(static_cast<unsigned char>(opName[0])));

    out << sp;
    writeDocComment(out, "Holds the marshaled result of operation " + op->mappedName() + ".");
    out << nl << "public static class " << opName << "MarshaledResult implements com.zeroc.Ice.MarshaledResult" << sb;

    const ParameterList params = op->parameters();
    const ParameterList outParams = op->outParameters();
    const string retval = getEscapedParamName(params, "returnValue");
    const string currentParamName = getEscapedParamName(params, "current");
    const string currentParam = "com.zeroc.Ice.Current " + currentParamName;

    out << sp;

    //
    // Emit a doc comment for the constructor if necessary.
    //
    if (comment)
    {
        out << nl << "/**";
        out << nl << " * This constructor marshals the results of operation " << op->mappedName() << " immediately.";

        const StringList& returns = comment->returns();
        if (ret && !returns.empty())
        {
            out << nl << " * @param " << retval << ' ';
            writeDocCommentLines(out, returns);
        }
        const map<string, StringList>& paramDocs = comment->parameters();
        for (const auto& outParam : outParams)
        {
            auto q = paramDocs.find(outParam->name());
            if (q != paramDocs.end() && !q->second.empty())
            {
                out << nl << " * @param " << outParam->mappedName() << ' ';
                writeDocCommentLines(out, q->second);
            }
        }
        out << nl << " * @param " << currentParamName << " The Current object of the incoming request.";
        out << nl << " */";
    }

    bool hasOpt = false;
    out << nl << "public " << opName << "MarshaledResult" << spar;
    if (ret)
    {
        out << (typeToString(ret, TypeModeIn, package, op->getMetadata(), true, op->returnIsOptional()) + " " + retval);
        hasOpt = op->returnIsOptional();
    }
    for (const auto& outParam : outParams)
    {
        out
            << (typeToString(
                    outParam->type(),
                    TypeModeIn,
                    package,
                    outParam->getMetadata(),
                    true,
                    outParam->optional()) +
                " " + outParam->mappedName());

        hasOpt = hasOpt || outParam->optional();
    }
    out << currentParam << epar;
    out << sb;
    out << nl << "_ostr = " << currentParamName << ".startReplyStream();";
    out << nl << "_ostr.startEncapsulation(" << currentParamName << ".encoding, " << opFormatTypeToString(op) << ");";

    writeResultTypeMarshalUnmarshalCode(out, op, package, "_ostr", "", false);

    if (op->returnsClasses())
    {
        out << nl << "_ostr.writePendingValues();";
    }

    out << nl << "_ostr.endEncapsulation();";

    out << eb;

    if (hasOpt)
    {
        out << sp;

        //
        // Emit a doc comment for the constructor if necessary.
        //
        if (comment)
        {
            out << nl << "/**";
            out << nl << " * This constructor marshals the results of operation " << op->mappedName()
                << " immediately (overload without Optional parameters).";

            const StringList& returns = comment->returns();
            if (ret && !returns.empty())
            {
                out << nl << " * @param " << retval << ' ';
                writeDocCommentLines(out, returns);
            }
            const map<string, StringList>& paramDocs = comment->parameters();
            for (const auto& outParam : outParams)
            {
                auto q = paramDocs.find(outParam->name());
                if (q != paramDocs.end() && !q->second.empty())
                {
                    out << nl << " * @param " << outParam->mappedName() << ' ';
                    writeDocCommentLines(out, q->second);
                }
            }
            out << nl << " * @param " << currentParamName << " The Current object of the incoming request.";
            out << nl << " */";
        }

        out << nl << "public " << opName << "MarshaledResult" << spar;
        if (ret)
        {
            out << (typeToString(ret, TypeModeIn, package, op->getMetadata(), true, false) + " " + retval);
        }
        for (const auto& outParam : outParams)
        {
            out
                << (typeToString(outParam->type(), TypeModeIn, package, outParam->getMetadata(), true, false) + " " +
                    outParam->mappedName());
        }

        out << currentParam << epar;
        out << sb;
        out << nl << "this" << spar;
        if (ret)
        {
            if (op->returnIsOptional())
            {
                out << ofFactory(ret) + "(" + retval + ")";
            }
            else
            {
                out << retval;
            }
        }
        for (const auto& outParam : outParams)
        {
            if (outParam->optional())
            {
                out << ofFactory(outParam->type()) + "(" + outParam->mappedName() + ")";
            }
            else
            {
                out << outParam->mappedName();
            }
        }

        out << currentParamName << epar << ';';
        out << eb;
    }

    out << sp;
    out << nl << "@Override" << nl << "public com.zeroc.Ice.OutputStream getOutputStream()" << sb << nl
        << "return _ostr;" << eb;

    out << sp;
    out << nl << "private com.zeroc.Ice.OutputStream _ostr;";
    out << eb;
}

void
Slice::JavaVisitor::allocatePatcher(Output& out, const TypePtr& type, const string& package, const string& name)
{
    BuiltinPtr b = dynamic_pointer_cast<Builtin>(type);
    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    assert((b && b->usesClasses()) || cl);

    string clsName;
    if (b)
    {
        clsName = "com.zeroc.Ice.Value";
    }
    else
    {
        clsName = getUnqualified(cl, package);
    }
    out << nl << "final com.zeroc.Ice.Holder<" << clsName << "> " << name << " = new com.zeroc.Ice.Holder<>();";
}

string
Slice::JavaVisitor::getPatcher(const TypePtr& type, const string& package, const string& dest)
{
    BuiltinPtr b = dynamic_pointer_cast<Builtin>(type);
    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    ostringstream ostr;
    if ((b && b->usesClasses()) || cl)
    {
        string clsName;
        if (b)
        {
            clsName = "com.zeroc.Ice.Value";
        }
        else
        {
            clsName = getUnqualified(cl, package);
        }
        ostr << "v -> " << dest << " = v, " << clsName << ".class";
    }
    return ostr.str();
}

void
Slice::JavaVisitor::writeSyncIceInvokeMethods(
    Output& out,
    const OperationPtr& p,
    const vector<string>& params,
    const ExceptionList& throws,
    const optional<DocComment>& dc)
{
    const string name = p->mappedName();
    const string package = getPackage(p->interface());

    const string resultType = getResultType(p, package, false, false);

    const string contextParamName = getEscapedParamName(p->parameters(), "context");
    const string contextDoc = "@param " + contextParamName + " The Context map to send with the invocation.";
    const string contextParam = "java.util.Map<java.lang.String, java.lang.String> " + contextParamName;
    const string noExplicitContextArg = "com.zeroc.Ice.ObjectPrx.noExplicitContext";

    const bool returnsParams = p->returnsAnyValues();
    const vector<string> args = getInArgs(p);

    // Generate a synchronous version of this operation which doesn't takes a context parameter.
    out << sp;
    writeProxyOpDocComment(out, p, package, dc, false, "");
    if (dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }
    out << nl << "default " << resultType << ' ' << name << spar << params << epar;
    writeThrowsClause(out, package, throws);
    out << sb;
    out << nl;
    if (returnsParams)
    {
        out << "return ";
    }
    out << name << spar << args << noExplicitContextArg << epar << ';';
    out << eb;

    // Generate a synchronous version of this operation which takes a context parameter.
    out << sp;
    writeProxyOpDocComment(out, p, package, dc, false, contextDoc);
    if (dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }
    out << nl << "default " << resultType << ' ' << name << spar << params << contextParam << epar;
    writeThrowsClause(out, package, throws);
    out << sb;
    if (throws.empty())
    {
        out << nl;
        if (returnsParams)
        {
            out << "return ";
        }
        out << "_iceI_" << name << "Async" << spar << args << contextParamName << "true" << epar
            << ".waitForResponse();";
    }
    else
    {
        out << nl << "try";
        out << sb;
        out << nl;
        if (returnsParams)
        {
            out << "return ";
        }
        out << "_iceI_" << name << "Async" << spar << args << contextParamName << "true" << epar
            << ".waitForResponseOrUserEx();";
        out << eb;
        for (const auto& t : throws)
        {
            string exS = getUnqualified(t, package);
            out << " catch (" << exS << " ex)";
            out << sb;
            out << nl << "throw ex;";
            out << eb;
        }
        out << " catch (com.zeroc.Ice.UserException ex)";
        out << sb;
        out << nl << "throw com.zeroc.Ice.UnknownUserException.fromTypeId(ex.ice_id());";
        out << eb;
    }
    out << eb;
}

void
Slice::JavaVisitor::writeAsyncIceInvokeMethods(
    Output& out,
    const OperationPtr& p,
    const vector<string>& params,
    const optional<DocComment>& dc)
{
    const string name = p->mappedName();
    const string package = getPackage(p->interface());

    const string resultType = getResultType(p, package, true, false);
    const string futureType = "java.util.concurrent.CompletableFuture<" + resultType + ">";

    const string contextParamName = getEscapedParamName(p->parameters(), "context");
    const string contextDoc = "@param " + contextParamName + " The Context map to send with the invocation.";
    const string contextParam = "java.util.Map<java.lang.String, java.lang.String> " + contextParamName;
    const string noExplicitContextArg = "com.zeroc.Ice.ObjectPrx.noExplicitContext";
    const vector<string> args = getInArgs(p);

    // Generate an overload of '<NAME>Async' that doesn't take a context parameter.
    out << sp;
    writeProxyOpDocComment(out, p, package, dc, true, "");
    if (dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }
    out << nl << "default " << futureType << ' ' << name << "Async" << spar << params << epar;
    out << sb;
    out << nl << "return _iceI_" << name << "Async" << spar << args << noExplicitContextArg << "false" << epar << ';';
    out << eb;

    // Generate an overload of '<NAME>Async' that takes a context parameter.
    out << sp;
    writeProxyOpDocComment(out, p, package, dc, true, contextDoc);
    if (dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }
    out << nl << "default " << futureType << ' ' << name << "Async" << spar << params << contextParam << epar;
    out << sb;
    out << nl << "return _iceI_" << name << "Async" << spar << args << contextParamName << "false" << epar << ';';
    out << eb;
}

void
Slice::JavaVisitor::writeIceIHelperMethods(
    Output& out,
    const OperationPtr& p,
    bool hasExceptionSpecification,
    bool optionalMapping)
{
    const string name = p->mappedName();
    const string package = getPackage(p->interface());

    const string resultType = getResultType(p, package, true, false);
    const string futureImplType = "com.zeroc.Ice.OutgoingAsync<" + resultType + ">";

    // Generate the internal method '_iceI_<NAME>Async'.
    out << sp;
    out << nl << "private " << futureImplType << " _iceI_" << name << "Async" << spar
        << getParamsProxy(p, package, optionalMapping, true)
        << "java.util.Map<java.lang.String, java.lang.String> context" << "boolean sync" << epar;
    out << sb;
    out << nl << futureImplType << " f = new com.zeroc.Ice.OutgoingAsync<>(this, \"" << p->name() << "\", "
        << sliceModeToIceMode(p->mode()) << ", sync, " << (hasExceptionSpecification ? "_iceE_" + name : "null")
        << ");";
    out << nl << "f.invoke(";
    out.useCurrentPosAsIndent();
    out << (p->returnsData() ? "true" : "false") << ", context, " << opFormatTypeToString(p) << ", ";
    if (!p->inParameters().empty())
    {
        out << "ostr -> {";
        out.inc();
        writeMarshalProxyParams(out, package, p, optionalMapping);
        out.dec();
        out << nl << '}';
    }
    else
    {
        out << "null";
    }
    out << ", ";
    if (p->returnsAnyValues())
    {
        out << "istr -> {";
        out.inc();
        writeUnmarshalProxyResults(out, package, p);
        out.dec();
        out << nl << "}";
    }
    else
    {
        out << "null";
    }
    out.restoreIndent();
    out << ");";
    out << nl << "return f;";
    out << eb;
}

void
Slice::JavaVisitor::writeMarshalProxyParams(
    Output& out,
    const string& package,
    const OperationPtr& op,
    bool optionalMapping)
{
    int iter = 0;
    for (const auto& param : op->sortedInParameters())
    {
        writeMarshalUnmarshalCode(
            out,
            package,
            param->type(),
            param->optional(),
            param->optional() && optionalMapping,
            param->tag(),
            "iceP_" + param->mappedName(),
            true,
            iter,
            "",
            param->getMetadata());
    }

    if (op->sendsClasses())
    {
        out << nl << "ostr.writePendingValues();";
    }
}

void
Slice::JavaVisitor::writeUnmarshalProxyResults(Output& out, const string& package, const OperationPtr& op)
{
    const ParameterList outParams = op->outParameters();
    const TypePtr ret = op->returnType();
    const string name = "ret";

    if (op->returnsMultipleValues())
    {
        string resultType = getResultType(op, package, false, false);
        out << nl << resultType << ' ' << name << " = new " << resultType << "();";
        out << nl << name << ".read(istr);";
        if (op->returnsClasses())
        {
            out << nl << "istr.readPendingValues();";
        }
        out << nl << "return " << name << ';';
    }
    else
    {
        string resultType = getResultType(op, package, false, false);

        bool isOptional;
        TypePtr type;
        int32_t tag;
        MetadataList metadata;
        if (ret)
        {
            type = ret;
            isOptional = op->returnIsOptional();
            tag = op->returnTag();
            metadata = op->getMetadata();
        }
        else
        {
            assert(outParams.size() == 1);
            isOptional = outParams.front()->optional();
            type = outParams.front()->type();
            tag = outParams.front()->tag();
            metadata = outParams.front()->getMetadata();
        }

        const bool val = type->isClassType();

        int iter = 0;

        if (val)
        {
            assert(!isOptional); // Optional classes are disallowed by the parser.
            allocatePatcher(out, type, package, name);
        }
        else
        {
            out << nl << resultType << ' ' << name << ';';
        }

        string patchParams = getPatcher(type, package, name + ".value");
        writeMarshalUnmarshalCode(
            out,
            package,
            type,
            isOptional,
            true,
            tag,
            name,
            false,
            iter,
            "",
            metadata,
            patchParams);

        if (op->returnsClasses())
        {
            out << nl << "istr.readPendingValues();";
        }

        if (val)
        {
            out << nl << "return " << name << ".value;";
        }
        else
        {
            out << nl << "return " << name << ';';
        }
    }
}

void
Slice::JavaVisitor::writeMarshalServantResults(
    Output& out,
    const string& package,
    const OperationPtr& op,
    const string& param)
{
    if (op->returnsMultipleValues())
    {
        out << nl << param << ".write(ostr);";
    }
    else
    {
        const ParameterList params = op->outParameters();
        bool isOptional;
        TypePtr type;
        int32_t tag;
        MetadataList metadata;
        if (op->returnType())
        {
            type = op->returnType();
            isOptional = op->returnIsOptional();
            tag = op->returnTag();
            metadata = op->getMetadata();
        }
        else
        {
            assert(params.size() == 1);
            isOptional = params.front()->optional();
            type = params.front()->type();
            tag = params.front()->tag();
            metadata = params.front()->getMetadata();
        }

        int iter = 0;
        writeMarshalUnmarshalCode(out, package, type, isOptional, true, tag, param, true, iter, "", metadata);
    }

    if (op->returnsClasses())
    {
        out << nl << "ostr.writePendingValues();";
    }
}

void
Slice::JavaVisitor::writeThrowsClause(
    Output& out,
    const string& package,
    const ExceptionList& throws,
    const OperationPtr& op)
{
    if (op && (op->hasMetadata("java:UserException")))
    {
        out.inc();
        out << nl << "throws com.zeroc.Ice.UserException";
        out.dec();
    }
    else if (throws.size() > 0)
    {
        out.inc();
        out << nl << "throws ";
        out.spar("");
        for (const auto& exception : throws)
        {
            out << getUnqualified(exception, package);
        }
        out.epar("");
        out.dec();
    }
}

void
Slice::JavaVisitor::writeMarshalDataMember(Output& out, const string& package, const DataMemberPtr& member, int& iter)
{
    const bool isOptional = member->optional();
    const bool forStruct = dynamic_pointer_cast<Struct>(member->container()) != nullptr;
    const string memberName = (forStruct ? "this." : "") + member->mappedName();

    // If this is an optional data member, we first have to handle the optional tag.
    if (isOptional)
    {
        assert(!forStruct);
        out << nl << "if (_" << memberName << ")";
        out << sb;
    }

    // Write the marshalling code like normal.
    writeMarshalUnmarshalCode(
        out,
        package,
        member->type(),
        isOptional,
        false,
        member->tag(),
        memberName,
        true,
        iter,
        forStruct ? "" : "ostr_",
        member->getMetadata());

    // If this is an optional data member, generate a closing brace to balance out the 'if' check.
    if (isOptional)
    {
        out << eb;
    }
}

void
Slice::JavaVisitor::writeUnmarshalDataMember(Output& out, const string& package, const DataMemberPtr& member, int& iter)
{
    const TypePtr& type = member->type();
    const bool isOptional = member->optional();
    const bool forStruct = (bool)dynamic_pointer_cast<Struct>(member->container());
    const string stream = forStruct ? "istr" : "istr_";
    assert(!isOptional || !forStruct);           // optional members aren't allowed in structs.
    assert(!isOptional || !type->isClassType()); // optional class types aren't allowed.

    // If this is an optional data member, we first have to handle the optional tag.
    if (isOptional)
    {
        out << nl << "if (_" << member->mappedName() << " = istr_.readOptional(" << member->tag() << ", "
            << getOptionalFormat(member->type()) << "))";
        out << sb;

        const string optionalFormat = type->getOptionalFormat();
        if (optionalFormat == "FSize")
        {
            out << nl << stream << ".skip(4);";
        }
        else if (optionalFormat == "VSize")
        {
            if (auto seq = dynamic_pointer_cast<Sequence>(type))
            {
                if (seq->type()->minWireSize() > 1)
                {
                    out << nl << stream << ".skipSize();";
                }
            }
            else if (!dynamic_pointer_cast<Builtin>(type))
            {
                out << nl << stream << ".skipSize();";
            }
        }
    }

    // Write the unmarshalling code like normal.
    const string memberName = (forStruct ? "this." : "") + member->mappedName();
    const MetadataList& metadata = member->getMetadata();
    const string patchParams = getPatcher(member->type(), package, member->mappedName());
    writeMarshalUnmarshalCode(
        out,
        package,
        type,
        false,
        false,
        0,
        memberName,
        false,
        iter,
        stream,
        metadata,
        patchParams);

    // If this is an optional data member, generate a closing brace to balance out the 'if' check.
    if (isOptional)
    {
        out << eb;
    }
}

void
Slice::JavaVisitor::writeConstantValue(
    Output& out,
    const TypePtr& type,
    const SyntaxTreeBasePtr& valueType,
    const string& value,
    const string& package)
{
    ConstPtr constant = dynamic_pointer_cast<Const>(valueType);
    if (constant)
    {
        out << getUnqualified(constant, package) << ".value";
    }
    else
    {
        BuiltinPtr bp;
        if ((bp = dynamic_pointer_cast<Builtin>(type)))
        {
            switch (bp->kind())
            {
                case Builtin::KindString:
                {
                    out << "\"" << toStringLiteral(value, "\b\f\n\r\t", "", ShortUCN, 0) << "\"";
                    break;
                }
                case Builtin::KindByte:
                {
                    int i = stoi(value);
                    if (i > 127)
                    {
                        i -= 256;
                    }
                    out << i; // Slice byte runs from 0-255, Java byte runs from -128 - 127.
                    break;
                }
                case Builtin::KindLong:
                {
                    out << value << "L"; // Need to append "L" modifier for long constants.
                    break;
                }
                case Builtin::KindBool:
                case Builtin::KindShort:
                case Builtin::KindInt:
                case Builtin::KindDouble:
                case Builtin::KindObjectProxy:
                case Builtin::KindValue:
                {
                    out << value;
                    break;
                }
                case Builtin::KindFloat:
                {
                    out << value << "F";
                    break;
                }
            }
        }
        else if (dynamic_pointer_cast<Enum>(type))
        {
            EnumeratorPtr lte = dynamic_pointer_cast<Enumerator>(valueType);
            assert(lte);
            out << getUnqualified(lte, package);
        }
        else
        {
            out << value;
        }
    }
}

void
Slice::JavaVisitor::writeDataMemberInitializers(Output& out, const DataMemberList& members, const string& package)
{
    for (const auto& member : members)
    {
        TypePtr t = member->type();
        if (member->defaultValue())
        {
            if (member->optional())
            {
                string capName = member->mappedName();
                capName[0] = static_cast<char>(toupper(static_cast<unsigned char>(capName[0])));
                out << nl << "set" << capName << '(';
                writeConstantValue(out, t, member->defaultValueType(), *member->defaultValue(), package);
                out << ");";
            }
            else
            {
                out << nl << "this." << member->mappedName() << " = ";
                writeConstantValue(out, t, member->defaultValueType(), *member->defaultValue(), package);
                out << ';';
            }
        }
        else
        {
            BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(t);
            if (builtin && builtin->kind() == Builtin::KindString)
            {
                out << nl << "this." << member->mappedName() << " = \"\";";
            }

            EnumPtr en = dynamic_pointer_cast<Enum>(t);
            if (en)
            {
                string firstEnum = en->enumerators().front()->mappedName();
                out << nl << "this." << member->mappedName() << " = " << getUnqualified(en, package) << '.' << firstEnum
                    << ';';
            }

            StructPtr st = dynamic_pointer_cast<Struct>(t);
            if (st)
            {
                string memberType = typeToString(st, TypeModeMember, package, member->getMetadata());
                out << nl << "this." << member->mappedName() << " = new " << memberType << "();";
            }
        }
    }
}

void
Slice::JavaVisitor::writeExceptionDocComment(Output& out, const OperationPtr& op, const DocComment& dc)
{
    for (const auto& [name, lines] : dc.exceptions())
    {
        // Try to locate the exception's definition using the name given in the comment.
        ExceptionPtr ex = op->container()->lookupException(name, false);
        string scopedName = (ex ? getUnqualified(ex, getPackage(op)) : name);
        out << nl << " * @throws " << scopedName << ' ';
        writeDocCommentLines(out, lines);
    }
}

void
Slice::JavaVisitor::writeRemarksDocComment(Output& out, const DocComment& comment)
{
    const StringList& remarks = comment.remarks();
    if (remarks.empty())
    {
        return;
    }

    out << nl << " * <p><b>Remarks:</b>";
    out << nl << " * ";
    writeDocCommentLines(out, remarks);
    out << "</p>";
}

void
Slice::JavaVisitor::writeHiddenDocComment(Output& out)
{
    out << nl << "/** @hidden */";
}

void
Slice::JavaVisitor::writeDocCommentLines(Output& out, const StringList& lines)
{
    //
    // This method emits a block of text, prepending a leading " * " to the second and
    // subsequent lines. We assume the caller prepended a leading " * " for the first
    // line if necessary.
    //
    assert(!lines.empty());
    StringList l = lines;
    out << l.front();
    l.pop_front();
    for (const auto& p : l)
    {
        out << nl << " *";
        if (!p.empty())
        {
            out << " " << p;
        }
    }
}

void
Slice::JavaVisitor::writeDocCommentLines(Output& out, const string& text)
{
    //
    // This method emits a block of text, prepending a leading " * " to the second and
    // subsequent lines. We assume the caller prepended a leading " * " for the first
    // line if necessary.
    //
    string::size_type start = 0;
    string::size_type pos;
    const string ws = "\n";
    pos = text.find_first_of(ws);
    if (pos == string::npos)
    {
        out << text;
    }
    else
    {
        string s = IceInternal::trim(text.substr(start, pos - start));
        out << s; // Emit the first line.
        start = pos + 1;
        while ((pos = text.find_first_of(ws, start)) != string::npos)
        {
            string line = IceInternal::trim(text.substr(start, pos - start));
            if (line.empty())
            {
                out << nl << " *";
            }
            else
            {
                out << nl << " * " << line;
            }
            start = pos + 1;
        }
        if (start < text.size())
        {
            string line = IceInternal::trim(text.substr(start));
            if (line.empty())
            {
                out << nl << " *";
            }
            else
            {
                out << nl << " * " << line;
            }
        }
    }
}

void
Slice::JavaVisitor::writeDocComment(Output& out, const UnitPtr& unt, const optional<DocComment>& dc)
{
    if (!dc)
    {
        return;
    }

    out << nl << "/**";
    const StringList& overview = dc->overview();
    if (!overview.empty())
    {
        out << nl << " * ";
        writeDocCommentLines(out, overview);
    }
    writeRemarksDocComment(out, *dc);

    const StringList& seeAlso = dc->seeAlso();
    if (!seeAlso.empty())
    {
        out << nl << " *";
        for (const auto& p : seeAlso)
        {
            out << nl << " * @see ";
            writeSeeAlso(out, unt, p);
        }
    }

    const StringList& deprecated = dc->deprecated();
    if (!deprecated.empty())
    {
        out << nl << " * @deprecated ";
        writeDocCommentLines(out, deprecated);
    }
    else if (dc->isDeprecated())
    {
        out << nl << " * @deprecated";
    }

    out << nl << " */";
}

void
Slice::JavaVisitor::writeDocComment(Output& out, const string& text)
{
    if (!text.empty())
    {
        out << nl << "/**";
        out << nl << " * ";
        writeDocCommentLines(out, text);
        out << nl << " */";
    }
}

void
Slice::JavaVisitor::writeProxyOpDocComment(
    Output& out,
    const OperationPtr& p,
    const string& package,
    const optional<DocComment>& dc,
    bool async,
    const string& contextParam)
{
    if (!dc)
    {
        return;
    }

    out << nl << "/**";
    const StringList& overview = dc->overview();
    if (!overview.empty())
    {
        out << nl << " * ";
        writeDocCommentLines(out, overview);
    }
    writeRemarksDocComment(out, *dc);

    //
    // Show in-params in order of declaration, but only those with docs.
    //
    const ParameterList paramList = p->inParameters();
    const map<string, StringList>& paramDocs = dc->parameters();
    for (const auto& param : paramList)
    {
        auto j = paramDocs.find(param->name());
        if (j != paramDocs.end() && !j->second.empty())
        {
            out << nl << " * @param " << param->mappedName() << ' ';
            writeDocCommentLines(out, j->second);
        }
    }
    if (!contextParam.empty())
    {
        out << nl << " * " << contextParam;
    }

    //
    // Handle the return value (if any).
    //
    if (p->returnsMultipleValues())
    {
        const string r = getResultType(p, package, true, false);
        if (async)
        {
            out << nl << " * @return A future that will be completed with an instance of " << r << '.';
        }
        else
        {
            out << nl << " * @return An instance of " << r << '.';
        }
    }
    else if (p->returnType())
    {
        const StringList returns = dc->returns();
        if (!returns.empty())
        {
            out << nl << " * @return ";
            writeDocCommentLines(out, returns);
        }
        else if (async)
        {
            out << nl << " * @return A future that will be completed with the result.";
        }
    }
    else if (!p->outParameters().empty())
    {
        assert(p->outParameters().size() == 1);
        const ParameterPtr param = p->outParameters().front();
        auto j = paramDocs.find(param->name());
        if (j != paramDocs.end() && !j->second.empty())
        {
            out << nl << " * @return ";
            writeDocCommentLines(out, j->second);
        }
        else if (async)
        {
            out << nl << " * @return A future that will be completed with the result.";
        }
    }
    else if (async)
    {
        //
        // No results but an async proxy operation still returns a future.
        //
        out << nl << " * @return A future that will be completed when the invocation completes.";
    }

    //
    // Async proxy methods don't declare user exceptions.
    //
    if (!async)
    {
        writeExceptionDocComment(out, p, *dc);
    }

    const StringList& seeAlso = dc->seeAlso();
    if (!seeAlso.empty())
    {
        out << nl << " *";
        StringList sa = seeAlso;
        for (const auto& q : sa)
        {
            out << nl << " * @see ";
            writeSeeAlso(out, p->unit(), q);
        }
    }

    const StringList& deprecated = dc->deprecated();
    if (!deprecated.empty())
    {
        out << nl << " * @deprecated ";
        writeDocCommentLines(out, deprecated);
    }
    else if (dc->isDeprecated())
    {
        out << nl << " * @deprecated";
    }

    out << nl << " */";
}

void
Slice::JavaVisitor::writeServantOpDocComment(Output& out, const OperationPtr& p, const string& package, bool async)
{
    const optional<DocComment>& dc = p->docComment();
    if (!dc)
    {
        return;
    }

    const map<string, StringList>& paramDocs = dc->parameters();
    const string currentParamName = getEscapedParamName(p->parameters(), "current");
    const string currentParam = " * @param " + currentParamName + " The Current object of the incoming request.";

    out << nl << "/**";
    const StringList& overview = dc->overview();
    if (!overview.empty())
    {
        out << nl << " * ";
        writeDocCommentLines(out, overview);
    }
    writeRemarksDocComment(out, *dc);

    // Show in-params in order of declaration, but only those with docs.
    for (const auto& param : p->inParameters())
    {
        auto j = paramDocs.find(param->name());
        if (j != paramDocs.end() && !j->second.empty())
        {
            out << nl << " * @param " << param->mappedName() << ' ';
            writeDocCommentLines(out, j->second);
        }
    }
    out << nl << currentParam;

    // Handle the return value (if any).
    if (p->returnsMultipleValues())
    {
        const string r = getResultType(p, package, true, false);
        if (async)
        {
            out << nl << " * @return A completion stage that the servant will complete with an instance of " << r
                << '.';
        }
        else
        {
            out << nl << " * @return An instance of " << r << '.';
        }
    }
    else if (p->returnType())
    {
        const StringList& returns = dc->returns();
        if (!returns.empty())
        {
            out << nl << " * @return ";
            writeDocCommentLines(out, returns);
        }
        else if (async)
        {
            out << nl << " * @return A completion stage that the servant will complete with the result.";
        }
    }
    else if (!p->outParameters().empty())
    {
        assert(p->outParameters().size() == 1);
        const ParameterPtr param = p->outParameters().front();
        auto j = paramDocs.find(param->name());
        if (j != paramDocs.end() && !j->second.empty())
        {
            out << nl << " * @return ";
            writeDocCommentLines(out, j->second);
        }
        else if (async)
        {
            out << nl << " * @return A completion stage that the servant will complete with the result.";
        }
    }
    else if (async)
    {
        //
        // No results but an async operation still returns a completion stage.
        //
        out << nl << " * @return A completion stage that the servant will complete when the invocation completes.";
    }

    writeExceptionDocComment(out, p, *dc);

    const StringList& seeAlso = dc->seeAlso();
    if (!seeAlso.empty())
    {
        out << nl << " *";
        for (const auto& q : seeAlso)
        {
            out << nl << " * @see ";
            writeSeeAlso(out, p->unit(), q);
        }
    }

    out << nl << " */";
}

void
Slice::JavaVisitor::writeSeeAlso(Output& out, const UnitPtr& unt, const string& ref)
{
    assert(!ref.empty());

    //
    // Try to look up the referenced type. If we find it, we translate it into a fully-scoped Java type.
    //

    string s = ref;
    string::size_type pos = s.find('#');
    string rest;
    if (pos != string::npos)
    {
        rest = s.substr(pos);
        s = s.substr(0, pos);
    }

    pos = 0;
    while ((pos = s.find('.', pos)) != string::npos)
    {
        s.replace(pos, 1, "::");
    }

    //
    // We assume a scoped name should be an absolute name.
    //
    if (s.find(':') != string::npos && s[0] != ':')
    {
        s.insert(0, "::");
    }

    TypeList l = unt->lookupTypeNoBuiltin(s, false, true);
    if (l.empty())
    {
        out << ref;
    }
    else
    {
        ContainedPtr cont = dynamic_pointer_cast<Contained>(l.front());
        assert(cont);
        out << getUnqualified(cont) << rest;
    }
}

void
Slice::JavaVisitor::writeParamDocComments(IceInternal::Output& out, const DataMemberList& members)
{
    bool first = true;
    for (const auto& member : members)
    {
        if (const auto& docComment = member->docComment())
        {
            if (first)
            {
                out << nl << " *";
                first = false;
            }
            const auto firstSentence = Slice::getFirstSentence(docComment->overview());
            out << nl << " * @param " << member->mappedName() << ' ' << firstSentence;
        }
    }
}

Slice::Gen::Gen(string base, const vector<string>& includePaths, string dir)
    : _base(std::move(base)),
      _includePaths(includePaths),
      _dir(std::move(dir))
{
}

Slice::Gen::~Gen() = default;

void
Slice::Gen::generate(const UnitPtr& p)
{
    validateJavaMetadata(p);

    TypesVisitor typesVisitor(_dir);
    p->visit(&typesVisitor);

    ServantVisitor servantVisitor(_dir);
    p->visit(&servantVisitor);
}

Slice::Gen::TypesVisitor::TypesVisitor(const string& dir) : JavaVisitor(dir) {}

bool
Slice::Gen::TypesVisitor::visitModuleStart(const ModulePtr& p)
{
    string prefix = getPackagePrefix(p);
    if (!prefix.empty() && p->isTopLevel()) // generate Marker class for top-level modules
    {
        string markerClass = prefix + "." + p->mappedName() + "._Marker";
        open(markerClass, p->file());

        Output& out = output();

        out << sp << nl << "interface _Marker";
        out << sb;
        out << eb;

        close();
    }
    return true;
}

bool
Slice::Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = p->mappedName();
    ClassDefPtr base = p->base();
    string package = getPackage(p);

    open(getUnqualified(p), p->file());
    Output& out = output();

    // Slice interfaces map to Java interfaces.
    out << sp;
    const optional<DocComment>& dc = p->docComment();
    writeDocComment(out, p->unit(), dc);
    if (dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }

    out << nl << "@com.zeroc.Ice.SliceTypeId(value = \"" << p->scoped() << "\")";
    if (p->compactId() != -1)
    {
        out << nl << "@com.zeroc.Ice.CompactSliceTypeId(value = " << p->compactId() << ")";
    }
    out << nl << "public class " << name;

    if (base)
    {
        out << " extends " << getUnqualified(base, package);
    }
    else
    {
        out << " extends com.zeroc.Ice.Value";
    }

    out << sb;
    return true;
}

void
Slice::Gen::TypesVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    Output& out = output();
    const string name = p->mappedName();
    const string package = getPackage(p);
    const ClassDefPtr base = p->base();

    const DataMemberList members = p->dataMembers();
    const DataMemberList allDataMembers = p->allDataMembers();
    const DataMemberList optionalMembers = p->orderedOptionalDataMembers();

    DataMemberList requiredMembers;
    for (const auto& member : allDataMembers)
    {
        if (!member->optional())
        {
            requiredMembers.push_back(member);
        }
    }

    if (!allDataMembers.empty())
    {
        //
        // Default constructor.
        //
        out << sp;
        writeDocComment(out, "Constructs a {@code " + name + "}.");
        out << nl << "public " << name << "()";
        out << sb;
        if (base)
        {
            out << nl << "super();";
        }
        writeDataMemberInitializers(out, members, package);
        out << eb;

        //
        // Generate constructor if the parameter list is not too large.
        //
        if (isValidMethodParameterList(allDataMembers))
        {
            DataMemberList baseDataMembers;
            if (base)
            {
                baseDataMembers = base->allDataMembers();
            }

            if (!requiredMembers.empty() && !optionalMembers.empty())
            {
                //
                // Generate a constructor accepting parameters for just the required members.
                //
                out << sp;
                out << nl << "/**";
                out << nl << " * Constructs a {@code " << name
                    << "} with values for all fields not marked optional in the Slice definition for {@code "
                    << p->scoped() << "}.";
                writeParamDocComments(out, requiredMembers);
                out << nl << " */";
                out << nl << "public " << name << spar;
                vector<string> parameters;
                for (const auto& member : requiredMembers)
                {
                    string memberName = member->mappedName();
                    string memberType =
                        typeToString(member->type(), TypeModeMember, package, member->getMetadata(), true, false);
                    parameters.push_back(memberType + " " + memberName);
                }
                out << parameters << epar;
                out << sb;
                if (!baseDataMembers.empty())
                {
                    bool hasBaseRequired = false;
                    for (const auto& baseDataMember : baseDataMembers)
                    {
                        if (!baseDataMember->optional())
                        {
                            hasBaseRequired = true;
                            break;
                        }
                    }
                    if (hasBaseRequired)
                    {
                        out << nl << "super" << spar;
                        vector<string> baseParamNames;
                        for (const auto& baseDataMember : baseDataMembers)
                        {
                            if (!baseDataMember->optional())
                            {
                                baseParamNames.push_back(baseDataMember->mappedName());
                            }
                        }
                        out << baseParamNames << epar << ';';
                    }
                }

                for (const auto& member : members)
                {
                    if (!member->optional())
                    {
                        string paramName = member->mappedName();
                        out << nl << "this." << paramName << " = " << paramName << ';';
                    }
                }
                writeDataMemberInitializers(out, p->orderedOptionalDataMembers(), package);
                out << eb;
            }

            //
            // Generate a constructor accepting parameters for all members.
            //
            out << sp;
            out << nl << "/**";
            out << nl << " * Constructs a {@code " << name << "} with values for all its fields.";
            writeParamDocComments(out, allDataMembers);
            out << nl << " */";
            out << nl << "public " << name << spar;
            vector<string> parameters;
            for (const auto& member : allDataMembers)
            {
                string memberName = member->mappedName();
                string memberType =
                    typeToString(member->type(), TypeModeMember, package, member->getMetadata(), true, false);
                parameters.push_back(memberType + " " + memberName);
            }
            out << parameters << epar;
            out << sb;
            if (base && allDataMembers.size() != members.size())
            {
                out << nl << "super" << spar;
                vector<string> baseParamNames;
                for (const auto& baseDataMember : baseDataMembers)
                {
                    baseParamNames.push_back(baseDataMember->mappedName());
                }
                out << baseParamNames << epar << ';';
            }
            for (const auto& member : members)
            {
                string paramName = member->mappedName();
                if (member->optional())
                {
                    string capName = paramName;
                    capName[0] = static_cast<char>(toupper(static_cast<unsigned char>(capName[0])));
                    out << nl << "set" << capName << '(' << paramName << ");";
                }
                else
                {
                    out << nl << "this." << paramName << " = " << paramName << ';';
                }
            }
            out << eb;
        }
    }

    // Implementation of 'clone'.
    out << sp << nl << "public " << name << " clone()";
    out << sb;
    out << nl << "return (" << name << ") super.clone();";
    out << eb;

    // Implementation of 'ice_staticId'.
    out << sp;
    out << nl << "/**";
    out << nl << " * Gets the type ID of the associated Slice interface.";
    out << nl << " *";
    out << nl << " * @return the string \"" << p->scoped() << "\"";
    out << nl << " */";
    out << nl << "public static java.lang.String ice_staticId()";
    out << sb;
    out << nl << "return \"" << p->scoped() << "\";";
    out << eb;

    // Implementation of 'ice_id'.
    out << sp;
    writeDocComment(out, "{@inheritDoc}");
    out << nl << "@Override";
    out << nl << "public java.lang.String ice_id()";
    out << sb;
    out << nl << "return ice_staticId();";
    out << eb;

    // Implementation of '_iceWriteImpl'.
    int iter = 0;

    out << sp;
    writeDocComment(
        out,
        "{@inheritDoc}\n"
        "@hidden");
    out << nl << "@Override";
    out << nl << "protected void _iceWriteImpl(com.zeroc.Ice.OutputStream ostr_)";
    out << sb;
    out << nl << "ostr_.startSlice(ice_staticId(), " << p->compactId() << (!base ? ", true" : ", false") << ");";
    for (const auto& member : members)
    {
        if (!member->optional())
        {
            writeMarshalDataMember(out, package, member, iter);
        }
    }
    for (const auto& optionalMember : optionalMembers)
    {
        writeMarshalDataMember(out, package, optionalMember, iter);
    }
    out << nl << "ostr_.endSlice();";
    if (base)
    {
        out << nl << "super._iceWriteImpl(ostr_);";
    }
    out << eb;

    // Implementation of '_iceReadImpl'.
    iter = 0;
    out << sp;
    writeDocComment(
        out,
        "{@inheritDoc}\n"
        "@hidden");
    out << nl << "@Override";
    out << nl << "protected void _iceReadImpl(com.zeroc.Ice.InputStream istr_)";
    out << sb;
    out << nl << "istr_.startSlice();";
    for (const auto& member : members)
    {
        if (!member->optional())
        {
            writeUnmarshalDataMember(out, package, member, iter);
        }
    }
    for (const auto& optionalMember : optionalMembers)
    {
        writeUnmarshalDataMember(out, package, optionalMember, iter);
    }
    out << nl << "istr_.endSlice();";
    if (base)
    {
        out << nl << "super._iceReadImpl(istr_);";
    }
    out << eb;

    // Generate the 'serialVersionUID' field.
    out << sp;
    out << nl << getSerialVersionUID(p);

    out << eb;
    close();
}

bool
Slice::Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string name = p->mappedName();
    ExceptionPtr base = p->base();
    string package = getPackage(p);

    open(getUnqualified(p), p->file());

    Output& out = output();

    out << sp;

    const optional<DocComment>& dc = p->docComment();
    writeDocComment(out, p->unit(), dc);
    if (dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }

    out << nl << "@com.zeroc.Ice.SliceTypeId(value = \"" << p->scoped() << "\")";
    out << nl << "public class " << name << " extends ";

    if (!base)
    {
        out << "com.zeroc.Ice.UserException";
    }
    else
    {
        out << getUnqualified(base, package);
    }
    out << sb;
    return true;
}

void
Slice::Gen::TypesVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    Output& out = output();

    string package = getPackage(p);
    ExceptionPtr base = p->base();

    string name = p->mappedName();
    DataMemberList members = p->dataMembers();
    DataMemberList allDataMembers = p->allDataMembers();
    DataMemberList baseDataMembers;
    if (base)
    {
        baseDataMembers = base->allDataMembers();
    }

    DataMemberList requiredMembers;
    DataMemberList optionalMembers = p->orderedOptionalDataMembers();
    for (const auto& member : allDataMembers)
    {
        if (!member->optional())
        {
            requiredMembers.push_back(member);
        }
    }

    int iter;

    // Default constructor.
    out << sp;
    writeDocComment(out, "Constructs a {@code " + name + "}.");
    out << nl << "public " << name << "()";
    out << sb;
    writeDataMemberInitializers(out, members, package);
    out << eb;

    // Additional constructors.
    if (!allDataMembers.empty())
    {
        // Only generate additional constructors if the parameter list is not too large.
        if (isValidMethodParameterList(allDataMembers))
        {
            if (!requiredMembers.empty() && !optionalMembers.empty())
            {
                bool hasBaseRequired = false;
                for (const auto& member : baseDataMembers)
                {
                    if (!member->optional())
                    {
                        hasBaseRequired = true;
                        break;
                    }
                }

                // Generate a constructor accepting parameters for just the required members.
                out << sp;
                out << nl << "/**";
                out << nl << " * Constructs a {@code " << name
                    << "} with values for all fields not marked optional in the Slice definition for {@code "
                    << p->scoped() << "}.";
                writeParamDocComments(out, requiredMembers);
                out << nl << " */";
                out << nl << "public " << name << spar;
                vector<string> parameters;
                for (const auto& member : requiredMembers)
                {
                    string memberName = member->mappedName();
                    string memberType =
                        typeToString(member->type(), TypeModeMember, package, member->getMetadata(), true, false);
                    parameters.push_back(memberType + " " + memberName);
                }
                out << parameters << epar;

                out << sb;
                if (!baseDataMembers.empty())
                {
                    if (hasBaseRequired)
                    {
                        out << nl << "super" << spar;
                        vector<string> baseParamNames;
                        for (const auto& member : baseDataMembers)
                        {
                            if (!member->optional())
                            {
                                baseParamNames.push_back(member->mappedName());
                            }
                        }
                        out << baseParamNames << epar << ';';
                    }
                }

                for (const auto& member : members)
                {
                    if (!member->optional())
                    {
                        string paramName = member->mappedName();
                        out << nl << "this." << paramName << " = " << paramName << ';';
                    }
                }
                writeDataMemberInitializers(out, optionalMembers, package);
                out << eb;
            }

            //
            // Primary constructor which takes all data members.
            //
            out << sp;
            out << nl << "/**";
            out << nl << " * Constructs a {@code " << name << "} with values for all its fields.";
            writeParamDocComments(out, allDataMembers);
            out << nl << " */";
            out << nl << "public " << name << spar;
            vector<string> parameters;
            for (const auto& member : allDataMembers)
            {
                string memberName = member->mappedName();
                string memberType =
                    typeToString(member->type(), TypeModeMember, package, member->getMetadata(), true, false);
                parameters.push_back(memberType + " " + memberName);
            }
            out << parameters << epar;
            out << sb;
            // Set any base members by calling the super-constructor (if necessary).
            if (base && allDataMembers.size() != members.size())
            {
                out << nl << "super" << spar;
                vector<string> baseParamNames;
                DataMemberList baseDataMemberList = base->allDataMembers();
                for (const auto& member : baseDataMemberList)
                {
                    baseParamNames.push_back(member->mappedName());
                }

                out << baseParamNames << epar << ';';
            }
            // Set any non-base members in the constructor body.
            for (const auto& member : members)
            {
                string paramName = member->mappedName();
                if (member->optional())
                {
                    string capName = paramName;
                    capName[0] = static_cast<char>(toupper(static_cast<unsigned char>(capName[0])));
                    out << nl << "set" << capName << '(' << paramName << ");";
                }
                else
                {
                    out << nl << "this." << paramName << " = " << paramName << ';';
                }
            }
            out << eb;
        }
    }

    out << sp;
    writeDocComment(out, "{@inheritDoc}");
    out << nl << "@Override";
    out << nl << "public java.lang.String ice_id()";
    out << sb;
    out << nl << "return \"" << p->scoped() << "\";";
    out << eb;

    out << sp;
    writeDocComment(
        out,
        "{@inheritDoc}\n"
        "@hidden");
    out << nl << "@Override";
    out << nl << "protected void _writeImpl(com.zeroc.Ice.OutputStream ostr_)";
    out << sb;
    out << nl << "ostr_.startSlice(\"" << p->scoped() << "\", -1, " << (!base ? "true" : "false") << ");";
    iter = 0;
    for (const auto& member : members)
    {
        if (!member->optional())
        {
            writeMarshalDataMember(out, package, member, iter);
        }
    }
    for (const auto& optionalMember : optionalMembers)
    {
        writeMarshalDataMember(out, package, optionalMember, iter);
    }
    out << nl << "ostr_.endSlice();";
    if (base)
    {
        out << nl << "super._writeImpl(ostr_);";
    }
    out << eb;

    out << sp;
    writeDocComment(
        out,
        "{@inheritDoc}\n"
        "@hidden");
    out << nl << "@Override";
    out << nl << "protected void _readImpl(com.zeroc.Ice.InputStream istr_)";
    out << sb;
    out << nl << "istr_.startSlice();";
    iter = 0;
    for (const auto& member : members)
    {
        if (!member->optional())
        {
            writeUnmarshalDataMember(out, package, member, iter);
        }
    }
    for (const auto& optionalMember : optionalMembers)
    {
        writeUnmarshalDataMember(out, package, optionalMember, iter);
    }
    out << nl << "istr_.endSlice();";
    if (base)
    {
        out << nl << "super._readImpl(istr_);";
    }
    out << eb;

    if (p->usesClasses() && !(base && base->usesClasses()))
    {
        out << sp;
        writeDocComment(
            out,
            "@{inheritDoc}\n"
            "@hidden");
        out << nl << "@Override";
        out << nl << "public boolean _usesClasses()";
        out << sb;
        out << nl << "return true;";
        out << eb;
    }

    out << sp;
    out << nl << getSerialVersionUID(p);

    out << eb;
    close();
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    open(getUnqualified(p), p->file());

    Output& out = output();
    out << sp;

    const optional<DocComment>& dc = p->docComment();
    writeDocComment(out, p->unit(), dc);
    if (dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }

    out << nl << "public final class " << p->mappedName() << " implements java.lang.Cloneable, java.io.Serializable";
    out << sb;

    return true;
}

void
Slice::Gen::TypesVisitor::visitStructEnd(const StructPtr& p)
{
    string package = getPackage(p);

    Output& out = output();

    DataMemberList members = p->dataMembers();
    int iter;

    string name = p->mappedName();
    string typeS = typeToString(p, TypeModeIn, package);

    out << sp;
    writeDocComment(out, "Constructs a {@code " + name + "}.");
    out << nl << "public " << name << "()";
    out << sb;
    writeDataMemberInitializers(out, members, package);
    out << eb;

    //
    // Generate constructor if the parameter list is not too large.
    //
    if (isValidMethodParameterList(members))
    {
        vector<string> parameters;
        vector<string> paramNames;
        for (const auto& member : members)
        {
            string memberName = member->mappedName();
            string memberType =
                typeToString(member->type(), TypeModeMember, package, member->getMetadata(), true, false);
            parameters.push_back(memberType + " " + memberName);
            paramNames.push_back(memberName);
        }

        out << sp;
        out << nl << "/**";
        out << nl << " * Constructs a {@code " << name
            << "} with values for all fields not marked optional in the Slice definition for {@code " << p->scoped()
            << "}.";
        writeParamDocComments(out, members);
        out << nl << " */";
        out << nl << "public " << name << spar << parameters << epar;
        out << sb;
        for (const auto& paramName : paramNames)
        {
            out << nl << "this." << paramName << " = " << paramName << ';';
        }
        out << eb;
    }

    out << sp;
    writeDocComment(out, "{@inheritDoc}");
    out << nl << "@Override";
    out << nl << "public boolean equals(java.lang.Object rhs)";
    out << sb;
    out << nl << "if (this == rhs)";
    out << sb;
    out << nl << "return true;";
    out << eb;
    out << nl << typeS << " r = null;";
    out << nl << "if (rhs instanceof " << typeS << ")";
    out << sb;
    out << nl << "r = (" << typeS << ")rhs;";
    out << eb;
    out << sp << nl << "if (r != null)";
    out << sb;
    for (const auto& member : members)
    {
        string memberName = member->mappedName();
        BuiltinPtr b = dynamic_pointer_cast<Builtin>(member->type());
        if (b)
        {
            switch (b->kind())
            {
                case Builtin::KindByte:
                case Builtin::KindBool:
                case Builtin::KindShort:
                case Builtin::KindInt:
                case Builtin::KindLong:
                case Builtin::KindFloat:
                case Builtin::KindDouble:
                {
                    out << nl << "if (this." << memberName << " != r." << memberName << ')';
                    out << sb;
                    out << nl << "return false;";
                    out << eb;
                    break;
                }

                case Builtin::KindString:
                case Builtin::KindObjectProxy:
                case Builtin::KindValue:
                {
                    out << nl << "if (this." << memberName << " != r." << memberName << ')';
                    out << sb;
                    out << nl << "if (this." << memberName << " == null || r." << memberName << " == null || !this."
                        << memberName << ".equals(r." << memberName << "))";
                    out << sb;
                    out << nl << "return false;";
                    out << eb;
                    out << eb;
                    break;
                }
            }
        }
        else
        {
            //
            // We treat sequences differently because the native equals() method for
            // a Java array does not perform a deep comparison. If the mapped type
            // is not overridden via metadata, we use the helper method
            // java.util.Arrays.equals() to compare native arrays.
            //
            // For all other types, we can use the native equals() method.
            //
            SequencePtr seq = dynamic_pointer_cast<Sequence>(member->type());
            if (seq)
            {
                if (hasTypeMetadata(seq, member->getMetadata()))
                {
                    out << nl << "if (this." << memberName << " != r." << memberName << ')';
                    out << sb;
                    out << nl << "if (this." << memberName << " == null || r." << memberName << " == null || !this."
                        << memberName << ".equals(r." << memberName << "))";
                    out << sb;
                    out << nl << "return false;";
                    out << eb;
                    out << eb;
                }
                else
                {
                    //
                    // Arrays.equals() handles null values.
                    //
                    out << nl << "if (!java.util.Arrays.equals(this." << memberName << ", r." << memberName << "))";
                    out << sb;
                    out << nl << "return false;";
                    out << eb;
                }
            }
            else
            {
                out << nl << "if (this." << memberName << " != r." << memberName << ')';
                out << sb;
                out << nl << "if (this." << memberName << " == null || r." << memberName << " == null || !this."
                    << memberName << ".equals(r." << memberName << "))";
                out << sb;
                out << nl << "return false;";
                out << eb;
                out << eb;
            }
        }
    }
    out << sp << nl << "return true;";
    out << eb;
    out << sp << nl << "return false;";
    out << eb;

    out << sp;
    writeDocComment(out, "{@inheritDoc}");
    out << nl << "@Override";
    out << nl << "public int hashCode()";
    out << sb;
    out << nl << "int h_ = 5381;";
    out << nl << "h_ = com.zeroc.Ice.HashUtil.hashAdd(h_, \"" << p->scoped() << "\");";
    for (const auto& member : members)
    {
        out << nl << "h_ = com.zeroc.Ice.HashUtil.hashAdd(h_, " << member->mappedName() << ");";
    }
    out << nl << "return h_;";
    out << eb;

    out << sp;
    writeDocComment(out, "{@inheritDoc}");
    out << nl << "@Override";
    out << nl << "public " << name << " clone()";
    out << sb;
    out << nl << name << " c = null;";
    out << nl << "try";
    out << sb;
    out << nl << "c = (" << name << ") super.clone();";
    out << eb << " catch (java.lang.CloneNotSupportedException ex)";
    out << sb;
    out << nl << "assert false; // impossible";
    out << eb;
    out << nl << "return c;";
    out << eb;

    out << sp;
    out << nl << "/**";
    out << nl << " * Marshals this object's fields into an output stream.";
    out << nl << " *";
    out << nl << " * @param ostr the output stream";
    out << nl << " */";
    out << nl << "public void ice_writeMembers(com.zeroc.Ice.OutputStream ostr)";
    out << sb;
    iter = 0;
    for (const auto& member : members)
    {
        writeMarshalDataMember(out, package, member, iter);
    }
    out << eb;

    out << sp;
    out << nl << "/**";
    out << nl << " * Unmarshals and sets this object's fields from an input stream.";
    out << nl << " *";
    out << nl << " * @param istr the input stream";
    out << nl << " */";
    out << nl << "public void ice_readMembers(com.zeroc.Ice.InputStream istr)";
    out << sb;
    iter = 0;
    for (const auto& member : members)
    {
        writeUnmarshalDataMember(out, package, member, iter);
    }
    out << eb;

    out << sp;
    out << nl << "/**";
    out << nl << " * Marshals " << getArticleFor(name) << " {@code " << name << "} into an output stream.";
    out << nl << " *";
    out << nl << " * @param ostr the output stream";
    out << nl << " * @param v the {@code " << name << "} to marshal; can be null";
    out << nl << " */";
    out << nl << "static public void ice_write(com.zeroc.Ice.OutputStream ostr, " << name << " v)";
    out << sb;
    out << nl << "if (v == null)";
    out << sb;
    out << nl << "_nullMarshalValue.ice_writeMembers(ostr);";
    out << eb << " else";
    out << sb;
    out << nl << "v.ice_writeMembers(ostr);";
    out << eb;
    out << eb;

    out << sp;
    out << nl << "/**";
    out << nl << " * Unmarshals " << getArticleFor(name) << " {@code " << name << "} from an input stream.";
    out << nl << " *";
    out << nl << " * @param istr the input stream ";
    out << nl << " * @return the unmarshaled {@code " << name << "}";
    out << nl << " */";
    out << nl << "static public " << name << " ice_read(com.zeroc.Ice.InputStream istr)";
    out << sb;
    out << nl << name << " v = new " << name << "();";
    out << nl << "v.ice_readMembers(istr);";
    out << nl << "return v;";
    out << eb;

    string optName = "java.util.Optional<" + name + ">";
    out << sp;
    out << nl << "/**";
    out << nl << " * Marshals an optional {@code " << name << "} into an output stream.";
    out << nl << " *";
    out << nl << " * @param ostr the output stream";
    out << nl << " * @param tag the tag";
    out << nl << " * @param v the value to marshal";
    out << nl << " */";
    out << nl << "static public void ice_write(com.zeroc.Ice.OutputStream ostr, int tag, " << optName << " v)";
    out << sb;
    out << nl << "if (v != null && v.isPresent())";
    out << sb;
    out << nl << "ice_write(ostr, tag, v.get());";
    out << eb;
    out << eb;

    out << sp;
    out << nl << "/**";
    out << nl << " * Marshals an optional {@code " << name << "} into an output stream.";
    out << nl << " *";
    out << nl << " * @param ostr the output stream";
    out << nl << " * @param tag the tag";
    out << nl << " * @param v the value to marshal";
    out << nl << " */";
    out << nl << "static public void ice_write(com.zeroc.Ice.OutputStream ostr, int tag, " << name << " v)";
    out << sb;
    out << nl << "if (ostr.writeOptional(tag, " << getOptionalFormat(p) << "))";
    out << sb;
    if (p->isVariableLength())
    {
        out << nl << "int pos = ostr.startSize();";
        out << nl << "ice_write(ostr, v);";
        out << nl << "ostr.endSize(pos);";
    }
    else
    {
        out << nl << "ostr.writeSize(" << p->minWireSize() << ");";
        out << nl << "ice_write(ostr, v);";
    }
    out << eb;
    out << eb;

    out << sp;
    out << nl << "/**";
    out << nl << " * Unmarshals an optional {@code " << name << "} from an input stream.";
    out << nl << " *";
    out << nl << " * @param istr the input stream";
    out << nl << " * @param tag the tag";
    out << nl << " * @return the unmarshaled value";
    out << nl << " */";
    out << nl << "static public " << optName << " ice_read(com.zeroc.Ice.InputStream istr, int tag)";
    out << sb;
    out << nl << "if (istr.readOptional(tag, " << getOptionalFormat(p) << "))";
    out << sb;
    if (p->isVariableLength())
    {
        out << nl << "istr.skip(4);";
    }
    else
    {
        out << nl << "istr.skipSize();";
    }
    out << nl << "return java.util.Optional.of(" << typeS << ".ice_read(istr));";
    out << eb << " else";
    out << sb;
    out << nl << "return java.util.Optional.empty();";
    out << eb;
    out << eb;

    out << sp << nl << "private static final " << name << " _nullMarshalValue = new " << name << "();";

    out << sp;
    out << nl << getSerialVersionUID(p);

    out << eb;
    close();
}

void
Slice::Gen::TypesVisitor::visitDataMember(const DataMemberPtr& p)
{
    const ContainedPtr contained = dynamic_pointer_cast<Contained>(p->container());

    const string name = p->mappedName();
    const bool getSet = p->hasMetadata("java:getset") || contained->hasMetadata("java:getset");
    const bool isOptional = p->optional();
    const TypePtr type = p->type();
    const BuiltinPtr b = dynamic_pointer_cast<Builtin>(type);
    const bool classType = type->isClassType();

    const MetadataList& metadata = p->getMetadata();
    const string s = typeToString(type, TypeModeMember, getPackage(contained), metadata, true, false);

    Output& out = output();

    out << sp;

    const optional<DocComment>& dc = p->docComment();
    writeDocComment(out, p->unit(), dc);
    if (dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }

    if (isOptional)
    {
        out << nl << "private " << s << ' ' << name << ';';
    }
    else
    {
        out << nl << "public " << s << ' ' << name << ';';
    }

    if (isOptional)
    {
        out << nl << "private boolean _" << name << ';';
    }

    //
    // Getter/Setter.
    //
    if (getSet || isOptional)
    {
        string capName = name;
        capName[0] = static_cast<char>(toupper(static_cast<unsigned char>(capName[0])));

        //
        // Getter.
        //
        out << sp;
        writeDocComment(out, p->unit(), dc);
        if (dc && dc->isDeprecated())
        {
            out << nl << "@Deprecated";
        }
        out << nl << "public " << s << " get" << capName << "()";
        out << sb;
        if (isOptional)
        {
            out << nl << "if (!_" << name << ')';
            out << sb;
            out << nl << "throw new java.util.NoSuchElementException(\"" << name << " is not set\");";
            out << eb;
        }
        out << nl << "return " << name << ';';
        out << eb;

        //
        // Setter.
        //
        out << sp;
        writeDocComment(out, p->unit(), dc);
        if (dc && dc->isDeprecated())
        {
            out << nl << "@Deprecated";
        }
        out << nl << "public void set" << capName << '(' << s << " " << name << ')';
        out << sb;
        if (isOptional)
        {
            out << nl << "_" << name << " = true;";
        }
        out << nl << "this." << name << " = " << name << ';';
        out << eb;

        //
        // Generate hasFoo and clearFoo for optional member.
        //
        if (isOptional)
        {
            out << sp;
            writeDocComment(out, p->unit(), dc);
            if (dc && dc->isDeprecated())
            {
                out << nl << "@Deprecated";
            }
            out << nl << "public boolean has" << capName << "()";
            out << sb;
            out << nl << "return _" << name << ';';
            out << eb;

            out << sp;
            writeDocComment(out, p->unit(), dc);
            if (dc && dc->isDeprecated())
            {
                out << nl << "@Deprecated";
            }
            out << nl << "public void clear" << capName << "()";
            out << sb;
            out << nl << "_" << name << " = false;";
            out << eb;

            const string optType = typeToString(type, TypeModeMember, getPackage(contained), metadata, true, true);

            out << sp;
            writeDocComment(out, p->unit(), dc);
            if (dc && dc->isDeprecated())
            {
                out << nl << "@Deprecated";
            }
            out << nl << "public void optional" << capName << '(' << optType << " v)";
            out << sb;
            out << nl << "if (v == null || !v.isPresent())";
            out << sb;
            out << nl << "_" << name << " = false;";
            out << eb << " else";
            out << sb;
            out << nl << "_" << name << " = true;";
            if (b && b->kind() == Builtin::KindInt)
            {
                out << nl << name << " = v.getAsInt();";
            }
            else if (b && b->kind() == Builtin::KindLong)
            {
                out << nl << name << " = v.getAsLong();";
            }
            else if (b && b->kind() == Builtin::KindDouble)
            {
                out << nl << name << " = v.getAsDouble();";
            }
            else
            {
                out << nl << name << " = v.get();";
            }
            out << eb;
            out << eb;

            out << sp;
            writeDocComment(out, p->unit(), dc);
            if (dc && dc->isDeprecated())
            {
                out << nl << "@Deprecated";
            }
            out << nl << "public " << optType << " optional" << capName << "()";
            out << sb;
            out << nl << "if (_" << name << ')';
            out << sb;
            if (classType)
            {
                out << nl << "return java.util.Optional.ofNullable(" << name << ");";
            }
            else if (b && b->kind() == Builtin::KindInt)
            {
                out << nl << "return java.util.OptionalInt.of(" << name << ");";
            }
            else if (b && b->kind() == Builtin::KindLong)
            {
                out << nl << "return java.util.OptionalLong.of(" << name << ");";
            }
            else if (b && b->kind() == Builtin::KindDouble)
            {
                out << nl << "return java.util.OptionalDouble.of(" << name << ");";
            }
            else
            {
                out << nl << "return java.util.Optional.of(" << name << ");";
            }
            out << eb << " else";
            out << sb;
            if (b && b->kind() == Builtin::KindInt)
            {
                out << nl << "return java.util.OptionalInt.empty();";
            }
            else if (b && b->kind() == Builtin::KindLong)
            {
                out << nl << "return java.util.OptionalLong.empty();";
            }
            else if (b && b->kind() == Builtin::KindDouble)
            {
                out << nl << "return java.util.OptionalDouble.empty();";
            }
            else
            {
                out << nl << "return java.util.Optional.empty();";
            }
            out << eb;
            out << eb;
        }

        //
        // Check for bool type.
        //
        if (b && b->kind() == Builtin::KindBool)
        {
            out << sp;
            if (dc && dc->isDeprecated())
            {
                out << nl << "@Deprecated";
            }
            out << nl << "public boolean is" << capName << "()";
            out << sb;
            if (isOptional)
            {
                out << nl << "if (!_" << name << ')';
                out << sb;
                out << nl << "throw new java.util.NoSuchElementException(\"" << name << " is not set\");";
                out << eb;
            }
            out << nl << "return " << name << ';';
            out << eb;
        }

        //
        // Check for unmodified sequence type and emit indexing methods.
        //
        SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
        if (seq)
        {
            if (!hasTypeMetadata(seq, metadata))
            {
                string elem =
                    typeToString(seq->type(), TypeModeMember, getPackage(contained), MetadataList(), true, false);

                //
                // Indexed getter.
                //
                out << sp;
                if (dc && dc->isDeprecated())
                {
                    out << nl << "@Deprecated";
                }
                out << nl << "public " << elem << " get" << capName << "(int index)";
                out << sb;
                if (isOptional)
                {
                    out << nl << "if (!_" << name << ')';
                    out << sb;
                    out << nl << "throw new java.util.NoSuchElementException(\"" << name << " is not set\");";
                    out << eb;
                }
                out << nl << "return this." << name << "[index];";
                out << eb;

                //
                // Indexed setter.
                //
                out << sp;
                if (dc && dc->isDeprecated())
                {
                    out << nl << "@Deprecated";
                }
                out << nl << "public void set" << capName << "(int index, " << elem << " val)";
                out << sb;
                if (isOptional)
                {
                    out << nl << "if (!_" << name << ')';
                    out << sb;
                    out << nl << "throw new java.util.NoSuchElementException(\"" << name << " is not set\");";
                    out << eb;
                }
                out << nl << "this." << name << "[index] = val;";
                out << eb;
            }
        }
    }
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string name = p->mappedName();
    string package = getPackage(p);
    string absolute = getUnqualified(p);
    EnumeratorList enumerators = p->enumerators();

    open(absolute, p->file());

    Output& out = output();

    out << sp;

    const optional<DocComment>& dc = p->docComment();
    writeDocComment(out, p->unit(), dc);
    if (dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }

    out << nl << "public enum " << name;
    out << sb;

    for (auto en = enumerators.begin(); en != enumerators.end(); ++en)
    {
        if (en != enumerators.begin())
        {
            out << ',';
        }
        const optional<DocComment>& edc = (*en)->docComment();
        writeDocComment(out, p->unit(), edc);
        if (edc && edc->isDeprecated())
        {
            out << nl << "@Deprecated";
        }
        out << nl << (*en)->mappedName() << '(' << (*en)->value() << ')';
    }
    out << ';';

    out << sp;
    out << nl << "/**";
    out << nl << " * Returns the integer value of this enumerator.";
    out << nl << " *";
    out << nl << " * @return the integer value of this enumerator";
    out << nl << " */";
    out << nl << "public int value()";
    out << sb;
    out << nl << "return _value;";
    out << eb;

    out << sp;
    out << nl << "/**";
    out << nl << " * Returns the {@code " << name << "} enumerator corresponding to the given integer value.";
    out << nl << " *";
    out << nl << " * @param v the value to match";
    out << nl << " * @return the {@code " << name
        << "} enumerator corresponding to the given integer value, or {@code null} if no such enumerator exists";
    out << nl << " */";
    out << nl << "public static " << name << " valueOf(int v)";
    out << sb;
    out << nl << "switch (v)";
    out << sb;
    out.dec();
    for (const auto& enumerator : enumerators)
    {
        out << nl << "case " << enumerator->value() << ':';
        out.inc();
        out << nl << "return " << enumerator->mappedName() << ';';
        out.dec();
    }
    out.inc();
    out << eb;
    out << nl << "return null;";
    out << eb;

    out << sp << nl << "private " << name << "(int v)";
    out << sb;
    out << nl << "_value = v;";
    out << eb;

    out << sp;
    out << nl << "/**";
    out << nl << " * Marshals this {@code " << name << "} into an output stream.";
    out << nl << " *";
    out << nl << " * @param ostr the output stream";
    out << nl << " */";
    out << nl << "public void ice_write(com.zeroc.Ice.OutputStream ostr)";
    out << sb;
    out << nl << "ostr.writeEnum(_value, " << p->maxValue() << ");";
    out << eb;

    out << sp;
    out << nl << "/**";
    out << nl << " * Marshals " << getArticleFor(name) << " {@code " << name << "} into an output stream.";
    out << nl << " *";
    out << nl << " * @param ostr the output stream";
    out << nl << " * @param v the {@code " << name << "} to marshal";
    out << nl << " */";
    out << nl << "public static void ice_write(com.zeroc.Ice.OutputStream ostr, " << name << " v)";
    out << sb;
    out << nl << "if (v == null)";
    out << sb;
    string firstEnum = enumerators.front()->mappedName();
    out << nl << "ostr.writeEnum(" << absolute << '.' << firstEnum << ".value(), " << p->maxValue() << ");";
    out << eb << " else";
    out << sb;
    out << nl << "ostr.writeEnum(v.value(), " << p->maxValue() << ");";
    out << eb;
    out << eb;

    out << sp;
    out << nl << "/**";
    out << nl << " * Unmarshals " << getArticleFor(name) << " {@code " << name << "} from an input stream.";
    out << nl << " *";
    out << nl << " * @param istr the input stream ";
    out << nl << " * @return the unmarshaled {@code " << name << "}";
    out << nl << " */";
    out << nl << "public static " << name << " ice_read(com.zeroc.Ice.InputStream istr)";
    out << sb;
    out << nl << "int v = istr.readEnum(" << p->maxValue() << ");";
    out << nl << "return validate(v);";
    out << eb;

    string optName = "java.util.Optional<" + name + ">";
    out << sp;
    out << nl << "/**";
    out << nl << " * Marshals an optional {@code " << name << "} into an output stream.";
    out << nl << " *";
    out << nl << " * @param ostr the output stream";
    out << nl << " * @param tag the tag";
    out << nl << " * @param v the value to marshal";
    out << nl << " */";
    out << nl << "public static void ice_write(com.zeroc.Ice.OutputStream ostr, int tag, " << optName << " v)";
    out << sb;
    out << nl << "if (v != null && v.isPresent())";
    out << sb;
    out << nl << "ice_write(ostr, tag, v.get());";
    out << eb;
    out << eb;

    out << sp;
    out << nl << "/**";
    out << nl << " * Marshals an optional {@code " << name << "} into an output stream.";
    out << nl << " *";
    out << nl << " * @param ostr the output stream";
    out << nl << " * @param tag the tag";
    out << nl << " * @param v the value to marshal";
    out << nl << " */";
    out << nl << "public static void ice_write(com.zeroc.Ice.OutputStream ostr, int tag, " << name << " v)";
    out << sb;
    out << nl << "if (ostr.writeOptional(tag, " << getOptionalFormat(p) << "))";
    out << sb;
    out << nl << "ice_write(ostr, v);";
    out << eb;
    out << eb;

    out << sp;
    out << nl << "/**";
    out << nl << " * Unmarshals an optional {@code " << name << "} from an input stream.";
    out << nl << " *";
    out << nl << " * @param istr the input stream";
    out << nl << " * @param tag the tag";
    out << nl << " * @return the unmarshaled value";
    out << nl << " */";
    out << nl << "public static " << optName << " ice_read(com.zeroc.Ice.InputStream istr, int tag)";
    out << sb;
    out << nl << "if (istr.readOptional(tag, " << getOptionalFormat(p) << "))";
    out << sb;
    out << nl << "return java.util.Optional.of(ice_read(istr));";
    out << eb << " else";
    out << sb;
    out << nl << "return java.util.Optional.empty();";
    out << eb;
    out << eb;

    out << sp << nl << "private static " << name << " validate(int v)";
    out << sb;
    out << nl << "final " << name << " e = valueOf(v);";
    out << nl << "if (e == null)";
    out << sb;
    out << nl << R"(throw new com.zeroc.Ice.MarshalException("enumerator value " + v + " is out of range");)";
    out << eb;
    out << nl << "return e;";
    out << eb;

    out << sp << nl << "private final int _value;";

    out << eb;
    close();
}

void
Slice::Gen::TypesVisitor::visitSequence(const SequencePtr& p)
{
    bool mappedToCustomType = p->hasMetadata("java:type");
    if (mapsToJavaBuiltinType(p->type()) && !mappedToCustomType)
    {
        return; // No helpers for sequences of primitive types (that aren't re-mapped with 'java:type').
    }

    string name = p->mappedName();
    string helper = getUnqualified(p) + "Helper";
    string package = getPackage(p);
    string typeS = typeToString(p, TypeModeIn, package);

    //
    // We cannot allocate an array of a generic type, such as
    //
    // arr = new Map<String, String>[sz];
    //
    // Attempting to compile this code results in a "generic array creation" error
    // message. This problem can occur when the sequence's element type is a
    // dictionary, or when the element type is a nested sequence that uses a custom
    // mapping.
    //
    // The solution is to rewrite the code as follows:
    //
    // arr = (Map<String, String>[])new Map[sz];
    //
    // Unfortunately, this produces an unchecked warning during compilation, so we
    // annotate the read() method to suppress the warning.
    //
    // A simple test is to look for a "<" character in the content type, which
    // indicates the use of a generic type.
    //
    bool suppressUnchecked = false;

    string instanceType, formalType;
    bool customType = getSequenceTypes(p, "", MetadataList(), instanceType, formalType);

    if (!customType)
    {
        //
        // Determine sequence depth.
        //
        TypePtr origContent = p->type();
        SequencePtr s = dynamic_pointer_cast<Sequence>(origContent);
        while (s)
        {
            //
            // Stop if the inner sequence type has a custom or serializable type.
            //
            if (hasTypeMetadata(s))
            {
                break;
            }
            origContent = s->type();
            s = dynamic_pointer_cast<Sequence>(origContent);
        }

        string origContentS = typeToString(origContent, TypeModeIn, package);
        suppressUnchecked = origContentS.find('<') != string::npos;
    }

    open(helper, p->file());
    Output& out = output();

    int iter;

    out << sp;
    writeDocComment(out, "Helper class for marshaling/unmarshaling " + name + ".");
    out << nl << "public final class " << name << "Helper";
    out << sb;

    out << nl << "/**";
    out << nl << " * Marshals a list of {@code " << name << "} into an output stream.";
    out << nl << " *";
    out << nl << " * @param ostr the output stream";
    out << nl << " * @param v the list to marshal";
    out << nl << " */";
    out << nl << "public static void write(com.zeroc.Ice.OutputStream ostr, " << typeS << " v)";
    out << sb;
    iter = 0;
    writeSequenceMarshalUnmarshalCode(out, package, p, "v", true, iter, false);
    out << eb;

    out << sp;
    out << nl << "/**";
    out << nl << " * Unmarshals a list of {@code " << name << "} from an input stream.";
    out << nl << " *";
    out << nl << " * @param istr the input stream";
    out << nl << " * @return the list";
    out << nl << " */";
    if (suppressUnchecked)
    {
        out << nl << "@SuppressWarnings(\"unchecked\")";
    }
    out << nl << "public static " << typeS << " read(com.zeroc.Ice.InputStream istr)";
    out << sb;
    out << nl << "final " << typeS << " v;";
    iter = 0;
    writeSequenceMarshalUnmarshalCode(out, package, p, "v", false, iter, false);
    out << nl << "return v;";
    out << eb;

    string optTypeS = "java.util.Optional<" + typeS + ">";
    out << sp;
    out << nl << "/**";
    out << nl << " * Marshals an optional list of {@code " << name << "} into an output stream.";
    out << nl << " *";
    out << nl << " * @param ostr the output stream";
    out << nl << " * @param tag the tag";
    out << nl << " * @param v the list to marshal";
    out << nl << " */";
    out << nl << "public static void write(com.zeroc.Ice.OutputStream ostr, int tag, " << optTypeS << " v)";
    out << sb;
    out << nl << "if (v != null && v.isPresent())";
    out << sb;
    out << nl << "write(ostr, tag, v.get());";
    out << eb;
    out << eb;

    out << sp;
    out << nl << "/**";
    out << nl << " * Marshals an optional list of {@code " << name << "} into an output stream.";
    out << nl << " *";
    out << nl << " * @param ostr the output stream";
    out << nl << " * @param tag the tag";
    out << nl << " * @param v the list to marshal";
    out << nl << " */";
    out << nl << "public static void write(com.zeroc.Ice.OutputStream ostr, int tag, " << typeS << " v)";
    out << sb;
    out << nl << "if (ostr.writeOptional(tag, " << getOptionalFormat(p) << "))";
    out << sb;
    if (p->type()->isVariableLength())
    {
        out << nl << "int pos = ostr.startSize();";
        writeSequenceMarshalUnmarshalCode(out, package, p, "v", true, iter, true);
        out << nl << "ostr.endSize(pos);";
    }
    else
    {
        // The sequence is an instance of java.util.List<E>, where E is a fixed-size type.
        // If the element type is bool or byte, we do NOT write an extra size.
        const size_t sz = p->type()->minWireSize();
        if (sz > 1)
        {
            out << nl << "final int optSize = v == null ? 0 : ";
            out << (mappedToCustomType ? "v.size();" : "v.length;");
            out << nl << "ostr.writeSize(optSize > 254 ? optSize * " << sz << " + 5 : optSize * " << sz << " + 1);";
        }
        writeSequenceMarshalUnmarshalCode(out, package, p, "v", true, iter, true);
    }
    out << eb;
    out << eb;

    out << sp;
    out << nl << "/**";
    out << nl << " * Unmarshals an optional list of {@code " << name << "} from an input stream.";
    out << nl << " *";
    out << nl << " * @param istr the input stream";
    out << nl << " * @param tag the tag";
    out << nl << " * @return the list";
    out << nl << " */";
    out << nl << "public static " << optTypeS << " read(com.zeroc.Ice.InputStream istr, int tag)";
    out << sb;
    out << nl << "if (istr.readOptional(tag, " << getOptionalFormat(p) << "))";
    out << sb;
    if (p->type()->isVariableLength())
    {
        out << nl << "istr.skip(4);";
    }
    else if (p->type()->minWireSize() > 1)
    {
        out << nl << "istr.skipSize();";
    }
    out << nl << typeS << " v;";
    writeSequenceMarshalUnmarshalCode(out, package, p, "v", false, iter, true);
    out << nl << "return java.util.Optional.of(v);";
    out << eb << " else";
    out << sb;
    out << nl << "return java.util.Optional.empty();";
    out << eb;
    out << eb;

    out << eb;
    close();
}

void
Slice::Gen::TypesVisitor::visitDictionary(const DictionaryPtr& p)
{
    string name = p->mappedName();
    string helper = getUnqualified(p) + "Helper";
    string package = getPackage(p);
    string formalType = typeToString(p, TypeModeIn, package, MetadataList(), true);

    open(helper, p->file());
    Output& out = output();

    int iter;

    out << sp;
    writeDocComment(out, "Helper class for marshaling/unmarshaling " + name + ".");
    out << nl << "public final class " << name << "Helper";
    out << sb;

    out << nl << "/**";
    out << nl << " * Marshals a dictionary of {@code " << name << "} into an output stream.";
    out << nl << " *";
    out << nl << " * @param ostr the output stream";
    out << nl << " * @param v the dictionary to marshal";
    out << nl << " */";
    out << nl << "public static void write(com.zeroc.Ice.OutputStream ostr, " << formalType << " v)";
    out << sb;
    iter = 0;
    writeDictionaryMarshalUnmarshalCode(out, package, p, "v", true, iter, false);
    out << eb;

    out << sp;
    out << nl << "/**";
    out << nl << " * Unmarshals a dictionary of {@code " << name << "} from an input stream.";
    out << nl << " *";
    out << nl << " * @param istr the input stream";
    out << nl << " * @return the dictionary";
    out << nl << " */";
    out << nl << "public static " << formalType << " read(com.zeroc.Ice.InputStream istr)";
    out << sb;
    out << nl << formalType << " v;";
    iter = 0;
    writeDictionaryMarshalUnmarshalCode(out, package, p, "v", false, iter, false);
    out << nl << "return v;";
    out << eb;

    string optTypeS = "java.util.Optional<" + formalType + ">";
    out << sp;
    out << nl << "/**";
    out << nl << " * Marshals an optional dictionary of {@code " << name << "} into an output stream.";
    out << nl << " *";
    out << nl << " * @param ostr the output stream";
    out << nl << " * @param tag the tag";
    out << nl << " * @param v the dictionary to marshal";
    out << nl << " */";
    out << nl << "public static void write(com.zeroc.Ice.OutputStream ostr, int tag, " << optTypeS << " v)";
    out << sb;
    out << nl << "if (v != null && v.isPresent())";
    out << sb;
    out << nl << "write(ostr, tag, v.get());";
    out << eb;
    out << eb;

    out << sp;
    out << nl << "/**";
    out << nl << " * Marshals an optional dictionary of {@code " << name << "} into an output stream.";
    out << nl << " *";
    out << nl << " * @param ostr the output stream";
    out << nl << " * @param tag the tag";
    out << nl << " * @param v the dictionary to marshal";
    out << nl << " */";
    out << nl << "public static void write(com.zeroc.Ice.OutputStream ostr, int tag, " << formalType << " v)";
    out << sb;
    out << nl << "if (ostr.writeOptional(tag, " << getOptionalFormat(p) << "))";
    out << sb;
    TypePtr keyType = p->keyType();
    TypePtr valueType = p->valueType();
    if (keyType->isVariableLength() || valueType->isVariableLength())
    {
        out << nl << "int pos = ostr.startSize();";
        writeDictionaryMarshalUnmarshalCode(out, package, p, "v", true, iter, true);
        out << nl << "ostr.endSize(pos);";
    }
    else
    {
        const size_t sz = keyType->minWireSize() + valueType->minWireSize();
        out << nl << "final int optSize = v == null ? 0 : v.size();";
        out << nl << "ostr.writeSize(optSize > 254 ? optSize * " << sz << " + 5 : optSize * " << sz << " + 1);";
        writeDictionaryMarshalUnmarshalCode(out, package, p, "v", true, iter, true);
    }
    out << eb;
    out << eb;

    out << sp;
    out << nl << "/**";
    out << nl << " * Unmarshals an optional dictionary of {@code " << name << "} from an input stream.";
    out << nl << " *";
    out << nl << " * @param istr the input stream";
    out << nl << " * @param tag the tag";
    out << nl << " * @return the dictionary";
    out << nl << " */";
    out << nl << "public static " << optTypeS << " read(com.zeroc.Ice.InputStream istr, int tag)";
    out << sb;
    out << nl << "if (istr.readOptional(tag, " << getOptionalFormat(p) << "))";
    out << sb;
    if (keyType->isVariableLength() || valueType->isVariableLength())
    {
        out << nl << "istr.skip(4);";
    }
    else
    {
        out << nl << "istr.skipSize();";
    }
    out << nl << formalType << " v;";
    writeDictionaryMarshalUnmarshalCode(out, package, p, "v", false, iter, true);
    out << nl << "return java.util.Optional.of(v);";
    out << eb << " else";
    out << sb;
    out << nl << "return java.util.Optional.empty();";
    out << eb;
    out << eb;

    out << eb;
    close();
}

void
Slice::Gen::TypesVisitor::visitConst(const ConstPtr& p)
{
    string package = getPackage(p);
    TypePtr type = p->type();

    open(getUnqualified(p), p->file());

    Output& out = output();

    out << sp;

    const optional<DocComment>& dc = p->docComment();
    writeDocComment(out, p->unit(), dc);
    if (dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }

    out << nl << "public interface " << p->mappedName();
    out << sb;
    out << nl << "/** The value of Slice constant '" << p->scoped() << "' */";
    out << nl << typeToString(type, TypeModeIn, package) << " value = ";
    writeConstantValue(out, type, p->valueType(), p->value(), package);
    out << ';' << eb;
    close();
}

bool
Slice::Gen::TypesVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    InterfaceList bases = p->bases();
    string package = getPackage(p);

    open(getUnqualified(p) + "Prx", p->file());

    Output& out = output();

    // Generate a Java interface as the user-visible type
    out << sp;
    const optional<DocComment>& dc = p->docComment();
    writeDocComment(out, p->unit(), dc);
    if (dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }
    out << nl << "@com.zeroc.Ice.SliceTypeId(value = \"" << p->scoped() << "\")";
    out << nl << "public interface " << p->mappedName() << "Prx extends ";
    if (bases.empty())
    {
        out << "com.zeroc.Ice.ObjectPrx";
    }
    else
    {
        out.spar("");
        for (const auto& base : bases)
        {
            out << getUnqualified(base, package) + "Prx";
        }
        out.epar("");
    }

    out << sb;

    return true;
}

void
Slice::Gen::TypesVisitor::visitInterfaceDefEnd(const InterfaceDefPtr& p)
{
    Output& out = output();

    const string contextParam = "java.util.Map<java.lang.String, java.lang.String> context";
    const string prxName = p->mappedName() + "Prx";
    const string prxIName = "_" + prxName + "I";

    out << sp;
    writeDocComment(
        out,
        "Creates a new proxy that implements {@link " + prxName +
            "}.\n"
            "@param communicator The communicator of the new proxy.\n"
            "@param proxyString The string representation of the proxy.\n"
            "@return The new proxy.");
    out << nl << "static " << prxName
        << " createProxy(com.zeroc.Ice.Communicator communicator, java.lang.String proxyString)";
    out << sb;
    out << nl << "return new " << prxIName << "(com.zeroc.Ice.ObjectPrx.createProxy(communicator, proxyString));";
    out << eb;

    out << sp;
    writeDocComment(
        out,
        "Contacts the remote server to verify that the object implements this type.\n"
        "Raises a local exception if a communication error occurs.\n"
        "@param obj The untyped proxy.\n"
        "@return A proxy for this type, or null if the object does not support this type.");
    out << nl << "static " << prxName << " checkedCast(com.zeroc.Ice.ObjectPrx obj)";
    out << sb;
    out << nl << "return checkedCast(obj, noExplicitContext);";
    out << eb;

    out << sp;
    writeDocComment(
        out,
        "Contacts the remote server to verify that the object implements this type.\n"
        "Raises a local exception if a communication error occurs.\n"
        "@param obj The untyped proxy.\n"
        "@param context The Context map to send with the invocation.\n"
        "@return A proxy for this type, or null if the object does not support this type.");
    out << nl << "static " << prxName << " checkedCast(com.zeroc.Ice.ObjectPrx obj, " << contextParam << ')';
    out << sb;
    out << nl << "return (obj != null && obj.ice_isA(ice_staticId(), context)) ? new " << prxIName << "(obj) : null;";
    out << eb;

    out << sp;
    writeDocComment(
        out,
        "Contacts the remote server to verify that a facet of the object implements this type.\n"
        "Raises a local exception if a communication error occurs.\n"
        "@param obj The untyped proxy.\n"
        "@param facet The name of the desired facet.\n"
        "@return A proxy for this type, or null if the object does not support this type.");
    out << nl << "static " << prxName << " checkedCast(com.zeroc.Ice.ObjectPrx obj, java.lang.String facet)";
    out << sb;
    out << nl << "return checkedCast(obj, facet, noExplicitContext);";
    out << eb;

    out << sp;
    writeDocComment(
        out,
        "Contacts the remote server to verify that a facet of the object implements this type.\n"
        "Raises a local exception if a communication error occurs.\n"
        "@param obj The untyped proxy.\n"
        "@param facet The name of the desired facet.\n"
        "@param context The Context map to send with the invocation.\n"
        "@return A proxy for this type, or null if the object does not support this type.");
    out << nl << "static " << prxName << " checkedCast(com.zeroc.Ice.ObjectPrx obj, java.lang.String facet, "
        << contextParam << ')';
    out << sb;
    out << nl << "return (obj == null) ? null : checkedCast(obj.ice_facet(facet), context);";
    out << eb;

    out << sp;
    writeDocComment(
        out,
        "Downcasts the given proxy to this type without contacting the remote server.\n"
        "@param obj The untyped proxy.\n"
        "@return A proxy for this type.");
    out << nl << "static " << prxName << " uncheckedCast(com.zeroc.Ice.ObjectPrx obj)";
    out << sb;
    out << nl << "return (obj == null) ? null : new " << prxIName << "(obj);";
    out << eb;

    out << sp;
    writeDocComment(
        out,
        "Downcasts the given proxy to this type without contacting the remote server.\n"
        "@param obj The untyped proxy.\n"
        "@param facet The name of the desired facet.\n"
        "@return A proxy for this type.");
    out << nl << "static " << prxName << " uncheckedCast(com.zeroc.Ice.ObjectPrx obj, java.lang.String facet)";
    out << sb;
    out << nl << "return (obj == null) ? null : new " << prxIName << "(obj.ice_facet(facet));";
    out << eb;

    // Generate overrides for all the methods on `ObjectPrx` with covariant return types.
    static constexpr string_view objectPrxMethods[] = {
        "ice_context(java.util.Map<java.lang.String, java.lang.String> newContext)",
        "ice_adapterId(java.lang.String newAdapterId)",
        "ice_endpoints(com.zeroc.Ice.Endpoint[] newEndpoints)",
        "ice_locatorCacheTimeout(int newTimeout)",
        "ice_invocationTimeout(int newTimeout)",
        "ice_connectionCached(boolean newCache)",
        "ice_endpointSelection(com.zeroc.Ice.EndpointSelectionType newType)",
        "ice_secure(boolean b)",
        "ice_encodingVersion(com.zeroc.Ice.EncodingVersion e)",
        "ice_preferSecure(boolean b)",
        "ice_router(com.zeroc.Ice.RouterPrx router)",
        "ice_locator(com.zeroc.Ice.LocatorPrx locator)",
        "ice_collocationOptimized(boolean b)",
        "ice_twoway()",
        "ice_oneway()",
        "ice_batchOneway()",
        "ice_datagram()",
        "ice_batchDatagram()",
        "ice_compress(boolean co)",
        "ice_connectionId(java.lang.String connectionId)",
        "ice_fixed(com.zeroc.Ice.Connection connection)",
    };
    for (const auto& method : objectPrxMethods)
    {
        out << sp;
        out << nl << "@Override";
        out << nl << prxName << " " << method << ";";
    }

    out << sp;
    out << nl << "/**";
    out << nl << " * Gets the type ID of the associated Slice interface.";
    out << nl << " *";
    out << nl << " * @return the string \"" << p->scoped() << "\"";
    out << nl << " */";
    out << nl << "static java.lang.String ice_staticId()";
    out << sb;
    out << nl << "return \"" << p->scoped() << "\";";
    out << eb;

    out << sp;
    writeDocComment(out, "@hidden");
    out << nl << "@Override";
    out << nl << "default " << prxName << " _newInstance(com.zeroc.Ice.Reference ref)";
    out << sb;
    out << nl << "return new " << prxIName << "(ref);";
    out << eb;

    out << eb;
    close();

    // Add a '_' prefix to the file name. We want this '_' to come after any package scopes.
    string absolute = getUnqualified(p) + "PrxI";
    auto scopePos = absolute.rfind('.');
    scopePos = (scopePos == string::npos ? 0 : scopePos + 1);
    absolute.insert(scopePos, 1, '_');
    open(absolute, p->file());

    Output& outi = output();

    outi << sp;
    writeHiddenDocComment(outi);
    const optional<DocComment>& dc = p->docComment();
    if (dc && dc->isDeprecated())
    {
        outi << nl << "@Deprecated";
    }
    outi << nl << "public class " << prxIName;
    outi << " extends com.zeroc.Ice._ObjectPrxFactoryMethods<" << prxName << ">";
    outi << " implements " << prxName;
    outi << sb;

    // Constructor which directly takes a Reference.
    outi << sp;
    outi << nl << prxIName << "(com.zeroc.Ice.Reference ref)";
    outi << sb;
    outi << nl << "super(ref);";
    outi << eb;

    // Copy constructor
    outi << sp;
    outi << nl << prxIName << "(com.zeroc.Ice.ObjectPrx obj)";
    outi << sb;
    outi << nl << "super(obj);";
    outi << eb;

    outi << sp;
    outi << nl << "private static final long serialVersionUID = 0L;";
    outi << eb;
    close();
}

void
Slice::Gen::TypesVisitor::visitOperation(const OperationPtr& p)
{
    Output& out = output();

    const string package = getPackage(p->interface());
    const vector<string> params = getParamsProxy(p, package, false);
    const vector<string> paramsOpt = getParamsProxy(p, package, true);
    const bool sendsOptionals = p->sendsOptionals();
    const optional<DocComment>& dc = p->docComment();

    // Arrange exceptions into most-derived to least-derived order. If we don't
    // do this, a base exception handler can appear before a derived exception
    // handler, causing compiler warnings and resulting in the base exception
    // being marshaled instead of the derived exception.
    ExceptionList throws = p->throws();
    throws.sort(Slice::DerivedToBaseCompare());

    // Synchronous methods with required parameters.
    writeSyncIceInvokeMethods(out, p, params, throws, dc);
    if (sendsOptionals)
    {
        // Synchronous methods using optional parameters (if any).
        writeSyncIceInvokeMethods(out, p, paramsOpt, throws, dc);
    }

    // Asynchronous methods with required parameters.
    writeAsyncIceInvokeMethods(out, p, params, dc);
    if (sendsOptionals)
    {
        // Asynchronous methods with optional parameters.
        writeAsyncIceInvokeMethods(out, p, paramsOpt, dc);
    }

    writeIceIHelperMethods(out, p, !throws.empty(), false);
    if (sendsOptionals)
    {
        writeIceIHelperMethods(out, p, !throws.empty(), true);
    }

    // Generate a list of exceptions that can be thrown by this operation.
    if (!throws.empty())
    {
        out << sp;
        writeHiddenDocComment(out);
        out << nl << "static final java.lang.Class<?>[] _iceE_" << p->mappedName() << " =";
        out << sb;
        for (auto t = throws.begin(); t != throws.end(); ++t)
        {
            if (t != throws.begin())
            {
                out << ",";
            }
            out << nl << getUnqualified(*t, package) << ".class";
        }
        out << eb << ';';
    }
}

Slice::Gen::ServantVisitor::ServantVisitor(const string& dir) : JavaVisitor(dir) {}

bool
Slice::Gen::ServantVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    InterfaceList bases = p->bases();

    string package = getPackage(p);
    open(getUnqualified(p), p->file());

    Output& out = output();

    out << sp;
    const optional<DocComment>& dc = p->docComment();
    writeDocComment(out, p->unit(), dc);

    out << nl << "@com.zeroc.Ice.SliceTypeId(value = \"" << p->scoped() << "\")";
    out << nl << "public interface " << p->mappedName() << " extends ";
    if (bases.empty())
    {
        out << "com.zeroc.Ice.Object";
    }
    else
    {
        out.spar("");
        for (const auto& base : bases)
        {
            out << getUnqualified(base, package);
        }
        out.epar("");
    }
    out << sb;
    return true;
}

void
Slice::Gen::ServantVisitor::visitInterfaceDefEnd(const InterfaceDefPtr& p)
{
    Output& out = output();

    //
    // Generate the dispatch code.
    //

    const string name = p->mappedName();
    const string package = getPackage(p);
    const OperationList ops = p->operations();

    for (const auto& op : ops)
    {
        vector<string> params = getParamsProxy(op, package, true);
        const string currentParam = "com.zeroc.Ice.Current " + getEscapedParamName(op->parameters(), "current");
        const string opName = op->mappedName();
        const bool amd = p->hasMetadata("amd") || op->hasMetadata("amd");

        ExceptionList throws = op->throws();

        out << sp;
        writeServantOpDocComment(out, op, package, amd);

        if (amd)
        {
            out << nl << "java.util.concurrent.CompletionStage<" << getResultType(op, package, true, true) << "> "
                << opName << "Async" << spar << params << currentParam << epar;
            writeThrowsClause(out, package, throws, op);
            out << ';';
        }
        else
        {
            out << nl << getResultType(op, package, false, true) << ' ' << opName << spar << params << currentParam
                << epar;
            writeThrowsClause(out, package, throws, op);
            out << ';';
        }
    }

    out << sp;
    out << nl << "/**";
    out << nl << " * Gets the type ID of the associated Slice interface.";
    out << nl << " *";
    out << nl << " * @return the string \"" << p->scoped() << "\"";
    out << nl << " */";
    out << nl << "static java.lang.String ice_staticId()";
    out << sb;

    out << nl << "return \"" << p->scoped() << "\";";
    out << eb;

    // Dispatch methods. We only generate methods for operations
    // defined in this InterfaceDef, because we reuse existing methods
    // for inherited operations.
    for (const auto& op : ops)
    {
        string opName = op->mappedName();
        out << sp;

        bool throwsUserException = !op->throws().empty() || op->hasMetadata("java:UserException");

        out << nl << "/**";
        out << nl << " * Dispatches the operation " << opName << ".";
        out << nl << " *";
        out << nl << " * @param obj the servant object";
        out << nl << " * @param request the incoming request";
        out << nl << " * @return a {@code CompletionStage} that will complete when the dispatch is done";
        if (throwsUserException)
        {
            out << nl << " * @throws com.zeroc.Ice.UserException in the event of a user exception";
        }
        out << nl << " * @hidden";
        out << nl << " */";

        out << nl << "static java.util.concurrent.CompletionStage<com.zeroc.Ice.OutgoingResponse> _iceD_" << opName
            << '(' << name << " obj, com.zeroc.Ice.IncomingRequest request)";
        if (throwsUserException)
        {
            out.inc();
            out << nl << "throws com.zeroc.Ice.UserException";
            out.dec();
        }
        out << sb;

        const bool amd = p->hasMetadata("amd") || op->hasMetadata("amd");
        const ParameterList inParams = op->inParameters();

        if (op->mode() == Operation::Mode::Normal)
        {
            out << nl << "request.current.checkNonIdempotent();";
        }

        if (!inParams.empty())
        {
            ParameterList values;

            //
            // Declare 'in' parameters.
            //
            out << nl << "com.zeroc.Ice.InputStream istr = request.inputStream;";
            out << nl << "istr.startEncapsulation();";
            for (const auto& param : inParams)
            {
                const TypePtr paramType = param->type();
                if (paramType->isClassType())
                {
                    assert(!param->optional()); // Optional classes are disallowed by the parser.
                    allocatePatcher(out, paramType, package, "icePP_" + param->mappedName());
                    values.push_back(param);
                }
                else
                {
                    const string typeS =
                        typeToString(paramType, TypeModeIn, package, param->getMetadata(), true, param->optional());
                    out << nl << typeS << " iceP_" << param->mappedName() << ';';
                }
            }

            //
            // Unmarshal 'in' parameters.
            //
            int iter = 0;
            for (const auto& param : op->sortedInParameters())
            {
                const string paramName = (param->type()->isClassType() ? ("icePP_") : "iceP_") + param->mappedName();
                const string patchParams = getPatcher(param->type(), package, paramName + ".value");
                writeMarshalUnmarshalCode(
                    out,
                    package,
                    param->type(),
                    param->optional(),
                    param->optional(),
                    param->tag(),
                    paramName,
                    false,
                    iter,
                    "",
                    param->getMetadata(),
                    patchParams);
            }
            if (op->sendsClasses())
            {
                out << nl << "istr.readPendingValues();";
            }
            out << nl << "istr.endEncapsulation();";

            for (const auto& value : values)
            {
                out << nl;
                out << typeToString(value->type(), TypeModeIn, package, value->getMetadata(), true, value->optional());
                out << " iceP_" << value->mappedName() << " = icePP_" << value->mappedName() << ".value;";
            }
        }
        else
        {
            out << nl << "request.inputStream.skipEmptyEncapsulation();";
        }

        vector<string> inArgs = getInArgs(op, true);
        string retS = getResultType(op, package, false, true);

        if (op->hasMarshaledResult())
        {
            if (amd)
            {
                out << nl << "var result = obj." << opName << "Async" << spar << inArgs << "request.current" << epar
                    << ";";
                out << nl << "return result.thenApply(r -> new com.zeroc.Ice.OutgoingResponse(r.getOutputStream()));";
            }
            else
            {
                out << nl << "var result = obj." << opName << spar << inArgs << "request.current" << epar << ";";
                out << nl
                    << "return java.util.concurrent.CompletableFuture.completedFuture(new "
                       "com.zeroc.Ice.OutgoingResponse(result.getOutputStream()));";
            }
        }
        else if (amd)
        {
            out << nl << "var result = obj." << opName << "Async" << spar << inArgs << "request.current" << epar << ";";
            if (retS == "void")
            {
                out << nl << "return result.thenApply(r -> request.current.createEmptyOutgoingResponse());";
            }
            else
            {
                out << nl << "return result.thenApply(r -> request.current.createOutgoingResponse(";
                out.inc();
                out << nl << "r,";
                out << nl << "(ostr, value) -> ";
                out << sb;
                writeMarshalServantResults(out, package, op, "value");
                out << eb;
                out << ",";
                out << nl << opFormatTypeToString(op) << "));";
                out.dec();
            }
        }
        else
        {
            out << nl;
            const bool returnsAnyValues = op->returnsAnyValues();
            if (returnsAnyValues)
            {
                out << retS << " ret = ";
            }
            out << "obj." << opName << spar << inArgs << "request.current" << epar << ';';

            if (returnsAnyValues)
            {
                out << nl << "var ostr = request.current.startReplyStream();";
                out << nl << "ostr.startEncapsulation(request.current.encoding, " << opFormatTypeToString(op) << ");";
                writeMarshalServantResults(out, package, op, "ret");
                out << nl << "ostr.endEncapsulation();";
                out << nl
                    << "return java.util.concurrent.CompletableFuture.completedFuture(new "
                       "com.zeroc.Ice.OutgoingResponse(ostr));";
            }
            else
            {
                out << nl
                    << "return "
                       "java.util.concurrent.CompletableFuture.completedFuture(request.current."
                       "createEmptyOutgoingResponse());";
            }
        }
        out << eb;
    }

    OperationList allOps = p->allOperations();
    if (!allOps.empty())
    {
        out << sp;
        out << nl << "/**";
        out << nl
            << " * Dispatches an incoming request to one of the methods of this generated interface, based on the "
               "operation name carried by the request.";
        out << nl << " *";
        out << nl << " * @param request the incoming request";
        out << nl << " * @return the outgoing response";
        out << nl
            << " * @throws com.zeroc.Ice.UserException if a {@code UserException} is thrown, Ice will marshal it as "
               "the response payload.";
        out << nl << " */";
        out << nl << "@Override";
        out << nl << "default java.util.concurrent.CompletionStage<com.zeroc.Ice.OutgoingResponse> dispatch("
            << "com.zeroc.Ice.IncomingRequest request)";
        out.inc();
        out << nl << "throws com.zeroc.Ice.UserException";
        out.dec();
        out << sb;
        out << nl << "return switch (request.current.operation)";
        out << sb;
        for (const auto& op : allOps)
        {
            out << nl << "case \"" << op->name() << "\" -> " << getUnqualified(op->interface(), package) << "._iceD_"
                << op->mappedName() << "(this, request);";
        }
        for (const auto& opName : {"ice_id", "ice_ids", "ice_isA", "ice_ping"})
        {
            out << nl << "case \"" << opName << "\" -> com.zeroc.Ice.Object._iceD_" << opName << "(this, request);";
        }
        out << nl << "default -> throw new com.zeroc.Ice.OperationNotExistException();";
        out << eb;
        out << ";";
        out << eb;
    }

    out << eb;
    close();
}

void
Slice::Gen::ServantVisitor::visitOperation(const OperationPtr& p)
{
    //
    // Generate the operation signature for a servant.
    //

    InterfaceDefPtr interface = dynamic_pointer_cast<InterfaceDef>(p->container());
    assert(interface);

    const string package = getPackage(interface);

    Output& out = output();

    const optional<DocComment>& dc = p->docComment();

    // Generate the "Result" type needed by operations that return multiple values.
    if (p->returnsMultipleValues())
    {
        writeResultType(out, p, package, dc);
    }

    // The "MarshaledResult" type is generated in the servant interface.
    if (p->hasMarshaledResult())
    {
        writeMarshaledResultType(out, p, package, dc);
    }
}
