// Copyright (c) ZeroC, Inc.

#include "IceRpcVisitors.h"
#include "../Slice/Util.h"
#include "CsUtil.h"
#include "IceRpcCsUtil.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iterator>

using namespace std;
using namespace Slice;
using namespace Slice::Csharp;
using namespace IceInternal;

namespace
{
    // Returns paramName as-is or escaped if it conflicts with any of the parameter names in params.
    // paramName does not start with an escape prefix.
    string escapeParamName(string paramName, const ParameterList& params)
    {
        for (const auto& param : params)
        {
            if (param->mappedName() == paramName)
            {
                return paramName + "_";
            }
        }
        return paramName;
    }

    // paramName does not start with an escape prefix.
    string escapeCapitalizedParamName(string paramName, const ParameterList& params)
    {
        for (const auto& param : params)
        {
            if (toPascalCase(param->mappedName()) == paramName)
            {
                return paramName + "_";
            }
        }
        return paramName;
    }

    string defaultServicePath(const InterfaceDefPtr& interface)
    {
        string path = interface->scoped().substr(2); // Remove leading '::'
        string::size_type pos = 0;
        while ((pos = path.find("::", pos)) != string::npos)
        {
            path.replace(pos, 2, ".");
            pos += 1;
        }

        return "/" + path;
    }

    string classFormat(const OperationPtr& operation)
    {
        FormatType format = operation->format().value_or(FormatType::CompactFormat);
        return format == FormatType::SlicedFormat ? "ClassFormat.Sliced" : "default";
    }

    void writeReturnTask(IceInternal::Output& out, const OperationPtr& operation, const string& taskType, bool dispatch)
    {
        string ns = getNamespace(operation->interface());

        out << "global::System.Threading.Tasks." << taskType;

        TypeContext returnContext = dispatch ? TypeContext::OutgoingParam : TypeContext::IncomingParam;

        if (operation->returnsAnyValues())
        {
            out << '<';
            ParameterList returnParams = operation->outParameters();
            if (operation->returnType())
            {
                string returnParamName = escapeCapitalizedParamName("ReturnValue", returnParams);
                returnParams.insert(returnParams.begin(), operation->returnParameter(returnParamName));
            }

            if (returnParams.size() == 1)
            {
                out << csType(returnParams.front()->type(), ns, returnContext, returnParams.front()->optional());
            }
            else
            {
                out << spar;
                for (const auto& param : returnParams)
                {
                    out
                        << (csType(param->type(), ns, returnContext, param->optional()) + " " +
                            toPascalCase(param->mappedName()));
                }
                out << epar;
            }
            out << '>';
        }
    }

    // A ValueTask that holds all the decoded in parameters,
    void writeParamsValueTask(IceInternal::Output& out, const OperationPtr& operation)
    {
        string ns = getNamespace(operation->interface());

        out << "global::System.Threading.Tasks.ValueTask";

        ParameterList inParameters = operation->inParameters();
        if (!inParameters.empty())
        {
            out << '<';
            if (inParameters.size() == 1)
            {
                out << csIncomingParamType(inParameters.front()->type(), ns, inParameters.front()->optional());
            }
            else
            {
                out << spar;
                for (const auto& param : inParameters)
                {
                    out
                        << (csIncomingParamType(param->type(), ns, param->optional()) + " " +
                            toPascalCase(param->mappedName()));
                }
                out << epar;
            }
            out << '>';
        }
    }

    /// Writes the method signature for an operation, without access or semicolon.
    /// @param out The output to write to.
    /// @param operation The operation being mapped to C#.
    /// @param ns The current C# namespace.
    /// @param dispatch If true, writes the signature for the dispatch method (with incoming parameter types and a
    /// ValueTask return type); if false, writes the signature for the proxy method.
    void
    writeMethodSignature(IceInternal::Output& out, const OperationPtr& operation, const std::string& ns, bool dispatch)
    {
        TypeContext paramContext = dispatch ? TypeContext::IncomingParam : TypeContext::OutgoingParam;

        string featuresParam = escapeParamName("features", operation->inParameters());
        string cancellationTokenParam = escapeParamName("cancellationToken", operation->inParameters());

        vector<string> extraParams;
        if (dispatch)
        {
            extraParams = {
                "IceRpc.Features.IFeatureCollection " + featuresParam,
                "global::System.Threading.CancellationToken " + cancellationTokenParam};
        }
        else
        {
            extraParams = {
                "IceRpc.Features.IFeatureCollection? " + featuresParam + " = null",
                "global::System.Threading.CancellationToken " + cancellationTokenParam + " = default"};
        }

        writeReturnTask(out, operation, dispatch ? "ValueTask" : "Task", dispatch);
        out << ' ' << removeEscapePrefix(operation->mappedName()) << "Async(";
        out.inc();
        for (const auto& param : operation->inParameters())
        {
            out << nl << csType(param->type(), ns, paramContext, param->optional()) << ' ' << param->mappedName()
                << ',';
        }

        for (auto q = extraParams.begin(); q != extraParams.end();)
        {
            out << nl << *q;
            if (++q != extraParams.end())
            {
                out << ',';
            }
        }
        out << ')';
        out.dec();
    }

    string accessModifier(const ContainedPtr& p) { return p->hasMetadata("cs:internal") ? "internal" : "public"; }
}

Slice::IceRpc::TypesVisitor::TypesVisitor(IceInternal::Output& out) : CsVisitor(out) {}

bool
Slice::IceRpc::TypesVisitor::visitStructStart(const StructPtr& p)
{
    _out << sp;
    writeDocComment(p, "record struct");
    emitObsoleteAttribute(p);

    _out << nl << accessModifier(p) << (p->hasMetadata("cs:readonly") ? " readonly" : "") << " partial record struct "
         << p->mappedName();
    _out << sb;
    return true;
}

void
Slice::IceRpc::TypesVisitor::visitStructEnd(const StructPtr& p)
{
    string escapedName = p->mappedName();
    string ns = getNamespace(p);

    bool hasRequiredField = writePrimaryConstructor(p, p->dataMembers(), {}, "struct");

    // Decoding constructor.
    _out << sp;
    writeDocLine(
        _out,
        "summary",
        "Initializes a new instance of the <see cref=\"" + escapedName + "\" /> struct from an IceDecoder.");

    if (hasRequiredField)
    {
        _out << nl << "[global::System.Diagnostics.CodeAnalysis.SetsRequiredMembers]";
    }

    _out << nl << accessModifier(p) << ' ' << escapedName << "(ref IceDecoder decoder)";
    _out << sb;
    for (const auto& field : p->dataMembers())
    {
        _out << nl << "this." + field->mappedName() << " = ";
        decodeField(_out, field->type(), ns);
        _out << ';';
    }
    _out << eb;

    // Encode method.
    _out << sp;
    writeDocLine(_out, "summary", "Encodes the fields of this struct with an Ice encoder.");

    _out << nl << accessModifier(p) << " readonly void Encode(ref IceEncoder encoder)";
    _out << sb;
    for (const auto& field : p->dataMembers())
    {
        _out << nl;
        encodeField(_out, "this." + field->mappedName(), field->type(), ns, TypeContext::Field, "encoder");
        _out << ';';
    }
    _out << eb;

    _out << eb;
}

bool
Slice::IceRpc::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string ns = getNamespace(p);

    _out << sp;
    writeDocComment(p, "class");
    emitObsoleteAttribute(p);
    _out << nl << "[IceTypeId(\"" << p->scoped() << "\")]";
    if (p->compactId() != -1)
    {
        _out << nl << "[CompactIceTypeId(" << p->compactId() << ")]";
    }
    _out << nl << accessModifier(p) << " partial class " << p->mappedName() << " : ";

    ClassDefPtr base = p->base();
    if (base)
    {
        _out << getUnqualified(base, ns);
    }
    else
    {
        _out << "IceClass";
    }
    _out << sb;
    return true;
}

void
Slice::IceRpc::TypesVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string escapedName = p->mappedName();
    string ns = getNamespace(p);

    if (!p->dataMembers().empty())
    {
        _out << sp;
    }
    _out << nl << "private static readonly string IceTypeId = typeof(" << escapedName << ").GetIceTypeId()!;";
    if (p->compactId() != -1)
    {
        _out << nl << "private static readonly int CompactIceTypeId = typeof(" << escapedName
             << ").GetCompactIceTypeId()!.Value;";
    }

    if (!p->allDataMembers().empty())
    {
        DataMemberList allBaseFields;
        if (p->base())
        {
            allBaseFields = p->base()->allDataMembers();
        }

        writePrimaryConstructor(p, p->dataMembers(), allBaseFields, "class");

        // Public parameterless constructor. Must be public for decoding to work.
        _out << sp;
        writeDocLine(_out, "summary", "Initializes a new instance of the <see cref=\"" + escapedName + "\" /> class.");
        _out << nl << "public " << escapedName << "()";
        _out << sb;
        _out << eb;
    }
    // else, no need to generate any constructor.

    writeEncodeDecode(p->compactId(), ns, p->base() != nullptr, p->dataMembers(), p->orderedOptionalDataMembers());

    _out << eb;
}

bool
Slice::IceRpc::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string ns = getNamespace(p);

    _out << sp;
    writeDocComment(p, "exception class");
    emitObsoleteAttribute(p);
    _out << nl << "[IceTypeId(\"" << p->scoped() << "\")]";
    _out << nl << accessModifier(p) << " partial class " << p->mappedName() << " : ";

    ExceptionPtr base = p->base();
    if (base)
    {
        _out << getUnqualified(base, ns);
    }
    else
    {
        _out << "IceException";
    }
    _out << sb;
    return true;
}

void
Slice::IceRpc::TypesVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    string escapedName = p->mappedName();
    string ns = getNamespace(p);

    if (!p->dataMembers().empty())
    {
        _out << sp;
    }
    _out << nl << "private static readonly string IceTypeId = typeof(" << escapedName << ").GetIceTypeId()!;";

    if (!p->allDataMembers().empty())
    {
        DataMemberList allBaseFields;
        if (p->base())
        {
            allBaseFields = p->base()->allDataMembers();
        }

        writePrimaryConstructor(p, p->dataMembers(), allBaseFields, "exception class");

        // Public parameterless constructor. Must be public for decoding to work.
        _out << sp;
        writeDocLine(
            _out,
            "summary",
            "Initializes a new instance of the <see cref=\"" + escapedName + "\" /> exception class.");
        _out << nl << "public " << escapedName << "()";
        _out << sb;
        _out << eb;
    }
    // else, no need to generate any constructor.

    writeEncodeDecode(-1, ns, p->base() != nullptr, p->dataMembers(), p->orderedOptionalDataMembers());

    _out << eb;
}

void
Slice::IceRpc::TypesVisitor::visitDataMember(const DataMemberPtr& p)
{
    ContainedPtr cont = dynamic_pointer_cast<Contained>(p->container());
    assert(cont);
    string ns = getNamespace(cont);

    _out << sp;
    writeDocComment(p);
    emitObsoleteAttribute(p);
    emitAttributes(p);
    _out << nl << accessModifier(cont) << ' ';
    if (csRequired(p))
    {
        _out << "required ";
    }
    _out << csFieldType(p->type(), ns, p->optional()) << ' ' << p->mappedName();

    if (cont->hasMetadata("cs:readonly"))
    {
        _out << " { get; init; }";
    }
    else
    {
        _out << " { get; set; }";
    }
}

void
Slice::IceRpc::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string escapedName = p->mappedName();
    string name = removeEscapePrefix(p->mappedName());
    string ns = getNamespace(p);
    EnumeratorList enumerators = p->enumerators();
    const bool hasExplicitValues = p->hasExplicitValues();

    _out << sp;
    writeDocComment(p, "enum");
    emitObsoleteAttribute(p);
    emitAttributes(p);
    _out << nl << accessModifier(p) << " enum " << escapedName;
    _out << sb;
    for (const auto& enumerator : enumerators)
    {
        if (!isFirstElement(enumerator))
        {
            _out << ',';
            _out << sp;
        }

        writeDocComment(enumerator);
        emitObsoleteAttribute(enumerator);
        emitAttributes(enumerator);
        _out << nl << enumerator->mappedName();
        if (hasExplicitValues)
        {
            _out << " = " << enumerator->value();
        }
    }
    _out << eb;

    //
    // XxxIntExtensions
    //
    _out << sp;
    ostringstream intExtensionsComment;
    intExtensionsComment << "Provides an extension method for creating " << getArticleFor(name) << " <see cref=\""
                         << escapedName << "\" /> from an int.";
    writeHelperDocComment(p, intExtensionsComment.str(), "enum helper class");
    _out << nl << accessModifier(p) << " static class " << name << "IntExtensions";
    _out << sb;

    // When the number of enumerators is smaller than the distance between the min and max
    // values, the values are not consecutive and we need to use a set to validate the value
    // during decoding.
    // Note that the values are not necessarily in order, e.g. we can use a simple range check
    // for enum E { A = 3, B = 2, C = 1 } during decoding.
    bool useSet = p->enumerators().size() < static_cast<size_t>(p->maxValue() - p->minValue() + 1);

    if (useSet)
    {
        _out << nl
             << "private static readonly global::System.Collections.Generic.HashSet<int> _enumeratorValues = new()";
        _out.spar("{ ");
        for (const auto& enumerator : enumerators)
        {
            _out << enumerator->value();
        }
        _out.epar(" };");
        _out << sp;
    }

    // TODO: doc-comment
    _out << nl << accessModifier(p) << " static " << escapedName << " As" << name << "(this int value) =>";
    _out.inc();
    if (useSet)
    {
        _out << nl << "_enumeratorValues.Contains(value) ? ";
    }
    else
    {
        _out << nl << "(value >= " << p->minValue() << " && value <= " << p->maxValue() << ") ? ";
    }
    _out << "(" << escapedName << ")value :";
    _out.inc();
    _out << nl << "throw new global::System.IO.InvalidDataException($\"Invalid value {value} for enum " << escapedName
         << ".\");";
    _out.dec();
    _out.dec();
    _out << eb;

    //
    // XxxIceEncoderExtensions and XxxIceDecoderExtensions
    //
    _out << sp;
    ostringstream encoderExtensionsComment;
    encoderExtensionsComment << "Provides an extension method for encoding " << getArticleFor(name) << " <see cref=\""
                             << escapedName << "\" />.";
    writeHelperDocComment(p, encoderExtensionsComment.str(), "enum helper class");
    _out << nl << accessModifier(p) << " static class " << name << "IceEncoderExtensions";
    _out << sb;

    // TODO: doc-comment
    _out << nl << accessModifier(p) << " static void Encode" << name << "(this ref IceEncoder encoder, " << escapedName
         << " value) => encoder.EncodeSize((int)value);";
    _out << eb;

    _out << sp;
    ostringstream decoderExtensionsComment;
    decoderExtensionsComment << "Provides an extension method for decoding " << getArticleFor(name) << " <see cref=\""
                             << escapedName << "\" />.";
    writeHelperDocComment(p, decoderExtensionsComment.str(), "enum helper class");
    _out << nl << accessModifier(p) << " static class " << name << "IceDecoderExtensions";
    _out << sb;

    // TODO: doc-comment
    _out << nl << accessModifier(p) << " static " << escapedName << " Decode" << name
         << "(this ref IceDecoder decoder) => " << name << "IntExtensions.As" << name << "(decoder.DecodeSize());";
    _out << eb;
}

bool
Slice::IceRpc::TypesVisitor::writePrimaryConstructor(
    const ContainedPtr& p,
    const DataMemberList& fields,
    const DataMemberList& allBaseFields,
    const string& kind)
{
    // Primary constructor with parameters for all fields.
    string escapedName = p->mappedName();
    string name = removeEscapePrefix(escapedName);
    string ns = getNamespace(p);

    _out << sp;
    writeDocLine(
        _out,
        "summary",
        "Initializes a new instance of the <see cref=\"" + escapedName + "\" /> " + kind + ".");

    vector<string> ctorParams;
    vector<string> baseParams;
    vector<string> ctorPropertyInits;
    bool hasRequiredField = false;

    for (const auto& field : allBaseFields)
    {
        string paramName = field->customMappedName().value_or(toCamelCase(field->name()));
        ctorParams.push_back(csFieldType(field->type(), ns, field->optional()) + " " + paramName);
        if (csRequired(field))
        {
            hasRequiredField = true;
        }
        baseParams.push_back(paramName);
    }

    for (const auto& field : fields)
    {
        string paramName = field->customMappedName().value_or(toCamelCase(field->name()));
        ctorParams.push_back(csFieldType(field->type(), ns, field->optional()) + " " + paramName);
        if (csRequired(field))
        {
            hasRequiredField = true;
        }

        ctorPropertyInits.push_back("this." + field->mappedName() + " = " + paramName + ';');
    }

    if (hasRequiredField)
    {
        _out << nl << "[global::System.Diagnostics.CodeAnalysis.SetsRequiredMembers]";
    }

    _out << nl << accessModifier(p) << ' ' << escapedName << spar << ctorParams << epar;
    if (!baseParams.empty())
    {
        _out.inc();
        _out << nl << ": base" << spar << baseParams << epar;
        _out.dec();
    }

    _out << sb;
    for (const auto& propertyInit : ctorPropertyInits)
    {
        _out << nl << propertyInit;
    }
    _out << eb;
    return hasRequiredField;
}

void
Slice::IceRpc::TypesVisitor::writeEncodeDecode(
    int compactId,
    const string& ns,
    bool hasBase,
    const DataMemberList& fields,
    const DataMemberList& orderedOptionalFields)
{
    _out << sp;
    emitNonBrowsableAttribute();
    _out << nl << "protected override void EncodeCore(ref IceEncoder encoder)";
    _out << sb;
    _out << nl << "encoder.StartSlice(IceTypeId";
    if (compactId != -1)
    {
        _out << ", CompactIceTypeId";
    }
    _out << ");";
    // Encode non-optional fields
    for (const auto& field : fields)
    {
        if (!field->optional())
        {
            _out << nl;
            encodeField(_out, "this." + field->mappedName(), field->type(), ns, TypeContext::Field, "encoder");
            _out << ';';
        }
    }
    // Encode optional fields
    for (const auto& field : orderedOptionalFields)
    {
        encodeOptionalField(
            _out,
            field->tag(),
            "this." + field->mappedName(),
            field->type(),
            ns,
            TypeContext::Field,
            "encoder");
    }

    if (hasBase)
    {
        _out << nl << "encoder.EndSlice(false);";
        _out << nl << "base.EncodeCore(ref encoder);";
    }
    else
    {
        _out << nl << "encoder.EndSlice(true);"; // last slice
    }
    _out << eb;

    _out << sp;
    emitNonBrowsableAttribute();
    _out << nl << "protected override void DecodeCore(ref IceDecoder decoder)";
    _out << sb;
    _out << nl << "decoder.StartSlice();";
    // Decode non-optional fields
    for (const auto& field : fields)
    {
        if (!field->optional())
        {
            _out << nl << "this." + field->mappedName() << " = ";
            decodeField(_out, field->type(), ns);
            _out << ';';
        }
    }
    // Decode optional fields
    for (const auto& field : orderedOptionalFields)
    {
        _out << nl << "this." + field->mappedName() << " = ";
        decodeOptionalField(_out, field->tag(), field->type(), ns, TypeContext::Field);
        _out << ';';
    }

    _out << nl << "decoder.EndSlice();";
    if (hasBase)
    {
        _out << nl << "base.DecodeCore(ref decoder);";
    }
    _out << eb;
}

Slice::IceRpc::ProxyVisitor::ProxyVisitor(IceInternal::Output& out) : CsVisitor(out) {}

bool
Slice::IceRpc::ProxyVisitor::visitModuleStart(const ModulePtr& p)
{
    if (!p->contains<InterfaceDef>())
    {
        return false;
    }
    return CsVisitor::visitModuleStart(p);
}

bool
Slice::IceRpc::ProxyVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    string ns = getNamespace(p);
    string escapedName = p->mappedName();
    string name = removeEscapePrefix(escapedName);

    // Generate the client interface.
    _out << sp;
    writeDocComment(p, "client-side interface");
    emitObsoleteAttribute(p);
    _out << nl << accessModifier(p) << " partial interface I" << name;
    if (!p->bases().empty())
    {
        _out << " : ";
        _out.spar("");
        for (const auto& base : p->bases())
        {
            _out << getUnqualified(base, ns, "I");
        }
        _out.epar("");
    }

    _out << sb;

    return true;
}

void
Slice::IceRpc::ProxyVisitor::visitInterfaceDefEnd(const InterfaceDefPtr& p)
{
    string ns = getNamespace(p);
    string escapedName = p->mappedName();
    string name = removeEscapePrefix(escapedName);

    _out << eb; // end of client interface

    // Generate the proxy struct.
    _out << sp;

    writeDocLines(
        _out,
        "summary",
        {"Implements <see cref=\"I" + name + "\" /> by making invocations on a remote IceRPC service.",
         "This remote service must implement Ice interface <c>" + p->scoped() + "</c>."});

    _out << nl << "[IceTypeId(\"" << p->scoped() << "\")]";
    emitObsoleteAttribute(p);
    _out << nl << accessModifier(p) << " readonly partial record struct " << name << "Proxy : " << 'I' << name
         << ", IIceProxy";

    _out << sb;

    writeProxyRequestClass(p);
    _out << sp;
    writeProxyResponseClass(p);
    _out << sp;

    // EncodeOptions, Invoker and ServiceAddress come from IIceProxy; they must be public.

    _out << nl << "/// <summary>Represents the default path for IceRPC services that implement Ice interface";
    _out << nl << "/// <c>" << p->scoped() << "</c>.</summary>";
    _out << nl << accessModifier(p) << " const string DefaultServicePath = \"" << defaultServicePath(p) << "\";";
    _out << sp;
    _out << nl << "/// <inheritdoc/>";
    _out << nl << "public IceEncodeOptions? EncodeOptions { get; init; }";
    _out << sp;
    _out << nl << "/// <inheritdoc/>";
    _out << nl << "public required IceRpc.IInvoker Invoker { get; init; }";
    _out << sp;
    _out << nl << "/// <inheritdoc/>";
    _out << nl << "public IceRpc.ServiceAddress ServiceAddress { get; init; } = _defaultServiceAddress;";
    _out << sp;
    _out << nl << "private static IceRpc.ServiceAddress _defaultServiceAddress =";
    _out.inc();
    _out << nl << "new(IceRpc.Protocol.IceRpc) { Path = DefaultServicePath };";
    _out.dec();
    _out << sp;
    _out << nl << "private static readonly IActivator _defaultActivator =";
    _out.inc();
    _out << nl << "IActivator.FromAssembly(typeof(" << name << "Proxy).Assembly);";
    _out.dec();

    // Implicit base conversions
    for (const auto& base : p->bases())
    {
        _out << sp;
        string baseName = getUnqualified(base, ns, "", "Proxy");
        writeDocLine(_out, "summary", "Provides an implicit conversion to <see cref =\"" + baseName + "\" />.");

        _out << nl << accessModifier(p) << " static implicit operator " << baseName << "(" << name << "Proxy proxy) =>";
        _out.inc();
        _out << nl
             << "new() { EncodeOptions = proxy.EncodeOptions, Invoker = proxy.Invoker, ServiceAddress = "
                "proxy.ServiceAddress };";
        _out.dec();
    }

    // FromPath static method.
    _out << sp;
    _out << nl << "/// <summary>Creates a relative proxy from a path.</summary>";
    _out << nl << "/// <param name=\"path\">The path.</param>";
    _out << nl << "/// <returns>The new relative proxy.</returns>";
    _out << nl << accessModifier(p) << " static " << name << "Proxy FromPath(string path) =>";
    _out.inc();
    _out << nl << "new(IceRpc.InvalidInvoker.Instance, new IceRpc.ServiceAddress { Path = path });";
    _out.dec();

    // Primary constructor (invoker, serviceAddress, encodeOptions).
    _out << sp;
    _out << nl << "/// <summary>Constructs a proxy from an invoker, a service address and encode options.</summary>";
    _out << nl << "/// <param name=\"invoker\">The invocation pipeline of the proxy.</param>";
    _out << nl
         << "/// <param name=\"serviceAddress\">The service address. <see langword=\"null\" /> is equivalent to an "
            "IceRPC service address";
    _out << nl << "/// with path <see cref=\"DefaultServicePath\" />.</param>";
    _out << nl
         << "/// <param name=\"encodeOptions\">The encode options, used to customize the encoding of request "
            "payloads.</param>";
    _out << nl << "[System.Diagnostics.CodeAnalysis.SetsRequiredMembers]";
    _out << nl << accessModifier(p) << ' ' << name << "Proxy(";
    _out.inc();
    _out << nl << "IceRpc.IInvoker invoker,";
    _out << nl << "IceRpc.ServiceAddress? serviceAddress = null,";
    _out << nl << "IceEncodeOptions? encodeOptions = null)";
    _out.dec();
    _out << sb;
    _out << nl << "Invoker = invoker;";
    _out << nl << "ServiceAddress = serviceAddress ?? _defaultServiceAddress;";
    _out << nl << "EncodeOptions = encodeOptions;";
    _out << eb;

    // Constructor (invoker, serviceAddressUri, encodeOptions).
    _out << sp;
    _out << nl
         << "/// <summary>Constructs a proxy from an invoker, a service address URI and encode options.</summary>";
    _out << nl << "/// <param name=\"invoker\">The invocation pipeline of the proxy.</param>";
    _out << nl << "/// <param name=\"serviceAddressUri\">A URI that represents a service address.</param>";
    _out << nl
         << "/// <param name=\"encodeOptions\">The encode options, used to customize the encoding of request "
            "payloads.</param>";
    _out << nl << "[System.Diagnostics.CodeAnalysis.SetsRequiredMembers]";
    _out << nl << accessModifier(p) << ' ' << name
         << "Proxy(IceRpc.IInvoker invoker, System.Uri serviceAddressUri, IceEncodeOptions? encodeOptions = "
            "null)";
    _out.inc();
    _out << nl << ": this(invoker, new IceRpc.ServiceAddress(serviceAddressUri), encodeOptions)";
    _out.dec();
    _out << sb;
    _out << eb;

    // Parameterless constructor.
    _out << sp;
    _out << nl
         << "/// <summary>Constructs a proxy with an IceRPC service address with path <see cref=\"DefaultServicePath\" "
            "/>.</summary>";
    _out << nl << accessModifier(p) << ' ' << name << "Proxy()";
    _out << sb;
    _out << eb;

    // Inherited operations
    for (const auto& operation : p->allInheritedOperations())
    {
        // The implementation of interface methods must be public.
        _out << sp;
        _out << nl << "/// <inheritdoc />";
        _out << nl << "public ";

        string featureParam = escapeParamName("features", operation->inParameters());
        string cancellationTokenParam = escapeParamName("cancellationToken", operation->inParameters());

        writeMethodSignature(_out, operation, ns, false);

        _out << " =>";
        _out.inc();
        _out << nl << "(" << getUnqualified(operation->interface(), ns, "", "Proxy") << ")this."
             << removeEscapePrefix(operation->mappedName()) << "Async";
        _out << spar;
        for (const auto& param : operation->inParameters())
        {
            _out << param->mappedName();
        }

        _out << featureParam << cancellationTokenParam;
        _out << epar;
        _out << ';';
        _out.dec();
    }

    // My operations
    for (const auto& operation : p->operations())
    {
        // The implementation of interface methods must be public.
        _out << sp;
        _out << nl << "/// <inheritdoc/>";
        _out << nl << "public ";

        string operationName = removeEscapePrefix(operation->mappedName());
        string featureParam = escapeParamName("features", operation->inParameters());
        string cancellationTokenParam = escapeParamName("cancellationToken", operation->inParameters());

        writeMethodSignature(_out, operation, ns, false);

        _out << " =>";
        _out.inc();

        // IceRPC-Slice provides a `[compress]` attribute to compress request/response payloads for requests/responses
        // sent over the icerpc protocol. There is no equivalent for [compress] in Ice-Slice; this is not a serious
        // limitation since Ice-Slice is used primarily with the ice protocol.

        _out << nl << "this.InvokeOperationAsync(";
        _out.inc();
        _out << nl << "\"" << operation->name() << "\",";
        if (operation->inParameters().empty())
        {
            _out << nl << "payload: Request.Encode" << operationName << "(encodeOptions: EncodeOptions),";
        }
        else
        {
            _out << nl << "payload: Request.Encode" << operationName;
            _out << spar;
            for (const auto& param : operation->inParameters())
            {
                _out << param->mappedName();
            }
            // TODO: escape encodeOptions
            _out << "encodeOptions: EncodeOptions";
            _out << epar;
            _out << ',';
        }
        _out << nl << "payloadContinuation: null,";
        _out << nl << "Response.Decode" << operationName << "Async,";
        _out << nl << featureParam << ",";
        if (operation->mode() == Operation::Idempotent)
        {
            _out << nl << "idempotent: true,";
        }
        // TODO: oneway attribute

        _out << nl << "cancellationToken: " << cancellationTokenParam << ");";
        _out.dec();

        _out.dec();
    }
    _out << eb;

    // Generate extension classes.
    _out << sp;
    writeDocLine(
        _out,
        "summary",
        R"(Provides an extension method for <see cref="IceEncoder" /> to encode a <see cref=")" + name +
            R"(Proxy" />.")");
    _out << nl << accessModifier(p) << " static class " << name << "ProxyIceEncoderExtensions";
    _out << sb;
    writeDocLine(_out, "summary", R"(Encodes a <see cref=")" + name + R"(Proxy" /> instance.)");
    writeDocLine(_out, R"(param name="encoder")", "The Ice encoder.", "param");
    writeDocLine(_out, R"(param name="proxy")", "The proxy to encode (can be null).", "param");
    _out << nl << accessModifier(p) << " static void Encode" << name << "Proxy(this ref IceEncoder encoder, " << name
         << "Proxy? proxy) =>";
    _out.inc();
    _out << nl << "encoder.EncodeProxy(proxy);";
    _out.dec();
    _out << eb;

    _out << sp;
    writeDocLine(
        _out,
        "summary",
        R"(Provides an extension method for <see cref="IceDecoder" /> to decode a nullable <see cref=")" + name +
            R"(Proxy" /> instance.)");

    _out << nl << accessModifier(p) << " static class " << name << "ProxyIceDecoderExtensions";
    _out << sb;
    writeDocLine(_out, "summary", R"(Decodes a nullable <see cref=")" + name + R"(Proxy" /> instance.)");
    writeDocLine(_out, R"(param name="decoder")", "The Ice decoder.", "param");
    _out << nl << accessModifier(p) << " static " << name << "Proxy? Decode" << name
         << "Proxy(this ref IceDecoder decoder) =>";
    _out.inc();
    _out << nl << "decoder.DecodeProxy<" << name << "Proxy>();";
    _out.dec();
    _out << eb;
}

void
Slice::IceRpc::ProxyVisitor::visitOperation(const OperationPtr& p)
{
    string ns = getNamespace(p->interface());

    _out << sp;

    // TODO: doc comment for operation
    emitObsoleteAttribute(p);
    _out << nl;
    writeMethodSignature(_out, p, ns, false);
    _out << ';';
}

void
Slice::IceRpc::ProxyVisitor::writeProxyRequestClass(const InterfaceDefPtr& interface)
{
    string ns = getNamespace(interface);

    writeDocLine(_out, "summary", "Provides static methods that encode operation arguments into request payloads.");
    _out << nl << accessModifier(interface) << " static class Request";
    _out << sb;

    for (const auto& operation : interface->operations())
    {
        writeDocLine(
            _out,
            "summary",
            "Encodes the argument(s) of operation <c>" + operation->name() + "</c> into a request payload.");
        // TODO: param doc comments
        writeDocLine(_out, R"(param name="encodeOptions")", "The Ice encode options.", "param");
        writeDocLine(_out, "returns", "The Ice-encoded payload.", "returns");

        _out << nl << accessModifier(interface) << " static global::System.IO.Pipelines.PipeReader Encode"
             << removeEscapePrefix(operation->mappedName()) << "(";

        _out.inc();
        for (const auto& param : operation->inParameters())
        {
            _out << nl << csOutgoingParamType(param->type(), ns, param->optional()) << ' ' << param->mappedName()
                 << ',';
        }
        _out << nl << "IceEncodeOptions? encodeOptions = null)";
        _out.dec();
        _out << sb;
        if (operation->inParameters().empty())
        {
            _out << nl << "return IceRpc.EmptyPipeReader.Instance;";
        }
        else
        {
            _out << nl << "var pipe_ = new global::System.IO.Pipelines.Pipe(";
            _out.inc();
            _out << nl << "encodeOptions?.PipeOptions ?? IceEncodeOptions.Default.PipeOptions);";
            _out.dec();
            _out << nl << "var encoder_ = new IceEncoder(pipe_.Writer, " << classFormat(operation) << ");";

            for (const auto& param : operation->sortedInParameters())
            {
                if (param->optional())
                {
                    encodeOptionalField(
                        _out,
                        param->tag(),
                        param->mappedName(),
                        param->type(),
                        ns,
                        TypeContext::OutgoingParam,
                        "encoder_");
                }
                else
                {
                    _out << nl;
                    encodeField(_out, param->mappedName(), param->type(), ns, TypeContext::OutgoingParam, "encoder_");
                    _out << ';';
                }
            }

            _out << nl << "pipe_.Writer.Complete();";
            _out << nl << "return pipe_.Reader;";
        }
        _out << eb;
    }

    _out << eb;
}

void
Slice::IceRpc::ProxyVisitor::writeProxyResponseClass(const InterfaceDefPtr& interface)
{
    string ns = getNamespace(interface);

    writeDocLine(
        _out,
        "summary",
        "Provides a <see cref=\"ResponseDecodeFunc{T}\" /> for each operation defined in Ice interface <c>" +
            interface->scoped() + "</c>.");
    _out << nl << accessModifier(interface) << " static class Response";
    _out << sb;

    for (const auto& operation : interface->operations())
    {
        writeDocLine(_out, "summary", "Decodes an incoming response for operation <c>" + operation->name() + "</c>.");
        _out << nl << accessModifier(interface) << " static async ";
        writeReturnTask(_out, operation, "ValueTask", false);
        _out << " Decode" << removeEscapePrefix(operation->mappedName()) << "Async(";
        _out.inc();
        _out << nl << "IceRpc.IncomingResponse response,";
        _out << nl << "IceRpc.OutgoingRequest request,";
        _out << nl << "IIceProxy sender,";
        _out << nl << "global::System.Threading.CancellationToken cancellationToken)";
        _out.dec();
        _out << sb;

        _out << nl << "try";
        _out << sb;

        if (operation->returnsAnyValues())
        {
            _out << nl << "return await response.DecodeReturnValueAsync(";
            _out.inc();
            _out << nl << "request,";
            _out << nl << "sender,";
            _out << nl << "(ref IceDecoder decoder) => ";

            string returnParamName = escapeParamName("returnValue", operation->outParameters());
            ParameterList returnParams = operation->sortedReturnAndOutParameters(returnParamName);

            if (returnParams.size() == 1)
            {
                // Simplified decoding function for a single return value.
                if (returnParams.front()->optional())
                {
                    decodeOptionalField(
                        _out,
                        returnParams.front()->tag(),
                        returnParams.front()->type(),
                        ns,
                        TypeContext::IncomingParam);
                }
                else
                {
                    decodeField(_out, returnParams.front()->type(), ns);
                }
            }
            else
            {
                _out << sb;

                // Decode all return params
                for (const auto& param : returnParams)
                {
                    _out << nl << csType(param->type(), ns, TypeContext::IncomingParam, param->optional()) << " sliceP_"
                         << removeEscapePrefix(param->mappedName()) << " = ";
                    if (param->optional())
                    {
                        decodeOptionalField(_out, param->tag(), param->type(), ns, TypeContext::IncomingParam);
                    }
                    else
                    {
                        decodeField(_out, param->type(), ns);
                    }
                    _out << ';';
                }

                // Return tuple with the return value (if any) first.
                _out << nl << "return " << spar;
                if (operation->returnType())
                {
                    _out << ("sliceP_" + returnParamName);
                }
                for (const auto& param : returnParams)
                {
                    if (param->name() != returnParamName)
                    {
                        _out << ("sliceP_" + removeEscapePrefix(param->mappedName()));
                    }
                }
                _out << epar << ";";

                _out << eb;
            }
            _out << ',';

            _out << nl << "_defaultActivator,";
            _out << nl << "cancellationToken).ConfigureAwait(false);";
            _out.dec();
        }
        else
        {
            _out << nl << "await response.DecodeVoidReturnValueAsync(";
            _out.inc();
            _out << nl << "request,";
            _out << nl << "sender,";
            _out << nl << "_defaultActivator,";
            _out << nl << "cancellationToken).ConfigureAwait(false);";
            _out.dec();
        }

        _out << eb;
        _out << nl << "catch (IceException exception)";
        ExceptionList exceptionList = operation->throws();
        if (!exceptionList.empty())
        {
            _out << " when (exception is not ";
            if (exceptionList.size() == 1)
            {
                _out << getUnqualified(exceptionList.front(), ns);
            }
            else
            {
                _out << '(';
                for (auto q = exceptionList.begin(); q != exceptionList.end(); ++q)
                {
                    const ExceptionPtr& exception = *q;
                    if (q != exceptionList.begin())
                    {
                        _out << " or ";
                    }
                    _out << getUnqualified(exception, ns);
                }
                _out << ')';
            }
            _out << ')';
        }
        _out << sb;
        _out << nl << "throw new global::System.IO.InvalidDataException(";
        _out.inc();
        _out << nl << "$\"Exception specification violation: response to a " << operation->name()
             << " request carries an exception of type '{exception.GetType()}'.\", exception);";
        _out.dec();
        _out << eb;
        _out << eb;
    }

    _out << eb;
}

Slice::IceRpc::SkeletonVisitor::SkeletonVisitor(IceInternal::Output& output) : CsVisitor(output) {}

bool
Slice::IceRpc::SkeletonVisitor::visitModuleStart(const ModulePtr& p)
{
    if (!p->contains<InterfaceDef>())
    {
        return false;
    }
    return CsVisitor::visitModuleStart(p);
}

bool
Slice::IceRpc::SkeletonVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    string ns = getNamespace(p);
    string escapedName = p->mappedName();
    string name = removeEscapePrefix(escapedName);

    // Generate the server-side interface.
    _out << sp;
    writeDocComment(p, "server-side interface");
    _out << nl << "[IceTypeId(\"" << p->scoped() << "\")]";
    _out << nl << "[IceRpc.DefaultServicePath(\"" << defaultServicePath(p) << "\")]";
    emitObsoleteAttribute(p);
    _out << nl << accessModifier(p) << " partial interface I" << name << "Service";
    if (!p->bases().empty())
    {
        _out << " : ";
        _out.spar("");
        for (const auto& base : p->bases())
        {
            _out << getUnqualified(base, ns, "I", "Service");
        }
        _out.epar("");
    }

    _out << sb;

    if (!p->operations().empty())
    {
        writeRequestClass(p);
        _out << sp;
        writeResponseClass(p);
        _out << sp;
        _out << nl << "private static readonly IActivator _defaultActivator =";
        _out.inc();
        _out << nl << "IActivator.FromAssembly(typeof(I" << name << "Service).Assembly);";
        _out.dec();
    }

    // We don't generate the skeleton methods for ::Ice::Object, as we provide hand-written default implementations
    // for these methods.
    if (p->scoped() == "::Ice::Object")
    {
        _out << eb; // end of interface
        return false;
    }
    else
    {
        return true;
    }
}

void
Slice::IceRpc::SkeletonVisitor::visitInterfaceDefEnd(const InterfaceDefPtr&)
{
    _out << eb;
}

void
Slice::IceRpc::SkeletonVisitor::visitOperation(const OperationPtr& p)
{
    _out << sp;

    string ns = getNamespace(p->interface());

    // TODO: doc comment for operation

    _out << nl << "[IceOperation(\"" << p->name() << "\"";

    if (p->hasMarshaledResult())
    {
        _out << ", EncodedReturn = true";
    }
    if (p->mode() == Operation::Idempotent)
    {
        _out << ", Idempotent = true";
    }
    ExceptionList exceptions = p->throws();
    if (!exceptions.empty())
    {
        _out << ", ExceptionSpecification = new System.Type[] ";
        _out.spar("{ ");
        for (const auto& exception : exceptions)
        {
            _out << ("typeof(" + getUnqualified(exception, ns) + ")");
        }
        _out.epar(" }");
    }
    _out << ")]";

    _out << nl;
    writeMethodSignature(_out, p, ns, true);
    _out << ';';
}

void
Slice::IceRpc::SkeletonVisitor::writeRequestClass(const InterfaceDefPtr& interface)
{
    string ns = getNamespace(interface);

    writeDocLine(_out, "summary", "Provides static methods that decode request payloads.");

    // Check if any of the base interfaces already has a Request class.
    // A Request class is generated for any interface that defines at least one operation.
    _out << nl << accessModifier(interface) << " static ";
    if (!interface->allInheritedOperations().empty())
    {
        _out << "new ";
    }
    _out << "class Request";

    _out << sb;

    for (const auto& operation : interface->operations())
    {
        _out << sp;
        writeDocLine(_out, "summary", "Decodes the request payload of operation <c>" + operation->name() + "</c>.");
        // TODO: param doc comments
        _out << nl << accessModifier(interface) << " static ";
        writeParamsValueTask(_out, operation);
        _out << " Decode" << removeEscapePrefix(operation->mappedName()) << "Async(";
        _out.inc();
        _out << nl << "IceRpc.IncomingRequest request,";
        _out << nl << "global::System.Threading.CancellationToken cancellationToken) =>";

        ParameterList inParameters = operation->inParameters();

        if (inParameters.empty())
        {
            _out << nl << "request.DecodeEmptyArgsAsync(cancellationToken);";
        }
        else
        {
            _out << nl << "request.DecodeArgsAsync(";
            _out.inc();
            _out << nl << "(ref IceDecoder decoder) => ";

            if (inParameters.size() == 1)
            {
                // Simplified decoding function for a single parameter.
                if (inParameters.front()->optional())
                {
                    decodeOptionalField(
                        _out,
                        inParameters.front()->tag(),
                        inParameters.front()->type(),
                        ns,
                        TypeContext::IncomingParam);
                }
                else
                {
                    decodeField(_out, inParameters.front()->type(), ns);
                }
            }
            else
            {
                _out << sb;

                // Decode all params (2 or more).
                for (const auto& param : inParameters)
                {
                    _out << nl << csType(param->type(), ns, TypeContext::IncomingParam, param->optional()) << " sliceP_"
                         << removeEscapePrefix(param->mappedName()) << " = ";
                    if (param->optional())
                    {
                        decodeOptionalField(_out, param->tag(), param->type(), ns, TypeContext::IncomingParam);
                    }
                    else
                    {
                        decodeField(_out, param->type(), ns);
                    }
                    _out << ';';
                }

                _out << nl << "return " << spar;
                for (const auto& param : inParameters)
                {
                    _out << ("sliceP_" + removeEscapePrefix(param->mappedName()));
                }
                _out << epar << ";";

                _out << eb;
            }
            _out << ',';

            _out << nl << "_defaultActivator,";
            _out << nl << "cancellationToken);";
            _out.dec();
        }
        _out.dec();
    }

    _out << eb;
}

void
Slice::IceRpc::SkeletonVisitor::writeResponseClass(const InterfaceDefPtr& interface)
{
    string ns = getNamespace(interface);

    writeDocLine(_out, "summary", "Provides static methods that encode operation arguments into request payloads.");

    // Check if any of the base interfaces already has a Response class.
    // A Response class is generated for any interface that defines at least one operation.
    _out << nl << accessModifier(interface) << " static ";
    if (!interface->allInheritedOperations().empty())
    {
        _out << "new ";
    }
    _out << "class Response";

    _out << sb;

    for (const auto& operation : interface->operations())
    {
        _out << sp;
        writeDocLine(
            _out,
            "summary",
            "Encodes the return value and out parameter(s) of operation <c>" + operation->name() +
                "</c> into a response payload.");
        // TODO: param doc comments
        writeDocLine(_out, "param name=\"encodeOptions\"", "The Ice encode options.", "param");
        writeDocLine(_out, "returns", "The Ice-encoded payload.");

        _out << nl << accessModifier(interface) << " static global::System.IO.Pipelines.PipeReader Encode"
             << removeEscapePrefix(operation->mappedName()) << "(";

        _out.inc();
        string returnParamName = escapeParamName("returnValue", operation->outParameters());

        if (operation->returnType())
        {
            _out << nl << csOutgoingParamType(operation->returnType(), ns, operation->returnIsOptional()) << ' '
                 << returnParamName << ',';
        }
        for (const auto& param : operation->outParameters())
        {
            _out << nl << csOutgoingParamType(param->type(), ns, param->optional()) << ' ' << param->mappedName()
                 << ',';
        }
        _out << nl << "IceEncodeOptions? encodeOptions = null)";
        _out.dec();
        _out << sb;
        if (operation->returnsAnyValues())
        {
            _out << nl << "var pipe_ = new global::System.IO.Pipelines.Pipe(";
            _out.inc();
            _out << nl << "encodeOptions?.PipeOptions ?? IceEncodeOptions.Default.PipeOptions);";
            _out.dec();
            _out << nl << "var encoder_ = new IceEncoder(pipe_.Writer, " << classFormat(operation) << ");";

            for (const auto& param : operation->sortedReturnAndOutParameters(returnParamName))
            {
                if (param->optional())
                {
                    encodeOptionalField(
                        _out,
                        param->tag(),
                        param->mappedName(),
                        param->type(),
                        ns,
                        TypeContext::OutgoingParam,
                        "encoder_");
                }
                else
                {
                    _out << nl;
                    encodeField(_out, param->mappedName(), param->type(), ns, TypeContext::OutgoingParam, "encoder_");
                    _out << ';';
                }
            }

            _out << nl << "pipe_.Writer.Complete();";
            _out << nl << "return pipe_.Reader;";
        }
        else
        {
            _out << nl << "return IceRpc.EmptyPipeReader.Instance;";
        }
        _out << eb;
    }

    _out << eb;
}
