// Copyright (c) ZeroC, Inc.
//

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
using namespace IceInternal;

namespace
{
    string getClassResolverPrefix(const UnitPtr& p)
    {
        DefinitionContextPtr dc = p->findDefinitionContext(p->topLevelFile());
        assert(dc);
        return dc->getMetadataArgs("swift:class-resolver-prefix").value_or("");
    }
}

Gen::Gen(const string& base, const vector<string>& includePaths, const string& dir)
    : _out(false, true), // No break before opening block in Swift + short empty blocks
      _includePaths(includePaths)
{
    _fileBase = base;
    string::size_type pos = base.find_last_of("/\\");
    if (pos != string::npos)
    {
        _fileBase = base.substr(pos + 1);
    }

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

Gen::~Gen()
{
    if (_out.isOpen())
    {
        _out << nl;
        _out.close();
    }
}

void
Gen::generate(const UnitPtr& p)
{
    SwiftGenerator::validateMetadata(p);
    SwiftGenerator::validateSwiftModuleMappings(p);

    ImportVisitor importVisitor(_out);
    p->visit(&importVisitor);
    importVisitor.writeImports();

    TypesVisitor typesVisitor(_out);
    p->visit(&typesVisitor);

    ServantVisitor servantVisitor(_out);
    p->visit(&servantVisitor);

    ServantExtVisitor servantExtVisitor(_out);
    p->visit(&servantExtVisitor);
}

void
Gen::printHeader()
{
    _out << "// Copyright (c) ZeroC, Inc.";
    _out << sp;
    _out << nl << "// slice2swift version " << ICE_STRING_VERSION;
}

Gen::ImportVisitor::ImportVisitor(IceInternal::Output& o) : out(o) {}

bool
Gen::ImportVisitor::visitModuleStart(const ModulePtr& p)
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
Gen::ImportVisitor::visitClassDefStart(const ClassDefPtr& p)
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
Gen::ImportVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
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
Gen::ImportVisitor::visitStructStart(const StructPtr& p)
{
    // Add imports required for data members
    for (const auto& dataMember : p->dataMembers())
    {
        addImport(dataMember->type(), p);
    }

    return true;
}

bool
Gen::ImportVisitor::visitExceptionStart(const ExceptionPtr& p)
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
Gen::ImportVisitor::visitSequence(const SequencePtr& seq)
{
    // Add import required for the sequence element type
    addImport(seq->type(), seq);
}

void
Gen::ImportVisitor::visitDictionary(const DictionaryPtr& dict)
{
    // Add imports required for the dictionary key and value types
    addImport(dict->keyType(), dict);
    addImport(dict->valueType(), dict);
}

void
Gen::ImportVisitor::writeImports()
{
    for (const auto& import : _imports)
    {
        out << nl << "import " << import;
    }
}

void
Gen::ImportVisitor::addImport(const SyntaxTreeBasePtr& p, const ContainedPtr& toplevel)
{
    // Only add imports for user defined constructs.
    auto definition = dynamic_pointer_cast<Contained>(p);
    if (definition)
    {
        string swiftM1 = getSwiftModule(getTopLevelModule(definition));
        string swiftM2 = getSwiftModule(getTopLevelModule(toplevel));
        if (swiftM1 != swiftM2)
        {
            addImport(swiftM1);
        }
    }
}

void
Gen::ImportVisitor::addImport(const string& module)
{
    if (find(_imports.begin(), _imports.end(), module) == _imports.end())
    {
        _imports.push_back(module);
    }
}

Gen::TypesVisitor::TypesVisitor(IceInternal::Output& o) : out(o) {}

bool
Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    const string prefix = getClassResolverPrefix(p->unit());
    const string swiftModule = getSwiftModule(getTopLevelModule(p));
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

    out << sp;
    out << nl << "public extension " << getUnqualified("Ice.DefaultSliceLoader", swiftModule);
    out << sb;
    out << nl << "@objc static func " << methodName.str() << "() -> AnyObject.Type";
    out << sb;
    out << nl << name << ".self";
    out << eb;
    if (p->compactId() != -1)
    {
        // Same for compact ID.
        out << sp;
        out << nl << "@objc static func resolveTypeId" << prefix << "_" << to_string(p->compactId())
            << "() -> AnyObject.Type";
        out << sb;
        out << nl << name << ".self";
        out << eb;
    }
    out << eb;

    out << sp;
    writeDocSummary(out, p);
    writeSwiftAttributes(out, p->getMetadata());
    out << nl << "open class " << name << ": ";
    if (base)
    {
        out << getRelativeTypeString(base, swiftModule);
    }
    else
    {
        out << getUnqualified("Ice.Value", swiftModule);
    }
    out << sb;

    const DataMemberList members = p->dataMembers();
    const DataMemberList baseMembers = base ? base->allDataMembers() : DataMemberList();
    const DataMemberList allMembers = p->allDataMembers();
    const DataMemberList optionalMembers = p->orderedOptionalDataMembers();

    writeMembers(out, members, p);

    if (!allMembers.empty())
    {
        if (!members.empty())
        {
            writeDefaultInitializer(out, true, !base);
            writeMemberwiseInitializer(out, members, baseMembers, allMembers, p, !base);
        }
        // else inherit the base class initializers
    }
    // else inherit Value's initializer.

    out << sp;
    out << nl << "/// - Returns: The Slice type ID of the interface supported by this object.";
    out << nl << "open override class func ice_staticId() -> Swift.String { \"" << p->scoped() << "\" }";

    out << sp;
    out << nl << "open override func _iceReadImpl(from istr: " << getUnqualified("Ice.InputStream", swiftModule)
        << ") throws";
    out << sb;
    out << nl << "_ = try istr.startSlice()";
    for (const auto& member : members)
    {
        if (!member->optional())
        {
            writeMarshalUnmarshalCode(out, member->type(), p, "self." + member->mappedName(), false);
        }
    }
    for (const auto& member : optionalMembers)
    {
        writeMarshalUnmarshalCode(out, member->type(), p, "self." + member->mappedName(), false, member->tag());
    }
    out << nl << "try istr.endSlice()";
    if (base)
    {
        out << nl << "try super._iceReadImpl(from: istr);";
    }
    out << eb;

    out << sp;
    out << nl << "open override func _iceWriteImpl(to ostr: " << getUnqualified("Ice.OutputStream", swiftModule) << ")";
    out << sb;
    out << nl << "ostr.startSlice(typeId: " << name << ".ice_staticId(), compactId: " << p->compactId()
        << ", last: " << (!base ? "true" : "false") << ")";
    for (const auto& member : members)
    {
        TypePtr type = member->type();
        if (!member->optional())
        {
            writeMarshalUnmarshalCode(out, member->type(), p, "self." + member->mappedName(), true);
        }
    }
    for (const auto& member : optionalMembers)
    {
        writeMarshalUnmarshalCode(out, member->type(), p, "self." + member->mappedName(), true, member->tag());
    }
    out << nl << "ostr.endSlice()";
    if (base)
    {
        out << nl << "super._iceWriteImpl(to: ostr);";
    }
    out << eb;

    return true;
}

void
Gen::TypesVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    out << eb;
}

bool
Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    const string swiftModule = getSwiftModule(getTopLevelModule(p));
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

    out << sp;
    out << nl << "public extension " << getUnqualified("Ice.DefaultSliceLoader", swiftModule);
    out << sb;
    out << nl << "@objc static func " << methodName.str() << "() -> AnyObject.Type";
    out << sb;
    out << nl << name << ".self";
    out << eb;
    out << eb;

    out << sp;
    writeDocSummary(out, p);
    writeSwiftAttributes(out, p->getMetadata());
    out << nl << "open class " << name << ": ";
    if (base)
    {
        out << getRelativeTypeString(base, swiftModule);
    }
    else
    {
        out << getUnqualified("Ice.UserException", swiftModule);
    }
    out << ", @unchecked Sendable";
    out << sb;

    const DataMemberList members = p->dataMembers();
    const DataMemberList allMembers = p->allDataMembers();
    const DataMemberList baseMembers = base ? base->allDataMembers() : DataMemberList();
    const DataMemberList optionalMembers = p->orderedOptionalDataMembers();

    writeMembers(out, members, p);

    if (!allMembers.empty())
    {
        if (!members.empty())
        {
            writeDefaultInitializer(out, true, !base);
            writeMemberwiseInitializer(out, members, baseMembers, allMembers, p, !base);
        }
        // else inherit the base class initializers
    }
    // else inherit UserException's initializer.

    out << sp;
    out << nl << "/// - Returns: The Slice type ID of this exception.";
    out << nl << "open override class func ice_staticId() -> Swift.String { \"" << p->scoped() << "\" }";

    out << sp;
    out << nl << "open override func _iceWriteImpl(to ostr: " << getUnqualified("Ice.OutputStream", swiftModule) << ")";
    out << sb;
    out << nl << "ostr.startSlice(typeId: " << name
        << ".ice_staticId(), compactId: -1, last: " << (!base ? "true" : "false") << ")";
    for (const auto& member : members)
    {
        if (!member->optional())
        {
            writeMarshalUnmarshalCode(out, member->type(), p, "self." + member->mappedName(), true);
        }
    }

    for (const auto& member : optionalMembers)
    {
        writeMarshalUnmarshalCode(out, member->type(), p, "self." + member->mappedName(), true, member->tag());
    }
    out << nl << "ostr.endSlice()";
    if (base)
    {
        out << nl << "super._iceWriteImpl(to: ostr);";
    }
    out << eb;

    out << sp;
    out << nl << "open override func _iceReadImpl(from istr: " << getUnqualified("Ice.InputStream", swiftModule)
        << ") throws";
    out << sb;
    out << nl << "_ = try istr.startSlice()";
    for (const auto& member : members)
    {
        if (!member->optional())
        {
            writeMarshalUnmarshalCode(out, member->type(), p, "self." + member->mappedName(), false);
        }
    }

    for (const auto& member : optionalMembers)
    {
        writeMarshalUnmarshalCode(out, member->type(), p, "self." + member->mappedName(), false, member->tag());
    }

    out << nl << "try istr.endSlice()";
    if (base)
    {
        out << nl << "try super._iceReadImpl(from: istr);";
    }
    out << eb;

    if (p->usesClasses() && !(base && base->usesClasses()))
    {
        out << sp;
        out << nl << "open override func _usesClasses() -> Swift.Bool" << sb;
        out << nl << "return true";
        out << eb;
    }

    out << eb;
    return false;
}

bool
Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    const string swiftModule = getSwiftModule(getTopLevelModule(p));
    const string name = getRelativeTypeString(p, swiftModule);
    const string docName = removeEscaping(name);
    bool isLegalKeyType = Dictionary::isLegalKeyType(p);
    const DataMemberList members = p->dataMembers();
    const string optionalFormat = getOptionalFormat(p);

    bool usesClasses = p->usesClasses();
    out << sp;
    writeDocSummary(out, p);
    writeSwiftAttributes(out, p->getMetadata());
    out << nl << "public " << (usesClasses ? "class " : "struct ") << name;

    // Only generate Hashable if this struct is a legal dictionary key type.
    if (isLegalKeyType)
    {
        out << ": Swift.Hashable";
    }
    out << sb;

    writeMembers(out, members, p);
    writeDefaultInitializer(out, false, true);
    writeMemberwiseInitializer(out, members, p);

    out << eb;

    out << sp;
    out << nl << "/// An `Ice.InputStream` extension to read `" << docName << "` structured values from the stream.";
    out << nl << "public extension " << getUnqualified("Ice.InputStream", swiftModule);
    out << sb;

    out << sp;
    out << nl << "/// Read a `" << docName << "` structured value from the stream.";
    out << nl << "///";
    out << nl << "/// - Returns: The structured value read from the stream.";
    out << nl << "func read() throws -> " << name;
    out << sb;
    out << nl << (usesClasses ? "let" : "var") << " v = " << name << "()";
    for (const auto& member : members)
    {
        writeMarshalUnmarshalCode(out, member->type(), p, "v." + member->mappedName(), false);
    }
    out << nl << "return v";
    out << eb;

    out << sp;
    out << nl << "/// Read an optional `" << docName << "?` structured value from the stream.";
    out << nl << "///";
    out << nl << "/// - Parameter tag: The numeric tag associated with the value.";
    out << nl << "///";
    out << nl << "/// - Returns: The structured value read from the stream.";
    out << nl << "func read(tag: Swift.Int32) throws -> " << name << "?";
    out << sb;
    out << nl << "guard try readOptional(tag: tag, expectedFormat: " << optionalFormat << ") else";
    out << sb;
    out << nl << "return nil";
    out << eb;
    if (p->isVariableLength())
    {
        out << nl << "try skip(4)";
    }
    else
    {
        out << nl << "try skipSize()";
    }
    out << nl << "return try read() as " << name;
    out << eb;

    out << eb;

    out << sp;
    out << nl << "/// An `Ice.OutputStream` extension to write `" << docName << "` structured values from the stream.";
    out << nl << "public extension " << getUnqualified("Ice.OutputStream", swiftModule);
    out << sb;

    out << nl << "/// Write a `" << docName << "` structured value to the stream.";
    out << nl << "///";
    out << nl << "/// - Parameter v: The value to write to the stream.";
    out << nl << "func write(_ v: " << name << ")" << sb;
    for (const auto& member : members)
    {
        writeMarshalUnmarshalCode(out, member->type(), p, "v." + member->mappedName(), true);
    }
    out << eb;

    out << sp;
    out << nl << "/// Write an optional `" << docName << "?` structured value to the stream.";
    out << nl << "///";
    out << nl << "/// - Parameter tag: The numeric tag associated with the value.";
    out << nl << "/// - Parameter value: The value to write to the stream.";
    out << nl << "func write(tag: Swift.Int32, value: " << name << "?)" << sb;
    out << nl << "if let v = value" << sb;
    out << nl << "if writeOptional(tag: tag, format: " << optionalFormat << ")" << sb;

    if (p->isVariableLength())
    {
        out << nl << "let pos = startSize()";
        out << nl << "write(v)";
        out << nl << "endSize(position: pos)";
    }
    else
    {
        out << nl << "write(size: " << p->minWireSize() << ")";
        out << nl << "write(v)";
    }
    out << eb;
    out << eb;
    out << eb;

    out << eb;

    return false;
}

void
Gen::TypesVisitor::visitSequence(const SequencePtr& p)
{
    const string swiftModule = getSwiftModule(getTopLevelModule(p));
    const string name = getRelativeTypeString(p, swiftModule);
    const string unescapedName = removeEscaping(name);

    const TypePtr type = p->type();
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);

    out << sp;
    writeDocSummary(out, p);
    out << nl << "public typealias " << name << " = ";

    if (builtin && builtin->kind() == Builtin::KindByte)
    {
        out << "Foundation.Data";
    }
    else
    {
        out << "[" << typeToString(type, p, false) << "]";
    }

    if (builtin && builtin->kind() <= Builtin::KindString)
    {
        return; // No helpers for sequence of primitive types
    }

    const string ostr = getUnqualified("Ice.OutputStream", swiftModule);
    const string istr = getUnqualified("Ice.InputStream", swiftModule);

    const string optionalFormat = getOptionalFormat(p);

    out << sp;
    out << nl << "/// Helper class to read and write `" << unescapedName << "` sequence values from";
    out << nl << "/// `Ice.InputStream` and `Ice.OutputStream`.";
    out << nl << "public struct " << unescapedName << "Helper";
    out << sb;

    out << nl << "/// Read a `" << unescapedName << "` sequence from the stream.";
    out << nl << "///";
    out << nl << "/// - Parameter istr: The stream to read from.";
    out << nl << "///";
    out << nl << "/// - Returns: The sequence read from the stream.";
    out << nl << "public static func read(from istr: " << istr << ") throws -> " << name;
    out << sb;
    out << nl << "let sz = try istr.readAndCheckSeqSize(minSize: " << p->type()->minWireSize() << ")";

    if (type->isClassType())
    {
        out << nl << "var v = " << name << "(repeating: nil, count: sz)";
        out << nl << "for i in 0 ..< sz";
        out << sb;
        out << nl << "try Swift.withUnsafeMutablePointer(to: &v[i])";
        out << sb;
        out << " p in";
        writeMarshalUnmarshalCode(out, type, p, "p.pointee", false);
        out << eb;
        out << eb;
    }
    else
    {
        out << nl << "var v = " << name << "()";
        out << nl << "v.reserveCapacity(sz)";
        out << nl << "for _ in 0 ..< sz";
        out << sb;
        string param = "let j: " + typeToString(p->type(), p);
        writeMarshalUnmarshalCode(out, type, p, param, false);
        out << nl << "v.append(j)";
        out << eb;
    }
    out << nl << "return v";
    out << eb;

    out << sp;
    out << nl << "/// Read an optional `" << unescapedName << "?` sequence from the stream.";
    out << nl << "///";
    out << nl << "/// - Parameter istr: The stream to read from.";
    out << nl << "/// - Parameter tag: The numeric tag associated with the value.";
    out << nl << "///";
    out << nl << "/// - Returns: The sequence read from the stream.";
    out << nl << "public static func read(from istr: " << istr << ", tag: Swift.Int32) throws -> " << name << "?";
    out << sb;
    out << nl << "guard try istr.readOptional(tag: tag, expectedFormat: " << optionalFormat << ") else";
    out << sb;
    out << nl << "return nil";
    out << eb;
    if (p->type()->isVariableLength())
    {
        out << nl << "try istr.skip(4)";
    }
    else if (p->type()->minWireSize() > 1)
    {
        out << nl << "try istr.skipSize()";
    }
    out << nl << "return try read(from: istr)";
    out << eb;

    out << sp;
    out << nl << "/// Write a `" << unescapedName << "` sequence to the stream.";
    out << nl << "///";
    out << nl << "/// - Parameter ostr: The stream to write to.";
    out << nl << "/// - Parameter v: The sequence value to write to the stream.";
    out << nl << "public static func write(to ostr: " << ostr << ", value v: " << name << ")";
    out << sb;
    out << nl << "ostr.write(size: v.count)";
    out << nl << "for item in v";
    out << sb;
    writeMarshalUnmarshalCode(out, type, p, "item", true);
    out << eb;
    out << eb;

    out << sp;
    out << nl << "/// Write an optional `" << unescapedName << "?` sequence to the stream.";
    out << nl << "///";
    out << nl << "/// - Parameters:";
    out << nl << "///   - ostr: The stream to write to.";
    out << nl << "///   - tag: The numeric tag associated with the value.";
    out << nl << "///   - v: The sequence value to write to the stream.";
    out << nl << "public static func write(to ostr: " << ostr << ",  tag: Swift.Int32, value v: " << name << "?)";
    out << sb;
    out << nl << "guard let val = v else";
    out << sb;
    out << nl << "return";
    out << eb;
    if (p->type()->isVariableLength())
    {
        out << nl << "if ostr.writeOptional(tag: tag, format: " << optionalFormat << ")";
        out << sb;
        out << nl << "let pos = ostr.startSize()";
        out << nl << "write(to: ostr, value: val)";
        out << nl << "ostr.endSize(position: pos)";
        out << eb;
    }
    else
    {
        if (p->type()->minWireSize() == 1)
        {
            out << nl << "if ostr.writeOptional(tag: tag, format: .VSize)";
        }
        else
        {
            out << nl << "if ostr.writeOptionalVSize(tag: tag, len: val.count, elemSize: " << p->type()->minWireSize()
                << ")";
        }
        out << sb;
        out << nl << "write(to: ostr, value: val)";
        out << eb;
    }
    out << eb;

    out << eb;
}

void
Gen::TypesVisitor::visitDictionary(const DictionaryPtr& p)
{
    const string swiftModule = getSwiftModule(getTopLevelModule(p));
    const string name = getRelativeTypeString(p, swiftModule);
    const string unescapedName = removeEscaping(name);

    const string keyType = typeToString(p->keyType(), p, false);
    const string valueType = typeToString(p->valueType(), p, false);

    out << sp;
    writeDocSummary(out, p);
    out << nl << "public typealias " << name << " = [" << keyType << ": " << valueType << "]";

    const string ostr = getUnqualified("Ice.OutputStream", swiftModule);
    const string istr = getUnqualified("Ice.InputStream", swiftModule);

    const string optionalFormat = getOptionalFormat(p);
    const bool isVariableLength = p->keyType()->isVariableLength() || p->valueType()->isVariableLength();
    const size_t minWireSize = p->keyType()->minWireSize() + p->valueType()->minWireSize();

    out << sp;
    out << nl << "/// Helper class to read and write `" << unescapedName << "` dictionary values from";
    out << nl << "/// `Ice.InputStream` and `Ice.OutputStream`.";
    out << nl << "public struct " << unescapedName << "Helper";
    out << sb;

    out << nl << "/// Read a `" << unescapedName << "` dictionary from the stream.";
    out << nl << "///";
    out << nl << "/// - Parameter istr: The stream to read from.";
    out << nl << "///";
    out << nl << "/// - Returns: The dictionary read from the stream.";
    out << nl << "public static func read(from istr: " << istr << ") throws -> " << name;
    out << sb;
    out << nl << "let sz = try Swift.Int(istr.readSize())";
    out << nl << "var v = " << name << "()";
    if (p->valueType()->isClassType())
    {
        out << nl << "let e = " << getUnqualified("Ice.DictEntryArray", swiftModule) << "<" << keyType << ", "
            << valueType << ">(size: sz)";
        out << nl << "for i in 0 ..< sz";
        out << sb;
        string keyParam = "let key: " + keyType;
        writeMarshalUnmarshalCode(out, p->keyType(), p, keyParam, false);
        out << nl << "v[key] = nil as " << valueType;
        out << nl << "Swift.withUnsafeMutablePointer(to: &v[key, default:nil])";
        out << sb;
        out << nl << "e.values[i] = Ice.DictEntry<" << keyType << ", " << valueType << ">("
            << "key: key, "
            << "value: $0)";
        out << eb;
        writeMarshalUnmarshalCode(out, p->valueType(), p, "e.values[i].value.pointee", false);
        out << eb;

        out << nl << "for i in 0..<sz" << sb;
        out << nl << "Swift.withUnsafeMutablePointer(to: &v[e.values[i].key, default:nil])";
        out << sb;
        out << nl << "e.values[i].value = $0";
        out << eb;
        out << eb;
    }
    else
    {
        out << nl << "for _ in 0 ..< sz";
        out << sb;
        string keyParam = "let key: " + keyType;
        writeMarshalUnmarshalCode(out, p->keyType(), p, keyParam, false);
        string valueParam = "let value: " + typeToString(p->valueType(), p);
        writeMarshalUnmarshalCode(out, p->valueType(), p, valueParam, false);
        out << nl << "v[key] = value";
        out << eb;
    }

    out << nl << "return v";
    out << eb;

    out << sp;
    out << nl << "/// Read an optional `" << unescapedName << "?` dictionary from the stream.";
    out << nl << "///";
    out << nl << "/// - Parameter istr: The stream to read from.";
    out << nl << "/// - Parameter tag: The numeric tag associated with the value.";
    out << nl << "///";
    out << nl << "/// - Returns: The dictionary read from the stream.";
    out << nl << "public static func read(from istr: " << istr << ", tag: Swift.Int32) throws -> " << name << "?";
    out << sb;
    out << nl << "guard try istr.readOptional(tag: tag, expectedFormat: " << optionalFormat << ") else";
    out << sb;
    out << nl << "return nil";
    out << eb;
    if (p->keyType()->isVariableLength() || p->valueType()->isVariableLength())
    {
        out << nl << "try istr.skip(4)";
    }
    else
    {
        out << nl << "try istr.skipSize()";
    }
    out << nl << "return try read(from: istr)";
    out << eb;

    out << sp;
    out << nl << "/// Write a `" << unescapedName << "` dictionary to the stream.";
    out << nl << "///";
    out << nl << "/// - Parameter ostr: The stream to write to.";
    out << nl << "/// - Parameter v: The dictionary value to write to the stream.";
    out << nl << "public static func write(to ostr: " << ostr << ", value v: " << name << ")";
    out << sb;
    out << nl << "ostr.write(size: v.count)";
    out << nl << "for (key, value) in v";
    out << sb;
    writeMarshalUnmarshalCode(out, p->keyType(), p, "key", true);
    writeMarshalUnmarshalCode(out, p->valueType(), p, "value", true);
    out << eb;
    out << eb;

    out << sp;
    out << nl << "/// Write an optional `" << unescapedName << "?` dictionary to the stream.";
    out << nl << "///";
    out << nl << "/// - Parameters:";
    out << nl << "///   - ostr: The stream to write to.";
    out << nl << "///   - tag: The numeric tag associated with the value.";
    out << nl << "///   - v: The dictionary value to write to the stream.";
    out << nl << "public static func write(to ostr: " << ostr << ", tag: Swift.Int32, value v: " << name << "?)";
    out << sb;
    out << nl << "guard let val = v else";
    out << sb;
    out << nl << "return";
    out << eb;
    if (isVariableLength)
    {
        out << nl << "if ostr.writeOptional(tag: tag, format: " << optionalFormat << ")";
        out << sb;
        out << nl << "let pos = ostr.startSize()";
        out << nl << "write(to: ostr, value: val)";
        out << nl << "ostr.endSize(position: pos)";
        out << eb;
    }
    else
    {
        out << nl << "if ostr.writeOptionalVSize(tag: tag, len: val.count, elemSize: " << minWireSize << ")";
        out << sb;
        out << nl << "write(to: ostr, value: val)";
        out << eb;
    }
    out << eb;

    out << eb;
}

void
Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    const string swiftModule = getSwiftModule(getTopLevelModule(p));
    const string name = getRelativeTypeString(p, swiftModule);
    const string docName = removeEscaping(name);
    const EnumeratorList enumerators = p->enumerators();
    const string enumType = p->maxValue() <= 0xFF ? "Swift.UInt8" : "Swift.Int32";
    const string optionalFormat = getOptionalFormat(p);

    out << sp;
    writeDocSummary(out, p);
    writeSwiftAttributes(out, p->getMetadata());
    out << nl << "public enum " << name << ": " << enumType;
    out << sb;

    for (const auto& enumerator : enumerators)
    {
        writeDocSummary(out, enumerator);
        out << nl << "case " << enumerator->mappedName() << " = " << enumerator->value();
    }

    out << nl << "public init()";
    out << sb;
    out << nl << "self = ." << (*enumerators.begin())->mappedName();
    out << eb;

    out << eb;

    out << sp;
    out << nl << "/// An `Ice.InputStream` extension to read `" << docName << "` enumerated values from the stream.";
    out << nl << "public extension " << getUnqualified("Ice.InputStream", swiftModule);
    out << sb;

    out << sp;
    out << nl << "/// Read an enumerated value.";
    out << nl << "///";
    out << nl << "/// - Returns:  The enumerated value.";
    out << nl << "func read() throws -> " << name;
    out << sb;
    out << nl << "let rawValue: " << enumType << " = try read(enumMaxValue: " << p->maxValue() << ")";
    out << nl << "guard let val = " << name << "(rawValue: rawValue) else";
    out << sb;
    out << nl << "throw " << getUnqualified("Ice.MarshalException", swiftModule) << "(\"invalid enum value\")";
    out << eb;
    out << nl << "return val";
    out << eb;

    out << sp;
    out << nl << "/// Read an optional enumerated value from the stream.";
    out << nl << "///";
    out << nl << "/// - Parameter tag: The numeric tag associated with the value.";
    out << nl << "///";
    out << nl << "/// - Returns: The enumerated value.";
    out << nl << "func read(tag: Swift.Int32) throws -> " << name << "?";
    out << sb;
    out << nl << "guard try readOptional(tag: tag, expectedFormat: " << optionalFormat << ") else";
    out << sb;
    out << nl << "return nil";
    out << eb;
    out << nl << "return try read() as " << name;
    out << eb;

    out << eb;

    out << sp;
    out << nl << "/// An `Ice.OutputStream` extension to write `" << docName << "` enumerated values to the stream.";
    out << nl << "public extension " << getUnqualified("Ice.OutputStream", swiftModule);
    out << sb;

    out << sp;
    out << nl << "/// Writes an enumerated value to the stream.";
    out << nl << "///";
    out << nl << "/// - Parameter v: The enumerator to write.";
    out << nl << "func write(_ v: " << name << ")";
    out << sb;
    out << nl << "write(enum: v.rawValue, maxValue: " << p->maxValue() << ")";
    out << eb;

    out << sp;
    out << nl << "/// Writes an optional enumerated value to the stream.";
    out << nl << "///";
    out << nl << "/// - Parameter tag: The numeric tag associated with the value.";
    out << nl << "/// - Parameter value: The enumerator to write.";
    out << nl << "func write(tag: Swift.Int32, value: " << name << "?)";
    out << sb;
    out << nl << "guard let v = value else";
    out << sb;
    out << nl << "return";
    out << eb;
    out << nl << "write(tag: tag, val: v.rawValue, maxValue: " << p->maxValue() << ")";
    out << eb;

    out << eb;
}

void
Gen::TypesVisitor::visitConst(const ConstPtr& p)
{
    const TypePtr type = p->type();
    const string swiftModule = getSwiftModule(getTopLevelModule(p));

    writeDocSummary(out, p);
    out << nl << "public let " << p->mappedName() << ": " << typeToString(type, p) << " = ";
    writeConstantValue(out, type, p->valueType(), p->value(), swiftModule);
    out << nl;
}

bool
Gen::TypesVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    InterfaceList bases = p->bases();

    const string swiftModule = getSwiftModule(getTopLevelModule(p));
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

    out << sp;
    out << nl << "/// Traits for Slice interface '" << p->name() << "'.";
    out << nl << "public struct " << traits << ": " << getUnqualified("Ice.SliceTraits", swiftModule);
    out << sb;
    out << nl << "public static let staticIds = " << ids.str();
    out << nl << "public static let staticId = \"" << p->scoped() << '"';
    out << eb;

    // Proxy class
    out << sp;
    writeProxyDocSummary(out, p, swiftModule);
    out << nl << "public protocol " << prx << ":";
    if (bases.size() == 0)
    {
        out << " " << getUnqualified("Ice.ObjectPrx", swiftModule);
    }
    else
    {
        for (auto i = bases.begin(); i != bases.end();)
        {
            out << " " << removeEscaping(getRelativeTypeString(*i, swiftModule)) << "Prx";
            if (++i != bases.end())
            {
                out << ",";
            }
        }
    }
    out << sb;
    out << eb;

    out << sp;
    out << nl;
    if (swiftModule == "Ice")
    {
        out << "internal ";
    }
    else
    {
        out << "private ";
    }
    out << "final class " << prxI << ": " << getUnqualified("Ice.ObjectPrxI", swiftModule) << ", " << prx;
    out << sb;

    out << nl << "public override class func ice_staticId() -> Swift.String";
    out << sb;
    out << nl << "return " << traits << ".staticId";
    out << eb;

    out << eb;

    //
    // makeProxy
    //
    out << sp;
    out << nl << "/// Makes a new proxy from a communicator and a proxy string.";
    out << nl << "///";
    out << nl << "/// - Parameters:";
    out << nl << "///    - communicator: The communicator of the new proxy.";
    out << nl << "///    - proxyString: The proxy string to parse.";
    out << nl << "///    - type: The type of the new proxy.";
    out << nl << "///";
    out << nl << "/// - Throws: `Ice.ParseException` if the proxy string is invalid.";
    out << nl << "///";
    out << nl << "/// - Returns: A new proxy with the requested type.";
    out << nl << "public func makeProxy(communicator: Ice.Communicator, proxyString: String, type: " << prx
        << ".Protocol) throws -> " << prx;
    out << sb;
    out << nl << "try communicator.makeProxyImpl(proxyString) as " << prxI;
    out << eb;

    //
    // checkedCast
    //
    out << sp;
    out << nl << "/// Casts a proxy to the requested type. This call contacts the server and verifies that the object";
    out << nl << "/// implements this type.";
    out << nl << "///";
    out << nl << "/// It will throw a local exception if a communication error occurs. You can optionally supply a";
    out << nl << "/// facet name and a context map.";
    out << nl << "///";
    out << nl << "/// - Parameters:";
    out << nl << "///   - prx: The proxy to be cast.";
    out << nl << "///   - type: The proxy type to cast to.";
    out << nl << "///   - facet: The optional name of the desired facet.";
    out << nl << "///   - context: The optional context dictionary for the remote invocation.";
    out << nl << "///";
    out << nl << "/// - Returns: A proxy with the requested type or nil if the objet does not support this type.";
    out << nl << "///";
    out << nl << "/// - Throws: `Ice.LocalException` if a communication error occurs.";
    out << nl << "public func checkedCast" << spar << ("prx: " + getUnqualified("Ice.ObjectPrx", swiftModule))
        << ("type: " + prx + ".Protocol") << ("facet: Swift.String? = nil")
        << ("context: " + getUnqualified("Ice.Context", swiftModule) + "? = nil") << epar << " async throws -> " << prx
        << "?";
    out << sb;
    out << nl << "return try await " << prxI << ".checkedCast(prx: prx, facet: facet, context: context) as " << prxI
        << "?";
    out << eb;

    //
    // uncheckedCast
    //
    out << sp;
    out << nl << "/// Downcasts the given proxy to this type without contacting the remote server.";
    out << nl << "///";
    out << nl << "/// - Parameters:";
    out << nl << "///   - prx: The proxy to be cast.";
    out << nl << "///   - type: The proxy type to cast to.";
    out << nl << "///   - facet: The optional name of the desired facet.";
    out << nl << "///";
    out << nl << "/// - Returns: A proxy with the requested type.";
    out << nl << "public func uncheckedCast" << spar << ("prx: " + getUnqualified("Ice.ObjectPrx", swiftModule))
        << ("type: " + prx + ".Protocol") << ("facet: Swift.String? = nil") << epar << " -> " << prx;
    out << sb;
    out << nl << "return " << prxI << ".uncheckedCast(prx: prx, facet: facet) as " << prxI;
    out << eb;

    //
    // ice_staticId
    //
    out << sp;
    out << nl << "/// Returns the Slice type id of the interface associated with this proxy type.";
    out << nl << "///";
    out << nl << "/// - Parameter type:  The proxy type to retrieve the type id.";
    out << nl << "///";
    out << nl << "/// - Returns: The type id of the interface associated with this proxy type.";
    out << nl << "public func ice_staticId" << spar << ("_ type: " + prx + ".Protocol") << epar << " -> Swift.String";
    out << sb;
    out << nl << "return " << traits << ".staticId";
    out << eb;

    //
    // InputStream extension
    //
    out << sp;
    out << nl << "/// Extension to `Ice.InputStream` class to support reading proxies of type";
    out << nl << "/// `" << prx << "`.";
    out << nl << "public extension " << getUnqualified("Ice.InputStream", swiftModule);
    out << sb;

    out << nl << "/// Extracts a proxy from the stream. The stream must have been initialized with a communicator.";
    out << nl << "///";
    out << nl << "/// - Parameter type: The type of the proxy to be extracted.";
    out << nl << "///";
    out << nl << "/// - Returns: The extracted proxy.";
    out << nl << "func read(_ type: " << prx << ".Protocol) throws -> " << prx << "?";
    out << sb;
    out << nl << "return try read() as " << prxI << "?";
    out << eb;

    out << nl << "/// Extracts a proxy from the stream. The stream must have been initialized with a communicator.";
    out << nl << "///";
    out << nl << "/// - Parameter tag:  The numeric tag associated with the value.";
    out << nl << "/// - Parameter type: The type of the proxy to be extracted.";
    out << nl << "///";
    out << nl << "/// - Returns: The extracted proxy.";
    out << nl << "func read(tag: Swift.Int32, type: " << prx << ".Protocol) throws -> " << prx << "?";
    out << sb;
    out << nl << "return try read(tag: tag) as " << prxI << "?";
    out << eb;

    out << eb;

    out << sp;
    writeProxyDocSummary(out, p, swiftModule);
    out << nl << "public extension " << prx;
    out << sb;

    return true;
}

void
Gen::TypesVisitor::visitInterfaceDefEnd(const InterfaceDefPtr&)
{
    out << eb;
}

void
Gen::TypesVisitor::visitOperation(const OperationPtr& op)
{
    writeProxyOperation(out, op);
}

Gen::ServantVisitor::ServantVisitor(::IceInternal::Output& o) : out(o) {}

bool
Gen::ServantVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    const string swiftModule = getSwiftModule(getTopLevelModule(p));

    const string servant = getRelativeTypeString(p, swiftModule);
    const string unescapedName = removeEscaping(servant);
    const string disp = unescapedName + "Disp";
    const string traits = unescapedName + "Traits";

    //
    // Disp struct
    //
    out << sp;
    out << sp;
    out << nl << "/// Dispatcher for `" << unescapedName << "` servants.";
    out << nl << "public struct " << disp << ": Ice.Dispatcher";
    out << sb;
    out << nl << "public let servant: " << servant;

    out << nl << "private static let defaultObject = " << getUnqualified("Ice.ObjectI", swiftModule) << "<" << traits
        << ">()";

    out << sp;
    out << nl << "public init(_ servant: " << servant << ")";
    out << sb;
    out << nl << "self.servant = servant";
    out << eb;

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

    out << sp;
    out << nl;
    out << "public func dispatch(_ request: Ice.IncomingRequest) async throws -> Ice.OutgoingResponse";
    out << sb;
    out << nl;
    out << "switch request.current.operation";
    out << sb;
    out.dec(); // to align case with switch
    for (const auto& [sliceName, mappedName] : allOpNames)
    {
        const string mappedDispatchName = "_iceD_" + removeEscaping(mappedName);
        out << nl << "case \"" << sliceName << "\":";
        out.inc();
        if (sliceName == "ice_id" || sliceName == "ice_ids" || sliceName == "ice_isA" || sliceName == "ice_ping")
        {
            out << nl << "try await (servant as? Ice.Object ?? " << disp << ".defaultObject)." << mappedDispatchName
                << "(request)";
        }
        else
        {
            out << nl << "try await servant." << mappedDispatchName << "(request)";
        }

        out.dec();
    }
    out << nl << "default:";
    out.inc();
    out << nl << "throw Ice.OperationNotExistException()";
    // missing dec to compensate for the extra dec after switch sb
    out << eb;
    out << eb;

    out << eb;

    //
    // Protocol
    //
    InterfaceList bases = p->bases();
    StringList baseNames;
    for (const auto& base : bases)
    {
        baseNames.push_back(getRelativeTypeString(base, swiftModule));
    }

    out << sp;
    writeDocSummary(out, p);
    out << nl << "public protocol " << servant;
    if (!baseNames.empty())
    {
        out << ":";
    }

    for (auto i = baseNames.begin(); i != baseNames.end();)
    {
        out << " " << (*i);
        if (++i != baseNames.end())
        {
            out << ",";
        }
    }

    out << sb;

    return true;
}

void
Gen::ServantVisitor::visitInterfaceDefEnd(const InterfaceDefPtr&)
{
    out << eb;
}

void
Gen::ServantVisitor::visitOperation(const OperationPtr& op)
{
    const string swiftModule = getSwiftModule(getTopLevelModule(op));

    out << sp;
    writeOpDocSummary(out, op, true);
    out << nl << "func " << op->mappedName();
    out << spar;
    for (const auto& param : op->inParameters())
    {
        const string typeString = typeToString(param->type(), op, param->optional());
        out << param->mappedName() + ": " + typeString;
    }
    out << ("current: " + getUnqualified("Ice.Current", swiftModule));
    out << epar;

    out << " async throws";
    if (op->returnsAnyValues())
    {
        out << " -> " << operationReturnType(op);
    }
}

Gen::ServantExtVisitor::ServantExtVisitor(::IceInternal::Output& o) : out(o) {}

bool
Gen::ServantExtVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    const string swiftModule = getSwiftModule(getTopLevelModule(p));

    out << sp;
    writeServantDocSummary(out, p, swiftModule);
    out << nl << "extension " << getRelativeTypeString(p, swiftModule);

    out << sb;
    return true;
}

void
Gen::ServantExtVisitor::visitInterfaceDefEnd(const InterfaceDefPtr&)
{
    out << eb;
}

void
Gen::ServantExtVisitor::visitOperation(const OperationPtr& op)
{
    writeDispatchOperation(out, op);
}
