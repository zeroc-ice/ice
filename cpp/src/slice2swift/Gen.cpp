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

namespace
{

}

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

    _out << nl << "import Foundation" << nl;
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
"// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.\n"
"//\n"
"// This copy of Ice is licensed to you under the terms described in the\n"
"// ICE_LICENSE file included in this distribution.\n"
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
        out << "import " << *i << nl;
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

bool Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = p->name();
    string swiftName = fixName(p);
    ClassList bases = p->bases();
    ClassDefPtr baseClass;
    if(!bases.empty() && !bases.front()->isInterface())
    {
        baseClass = bases.front();
    }

    if(p->isDelegate())
    {
        const OperationPtr op = p->operations().front();
        const ParamDeclList parameters = op->parameters();
        const string ret = typeToString(op->returnType());
        out << nl << "public typealias " << swiftName << " = ";
        out << "(";
        for(ParamDeclList::const_iterator q = parameters.begin(); q != parameters.end() && !(*q)->isOutParam(); ++q)
        {
             if(q != parameters.begin())
            {
                out << ", ";
            }

            out << typeToString((*q)->type());
        }
        out << ") -> " << (ret.empty() ? "Void" : ret);

        return false;
    }

    if(p->isLocal())
    {
        //
        // If there are any operations generate a protocol
        // Otherwise if just datamembers then a class
        //
        if(p->allOperations().empty())
        {
            out << nl << "public class " << swiftName;
            if(baseClass)
            {
                out << ": " << fixName(baseClass);
            }
            out << sb;

            if(!p->dataMembers().empty())
            {
                writeDataMembers(out, p->dataMembers());
                writeInitializer(out, p->dataMembers(), p->allDataMembers());
            }
            out << eb << nl;
        }
        else
        {
            out << nl << "public protocol " << swiftName;

            // TODO check for delegate metadata and map to closure

            if(baseClass)
            {
                out << ": " << fixName(baseClass);
            }
            else
            {
                // All protocols must be implemented by a class
                out << ": AnyObject";
            }
            out << sb;

            if(!p->dataMembers().empty())
            {
                writeDataMembers(out, p->dataMembers(), true);
            }

            OperationList ops = p->operations();
            for(OperationList::iterator r = ops.begin(); r != ops.end(); ++r)
            {
                writeOperation(out, *r, p->isLocal());
            }
            out << eb << nl;
        }

    }
    else
    {
        //
        // interface: generate Swift protocol and protocolPrx
        // class: generate class protocolPrx, and protocolDisp
        //

        out << nl << "public protocol " << swiftName;
        if(!p->isInterface())
        {
            out << "Disp";
        }
        if(baseClass)
        {
            out << ": " << fixIdent(baseClass->name());
        }
        else
        {
            // All protocols must be implemented by a class
            out << ": class";
        }
        out << sb;
        // TODO
        out << eb << nl;

        if(!p->isInterface())
        {
            out << nl << "public class " << swiftName;
            if(baseClass)
            {
                out << ": " << fixIdent(baseClass->name());
            }
            out << sb;
            if(!p->dataMembers().empty())
            {
                writeDataMembers(out, p->dataMembers());
                writeInitializer(out, p->dataMembers(), p->allDataMembers());
            }
            out << eb << nl;
        }

        out << nl << "public protocol " << swiftName << "Prx";
        if(baseClass)
        {
            out << ": " << fixIdent(baseClass->name());
        }
        else
        {
            out << ": " << "Ice.ObjectPrx";
        }
        out << sb << eb << nl;

        // TODO add class ClassResolver extension
        // @objc dynamic Demo_ClassName() -> AnyObject

        if(p->isInterface())
        {
            const string proxyImpl = "_" + swiftName + "PrxI";
            out << nl << "public extension " << swiftName + "Prx";
            out << sb;
            OperationList ops = p->operations();
            for(OperationList::iterator r = ops.begin(); r != ops.end(); ++r)
            {
                const OperationPtr op = *r;
                writeOperation(out, op, p->isLocal());
                out << sb << nl;
                writeMarshalUnmarshalCode(out, p, op);
                out << eb << nl;
            }
            out << eb << nl;

            out << "public class " << proxyImpl << ": _ObjectPrxI, " << swiftName + "Prx";
            out << sb;
            out << nl << "override public class func ice_staticId() -> String";
            out << sb;
            out << nl << "return \"" << p->scoped() << "\"";
            out << eb;
            out << eb << nl;

            writeCastFuncs(out, p);
            writeStaticId(out, p);
        }
    }

    return false;
}

bool Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    // const string name = fixIdent(p->name());
    const string name = p->name();
    const string scoped = p->scoped();
    // const string abs = getAbsolute(p);
    // const bool basePreserved = p->inheritsMetaData("preserve-slice");
    // const bool preserved = p->hasMetaData("preserve-slice");

    ExceptionPtr base = p->base();

    out << nl << "public class " << name;
    if(base)
    {
        out << ": " << getAbsolute(base);
    }
    else if(p->isLocal())
    {
        out << ": Ice.LocalException";
    }
    else
    {
        out << ": Ice.UserException";
    }
    out << sb;

    if(!p->dataMembers().empty())
    {

        writeDataMembers(out, p->dataMembers());
        out << nl;
        writeInitializer(out, p->dataMembers(), p->allDataMembers());
    }

    if(!p->isLocal())
    {
        const DataMemberList members = p->dataMembers();

        out << nl << nl << "required public init(from ins: Ice.InputStream) throws" << sb;
        for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q) {
            out << nl << "try self." << (*q)->name() << " = " << typeToString((*q)->type()) << "(from: ins)";
        }
        out << eb;

        // TODO if we no longer need this.
        // out << nl << nl << "public func ice_read(from ins: Ice.InputStream) throws" << sb;
        // for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q) {
        //     out << nl << "try self." << (*q)->name() << ".ice_read(from: ins)";
        // }
        // out << eb;

        out << nl << nl << "public func ice_write(to os: Ice.OutputStream)" << sb;
        out << nl << "os.write(";
        for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q) {
            if(q != members.begin())
            {
                out << ", ";
            }
            out << "self." << (*q)->name();
        }
        out << ")";
        out << eb;

        out << nl << "public class func ice_staticId() -> String";
        out << sb;
        out << nl << "return \"" << p->scoped() << "\"";
        out << eb;
    }

    out << nl << eb;
    out << nl;

    return false;
}

bool Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    const string name = fixIdent(p->name());
    const string scoped = p->scoped();
    const string abs = getAbsolute(p);

    const DataMemberList members = p->dataMembers();

    string hashable = ": Hashable";
    for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q)
    {
        if(!BuiltinPtr::dynamicCast((*q)->type()))
        {
            hashable = "";
            break;
        }
    }

    out << nl << "public struct " << name << hashable << sb;

    writeDataMembers(out, members);
    writeInitializer(out, members);

    out << eb << nl;

    if(!p->isLocal()) {
        out << nl << "extension " << name << ": Ice.Streamable" << sb;

        out << nl << nl << "public init()" << sb;
        for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q) {
            out << nl << "self." << (*q)->name() << " = " << typeToString((*q)->type()) << "()";
        }
        out << eb;

        out << nl << nl << "public init(from ins: Ice.InputStream) throws" << sb;
        for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q) {
            out << nl << "try self." << (*q)->name() << " = " << typeToString((*q)->type()) << "(from: ins)";
        }
        out << eb;

        // TODO: remove if we no longer need this
        // out << nl << nl << "public mutating func ice_read(from ins: Ice.InputStream) throws" << sb;
        // for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q) {
        //     out << nl << "try self." << (*q)->name() << ".ice_read(from: ins)";
        // }
        // out << eb;

        out << nl << nl << "public func ice_write(to os: Ice.OutputStream)" << sb;
        out << nl << "os.write(";
        for(DataMemberList::const_iterator q = members.begin(); q != members.end(); ++q) {
            if(q != members.begin())
            {
                out << ", ";
            }
            out << "self." << (*q)->name();
        }
        out << ")";
        out << eb;

        out << eb << nl;
    }

    return false;
}

void Gen::TypesVisitor::visitSequence(const SequencePtr& p)
{
    const string name = fixIdent(p->name());
    const TypePtr type = p->type();

    out << nl << "public typealias " << name << " = [" << typeToString(type) << "]";
}

void Gen::TypesVisitor::visitDictionary(const DictionaryPtr& p)
{
    const string name = fixIdent(p->name());

    const TypePtr keyType = p->keyType();
    const TypePtr valueType = p->valueType();

    out << nl << "public typealias " << name << " = [" << typeToString(keyType) << ":" << typeToString(valueType) << "]";
}

void Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    const string name = fixIdent(p->name());
    const string scoped = p->scoped();
    const string abs = getAbsolute(p);

    const string enumType = p->maxValue() <= 0xFF ? "UInt8" : "Int32";

    out << nl << "public enum " << name << ": " << enumType << sb;

    const EnumeratorList enumerators = p->enumerators();

    for(EnumeratorList::const_iterator en = enumerators.begin(); en != enumerators.end(); ++en)
    {
        out << nl << "case " << fixIdent((*en)->name()) << " = " << (*en)->value();
    }

    out << eb << nl;

    out << nl << "extension " << name << ": Ice.Streamable" << sb;

    out << nl << nl << "public init()" << sb;

    out << nl << "self = ." << fixIdent((*enumerators.begin())->name());

    out << eb;

    out << nl << nl << "public init(from ins: Ice.InputStream) throws" << sb;
    out << nl << "var rawValue = " << enumType << "()";
    out << nl << "try ins.read(enum: &rawValue, maxValue: " << p->maxValue() << ")";
    out << nl << "guard let val = " << name << "(rawValue: rawValue) else" << sb;
    out << nl << "throw MarshalException(reason: \"invalid enum value\")";
    out << eb;
    out << nl << "self = val";
    out << eb;

    //TODO: remove if we no longer need this.
    // out << nl << nl << "public mutating func ice_read(from ins: Ice.InputStream) throws" << sb;
    // out << nl << "var rawValue = " << enumType << "()";
    // out << nl << "try ins.read(enum: &rawValue, maxValue: " << p->maxValue() << ")";
    // out << nl << "guard let val = " << name << "(rawValue: rawValue) else" << sb;
    // out << nl << "throw MarshalException(reason: \"invalid enum value\")";
    // out << eb;
    // out << nl << "self = val";
    // out << eb;

    out << nl << nl << "public func ice_write(to os: Ice.OutputStream)" << sb;
    out << nl << "os.write(enum: self.rawValue, maxValue: " << p->maxValue() << ")";
    out << eb;

    out << eb << nl;
}

void Gen::TypesVisitor::visitConst(const ConstPtr& p)
{
    const string name = fixIdent(p->name());
    const TypePtr type = p->type();

    out << nl << "public let " << name << ": " << typeToString(type) << " = " << p->value();
    out << nl;
}
