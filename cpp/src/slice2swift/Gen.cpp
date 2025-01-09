//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
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
        throw FileException(__FILE__, __LINE__, os.str());
    }
    FileTracker::instance()->addFile(file);

    printHeader();
    printGeneratedHeader(_out, _fileBase + ".ice");

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

    ImportVisitor importVisitor(_out);
    p->visit(&importVisitor);
    importVisitor.writeImports();

    TypesVisitor typesVisitor(_out);
    p->visit(&typesVisitor);

    ProxyVisitor proxyVisitor(_out);
    p->visit(&proxyVisitor);

    ValueVisitor valueVisitor(_out);
    p->visit(&valueVisitor);

    ObjectVisitor objectVisitor(_out);
    p->visit(&objectVisitor);

    ObjectExtVisitor objectExtVisitor(_out);
    p->visit(&objectExtVisitor);
}

void
Gen::printHeader()
{
    static const char* header = "//\n"
                                "// Copyright (c) ZeroC, Inc. All rights reserved.\n"
                                "//\n";

    _out << header;
    _out << "//\n";
    _out << "// Ice version " << ICE_STRING_VERSION << "\n";
    _out << "//\n";
}

Gen::ImportVisitor::ImportVisitor(IceInternal::Output& o) : out(o) {}

bool
Gen::ImportVisitor::visitModuleStart(const ModulePtr& p)
{
    //
    // Always import Ice module first if not building Ice
    //
    if (dynamic_pointer_cast<Unit>(p->container()) && _imports.empty())
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
    //
    // Add imports required for base class
    //
    ClassDefPtr base = p->base();
    if (base)
    {
        addImport(dynamic_pointer_cast<Contained>(base), p);
    }

    //
    // Add imports required for data members
    //
    const DataMemberList allDataMembers = p->allDataMembers();
    for (DataMemberList::const_iterator i = allDataMembers.begin(); i != allDataMembers.end(); ++i)
    {
        addImport((*i)->type(), p);
    }

    return false;
}

bool
Gen::ImportVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    //
    // Add imports required for base interfaces
    //
    InterfaceList bases = p->bases();
    for (InterfaceList::const_iterator i = bases.begin(); i != bases.end(); ++i)
    {
        addImport(dynamic_pointer_cast<Contained>(*i), p);
    }

    //
    // Add imports required for operation parameters and return type
    //
    const OperationList operationList = p->allOperations();
    for (OperationList::const_iterator i = operationList.begin(); i != operationList.end(); ++i)
    {
        const TypePtr ret = (*i)->returnType();
        if (ret && ret->definitionContext())
        {
            addImport(ret, p);
        }

        const ParameterList paramList = (*i)->parameters();
        for (ParameterList::const_iterator j = paramList.begin(); j != paramList.end(); ++j)
        {
            addImport((*j)->type(), p);
        }
    }

    return false;
}

bool
Gen::ImportVisitor::visitStructStart(const StructPtr& p)
{
    //
    // Add imports required for data members
    //
    const DataMemberList dataMembers = p->dataMembers();
    for (DataMemberList::const_iterator i = dataMembers.begin(); i != dataMembers.end(); ++i)
    {
        addImport((*i)->type(), p);
    }

    return true;
}

bool
Gen::ImportVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    //
    // Add imports required for base exceptions
    //
    ExceptionPtr base = p->base();
    if (base)
    {
        addImport(dynamic_pointer_cast<Contained>(base), p);
    }

    //
    // Add imports required for data members
    //
    const DataMemberList allDataMembers = p->allDataMembers();
    for (DataMemberList::const_iterator i = allDataMembers.begin(); i != allDataMembers.end(); ++i)
    {
        addImport((*i)->type(), p);
    }
    return true;
}

void
Gen::ImportVisitor::visitSequence(const SequencePtr& seq)
{
    //
    // Add import required for the sequence element type
    //
    addImport(seq->type(), seq);
}

void
Gen::ImportVisitor::visitDictionary(const DictionaryPtr& dict)
{
    //
    // Add imports required for the dictionary key and value types
    //
    addImport(dict->keyType(), dict);
    addImport(dict->valueType(), dict);
}

void
Gen::ImportVisitor::writeImports()
{
    for (vector<string>::const_iterator i = _imports.begin(); i != _imports.end(); ++i)
    {
        out << nl << "import " << *i;
    }
}

void
Gen::ImportVisitor::addImport(const TypePtr& definition, const ContainedPtr& toplevel)
{
    if (!dynamic_pointer_cast<Builtin>(definition))
    {
        ModulePtr m1 = getTopLevelModule(definition);
        ModulePtr m2 = getTopLevelModule(toplevel);

        string swiftM1 = getSwiftModule(m1);
        string swiftM2 = getSwiftModule(m2);
        if (swiftM1 != swiftM2 && find(_imports.begin(), _imports.end(), swiftM1) == _imports.end())
        {
            _imports.push_back(swiftM1);
        }
    }
}

void
Gen::ImportVisitor::addImport(const ContainedPtr& definition, const ContainedPtr& toplevel)
{
    ModulePtr m1 = getTopLevelModule(definition);
    ModulePtr m2 = getTopLevelModule(toplevel);

    string swiftM1 = getSwiftModule(m1);
    string swiftM2 = getSwiftModule(m2);
    if (swiftM1 != swiftM2 && find(_imports.begin(), _imports.end(), swiftM1) == _imports.end())
    {
        _imports.push_back(swiftM1);
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
Gen::TypesVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    const string swiftModule = getSwiftModule(getTopLevelModule(dynamic_pointer_cast<Contained>(p)));
    const string name = fixIdent(getRelativeTypeString(p, swiftModule));
    const string traits = fixIdent(getRelativeTypeString(p, swiftModule) + "Traits");

    StringList allIds = p->ids();
    ostringstream ids;

    ids << "[";
    for (StringList::const_iterator r = allIds.begin(); r != allIds.end(); ++r)
    {
        if (r != allIds.begin())
        {
            ids << ", ";
        }
        ids << "\"" << (*r) << "\"";
    }
    ids << "]";

    out << sp;
    out << nl << "/// Traits for Slice interface '" << name << "`.";
    out << nl << "public struct " << traits << ": " << getUnqualified("Ice.SliceTraits", swiftModule);
    out << sb;
    out << nl << "public static let staticIds = " << ids.str();
    out << nl << "public static let staticId = \"" << p->scoped() << '"';
    out << eb;

    return false;
}

bool
Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    const string swiftModule = getSwiftModule(getTopLevelModule(dynamic_pointer_cast<Contained>(p)));
    const string name = getRelativeTypeString(p, swiftModule);

    ExceptionPtr base = p->base();

    const string prefix = getClassResolverPrefix(p->unit());

    //
    // For each UserException class we generate an extension in ClassResolver
    //
    ostringstream factory;
    factory << prefix;
    vector<string> parts = splitScopedName(p->scoped());
    for (vector<string>::const_iterator it = parts.begin(); it != parts.end();)
    {
        factory << (*it);
        if (++it != parts.end())
        {
            factory << "_";
        }
    }

    out << sp;
    out << nl << "@_documentation(visibility: internal)";
    out << nl << "public class " << name
        << "_TypeResolver: " << getUnqualified("Ice.UserExceptionTypeResolver", swiftModule);
    out << sb;
    out << nl << "public override func type() -> " << getUnqualified("Ice.UserException.Type", swiftModule);
    out << sb;
    out << nl << "return " << fixIdent(name) << ".self";
    out << eb;
    out << eb;

    out << sp;
    out << nl << "public extension " << getUnqualified("Ice.ClassResolver", swiftModule);
    out << sb;
    out << nl << "@objc static func " << factory.str() << "() -> "
        << getUnqualified("Ice.UserExceptionTypeResolver", swiftModule);
    out << sb;
    out << nl << "return " << name << "_TypeResolver()";
    out << eb;
    out << eb;

    out << sp;
    writeDocSummary(out, p);
    writeSwiftAttributes(out, p->getMetadata());
    out << nl << "open class " << fixIdent(name) << ": ";
    if (base)
    {
        out << fixIdent(getRelativeTypeString(base, swiftModule));
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
    out << nl << "ostr.startSlice(typeId: " << fixIdent(name)
        << ".ice_staticId(), compactId: -1, last: " << (!base ? "true" : "false") << ")";
    for (DataMemberList::const_iterator i = members.begin(); i != members.end(); ++i)
    {
        DataMemberPtr member = *i;
        if (!member->optional())
        {
            writeMarshalUnmarshalCode(out, member->type(), p, "self." + fixIdent(member->name()), true);
        }
    }

    for (DataMemberList::const_iterator i = optionalMembers.begin(); i != optionalMembers.end(); ++i)
    {
        DataMemberPtr member = *i;
        writeMarshalUnmarshalCode(out, member->type(), p, "self." + fixIdent(member->name()), true, member->tag());
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
    for (DataMemberList::const_iterator i = members.begin(); i != members.end(); ++i)
    {
        DataMemberPtr member = *i;
        if (!member->optional())
        {
            writeMarshalUnmarshalCode(out, member->type(), p, "self." + fixIdent(member->name()), false);
        }
    }

    for (DataMemberList::const_iterator i = optionalMembers.begin(); i != optionalMembers.end(); ++i)
    {
        DataMemberPtr member = *i;
        writeMarshalUnmarshalCode(out, member->type(), p, "self." + fixIdent(member->name()), false, member->tag());
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
    const string swiftModule = getSwiftModule(getTopLevelModule(dynamic_pointer_cast<Contained>(p)));
    const string name = fixIdent(getRelativeTypeString(p, swiftModule));
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
    out << nl << "/// An `Ice.InputStream` extension to read '" << name << "` structured values from the stream.";
    out << nl << "public extension " << getUnqualified("Ice.InputStream", swiftModule);
    out << sb;

    out << sp;
    out << nl << "/// Read a '" << name << "` structured value from the stream.";
    out << nl << "///";
    out << nl << "/// - Returns: The structured value read from the stream.";
    out << nl << "func read() throws -> " << name;
    out << sb;
    out << nl << (usesClasses ? "let" : "var") << " v = " << name << "()";
    for (const auto& member : members)
    {
        writeMarshalUnmarshalCode(out, member->type(), p, "v." + fixIdent(member->name()), false);
    }
    out << nl << "return v";
    out << eb;

    out << sp;
    out << nl << "/// Read an optional '" << name << "?` structured value from the stream.";
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
    out << nl << "/// An `Ice.OutputStream` extension to write '" << name << "` structured values from the stream.";
    out << nl << "public extension " << getUnqualified("Ice.OutputStream", swiftModule);
    out << sb;

    out << nl << "/// Write a '" << name << "` structured value to the stream.";
    out << nl << "///";
    out << nl << "/// - Parameter v: The value to write to the stream.";
    out << nl << "func write(_ v: " << name << ")" << sb;
    for (const auto& member : members)
    {
        writeMarshalUnmarshalCode(out, member->type(), p, "v." + fixIdent(member->name()), true);
    }
    out << eb;

    out << sp;
    out << nl << "/// Write an optional '" << name << "?` structured value to the stream.";
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
    const string swiftModule = getSwiftModule(getTopLevelModule(dynamic_pointer_cast<Contained>(p)));
    const string name = getRelativeTypeString(p, swiftModule);

    const TypePtr type = p->type();
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(p->type());

    out << sp;
    writeDocSummary(out, p);
    out << nl << "public typealias " << fixIdent(name) << " = ";

    if (builtin && builtin->kind() == Builtin::KindByte)
    {
        out << "Foundation.Data";
    }
    else
    {
        out << "[" << typeToString(p->type(), p, false) << "]";
    }

    if (builtin && builtin->kind() <= Builtin::KindString)
    {
        return; // No helpers for sequence of primitive types
    }

    const string ostr = getUnqualified("Ice.OutputStream", swiftModule);
    const string istr = getUnqualified("Ice.InputStream", swiftModule);

    const string optionalFormat = getOptionalFormat(p);

    out << sp;
    out << nl << "/// Helper class to read and write '" << fixIdent(name) << "` sequence values from";
    out << nl << "/// `Ice.InputStream` and `Ice.OutputStream`.";
    out << nl << "public struct " << name << "Helper";
    out << sb;

    out << nl << "/// Read a '" << fixIdent(name) << "` sequence from the stream.";
    out << nl << "///";
    out << nl << "/// - Parameter istr: The stream to read from.";
    out << nl << "///";
    out << nl << "/// - Returns: The sequence read from the stream.";
    out << nl << "public static func read(from istr: " << istr << ") throws -> " << fixIdent(name);
    out << sb;
    out << nl << "let sz = try istr.readAndCheckSeqSize(minSize: " << p->type()->minWireSize() << ")";

    if (type->isClassType())
    {
        out << nl << "var v = " << fixIdent(name) << "(repeating: nil, count: sz)";
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
        out << nl << "var v = " << fixIdent(name) << "()";
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
    out << nl << "/// Read an optional '" << fixIdent(name) << "?` sequence from the stream.";
    out << nl << "///";
    out << nl << "/// - Parameter istr: The stream to read from.";
    out << nl << "/// - Parameter tag: The numeric tag associated with the value.";
    out << nl << "///";
    out << nl << "/// - Returns: The sequence read from the stream.";
    out << nl << "public static func read(from istr: " << istr << ", tag: Swift.Int32) throws -> " << fixIdent(name)
        << "?";
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
    out << nl << "/// Write a '" << fixIdent(name) << "` sequence to the stream.";
    out << nl << "///";
    out << nl << "/// - Parameter ostr: The stream to write to.";
    out << nl << "/// - Parameter value: The sequence value to write to the stream.";
    out << nl << "public static func write(to ostr: " << ostr << ", value v: " << fixIdent(name) << ")";
    out << sb;
    out << nl << "ostr.write(size: v.count)";
    out << nl << "for item in v";
    out << sb;
    writeMarshalUnmarshalCode(out, type, p, "item", true);
    out << eb;
    out << eb;

    out << sp;
    out << nl << "/// Write an optional '" << fixIdent(name) << "?` sequence to the stream.";
    out << nl << "///";
    out << nl << "/// - Parameters:";
    out << nl << "///   - ostr: The stream to write to.";
    out << nl << "///   - tag: The numeric tag associated with the value.";
    out << nl << "///   - value: The sequence value to write to the stream.";
    out << nl << "public static func write(to ostr: " << ostr << ",  tag: Swift.Int32, value v: " << fixIdent(name)
        << "?)";
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
    const string swiftModule = getSwiftModule(getTopLevelModule(dynamic_pointer_cast<Contained>(p)));
    const string name = getRelativeTypeString(p, swiftModule);

    const string keyType = typeToString(p->keyType(), p, false);
    const string valueType = typeToString(p->valueType(), p, false);
    out << sp;
    writeDocSummary(out, p);
    out << nl << "public typealias " << fixIdent(name) << " = [" << keyType << ": " << valueType << "]";

    const string ostr = getUnqualified("Ice.OutputStream", swiftModule);
    const string istr = getUnqualified("Ice.InputStream", swiftModule);

    const string optionalFormat = getOptionalFormat(p);
    const bool isVariableLength = p->keyType()->isVariableLength() || p->valueType()->isVariableLength();
    const size_t minWireSize = p->keyType()->minWireSize() + p->valueType()->minWireSize();

    out << sp;
    out << nl << "/// Helper class to read and write '" << fixIdent(name) << "` dictionary values from";
    out << nl << "/// `Ice.InputStream` and `Ice.OutputStream`.";
    out << nl << "public struct " << name << "Helper";
    out << sb;

    out << nl << "/// Read a '" << fixIdent(name) << "` dictionary from the stream.";
    out << nl << "///";
    out << nl << "/// - Parameter istr: The stream to read from.";
    out << nl << "///";
    out << nl << "/// - Returns: The dictionary read from the stream.";
    out << nl << "public static func read(from istr: " << istr << ") throws -> " << fixIdent(name);
    out << sb;
    out << nl << "let sz = try Swift.Int(istr.readSize())";
    out << nl << "var v = " << fixIdent(name) << "()";
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
    out << nl << "/// Read an optional '" << fixIdent(name) << "?` dictionary from the stream.";
    out << nl << "///";
    out << nl << "/// - Parameter istr: The stream to read from.";
    out << nl << "/// - Parameter tag: The numeric tag associated with the value.";
    out << nl << "///";
    out << nl << "/// - Returns: The dictionary read from the stream.";
    out << nl << "public static func read(from istr: " << istr << ", tag: Swift.Int32) throws -> " << fixIdent(name)
        << "?";
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
    out << nl << "/// Write a '" << fixIdent(name) << "` dictionary to the stream.";
    out << nl << "///";
    out << nl << "/// - Parameter ostr: The stream to write to.";
    out << nl << "/// - Parameter value: The dictionary value to write to the stream.";
    out << nl << "public static func write(to ostr: " << ostr << ", value v: " << fixIdent(name) << ")";
    out << sb;
    out << nl << "ostr.write(size: v.count)";
    out << nl << "for (key, value) in v";
    out << sb;
    writeMarshalUnmarshalCode(out, p->keyType(), p, "key", true);
    writeMarshalUnmarshalCode(out, p->valueType(), p, "value", true);
    out << eb;
    out << eb;

    out << sp;
    out << nl << "/// Write an optional '" << fixIdent(name) << "?` dictionary to the stream.";
    out << nl << "///";
    out << nl << "/// - Parameters:";
    out << nl << "///   - ostr: The stream to write to.";
    out << nl << "///   - tag: The numeric tag associated with the value.";
    out << nl << "///   - value: The dictionary value to write to the stream.";
    out << nl << "public static func write(to ostr: " << ostr << ", tag: Swift.Int32, value v: " << fixIdent(name)
        << "?)";
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
    const string swiftModule = getSwiftModule(getTopLevelModule(dynamic_pointer_cast<Contained>(p)));
    const string name = fixIdent(getRelativeTypeString(p, swiftModule));
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
        out << nl << "case " << fixIdent(enumerator->name()) << " = " << enumerator->value();
    }

    out << nl << "public init()";
    out << sb;
    out << nl << "self = ." << fixIdent((*enumerators.begin())->name());
    out << eb;

    out << eb;

    out << sp;
    out << nl << "/// An `Ice.InputStream` extension to read '" << name << "` enumerated values from the stream.";
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
    out << nl << "/// An `Ice.OutputStream` extension to write '" << name << "` enumerated values to the stream.";
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
    const string name = fixIdent(p->name());
    const TypePtr type = p->type();
    const string swiftModule = getSwiftModule(getTopLevelModule(dynamic_pointer_cast<Contained>(p)));

    writeDocSummary(out, p);
    out << nl << "public let " << name << ": " << typeToString(type, p) << " = ";
    writeConstantValue(out, type, p->valueType(), p->value(), swiftModule);
    out << nl;
}

Gen::ProxyVisitor::ProxyVisitor(::IceInternal::Output& o) : out(o) {}

bool
Gen::ProxyVisitor::visitModuleStart(const ModulePtr& p)
{
    return p->contains<InterfaceDef>();
}

bool
Gen::ProxyVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    InterfaceList bases = p->bases();

    const string swiftModule = getSwiftModule(getTopLevelModule(dynamic_pointer_cast<Contained>(p)));
    const string name = getRelativeTypeString(p, swiftModule);
    const string traits = name + "Traits";
    const string prx = name + "Prx";
    const string prxI = name + "PrxI";

    out << sp;
    writeProxyDocSummary(out, p, swiftModule);
    out << nl << "public protocol " << prx << ":";
    if (bases.size() == 0)
    {
        out << " " << getUnqualified("Ice.ObjectPrx", swiftModule);
    }
    else
    {
        for (InterfaceList::const_iterator i = bases.begin(); i != bases.end();)
        {
            out << " " << getRelativeTypeString(*i, swiftModule) << "Prx";
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
    out << nl << "/// '" << prx << "`.";
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
Gen::ProxyVisitor::visitInterfaceDefEnd(const InterfaceDefPtr&)
{
    out << eb;
}

void
Gen::ProxyVisitor::visitOperation(const OperationPtr& op)
{
    writeProxyOperation(out, op);
}

Gen::ValueVisitor::ValueVisitor(::IceInternal::Output& o) : out(o) {}

bool
Gen::ValueVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    const string prefix = getClassResolverPrefix(p->unit());
    const string swiftModule = getSwiftModule(getTopLevelModule(dynamic_pointer_cast<Contained>(p)));
    const string name = getRelativeTypeString(p, swiftModule);

    ClassDefPtr base = p->base();

    out << sp;
    out << nl << "@_documentation(visibility: internal)";
    out << nl << "public class " << name << "_TypeResolver: " << getUnqualified("Ice.ValueTypeResolver", swiftModule);
    out << sb;
    out << nl << "public override func type() -> " << getUnqualified("Ice.Value.Type", swiftModule);
    out << sb;
    out << nl << "return " << fixIdent(name) << ".self";
    out << eb;
    out << eb;

    if (p->compactId() >= 0)
    {
        //
        // For each Value class using a compact id we generate an extension
        // method in TypeIdResolver.
        //
        out << sp;
        out << nl << "public extension " << getUnqualified("Ice.TypeIdResolver", swiftModule);
        out << sb;
        out << nl << "@objc static func TypeId_" << p->compactId() << "() -> Swift.String";
        out << sb;
        out << nl << "return \"" << p->scoped() << "\"";
        out << eb;
        out << eb;
    }

    //
    // For each Value class we generate an extension method in ClassResolver
    //
    ostringstream factory;
    factory << prefix;
    vector<string> parts = splitScopedName(p->scoped());
    for (vector<string>::const_iterator it = parts.begin(); it != parts.end();)
    {
        factory << (*it);
        if (++it != parts.end())
        {
            factory << "_";
        }
    }

    out << sp;
    out << nl << "public extension " << getUnqualified("Ice.ClassResolver", swiftModule);
    out << sb;
    out << nl << "@objc static func " << factory.str() << "() -> "
        << getUnqualified("Ice.ValueTypeResolver", swiftModule);
    out << sb;
    out << nl << "return " << name << "_TypeResolver()";
    out << eb;
    out << eb;

    out << sp;
    writeDocSummary(out, p);
    writeSwiftAttributes(out, p->getMetadata());
    out << nl << "open class " << fixIdent(name) << ": ";
    if (base)
    {
        out << fixIdent(getRelativeTypeString(base, swiftModule));
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
            writeMarshalUnmarshalCode(out, member->type(), p, "self." + fixIdent(member->name()), false);
        }
    }
    for (const auto& member : optionalMembers)
    {
        writeMarshalUnmarshalCode(out, member->type(), p, "self." + fixIdent(member->name()), false, member->tag());
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
    out << nl << "ostr.startSlice(typeId: " << fixIdent(name) << ".ice_staticId(), compactId: " << p->compactId()
        << ", last: " << (!base ? "true" : "false") << ")";
    for (const auto& member : members)
    {
        TypePtr type = member->type();
        if (!member->optional())
        {
            writeMarshalUnmarshalCode(out, member->type(), p, "self." + fixIdent(member->name()), true);
        }
    }
    for (const auto& member : optionalMembers)
    {
        writeMarshalUnmarshalCode(out, member->type(), p, "self." + fixIdent(member->name()), true, member->tag());
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
Gen::ValueVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    out << eb;
}

Gen::ObjectVisitor::ObjectVisitor(::IceInternal::Output& o) : out(o) {}

bool
Gen::ObjectVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    const string swiftModule = getSwiftModule(getTopLevelModule(dynamic_pointer_cast<Contained>(p)));
    const string disp = fixIdent(getRelativeTypeString(p, swiftModule) + "Disp");
    const string traits = fixIdent(getRelativeTypeString(p, swiftModule) + "Traits");
    const string servant = fixIdent(getRelativeTypeString(p, swiftModule));

    //
    // Disp struct
    //
    out << sp;
    out << sp;
    out << nl << "/// Dispatcher for '" << servant << "` servants.";
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

    StringList allOpNames;
    transform(
        allOps.begin(),
        allOps.end(),
        back_inserter(allOpNames),
        [](const ContainedPtr& it) { return it->name(); });

    allOpNames.push_back("ice_id");
    allOpNames.push_back("ice_ids");
    allOpNames.push_back("ice_isA");
    allOpNames.push_back("ice_ping");

    out << sp;
    out << nl;
    out << "public func dispatch(_ request: Ice.IncomingRequest) async throws -> Ice.OutgoingResponse";
    out << sb;
    out << nl;
    out << "switch request.current.operation";
    out << sb;
    out.dec(); // to align case with switch
    for (const auto& opName : allOpNames)
    {
        out << nl << "case \"" << opName << "\":";
        out.inc();
        if (opName == "ice_id" || opName == "ice_ids" || opName == "ice_isA" || opName == "ice_ping")
        {
            out << nl << "try await (servant as? Ice.Object ?? " << disp << ".defaultObject)._iceD_" << opName
                << "(request)";
        }
        else
        {
            out << nl << "try await servant._iceD_" << opName << "(request)";
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
    for (InterfaceList::const_iterator i = bases.begin(); i != bases.end(); ++i)
    {
        baseNames.push_back(fixIdent(getRelativeTypeString(*i, swiftModule)));
    }

    // Check for 'swift:inherits' metadata.
    for (const auto& metadata : p->getMetadata())
    {
        if (metadata->directive() == "swift:inherits")
        {
            baseNames.push_back(metadata->arguments());
        }
    }

    out << sp;
    writeDocSummary(out, p);
    out << nl << "public protocol " << servant;
    if (!baseNames.empty())
    {
        out << ":";
    }

    for (StringList::const_iterator i = baseNames.begin(); i != baseNames.end();)
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
Gen::ObjectVisitor::visitInterfaceDefEnd(const InterfaceDefPtr&)
{
    out << eb;
}

void
Gen::ObjectVisitor::visitOperation(const OperationPtr& op)
{
    const string swiftModule = getSwiftModule(getTopLevelModule(dynamic_pointer_cast<Contained>(op)));
    const string opName = fixIdent(op->name());
    const ParamInfoList allInParams = getAllInParams(op);
    const ParamInfoList allOutParams = getAllOutParams(op);

    out << sp;
    writeOpDocSummary(out, op, true);
    out << nl << "func " << opName;
    out << spar;
    for (ParamInfoList::const_iterator q = allInParams.begin(); q != allInParams.end(); ++q)
    {
        ostringstream s;
        s << q->name << ": " << q->typeStr;
        out << s.str();
    }
    out << ("current: " + getUnqualified("Ice.Current", swiftModule));
    out << epar;

    out << " async throws";
    if (allOutParams.size() > 0)
    {
        out << " -> " << operationReturnType(op);
    }
}

Gen::ObjectExtVisitor::ObjectExtVisitor(::IceInternal::Output& o) : out(o) {}

bool
Gen::ObjectExtVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    const string swiftModule = getSwiftModule(getTopLevelModule(dynamic_pointer_cast<Contained>(p)));
    const string name = getRelativeTypeString(p, swiftModule);

    out << sp;
    writeServantDocSummary(out, p, swiftModule);
    out << nl << "extension " << fixIdent(name);

    out << sb;
    return true;
}

void
Gen::ObjectExtVisitor::visitInterfaceDefEnd(const InterfaceDefPtr&)
{
    out << eb;
}

void
Gen::ObjectExtVisitor::visitOperation(const OperationPtr& op)
{
    writeDispatchOperation(out, op);
}
