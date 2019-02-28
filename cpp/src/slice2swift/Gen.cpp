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

    const DataMemberList dataMembers = p->dataMembers();
    const DataMemberList allMembers = p->allDataMembers();
    const DataMemberList baseMembers = base ? base->allDataMembers() : DataMemberList();

    writeMembers(out, dataMembers, p);
    writeMemberwiseInitializer(out, dataMembers, baseMembers, allMembers, p, !base && !p->isLocal());

    if(!p->isLocal())
    {
        out << sp;
        out << nl << "required public init(from ins: " << getUnqualified("Ice.InputStream", swiftModule) << ") throws";
        out << sb;
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            DataMemberPtr member = *q;
            StringList metadata = member->getMetaData();
            out << nl << "self." << member->name() << " = try "
                << getUnqualified(getAbsolute(member->type()), swiftModule) << "(from: ins)";
        }
        out << eb;

        out << sp;
        out << nl << "public func ice_write(to os: " << getUnqualified("Ice.OutputStream", swiftModule) << ")";
        out << sb;
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            out << nl << fixIdent((*q)->name()) << ".ice_write(to: os)";
        }
        out << eb;

        out << sp;
        out << nl << "public class func ice_staticId() -> Swift.String";
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
        out << nl << "extension " << name << ": Ice.Streamable" << sb;

        out << sp;
        out << nl << "public init(from ins: Ice.InputStream) throws" << sb;
        for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
        {
            DataMemberPtr member = *q;
            StringList metadata = member->getMetaData();
            TypePtr type = member->type();
            out << nl << "self." << member->name() << " = ";
            if(isProxyType(type))
            {
                out << "try ins.read(proxy: " << getUnqualified(getAbsolute(type), swiftModule) << ".self)";
            }
            else
            {
                out << "try " << typeToString(type, p, metadata, member->optional()) << "(from: ins)";
            }
        }
        out << eb;

        out << sp;
        out << nl << "public func ice_write(to os: Ice.OutputStream)" << sb;
        for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
        {
            out << nl << fixIdent((*q)->name()) << ".ice_write(to: os)";
        }
        out << eb;

        out << eb;
    }

    return false;
}

void
Gen::TypesVisitor::visitSequence(const SequencePtr& p)
{
    const string name = fixIdent(p->name());
    const TypePtr type = p->type();

    out << sp;
    out << nl << "public typealias " << name << " = [" << typeToString(type, p) << "]";
}

void
Gen::TypesVisitor::visitDictionary(const DictionaryPtr& p)
{
    const string name = fixIdent(p->name());

    const TypePtr keyType = p->keyType();
    const TypePtr valueType = p->valueType();

    out << sp;
    out << nl << "public typealias " << name << " = [" << typeToString(keyType, p) << ":" << typeToString(valueType, p)
        << "]";
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
    out << eb;

    out << sp;
    out << nl << "extension " << name << ": Ice.Streamable";
    out << sb;

    out << nl << "public init()";
    out << sb;
    out << nl << "self = ." << fixIdent((*enumerators.begin())->name());
    out << eb;

    out << sp;
    out << nl << "public init(from ins: Ice.InputStream) throws";
    out << sb;
    out << nl << "var rawValue = " << enumType << "()";
    out << nl << "try ins.read(enum: &rawValue, maxValue: " << p->maxValue() << ")";
    out << nl << "guard let val = " << name << "(rawValue: rawValue) else" << sb;
    out << nl << "throw MarshalException(reason: \"invalid enum value\")";
    out << eb;
    out << nl << "self = val";
    out << eb;

    out << sp;
    out << nl << "public func ice_write(to os: Ice.OutputStream)";
    out << sb;
    out << nl << "os.write(enum: self.rawValue, maxValue: " << p->maxValue() << ")";
    out << eb;

    out << eb;
}

void
Gen::TypesVisitor::visitConst(const ConstPtr& p)
{
    const string name = fixIdent(p->name());
    const TypePtr type = p->type();

    out << nl << "public let " << name << ": " << typeToString(type, p) << " = " << p->value();
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

    const string prx = p->name() + "Prx";
    const string prxI = "_" + p->name() + "PrxI";

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
    // ImputStream extension
    //
    out << sp;
    out << nl << "public extension " << getUnqualified("Ice.InputStream", swiftModule);
    out << sb;

    out << nl << "func read(proxy: " << prx << ".Protocol) throws -> " << prx << "?";
    out << sb;
    out << nl << "return try " << prxI << ".ice_read(from: self)";
    out << eb;

    out << sp;
    out << nl << "func read(proxyArray: " << prx << ".Protocol) throws -> [" << prx << "?]";
    out << sb;
    out << nl << "return try read(proxyArray:" << prxI << ".self)";
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
    const string opName = fixIdent(op->name());

    ParamDeclList paramList = op->parameters();
    ParamDeclList inParams = op->inParameters();
    ParamDeclList outParams = op->outParameters();

    ExceptionList throws = op->throws();
    throws.sort();
    throws.unique();

    out << sp;
    out << nl << "func " << opName;
    out << spar;
    out << epar;
    out << sb;
    out << eb;
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

      const bool basePreserved = p->inheritsMetaData("preserve-slice");
    const bool preserved = p->hasMetaData("preserve-slice");

    writeMembers(out, members, p);
    writeDefaultInitializer(out, members, p, true);
    writeMemberwiseInitializer(out, members, baseMembers, allMembers, p, base == 0);

    out << sp;
    out << nl << "public class func ice_staticId() -> Swift.String";
    out << sb;
    out << nl << "return \"" << p->scoped() << "\"";
    out << eb;

    out << sp;
    out << nl << "public func iceReadImpl(from: " << getUnqualified("Ice.InputStream", swiftModule) << ") throws";
    out << sb;
    out << eb;

    out << sp;
    out << nl << "public func iceWriteImpl(to: " << getUnqualified("Ice.OutputStream", swiftModule) << ")";
    out << sb;
    out << nl << "to.startSlice(ice_staticId(), " << p->compactId() << (!base ? ", true" : ", false") << ");";
    for(DataMemberList::const_iterator i = members.begin(); i != members.end(); ++i)
    {
        DataMemberPtr member = *i;
        if(!member->optional())
        {
            out << nl << fixIdent(member->name()) << ".ice_write(to: to)";
        }
    }
    for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
    {
        writeMarshalDataMember(*d, fixId(*d, DotNet::ICloneable, true), ns);
    }
    _out << nl << "to.endSlice();";
    if(base)
    {
        _out << nl << "super.iceWriteImpl(to: to);";
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

    out << sp;
    out << nl << "func " << name;
    out << spar;
    for(ParamDeclList::const_iterator i = params.begin(); i != params.end(); ++i)
    {
        ParamDeclPtr param = *i;
        if(!param->isOutParam())
        {
            TypePtr type = param->type();
            ostringstream s;
            s << fixIdent(param->name()) << ": "
              << typeToString(type, p, param->getMetaData(), param->optional(), TypeContextInParam);
            out << s.str();
        }
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
            out << typeToString(ret, p, p->getMetaData(), p->returnIsOptional());
        }
        else if(!ret && outParams.size() == 1)
        {
            ParamDeclPtr param = outParams.front();
            out << typeToString(param->type(), p, param->getMetaData(), param->optional());
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
                        typeToString(param->type(), p, p->getMetaData(), param->optional()));
            }
            out << epar;
        }
    }
}
