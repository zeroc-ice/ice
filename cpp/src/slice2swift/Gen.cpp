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
#include <Slice/Parser.h>
#include <Slice/FileTracker.h>
#include <Slice/Util.h>

#include "Gen.h"

using namespace std;
using namespace Slice;
using namespace IceUtilInternal;

Gen::Gen(const string& base, const vector<string>& includePaths, const string& dir) :
    _out(false), // No break before opening block in Swift
    _includePaths(includePaths)
{
    _fileBase = base;
    string::size_type pos = base.find_last_of("/\\");
    if(pos != string::npos)
    {
        _fileBase = base.substr(pos + 1);
    }

    string file = _fileBase + ".swift";

    if(!dir.empty())
    {
        file = dir + '/' + file;
    }

    _out.open(file.c_str());
    if(!_out)
    {
        ostringstream os;
        os << "cannot open `" << file << "': " << IceUtilInternal::errorToString(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }
    FileTracker::instance()->addFile(file);

    printHeader();
    printGeneratedHeader(_out, _fileBase + ".ice");

    _out << nl << "import Foundation";
}

Gen::~Gen()
{
    if(_out.isOpen())
    {
        _out << nl;
    }
}

void
Gen::generate(const UnitPtr& p)
{
    SwiftGenerator::validateMetaData(p);

    ImportVisitor importVisitor(_out);
    p->visit(&importVisitor, false);
    importVisitor.writeImports();

    TypesVisitor typesVisitor(_out);
    p->visit(&typesVisitor, false);

    ProxyVisitor proxyVisitor(_out);
    p->visit(&proxyVisitor, false);

    ValueVisitor valueVisitor(_out);
    p->visit(&valueVisitor, false);

    LocalObjectVisitor localObjectVisitor(_out);
    p->visit(&localObjectVisitor, false);
}

void
Gen::closeOutput()
{
    _out.close();
}

void
Gen::printHeader()
{
    static const char* header =
"// **********************************************************************\n"
"//\n"
"// Copyright (c) ZeroC, Inc. All rights reserved.\n"
"//\n"
"// **********************************************************************\n"
        ;

    _out << header;
    _out << "//\n";
    _out << "// Ice version " << ICE_STRING_VERSION << "\n";
    _out << "//\n";
}

Gen::ImportVisitor::ImportVisitor(IceUtilInternal::Output& o) : out(o)
{
}

bool
Gen::ImportVisitor::visitModuleStart(const ModulePtr& p)
{
    //
    // Always import Ice module first if not building Ice
    //
    if(UnitPtr::dynamicCast(p->container()) && _imports.empty())
    {
        string swiftModule = getSwiftModule(p);
        if(swiftModule != "Ice")
        {
            _imports.push_back("Ice");
        }
    }
    return true;
}

bool
Gen::ImportVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    //
    // Add imports required for base classes
    //
    ClassList bases = p->bases();
    for(ClassList::const_iterator i = bases.begin(); i != bases.end(); ++i)
    {
        addImport(ContainedPtr::dynamicCast(*i), p);
    }

    //
    // Add imports required for data members
    //
    const DataMemberList allDataMembers = p->allDataMembers();
    for(DataMemberList::const_iterator i = allDataMembers.begin(); i != allDataMembers.end(); ++i)
    {
        addImport((*i)->type(), p);
    }

    //
    // Add imports required for operation parameters and return type
    //
    const OperationList operationList = p->allOperations();
    for(OperationList::const_iterator i = operationList.begin(); i != operationList.end(); ++i)
    {
        const TypePtr ret = (*i)->returnType();
        if(ret && ret->definitionContext())
        {
            addImport(ret, p);
        }

        const ParamDeclList paramList = (*i)->parameters();
        for(ParamDeclList::const_iterator j = paramList.begin(); j != paramList.end(); ++j)
        {
            addImport((*j)->type(), p);
        }
    }

    if(!p->isLocal() && p->isInterface())
    {
        const string promiseKit = "PromiseKit";
        if(find(_imports.begin(), _imports.end(), promiseKit) == _imports.end())
        {
            _imports.push_back(promiseKit);
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
    for(DataMemberList::const_iterator i = dataMembers.begin(); i != dataMembers.end(); ++i)
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
    if(base)
    {
        addImport(ContainedPtr::dynamicCast(base), p);
    }

    //
    // Add imports required for data members
    //
    const DataMemberList allDataMembers = p->allDataMembers();
    for(DataMemberList::const_iterator i = allDataMembers.begin(); i != allDataMembers.end(); ++i)
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
    for(vector<string>::const_iterator i = _imports.begin(); i != _imports.end(); ++i)
    {
        out << nl << "import " << *i;
    }
}

void
Gen::ImportVisitor::addImport(const TypePtr& definition, const ContainedPtr& toplevel)
{
    if(!BuiltinPtr::dynamicCast(definition))
    {
        ModulePtr m1 = getTopLevelModule(definition);
        ModulePtr m2 = getTopLevelModule(toplevel);

        string swiftM1 = getSwiftModule(m1);
        string swiftM2 = getSwiftModule(m2);
        if(swiftM1 != swiftM2 && find(_imports.begin(), _imports.end(), swiftM1) == _imports.end())
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
    if(swiftM1 != swiftM2 && find(_imports.begin(), _imports.end(), swiftM1) == _imports.end())
    {
        _imports.push_back(swiftM1);
    }
}

Gen::TypesVisitor::TypesVisitor(IceUtilInternal::Output& o) : out(o)
{
}

bool
Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr&)
{
    return false;
}

bool
Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    const string swiftModule = getSwiftModule(getTopLevelModule(ContainedPtr::dynamicCast(p)));
    const string name = getUnqualified(getAbsolute(p), swiftModule);

    ExceptionPtr base = p->base();

    if(!p->isLocal())
    {
        //
        // For each UserException class we generate a extension in ClassResolver
        //
        ostringstream factory;
        StringList parts = splitScopedName(p->scoped());
        for(StringList::const_iterator it = parts.begin(); it != parts.end();)
        {
            factory << (*it);
            if(++it != parts.end())
            {
                factory << "_";
            }
        }

        out << sp;
        out << nl << "public class " << name << "_TypeResolver: Ice.UserExceptionTypeResolver";
        out << sb;
        out << nl << "public override func type() -> Ice.UserException.Type";
        out << sb;
        out << nl << "return " << name << ".self";
        out << eb;
        out << eb;

        out << sp;
        out << nl << "public extension Ice.ClassResolver";
        out << sb;
        out << nl << "@objc public static func " << factory.str() << "() -> Ice.UserExceptionTypeResolver";
        out << sb;
        out << nl << "return " << name << "_TypeResolver()";
        out << eb;
        out << eb;
    }

    out << sp;
    out << nl << "public class " << name << ": ";
    if(base)
    {
        out << getUnqualified(getAbsolute(base), swiftModule);
    }
    else if(p->isLocal())
    {
        out << getUnqualified("Ice.LocalException", swiftModule);
    }
    else
    {
        out << getUnqualified("Ice.UserException", swiftModule);
    }
    out << sb;

    const DataMemberList members = p->dataMembers();
    const DataMemberList allMembers = p->allDataMembers();
    const DataMemberList baseMembers = base ? base->allDataMembers() : DataMemberList();

    StringPairList extraParams;
    if(p->isLocal())
    {
        extraParams.push_back(make_pair("file", "String = #file"));
        extraParams.push_back(make_pair("line", "Int = #line"));
    }

    bool rootClass = !base && !p->isLocal();
    // bool required =
    writeMembers(out, members, p);

    // Local exceptions do not need default initializers
    if(!p->isLocal())
    {
        writeDefaultInitializer(out, members, p, rootClass, true);
    }
    writeMemberwiseInitializer(out, members, baseMembers, allMembers, p, rootClass, extraParams);

    if(!p->isLocal())
    {
        out << sp;
        out << nl;
        if(base)
        {
            out << "override ";
        }
        out << "public func _iceWriteImpl(to ostr: " << getUnqualified("Ice.OutputStream", swiftModule) << ")";
        out << sb;
        out << nl << "ostr.startSlice(typeId: " << name << ".ice_staticId(), compactId: -1, last: "
            << (!base ? "true" : "false") << ")";
        for(DataMemberList::const_iterator i = members.begin(); i != members.end(); ++i)
        {
            writeMarshalUnmarshalCode(out, *i, p, false, false, true);
        }
        out << nl << "ostr.endSlice()";
        if(base)
        {
            out << nl << "super._iceWriteImpl(to: ostr);";
        }
        out << eb;

        out << sp;
        out << nl;
        if(base)
        {
            out << "override ";
        }
        out << "public func _iceReadImpl(from istr: " << getUnqualified("Ice.InputStream", swiftModule) << ") throws";
        out << sb;
        out << nl << "try istr.startSlice()";
        for(DataMemberList::const_iterator i = members.begin(); i != members.end(); ++i)
        {
            writeMarshalUnmarshalCode(out, *i, p, false, false, false);
        }
        out << nl << "try istr.endSlice()";
        if(base)
        {
            out << nl << "try super._iceReadImpl(from: istr);";
        }
        out << eb;

        if(p->usesClasses(false) && (!base || (base && !base->usesClasses(false))))
        {
            out << sp;
            out << nl;
            out << "public func _usesClasses() -> Bool";
            out << sb;
            out << nl << "return true";
            out << eb;
        }

        out << sp;
        out << nl;
        if(base)
        {
            out << "override ";
        }
        out << "public func ice_id() -> Swift.String";
        out << sb;
        out << nl << "return \"" << p->scoped() << "\"";
        out << eb;

        out << sp;
        out << nl;
        if(base)
        {
            out << "override ";
        }
        out << "public class func ice_staticId() -> Swift.String";
        out << sb;
        out << nl << "return \"" << p->scoped() << "\"";
        out << eb;
    }

    out << eb;
    return false;
}

bool
Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    const string swiftModule = getSwiftModule(getTopLevelModule(ContainedPtr::dynamicCast(p)));
    const string name = getUnqualified(getAbsolute(p), swiftModule);
    bool containsSequence;
    bool legalKeyType = Dictionary::legalKeyType(p, containsSequence);
    const DataMemberList members = p->dataMembers();

    out << sp;
    out << nl << "public struct " << name;
    if(legalKeyType)
    {
        out << ": Swift.Hashable";
    }
    out << sb;

    writeMembers(out, members, p);
    writeDefaultInitializer(out, members, p);
    writeMemberwiseInitializer(out, members, p);

    out << eb << nl;

    if(!p->isLocal())
    {
        out << nl << "public extension " << getUnqualified("Ice.InputStream", swiftModule);
        out << sb;
        out << nl << "func read() throws -> " << name;
        out << sb;
        for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
        {
            writeMarshalUnmarshalCode(out, *q, p, true, true, false);
        }

        out << nl << "return " << name;
        out << spar;
        for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
        {
            out << ((*q)->name() + ": " + (*q)->name());
        }
        out << epar;
        out << eb;

        out << eb;

        out << sp;
        out << nl << "public extension " << getUnqualified("Ice.OutputStream", swiftModule);
        out << sb;

        out << nl << "func write(_ v: " << name << ")" << sb;
        for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
        {
            writeMarshalUnmarshalCode(out, *q, p, true, false, true);
        }
        out << eb;

        out << eb;
    }

    return false;
}

void
Gen::TypesVisitor::visitSequence(const SequencePtr& p)
{
    const string swiftModule = getSwiftModule(getTopLevelModule(ContainedPtr::dynamicCast(p)));
    const string name = getUnqualified(getAbsolute(p), swiftModule);

    const TypePtr type = p->type();

    out << sp;
    out << nl << "public typealias " << name << " = [" << typeToString(p->type(), p) << "]";

    if(p->isLocal())
    {
        return;
    }

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p->type());
    if(builtin && builtin->kind() <= Builtin::KindString)
    {
        return; // No helpers for sequence of primitive types
    }

    const string ostr = getUnqualified("Ice.OutputStream", swiftModule);
    const string istr = getUnqualified("Ice.InputStream", swiftModule);

    const string optionalFormat = getUnqualified(getOptionalFormat(p->type()), swiftModule);

    out << sp;
    out << nl << "public struct " << name << "Helper";
    out << sb;

    out << nl << "public static func read(from istr: " << istr << ") throws -> " << name;
    out << sb;
    out << nl << "let sz = try istr.readAndCheckSeqSize(minSize: " << p->type()->minWireSize() << ")";
    out << nl << "var v = " << name << "()";
    out << nl << "v.reserveCapacity(sz)";
    out << nl << "for _ in 0 ..< sz";
    out << sb;
    writeMarshalUnmarshalCode(out, type, typeToString(p->type(), p), "j", false, true, false);
    out << nl << "v.append(j)";
    out << eb;
    out << nl << "return v";
    out << eb;

    out << nl << "public static func read(from istr: " << istr << ", tag: Int32) throws -> " << name << "?";
    out << sb;
    out << nl << "guard try istr.readOptional(tag: tag, expectedFormat: " << optionalFormat << ") else";
    out << sb;
    out << nl << "return nil";
    out << eb;
    if(p->type()->isVariableLength())
    {
        out << nl << "try istr.skip(4)";
    }
    else if(p->type()->minWireSize() > 1)
    {
        out << nl << "try istr.skipSize()";
    }
    out << nl << "return try read(from: istr)";
    out << eb;

    out << sp;
    out << nl << "public static func write(to ostr: " << ostr << ", value v: " << name << ")";
    out << sb;
    out << nl << "ostr.write(size: v.count)";
    out << nl << "v.forEach";
    out << sb;
    writeMarshalUnmarshalCode(out, type, typeToString(p->type(), p), "$0", false, false, true);
    out << eb;
    out << eb;

    out << sp;
    out << nl << "public static func write(to ostr: " << ostr << ",  tag: Int32, value v: "<< name << "?)";
    out << sb;
    out << nl << "guard let val = v else";
    out << sb;
    out << nl << "return";
    out << eb;
    if(p->type()->isVariableLength())
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
        out << nl << "if ostr.writeOptionalVSize(tag: tag, len: val.count, elemSize: " << p->type()->minWireSize() << ")";
        out << sb;
        out << nl << "ostr.write(size: val.count)";
        out << nl << "write(to: ostr, value: val)";
        out << eb;
    }
    out << eb;

    out << eb;
}

void
Gen::TypesVisitor::visitDictionary(const DictionaryPtr& p)
{
    const string swiftModule = getSwiftModule(getTopLevelModule(ContainedPtr::dynamicCast(p)));
    const string name = getUnqualified(getAbsolute(p), swiftModule);

    out << sp;
    out << nl << "public typealias " << name << " = ["
        << typeToString(p->keyType(), p) << ": "
        << typeToString(p->valueType(), p)
        << "]";

    if(p->isLocal())
    {
        return;
    }

    const string ostr = getUnqualified("Ice.OutputStream", swiftModule);
    const string istr = getUnqualified("Ice.InputStream", swiftModule);

    const string optionalFormat = getUnqualified(getOptionalFormat(p), swiftModule);
    const bool isVariableLength = p->keyType()->isVariableLength() || p->valueType()->isVariableLength();
    const int minWireSize = p->keyType()->minWireSize() + p->valueType()->minWireSize();

    out << sp;
    out << nl << "public struct " << name << "Helper";
    out << sb;

    out << nl << "public static func read(from istr: " << istr << ") throws -> " << name;
    out << sb;
    out << nl << "let sz = try Int(istr.readSize())";
    out << nl << "var v = " << name << "()";
    out << nl << "for _ in 0 ..< sz";
    out << sb;
    writeMarshalUnmarshalCode(out, p->keyType(), typeToString(p->keyType(), p), "key", false, true, false);
    writeMarshalUnmarshalCode(out, p->valueType(), typeToString(p->valueType(), p), "value", false, true, false);
    out << nl << "v[key] = value";
    out << eb;
    out << nl << "return v";
    out << eb;

    out << nl << "public static func read(from istr: " << istr << ", tag: Int32) throws -> " << name << "?";
    out << sb;
    out << nl << "guard try istr.readOptional(tag: tag, expectedFormat: " << optionalFormat << ") else";
    out << sb;
    out << nl << "return nil";
    out << eb;
    if(p->keyType()->isVariableLength() || p->valueType()->isVariableLength())
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
    out << nl << "public static func write(to ostr: " << ostr << ", value v: " << name << ")";
    out << sb;
    out << nl << "ostr.write(size: v.count)";
    out << nl << "v.forEach";
    out << sb;
    out << "key, value in";
    writeMarshalUnmarshalCode(out, p->keyType(), typeToString(p->keyType(), p), "key", false, false, true);
    writeMarshalUnmarshalCode(out, p->valueType(), typeToString(p->valueType(), p), "value", false, false, true);
    out << eb;
    out << eb;

    out << sp;
    out << nl << "public static func write(to ostr: " << ostr << ", tag: Int32, value v: "<< name << "?)";
    out << sb;
    out << nl << "guard let val = v else";
    out << sb;
    out << nl << "return";
    out << eb;
    if(isVariableLength)
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
        out << nl << "ostr.write(size: val.count)";
        out << nl << "write(to: ostr, value: val)";
        out << eb;
    }
    out << eb;

    out << eb;
}

void
Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    const string swiftModule = getSwiftModule(getTopLevelModule(ContainedPtr::dynamicCast(p)));
    const string name = getUnqualified(getAbsolute(p), swiftModule);
    const EnumeratorList enumerators = p->enumerators();
    const string enumType = p->maxValue() <= 0xFF ? "UInt8" : "Int32";

    out << sp;
    out << nl << "public enum " << name << ": " << enumType;
    out << sb;

    for(EnumeratorList::const_iterator en = enumerators.begin(); en != enumerators.end(); ++en)
    {
        out << nl << "case " << fixIdent((*en)->name()) << " = " << (*en)->value();
    }

    out << nl << "public init()";
    out << sb;
    out << nl << "self = ." << fixIdent((*enumerators.begin())->name());
    out << eb;

    out << eb;

    out << sp;
    out << nl << "public extension " << getUnqualified("Ice.InputStream", swiftModule);
    out << sb;
    out << nl << "func read() throws -> " << name;
    out << sb;
    out << nl << "var rawValue = " << enumType << "()";
    out << nl << "try read(enum: &rawValue, maxValue: " << p->maxValue() << ")";
    out << nl << "guard let val = " << name << "(rawValue: rawValue) else";
    out << sb;
    out << nl << "throw MarshalException(reason: \"invalid enum value\")";
    out << eb;
    out << nl << "return val";
    out << eb;

    out << eb;

    out << sp;
    out << nl << "public extension " << getUnqualified("Ice.OutputStream", swiftModule);
    out << sb;
    out << nl << "func write(_ v: " << name << ")";
    out << sb;
    out << nl << "write(enum: v.rawValue, maxValue: " << p->maxValue() << ")";
    out << eb;

    out << eb;
}

void
Gen::TypesVisitor::visitConst(const ConstPtr& p)
{
    const string name = fixIdent(p->name());
    const TypePtr type = p->type();
    const string swiftModule = getSwiftModule(getTopLevelModule(ContainedPtr::dynamicCast(p)));

    out << nl << "public let " << name << ": " << typeToString(type, p) << " = ";
    writeConstantValue(out, type, p->valueType(), p->value(), p->getMetaData(), swiftModule);
    out << nl;
}

Gen::ProxyVisitor::ProxyVisitor(::IceUtilInternal::Output& o) : out(o)
{
}

bool
Gen::ProxyVisitor::visitModuleStart(const ModulePtr& p)
{
    return p->hasNonLocalClassDefs();
}

void
Gen::ProxyVisitor::visitModuleEnd(const ModulePtr&)
{
}

bool
Gen::ProxyVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal() || (!p->isInterface() && p->allOperations().empty()))
    {
        return false;
    }

    ClassList bases = p->bases();
    ClassDefPtr baseClass;
    if(!bases.empty() && !bases.front()->isInterface())
    {
        baseClass = bases.front();
    }

    const string swiftModule = getSwiftModule(getTopLevelModule(ContainedPtr::dynamicCast(p)));
    const string name = getUnqualified(getAbsolute(p), swiftModule);
    const string prx = name + "Prx";
    const string prxI = "_" + name + "PrxI";

    out << sp;
    out << nl << "public protocol " << prx << ": "
        << (baseClass ? fixIdent(baseClass->name()) : getUnqualified("Ice.ObjectPrx", swiftModule));
    out << sb;
    out << eb;

    out << sp;
    out << nl << "public class " << prxI << ": " << getUnqualified("Ice._ObjectPrxI", swiftModule) << ", " << prx;
    out << sb;

    out << nl << "override public class func ice_staticId() -> String";
    out << sb;
    out << nl << "return \"" << p->scoped() << "\"";
    out << eb;

    out << eb;

    //
    // checkedCast
    //
    out << sp;
    out << nl << "public func checkedCast" << spar
        << ("prx: " + getUnqualified("Ice.ObjectPrx", swiftModule))
        << ("type: " + prx + ".Protocol")
        << ("facet: String? = nil")
        << ("context: Context? = nil") << epar << " throws -> " << prx << "?";
    out << sb;
    out << nl << "return try " << prxI << ".checkedCast(prx: prx, facet: facet, context: context) as " << prxI << "?";
    out << eb;

    //
    // uncheckedCast
    //
    out << sp;
    out << nl << "public func uncheckedCast" << spar
        << ("prx: " + getUnqualified("Ice.ObjectPrx", swiftModule))
        << ("type: " + prx + ".Protocol")
        << ("facet: String? = nil")
        << ("context: Context? = nil") << epar << " -> " << prx << "?";
    out << sb;
    out << nl << "return " << prxI << ".uncheckedCast(prx: prx, facet: facet, context: context) as " << prxI << "?";
    out << eb;

    //
    // InputStream extension
    //
    out << sp;
    out << nl << "public extension " << getUnqualified("Ice.InputStream", swiftModule);
    out << sb;

    out << nl << "func read() throws -> " << prx << "?";
    out << sb;
    out << nl << "return try read() as " << prxI << "?";
    out << eb;

    out << sp;
    out << nl << "func read(tag: Int32) throws -> " << prx << "?";
    out << sb;
    out << nl << "return try read(tag: tag) as " << prxI << "?";
    out << eb;

    out << eb;

    out << sp;
    out << nl << "public extension " << prx;
    out << sb;

    return true;
}

void
Gen::ProxyVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    out << eb;
}

void
Gen::ProxyVisitor::visitOperation(const OperationPtr& op)
{
    writeProxyOperation(out, op, false);
    writeProxyOperation(out, op, true);
}

Gen::ValueVisitor::ValueVisitor(::IceUtilInternal::Output& o) : out(o)
{
}

bool
Gen::ValueVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal() || p->isInterface())
    {
        return false;
    }

    const string swiftModule = getSwiftModule(getTopLevelModule(ContainedPtr::dynamicCast(p)));
    const string name = getUnqualified(getAbsolute(p), swiftModule);

    ClassList bases = p->bases();
    ClassDefPtr base;
    if(!bases.empty() && !bases.front()->isInterface())
    {
        base = bases.front();
    }

    //
    // For each Value class we generate a extension in ClassResolver
    //
    ostringstream factory;
    StringList parts = splitScopedName(p->scoped());
    for(StringList::const_iterator it = parts.begin(); it != parts.end();)
    {
        factory << (*it);
        if(++it != parts.end())
        {
            factory << "_";
        }
    }

    out << sp;
    out << nl << "public class " << name << "_TypeResolver: Ice.ValueTypeResolver";
    out << sb;
    out << nl << "public override func type() -> Ice.Value.Type";
    out << sb;
    out << nl << "return " << name << ".self";
    out << eb;
    out << eb;

    out << sp;
    out << nl << "public extension Ice.ClassResolver";
    out << sb;
    out << nl << "@objc public static func " << factory.str() << "() -> Ice.ValueTypeResolver";
    out << sb;
    out << nl << "return " << name << "_TypeResolver()";
    out << eb;
    out << eb;

    out << sp;
    out << nl << "public class " << name << ": ";
    if(base)
    {
        out << getUnqualified(getAbsolute(base), swiftModule);
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

    // TODO:
    //const bool basePreserved = p->inheritsMetaData("preserve-slice");
    //const bool preserved = p->hasMetaData("preserve-slice");

    writeMembers(out, members, p);
    writeDefaultInitializer(out, members, p, !base, true);
    writeMemberwiseInitializer(out, members, baseMembers, allMembers, p, base == 0);

    out << sp;
    out << nl;
    if(base)
    {
        out << "override ";
    }
    out << "public class func ice_staticId() -> Swift.String";
    out << sb;
    out << nl << "return \"" << p->scoped() << "\"";
    out << eb;

    out << sp;
    out << nl;
    out << nl;
    if(base)
    {
        out << "override ";
    }
    out << "public func _iceReadImpl(from istr: " << getUnqualified("Ice.InputStream", swiftModule) << ") throws";
    out << sb;
    out << nl << "try istr.startSlice()";
    for(DataMemberList::const_iterator i = members.begin(); i != members.end(); ++i)
    {
        DataMemberPtr member = *i;
        TypePtr type = member->type();
        if(!member->optional())
        {
            writeMarshalUnmarshalCode(out, member, p, false, false, false);
        }
    }
    for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
    {
        writeMarshalUnmarshalCode(out, *d, p, false, false, false, (*d)->tag());
    }
    out << nl << "try istr.endSlice()";
    if(base)
    {
        out << nl << "try super._iceReadImpl(from: istr);";
    }
    out << eb;

    out << sp;
    out << nl;
    if(base)
    {
        out << "override ";
    }
    out << "public func _iceWriteImpl(to ostr: " << getUnqualified("Ice.OutputStream", swiftModule) << ")";
    out << sb;
    out << nl << "ostr.startSlice(typeId: " << name << ".ice_staticId(), compactId:" << p->compactId() << ", last: "
        << (!base ? "true" : "false") << ")";
    for(DataMemberList::const_iterator i = members.begin(); i != members.end(); ++i)
    {
        DataMemberPtr member = *i;
        TypePtr type = member->type();
        if(!member->optional())
        {
            writeMarshalUnmarshalCode(out, member, p, false, false, true);
        }
    }
    for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
    {
        writeMarshalUnmarshalCode(out, *d, p, false, false, true, (*d)->tag());
    }
    out << nl << "ostr.endSlice()";
    if(base)
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

void
Gen::ValueVisitor::visitOperation(const OperationPtr&)
{
}

Gen::ObjectVisitor::ObjectVisitor(::IceUtilInternal::Output& o) : out(o)
{
}

bool
Gen::ObjectVisitor::visitModuleStart(const ModulePtr&)
{
    return true;
}

void
Gen::ObjectVisitor::visitModuleEnd(const ModulePtr&)
{
}

bool
Gen::ObjectVisitor::visitClassDefStart(const ClassDefPtr&)
{
    out << sb;
    out << eb;
    return true;
}

void
Gen::ObjectVisitor::visitClassDefEnd(const ClassDefPtr&)
{
}

void
Gen::ObjectVisitor::visitOperation(const OperationPtr&)
{
}

Gen::LocalObjectVisitor::LocalObjectVisitor(::IceUtilInternal::Output& o) : out(o)
{
}

bool
Gen::LocalObjectVisitor::visitModuleStart(const ModulePtr&)
{
    return true;
}

void
Gen::LocalObjectVisitor::visitModuleEnd(const ModulePtr&)
{
}

bool
Gen::LocalObjectVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(!p->isLocal())
    {
        return false;
    }

    const string swiftModule = getSwiftModule(getTopLevelModule(ContainedPtr::dynamicCast(p)));
    const string name = getUnqualified(getAbsolute(p), swiftModule);

    if(p->isDelegate())
    {
        OperationPtr op = p->allOperations().front();
        const ParamDeclList params = op->parameters();

        out << sp;
        out << nl << "public typealias " << name << " = ";
        out << spar;
        for(ParamDeclList::const_iterator i = params.begin(); i != params.end(); ++i)
        {
            ParamDeclPtr param = *i;
            if(!param->isOutParam())
            {
                TypePtr type = param->type();
                ostringstream s;
                s << typeToString(type, p, param->getMetaData(), param->optional());
                out << s.str();
            }
        }
        out << epar;
        if(!op->hasMetaData("swift:noexcept"))
        {
            out << " throws";
        }
        out << " -> ";

        TypePtr ret = op->returnType();
        ParamDeclList outParams = op->outParameters();

        if(ret || !outParams.empty())
        {
            if(outParams.empty())
            {
                out << typeToString(ret, op, op->getMetaData(), op->returnIsOptional());
            }
            else if(!ret && outParams.size() == 1)
            {
                ParamDeclPtr param = outParams.front();
                out << typeToString(param->type(), op, param->getMetaData(), param->optional());
            }
            else
            {
                string returnValueS = "returnValue";
                for(ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end(); ++i)
                {
                    ParamDeclPtr param = *i;
                    if(param->name() == "returnValue")
                    {
                        returnValueS = "_returnValue";
                        break;
                    }
                }

                out << spar;
                out << (returnValueS + ": " + typeToString(ret, op, op->getMetaData(), op->returnIsOptional()));
                for(ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end(); ++i)
                {
                    ParamDeclPtr param = *i;
                    out << (fixIdent(param->name()) + ": " +
                            typeToString(param->type(), op, op->getMetaData(), param->optional()));
                }
                out << epar;
            }
        }
        else
        {
            out << "Void";
        }
        return false;
    }

    ClassList bases = p->bases();
    ClassDefPtr base;
    if(!bases.empty() && !bases.front()->isInterface())
    {
        base = bases.front();
    }

    const DataMemberList members = p->dataMembers();
    const DataMemberList baseMembers = base ? base->allDataMembers() : DataMemberList();
    const DataMemberList allMembers = p->allDataMembers();

    //
    // Interfaces and local class with operations map to a protocol
    //
    bool protocol = p->isInterface() || !p->allOperations().empty();
    if(protocol)
    {
        out << sp;
        out << nl << "public protocol " << name << ": "
            << (base ? getUnqualified(getAbsolute(base), swiftModule) : "Swift.AnyObject");
        out << sb;
        writeMembers(out, members, p, TypeContextProtocol);
    }
    else
    {
        out << sp;
        out << nl << "public class " << name;
        if(base)
        {
            out << ": " << getUnqualified(getAbsolute(base), swiftModule);
        }
        out << sb;
        writeMembers(out, members, p);
        writeMemberwiseInitializer(out, members, baseMembers, allMembers, p, base == 0);
    }
    return true;
}

void
Gen::LocalObjectVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    out << eb;
}

void
Gen::LocalObjectVisitor::visitOperation(const OperationPtr& p)
{
    const string name = fixIdent(p->name());
    ParamDeclList params = p->parameters();
    ParamDeclList inParams = p->inParameters();

    int typeCtx = TypeContextInParam | TypeContextLocal;

    out << sp;
    out << nl << "func " << name;
    out << spar;
    for(ParamDeclList::const_iterator i = inParams.begin(); i != inParams.end(); ++i)
    {
        ParamDeclPtr param = *i;
        TypePtr type = param->type();
        ostringstream s;
        if(inParams.size() == 1)
        {
            s << "_ ";
        }
        s << fixIdent(param->name()) << ": "
          << typeToString(type, p, param->getMetaData(), param->optional(), typeCtx);
        out << s.str();
    }
    out << epar;

    if(!p->hasMetaData("swift:noexcept"))
    {
        out << " throws";
    }

    TypePtr ret = p->returnType();
    ParamDeclList outParams = p->outParameters();

    if(ret || !outParams.empty())
    {
        out << " -> ";
        if(outParams.empty())
        {
            out << typeToString(ret, p, p->getMetaData(), p->returnIsOptional(), typeCtx);
        }
        else if(!ret && outParams.size() == 1)
        {
            ParamDeclPtr param = outParams.front();
            out << typeToString(param->type(), p, param->getMetaData(), param->optional(), typeCtx);
        }
        else
        {
            string returnValueS = "returnValue";
            for(ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end(); ++i)
            {
                ParamDeclPtr param = *i;
                if(param->name() == "returnValue")
                {
                    returnValueS = "_returnValue";
                    break;
                }
            }

            out << spar;
            out << (returnValueS + ": " + typeToString(ret, p, p->getMetaData(), p->returnIsOptional()));
            for(ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end(); ++i)
            {
                ParamDeclPtr param = *i;
                out << (fixIdent(param->name()) + ": " +
                        typeToString(param->type(), p, p->getMetaData(), param->optional(), typeCtx));
            }
            out << epar;
        }
    }
}
