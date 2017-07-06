// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/Functional.h>
#include "Gen.h"
#include <limits>
#include <sys/stat.h>
#ifndef _WIN32
#  include <unistd.h>
#else
#  include <direct.h>
#endif
#include <IceUtil/Iterator.h>
#include <IceUtil/UUID.h>
#include <Slice/Checksum.h>
#include <Slice/FileTracker.h>
#include <Slice/Util.h>
#include <string.h>

using namespace std;
using namespace Slice;
using namespace IceUtilInternal;

namespace
{

string
sliceModeToIceMode(Operation::Mode opMode)
{
    string mode;
    switch(opMode)
    {
        case Operation::Normal:
        {
            mode = "ICENormal";
            break;
        }
        case Operation::Nonmutating:
        {
            mode = "ICENonmutating";
            break;
        }
        case Operation::Idempotent:
        {
            mode = "ICEIdempotent";
            break;
        }
        default:
        {
            assert(false);
            break;
        }
    }
    return mode;
}

string
opFormatTypeToString(const OperationPtr& op)
{
    switch(op->format())
    {
        case DefaultFormat:
        {
            return "ICEDefaultFormat";
            break;
        }
        case CompactFormat:
        {
            return "ICECompactFormat";
            break;
        }
        case SlicedFormat:
        {
            return "ICESlicedFormat";
            break;
        }
        default:
        {
            assert(false);
        }
    }

    return "???";
}

string
getDeprecateSymbol(const ContainedPtr& p1, const ContainedPtr& p2)
{
    string deprecateMetadata, deprecateSymbol;
    if(p1->findMetaData("deprecate", deprecateMetadata) ||
       (p2 != 0 && p2->findMetaData("deprecate", deprecateMetadata)))
    {
        string msg = "is deprecated";
        if(deprecateMetadata.find("deprecate:") == 0 && deprecateMetadata.size() > 10)
        {
            msg = deprecateMetadata.substr(10);
        }
        deprecateSymbol = " ICE_DEPRECATED_API(\"" + msg + "\")";
    }
    return deprecateSymbol;
}

string
getEscapedParamName(const OperationPtr& p, const string& name)
{
    ParamDeclList params = p->parameters();

    for(ParamDeclList::const_iterator i = params.begin(); i != params.end(); ++i)
    {
        if((*i)->name() == name)
        {
            return name + "_";
        }
    }
    return name;
}

class SortParamDeclByTagFn
{
public:

    static bool compare(const ParamDeclPtr& lhs, const ParamDeclPtr& rhs)
    {
        return lhs->tag() < rhs->tag();
    }
};

}

Slice::ObjCVisitor::ObjCVisitor(Output& h, Output& m, const string& dllExport) :
    _H(h),
    _M(m),
    _dllExport(dllExport)
{
}

Slice::ObjCVisitor::~ObjCVisitor()
{
}

void
Slice::ObjCVisitor::writeMarshalUnmarshalParams(const ParamDeclList& params, const OperationPtr& op, bool marshal,
                                                bool reference)
{
    ParamDeclList optionals;
    for(ParamDeclList::const_iterator p = params.begin(); p != params.end(); ++p)
    {
        if((*p)->optional())
        {
            optionals.push_back(*p);
        }
        else
        {
            string name = getParamName(*p, true);
            if(reference)
            {
                name = "*" + name;
            }
            writeMarshalUnmarshalCode(_M, (*p)->type(), name, marshal, true);
        }
    }
    if(op && op->returnType())
    {
        if(!op->returnIsOptional())
        {
            writeMarshalUnmarshalCode(_M, op->returnType(), "ret", marshal, true);
        }
    }

    optionals.sort(SortParamDeclByTagFn::compare);
    bool checkReturnType = op && op->returnIsOptional();
    for(ParamDeclList::const_iterator p = optionals.begin(); p != optionals.end(); ++p)
    {
        if(checkReturnType && op->returnTag() < (*p)->tag())
        {
            writeOptParamMarshalUnmarshalCode(_M, op->returnType(), "ret", op->returnTag(), marshal);
            checkReturnType = false;
        }

        string name = getParamName(*p, true);
        if(reference)
        {
            name = "*" + name;
        }

        writeOptParamMarshalUnmarshalCode(_M, (*p)->type(), name, (*p)->tag(), marshal);
    }
    if(checkReturnType)
    {
        writeOptParamMarshalUnmarshalCode(_M, op->returnType(), "ret", op->returnTag(), marshal);
    }
}

void
Slice::ObjCVisitor::writeDispatchAndMarshalling(const ClassDefPtr& p)
{
    string name = fixName(p);
    string scoped = p->scoped();
    ClassList allBases = p->allBases();
    StringList ids;

    transform(allBases.begin(), allBases.end(), back_inserter(ids), ::IceUtil::constMemFun(&Contained::scoped));

    StringList other;
    other.push_back(p->scoped());
    other.push_back("::Ice::Object");
    other.sort();
    ids.merge(other);
    ids.unique();

    StringList::const_iterator firstIter = ids.begin();
    StringList::const_iterator scopedIter = find(ids.begin(), ids.end(), scoped);
    assert(scopedIter != ids.end());
    StringList::difference_type scopedPos = IceUtilInternal::distance(firstIter, scopedIter);

    _M << sp << nl << "static NSString * iceS_" << name << "_ids[] =";
    _M << sb;
    {
        StringList::const_iterator q = ids.begin();
        while(q != ids.end())
        {
            _M << nl << "@\"" << *q << '"';
            if(++q != ids.end())
            {
                _M << ',';
            }
        }
    }
    _M << eb << ";";

    OperationList ops = p->operations();
    OperationList::const_iterator r;
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        ContainerPtr container = op->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        assert(cl);

        string opName = getName(op);
        _M << sp << nl << "+(void) iceD_" << op->name() << ":(id<" << name << ">)target current:(ICECurrent *)current "
           << "is:(id<ICEInputStream>)istr os:(id<ICEOutputStream>)ostr";
        _M << sb;

        _M << nl << "ICEInternalCheckModeAndSelector(target, " << sliceModeToIceMode(op->mode()) << ", @selector(";
        string selector = getSelector(op);
        if(!selector.empty())
        {
            _M << selector << "current:";
        }
        else
        {
            _M << opName << ":";
        }
        _M << "), current);";

        _M << nl << "ICEEncodingVersion* encoding = [istr startEncapsulation];";
        ParamDeclList inParams;
        ParamDeclList outParams;
        ParamDeclList paramList = op->parameters();
        for(ParamDeclList::const_iterator pli = paramList.begin(); pli != paramList.end(); ++pli)
        {
            if(!(*pli)->isOutParam())
            {
                inParams.push_back(*pli);
            }
            else
            {
                outParams.push_back(*pli);
            }
        }

        for(ParamDeclList::const_iterator inp = inParams.begin(); inp != inParams.end(); ++inp)
        {
            TypePtr type = (*inp)->type();
            _M << nl << outTypeToString(type, (*inp)->optional(), true) << " " << getParamName(*inp, true);
            if(!isValueType(type))
            {
                _M << " = nil";
            }
            _M << ";";
        }
        writeMarshalUnmarshalParams(inParams, 0, false);
        if(op->sendsClasses(false))
        {
            _M << nl << "[istr readPendingValues];";
        }
        _M << nl << "[istr endEncapsulation];";
        for(ParamDeclList::const_iterator outp = outParams.begin(); outp != outParams.end(); ++outp)
        {
            TypePtr type = (*outp)->type();
            _M << nl << inTypeToString(type, (*outp)->optional(), true) << " " << getParamName(*outp, true);
            if((*outp)->optional())
            {
                _M << " = ICENone";
            }
            _M << ";";
        }
        _M << nl << "[ostr startEncapsulation:encoding format:" << opFormatTypeToString(*r) << "];";
        TypePtr returnType = op->returnType();
        if(returnType)
        {
            _M << nl << inTypeToString(returnType, op->returnIsOptional(), true) << " ret = ";
        }
        else
        {
            _M << nl;
        }
        string args = getServerArgs(op);
        _M << "[target " << opName << args;
        if(!args.empty())
        {
            _M << " current";
        }
        _M << ":current];";
        writeMarshalUnmarshalParams(outParams, op, true);
        if(op->returnsClasses(false))
        {
            _M << nl << "[ostr writePendingValues];";
        }
        _M << nl << "[ostr endEncapsulation];";
        _M << eb;
    }

    OperationList allOps = p->allOperations();
    if(!allOps.empty())
    {
        map<string, string> allOpNames;
        for(OperationList::const_iterator p = allOps.begin(); p != allOps.end(); ++p)
        {
            allOpNames.insert(make_pair((*p)->name(), fixName(ClassDefPtr::dynamicCast((*p)->container()))));
        }

        allOpNames["ice_id"] = "ICEObject";
        allOpNames["ice_ids"] = "ICEObject";
        allOpNames["ice_isA"] = "ICEObject";
        allOpNames["ice_ping"] = "ICEObject";

        map<string, string>::const_iterator q;

        _M << sp << nl << "static NSString * iceS_" << name << "_all[] =";
        _M << sb;
        q = allOpNames.begin();
        while(q != allOpNames.end())
        {
            _M << nl << "@\"" << q->first << '"';
            if(++q != allOpNames.end())
            {
                _M << ',';
            }
        }
        _M << eb << ';';

        _M << sp << nl << "-(void) iceDispatch:(ICECurrent *)current is:(id<ICEInputStream>)istr "
           << "os:(id<ICEOutputStream>)ostr";
        _M << sb;
        _M << nl << "id target = [self iceTarget];";
        _M << nl << "switch(ICEInternalLookupString(iceS_" << name << "_all, sizeof(iceS_" << name
           << "_all) / sizeof(NSString*), current.operation))";
        _M << sb;
        int i = 0;
        for(q = allOpNames.begin(); q != allOpNames.end(); ++q)
        {
            _M << nl << "case " << i++ << ':';
            _M.inc();
            if(q->second == "ICEObject")
            {
                _M << nl << "[ICEServant iceD_" << q->first << ":(id<" << q->second << ">)self";
            }
            else
            {
                _M << nl << "[" << q->second << " iceD_" << q->first << ":(id<" << q->second << ">)target";
            }
            _M << " current:current is:istr os:ostr];";

            _M << nl << "return;";
            _M.dec();
        }
        _M << nl << "default:";
        _M.inc();
        _M << nl << "@throw [ICEOperationNotExistException operationNotExistException:";
        _M.useCurrentPosAsIndent();
        _M << "__FILE__";
        _M << nl << "line:__LINE__";
        _M << nl << "id:current.id_";
        _M << nl << "facet:current.facet";
        _M << nl << "operation:current.operation];";
        _M.restoreIndent();
        _M.dec();
        _M << eb;
        _M << eb;
    }

    _M << sp << nl << "+(NSString * const*) iceStaticIds:(int*)count idIndex:(int*)idx";
    _M << sb;
    _M << nl << "*count = sizeof(iceS_" << name << "_ids) / sizeof(NSString *);";
    _M << nl << "*idx = " << scopedPos << ";";
    _M << nl << "return iceS_" << name << "_ids;";
    _M << eb;
}

string
Slice::ObjCVisitor::getName(const OperationPtr& op) const
{
    if(!op->parameters().empty())
    {
        return fixId(op->name(), BaseTypeNone);
    }
    else
    {
        return fixId(op->name(), BaseTypeObject);
    }
}

string
Slice::ObjCVisitor::getSelector(const OperationPtr& op) const
{
    string result;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if(q == paramList.begin())
        {
            result += getName(op) + ":";
        }
        else
        {
            result += getParamId(*q) + ":";
        }
    }
    return result;
}

string
Slice::ObjCVisitor::getParams(const OperationPtr& op, bool internal) const
{
    string result;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        TypePtr type = (*q)->type();
        string typeString;
        if((*q)->isOutParam())
        {
            typeString = outTypeToString(type, (*q)->optional(), false, true);
        }
        else
        {
            typeString = inTypeToString(type, (*q)->optional());
        }
        if(q != paramList.begin())
        {
            result += " " + getParamId(*q);
        }
        result += ":(" + typeString + ")" + getParamName(*q, internal);
    }
    return result;
}

string
Slice::ObjCVisitor::getBlockParams(const OperationPtr& op) const
{
    string result;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        TypePtr type = (*q)->type();
        string typeString;
        if((*q)->isOutParam())
        {
            typeString = outTypeToString(type, (*q)->optional(), false, true);
        }
        else
        {
            typeString = inTypeToString(type, (*q)->optional());
        }
        if(q != paramList.begin())
        {
            result += " " + getParamId(*q);
        }
        result += "(" + typeString + ")";
    }
    return result;
}

string
Slice::ObjCVisitor::getMarshalParams(const OperationPtr& op, bool internal) const
{
    ParamDeclList paramList = op->parameters();
    string result;
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if(!(*q)->isOutParam())
        {
            TypePtr type = (*q)->type();
            if(!result.empty())
            {
                result += " " + getParamId(*q);
            }
            result += ":(" + inTypeToString(type, (*q)->optional()) + ")" + getParamName(*q, internal);
        }
    }
    return result;
}

string
Slice::ObjCVisitor::getUnmarshalParams(const OperationPtr& op, bool internal) const
{
    string result;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if((*q)->isOutParam())
        {
            TypePtr type = (*q)->type();
            if(!result.empty())
            {
                result += " " + getParamId(*q);
            }
            result += ":(" + outTypeToString(type, (*q)->optional(), false, true) + ")" + getParamName(*q, internal);
        }
    }
    return result;
}

string
Slice::ObjCVisitor::getServerParams(const OperationPtr& op) const
{
    string result;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        TypePtr type = (*q)->type();
        string typeString;
        if((*q)->isOutParam())
        {
            typeString = inTypeToString(type, (*q)->optional(), false, true);
        }
        else
        {
            typeString = outTypeToString(type, (*q)->optional());
        }
        if(q != paramList.begin())
        {
            result += " " + getParamId(*q);
        }
        result += ":(" + typeString + ")" + getParamName(*q, false);
    }
    return result;
}

string
Slice::ObjCVisitor::getResponseCBSig(const OperationPtr& op) const
{
    string result;
    TypePtr returnType = op->returnType();
    if(returnType)
    {
        result = outTypeToString(returnType, op->returnIsOptional());
    }

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if((*q)->isOutParam())
        {
            TypePtr type = (*q)->type();
            if(!result.empty())
            {
                result += ", ";
            }
            result += outTypeToString(type, (*q)->optional());
        }
    }
    return "void(^)(" + result + ")";
}

string
Slice::ObjCVisitor::getArgs(const OperationPtr& op) const
{
    string result;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if(q != paramList.begin())
        {
            result += " " + getParamId(*q);
        }
        result += ":" + getParamName(*q, true);
    }
    return result;
}

string
Slice::ObjCVisitor::getMarshalArgs(const OperationPtr& op) const
{
    string result;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if(!(*q)->isOutParam())
        {
            if(!result.empty())
            {
                result += " " + getParamId(*q);
            }
            result += ":" + getParamName(*q, true);
        }
    }
    return result;
}

string
Slice::ObjCVisitor::getUnmarshalArgs(const OperationPtr& op) const
{
    string result;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if((*q)->isOutParam())
        {
            if(!result.empty())
            {
                result += " " + getParamId(*q);
            }
            result += ":" + getParamName(*q, true);
        }
    }
    return result;
}

string
Slice::ObjCVisitor::getServerArgs(const OperationPtr& op) const
{
    string result;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if(q != paramList.begin())
        {
            result += " " + getParamId(*q);
        }
        result += ":";
        if((*q)->isOutParam())
        {
            result += "&";
        }
        result += getParamName(*q, true);
    }
    return result;
}

Slice::Gen::Gen(const string& name, const string& base, const string& include, const vector<string>& includePaths,
                const string& dir, const string& dllExport) :
    _base(base),
    _include(include),
    _includePaths(includePaths),
    _dllExport(dllExport)
{
    for(vector<string>::iterator p = _includePaths.begin(); p != _includePaths.end(); ++p)
    {
        *p = fullPath(*p);
    }

    string::size_type pos = _base.find_last_of("/\\");
    if(pos != string::npos)
    {
        _base.erase(0, pos + 1);
    }

    string fileH = _base + ".h";
    string fileM = _base + ".m";
    string fileImplH = _base + "I.h";
    string fileImplM = _base + "I.m";

    if(!dir.empty())
    {
        fileH = dir + '/' + fileH;
        fileM = dir + '/' + fileM;
        fileImplH = dir + '/' + fileImplH;
        fileImplM = dir + '/' + fileImplM;
    }

    _H.open(fileH.c_str());
    if(!_H)
    {
        ostringstream os;
        os << "cannot open `" << fileH << "': " << strerror(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }
    FileTracker::instance()->addFile(fileH);
    printHeader(_H);
    _H << nl << "// Generated from file `" << _base << ".ice'";

    _M.open(fileM.c_str());
    if(!_M)
    {
        ostringstream os;
        os << "cannot open `" << fileM << "': " << strerror(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }
    FileTracker::instance()->addFile(fileM);
    printHeader(_M);
    _M << nl << "// Generated from file `" << _base << ".ice'";
}

Slice::Gen::~Gen()
{
    if(_H.isOpen())
    {
        _H << '\n';
        _M << '\n';
    }
}

bool
Slice::Gen::operator!() const
{
    if(!_H || !_M)
    {
        return true;
    }
    return false;
}

void
Slice::Gen::generate(const UnitPtr& p)
{
    ObjCGenerator::validateMetaData(p);

    //
    // Give precedence to --dll-export command-line option
    //
    if(_dllExport.empty())
    {
        DefinitionContextPtr dc = p->findDefinitionContext(p->topLevelFile());
        assert(dc);
        static const string dllExportPrefix = "objc:dll-export:";
        string meta = dc->findMetaData(dllExportPrefix);
        if(meta.size() > dllExportPrefix.size())
        {
            _dllExport = meta.substr(dllExportPrefix.size());
        }
    }

    _H << sp << nl << "#import <objc/Ice/Config.h>";
    if(p->hasNonLocalClassDecls())
    {
        _H << nl << "#import <objc/Ice/Proxy.h>";
        _H << nl << "#import <objc/Ice/Current.h>";
        _H << nl << "#import <objc/Ice/Object.h>";
    }
    else if(p->hasLocalClassDefsWithAsync())
    {
        _H << nl << "#import <objc/Ice/Proxy.h>";
    }
    _H << nl << "#import <objc/Ice/Stream.h>";
    _H << nl << "#import <objc/Ice/LocalObject.h>";
    _H << nl << "#import <objc/Ice/Exception.h>";
    _M << nl << "#import <objc/Ice/LocalException.h>";
    _M << nl << "#import <objc/Ice/Stream.h>";
    if(p->hasContentsWithMetaData("preserve-slice"))
    {
        _H << nl << "#import <objc/Ice/SlicedData.h>";
    }
    _M << nl << "#import <";
    if(!_include.empty())
    {
        _M << _include << "/";
    }
    _M << _base << ".h>";

    // Necessary for objc_getClass use when marshalling/unmarshalling proxies.
    _M << nl << "#import <objc/runtime.h>";

    StringList includes = p->includeFiles();
    for(StringList::const_iterator q = includes.begin(); q != includes.end(); ++q)
    {
        static const string headerDirPrefix = "objc:header-dir:";
        DefinitionContextPtr dc = p->findDefinitionContext(*q);
        assert(dc);
        string meta = dc->findMetaData(headerDirPrefix);
        string headerDir;
        if(meta.size() > headerDirPrefix.size())
        {
            headerDir = meta.substr(headerDirPrefix.size());
        }
        _H << "\n#import <";
        if(!headerDir.empty())
        {
            _H << headerDir << "/";
        }
        _H << changeInclude(*q, _includePaths) << ".h>";
    }

    if(_dllExport.size())
    {
        _M << nl;
        _M << nl << "#ifndef " << _dllExport << "_EXPORTS";
        _M << nl << "#   define " << _dllExport << "_EXPORTS";
        _M << nl << "#endif";

        _H << nl;
        _H << nl << "#ifndef " << _dllExport;
        _H << nl << "#   if defined(ICE_STATIC_LIBS)";
        _H << nl << "#       define " << _dllExport << " /**/";
        _H << nl << "#   elif defined(" << _dllExport << "_EXPORTS)";
        _H << nl << "#       define " << _dllExport << " ICE_DECLSPEC_EXPORT";
        _H << nl << "#   else";
        _H << nl << "#       define " << _dllExport << " ICE_DECLSPEC_IMPORT";
        _H << nl << "#   endif";
        _H << nl << "#endif";
    }

    if(!_dllExport.empty())
    {
        _dllExport += " ";
    }

    //
    // Disable shadow warnings in .cppm file
    //
    _M << sp;
    _M.zeroIndent();
    _M << nl << "#ifdef __clang__";
    _M << nl << "#   pragma clang diagnostic ignored \"-Wshadow-ivar\"";
    _M << nl << "#endif";

    UnitVisitor unitVisitor(_H, _M, _dllExport);
    p->visit(&unitVisitor, false);

    ObjectDeclVisitor objectDeclVisitor(_H, _M, _dllExport);
    p->visit(&objectDeclVisitor, false);

    ProxyDeclVisitor proxyDeclVisitor(_H, _M, _dllExport);
    p->visit(&proxyDeclVisitor, false);

    TypesVisitor typesVisitor(_H, _M, _dllExport);
    p->visit(&typesVisitor, false);

    ProxyVisitor proxyVisitor(_H, _M, _dllExport);
    p->visit(&proxyVisitor, false);

    DelegateMVisitor delegateMVisitor(_H, _M, _dllExport);
    p->visit(&delegateMVisitor, false);

    HelperVisitor HelperVisitor(_H, _M, _dllExport);
    p->visit(&HelperVisitor, false);
}

void
Slice::Gen::closeOutput()
{
    _H.close();
    _M.close();
}

void
Slice::Gen::printHeader(Output& o)
{
    static const char* header =
"// **********************************************************************\n"
"//\n"
"// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.\n"
"//\n"
"// This copy of Ice is licensed to you under the terms described in the\n"
"// ICE_LICENSE file included in this distribution.\n"
"//\n"
"// **********************************************************************\n"
        ;

    o << header;
    o << "\n// Ice version " << ICE_STRING_VERSION;
}

Slice::Gen::UnitVisitor::UnitVisitor(Output& H, Output& M, const string& dllExport) :
    ObjCVisitor(H, M, dllExport)
{
}

bool
Slice::Gen::UnitVisitor::visitModuleStart(const ModulePtr& p)
{
    string dummy;
    if(p->findMetaData("objc:prefix", dummy))
    {
        _prefixes.push_back(modulePrefix(p));
    }
    return true;
}

void
Slice::Gen::UnitVisitor::visitUnitEnd(const UnitPtr& unit)
{
    string uuid = IceUtil::generateUUID();
    for(string::size_type pos = 0; pos < uuid.size(); ++pos)
    {
        if(!isalnum(uuid[pos]))
        {
            uuid[pos] = '_';
        }
    }

    if(!_prefixes.empty())
    {
        _M << sp << nl << "@implementation ICEInternalPrefixTable(C" << uuid << ")";
        _M << nl << "-(void)addPrefixes_C" << uuid << ":(NSMutableDictionary*)prefixTable";
        _M << sb;
        for(vector<Slice::ObjCGenerator::ModulePrefix>::const_iterator p = _prefixes.begin(); p != _prefixes.end(); ++p)
        {
            _M << nl << "[prefixTable setObject:@\"" << p->name << "\" forKey:@\"" << p->m->scoped() << "\"];";
        }
        _M << eb;
        _M << nl << "@end";
    }
}

Slice::Gen::ObjectDeclVisitor::ObjectDeclVisitor(Output& H, Output& M, const string& dllExport)
    : ObjCVisitor(H, M, dllExport)
{
}

void
Slice::Gen::ObjectDeclVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    if(p->definition() && p->definition()->isDelegate())
    {
        return;
    }

    _H << sp;
    if(!p->isLocal() || !p->isInterface())
    {
        _H << nl << "@class " << fixName(p) << ";";
    }
    _H << nl << "@protocol " << fixName(p) << ";";
}

Slice::Gen::ProxyDeclVisitor::ProxyDeclVisitor(Output& H, Output& M, const string& dllExport)
    : ObjCVisitor(H, M, dllExport)
{
}

void
Slice::Gen::ProxyDeclVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    if(!p->isLocal())
    {
        _H << sp << nl << "@class " << fixName(p) << "Prx;";
        _H << nl << "@protocol " << fixName(p) << "Prx;";
    }
}

Slice::Gen::TypesVisitor::TypesVisitor(Output& H, Output& M, const string& dllExport)
    : ObjCVisitor(H, M, dllExport)
{
}

bool
Slice::Gen::TypesVisitor::visitModuleStart(const ModulePtr& p)
{
    string suffix;
    StringList names = splitScopedName(p->scoped());
    for(StringList::const_iterator i = names.begin(); i != names.end(); ++i)
    {
        if(i != names.begin())
        {
            suffix += "_";
        }
        suffix += *i;
    }
    string symbol = "ICE_MODULE_PREFIX_";
    symbol += suffix;
    return true;
}

void
Slice::Gen::TypesVisitor::visitModuleEnd(const ModulePtr&)
{
}

bool
Slice::Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = fixName(p);
    ClassList bases = p->bases();

    if(p->isDelegate())
    {
        OperationPtr o = p->allOperations().front();
        _H << sp << nl << "typedef " << typeToString(o->returnType());
        _H << " (^" << name << ")" << getBlockParams(o) << ";";
        return false;
    }

    _H << sp << nl << _dllExport << "@protocol " << name;

    if(!bases.empty())
    {
        _H << " <";
        for(ClassList::const_iterator i = bases.begin(); i != bases.end(); ++i)
        {
            string baseName = fixName(*i);
            if(i != bases.begin())
            {
                _H << ", ";
            }
            _H << baseName;
        }
        _H << ">";
    }
    else
    {
        _H << " <NSObject>";
    }

    if(!p->isLocal() || !p->isInterface())
    {
        _M << sp << nl << "@implementation " << name;
    }
    return true;
}

void
Slice::Gen::TypesVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string name = fixName(p);
    ClassList bases = p->bases();
    bool basePreserved = p->inheritsMetaData("preserve-slice");
    bool preserved = p->hasMetaData("preserve-slice");
    bool hasBaseClass = !bases.empty() && !bases.front()->isInterface();
    string baseName = hasBaseClass ? fixName(bases.front()) : (p->isLocal() ? "ICELocalObject" : "ICEServant");
    DataMemberList baseDataMembers;
    if(hasBaseClass)
    {
        baseDataMembers = bases.front()->allDataMembers();
    }
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList optionalMembers = p->orderedOptionalDataMembers();
    DataMemberList allDataMembers = p->allDataMembers();

    _H << nl << "@end";

    if(p->isLocal() && p->isInterface())
    {
        return;
    }

    _H << sp << nl << _dllExport << "@interface " << name << " : " << baseName;

    if(!dataMembers.empty() || (preserved && !basePreserved))
    {
        //
        // Data member declarations.
        //
        _H << sb;

        if(!dataMembers.empty())
        {
            writeMembers(dataMembers, BaseTypeObject);
        }

        if(preserved && !basePreserved)
        {
            _H << nl << "id<ICESlicedData> iceSlicedData_;";
        }

        _H << eb;
        _H << sp;

        _M << sp;
    }

    //
    // @property and @synthesize for each data member.
    //
    writeProperties(dataMembers, BaseTypeObject);
    writeSynthesize(dataMembers, BaseTypeObject);

    //
    // Init, factory, copyWithZone and dealloc methods.
    //
    writeInit(p, dataMembers, baseDataMembers, allDataMembers, requiresMemberInit(dataMembers), BaseTypeObject, Other);
    writeFactory(p, allDataMembers, BaseTypeObject, Other);
    writeCopyWithZone(p, allDataMembers, BaseTypeObject, Other);
    writeMemberDealloc(dataMembers, BaseTypeObject, preserved && !basePreserved ? "iceSlicedData_" : "");

    //
    // Setter, has, clear selectors for optionals
    //
    writeOptionalDataMemberSelectors(dataMembers, BaseTypeObject);

    //
    // If the class uses a compact id we generate a +load method to register the compact id
    // with the compact id resolver.
    //
    if(p->compactId() >= 0)
    {
        _M << sp << nl << "+(void) load";
        _M << sb;
        _M << nl << "[CompactIdMapHelper registerClass:@\"" << p->scoped() << "\" value:" << p->compactId() << "];";
        _M << eb;
    }

    //
    // Operations
    //
    if(!p->isLocal())
    {
        OperationList ops = p->operations();
        OperationList::const_iterator r;
        for(r = ops.begin(); r != ops.end(); ++r)
        {
            OperationPtr op = *r;
            _H << nl << "+(void) iceD_" << op->name() << ":(id<" << name
               << ">)target current:(ICECurrent *)current "
               << "is:(id<ICEInputStream>)istr os:(id<ICEOutputStream>)ostr;";
        }

        //
        // Marshaling/unmarshaling
        //

        _M << sp << nl << "-(void) iceWriteImpl:(id<ICEOutputStream>)ostr";
        _M << sb;
        _M << nl << "[ostr startSlice:@\"" << p->scoped() << "\" compactId:" << p->compactId() << " lastSlice:"
           << (!hasBaseClass ? "YES" : "NO") << "];";
        writeMemberMarshal(dataMembers, optionalMembers, BaseTypeObject);
        _M << nl << "[ostr endSlice];";
        if(hasBaseClass)
        {
            _M << nl << "[super iceWriteImpl:ostr];";
        }
        _M << eb;

        _M << sp << nl << "-(void) iceReadImpl:(id<ICEInputStream>)istr";
        _M << sb;
        _M << nl << "[istr startSlice];";
        writeMemberUnmarshal(dataMembers, optionalMembers, BaseTypeObject);
        _M << nl << "[istr endSlice];";
        if(hasBaseClass)
        {
            _M << nl << "[super iceReadImpl:istr];";
        }
        _M << eb;

        writeDispatchAndMarshalling(p);

        if(preserved && !basePreserved)
        {
            _M << nl << "-(id<ICESlicedData>) ice_getSlicedData";
            _M << sb;
            _M << nl << "return ICE_AUTORELEASE(ICE_RETAIN(iceSlicedData_));";
            _M << eb;

            _M << nl << "-(void) iceWrite:(id<ICEOutputStream>)ostr";
            _M << sb;
            _M << nl << "[ostr startValue:iceSlicedData_];";
            _M << nl << "[self iceWriteImpl:ostr];";
            _M << nl << "[ostr endValue];";
            _M << eb;

            _M << nl << "-(void) iceRead:(id<ICEInputStream>)istr";
            _M << sb;
            _M << nl << "[istr startValue];";
            _M << nl << "[self iceReadImpl:istr];";
            _M << nl << "iceSlicedData_ = [istr endValue:YES];";
            _M << eb;
        }
    }

    _H << nl << "@end";
    _M << nl << "@end";
}

void
Slice::Gen::TypesVisitor::visitOperation(const OperationPtr& p)
{
    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);

    string name = getName(p);
    TypePtr returnType = p->returnType();
    string retString;
    string params;

    if(cl->isLocal())
    {
        retString = outTypeToString(returnType, p->returnIsOptional());
        params = getParams(p);
    }
    else
    {
        retString = inTypeToString(returnType, p->returnIsOptional());
        params = getServerParams(p);
    }

    const string deprecateSymbol = getDeprecateSymbol(p, cl);

    _H << nl << "-(" << retString << ") " << name << params;
    if(!cl->isLocal())
    {
        if(!params.empty())
        {
            _H << " current";
        }
        _H << ":(ICECurrent *)" << getEscapedParamName(p, "current") << deprecateSymbol << ";";
    }
    else
    {
        _H << deprecateSymbol << ";";
    }

    if(cl->isLocal() && (cl->hasMetaData("async-oneway") || p->hasMetaData("async-oneway")))
    {
        string marshalParams = getMarshalParams(p);
        string unmarshalParams = getUnmarshalParams(p);

        _H << nl << "-(id<ICEAsyncResult>) begin_" << name << marshalParams << deprecateSymbol << ";";
        _H << nl << "-(id<ICEAsyncResult>) begin_" << name << marshalParams;
        if(!marshalParams.empty())
        {
            _H << " exception";
        }
        _H << ":(void(^)(ICEException*))exception" << deprecateSymbol << ";";
        _H << nl << "-(id<ICEAsyncResult>) begin_" << name << marshalParams;
        if(!marshalParams.empty())
        {
            _H << " exception";
        }
        _H << ":(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent" << deprecateSymbol << ";";

        _H << nl << "-(void) end_" << name << unmarshalParams;
        if(!unmarshalParams.empty())
        {
            _H << " result";
        }
        _H << ":(id<ICEAsyncResult>)result" << deprecateSymbol << ";";
    }
}

void
Slice::Gen::TypesVisitor::visitSequence(const SequencePtr& p)
{
    string prefix = moduleName(findModule(p));
    string name = fixName(p);

    if(isValueType(p->type()))
    {
        _H << sp << nl << "typedef NSData " << name << ";";
        _H << nl << "typedef NSMutableData " << prefix << "Mutable" << p->name() << ";";
    }
    else
    {
        _H << sp << nl << "typedef NSArray " << name << ";";
        _H << nl << "typedef NSMutableArray " << prefix << "Mutable" << p->name() << ";";
    }
}

bool
Slice::Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string name = fixName(p);
    ExceptionPtr base = p->base();
    DataMemberList dataMembers = p->dataMembers();
    bool basePreserved = p->inheritsMetaData("preserve-slice");
    bool preserved = p->hasMetaData("preserve-slice");

    _H << sp;

    _H << nl << _dllExport << "@interface " << name << " : ";
    if(base)
    {
        _H << fixName(base);
    }
    else
    {
        _H << (p->isLocal() ? "ICELocalException" : "ICEUserException");
    }
    if(!dataMembers.empty() || (preserved && !basePreserved))
    {
        _H << sb;
    }

    _M << sp << nl << "@implementation " << name;

    return true;
}

void
Slice::Gen::TypesVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    string name = fixName(p);

    bool basePreserved = p->inheritsMetaData("preserve-slice");
    bool preserved = p->hasMetaData("preserve-slice");
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList optionalMembers = p->orderedOptionalDataMembers();
    DataMemberList allDataMembers = p->allDataMembers();
    DataMemberList::const_iterator q;

    DataMemberList baseDataMembers;
    if(p->base())
    {
        baseDataMembers = p->base()->allDataMembers();
    }

    if(!dataMembers.empty() || (preserved && !basePreserved))
    {
        //
        // Data member declarations.
        //
        if(!dataMembers.empty())
        {
            writeMembers(dataMembers, BaseTypeException);
        }

        if(preserved && !basePreserved)
        {
            _H << nl << "id<ICESlicedData> slicedData_;";
        }

        _H << eb;
        _H << sp;
        _M << sp;

        //
        // @property and @synthesize for each data member.
        //
        if(!dataMembers.empty())
        {
            writeProperties(dataMembers, BaseTypeException);
            writeSynthesize(dataMembers, BaseTypeException);
        }
        _H << sp;
    }

    //
    // ice_id
    //
    _H << nl << "-(NSString *) ice_id;";
    _M << sp << nl << "-(NSString *) ice_id";
    _M << sb;
    _M << nl << "return @\"" << p->scoped() << "\";";
    _M << eb;

    ContainerType ct = p->isLocal() ? LocalException : Other;
    writeInit(p, dataMembers, baseDataMembers, allDataMembers, requiresMemberInit(dataMembers), BaseTypeException, ct);
    writeFactory(p, allDataMembers, BaseTypeException, ct);
    writeCopyWithZone(p, allDataMembers, BaseTypeException, ct);
    writeMemberDealloc(dataMembers, BaseTypeException, preserved && !basePreserved ? "slicedData_" : "");

    //
    // Setter, has, clear selectors for optionals
    //
    writeOptionalDataMemberSelectors(dataMembers, BaseTypeObject);

    //
    // Marshaling/unmarshaling
    //
    ExceptionPtr base = p->base();
    if(!p->allClassDataMembers().empty())
    {
        if(!base || (base && !base->usesClasses(false)))
        {
            _M << sp << nl << "-(BOOL) iceUsesClasses";
            _M << sb;
            _M << nl << "return YES;";
            _M << eb;
        }
    }

    if(!p->isLocal())
    {
        _M << sp << nl << "-(void) iceWriteImpl:(id<ICEOutputStream>)ostr";
        _M << sb;
        _M << nl << "[ostr startSlice:@\"" << p->scoped() << "\" compactId:-1 lastSlice:"
           << (!base ? "YES" : "NO") << "];";
        writeMemberMarshal(dataMembers, optionalMembers, BaseTypeException);
        _M << nl << "[ostr endSlice];";
        if(base)
        {
            _M << nl << "[super iceWriteImpl:ostr];";
        }
        _M << eb;

        _M << sp << nl << "-(void) iceReadImpl:(id<ICEInputStream>)istr";
        _M << sb;
        _M << nl << "[istr startSlice];";
        writeMemberUnmarshal(dataMembers, optionalMembers, BaseTypeException);
        _M << nl << "[istr endSlice];";
        if(base)
        {
            _M << nl << "[super iceReadImpl:istr];";
        }
        _M << eb;

        if(preserved && !basePreserved)
        {
            _M << nl << nl << "-(id<ICESlicedData>) ice_getSlicedData";
            _M << sb;
            _M << nl << "return ICE_AUTORELEASE(ICE_RETAIN(slicedData_));";
            _M << eb;

            _M << nl << nl << "-(void) iceWrite:(id<ICEOutputStream>)ostr";
            _M << sb;
            _M << nl << "[ostr startException:slicedData_];";
            _M << nl << "[self iceWriteImpl:ostr];";
            _M << nl << "[ostr endException];";
            _M << eb;

            _M << nl << nl << "-(void) iceRead:(id<ICEInputStream>)istr";
            _M << sb;
            _M << nl << "[istr startException];";
            _M << nl << "[self iceReadImpl:istr];";
            _M << nl << "slicedData_ = [istr endException:YES];";
            _M << eb;
        }
    }

    _H << nl << "@end";
    _M << nl << "@end";
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    string name = fixName(p);

    _H << sp;

    _H << nl << _dllExport << "@interface " << name << " : NSObject <NSCopying>";
    _H << sb;
    _H << nl << "@private";
    _H.inc();

    _M << sp << nl << "@implementation " << name << sp;

    return true;
}

void
Slice::Gen::TypesVisitor::visitStructEnd(const StructPtr& p)
{
    string name = fixName(p);
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList::const_iterator q;

    //
    // Data member declarations.
    //
    writeMembers(dataMembers, BaseTypeObject);

    _H.dec();
    _H << eb;

    _H << sp;

    //
    // @property and @synthesize for each data member.
    //
    writeProperties(dataMembers, BaseTypeObject);
    writeSynthesize(dataMembers, BaseTypeObject);

    writeInit(p, dataMembers, DataMemberList(), dataMembers, requiresMemberInit(dataMembers), BaseTypeObject, Other);
    writeFactory(p, dataMembers, BaseTypeObject, Other);
    writeCopyWithZone(p, dataMembers, BaseTypeObject, Other);
    writeMemberDealloc(dataMembers, BaseTypeObject);

    //
    // hash
    //
    writeMemberHashCode(dataMembers, BaseTypeObject);

    //
    // isEqual
    //
    _M << sp << nl << "-(BOOL) isEqual:(id)o_";
    _M << sb;
    _M << nl << "if(self == o_)";
    _M << sb;
    _M << nl << "return YES;";
    _M << eb;
    _M << nl << "if(!o_ || ![o_ isKindOfClass:[self class]])";
    _M << sb;
    _M << nl << "return NO;";
    _M << eb;
    writeMemberEquals(dataMembers, BaseTypeObject);
    _M << eb;

    _H << nl << "// This class also overrides hash, and isEqual:";

    if(!p->isLocal())
    {
        //
        // Marshaling/unmarshaling
        //
        _H << nl << "-(void) iceWrite:(id<ICEOutputStream>)ostr;";
        _H << nl << "-(void) iceRead:(id<ICEInputStream>)istr;";

        _M << sp << nl << "-(void) iceWrite:(id<ICEOutputStream>)ostr";
        _M << sb;
        writeMemberMarshal(dataMembers, DataMemberList(), BaseTypeObject);
        _M << eb;

        _M << sp << nl << "-(void) iceRead:(id<ICEInputStream>)istr";
        _M << sb;
        writeMemberUnmarshal(dataMembers, DataMemberList(), BaseTypeObject);
        _M << eb;
    }

    _H << nl << "@end";
    _M << nl << "@end";
}

void
Slice::Gen::TypesVisitor::visitDictionary(const DictionaryPtr& p)
{
    string prefix = moduleName(findModule(p));
    string name = fixName(p);

    _H << sp << nl << "typedef NSDictionary " << name << ";";
    _H << nl << "typedef NSMutableDictionary " << prefix << "Mutable" << p->name() << ";";
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string name = fixName(p);
    EnumeratorList enumerators = p->enumerators();
    string enumeratorPrefix = p->hasMetaData("objc:scoped") ? p->name() : "";

    _H << sp;

    //
    // Check if any of the enumerators were assigned an explicit value.
    //
    const bool explicitValue = p->explicitValue();

    _H << nl << "typedef enum : ICEInt";
    _H << sb;
    EnumeratorList::const_iterator en = enumerators.begin();
    while(en != enumerators.end())
    {
        _H << nl << moduleName(findModule(*en)) << enumeratorPrefix << (*en)->name();
        //
        // If any of the enumerators were assigned an explicit value, we emit
        // an explicit value for *all* enumerators.
        //
        if(explicitValue)
        {
            _H << " = " << int64ToString((*en)->value());
        }
        if(++en != enumerators.end())
        {
            _H << ',';
        }
    }
    _H << eb << " " << name << ";";
}

void
Slice::Gen::TypesVisitor::visitConst(const ConstPtr& p)
{
    _H << sp;
    if(isString(p->type()))
    {
        _H << nl << "static NSString * const";
    }
    else
    {
        _H << nl << "static const " << typeToString(p->type());
    }
    _H << " " << fixName(p) << " = ";
    writeConstantValue(_H, p->type(), p->valueType(), p->value());
    _H << ';';
}

void
Slice::Gen::TypesVisitor::writeConstantValue(IceUtilInternal::Output& out, const TypePtr& type,
                                             const SyntaxTreeBasePtr& valueType, const string& val) const
{
    if(isString(type))
    {
        out << "@\"" << toStringLiteral(val, "\a\b\f\n\r\t\v", "?", Octal, 0) << "\"";
    }
    else
    {
        EnumPtr ep = EnumPtr::dynamicCast(type);
        if(ep)
        {
            EnumeratorPtr lte = EnumeratorPtr::dynamicCast(valueType);
            assert(lte);
            string enumeratorPrefix = ep->hasMetaData("objc:scoped") ? ep->name() : "";
            out << moduleName(findModule(ep)) << enumeratorPrefix << lte->name();
        }
        else
        {
            if(val == "true")
            {
                out << "YES";
            }
            else if(val == "false")
            {
                out << "NO";
            }
            else
            {
                out << val;
            }
        }
    }
}

void
Slice::Gen::TypesVisitor::writeInit(const ContainedPtr& p, const DataMemberList& dataMembers,
                                    const DataMemberList& baseDataMembers, const DataMemberList& allDataMembers,
                                    bool requiresMemberInit, int baseType, ContainerType ct) const
{
    if(dataMembers.empty())
    {
        return;
    }

    _H << sp;
    if(requiresMemberInit)
    {
        _H << nl << "-(id) init;";
        _M << sp << nl << "-(id) init";
        _M << sb;
        _M << nl << "self = [super init];";
        _M << nl << "if(!self)";
        _M << sb;
        _M << nl << "return nil;";
        _M << eb;
        writeMemberDefaultValueInit(dataMembers, baseType);
        _M << nl << "return self;";
        _M << eb;
    }
    _H << nl << "-(id) init";
    _M << sp << nl << "-(id) init";
    writeMemberSignature(allDataMembers, baseType, ct);
    _H << ";";
    _M << sb;
    _M << nl << "self = [super init";
    if(ct == LocalException)
    {
        _M << ":file_ line:line_";
    }
    writeMemberCall(baseDataMembers, baseType, ct, WithEscape);
    _M << "];";
    _M << nl << "if(!self)";
    _M << sb;
    _M << nl << "return nil;";
    _M << eb;
    writeMemberInit(dataMembers, baseType);
    _M << nl << "return self;";
    _M << eb;
}

void
Slice::Gen::TypesVisitor::writeFactory(const ContainedPtr& p, const DataMemberList& dataMembers, int baseType,
                                       ContainerType ct) const
{
    if(!dataMembers.empty())
    {
        // Always write factory method with no parameters first
        writeFactory(p, DataMemberList(), baseType, ct);
    }

    string name = fixName(p);
    string factoryMethod = getFactoryMethod(p, false);

    _H << nl << "+(id) " << factoryMethod;
    _M << sp << nl << "+(id) " << factoryMethod;
    writeMemberSignature(dataMembers, baseType, ct);
    _H << ";";
    _M << sb;

    //
    // The cast avoids a compiler warning that is emitted if different structs
    // have members with the same name but different types.
    //
    _M << nl << "return ICE_AUTORELEASE([(" << name << " *)[self alloc] init";
    if(ct == LocalException)
    {
        _M << ":file_ line:line_";
    }
    writeMemberCall(dataMembers, baseType, ct, WithEscape);
    _M << "]);";
    _M << eb;

    //
    // TODO: DEPRECATED: we used to
    //
    string deprecatedFactoryMethod = getFactoryMethod(p, true);
    if(factoryMethod != deprecatedFactoryMethod)
    {
        _H << nl << "+(id) " << deprecatedFactoryMethod;
        _M << sp << nl << "+(id) " << deprecatedFactoryMethod;
        writeMemberSignature(dataMembers, baseType, ct);
        _H << " ICE_DEPRECATED_API(\"use " << factoryMethod << " instead\");";
        _M << sb;
        _M << nl << "return ICE_AUTORELEASE([self " << factoryMethod;
        if(ct == LocalException)
        {
            _M << ":file_ line:line_";
        }
        writeMemberCall(dataMembers, baseType, ct, WithEscape);
        _M << "]);";
        _M << eb;
    }
}

void
Slice::Gen::TypesVisitor::writeCopyWithZone(const ContainedPtr& p, const DataMemberList& dataMembers,
                                            int baseType, ContainerType ct) const
{
    if(dataMembers.empty())
    {
        return;
    }

    string name = fixName(p);
    _M << sp << nl << "-(id) copyWithZone:(NSZone *)zone";
    _M << sb;
    _M << nl << "return [(" << name << " *)[[self class] allocWithZone:zone] init";
    if(ct == LocalException)
    {
        _M << ":file line:line";
    }
    writeMemberCall(dataMembers, baseType, ct, NoEscape);
    _M << "];";
    _M << eb;
    _H << nl << "// This class also overrides copyWithZone:";
}

void
Slice::Gen::TypesVisitor::writeMembers(const DataMemberList& dataMembers, int baseType) const
{
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        TypePtr type = (*q)->type();
        string name = fixId((*q)->name(), baseType);

        ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
        if(cl && cl->isInterface() && cl->isLocal())
        {
            // We can't just use id<PROTOCOL> here, PROTOCOL might be forward declared.
            _H << nl << "ICELocalObject<" << fixName(cl) << "> *";
        }
        else
        {
            _H << nl << typeToString(type) << " ";
            if(mapsToPointerType(type))
            {
                _H << "*";
            }
        }
        _H << name << ";";

        if((*q)->optional())
        {
            _H << nl << "BOOL has_" << name << "__;";
        }
    }
}

void
Slice::Gen::TypesVisitor::writeMemberSignature(const DataMemberList& dataMembers, int baseType,
                                               ContainerType ct) const
{
    if(ct == LocalException)
    {
        _H << ":(const char*)file line:(int)line";
        _M << ":(const char*)file_ line:(int)line_";
    }
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        TypePtr type = (*q)->type();
        string typeString;
        ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
        if(cl && cl->isInterface() && cl->isLocal())
        {
            // We can't just use id<PROTOCOL> here, PROTOCOL might be forward declared.
            typeString = "ICELocalObject<" + fixName(cl) + "> *";
        }
        else
        {
            typeString = inTypeToString(type, (*q)->optional());
        }

        if(q != dataMembers.begin() || ct == LocalException)
        {
            _H << " " << getParamId(*q);
            _M << " " << getParamId(*q);
        }
        _H << ":(" << typeString << ")" << fixId((*q)->name());
        _M << ":(" << typeString << ")" << getParamName(*q, true);
    }
}

void
Slice::Gen::TypesVisitor::writeMemberCall(const DataMemberList& dataMembers, int baseType,
                                          ContainerType ct, Escape esc) const
{
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string name = (*q)->name();

        if(q != dataMembers.begin() || ct == LocalException)
        {
            _M << " " << getParamId(*q);
        }

        name = esc == NoEscape ? fixId(name, baseType) : fixId(name);
        if(esc == NoEscape)
        {
            if((*q)->optional())
            {
                if(isValueType((*q)->type()))
                {
                    _M << ":" << "(self->has_" << name << "__ ? @(self->" << name << ") : ICENone)";
                }
                else
                {
                    _M << ":" << "(self->has_" << name << "__ ? self->" << name << " : ICENone)";
                }
            }
            else
            {
                _M << ":" << name;
            }
        }
        else
        {
            _M << ":" << getParamName(*q, true);
        }
    }
}

bool
Slice::Gen::TypesVisitor::requiresMemberInit(const DataMemberList& members) const
{
    for(DataMemberList::const_iterator p = members.begin(); p != members.end(); ++p)
    {
        if((*p)->defaultValueType())
        {
            return true;
        }

        if((*p)->optional())
        {
            return true;
        }

        TypePtr t = (*p)->type();
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(t);
        if((builtin && builtin->kind() == Builtin::KindString) || EnumPtr::dynamicCast(t) || StructPtr::dynamicCast(t))
        {
            return true;
        }
    }
    return false;
}

void
Slice::Gen::TypesVisitor::writeMemberDefaultValueInit(const DataMemberList& dataMembers, int baseType) const
{
    for(DataMemberList::const_iterator p = dataMembers.begin(); p != dataMembers.end(); ++p)
    {
        string name = fixId((*p)->name(), baseType);
        if((*p)->defaultValueType())
        {
            if((*p)->optional())
            {
                _M << nl << "self->has_" << name << "__ = YES;";
            }
            _M << nl << "self->" << name << " = ";
            writeConstantValue(_M, (*p)->type(), (*p)->defaultValueType(), (*p)->defaultValue());
            _M << ";";
        }
        else
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast((*p)->type());
            if(builtin && builtin->kind() == Builtin::KindString)
            {
                _M << nl <<  "self->" << name << " = @\"\";";
            }

            EnumPtr en = EnumPtr::dynamicCast((*p)->type());
            if(en)
            {
                string firstEnum = fixName(en->enumerators().front());
                _M << nl <<  "self->" << name << " = " << firstEnum << ';';
            }

            if(!(*p)->optional())
            {
                StructPtr st = StructPtr::dynamicCast((*p)->type());
                if(st)
                {
                    _M << nl <<  "self->" << name << " = [[" << typeToString(st) << " alloc] init];";
                }
            }
        }
    }
}

void
Slice::Gen::TypesVisitor::writeMemberInit(const DataMemberList& dataMembers, int baseType) const
{
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        TypePtr type = (*q)->type();
        string name = fixId((*q)->name());
        if((*q)->optional())
        {
            if(isValueType(type))
            {
                _M << nl << "self->has_" << name << "__ = [ICEOptionalGetter " << getOptionalHelperGetter(type) << ":";
                _M << getParamName(*q, true) << " value:&self->" << fixId((*q)->name(), baseType) << "];";
            }
            else
            {
                _M << nl << "self->has_" << name << "__ = [ICEOptionalGetter getRetained:";
                _M << getParamName(*q, true) << " value:&self->" << fixId((*q)->name(), baseType);
                _M << " type:[" << typeToObjCTypeString(type) << " class]];";
            }
        }
        else
        {
            if(isValueType(type))
            {
                _M << nl << "self->" << fixId((*q)->name(), baseType) << " = " << getParamName(*q, true) << ";";
            }
            else
            {
                _M << nl << "self->" << fixId((*q)->name(), baseType) << " = ICE_RETAIN(" << getParamName(*q, true) << ");";
            }
        }
    }
}

void
Slice::Gen::TypesVisitor::writeProperties(const DataMemberList& dataMembers, int baseType) const
{
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        TypePtr type = (*q)->type();
        string name = fixId((*q)->name(), baseType);

        string typeString;
        ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
        if(cl && cl->isInterface() && cl->isLocal())
        {
            // We can't just use id<PROTOCOL> here, PROTOCOL might be forward declared.
            typeString = "ICELocalObject<" + fixName(cl) + "> *";
        }
        else
        {
            typeString = typeToString(type) + " ";
            if(mapsToPointerType(type))
            {
                typeString += "*";
            }
        }

        _H << nl << "@property(nonatomic, ";
        if(isValueType(type))
        {
            _H << "assign";
        }
        else
        {
            _H << "ICE_STRONG_ATTR";
        }
        _H << ") " << typeString << name << ";";
    }
}

void
Slice::Gen::TypesVisitor::writeSynthesize(const DataMemberList& dataMembers, int baseType) const
{
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string name = fixId((*q)->name(), baseType);
        _M << nl << "@synthesize " << name << ";";
    }
}

void
Slice::Gen::TypesVisitor::writeOptionalDataMemberSelectors(const DataMemberList& dataMembers, int baseType) const
{
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        if(!(*q)->optional())
        {
            continue;
        }

        TypePtr type = (*q)->type();
        string typeString = inTypeToString(type, false);

        string name = fixId((*q)->name(), baseType);
        string paramName = getParamName(*q, true);

        string capName = (*q)->name();
        capName[0] = toupper(static_cast<unsigned char>(capName[0]));

        _H << nl << "-(void)set" << capName << ":(" << typeString << ")" << name << ";";

        _M << nl << "-(void)set" << capName << ":(" << typeString << ")" << paramName;
        _M << sb;
        _M << nl << "self->has_" << name << "__ = YES;";
        bool isValue = isValueType(type);
        if(isValue)
        {
            _M << nl << "self->" << name << " = " << paramName << ";";
        }
        else
        {
            _M << nl << "ICE_RELEASE(self->" << name << ");";
            _M << nl << "self->" << name << " = ICE_RETAIN(" << paramName << ");";
        }
        _M << eb;

        _H << nl << "-(BOOL)has" << capName << ";";
        _M << nl << "-(BOOL)has" << capName;
        _M << sb;
        _M << nl << "return self->has_" << name << "__;";
        _M << eb;

        _H << nl << "-(void)clear" << capName << ";";
        _M << nl << "-(void)clear" << capName;
        _M << sb;
        _M << nl << "self->has_" << name << "__ = NO;";
        if(!isValue)
        {
            _M << nl << "ICE_RELEASE(self->" << name << ");";
            _M << nl << "self->" << name << " = nil;";
        }
        _M << eb;
    }
}

void
Slice::Gen::TypesVisitor::writeMemberHashCode(const DataMemberList& dataMembers, int baseType) const
{
    _M << sp << nl << "-(NSUInteger) hash";
    _M << sb;
    _M << nl << "NSUInteger h_ = 5381;";
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        TypePtr type = (*q)->type();
        string name = fixId((*q)->name(), baseType);

        if(isValueType(type))
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
            if(builtin)
            {
                switch(builtin->kind())
                {
                    case Builtin::KindLong:
                    {
                        _M << nl << "h_ = ((h_ << 5) + h_) ^ (uint)(" << name << " ^ (" << name << " >> 32));";
                        break;
                    }
                    case Builtin::KindFloat:
                    case Builtin::KindDouble:
                    {
                        _M << sb;
                        _M << nl << "h_ = ((h_ << 5) + h_) ^ [@(" << name << ") hash];";
                        _M << eb;
                        break;
                    }
                    default:
                    {
                        _M << nl << "h_ = ((h_ << 5) + h_) ^ (2654435761u * " << name << ");";
                        break;
                    }
                }
            }
            else
            {
                 _M << nl << "h_ = ((h_ << 5) + h_) ^ " << name << ";";
            }
        }
        else
        {
            _M << nl << "h_ = ((h_ << 5) + h_) ^ [self->" << name << " hash];";
        }
    }
    _M << nl << "return h_;";
    _M << eb;
}

void
Slice::Gen::TypesVisitor::writeMemberEquals(const DataMemberList& dataMembers, int baseType) const
{
    if(!dataMembers.empty())
    {
        ContainerPtr container = (*dataMembers.begin())->container();
        ContainedPtr contained = ContainedPtr::dynamicCast(container);
        string containerName = fixName(contained);
        _M << nl << containerName << " *obj_ = (" << containerName << " *)o_;";
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            TypePtr type = (*q)->type();
            string name = fixId((*q)->name(), baseType);

            if(isValueType(type))
            {
                _M << nl << "if(" << name << " != obj_->" << name << ")";
                _M << sb;
                _M << nl << "return NO;";
                _M << eb;
            }
            else
            {
                _M << nl << "if(!" << name << ")";
                _M << sb;
                _M << nl << "if(obj_->" << name << ")";
                _M << sb;
                _M << nl << "return NO;";
                _M << eb;
                _M << eb;
                _M << nl << "else";
                _M << sb;
                _M << nl << "if(![self." << name << " ";
                _M << (isString(type) ? "isEqualToString" : "isEqual");
                _M << ":obj_->" << name << "])";
                _M << sb;
                _M << nl << "return NO;";
                _M << eb;
                _M << eb;
            }
        }
    }
    _M << nl << "return YES;";
}

void
Slice::Gen::TypesVisitor::writeMemberDealloc(const DataMemberList& dataMembers, int baseType, const string& slicedData) const
{
    bool needsDealloc = !slicedData.empty();
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        if(!isValueType((*q)->type()))
        {
            needsDealloc = true;
            break;
        }
    }
    if(!needsDealloc)
    {
        return; // No reference type data members.
    }

    _M << sp;
    _M.zeroIndent();
    _M << nl << "#if defined(__clang__) && !__has_feature(objc_arc)";
    _M.restoreIndent();
    _M << nl << "-(void) dealloc";
    _M << sb;
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        if(!isValueType((*q)->type()))
        {
            _M << nl << "[self->" << fixId((*q)->name(), baseType) << " release];";
        }
    }
    if(!slicedData.empty())
    {
        _M << nl << "[(NSObject*)" << slicedData << " release];";
    }
    _M << nl << "[super dealloc];";
    _M << eb;
    _M.zeroIndent();
    _M << nl << "#endif";
    _M.restoreIndent();
    _H << nl << "// This class also overrides dealloc";
}

void
Slice::Gen::TypesVisitor::writeMemberMarshal(const DataMemberList& dataMembers, const DataMemberList& optionalMembers,
                                             int baseType) const
{
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        if(!(*q)->optional())
        {
            writeMarshalUnmarshalCode(_M, (*q)->type(), "self->" + fixId((*q)->name(), baseType), true, false);
        }
    }
    for(DataMemberList::const_iterator q = optionalMembers.begin(); q != optionalMembers.end(); ++q)
    {
        string name = fixId((*q)->name(), baseType);
        string frmt = getOptionalFormat((*q)->type());
        _M << nl << "if(self->has_" << name << "__ && [ostr writeOptional:" << (*q)->tag() << " format:" << frmt << "])";
        _M << sb;
        writeOptMemberMarshalUnmarshalCode(_M, (*q)->type(), "self->" + name, true);
        _M << eb;
    }
}

void
Slice::Gen::TypesVisitor::writeMemberUnmarshal(const DataMemberList& dataMembers, const DataMemberList& optionalMembers,
                                               int baseType) const
{
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        if(!(*q)->optional())
        {
            writeMarshalUnmarshalCode(_M, (*q)->type(), "self->" + fixId((*q)->name(), baseType), false, false);
        }
    }
    for(DataMemberList::const_iterator q = optionalMembers.begin(); q != optionalMembers.end(); ++q)
    {
        string name = fixId((*q)->name(), baseType);
        string frmt = getOptionalFormat((*q)->type());
        _M << nl << "if([istr readOptional:" << (*q)->tag() << " format:" << frmt << "])";
        _M << sb;
        _M << nl << "self->has_" << name << "__ = YES;";
        writeOptMemberMarshalUnmarshalCode(_M, (*q)->type(), "self->" + name, false);
        _M << eb;
        _M << nl << "else";
        _M << sb;
        _M << nl << "self->has_" << name << "__ = NO;";
        _M << eb;
    }
}

Slice::Gen::ProxyVisitor::ProxyVisitor(Output& H, Output& M, const string& dllExport)
    : ObjCVisitor(H, M, dllExport)
{
}

bool
Slice::Gen::ProxyVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
    {
        return false;
    }

    string name = fixName(p);
    ClassList bases = p->bases();

    _H << sp << nl << _dllExport << "@protocol " << name << "Prx <";
    if(bases.empty())
    {
        _H << "ICEObjectPrx";
    }
    else
    {
        ClassList::const_iterator q = bases.begin();
        while(q != bases.end())
        {
            _H << fixName(*q) + "Prx";
            if(++q != bases.end())
            {
                _H << ", ";
            }
        }
    }
    _H << ">";

    return true;
}

void
Slice::Gen::ProxyVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _H << nl << "@end";
}

void
Slice::Gen::ProxyVisitor::visitOperation(const OperationPtr& p)
{
    string name = getName(p);
    TypePtr returnType = p->returnType();
    string retString = outTypeToString(returnType, p->returnIsOptional());
    string params = getParams(p);
    string marshalParams = getMarshalParams(p);
    string unmarshalParams = getUnmarshalParams(p);

    const string deprecateSymbol = getDeprecateSymbol(p, ClassDefPtr::dynamicCast(p->container()));

    //
    // Write two versions of the operation--with and without a
    // context parameter.
    //
    _H << nl << "-(" << retString << ") " << name << params << deprecateSymbol << ";";

    _H << nl << "-(" << retString << ") " << name << params;
    if(!params.empty())
    {
        _H << " context";
    }
    _H << ":(ICEContext *)" << getEscapedParamName(p, "context") << deprecateSymbol << ";";

    _H << nl << "-(id<ICEAsyncResult>) begin_" << p->name() << marshalParams << deprecateSymbol << ";";
    _H << nl << "-(id<ICEAsyncResult>) begin_" << p->name() << marshalParams;
    if(!marshalParams.empty())
    {
        _H << " context";
    }
    _H << ":(ICEContext *)" << getEscapedParamName(p, "context") << deprecateSymbol << ";";

    _H << nl << "-(" << retString << ") end_" << p->name() << unmarshalParams;
    if(!unmarshalParams.empty())
    {
        _H << " result";
    }
    _H << ":(id<ICEAsyncResult>)" << getEscapedParamName(p, "result") << deprecateSymbol << ";";

    string responseExceptionDecl = ":(" + getResponseCBSig(p) + ")" + getEscapedParamName(p, "response") +
        " exception:(void(^)(ICEException*))" + getEscapedParamName(p, "exception");
    string responseExceptionSentDecl = responseExceptionDecl + " sent:(void(^)(BOOL))" + getEscapedParamName(p, "sent");

    _H << nl << "-(id<ICEAsyncResult>) begin_" << p->name() << marshalParams;
    if(!marshalParams.empty())
    {
        _H << " response";
    }
    _H << responseExceptionDecl << deprecateSymbol << ";";

    _H << nl << "-(id<ICEAsyncResult>) begin_" << p->name() << marshalParams;
    if(!marshalParams.empty())
    {
        _H << " context";
    }
    _H << ":(ICEContext *)" << getEscapedParamName(p, "context") << " response" << responseExceptionDecl
       << deprecateSymbol << ";";

    _H << nl << "-(id<ICEAsyncResult>) begin_" << p->name() << marshalParams;
    if(!marshalParams.empty())
    {
        _H << " response";
    }
    _H << responseExceptionSentDecl << deprecateSymbol << ";";

    _H << nl << "-(id<ICEAsyncResult>) begin_" << p->name() << marshalParams;
    if(!marshalParams.empty())
    {
        _H << " context";
    }
    _H << ":(ICEContext *)" << getEscapedParamName(p, "context");
    _H << " response" << responseExceptionSentDecl << deprecateSymbol << ";";
}

Slice::Gen::HelperVisitor::HelperVisitor(Output& H, Output& M, const string& dllExport) :
    ObjCVisitor(H, M, dllExport)
{
}

bool
Slice::Gen::HelperVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
    {
        return false;
    }

    //
    // Proxy helper
    //
    {
        string name = moduleName(findModule(p)) + p->name() + "PrxHelper";
        _H << sp << nl << _dllExport << "@interface " << name << " : ICEProxyHelper";
        _H << nl << "@end";

        _M << sp << nl << "@implementation " << name;
        _M << nl << "+(id) readRetained:(id<ICEInputStream>)stream";
        _M << sb;
        _M << nl << "return [stream newProxy:[" << fixName(p) << "Prx class]];";
        _M << eb;
        _M << nl << "@end";
    }

    //
    // Class helper
    //
    {
        string name = moduleName(findModule(p)) + p->name() + "Helper";
        if(p->isInterface())
        {
            _H << sp << nl << "typedef ICEObjectHelper " << name << ";";
        }
        else
        {
            _H << sp << nl << _dllExport << "@interface " << name << " : ICEObjectHelper";
            _H << nl << "@end";

            _M << sp << nl << "@implementation " << name;
            _M << nl << "+(void) readRetained:(ICEObject*ICE_STRONG_QUALIFIER*)obj stream:(id<ICEInputStream>)stream";
            _M << sb;
            _M << nl << "[stream newValue:obj expectedType:[" << fixName(p) << " class]];";
            _M << eb;
            _M << nl << "@end";
        }
    }
    return false;
}

void
Slice::Gen::HelperVisitor::visitEnum(const EnumPtr& p)
{
    if(p->isLocal())
    {
        return;
    }

    string name = moduleName(findModule(p)) + p->name() + "Helper";

    _H << sp << nl << _dllExport << "@interface " << name << " : ICEEnumHelper";
    _H << nl << "@end";

    _M << sp << nl << "@implementation " << name;
    _M << nl << "+(ICEInt) getMinValue";
    _M << sb;
    _M << nl << "return " << p->minValue() << ";";
    _M << eb;
    _M << nl << "+(ICEInt) getMaxValue";
    _M << sb;
    _M << nl << "return " << p->maxValue() << ";";
    _M << eb;
    _M << nl << "@end";
}

void
Slice::Gen::HelperVisitor::visitSequence(const SequencePtr& p)
{
    if(p->isLocal())
    {
        return;
    }

    string name = moduleName(findModule(p)) + p->name() + "Helper";

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p->type());
    if(builtin)
    {
        _H << sp << nl << "typedef ICE" << getBuiltinName(builtin) << "SequenceHelper " << name << ";";
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(p->type());
    if(en)
    {
        _H << sp << nl << _dllExport << "@interface " << name << " : ICEEnumSequenceHelper";
        _H << nl << "@end";

        string typeS = typeToString(en);
        int min = en->minValue();
        int max = en->maxValue();
        _M << sp << nl << "@implementation " << name;
        _M << nl << "+(id) readRetained:(id<ICEInputStream>)stream";
        _M << sb;
        _M << nl << "return [stream newEnumSeq:" << min << " max:" << max << "];";
        _M << eb;
        _M << nl << "+(void) write:(id)obj stream:(id<ICEOutputStream>)stream";
        _M << sb;
        _M << nl << "[stream writeEnumSeq:obj min:" << min << " max:" << max << "];";
        _M << eb;
        _M << nl << "@end";
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(p->type());
    if(cl)
    {
        _H << sp << nl << _dllExport << "@interface " << name << " : ICEObjectSequenceHelper";
        _H << nl << "@end";

        _M << sp << nl << "@implementation " << name;
        _M << nl << "+(id) readRetained:(id<ICEInputStream>)stream";
        _M << sb;
        _M << nl << "return [stream newValueSeq:[" << fixName(cl) << " class]];";
        _M << eb;
        _M << nl << "@end";
        return;
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(p->type());
    ContainedPtr contained = ContainedPtr::dynamicCast(p->type());
    _H << sp << nl << _dllExport << "@interface " << name << " : ICEArraySequenceHelper";
    _H << nl << "@end";

    _M << sp << nl << "@implementation " << name;
    _M << nl << "+(Class) getElementHelper";
    _M << sb;
    if(proxy)
    {
        string name = moduleName(findModule(proxy->_class())) + proxy->_class()->name();
        _M << nl << "return [" << name << "PrxHelper class];";
    }
    else
    {
        assert(contained);
        _M << nl << "return [" << moduleName(findModule(contained)) + contained->name() + "Helper class];";
    }
    _M << eb;
    _M << nl << "+(Class) getOptionalHelper";
    _M << sb;
    if(p->type()->isVariableLength())
    {
        _M << nl << "return [ICEVarLengthOptionalHelper class];";
    }
    else if(p->type()->minWireSize() == 1)
    {
        _M << nl << "return [ICEFixedSize1SequenceOptionalHelper class];";
    }
    else
    {
        _M << nl << "return [ICEFixedSequenceOptionalHelper class];";
    }
    _M << eb;
    _M << nl << "@end";
}

void
Slice::Gen::HelperVisitor::visitDictionary(const DictionaryPtr& p)
{
    if(p->isLocal())
    {
        return;
    }

    string name = moduleName(findModule(p)) + p->name() + "Helper";

    TypePtr keyType = p->keyType();
    string keyS;
    BuiltinPtr keyBuiltin = BuiltinPtr::dynamicCast(keyType);
    if(keyBuiltin)
    {
        keyS = "ICE" + getBuiltinName(BuiltinPtr::dynamicCast(keyType)) + "Helper";
    }
    else
    {
        ContainedPtr contained = ContainedPtr::dynamicCast(keyType);
        keyS = moduleName(findModule(contained)) + contained->name() + "Helper";
    }

    string valueS;
    TypePtr valueType = p->valueType();
    BuiltinPtr valueBuiltin = BuiltinPtr::dynamicCast(valueType);
    ClassDeclPtr valueClass = ClassDeclPtr::dynamicCast(valueType);
    if((valueBuiltin && valueBuiltin->kind() == Builtin::KindObject) || valueClass)
    {
        _H << sp << nl << _dllExport << "@interface " << name << " : ICEObjectDictionaryHelper";
        _H << nl << "@end";

        _M << sp << nl << "@implementation " << name;
        _M << nl << "+(id) readRetained:(id<ICEInputStream>)stream";
        _M << sb;
        if(valueClass && !valueClass->isInterface())
        {
            valueS = fixName(valueClass);
            _M << nl << "return [stream newValueDict:[" << keyS << " class] expectedType:[" << valueS << " class]];";
        }
        else
        {
            _M << nl << "return [stream newValueDict:[" << keyS << " class] expectedType:[ICEObject class]];";
        }
        _M << eb;
        _M << nl << "+(void) write:(id)obj stream:(id<ICEOutputStream>)stream";
        _M << sb;
        _M << nl << "[stream writeValueDict:obj helper:[" << keyS << " class]];";
        _M << eb;
        _M << nl << "@end";
        return;
    }

    ProxyPtr valueProxy = ProxyPtr::dynamicCast(valueType);
    if(valueBuiltin)
    {
        valueS = "ICE" + getBuiltinName(BuiltinPtr::dynamicCast(valueType)) + "Helper";
    }
    else if(valueProxy)
    {
        valueS = moduleName(findModule(valueProxy->_class())) + valueProxy->_class()->name() + "PrxHelper";
    }
    else
    {
        ContainedPtr contained = ContainedPtr::dynamicCast(valueType);
        valueS = moduleName(findModule(contained)) + contained->name() + "Helper";
    }
    _H << sp << nl << _dllExport << "@interface " << name << " : ICEDictionaryHelper";
    _H << nl << "@end";

    _M << sp << nl << "@implementation " << name;
    _M << nl << "+(ICEKeyValueTypeHelper) getKeyValueHelper";
    _M << sb;
    _M << nl << "ICEKeyValueTypeHelper c;";
    _M << nl << "c.key = [" << keyS << " class];";
    _M << nl << "c.value = [" << valueS << " class];";
    _M << nl << "return c;";
    _M << eb;
    _M << nl << "+(Class) getOptionalHelper";
    _M << sb;
    if(keyType->isVariableLength() || valueType->isVariableLength())
    {
        _M << nl << "return [ICEVarLengthOptionalHelper class];";
    }
    else
    {
        _M << nl << "return [ICEFixedDictionaryOptionalHelper class];";
    }
    _M << eb;
    _M << nl << "@end";
}

bool
Slice::Gen::HelperVisitor::visitStructStart(const StructPtr& p)
{
    if(p->isLocal())
    {
        return false;
    }

    string name = fixName(p);

    _H << sp << nl << _dllExport << "@interface " << name  << "Helper : ICEStructHelper";
    _H << nl << "@end";

    _M << sp << nl << "@implementation " << name << "Helper";

    _M << nl << "+(Class) getType";
    _M << sb;
    _M << nl << "return [" << name << " class];";
    _M << eb;
    _M << nl << "+(Class) getOptionalHelper";
    _M << sb;
    if(p->isVariableLength())
    {
        _M << nl << "return [ICEVarLengthOptionalHelper class];";
    }
    else
    {
        _M << nl << "return [ICEFixedLengthOptionalHelper class];";
    }
    _M << eb;

    _M << nl << "+(ICEInt) minWireSize";
    _M << sb;
    _M << nl << "return " << p->minWireSize() << ";";
    _M << eb;

    _M << nl << "@end";
    return false;
}

Slice::Gen::DelegateMVisitor::DelegateMVisitor(Output& H, Output& M, const string& dllExport)
    : ObjCVisitor(H, M, dllExport)
{
}

bool
Slice::Gen::DelegateMVisitor::visitModuleStart(const ModulePtr& p)
{
    return true;
}

void
Slice::Gen::DelegateMVisitor::visitModuleEnd(const ModulePtr&)
{
}

bool
Slice::Gen::DelegateMVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
    {
        return false;
    }

    string name = fixName(p);
    ClassList bases = p->bases();
    OperationList ops = p->allOperations();
    OperationList::const_iterator r;

    _H << sp << nl << _dllExport << "@interface " << name << "Prx : ICEObjectPrx <" << name << "Prx>";
    _H << nl << "+(NSString *) ice_staticId;";

    _M << sp << nl << "@implementation " << name << "Prx";
    for(r = ops.begin(); r != ops.end(); ++r)
    {
        string opName = getName(*r);
        TypePtr returnType = (*r)->returnType();
        string retString = outTypeToString(returnType, (*r)->returnIsOptional());
        string params = getParams(*r, true);
        string args = getArgs(*r);

        ParamDeclList inParams;
        ParamDeclList outParams;
        ParamDeclList paramList = (*r)->parameters();
        for(ParamDeclList::const_iterator pli = paramList.begin(); pli != paramList.end(); ++pli)
        {
            if((*pli)->isOutParam())
            {
                outParams.push_back(*pli);
            }
            else
            {
                inParams.push_back(*pli);
            }
        }

        ContainerPtr container = (*r)->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        string className = fixName(cl);

        //
        // Write context-less operation that forwards to the version with a context.
        //
        _M << sp << nl << "-(" << retString << ") " << opName << params;
        _M << sb;
        _M << nl;
        if(returnType)
        {
            _M << "return ";
        }
        _M << "[self " << opName << args << (args.empty() ? ":nil" : " context:nil") << "];";
        _M << eb;

        //
        // Write version with context.
        //
        _M << sp << nl << "-(" << retString << ") " << opName << params;
        if(!params.empty())
        {
            _M << " context";
        }
        _M << ":(ICEContext *)context";
        _M << sb;
        if(returnType)
        {
            _M << nl << "__block " << retString << " ret";
            if(!isValueType(returnType))
            {
                _M << " = nil;";
            }
            else
            {
                _M << ";";
            }
        }

        string marshal;
        string marshalArgs = getMarshalArgs(*r);
        string marshalParams = getMarshalParams(*r, true);
        if(!inParams.empty())
        {
            _M << nl << "ICEMarshalCB marshal = ^(id<ICEOutputStream> ostr) ";
            _M << "{ [" << className << "Prx iceI_" << (*r)->name() << "_marshal" << marshalArgs;
            _M << (marshalArgs.empty() ? ":ostr" : " os:ostr") << "]; };";
            marshal = "marshal";
        }
        else
        {
            marshal = "nil";
        }

        string unmarshal;
        string unmarshalArgs = getUnmarshalArgs(*r);
        string unmarshalParams = getUnmarshalParams(*r, true);
        if((*r)->returnsData())
        {
            _M << nl << "ICEUnmarshalCB unmarshal = ^(id<ICEInputStream> istr, BOOL ok) ";
            if(returnType)
            {
                _M << "{ ret = ";
            }
            else
            {
                _M << "{ ";
            }
            _M << "[" << className << "Prx iceI_" << (*r)->name() << "_unmarshal" << unmarshalArgs;
            _M << (unmarshalArgs.empty() ? ":istr" : " is:istr") << " ok:ok]; };";
            unmarshal = "unmarshal";
        }
        else
        {
            unmarshal = "nil";
        }

        _M << nl << "[self iceInvoke:@\"" << (*r)->name() <<  "\" mode:" << sliceModeToIceMode((*r)->sendMode())
           << " format:" << opFormatTypeToString(*r) << " marshal:" << marshal
           << " unmarshal:" << unmarshal << " context:context];";
        if(returnType)
        {
            _M << nl << "return ret;";
        }
        _M << eb;

        //
        // Write begin_ context-less operation that forwards to the version with a context.
        //
        _M << sp << nl << "-(id<ICEAsyncResult>) begin_" << (*r)->name() << marshalParams;
        _M << sb;
        _M << nl << "return [self begin_" << (*r)->name() << marshalArgs;
        _M << (marshalArgs.empty() ? "" : " context") << ":nil];";
        _M << eb;

        //
        // Write begin_ version with context.
        //
        _M << sp << nl << "-(id<ICEAsyncResult>) begin_" << (*r)->name() << marshalParams;
        _M << (marshalParams.empty() ? "" : " context") << ":(ICEContext *)context";
        _M << sb;
        if(!inParams.empty())
        {
            _M << nl << "ICEMarshalCB marshal = ^(id<ICEOutputStream> ostr) ";
            _M << "{ [" << className << "Prx iceI_" << (*r)->name() << "_marshal" << marshalArgs;
            _M << (marshalArgs.empty() ? "" : " os") << ":ostr]; };";
        }
        _M << nl << "return [self iceI_begin_invoke:@\"" << (*r)->name() <<  "\" mode:"
           << sliceModeToIceMode((*r)->sendMode()) << " format:" << opFormatTypeToString(*r)
           << " marshal:" << marshal
           << " returnsData:" << ((*r)->returnsData() ? "YES" : "NO")
           << " context:context];";
        _M << eb;

        //
        // Write end_ operation
        //
        _M << sp << nl << "-(" << retString << ") end_" << (*r)->name() << unmarshalParams;
        if(!unmarshalParams.empty())
        {
            _M << " result";
        }
        _M << ":(id<ICEAsyncResult>)result";
        _M << sb;
        if(returnType)
        {
            _M << nl << "__block " << retString << " ret";
            if(!isValueType(returnType))
            {
                _M << " = nil;";
            }
            else
            {
                _M << ";";
            }
        }
        if((*r)->returnsData())
        {
            _M << nl << "ICEUnmarshalCB unmarshal = ^(id<ICEInputStream> istr, BOOL ok) ";
            if(returnType)
            {
                _M << "{ ret = ";
            }
            else
            {
                _M << "{ ";
            }
            _M << "[" << className << "Prx iceI_" << (*r)->name() << "_unmarshal" << unmarshalArgs;
            _M << (unmarshalArgs.empty() ? ":istr" : " is:istr") << " ok:ok]; };";
        }
        _M << nl << "[self iceI_end_invoke:@\"" << (*r)->name() << "\" unmarshal:" << unmarshal << " result:result];";
        if(returnType)
        {
            _M << nl << "return ret;";
        }
        _M << eb;

        //
        // Write begin_ operations with callbacks
        //
        string responseCBSig = getResponseCBSig(*r);

        _M << sp << nl << "-(id<ICEAsyncResult>) begin_" << (*r)->name() << marshalParams;
        _M << (marshalParams.empty() ? "" : " response") << ":(" << responseCBSig << ")response";
        _M << " exception:(void(^)(ICEException*))exception";
        _M << sb;
        _M << nl << "return [self begin_" << (*r)->name() << marshalArgs;
        _M << (marshalArgs.empty() ? "" : " context") << ":nil response:response exception:exception sent:nil];";
        _M << eb;

        _M << sp << nl << "-(id<ICEAsyncResult>) begin_" << (*r)->name() << marshalParams;
        _M << (marshalParams.empty() ? "" : " response") << ":(" << responseCBSig << ")response";
        _M << " exception:(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent";
        _M << sb;
        _M << nl << "return [self begin_" << (*r)->name() << marshalArgs;
        _M << (marshalArgs.empty() ? "" : " context") << ":nil response:response exception:exception sent:sent];";
        _M << eb;

        _M << sp << nl << "-(id<ICEAsyncResult>) begin_" << (*r)->name() << marshalParams;
        _M << (marshalParams.empty() ? "" : " context") << ":(ICEContext*)context";
        _M << " response:(" << responseCBSig << ")response exception:(void(^)(ICEException*))exception";
        _M << sb;
        _M << nl << "return [self begin_" << (*r)->name() << marshalArgs;
        _M << (marshalArgs.empty() ? "" : " context") << ":context response:response exception:exception sent:nil];";
        _M << eb;

        _M << sp << nl << "-(id<ICEAsyncResult>) begin_" << (*r)->name() << marshalParams;
        _M << (marshalParams.empty() ? "" : " context") << ":(ICEContext*)context";
        _M << " response:(" << responseCBSig << ")response exception:(void(^)(ICEException*))exception";
        _M << " sent:(void(^)(BOOL))sent";
        _M << sb;
        if(!inParams.empty())
        {
            _M << nl << "ICEMarshalCB marshal = ^(id<ICEOutputStream> ostr) ";
            _M << "{ [" << className << "Prx iceI_" << (*r)->name() << "_marshal" << marshalArgs;
            _M << (marshalArgs.empty() ? "" : " os") << ":ostr]; };";
        }
        if((*r)->returnsData())
        {
            _M << nl << "void(^completed)(id<ICEInputStream>, BOOL) = ^(id<ICEInputStream> istr, BOOL ok)";
            _M << sb;
            string responseCallArgs;
            string unmarshalCallArgs;
            if(!outParams.empty() || returnType)
            {
                if(returnType)
                {
                    responseCallArgs += "ret";
                }

                for(ParamDeclList::const_iterator op = outParams.begin(); op != outParams.end(); ++op)
                {
                    string name = getParamName(*op, true);
                    _M << nl << outTypeToString((*op)->type(), (*op)->optional(), true) << " " << name;
                    if(!isValueType((*op)->type()))
                    {
                        _M << " = nil";
                    }
                    _M << ";";

                    if(!unmarshalCallArgs.empty())
                    {
                        unmarshalCallArgs += " " + getParamId(*op);
                    }
                    unmarshalCallArgs += ":&" + name;

                    if(!responseCallArgs.empty())
                    {
                        responseCallArgs += ", ";
                    }
                    responseCallArgs += name;
                }
                if(returnType)
                {
                    _M << nl << outTypeToString(returnType, (*r)->returnIsOptional(), true) << " ret";
                    if(!isValueType(returnType))
                    {
                        _M << " = nil";
                    }
                    _M << ";";
                }
            }
            _M << nl << "@try";
            _M << sb;
            _M << nl;
            if(returnType)
            {
                _M << "ret = ";
            }
            _M << "[" << className << "Prx iceI_" << (*r)->name() << "_unmarshal" << unmarshalCallArgs;
            _M << (unmarshalCallArgs.empty() ? ":istr" : " is:istr") << " ok:ok];";
            _M << eb;
            _M << nl << "@catch(ICEException* ex)";
            _M << sb;
            _M << nl << "if(exception)";
            _M << sb;
            _M << nl << "exception(ex);";
            _M << eb;
            _M << nl << "return;";
            _M << eb;
            _M << nl << "if(response)";
            _M << sb;
            _M << nl << "response(" << responseCallArgs << ");";
            _M << eb;
            _M << eb << ";";
            if(returnType || !outParams.empty())
            {
                _M << nl << "return [self iceI_begin_invoke:@\"" << (*r)->name() <<  "\" mode:"
                   << sliceModeToIceMode((*r)->sendMode()) << " format:" << opFormatTypeToString(*r)
                   << " marshal:" << marshal
                   << " completed:completed response:(response != nil) exception:exception sent:sent context:context];";
            }
            else
            {
                _M << nl << "return [self iceI_begin_invoke:@\"" << (*r)->name() <<  "\" mode:"
                   << sliceModeToIceMode((*r)->sendMode()) << " format:" << opFormatTypeToString(*r)
                   << " marshal:" << marshal << " completed:completed response:YES exception:exception sent:sent context:context];";
            }
        }
        else
        {
            _M << nl << "return [self iceI_begin_invoke:@\"" << (*r)->name() <<  "\" mode:"
               << sliceModeToIceMode((*r)->sendMode()) << " format:" << opFormatTypeToString(*r)
               << " marshal:" << marshal << " response:response exception:exception sent:sent context:context];";
        }
        _M << eb;
    }

    _M << sp << nl << "+(NSString *) ice_staticId";
    _M << sb;
    _M << nl << "return @\"" << p->scoped() << "\";";
    _M << eb;

    return true;
}

void
Slice::Gen::DelegateMVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    _H << nl << "@end";
    _M << nl << "@end";
}

void
Slice::Gen::DelegateMVisitor::visitOperation(const OperationPtr& p)
{
    TypePtr returnType = p->returnType();
    string retString = outTypeToString(returnType, p->returnIsOptional());
    string marshalParams = getMarshalParams(p, true);
    string unmarshalParams = getUnmarshalParams(p, true);

    ParamDeclList inParams;
    ParamDeclList outParams;
    ParamDeclList paramList = p->parameters();
    for(ParamDeclList::const_iterator pli = paramList.begin(); pli != paramList.end(); ++pli)
    {
        if((*pli)->isOutParam())
        {
            outParams.push_back(*pli);
        }
        else
        {
            inParams.push_back(*pli);
        }
    }

    //
    // Write class method to invoke each operation.
    //
    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    string className = fixName(cl);
    if(!inParams.empty())
    {
        _H << nl << "+(void) iceI_" << p->name() << "_marshal" << marshalParams;
        if(!marshalParams.empty())
        {
            _H << " os";
        }
        _H << ":(id<ICEOutputStream>)ostr;";

        _M << sp << nl << "+(void) iceI_" << p->name() << "_marshal" << marshalParams;
        if(!marshalParams.empty())
        {
            _M << " os";
        }
        _M << ":(id<ICEOutputStream>)ostr";
        _M << sb;
        writeMarshalUnmarshalParams(inParams, 0, true);
        if(p->sendsClasses(false))
        {
            _M << nl << "[ostr writePendingValues];";
        }
        _M << eb;
    }

    if(p->returnsData())
    {
        _H << nl << "+(" << retString << ") iceI_" << p->name() << "_unmarshal" << unmarshalParams;
        if(!unmarshalParams.empty())
        {
            _H << " is";
        }
        _H << ":(id<ICEInputStream>)istr ok:(BOOL)ok;";

        _M << nl << "+(" << retString << ") iceI_" << p->name() << "_unmarshal" << unmarshalParams;
        if(!unmarshalParams.empty())
        {
            _M << " is";
        }
        _M << ":(id<ICEInputStream>)istr ok:(BOOL)ok";
        _M << sb;
        if(returnType)
        {
            _M << nl << outTypeToString(returnType, p->returnIsOptional(), true) << " ret";
            if(!isValueType(returnType))
            {
                _M << " = nil;";
            }
            else
            {
                _M << ";";
            }
        }
        if(p->returnsData())
        {
            for(ParamDeclList::const_iterator op = outParams.begin(); op != outParams.end(); ++op)
            {
                if(!isValueType((*op)->type()))
                {
                    _M << nl << "*" << getParamName(*op, true) << " = nil;";
                }
            }
        }
        _M << nl << "if(!ok)";
        _M << sb;
        _M << nl << "@try";
        _M << sb;
        _M << nl << "[istr startEncapsulation];";
        _M << nl << "[istr throwException];";
        _M << eb;
        //
        // Arrange exceptions into most-derived to least-derived order. If we don't
        // do this, a base exception handler can appear before a derived exception
        // handler, causing compiler warnings and resulting in the base exception
        // being marshaled instead of the derived exception.
        //
        ExceptionList throws = p->throws();
        throws.sort();
        throws.unique();
        throws.sort(Slice::DerivedToBaseCompare());

        for(ExceptionList::const_iterator e = throws.begin(); e != throws.end(); ++e)
        {
            _M << nl << "@catch(" << fixName(*e) << " *ex)";
            _M << sb;
            _M << nl << "[istr endEncapsulation];";
            _M << nl << "@throw;";
            _M << eb;
        }
        _M << nl << "@catch(ICEUserException *ex)";
        _M << sb;
        _M << nl << "[istr endEncapsulation];";
        _M << nl << "@throw [ICEUnknownUserException unknownUserException:__FILE__ line:__LINE__ "
           << "unknown:[ex ice_id]];";
        _M << eb;
        _M << eb;

        if(returnType || !outParams.empty())
        {
            _M << nl << "else";
            _M << sb;
            _M << nl << "[istr startEncapsulation];";
            writeMarshalUnmarshalParams(outParams, p, false, true);
            if(p->returnsClasses(false))
            {
                _M << nl << "[istr readPendingValues];";
            }
            _M << nl << "[istr endEncapsulation];";
            _M << eb;
        }

        if(returnType)
        {
            _M << nl << "return ret;";
        }
        _M << eb;
    }
}
