// Copyright (c) ZeroC, Inc.

#include "../Ice/OutputUtil.h"
#include "../Slice/FileTracker.h"
#include "../Slice/Parser.h"
#include "../Slice/Util.h"
#include "Ice/StringUtil.h"

#include <algorithm>
#include <cassert>
#include <iterator>

#include "Gen.h"

using namespace std;
using namespace Slice;
using namespace Slice::Swift;
using namespace IceInternal;

namespace
{
    string getClassResolverPrefix(const UnitPtr& unit)
    {
        DefinitionContextPtr dc = unit->findDefinitionContext(unit->topLevelFile());
        assert(dc);
        return dc->getMetadataArgs("swift:class-resolver-prefix").value_or("");
    }

    string opFormatTypeToString(const OperationPtr& op)
    {
        optional<FormatType> opFormat = op->format();
        if (opFormat)
        {
            switch (*opFormat)
            {
                case CompactFormat:
                    return ".compactFormat";
                case SlicedFormat:
                    return ".slicedFormat";
                default:
                    assert(false);
                    return "???";
            }
        }
        else
        {
            return "nil";
        }
    }
}

Gen::Gen(const string& base, const string& dir)
    : _out(false, true), // No break before opening block in Swift + short empty blocks
      _fileBase(Slice::baseName(base))
{
    string file = _fileBase + ".swift";

    if (!dir.empty())
    {
        file = dir + '/' + file;
    }

    _out.open(file.c_str());
    if (!_out)
    {
        ostringstream os;
        os << "cannot open '" << file << "': " << IceInternal::errorToString(errno);
        throw FileException(os.str());
    }
    FileTracker::instance()->addFile(file);

    printHeader();
    printGeneratedHeader(_out, _fileBase + ".ice");
    _out << sp;

    _out << nl << "import Foundation";
}

Slice::Gen::~Gen()
{
    if (_out.isOpen())
    {
        _out << nl;
        _out.close();
    }
}

void
Slice::Gen::generate(const UnitPtr& unit)
{
    validateSwiftMetadata(unit);
    validateSwiftModuleMappings(unit);

    ImportVisitor importVisitor(_out);
    unit->visit(&importVisitor);
    importVisitor.writeImports();

    TypesVisitor typesVisitor(_out);
    unit->visit(&typesVisitor);

    ServantVisitor servantVisitor(_out);
    unit->visit(&servantVisitor);

    ServantExtVisitor servantExtVisitor(_out);
    unit->visit(&servantExtVisitor);
}

void
Slice::Gen::printHeader()
{
    _out << "// Copyright (c) ZeroC, Inc.";
    _out << sp;
    _out << nl << "// slice2swift version " << ICE_STRING_VERSION;
}

Slice::ImportVisitor::ImportVisitor(IceInternal::Output& out) : _out(out) {}

bool
Slice::ImportVisitor::visitModuleStart(const ModulePtr& p)
{
    // Always import Ice module first if not building Ice
    if (p->isTopLevel() && _imports.empty())
    {
        string swiftModule = getSwiftModule(p);
        if (swiftModule != "Ice")
        {
            addImport("Ice");
        }
    }

    return true;
}

bool
Slice::ImportVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    // Add imports required for base class
    addImport(p->base(), p);

    // Add imports required for data members
    for (const auto& member : p->allDataMembers())
    {
        addImport(member->type(), p);
    }

    return false;
}

bool
Slice::ImportVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    // Add imports required for base interfaces
    InterfaceList bases = p->bases();
    for (const auto& base : bases)
    {
        addImport(base, p);
    }

    // Add imports required for operation parameters and return type
    for (const auto& operation : p->allOperations())
    {
        addImport(operation->returnType(), p);

        for (const auto& j : operation->parameters())
        {
            addImport(j->type(), p);
        }
    }

    return false;
}

bool
Slice::ImportVisitor::visitStructStart(const StructPtr& p)
{
    // Add imports required for data members
    for (const auto& dataMember : p->dataMembers())
    {
        addImport(dataMember->type(), p);
    }

    return true;
}

bool
Slice::ImportVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    // Add imports required for base exceptions
    addImport(p->base(), p);

    // Add imports required for data members
    for (const auto& member : p->allDataMembers())
    {
        addImport(member->type(), p);
    }
    return true;
}

void
Slice::ImportVisitor::visitSequence(const SequencePtr& seq)
{
    // Add import required for the sequence element type
    addImport(seq->type(), seq);
}

void
Slice::ImportVisitor::visitDictionary(const DictionaryPtr& dict)
{
    // Add imports required for the dictionary key and value types
    addImport(dict->keyType(), dict);
    addImport(dict->valueType(), dict);
}

void
Slice::ImportVisitor::writeImports()
{
    for (const auto& import : _imports)
    {
        _out << nl << "import " << import;
    }
}

void
Slice::ImportVisitor::addImport(const SyntaxTreeBasePtr& p, const ContainedPtr& usedBy)
{
    // Only add imports for user defined constructs...
    auto definition = dynamic_pointer_cast<Contained>(p);
    if (definition)
    {
        // ... and only if the type lives in a different module than where it's being used.
        string swiftM1 = getSwiftModule(definition->getTopLevelModule());
        string swiftM2 = getSwiftModule(usedBy->getTopLevelModule());
        if (swiftM1 != swiftM2)
        {
            addImport(swiftM1);
        }
    }
}

void
Slice::ImportVisitor::addImport(const string& module)
{
    if (find(_imports.begin(), _imports.end(), module) == _imports.end())
    {
        _imports.push_back(module);
    }
}

Slice::TypesVisitor::TypesVisitor(IceInternal::Output& out) : _out(out) {}

bool
Slice::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    const string prefix = getClassResolverPrefix(p->unit());
    const string swiftModule = getSwiftModule(p->getTopLevelModule());
    const string name = getRelativeTypeString(p, swiftModule);

    ClassDefPtr base = p->base();

    // For each Value class we generate an extension method in DefaultSliceLoader.
    // This method name is based off of the Slice type ID, not the mapped name.
    ostringstream methodName;
    methodName << "resolveTypeId" << prefix << "_";
    vector<string> parts = splitScopedName(p->scoped());
    for (auto it = parts.begin(); it != parts.end();)
    {
        methodName << (*it);
        if (++it != parts.end())
        {
            methodName << "_";
        }
    }

    _out << sp;
    _out << nl << "public extension " << getUnqualified("Ice.DefaultSliceLoader", swiftModule);
    _out << sb;
    _out << nl << "@objc static func " << methodName.str() << "() -> AnyObject.Type";
    _out << sb;
    _out << nl << name << ".self";
    _out << eb;
    if (p->compactId() != -1)
    {
        // Same for compact ID.
        _out << sp;
        _out << nl << "@objc static func resolveTypeId" << prefix << "_" << to_string(p->compactId())
             << "() -> AnyObject.Type";
        _out << sb;
        _out << nl << name << ".self";
        _out << eb;
    }
    _out << eb;

    _out << sp;
    writeDocSummary(_out, p, "class");
    writeSwiftAttributes(_out, p->getMetadata());
    _out << nl << "open class " << name << ": ";
    if (base)
    {
        _out << getRelativeTypeString(base, swiftModule);
    }
    else
    {
        _out << getUnqualified("Ice.Value", swiftModule);
    }
    _out << sb;

    const DataMemberList members = p->dataMembers();
    const DataMemberList baseMembers = base ? base->allDataMembers() : DataMemberList();
    const DataMemberList allMembers = p->allDataMembers();
    const DataMemberList optionalMembers = p->orderedOptionalDataMembers();

    writeMembers(_out, members, p);

    if (!allMembers.empty())
    {
        if (!members.empty())
        {
            writeDefaultInitializer(_out, true, !base);
            writeMemberwiseInitializer(_out, members, baseMembers, allMembers, p, !base);
        }
        // else inherit the base class initializers
    }
    // else inherit Value's initializer.

    _out << sp;
    _out << nl << "/// - Returns: The Slice type ID of the interface supported by this object.";
    _out << nl << "open override class func ice_staticId() -> Swift.String { \"" << p->scoped() << "\" }";

    _out << sp;
    _out << nl << "open override func _iceReadImpl(from istr: " << getUnqualified("Ice.InputStream", swiftModule)
         << ") throws";
    _out << sb;
    _out << nl << "_ = try istr.startSlice()";
    if (!members.empty())
    {
        _out << nl << "nonisolated(unsafe) let iceP_self = self";

        for (const auto& member : members)
        {
            if (!member->isOptional())
            {
                writeMarshalUnmarshalCode(_out, member->type(), p, "iceP_self." + member->mappedName(), false);
            }
        }
        for (const auto& member : optionalMembers)
        {
            writeMarshalUnmarshalCode(
                _out,
                member->type(),
                p,
                "iceP_self." + member->mappedName(),
                false,
                member->tag());
        }
    }

    _out << nl << "try istr.endSlice()";
    if (base)
    {
        _out << nl << "try super._iceReadImpl(from: istr);";
    }
    _out << eb;

    _out << sp;
    _out << nl << "open override func _iceWriteImpl(to ostr: " << getUnqualified("Ice.OutputStream", swiftModule) << ")";
    _out << sb;
    _out << nl << "ostr.startSlice(typeId: " << name << ".ice_staticId(), compactId: " << p->compactId()
         << ", last: " << (!base ? "true" : "false") << ")";
    for (const auto& member : members)
    {
        TypePtr type = member->type();
        if (!member->isOptional())
        {
            writeMarshalUnmarshalCode(_out, member->type(), p, "self." + member->mappedName(), true);
        }
    }
    for (const auto& member : optionalMembers)
    {
        writeMarshalUnmarshalCode(_out, member->type(), p, "self." + member->mappedName(), true, member->tag());
    }
    _out << nl << "ostr.endSlice()";
    if (base)
    {
        _out << nl << "super._iceWriteImpl(to: ostr);";
    }
    _out << eb;

    return true;
}

void
Slice::TypesVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _out << eb;
}

bool
Slice::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    const string swiftModule = getSwiftModule(p->getTopLevelModule());
    const string name = getRelativeTypeString(p, swiftModule);

    ExceptionPtr base = p->base();

    const string prefix = getClassResolverPrefix(p->unit());

    // For each UserException class we generate an extension in DefaultSliceLoader.
    // This method name is based off of the Slice type ID, not the mapped name.
    ostringstream methodName;
    methodName << "resolveTypeId" << prefix << "_";
    vector<string> parts = splitScopedName(p->scoped());
    for (auto it = parts.begin(); it != parts.end();)
    {
        methodName << (*it);
        if (++it != parts.end())
        {
            methodName << "_";
        }
    }

    _out << sp;
    _out << nl << "public extension " << getUnqualified("Ice.DefaultSliceLoader", swiftModule);
    _out << sb;
    _out << nl << "@objc static func " << methodName.str() << "() -> AnyObject.Type";
    _out << sb;
    _out << nl << name << ".self";
    _out << eb;
    _out << eb;

    _out << sp;
    writeDocSummary(_out, p, "exception class");
    writeSwiftAttributes(_out, p->getMetadata());
    _out << nl << "open class " << name << ": ";
    if (base)
    {
        _out << getRelativeTypeString(base, swiftModule);
    }
    else
    {
        _out << getUnqualified("Ice.UserException", swiftModule);
    }
    _out << ", @unchecked Sendable";
    _out << sb;

    const DataMemberList members = p->dataMembers();
    const DataMemberList allMembers = p->allDataMembers();
    const DataMemberList baseMembers = base ? base->allDataMembers() : DataMemberList();
    const DataMemberList optionalMembers = p->orderedOptionalDataMembers();

    writeMembers(_out, members, p);

    if (!allMembers.empty())
    {
        if (!members.empty())
        {
            writeDefaultInitializer(_out, true, !base);
            writeMemberwiseInitializer(_out, members, baseMembers, allMembers, p, !base);
        }
        // else inherit the base class initializers
    }
    // else inherit UserException's initializer.

    _out << sp;
    _out << nl << "/// - Returns: The Slice type ID of this exception.";
    _out << nl << "open override class func ice_staticId() -> Swift.String { \"" << p->scoped() << "\" }";

    _out << sp;
    _out << nl << "open override func _iceWriteImpl(to ostr: " << getUnqualified("Ice.OutputStream", swiftModule) << ")";
    _out << sb;
    _out << nl << "ostr.startSlice(typeId: " << name
         << ".ice_staticId(), compactId: -1, last: " << (!base ? "true" : "false") << ")";
    for (const auto& member : members)
    {
        if (!member->isOptional())
        {
            writeMarshalUnmarshalCode(_out, member->type(), p, "self." + member->mappedName(), true);
        }
    }

    for (const auto& member : optionalMembers)
    {
        writeMarshalUnmarshalCode(_out, member->type(), p, "self." + member->mappedName(), true, member->tag());
    }
    _out << nl << "ostr.endSlice()";
    if (base)
    {
        _out << nl << "super._iceWriteImpl(to: ostr);";
    }
    _out << eb;

    _out << sp;
    _out << nl << "open override func _iceReadImpl(from istr: " << getUnqualified("Ice.InputStream", swiftModule)
         << ") throws";
    _out << sb;
    _out << nl << "_ = try istr.startSlice()";
    for (const auto& member : members)
    {
        if (!member->isOptional())
        {
            writeMarshalUnmarshalCode(_out, member->type(), p, "self." + member->mappedName(), false);
        }
    }

    for (const auto& member : optionalMembers)
    {
        writeMarshalUnmarshalCode(_out, member->type(), p, "self." + member->mappedName(), false, member->tag());
    }

    _out << nl << "try istr.endSlice()";
    if (base)
    {
        _out << nl << "try super._iceReadImpl(from: istr);";
    }
    _out << eb;

    if (p->usesClasses() && !(base && base->usesClasses()))
    {
        _out << sp;
        _out << nl << "open override func _usesClasses() -> Swift.Bool" << sb;
        _out << nl << "return true";
        _out << eb;
    }

    _out << eb;
    return false;
}

bool
Slice::TypesVisitor::visitStructStart(const StructPtr& p)
{
    const string swiftModule = getSwiftModule(p->getTopLevelModule());
    const string name = getRelativeTypeString(p, swiftModule);
    const string docName = removeEscaping(name);
    bool isLegalKeyType = Dictionary::isLegalKeyType(p);
    const DataMemberList members = p->dataMembers();
    const string optionalFormat = getOptionalFormat(p);

    bool usesClasses = p->usesClasses();
    _out << sp;
    writeDocSummary(_out, p, usesClasses ? "class" : "struct");
    writeSwiftAttributes(_out, p->getMetadata());
    _out << nl << "public " << (usesClasses ? "final class " : "struct ") << name;

    // Vector of protocols this struct conforms to.
    vector<string> structProtocols;
    if (isLegalKeyType)
    {
        structProtocols.emplace_back("Swift.Hashable");
    }
    if (!usesClasses)
    {
        structProtocols.emplace_back("Swift.Sendable");
    }

    if (!structProtocols.empty())
    {
        _out << ": ";
        _out.spar("");
        _out << structProtocols;
        _out.epar("");
    }

    _out << sb;

    writeMembers(_out, members, p);
    writeDefaultInitializer(_out, false, true);
    writeMemberwiseInitializer(_out, members, p);

    _out << eb;

    _out << sp;
    _out << nl << "/// An `Ice.InputStream` extension to read `" << docName << "` structured values from the stream.";
    _out << nl << "public extension " << getUnqualified("Ice.InputStream", swiftModule);
    _out << sb;

    _out << sp;
    _out << nl << "/// Read a `" << docName << "` structured value from the stream.";
    _out << nl << "///";
    _out << nl << "/// - Returns: The structured value read from the stream.";
    _out << nl << "func read() throws -> sending " << name;
    _out << sb;
    if (usesClasses)
    {
        _out << nl << "nonisolated(unsafe) let";
    }
    else
    {
        _out << nl << "var";
    }
    _out << " v = " << name << "()";
    for (const auto& member : members)
    {
        writeMarshalUnmarshalCode(_out, member->type(), p, "v." + member->mappedName(), false);
    }
    _out << nl << "return v";
    _out << eb;

    _out << sp;
    _out << nl << "/// Read an optional `" << docName << "?` structured value from the stream.";
    _out << nl << "///";
    _out << nl << "/// - Parameter tag: The numeric tag associated with the value.";
    _out << nl << "/// - Returns: The structured value read from the stream.";
    _out << nl << "func read(tag: Swift.Int32) throws -> sending " << name << "?";
    _out << sb;
    _out << nl << "guard try readOptional(tag: tag, expectedFormat: " << optionalFormat << ") else";
    _out << sb;
    _out << nl << "return nil";
    _out << eb;
    if (p->isVariableLength())
    {
        _out << nl << "try skip(4)";
    }
    else
    {
        _out << nl << "try skipSize()";
    }
    _out << nl << "return try read() as " << name;
    _out << eb;

    _out << eb;

    _out << sp;
    _out << nl << "/// An `Ice.OutputStream` extension to write `" << docName << "` structured values from the stream.";
    _out << nl << "public extension " << getUnqualified("Ice.OutputStream", swiftModule);
    _out << sb;

    _out << nl << "/// Write a `" << docName << "` structured value to the stream.";
    _out << nl << "///";
    _out << nl << "/// - Parameter v: The value to write to the stream.";
    _out << nl << "func write(_ v: " << name << ")" << sb;
    for (const auto& member : members)
    {
        writeMarshalUnmarshalCode(_out, member->type(), p, "v." + member->mappedName(), true);
    }
    _out << eb;

    _out << sp;
    _out << nl << "/// Write an optional `" << docName << "?` structured value to the stream.";
    _out << nl << "///";
    _out << nl << "/// - Parameters:";
    _out << nl << "///   - tag: The numeric tag associated with the value.";
    _out << nl << "///   - value: The value to write to the stream.";
    _out << nl << "func write(tag: Swift.Int32, value: " << name << "?)" << sb;
    _out << nl << "if let v = value" << sb;
    _out << nl << "if writeOptional(tag: tag, format: " << optionalFormat << ")" << sb;

    if (p->isVariableLength())
    {
        _out << nl << "let pos = startSize()";
        _out << nl << "write(v)";
        _out << nl << "endSize(position: pos)";
    }
    else
    {
        _out << nl << "write(size: " << p->minWireSize() << ")";
        _out << nl << "write(v)";
    }
    _out << eb;
    _out << eb;
    _out << eb;

    _out << eb;

    return false;
}

void
Slice::TypesVisitor::visitSequence(const SequencePtr& p)
{
    const string swiftModule = getSwiftModule(p->getTopLevelModule());
    const string name = getRelativeTypeString(p, swiftModule);
    const string unescapedName = removeEscaping(name);

    const TypePtr type = p->type();
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);

    _out << sp;
    writeDocSummary(_out, p);
    _out << nl << "public typealias " << name << " = ";

    if (builtin && builtin->kind() == Builtin::KindByte)
    {
        _out << "Foundation.Data";
    }
    else
    {
        _out << "[" << typeToString(type, p, false) << "]";
    }

    if (builtin && builtin->kind() <= Builtin::KindString)
    {
        return; // No helpers for sequence of primitive types
    }

    const string ostr = getUnqualified("Ice.OutputStream", swiftModule);
    const string istr = getUnqualified("Ice.InputStream", swiftModule);

    const string optionalFormat = getOptionalFormat(p);

    _out << sp;
    _out << nl << "/// Helper class to read and write `" << unescapedName << "` sequence values from";
    _out << nl << "/// `Ice.InputStream` and `Ice.OutputStream`.";
    _out << nl << "public struct " << unescapedName << "Helper";
    _out << sb;

    _out << nl << "/// Read a `" << unescapedName << "` sequence from the stream.";
    _out << nl << "///";
    _out << nl << "/// - Parameter istr: The stream to read from.";
    _out << nl << "/// - Returns: The sequence read from the stream.";
    _out << nl << "public static func read(from istr: " << istr << ") throws -> sending " << name;
    _out << sb;
    _out << nl << "let sz = try istr.readAndCheckSeqSize(minSize: " << p->type()->minWireSize() << ")";

    if (type->isClassType())
    {
        _out << nl << "nonisolated(unsafe) var v = " << name << "(repeating: nil, count: sz)";
        _out << nl << "for i in 0 ..< sz";
        _out << sb;
        _out << nl << "try Swift.withUnsafeMutablePointer(to: &v[i])";
        _out << sb;
        _out << " p in";
        _out << nl << "nonisolated(unsafe) let p = p";
        writeMarshalUnmarshalCode(_out, type, p, "p.pointee", false);
        _out << eb;
        _out << eb;
    }
    else
    {
        _out << nl << "var v = " << name << "()";
        _out << nl << "v.reserveCapacity(sz)";
        _out << nl << "for _ in 0 ..< sz";
        _out << sb;
        string param = "let j: " + typeToString(p->type(), p);
        writeMarshalUnmarshalCode(_out, type, p, param, false);
        _out << nl << "v.append(j)";
        _out << eb;
    }
    _out << nl << "return v";
    _out << eb;

    _out << sp;
    _out << nl << "/// Read an optional `" << unescapedName << "?` sequence from the stream.";
    _out << nl << "///";
    _out << nl << "/// - Parameters:";
    _out << nl << "///   - istr: The stream to read from.";
    _out << nl << "///   - tag: The numeric tag associated with the value.";
    _out << nl << "/// - Returns: The sequence read from the stream.";
    _out << nl << "public static func read(from istr: " << istr << ", tag: Swift.Int32) throws -> sending " << name
         << "?";
    _out << sb;
    _out << nl << "guard try istr.readOptional(tag: tag, expectedFormat: " << optionalFormat << ") else";
    _out << sb;
    _out << nl << "return nil";
    _out << eb;
    if (p->type()->isVariableLength())
    {
        _out << nl << "try istr.skip(4)";
    }
    else if (p->type()->minWireSize() > 1)
    {
        _out << nl << "try istr.skipSize()";
    }
    _out << nl << "return try read(from: istr)";
    _out << eb;

    _out << sp;
    _out << nl << "/// Write a `" << unescapedName << "` sequence to the stream.";
    _out << nl << "///";
    _out << nl << "/// - Parameters:";
    _out << nl << "///   - ostr: The stream to write to.";
    _out << nl << "///   - v: The sequence value to write to the stream.";
    _out << nl << "public static func write(to ostr: " << ostr << ", value v: " << name << ")";
    _out << sb;
    _out << nl << "ostr.write(size: v.count)";
    _out << nl << "for item in v";
    _out << sb;
    writeMarshalUnmarshalCode(_out, type, p, "item", true);
    _out << eb;
    _out << eb;

    _out << sp;
    _out << nl << "/// Write an optional `" << unescapedName << "?` sequence to the stream.";
    _out << nl << "///";
    _out << nl << "/// - Parameters:";
    _out << nl << "///   - ostr: The stream to write to.";
    _out << nl << "///   - tag: The numeric tag associated with the value.";
    _out << nl << "///   - v: The sequence value to write to the stream.";
    _out << nl << "public static func write(to ostr: " << ostr << ",  tag: Swift.Int32, value v: " << name << "?)";
    _out << sb;
    _out << nl << "guard let val = v else";
    _out << sb;
    _out << nl << "return";
    _out << eb;
    if (p->type()->isVariableLength())
    {
        _out << nl << "if ostr.writeOptional(tag: tag, format: " << optionalFormat << ")";
        _out << sb;
        _out << nl << "let pos = ostr.startSize()";
        _out << nl << "write(to: ostr, value: val)";
        _out << nl << "ostr.endSize(position: pos)";
        _out << eb;
    }
    else
    {
        if (p->type()->minWireSize() == 1)
        {
            _out << nl << "if ostr.writeOptional(tag: tag, format: .VSize)";
        }
        else
        {
            _out << nl << "if ostr.writeOptionalVSize(tag: tag, len: val.count, elemSize: " << p->type()->minWireSize()
                 << ")";
        }
        _out << sb;
        _out << nl << "write(to: ostr, value: val)";
        _out << eb;
    }
    _out << eb;

    _out << eb;
}

void
Slice::TypesVisitor::visitDictionary(const DictionaryPtr& p)
{
    const string swiftModule = getSwiftModule(p->getTopLevelModule());
    const string name = getRelativeTypeString(p, swiftModule);
    const string unescapedName = removeEscaping(name);

    const string keyType = typeToString(p->keyType(), p, false);
    const string valueType = typeToString(p->valueType(), p, false);

    _out << sp;
    writeDocSummary(_out, p);
    _out << nl << "public typealias " << name << " = [" << keyType << ": " << valueType << "]";

    const string ostr = getUnqualified("Ice.OutputStream", swiftModule);
    const string istr = getUnqualified("Ice.InputStream", swiftModule);

    const string optionalFormat = getOptionalFormat(p);
    const bool isVariableLength = p->keyType()->isVariableLength() || p->valueType()->isVariableLength();
    const size_t minWireSize = p->keyType()->minWireSize() + p->valueType()->minWireSize();

    _out << sp;
    _out << nl << "/// Helper class to read and write `" << unescapedName << "` dictionary values from";
    _out << nl << "/// `Ice.InputStream` and `Ice.OutputStream`.";
    _out << nl << "public struct " << unescapedName << "Helper";
    _out << sb;

    _out << nl << "/// Read a `" << unescapedName << "` dictionary from the stream.";
    _out << nl << "///";
    _out << nl << "/// - Parameter istr: The stream to read from.";
    _out << nl << "/// - Returns: The dictionary read from the stream.";
    _out << nl << "public static func read(from istr: " << istr << ") throws -> sending " << name;
    _out << sb;
    _out << nl << "let sz = try Swift.Int(istr.readSize())";
    _out << nl << "var v = " << name << "()";
    if (p->valueType()->isClassType())
    {
        _out << nl << "nonisolated(unsafe) let e = " << getUnqualified("Ice.DictEntryArray", swiftModule) << "<"
             << keyType << ", " << valueType << ">(size: sz)";
        _out << nl << "for i in 0 ..< sz";
        _out << sb;
        string keyParam = "let key: " + keyType;
        writeMarshalUnmarshalCode(_out, p->keyType(), p, keyParam, false);
        _out << nl << "v[key] = nil as " << valueType;
        _out << nl << "Swift.withUnsafeMutablePointer(to: &v[key, default:nil])";
        _out << sb;
        _out << nl << "e.values[i] = Ice.DictEntry<" << keyType << ", " << valueType << ">("
             << "key: key, "
             << "value: $0)";
        _out << eb;
        writeMarshalUnmarshalCode(_out, p->valueType(), p, "e.values[i].value.pointee", false);
        _out << eb;

        _out << nl << "for i in 0..<sz" << sb;
        _out << nl << "Swift.withUnsafeMutablePointer(to: &v[e.values[i].key, default:nil])";
        _out << sb;
        _out << nl << "e.values[i].value = $0";
        _out << eb;
        _out << eb;
    }
    else
    {
        _out << nl << "for _ in 0 ..< sz";
        _out << sb;
        string keyParam = "let key: " + keyType;
        writeMarshalUnmarshalCode(_out, p->keyType(), p, keyParam, false);
        string valueParam = "let value: " + typeToString(p->valueType(), p);
        writeMarshalUnmarshalCode(_out, p->valueType(), p, valueParam, false);
        _out << nl << "v[key] = value";
        _out << eb;
    }

    _out << nl << "return v";
    _out << eb;

    _out << sp;
    _out << nl << "/// Read an optional `" << unescapedName << "?` dictionary from the stream.";
    _out << nl << "///";
    _out << nl << "/// - Parameters:";
    _out << nl << "///   - istr: The stream to read from.";
    _out << nl << "///   - tag: The numeric tag associated with the value.";
    _out << nl << "/// - Returns: The dictionary read from the stream.";
    _out << nl << "public static func read(from istr: " << istr << ", tag: Swift.Int32) throws -> sending " << name
         << "?";
    _out << sb;
    _out << nl << "guard try istr.readOptional(tag: tag, expectedFormat: " << optionalFormat << ") else";
    _out << sb;
    _out << nl << "return nil";
    _out << eb;
    if (p->keyType()->isVariableLength() || p->valueType()->isVariableLength())
    {
        _out << nl << "try istr.skip(4)";
    }
    else
    {
        _out << nl << "try istr.skipSize()";
    }
    _out << nl << "return try read(from: istr)";
    _out << eb;

    _out << sp;
    _out << nl << "/// Write a `" << unescapedName << "` dictionary to the stream.";
    _out << nl << "///";
    _out << nl << "/// - Parameters:";
    _out << nl << "///   - ostr: The stream to write to.";
    _out << nl << "///   - v: The dictionary value to write to the stream.";
    _out << nl << "public static func write(to ostr: " << ostr << ", value v: " << name << ")";
    _out << sb;
    _out << nl << "ostr.write(size: v.count)";
    _out << nl << "for (key, value) in v";
    _out << sb;
    writeMarshalUnmarshalCode(_out, p->keyType(), p, "key", true);
    writeMarshalUnmarshalCode(_out, p->valueType(), p, "value", true);
    _out << eb;
    _out << eb;

    _out << sp;
    _out << nl << "/// Write an optional `" << unescapedName << "?` dictionary to the stream.";
    _out << nl << "///";
    _out << nl << "/// - Parameters:";
    _out << nl << "///   - ostr: The stream to write to.";
    _out << nl << "///   - tag: The numeric tag associated with the value.";
    _out << nl << "///   - v: The dictionary value to write to the stream.";
    _out << nl << "public static func write(to ostr: " << ostr << ", tag: Swift.Int32, value v: " << name << "?)";
    _out << sb;
    _out << nl << "guard let val = v else";
    _out << sb;
    _out << nl << "return";
    _out << eb;
    if (isVariableLength)
    {
        _out << nl << "if ostr.writeOptional(tag: tag, format: " << optionalFormat << ")";
        _out << sb;
        _out << nl << "let pos = ostr.startSize()";
        _out << nl << "write(to: ostr, value: val)";
        _out << nl << "ostr.endSize(position: pos)";
        _out << eb;
    }
    else
    {
        _out << nl << "if ostr.writeOptionalVSize(tag: tag, len: val.count, elemSize: " << minWireSize << ")";
        _out << sb;
        _out << nl << "write(to: ostr, value: val)";
        _out << eb;
    }
    _out << eb;

    _out << eb;
}

void
Slice::TypesVisitor::visitEnum(const EnumPtr& p)
{
    const string swiftModule = getSwiftModule(p->getTopLevelModule());
    const string name = getRelativeTypeString(p, swiftModule);
    const string docName = removeEscaping(name);
    const EnumeratorList enumerators = p->enumerators();
    const string enumType = p->maxValue() <= 0xFF ? "Swift.UInt8" : "Swift.Int32";
    const string optionalFormat = getOptionalFormat(p);

    _out << sp;
    writeDocSummary(_out, p, "enum");
    writeSwiftAttributes(_out, p->getMetadata());
    _out << nl << "public enum " << name << ": " << enumType << ", Swift.Sendable";
    _out << sb;

    for (const auto& enumerator : enumerators)
    {
        writeDocSummary(_out, enumerator);
        _out << nl << "case " << enumerator->mappedName() << " = " << enumerator->value();
    }

    _out << nl << "public init()";
    _out << sb;
    _out << nl << "self = ." << (*enumerators.begin())->mappedName();
    _out << eb;

    _out << eb;

    _out << sp;
    _out << nl << "/// An `Ice.InputStream` extension to read `" << docName << "` enumerated values from the stream.";
    _out << nl << "public extension " << getUnqualified("Ice.InputStream", swiftModule);
    _out << sb;

    _out << sp;
    _out << nl << "/// Read an enumerated value.";
    _out << nl << "///";
    _out << nl << "/// - Returns: The enumerated value.";
    _out << nl << "func read() throws -> " << name;
    _out << sb;
    _out << nl << "let rawValue: " << enumType << " = try read(enumMaxValue: " << p->maxValue() << ")";
    _out << nl << "guard let val = " << name << "(rawValue: rawValue) else";
    _out << sb;
    _out << nl << "throw " << getUnqualified("Ice.MarshalException", swiftModule) << "(\"invalid enum value\")";
    _out << eb;
    _out << nl << "return val";
    _out << eb;

    _out << sp;
    _out << nl << "/// Read an optional enumerated value from the stream.";
    _out << nl << "///";
    _out << nl << "/// - Parameter tag: The numeric tag associated with the value.";
    _out << nl << "/// - Returns: The enumerated value.";
    _out << nl << "func read(tag: Swift.Int32) throws -> " << name << "?";
    _out << sb;
    _out << nl << "guard try readOptional(tag: tag, expectedFormat: " << optionalFormat << ") else";
    _out << sb;
    _out << nl << "return nil";
    _out << eb;
    _out << nl << "return try read() as " << name;
    _out << eb;

    _out << eb;

    _out << sp;
    _out << nl << "/// An `Ice.OutputStream` extension to write `" << docName << "` enumerated values to the stream.";
    _out << nl << "public extension " << getUnqualified("Ice.OutputStream", swiftModule);
    _out << sb;

    _out << sp;
    _out << nl << "/// Writes an enumerated value to the stream.";
    _out << nl << "///";
    _out << nl << "/// - Parameter v: The enumerator to write.";
    _out << nl << "func write(_ v: " << name << ")";
    _out << sb;
    _out << nl << "write(enum: v.rawValue, maxValue: " << p->maxValue() << ")";
    _out << eb;

    _out << sp;
    _out << nl << "/// Writes an optional enumerated value to the stream.";
    _out << nl << "///";
    _out << nl << "/// - Parameters:";
    _out << nl << "///   - tag: The numeric tag associated with the value.";
    _out << nl << "///   - value: The enumerator to write.";
    _out << nl << "func write(tag: Swift.Int32, value: " << name << "?)";
    _out << sb;
    _out << nl << "guard let v = value else";
    _out << sb;
    _out << nl << "return";
    _out << eb;
    _out << nl << "write(tag: tag, val: v.rawValue, maxValue: " << p->maxValue() << ")";
    _out << eb;

    _out << eb;
}

void
Slice::TypesVisitor::visitConst(const ConstPtr& p)
{
    const TypePtr type = p->type();
    const string swiftModule = getSwiftModule(p->getTopLevelModule());
    const string mappedName = getRelativeTypeString(p, swiftModule);

    _out << sp;
    writeDocSummary(_out, p, "constant");
    _out << nl << "public let " << mappedName << ": " << typeToString(type, p) << " = ";
    writeConstantValue(_out, type, p->valueType(), p->value(), swiftModule);
}

bool
Slice::TypesVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    InterfaceList bases = p->bases();

    const string swiftModule = getSwiftModule(p->getTopLevelModule());
    const string unescapedName = removeEscaping(getRelativeTypeString(p, swiftModule));
    const string traits = unescapedName + "Traits";
    const string prx = unescapedName + "Prx";
    const string prxI = unescapedName + "PrxI";

    // SliceTraits struct
    StringList allIds = p->ids();
    ostringstream ids;

    ids << "[";
    for (auto r = allIds.begin(); r != allIds.end(); ++r)
    {
        if (r != allIds.begin())
        {
            ids << ", ";
        }
        ids << "\"" << (*r) << "\"";
    }
    ids << "]";

    _out << sp;
    _out << nl << "/// Traits for Slice interface '" << p->name() << "'.";
    _out << nl << "public struct " << traits << ": " << getUnqualified("Ice.SliceTraits", swiftModule);
    _out << sb;
    _out << nl << "public static let staticIds = " << ids.str();
    _out << nl << "public static let staticId = \"" << p->scoped() << '"';
    _out << eb;

    // Proxy class
    _out << sp;
    writeDocSummary(_out, p, "proxy protocol");
    _out << nl << "public protocol " << prx << ": ";
    if (bases.size() == 0)
    {
        _out << getUnqualified("Ice.ObjectPrx", swiftModule);
    }
    else
    {
        _out.spar("");
        for (const auto& baseInterface : bases)
        {
            _out << (removeEscaping(getRelativeTypeString(baseInterface, swiftModule)) + "Prx");
        }
        _out.epar("");
    }
    _out << sb;
    _out << eb;

    _out << sp;
    _out << nl;
    if (swiftModule == "Ice")
    {
        _out << "internal ";
    }
    else
    {
        _out << "private ";
    }
    _out << "final class " << prxI << ": " << getUnqualified("Ice.ObjectPrxI", swiftModule) << ", " << prx
         << ", @unchecked Sendable";
    _out << sb;

    _out << nl << "public override class func ice_staticId() -> Swift.String";
    _out << sb;
    _out << nl << "return " << traits << ".staticId";
    _out << eb;

    _out << eb;

    //
    // makeProxy
    //
    _out << sp;
    _out << nl << "/// Makes a new proxy from a communicator and a proxy string.";
    _out << nl << "///";
    _out << nl << "/// - Parameters:";
    _out << nl << "///    - communicator: The communicator of the new proxy.";
    _out << nl << "///    - proxyString: The proxy string to parse.";
    _out << nl << "///    - type: The type of the new proxy.";
    _out << nl << "/// - Returns: A new proxy with the requested type.";
    _out << nl << "/// - Throws: `Ice.ParseException` if the proxy string is invalid.";
    _out << nl << "public func makeProxy(communicator: Ice.Communicator, proxyString: String, type: " << prx
         << ".Protocol) throws -> " << prx;
    _out << sb;
    _out << nl << "try communicator.makeProxyImpl(proxyString) as " << prxI;
    _out << eb;

    //
    // checkedCast
    //
    _out << sp;
    _out << nl
         << "/// Creates a new proxy from an existing proxy after confirming the target object's type via a remote "
            "invocation.";
    _out << nl << "///";
    _out << nl << "/// This call throws a local exception if a communication error occurs. You can optionally supply a";
    _out << nl << "/// facet name and a context map.";
    _out << nl << "///";
    _out << nl << "/// - Parameters:";
    _out << nl << "///   - prx: The source proxy.";
    _out << nl << "///   - type: The type of the new proxy.";
    _out << nl << "///   - facet: The optional name of the desired facet.";
    _out << nl << "///   - context: The optional context dictionary for the remote invocation.";
    _out << nl
         << "/// - Returns: A proxy with the requested type or nil if the target object does not support this type.";
    _out << nl << "/// - Throws: `Ice.LocalException` if a communication error occurs.";
    _out << nl << "public func checkedCast" << spar << ("prx: " + getUnqualified("Ice.ObjectPrx", swiftModule))
         << ("type: " + prx + ".Protocol") << ("facet: Swift.String? = nil")
         << ("context: " + getUnqualified("Ice.Context", swiftModule) + "? = nil") << epar << " async throws -> " << prx
         << "?";
    _out << sb;
    _out << nl << "return try await " << prxI << ".checkedCast(prx: prx, facet: facet, context: context) as " << prxI
         << "?";
    _out << eb;

    //
    // uncheckedCast
    //
    _out << sp;
    _out << nl << "/// Creates a new proxy from an existing proxy.";
    _out << nl << "///";
    _out << nl << "/// - Parameters:";
    _out << nl << "///   - prx: The source proxy.";
    _out << nl << "///   - type: The type of the new proxy.";
    _out << nl << "///   - facet: The optional name of the desired facet.";
    _out << nl << "/// - Returns: A new proxy with the requested type.";
    _out << nl << "public func uncheckedCast" << spar << ("prx: " + getUnqualified("Ice.ObjectPrx", swiftModule))
         << ("type: " + prx + ".Protocol") << ("facet: Swift.String? = nil") << epar << " -> " << prx;
    _out << sb;
    _out << nl << "return " << prxI << ".uncheckedCast(prx: prx, facet: facet) as " << prxI;
    _out << eb;

    //
    // ice_staticId
    //
    _out << sp;
    _out << nl << "/// Returns the Slice type id of the interface associated with this proxy type.";
    _out << nl << "///";
    _out << nl << "/// - Parameter type:  The proxy type to retrieve the type id.";
    _out << nl << "/// - Returns: The type id of the interface associated with this proxy type.";
    _out << nl << "public func ice_staticId" << spar << ("_ type: " + prx + ".Protocol") << epar << " -> Swift.String";
    _out << sb;
    _out << nl << "return " << traits << ".staticId";
    _out << eb;

    //
    // InputStream extension
    //
    _out << sp;
    _out << nl << "/// Extension to `Ice.InputStream` class to support reading proxies of type";
    _out << nl << "/// `" << prx << "`.";
    _out << nl << "public extension " << getUnqualified("Ice.InputStream", swiftModule);
    _out << sb;

    _out << nl << "/// Extracts a proxy from the stream. The stream must have been initialized with a communicator.";
    _out << nl << "///";
    _out << nl << "/// - Parameter type: The type of the proxy to be extracted.";
    _out << nl << "/// - Returns: The extracted proxy.";
    _out << nl << "func read(_ type: " << prx << ".Protocol) throws -> " << prx << "?";
    _out << sb;
    _out << nl << "return try read() as " << prxI << "?";
    _out << eb;

    _out << nl << "/// Extracts a proxy from the stream. The stream must have been initialized with a communicator.";
    _out << nl << "///";
    _out << nl << "/// - Parameters:";
    _out << nl << "///   - tag: The numeric tag associated with the value.";
    _out << nl << "///   - type: The type of the proxy to be extracted.";
    _out << nl << "/// - Returns: The extracted proxy.";
    _out << nl << "func read(tag: Swift.Int32, type: " << prx << ".Protocol) throws -> " << prx << "?";
    _out << sb;
    _out << nl << "return try read(tag: tag) as " << prxI << "?";
    _out << eb;

    _out << eb;

    _out << sp;
    _out << nl << "public extension " << prx;
    _out << sb;

    return true;
}

void
Slice::TypesVisitor::visitInterfaceDefEnd(const InterfaceDefPtr&)
{
    _out << eb;
}

void
Slice::TypesVisitor::visitOperation(const OperationPtr& op)
{
    const ParameterList inParams = op->inParameters();
    const bool returnsAnyValues = op->returnsAnyValues();
    const string swiftModule = getSwiftModule(op->getTopLevelModule());

    _out << sp;
    writeOpDocSummary(_out, op, false);
    _out << nl << "func " << op->mappedName();
    _out << spar;
    for (const auto& param : inParams)
    {
        const bool isOptional = param->isOptional();
        const string typeString = typeToString(param->type(), op, isOptional);
        const string paramName = "iceP_" + removeEscaping(param->mappedName());
        const string paramLabel = (inParams.size() == 1 ? "_" : param->mappedName());
        _out << (paramLabel + " " + paramName + ": " + typeString + (isOptional ? " = nil" : ""));
    }
    _out << "context: " + getUnqualified("Ice.Context", swiftModule) + "? = nil";
    _out << epar;
    _out << " async throws";
    if (returnsAnyValues)
    {
        _out << " -> " << operationReturnType(op);
    }

    _out << sb;

    //
    // Invoke
    //

    if (op->hasMetadata("oneway"))
    {
        _out << nl << "if _impl.ice_isTwoway()";
        _out << sb;
        _out << nl << "throw " << getUnqualified("Ice.OnewayOnlyException", swiftModule) << "(operation: \""
             << op->name() << "\")";
        _out << eb;
    }

    _out << nl << "return try await _impl._invoke(";

    _out.useCurrentPosAsIndent();
    _out << "operation: \"" << op->name() << "\",";
    _out << nl << "mode: " << modeToString(op->mode()) << ",";

    if (op->format())
    {
        _out << nl << "format: " << opFormatTypeToString(op);
        _out << ",";
    }

    if (!inParams.empty())
    {
        _out << nl << "write: ";
        writeMarshalInParams(_out, op);
        _out << ",";
    }

    if (returnsAnyValues)
    {
        _out << nl << "read: read,";
    }

    if (!op->throws().empty())
    {
        _out << nl << "userException:";
        writeUnmarshalUserException(_out, op);
        _out << ",";
    }

    _out << nl << "context: context)";

    _out.restoreIndent();

    if (returnsAnyValues)
    {
        writeUnmarshalOutParams(_out, op);
    }
    _out << eb;
}

Slice::ServantVisitor::ServantVisitor(IceInternal::Output& out) : _out(out) {}

bool
Slice::ServantVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    const string swiftModule = getSwiftModule(p->getTopLevelModule());
    const string servant = getRelativeTypeString(p, swiftModule);

    //
    // Protocol
    //
    InterfaceList bases = p->bases();
    StringList baseNames;
    for (const auto& base : bases)
    {
        baseNames.push_back(getRelativeTypeString(base, swiftModule));
    }

    _out << sp;
    writeDocSummary(_out, p, "skeleton protocol");
    _out << nl << "public protocol " << servant << ": ";
    if (baseNames.empty())
    {
        _out << getUnqualified("Ice.Dispatcher", swiftModule);
    }
    else
    {
        _out.spar("");
        for (const auto& baseName : baseNames)
        {
            _out << baseName;
        }
        _out.epar("");
    }

    _out << sb;
    return true;
}

void
Slice::ServantVisitor::visitInterfaceDefEnd(const InterfaceDefPtr&)
{
    _out << eb;
}

void
Slice::ServantVisitor::visitOperation(const OperationPtr& op)
{
    const string swiftModule = getSwiftModule(op->getTopLevelModule());

    _out << sp;
    writeOpDocSummary(_out, op, true);
    _out << nl << "func " << op->mappedName();
    _out << spar;
    for (const auto& param : op->inParameters())
    {
        const string typeString = typeToString(param->type(), op, param->isOptional());
        _out << param->mappedName() + ": " + (param->type()->usesClasses() ? "sending " : "") + typeString;
    }
    _out << ("current: " + getUnqualified("Ice.Current", swiftModule));
    _out << epar;

    _out << " async throws";
    if (op->returnsAnyValues())
    {
        _out << " -> " << operationReturnType(op);
    }
}

Slice::ServantExtVisitor::ServantExtVisitor(IceInternal::Output& out) : _out(out) {}

bool
Slice::ServantExtVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    const string swiftModule = getSwiftModule(p->getTopLevelModule());
    const string servant = getRelativeTypeString(p, swiftModule);
    const string unescapedName = removeEscaping(servant);
    const string traits = unescapedName + "Traits";

    _out << sp;
    _out << nl << "extension " << servant;
    _out << sb;
    _out << nl << "private static var defaultObject: " << getUnqualified("Ice.Object", swiftModule) << sb;
    _out << nl << getUnqualified("Ice.DefaultObject", swiftModule) << "<" << traits << ">()";
    _out << eb;

    const OperationList allOps = p->allOperations();

    list<pair<string, string>> allOpNames;
    transform(
        allOps.begin(),
        allOps.end(),
        back_inserter(allOpNames),
        [](const ContainedPtr& it) { return std::make_pair(it->name(), it->mappedName()); });

    allOpNames.emplace_back("ice_id", "ice_id");
    allOpNames.emplace_back("ice_ids", "ice_ids");
    allOpNames.emplace_back("ice_isA", "ice_isA");
    allOpNames.emplace_back("ice_ping", "ice_ping");

    _out << sp;
    _out << nl
         << "/// Dispatches an incoming request to one of the instance methods of the generated protocol, based on the "
            "operation name carried by the request.";
    _out << nl << "/// - Parameter request: The incoming request.";
    _out << nl << "/// - Returns: The outgoing response.";
    _out << nl << "public func dispatch(_ request: sending Ice.IncomingRequest) async throws -> Ice.OutgoingResponse"
         << sb;
    _out << nl << "try await Self.dispatch(self, request: request)";
    _out << eb;

    _out << sp;
    _out << nl
         << "/// Dispatches an incoming request to one of the instance methods of the generated protocol, based on the "
            "operation name carried by the request.";
    _out << nl
         << "/// Call this static method from the `dispatch` method of your servant class when you want to reuse a "
            "base servant class in a derived servant class.";
    _out << nl << "/// - Parameters:";
    _out << nl << "///   - servant: The servant to dispatch the request to.";
    _out << nl << "///   - request: The incoming request.";
    _out << nl << "/// - Returns: The outgoing response.";
    _out << nl << "public static func dispatch(_ servant: " << servant
         << ", request: sending Ice.IncomingRequest) async throws -> Ice.OutgoingResponse" << sb;
    _out << nl << "switch request.current.operation";
    _out << sb;
    _out.dec(); // to align case with switch
    for (const auto& [sliceName, mappedName] : allOpNames)
    {
        const string mappedDispatchName = "_iceD_" + removeEscaping(mappedName);
        _out << nl << "case \"" << sliceName << "\":";
        _out.inc();
        if (sliceName == "ice_id" || sliceName == "ice_ids" || sliceName == "ice_isA" || sliceName == "ice_ping")
        {
            _out << nl << "try await (servant as? Ice.Object ?? " << "Self.defaultObject)." << mappedDispatchName
                 << "(request)";
        }
        else
        {
            _out << nl << "try await servant." << mappedDispatchName << "(request)";
        }

        _out.dec();
    }
    _out << nl << "default:";
    _out.inc();
    _out << nl << "throw Ice.OperationNotExistException()";
    // missing dec to compensate for the extra dec after switch sb
    _out << eb;
    _out << eb;

    return true;
}

void
Slice::ServantExtVisitor::visitInterfaceDefEnd(const InterfaceDefPtr&)
{
    _out << eb;
}

void
Slice::ServantExtVisitor::visitOperation(const OperationPtr& op)
{
    const string opName = op->mappedName();
    const ParameterList inParams = op->inParameters();
    const bool returnsAnyValues = op->returnsAnyValues();

    _out << sp;
    _out << nl << "public func _iceD_" << removeEscaping(opName)
         << "(_ request: Ice.IncomingRequest) async throws -> Ice.OutgoingResponse";

    _out << sb;

    if (op->mode() == Operation::Mode::Normal)
    {
        _out << nl << "try request.current.checkNonIdempotent()";
    }

    if (inParams.empty())
    {
        _out << nl << "_ = try request.inputStream.skipEmptyEncapsulation()";
    }
    else
    {
        _out << nl << "let istr = request.inputStream";
        _out << nl << "_ = try istr.startEncapsulation()";
        writeUnmarshalInParams(_out, op);
    }

    _out << nl;
    if (returnsAnyValues)
    {
        _out << "let result = ";
    }

    _out << "try await self." << opName;
    _out << spar;
    for (const auto& param : inParams)
    {
        // The swift compiler reports an error if you escape an argument label when calling a function.
        // So we always need to remove escaping here.
        const string paramName = removeEscaping(param->mappedName());
        _out << (paramName + ": iceP_" + paramName);
    }
    _out << "current: request.current";
    _out << epar;

    if (!returnsAnyValues)
    {
        _out << nl << "return request.current.makeEmptyOutgoingResponse()";
    }
    else
    {
        _out << nl << "return request.current.makeOutgoingResponse(result, formatType: " << opFormatTypeToString(op)
             << ")";
        _out << sb;
        _out << " ostr, value in ";
        writeMarshalAsyncOutParams(_out, op);
        _out << eb;
    }

    _out << eb;
}
