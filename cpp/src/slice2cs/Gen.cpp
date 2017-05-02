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
#include <IceUtil/StringUtil.h>
#include <IceUtil/FileUtil.h>
#include <Gen.h>

#include <limits>
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
#include <DotNetNames.h>
#include <string.h>

using namespace std;
using namespace Slice;
using namespace IceUtil;
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
            mode = "Ice.OperationMode.Normal";
            break;
        }
        case Operation::Nonmutating:
        {
            mode = "Ice.OperationMode.Nonmutating";
            break;
        }
        case Operation::Idempotent:
        {
            mode = "Ice.OperationMode.Idempotent";
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
        return "Ice.FormatType.DefaultFormat";
    case CompactFormat:
        return "Ice.FormatType.CompactFormat";
    case SlicedFormat:
        return "Ice.FormatType.SlicedFormat";
    default:
        assert(false);
    }

    return "???";
}

string
getDeprecateReason(const ContainedPtr& p1, const ContainedPtr& p2, const string& type)
{
    string deprecateMetadata, deprecateReason;
    if(p1->findMetaData("deprecate", deprecateMetadata) ||
       (p2 != 0 && p2->findMetaData("deprecate", deprecateMetadata)))
    {
        deprecateReason = "This " + type + " has been deprecated.";
        const string prefix = "deprecate:";
        if(deprecateMetadata.find(prefix) == 0 && deprecateMetadata.size() > prefix.size())
        {
            deprecateReason = deprecateMetadata.substr(prefix.size());
        }
    }
    return deprecateReason;
}

void
emitDeprecate(const ContainedPtr& p1, const ContainedPtr& p2, Output& out, const string& type)
{
    string reason = getDeprecateReason(p1, p2, type);
    if(!reason.empty())
    {
        out << nl << "[_System.Obsolete(\"" << reason << "\")]";
    }
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

string
getEscapedParamName(const DataMemberList& params, const string& name)
{
    for(DataMemberList::const_iterator i = params.begin(); i != params.end(); ++i)
    {
        if((*i)->name() == name)
        {
            return name + "_";
        }
    }
    return name;
}

string
resultStructReturnValueName(const ParamDeclList& outParams)
{
    for(ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end(); ++i)
    {
        if((*i)->name() == "returnValue")
        {
            return "returnValue_";
        }
    }
    return "returnValue";
}

}

Slice::CsVisitor::CsVisitor(Output& out) :
    _out(out)
{
}

Slice::CsVisitor::~CsVisitor()
{
}

void
Slice::CsVisitor::writeMarshalUnmarshalParams(const ParamDeclList& params, const OperationPtr& op, bool marshal,
                                              bool resultStruct, bool publicNames, const string& customStream)
{
    ParamDeclList optionals;

    string paramPrefix = "";
    string returnValueS = "ret";

    if(op && resultStruct)
    {
        if((op->returnType() && !params.empty()) || params.size() > 1)
        {
            paramPrefix = "ret.";
            returnValueS = resultStructReturnValueName(params);
        }
    }

    for(ParamDeclList::const_iterator pli = params.begin(); pli != params.end(); ++pli)
    {
        string param = paramPrefix.empty() && !publicNames ? "iceP_" + (*pli)->name() : fixId((*pli)->name());
        TypePtr type = (*pli)->type();
        if(!marshal && isClassType(type))
        {
            ostringstream os;
            os << '(' << typeToString(type) << " v) => {" << paramPrefix << param << " = v; }";
            param = os.str();
        }
        else
        {
            param = paramPrefix + param;
        }

        if((*pli)->optional())
        {
            optionals.push_back(*pli);
        }
        else
        {
            writeMarshalUnmarshalCode(_out, type, param, marshal, customStream);
        }
    }

    TypePtr ret;

    if(op && op->returnType())
    {
        ret = op->returnType();
        string param;
        if(!marshal && isClassType(ret))
        {
            ostringstream os;
            os << '(' << typeToString(ret) << " v) => {" << paramPrefix << returnValueS << " = v; }";
            param = os.str();
        }
        else
        {
            param = paramPrefix + returnValueS;
        }

        if(!op->returnIsOptional())
        {
            writeMarshalUnmarshalCode(_out, ret, param, marshal, customStream);
        }
    }

    //
    // Sort optional parameters by tag.
    //
    class SortFn
    {
    public:
        static bool compare(const ParamDeclPtr& lhs, const ParamDeclPtr& rhs)
        {
            return lhs->tag() < rhs->tag();
        }
    };
    optionals.sort(SortFn::compare);

    //
    // Handle optional parameters.
    //
    bool checkReturnType = op && op->returnIsOptional();
    for(ParamDeclList::const_iterator pli = optionals.begin(); pli != optionals.end(); ++pli)
    {
        if(checkReturnType && op->returnTag() < (*pli)->tag())
        {
            string param;
            if(!marshal && isClassType(ret))
            {
                ostringstream os;
                os << '(' << typeToString(ret) << " v) => {" << paramPrefix << returnValueS << " = v; }";
                param = os.str();
            }
            else
            {
                param = paramPrefix + returnValueS;
            }
            writeOptionalMarshalUnmarshalCode(_out, ret, param, op->returnTag(), marshal, customStream);
            checkReturnType = false;
        }

        string param = paramPrefix.empty() && !publicNames ? "iceP_" + (*pli)->name() : fixId((*pli)->name());
        TypePtr type = (*pli)->type();
        if(!marshal && isClassType(type))
        {
            ostringstream os;
            os << '(' << typeToString(type) << " v) => {" << paramPrefix << param << " = v; }";
            param = os.str();
        }
        else
        {
            param = paramPrefix + param;
        }

        writeOptionalMarshalUnmarshalCode(_out, type, param, (*pli)->tag(), marshal, customStream);
    }

    if(checkReturnType)
    {
        string param;
        if(!marshal && isClassType(ret))
        {
            ostringstream os;
            os << '(' << typeToString(ret) << " v) => {" << paramPrefix << returnValueS << " = v; }";
            param = os.str();
        }
        else
        {
            param = paramPrefix + returnValueS;
        }
        writeOptionalMarshalUnmarshalCode(_out, ret, param, op->returnTag(), marshal, customStream);
    }
}

void
Slice::CsVisitor::writeMarshalDataMember(const DataMemberPtr& member, const string& name, bool forStruct)
{
    if(member->optional())
    {
        assert(!forStruct);
        writeOptionalMarshalUnmarshalCode(_out, member->type(), name, member->tag(), true, "ostr_");
    }
    else
    {
        string stream = forStruct ? "" : "ostr_";
        string memberName = name;
        if(forStruct)
        {
            memberName = "this." + memberName;
        }

        writeMarshalUnmarshalCode(_out, member->type(), memberName, true, stream);
    }
}

void
Slice::CsVisitor::writeUnmarshalDataMember(const DataMemberPtr& member, const string& name, bool forStruct)
{
    string param = name;
    if(isClassType(member->type()))
    {
        ostringstream os;
        os << '(' << typeToString(member->type()) << " v) => { this." << name << " = v; }";
        param = os.str();
    }
    else if(forStruct)
    {
        param = "this." + name;
    }

    if(member->optional())
    {
        assert(!forStruct);
        writeOptionalMarshalUnmarshalCode(_out, member->type(), param, member->tag(), false, "istr_");
    }
    else
    {
        writeMarshalUnmarshalCode(_out, member->type(), param, false, forStruct ? "" : "istr_");
    }
}

void
Slice::CsVisitor::writeInheritedOperations(const ClassDefPtr& p)
{
    ClassList bases = p->bases();
    if(!bases.empty() && !bases.front()->isInterface())
    {
        bases.pop_front();
    }
    if(!bases.empty())
    {
        _out << sp << nl << "#region Inherited Slice operations";

        OperationList allOps;
        for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
        {
            OperationList tmp = (*q)->allOperations();
            allOps.splice(allOps.end(), tmp);
        }
        allOps.sort();
        allOps.unique();
        for(OperationList::const_iterator i = allOps.begin(); i != allOps.end(); ++i)
        {
            string retS;
            vector<string> params, args;
            string name = getDispatchParams(*i, retS, params, args);
            _out << sp << nl << "public abstract " << retS << " " << name << spar << params << epar << ';';
        }

        _out << sp << nl << "#endregion"; // Inherited Slice operations
    }
}

void
Slice::CsVisitor::writeDispatch(const ClassDefPtr& p)
{
    string name = fixId(p->name());
    string scoped = p->scoped();
    ClassList allBases = p->allBases();
    StringList ids;
    ClassList bases = p->bases();
    bool hasBaseClass = !bases.empty() && !bases.front()->isInterface();
    transform(allBases.begin(), allBases.end(), back_inserter(ids), constMemFun(&Contained::scoped));
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

    _out << sp << nl << "#region Slice type-related members";

    _out << sp;
    if(!p->isInterface())
    {
        emitGeneratedCodeAttribute();
    }

    _out << nl << "private static readonly string[] _ids =";
    _out << sb;
    {
        StringList::const_iterator q = ids.begin();
        while(q != ids.end())
        {
            _out << nl << '"' << *q << '"';
            if(++q != ids.end())
            {
                _out << ',';
            }
        }
    }
    _out << eb << ";";

    _out << sp;
    if(!p->isInterface())
    {
        emitGeneratedCodeAttribute();
    }
    _out << nl << "public override bool ice_isA(string s, Ice.Current current = null)";
    _out << sb;
    _out << nl << "return _System.Array.BinarySearch(_ids, s, IceUtilInternal.StringUtil.OrdinalStringComparer) >= 0;";
    _out << eb;

    _out << sp;
    if(!p->isInterface())
    {
        emitGeneratedCodeAttribute();
    }
    _out << nl << "public override string[] ice_ids(Ice.Current current = null)";
    _out << sb;
    _out << nl << "return _ids;";
    _out << eb;

    _out << sp;
    if(!p->isInterface())
    {
        emitGeneratedCodeAttribute();
    }
    _out << nl << "public override string ice_id(Ice.Current current = null)";
    _out << sb;
    _out << nl << "return _ids[" << scopedPos << "];";
    _out << eb;

    _out << sp;
    if(!p->isInterface())
    {
        emitGeneratedCodeAttribute();
    }

    _out << nl << "public static new string ice_staticId()";
    _out << sb;
    _out << nl << "return _ids[" << scopedPos << "];";
    _out << eb;

    _out << sp << nl << "#endregion"; // Slice type-related members

    OperationList ops = p->operations();
    if(ops.size() != 0)
    {
        _out << sp << nl << "#region Operation dispatch";
    }

    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        ContainerPtr container = op->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        assert(cl);

        string opName = op->name();
        _out << sp;
        _out << nl << "[_System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Design\", \"CA1011\")]";
        if(!p->isInterface())
        {
            emitGeneratedCodeAttribute();
        }
        _out << nl << "public static _System.Threading.Tasks.Task<Ice.OutputStream>";
        _out << nl << "iceD_" << opName << "(" << name << (p->isInterface() ? "" : "Disp_") << " obj, "
             <<  "IceInternal.Incoming inS, Ice.Current current)";
        _out << sb;

        TypePtr ret = op->returnType();
        ParamDeclList inParams = op->inParameters();
        ParamDeclList outParams = op->outParameters();

        _out << nl << "Ice.ObjectImpl.iceCheckMode(" << sliceModeToIceMode(op->mode()) << ", current.mode);";
        if(!inParams.empty())
        {
            //
            // Unmarshal 'in' parameters.
            //
            _out << nl << "var istr = inS.startReadParams();";
            for(ParamDeclList::const_iterator pli = inParams.begin(); pli != inParams.end(); ++pli)
            {
                string param = "iceP_" + (*pli)->name();
                string typeS = typeToString((*pli)->type(), (*pli)->optional());
                const bool isClass = isClassType((*pli)->type());

                if((*pli)->optional())
                {
                    _out << nl << typeS << ' ' << param << (isClass ? " = Ice.Util.None" : "") << ';';
                }
                else
                {
                    _out << nl << typeS << ' ' << param << ';';
                    StructPtr st = StructPtr::dynamicCast((*pli)->type());
                    if(st && isValueType(st))
                    {
                        _out << nl << param << " = new " << typeS << "();";
                    }
                    else if(st || isClass)
                    {
                        _out << nl << param << " = null;";
                    }
                }
            }
            writeMarshalUnmarshalParams(inParams, 0, false);
            if(op->sendsClasses(false))
            {
                _out << nl << "istr.readPendingValues();";
            }
            _out << nl << "inS.endReadParams();";
        }
        else
        {
            _out << nl << "inS.readEmptyParams();";
        }

        if(op->format() != DefaultFormat)
        {
            _out << nl << "inS.setFormat(" << opFormatTypeToString(op) << ");";
        }

        vector<string> inArgs;
        for(ParamDeclList::const_iterator pli = inParams.begin(); pli != inParams.end(); ++pli)
        {
            inArgs.push_back("iceP_" + (*pli)->name());
        }

        const bool amd = p->hasMetaData("amd") || op->hasMetaData("amd");
        if(op->hasMarshaledResult())
        {
            _out << nl << "return inS." << (amd ? "setMarshaledResultTask" : "setMarshaledResult");
            _out << "(obj." << opName << (amd ? "Async" : "") << spar << inArgs << "current" << epar << ");";
            _out << eb;
        }
        else if(amd)
        {
            string retS = resultType(op);
            _out << nl << "return inS.setResultTask" << (retS.empty() ? "" : ('<' + retS + '>'));
            _out << "(obj." << opName << "Async" << spar << inArgs << "current" << epar;
            if(!retS.empty())
            {
                _out << ",";
                _out.inc();
                if(!ret && outParams.size() == 1)
                {
                    _out << nl << "(ostr, " << "iceP_" << outParams.front()->name() << ") =>";
                }
                else
                {
                    _out << nl << "(ostr, ret) =>";
                }
                _out << sb;
                writeMarshalUnmarshalParams(outParams, op, true, true);
                if(op->returnsClasses(false))
                {
                    _out << nl << "ostr.writePendingValues();";
                }
                _out << eb;
                _out.dec();
            }
            _out << ");";
            _out << eb;
        }
        else
        {
            for(ParamDeclList::const_iterator pli = outParams.begin(); pli != outParams.end(); ++pli)
            {
                string typeS = typeToString((*pli)->type(), (*pli)->optional());
                _out << nl << typeS << ' ' << "iceP_" + (*pli)->name() << ";";
            }

            //
            // Call on the servant.
            //
            _out << nl;
            if(ret)
            {
                _out << "var ret = ";
            }
            _out << "obj." << fixId(opName, DotNet::ICloneable, true) << spar << inArgs;
            for(ParamDeclList::const_iterator pli = outParams.begin(); pli != outParams.end(); ++pli)
            {
                _out << "out iceP_" + (*pli)->name();
            }
            _out << "current" << epar << ';';

            //
            // Marshal 'out' parameters and return value.
            //
            if(!outParams.empty() || ret)
            {
                _out << nl << "var ostr = inS.startWriteParams();";
                writeMarshalUnmarshalParams(outParams, op, true);
                if(op->returnsClasses(false))
                {
                    _out << nl << "ostr.writePendingValues();";
                }
                _out << nl << "inS.endWriteParams(ostr);";
                _out << nl << "return inS.setResult(ostr);";
            }
            else
            {
                _out << nl << "return inS.setResult(inS.writeEmptyParams());";
            }
            _out << eb;
        }
    }

    OperationList allOps = p->allOperations();
    if(!allOps.empty() || (!p->isInterface() && !hasBaseClass))
    {
        StringList allOpNames;
        transform(allOps.begin(), allOps.end(), back_inserter(allOpNames), constMemFun(&Contained::name));
        allOpNames.push_back("ice_id");
        allOpNames.push_back("ice_ids");
        allOpNames.push_back("ice_isA");
        allOpNames.push_back("ice_ping");
        allOpNames.sort();
        allOpNames.unique();

        _out << sp << nl << "private static readonly string[] _all =";
        _out << sb;
        for(StringList::const_iterator q = allOpNames.begin(); q != allOpNames.end();)
        {
            _out << nl << '"' << *q << '"';
            if(++q != allOpNames.end())
            {
                _out << ',';
            }
        }
        _out << eb << ';';

        _out << sp;
        if(!p->isInterface())
        {
            emitGeneratedCodeAttribute();
        }
        _out << nl << "public override _System.Threading.Tasks.Task<Ice.OutputStream>";
        _out << nl << "iceDispatch(IceInternal.Incoming inS, Ice.Current current)";
        _out << sb;
        _out << nl << "int pos = _System.Array.BinarySearch(_all, current.operation, "
            << "IceUtilInternal.StringUtil.OrdinalStringComparer);";
        _out << nl << "if(pos < 0)";
        _out << sb;
        _out << nl << "throw new Ice.OperationNotExistException(current.id, current.facet, current.operation);";
        _out << eb;
        _out << sp << nl << "switch(pos)";
        _out << sb;
        int i = 0;
        for(StringList::const_iterator q = allOpNames.begin(); q != allOpNames.end(); ++q)
        {
            string opName = *q;

            _out << nl << "case " << i++ << ':';
            _out << sb;
            if(opName == "ice_id")
            {
                _out << nl << "return Ice.ObjectImpl.iceD_ice_id(this, inS, current);";
            }
            else if(opName == "ice_ids")
            {
                _out << nl << "return Ice.ObjectImpl.iceD_ice_ids(this, inS, current);";
            }
            else if(opName == "ice_isA")
            {
                _out << nl << "return Ice.ObjectImpl.iceD_ice_isA(this, inS, current);";
            }
            else if(opName == "ice_ping")
            {
                _out << nl << "return Ice.ObjectImpl.iceD_ice_ping(this, inS, current);";
            }
            else
            {
                //
                // There's probably a better way to do this
                //
                for(OperationList::const_iterator t = allOps.begin(); t != allOps.end(); ++t)
                {
                    if((*t)->name() == (*q))
                    {
                        ContainerPtr container = (*t)->container();
                        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
                        assert(cl);
                        if(cl->scoped() == p->scoped())
                        {
                            _out << nl << "return iceD_" << opName << "(this, inS, current);";
                        }
                        else
                        {
                            _out << nl << "return " << fixId(cl->scoped() + "Disp_") << ".iceD_" << opName << "(this, inS, current);";
                        }
                        break;
                    }
                }
            }
            _out << eb;
        }
        _out << eb;
        _out << sp << nl << "_System.Diagnostics.Debug.Assert(false);";
        _out << nl << "throw new Ice.OperationNotExistException(current.id, current.facet, current.operation);";
        _out << eb;
    }

    if(ops.size() != 0)
    {
        _out << sp << nl << "#endregion"; // Operation dispatch
    }
}

void
Slice::CsVisitor::writeMarshaling(const ClassDefPtr& p)
{
    string name = fixId(p->name());
    string scoped = p->scoped();
    ClassList allBases = p->allBases();
    StringList ids;
    ClassList bases = p->bases();

    transform(allBases.begin(), allBases.end(), back_inserter(ids), constMemFun(&Contained::scoped));
    StringList other;
    other.push_back(p->scoped());
    other.push_back("::Ice::Value");
    other.sort();
    ids.merge(other);
    ids.unique();

    assert(find(ids.begin(), ids.end(), scoped) != ids.end());

    //
    // Marshaling support
    //
    DataMemberList allClassMembers = p->allClassDataMembers();
    DataMemberList members = p->dataMembers();
    DataMemberList optionalMembers = p->orderedOptionalDataMembers();
    DataMemberList classMembers = p->classDataMembers();
    const bool basePreserved = p->inheritsMetaData("preserve-slice");
    const bool preserved = p->hasMetaData("preserve-slice");

    ClassDefPtr base;
    if(!bases.empty() && !bases.front()->isInterface())
    {
        base = bases.front();
    }

    _out << sp << nl << "#region Marshaling support";

    if(preserved && !basePreserved)
    {
        _out << sp;
        if(!p->isInterface())
        {
            emitGeneratedCodeAttribute();
        }

        _out << nl << "public override void iceWrite(Ice.OutputStream ostr_)";
        _out << sb;
        _out << nl << "ostr_.startValue(iceSlicedData_);";
        _out << nl << "iceWriteImpl(ostr_);";
        _out << nl << "ostr_.endValue();";
        _out << eb;

        _out << sp;
        if(!p->isInterface())
        {
            emitGeneratedCodeAttribute();
        }
        _out << nl << "public override void iceRead(Ice.InputStream istr_)";
        _out << sb;
        _out << nl << "istr_.startValue();";
        _out << nl << "iceReadImpl(istr_);";
        _out << nl << "iceSlicedData_ = istr_.endValue(true);";
        _out << eb;
    }

    _out << sp;
    if(!p->isInterface())
    {
        emitGeneratedCodeAttribute();
    }
    _out << nl << "protected override void iceWriteImpl(Ice.OutputStream ostr_)";
    _out << sb;
    _out << nl << "ostr_.startSlice(ice_staticId(), " << p->compactId() << (!base ? ", true" : ", false") << ");";
    for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
    {
        if(!(*d)->optional())
        {
            writeMarshalDataMember(*d, fixId(*d, DotNet::ICloneable, true));
        }
    }
    for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
    {
        writeMarshalDataMember(*d, fixId(*d, DotNet::ICloneable, true));
    }
    _out << nl << "ostr_.endSlice();";
    if(base)
    {
        _out << nl << "base.iceWriteImpl(ostr_);";
    }
    _out << eb;

    _out << sp;
    if(!p->isInterface())
    {
        emitGeneratedCodeAttribute();
    }
    _out << nl << "protected override void iceReadImpl(Ice.InputStream istr_)";
    _out << sb;
    _out << nl << "istr_.startSlice();";
    for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
    {
        if(!(*d)->optional())
        {
            writeUnmarshalDataMember(*d, fixId(*d, DotNet::ICloneable, true));
        }
    }
    for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
    {
        writeUnmarshalDataMember(*d, fixId(*d, DotNet::ICloneable, true));
    }
    _out << nl << "istr_.endSlice();";
    if(base)
    {
        _out << nl << "base.iceReadImpl(istr_);";
    }
    _out << eb;

    if(preserved && !basePreserved)
    {
        _out << sp << nl << "protected Ice.SlicedData iceSlicedData_;";
    }

    _out << sp << nl << "#endregion"; // Marshalling support
}

string
Slice::CsVisitor::getParamAttributes(const ParamDeclPtr& p)
{
    string result;
    StringList metaData = p->getMetaData();
    for(StringList::const_iterator i = metaData.begin(); i != metaData.end(); ++i)
    {
        static const string prefix = "cs:attribute:";
        if(i->find(prefix) == 0)
        {
            result += "[" + i->substr(prefix.size()) + "] ";
        }
    }
    return result;
}

vector<string>
Slice::CsVisitor::getParams(const OperationPtr& op)
{
    vector<string> params;
    ParamDeclList paramList = op->parameters();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container()); // Get the class containing the op.
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        string param = getParamAttributes(*q);
        if((*q)->isOutParam())
        {
            param += "out ";
        }
        param += typeToString((*q)->type(), (*q)->optional(), cl->isLocal()) + " " + fixId((*q)->name());
        params.push_back(param);
    }
    return params;
}

vector<string>
Slice::CsVisitor::getInParams(const OperationPtr& op, bool internal)
{
    vector<string> params;

    string name = fixId(op->name());
    ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container()); // Get the class containing the op.

    ParamDeclList paramList = op->inParameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        params.push_back(getParamAttributes(*q) + typeToString((*q)->type(), (*q)->optional(), cl->isLocal())
                         + " " + (internal ? "iceP_" + (*q)->name() : fixId((*q)->name())));
    }
    return params;
}

vector<string>
Slice::CsVisitor::getOutParams(const OperationPtr& op, bool returnParam, bool outKeyword)
{
    vector<string> params;

    if(returnParam)
    {
        TypePtr ret = op->returnType();
        if(ret)
        {
            params.push_back(typeToString(ret, op->returnIsOptional()) + " ret");
        }
    }

    ParamDeclList paramList = op->outParameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        string s = getParamAttributes(*q);
        if(outKeyword)
        {
            s += "out ";
        }
        s += typeToString((*q)->type(), (*q)->optional()) + ' ' + fixId((*q)->name());
        params.push_back(s);
    }

    return params;
}

vector<string>
Slice::CsVisitor::getArgs(const OperationPtr& op)
{
    vector<string> args;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        string arg = fixId((*q)->name());
        if((*q)->isOutParam())
        {
            arg = "out " + arg;
        }
        args.push_back(arg);
    }
    return args;
}

vector<string>
Slice::CsVisitor::getInArgs(const OperationPtr& op, bool internal)
{
    vector<string> args;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if(!(*q)->isOutParam())
        {
            args.push_back(internal ? "iceP_" + (*q)->name() : fixId((*q)->name()));
        }
    }
    return args;
}

string
Slice::CsVisitor::getDispatchParams(const OperationPtr& op, string& retS, vector<string>& params, vector<string>& args)
{
    string name;

    ParamDeclList paramDecls;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());

    if(cl->hasMetaData("amd") || op->hasMetaData("amd"))
    {
        name = op->name() + "Async";
        params = getInParams(op);
        args = getInArgs(op);
        paramDecls = op->inParameters();
        retS = taskResultType(op, true);
    }
    else if(op->hasMarshaledResult())
    {
        name = fixId(op->name(), DotNet::ICloneable, true);
        params = getInParams(op);
        args = getInArgs(op);
        paramDecls = op->inParameters();
        retS = resultType(op, true);
    }
    else
    {
        name = fixId(op->name(), DotNet::ICloneable, true);
        params = getParams(op);
        args = getArgs(op);
        paramDecls = op->parameters();
        retS = typeToString(op->returnType(), op->returnIsOptional());
    }

    string currentParamName = getEscapedParamName(op, "current");
    params.push_back("Ice.Current " + currentParamName + " = null");
    args.push_back(currentParamName);
    return name;
}

void
Slice::CsVisitor::emitAttributes(const ContainedPtr& p)
{
    StringList metaData = p->getMetaData();
    for(StringList::const_iterator i = metaData.begin(); i != metaData.end(); ++i)
    {
        static const string prefix = "cs:attribute:";
        if(i->find(prefix) == 0)
        {
            _out << nl << '[' << i->substr(prefix.size()) << ']';
        }
    }
}

void
Slice::CsVisitor::emitComVisibleAttribute()
{
    _out << nl << "[_System.Runtime.InteropServices.ComVisible(false)]";
}

void
Slice::CsVisitor::emitGeneratedCodeAttribute()
{
    _out << nl << "[_System.CodeDom.Compiler.GeneratedCodeAttribute(\"slice2cs\", \"" << ICE_STRING_VERSION << "\")]";
}

void
Slice::CsVisitor::emitPartialTypeAttributes()
{
    //
    // We are not supposed to mark an entire partial type with GeneratedCodeAttribute, therefore
    // FxCop may complain about naming convention violations. These attributes suppress those
    // warnings, but only when the generated code is compiled with /define:CODE_ANALYSIS.
    //
    _out << nl << "[_System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1704\")]";
    _out << nl << "[_System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1707\")]";
    _out << nl << "[_System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1709\")]";
    _out << nl << "[_System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1710\")]";
    _out << nl << "[_System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1711\")]";
    _out << nl << "[_System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1715\")]";
    _out << nl << "[_System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1716\")]";
    _out << nl << "[_System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1720\")]";
    _out << nl << "[_System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1722\")]";
    _out << nl << "[_System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1724\")]";
}

string
Slice::CsVisitor::writeValue(const TypePtr& type)
{
    assert(type);

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindBool:
            {
                return "false";
                break;
            }
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindLong:
            {
                return "0";
                break;
            }
            case Builtin::KindFloat:
            {
                return "0.0f";
                break;
            }
            case Builtin::KindDouble:
            {
                return "0.0";
                break;
            }
            default:
            {
                return "null";
                break;
            }
        }
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        return fixId(en->scoped()) + "." + fixId((*en->enumerators().begin())->name());
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        return st->hasMetaData("cs:class") ? string("null") : "new " + fixId(st->scoped()) + "()";
    }

    return "null";
}

void
Slice::CsVisitor::writeConstantValue(const TypePtr& type, const SyntaxTreeBasePtr& valueType, const string& value)
{
    ConstPtr constant = ConstPtr::dynamicCast(valueType);
    if(constant)
    {
        _out << fixId(constant->scoped()) << ".value";
    }
    else
    {
        BuiltinPtr bp = BuiltinPtr::dynamicCast(type);
        if(bp && bp->kind() == Builtin::KindString)
        {
            _out << "\"" << toStringLiteral(value, "\a\b\f\n\r\t\v\0", "", UCN, 0) << "\"";
        }
        else if(bp && bp->kind() == Builtin::KindLong)
        {
            _out << value << "L";
        }
        else if(bp && bp->kind() == Builtin::KindFloat)
        {
            _out << value << "F";
        }
        else if(EnumPtr::dynamicCast(type))
        {
            EnumeratorPtr lte = EnumeratorPtr::dynamicCast(valueType);
            assert(lte);
            _out << fixId(lte->scoped());
        }
        else
        {
            _out << value;
        }
    }
}

bool
Slice::CsVisitor::requiresDataMemberInitializers(const DataMemberList& members)
{
    for(DataMemberList::const_iterator p = members.begin(); p != members.end(); ++p)
    {
        if((*p)->defaultValueType())
        {
            return true;
        }
        else if((*p)->optional())
        {
            return true;
        }
        else if(BuiltinPtr::dynamicCast((*p)->type()) || StructPtr::dynamicCast((*p)->type()))
        {
            return true;
        }
    }
    return false;
}

void
Slice::CsVisitor::writeDataMemberInitializers(const DataMemberList& members, int baseTypes, bool propertyMapping)
{
    for(DataMemberList::const_iterator p = members.begin(); p != members.end(); ++p)
    {
        if((*p)->defaultValueType())
        {
            _out << nl << "this.";
            if(propertyMapping)
            {
                _out << "_" + (*p)->name();
            }
            else
            {
                _out << fixId((*p)->name(), baseTypes);
            }
            _out << " = ";
            writeConstantValue((*p)->type(), (*p)->defaultValueType(), (*p)->defaultValue());
            _out << ';';
        }
        else if((*p)->optional())
        {
            _out << nl << "this." << fixId((*p)->name(), baseTypes) << " = new " << typeToString((*p)->type(), true)
                 << "();";
        }
        else
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast((*p)->type());
            if(builtin && builtin->kind() == Builtin::KindString)
            {
                _out << nl << "this." << fixId((*p)->name(), baseTypes) << " = \"\";";
            }

            StructPtr st = StructPtr::dynamicCast((*p)->type());
            if(st)
            {
                _out << nl << "this." << fixId((*p)->name(), baseTypes) << " = new " << typeToString(st, false) << "();";
            }
        }
    }
}

string
Slice::CsVisitor::toCsIdent(const string& s)
{
    string::size_type pos = s.find('#');
    if(pos == string::npos)
    {
        return s;
    }

    string result = s;
    if(pos == 0)
    {
        return result.erase(0, 1);
    }

    result[pos] = '.';
    return result;
}

string
Slice::CsVisitor::editMarkup(const string& s)
{
    //
    // Strip HTML markup and javadoc links--VS doesn't display them.
    //
    string result = s;
    string::size_type pos = 0;
    do
    {
        pos = result.find('<', pos);
        if(pos != string::npos)
        {
            string::size_type endpos = result.find('>', pos);
            if(endpos == string::npos)
            {
                break;
            }
            result.erase(pos, endpos - pos + 1);
        }
    }
    while(pos != string::npos);

    const string link = "{@link";
    pos = 0;
    do
    {
        pos = result.find(link, pos);
        if(pos != string::npos)
        {
            result.erase(pos, link.size() + 1); // erase following white space too
            string::size_type endpos = result.find('}', pos);
            if(endpos != string::npos)
            {
                string ident = result.substr(pos, endpos - pos);
                result.erase(pos, endpos - pos + 1);
                result.insert(pos, toCsIdent(ident));
            }
        }
    }
    while(pos != string::npos);

    //
    // Strip @see sections because VS does not display them.
    //
    static const string seeTag = "@see";
    pos = 0;
    do
    {
        //
        // Look for the next @ and delete up to that, or
        // to the end of the string, if not found.
        //
        pos = result.find(seeTag, pos);
        if(pos != string::npos)
        {
            string::size_type next = result.find('@', pos + seeTag.size());
            if(next != string::npos)
            {
                result.erase(pos, next - pos);
            }
            else
            {
                result.erase(pos, string::npos);
            }
        }
    } while(pos != string::npos);

    //
    // Replace @param, @return, and @throws with corresponding <param>, <returns>, and <exception> tags.
    //
    static const string paramTag = "@param";
    pos = 0;
    do
    {
        pos = result.find(paramTag, pos);
        if(pos != string::npos)
        {
            result.erase(pos, paramTag.size() + 1);

            string::size_type startIdent = result.find_first_not_of(" \t", pos);
            if(startIdent != string::npos)
            {
                string::size_type endIdent = result.find_first_of(" \t", startIdent);
                if(endIdent != string::npos)
                {
                    string ident = result.substr(startIdent, endIdent - startIdent);
                    string::size_type endComment = result.find_first_of("@<", endIdent);
                    string comment = result.substr(endIdent + 1,
                                                   endComment == string::npos ? endComment : endComment - endIdent - 1);
                    result.erase(startIdent, endComment == string::npos ? string::npos : endComment - startIdent);
                    string newComment = "<param name=\"" + ident + "\">" + comment + "</param>\n";
                    result.insert(startIdent, newComment);
                    pos = startIdent + newComment.size();
                }
            }
            else
            {
               pos += paramTag.size();
            }
        }
    } while(pos != string::npos);

    static const string returnTag = "@return";
    pos = result.find(returnTag);
    if(pos != string::npos)
    {
        result.erase(pos, returnTag.size() + 1);
        string::size_type endComment = result.find_first_of("@<", pos);
        string comment = result.substr(pos, endComment == string::npos ? endComment : endComment - pos);
        result.erase(pos, endComment == string::npos ? string::npos : endComment - pos);
        string newComment = "<returns>" + comment + "</returns>\n";
        result.insert(pos, newComment);
        pos = pos + newComment.size();
    }

    static const string throwsTag = "@throws";
    pos = 0;
    do
    {
        pos = result.find(throwsTag, pos);
        if(pos != string::npos)
        {
            result.erase(pos, throwsTag.size() + 1);

            string::size_type startIdent = result.find_first_not_of(" \t", pos);
            if(startIdent != string::npos)
            {
                string::size_type endIdent = result.find_first_of(" \t", startIdent);
                if(endIdent != string::npos)
                {
                    string ident = result.substr(startIdent, endIdent - startIdent);
                    string::size_type endComment = result.find_first_of("@<", endIdent);
                    string comment = result.substr(endIdent + 1,
                                                   endComment == string::npos ? endComment : endComment - endIdent - 1);
                    result.erase(startIdent, endComment == string::npos ? string::npos : endComment - startIdent);
                    string newComment = "<exception name=\"" + ident + "\">" + comment + "</exception>\n";
                    result.insert(startIdent, newComment);
                    pos = startIdent + newComment.size();
                }
            }
            else
            {
               pos += throwsTag.size();
            }
        }
    } while(pos != string::npos);

    return result;
}

StringList
Slice::CsVisitor::splitIntoLines(const string& comment)
{
    string s = editMarkup(comment);
    StringList result;
    string::size_type pos = 0;
    string::size_type nextPos;
    while((nextPos = s.find_first_of('\n', pos)) != string::npos)
    {
        result.push_back(string(s, pos, nextPos - pos));
        pos = nextPos + 1;
    }
    string lastLine = string(s, pos);
    if(lastLine.find_first_not_of(" \t\n\r") != string::npos)
    {
        result.push_back(lastLine);
    }
    return result;
}

void
Slice::CsVisitor::splitComment(const ContainedPtr& p, StringList& summaryLines, StringList& remarksLines)
{
    string s = p->comment();
    string summary;
    unsigned int i;
    for(i = 0; i < s.size(); ++i)
    {
        if(s[i] == '.' && (i + 1 >= s.size() || isspace(static_cast<unsigned char>(s[i + 1]))))
        {
            summary += '.';
            ++i;
            break;
        }
        else
        {
            summary += s[i];
        }
    }
    summaryLines = splitIntoLines(summary);
    if(!summaryLines.empty())
    {
        remarksLines = splitIntoLines(trim(s.substr(i)));
    }
}


void
Slice::CsVisitor::writeDocComment(const ContainedPtr& p, const string& deprecateReason, const string& extraParam)
{
    StringList summaryLines;
    StringList remarksLines;
    splitComment(p, summaryLines, remarksLines);

    if(summaryLines.empty())
    {
        if(!deprecateReason.empty())
        {
            _out << nl << "///";
            _out << nl << "/// <summary>" << deprecateReason << "</summary>";
            _out << nl << "///";
        }
        return;
    }

    _out << nl << "/// <summary>";

    for(StringList::const_iterator i = summaryLines.begin(); i != summaryLines.end(); ++i)
    {
        _out << nl << "///";
        if(!(*i).empty())
        {
            _out << " " << *i;
        }
    }

    //
    // We generate everything into the summary tag (despite what the MSDN doc says) because
    // Visual Studio only shows the <summary> text and omits the <remarks> text.
    //
    if(!deprecateReason.empty())
    {
        _out << nl << "///";
        _out << nl << "/// <para>" << deprecateReason << "</para>";
        _out << nl << "///";
    }

    bool summaryClosed = false;

    if(!remarksLines.empty())
    {
        for(StringList::const_iterator i = remarksLines.begin(); i != remarksLines.end(); ++i)
        {
            //
            // The first param, returns, or exception tag ends the description.
            //
            static const string paramTag = "<param";
            static const string returnsTag = "<returns";
            static const string exceptionTag = "<exception";

            if(!summaryClosed &&
               (i->find(paramTag) != string::npos ||
                i->find(returnsTag) != string::npos ||
                i->find(exceptionTag) != string::npos))
            {
                _out << nl << "/// </summary>";
                _out << nl << "/// " << *i;
                summaryClosed = true;
            }
            else
            {
                _out << nl << "///";
                if(!(*i).empty())
                {
                    _out << " " << *i;
                }
            }
        }
    }

    if(!summaryClosed)
    {
        _out << nl << "/// </summary>";
    }

    if(!extraParam.empty())
    {
        _out << nl << "/// " << extraParam;
    }

    _out << sp;
}

void
Slice::CsVisitor::writeDocCommentAMI(const OperationPtr& p, ParamDir paramType, const string& deprecateReason,
                                     const string& extraParam1, const string& extraParam2, const string& extraParam3)
{
    StringList summaryLines;
    StringList remarksLines;
    splitComment(p, summaryLines, remarksLines);

    if(summaryLines.empty() && deprecateReason.empty())
    {
        return;
    }

    //
    // Output the leading comment block up until the first tag.
    //
    _out << nl << "/// <summary>";
    for(StringList::const_iterator i = summaryLines.begin(); i != summaryLines.end(); ++i)
    {
        _out << nl << "///";
        if(!(*i).empty())
        {
            _out << " " << *i;
        }
    }

    bool done = false;
    for(StringList::const_iterator i = remarksLines.begin(); i != remarksLines.end() && !done; ++i)
    {
        string::size_type pos = i->find('<');
        done = true;
        if(pos != string::npos)
        {
            if(pos != 0)
            {
                _out << nl << "/// " << i->substr(0, pos);
            }
        }
        else
        {
            _out << nl << "///";
            if(!(*i).empty())
            {
                _out << " " << *i;
            }
        }
    }
    _out << nl << "/// </summary>";

    //
    // Write the comments for the parameters.
    //
    writeDocCommentParam(p, paramType, false);

    if(!extraParam1.empty())
    {
        _out << nl << "/// " << extraParam1;
    }

    if(!extraParam2.empty())
    {
        _out << nl << "/// " << extraParam2;
    }

    if(!extraParam3.empty())
    {
        _out << nl << "/// " << extraParam3;
    }

    if(paramType == InParam)
    {
        _out << nl << "/// <returns>An asynchronous result object.</returns>";
    }
    else if(p->returnType())
    {
        //
        // Find the comment for the return value (if any).
        //
        static const string returnsTag = "<returns>";
        static const string returnsCloseTag = "</returns>";
        bool doneReturn = false;
        bool foundReturn = false;
        for(StringList::const_iterator i = remarksLines.begin(); i != remarksLines.end() && !doneReturn; ++i)
        {
            if(!foundReturn)
            {
                string::size_type pos = i->find(returnsTag);
                if(pos != string::npos)
                {
                    foundReturn = true;
                    string::size_type endpos = i->find(returnsCloseTag, pos + 1);
                    if(endpos != string::npos)
                    {
                        _out << nl << "/// " << i->substr(pos, endpos - pos + returnsCloseTag.size());
                        doneReturn = true;
                    }
                    else
                    {
                        _out << nl << "/// " << i->substr(pos);
                    }
                }
            }
            else
            {
                string::size_type pos = i->find(returnsCloseTag);
                if(pos != string::npos)
                {
                    _out << nl << "/// " << i->substr(0, pos + returnsCloseTag.size());
                    doneReturn = true;
                }
                else
                {
                    _out << nl << "///";
                    if(!(*i).empty())
                    {
                        _out << " " << *i;
                    }
                }
            }
        }
        if(foundReturn && !doneReturn)
        {
            _out << returnsCloseTag;
        }
    }

    if(!deprecateReason.empty())
    {
        _out << nl << "/// <para>" << deprecateReason << "</para>";
    }
}

void
Slice::CsVisitor::writeDocCommentTaskAsyncAMI(const OperationPtr& p, const string& deprecateReason,
                                              const string& extraParam1, const string& extraParam2,
                                              const string& extraParam3)
{
    StringList summaryLines;
    StringList remarksLines;
    splitComment(p, summaryLines, remarksLines);

    if(summaryLines.empty() && deprecateReason.empty())
    {
        return;
    }

    //
    // Output the leading comment block up until the first tag.
    //
    _out << nl << "/// <summary>";
    for(StringList::const_iterator i = summaryLines.begin(); i != summaryLines.end(); ++i)
    {
        _out << nl << "///";
        if(!(*i).empty())
        {
            _out << " " << *i;
        }
    }

    bool done = false;
    for(StringList::const_iterator i = remarksLines.begin(); i != remarksLines.end() && !done; ++i)
    {
        string::size_type pos = i->find('<');
        done = true;
        if(pos != string::npos)
        {
            if(pos != 0)
            {
                _out << nl << "/// " << i->substr(0, pos);
            }
        }
        else
        {
            _out << nl << "///";
            if(!(*i).empty())
            {
                _out << " " << *i;
            }
        }
    }
    _out << nl << "/// </summary>";

    //
    // Write the comments for the parameters.
    //
    writeDocCommentParam(p, InParam, false);

    if(!extraParam1.empty())
    {
        _out << nl << "/// " << extraParam1;
    }

    if(!extraParam2.empty())
    {
        _out << nl << "/// " << extraParam2;
    }

    if(!extraParam3.empty())
    {
        _out << nl << "/// " << extraParam3;
    }


    _out << nl << "/// <returns>The task object representing the asynchronous operation.</returns>";

    if(!deprecateReason.empty())
    {
        _out << nl << "/// <para>" << deprecateReason << "</para>";
    }
}

void
Slice::CsVisitor::writeDocCommentAMD(const OperationPtr& p, const string& extraParam)
{
    ContainerPtr container = p->container();
    ClassDefPtr contained = ClassDefPtr::dynamicCast(container);
    string deprecateReason = getDeprecateReason(p, contained, "operation");

    StringList summaryLines;
    StringList remarksLines;
    splitComment(p, summaryLines, remarksLines);

    if(summaryLines.empty() && deprecateReason.empty())
    {
        return;
    }

    //
    // Output the leading comment block up until the first tag.
    //
    _out << nl << "/// <summary>";
    for(StringList::const_iterator i = summaryLines.begin(); i != summaryLines.end(); ++i)
    {
        _out << nl << "///";
        if(!(*i).empty())
        {
            _out << " " << *i;
        }
    }

    bool done = false;
    for(StringList::const_iterator i = remarksLines.begin(); i != remarksLines.end() && !done; ++i)
    {
        string::size_type pos = i->find('<');
        done = true;
        if(pos != string::npos)
        {
            if(pos != 0)
            {
                _out << nl << "/// " << i->substr(0, pos);
            }
        }
        else
        {
            _out << nl << "///";
            if(!(*i).empty())
            {
                _out << " " << *i;
            }
        }
    }
    _out << nl << "/// </summary>";

    //
    // Write the comments for the parameters.
    //
    writeDocCommentParam(p, InParam, true);

    if(!extraParam.empty())
    {
        _out << nl << "/// " << extraParam;
    }

    _out << nl << "/// <returns>The task object representing the asynchronous operation.</returns>";

    if(!deprecateReason.empty())
    {
        _out << nl << "/// <para>" << deprecateReason << "</para>";
    }
}

void
Slice::CsVisitor::writeDocCommentParam(const OperationPtr& p, ParamDir paramType, bool amd)
{
    //
    // Collect the names of the in- or -out parameters to be documented.
    //
    ParamDeclList tmp = p->parameters();
    vector<string> params;
    for(ParamDeclList::const_iterator q = tmp.begin(); q != tmp.end(); ++q)
    {
        if((*q)->isOutParam() && paramType == OutParam)
        {
            params.push_back((*q)->name());
        }
        else if(!(*q)->isOutParam() && paramType == InParam)
        {
            params.push_back((*q)->name());
        }
    }

    //
    // Print the comments for all the parameters that appear in the parameter list.
    //
    StringList summaryLines;
    StringList remarksLines;
    splitComment(p, summaryLines, remarksLines);

    const string paramTag = "<param";
    StringList::const_iterator i = remarksLines.begin();
    while(i != remarksLines.end())
    {
        string line = *i++;
        if(line.find(paramTag) != string::npos)
        {
            string::size_type paramNamePos = line.find('"', paramTag.length());
            if(paramNamePos != string::npos)
            {
                string::size_type paramNameEndPos = line.find('"', paramNamePos + 1);
                string paramName = line.substr(paramNamePos + 1, paramNameEndPos - paramNamePos - 1);
                if(std::find(params.begin(), params.end(), paramName) != params.end())
                {
                    _out << nl << "/// " << line;
                    StringList::const_iterator j;
                    if(i == remarksLines.end())
                    {
                        break;
                    }
                    j = i++;
                    while(j != remarksLines.end())
                    {
                        string::size_type endpos = j->find("</param>");
                        if(endpos == string::npos)
                        {
                            i = j;
                            string s = *j++;
                            _out << nl << "///";
                            if(!s.empty())
                            {
                                _out << " " << s;
                            }
                        }
                        else
                        {
                            _out << nl << "/// " << *j++;
                            break;
                        }
                    }
                }
            }
        }
    }
}

Slice::Gen::Gen(const string& base, const vector<string>& includePaths, const string& dir,
                bool tie, bool impl, bool implTie) :
    _includePaths(includePaths),
    _tie(tie)
{
    string fileBase = base;
    string::size_type pos = base.find_last_of("/\\");
    if(pos != string::npos)
    {
        fileBase = base.substr(pos + 1);
    }
    string file = fileBase + ".cs";
    string fileImpl = fileBase + "I.cs";

    if(!dir.empty())
    {
        file = dir + '/' + file;
        fileImpl = dir + '/' + fileImpl;
    }

    _out.open(file.c_str());
    if(!_out)
    {
        ostringstream os;
        os << "cannot open `" << file << "': " << strerror(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }
    FileTracker::instance()->addFile(file);
    printHeader();

    printGeneratedHeader(_out, fileBase + ".ice");

    _out << sp << nl << "using _System = global::System;";

    _out << sp << nl << "#pragma warning disable 1591"; // See bug 3654

    if(impl || implTie)
    {
        IceUtilInternal::structstat st;
        if(!IceUtilInternal::stat(fileImpl, &st))
        {
            ostringstream os;
            os << "`" << fileImpl << "' already exists - will not overwrite";
            throw FileException(__FILE__, __LINE__, os.str());
        }

        _impl.open(fileImpl.c_str());
        if(!_impl)
        {
            ostringstream os;
            os << ": cannot open `" << fileImpl << "': " << strerror(errno);
            throw FileException(__FILE__, __LINE__, os.str());
        }

        FileTracker::instance()->addFile(fileImpl);
    }
}

Slice::Gen::~Gen()
{
    if(_out.isOpen())
    {
        _out << '\n';
    }
    if(_impl.isOpen())
    {
        _impl << '\n';
    }
}

void
Slice::Gen::generate(const UnitPtr& p)
{
    CsGenerator::validateMetaData(p);

    UnitVisitor unitVisitor(_out);
    p->visit(&unitVisitor, false);

    CompactIdVisitor compactIdVisitor(_out);
    p->visit(&compactIdVisitor, false);

    TypesVisitor typesVisitor(_out);
    p->visit(&typesVisitor, false);

    //
    // The async delegates are emitted before the proxy definition
    // because the proxy methods need to know the type.
    //
    AsyncDelegateVisitor asyncDelegateVisitor(_out);
    p->visit(&asyncDelegateVisitor, false);

    ResultVisitor resultVisitor(_out);
    p->visit(&resultVisitor, false);

    ProxyVisitor proxyVisitor(_out);
    p->visit(&proxyVisitor, false);

    OpsVisitor opsVisitor(_out);
    p->visit(&opsVisitor, false);

    HelperVisitor helperVisitor(_out);
    p->visit(&helperVisitor, false);

    DispatcherVisitor dispatcherVisitor(_out, _tie);
    p->visit(&dispatcherVisitor, false);
}

void
Slice::Gen::generateImpl(const UnitPtr& p)
{
    _impl << sp << nl << "using _System = global::System;";
    ImplVisitor implVisitor(_impl);
    p->visit(&implVisitor, false);
}

void
Slice::Gen::generateImplTie(const UnitPtr& p)
{
    _impl << sp << nl << "using _System = global::System;";
    ImplTieVisitor implTieVisitor(_impl);
    p->visit(&implTieVisitor, false);
}

void
Slice::Gen::generateChecksums(const UnitPtr& u)
{
    ChecksumMap map = createChecksums(u);
    if(!map.empty())
    {
        string className = "X" + generateUUID();
        for(string::size_type pos = 1; pos < className.size(); ++pos)
        {
            if(!isalnum(static_cast<unsigned char>(className[pos])))
            {
                className[pos] = '_';
            }
        }

        _out << sp << nl << "namespace IceInternal";
        _out << sb;
        _out << nl << "namespace SliceChecksums";
        _out << sb;
        _out << nl << "[_System.CodeDom.Compiler.GeneratedCodeAttribute(\"slice2cs\", \"" << ICE_STRING_VERSION
             << "\")]";
        _out << nl << "public sealed class " << className;
        _out << sb;
        _out << nl << "public static _System.Collections.Hashtable map = new _System.Collections.Hashtable();";
        _out << sp << nl << "static " << className << "()";
        _out << sb;
        for(ChecksumMap::const_iterator p = map.begin(); p != map.end(); ++p)
        {
            _out << nl << "map.Add(\"" << p->first << "\", \"";
            ostringstream str;
            str.flags(ios_base::hex);
            str.fill('0');
            for(vector<unsigned char>::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
            {
                str << static_cast<int>(*q);
            }
            _out << str.str() << "\");";
        }
        _out << eb;
        _out << eb << ';';
        _out << eb;
        _out << eb;
    }
}

void
Slice::Gen::closeOutput()
{
    _out.close();
    _impl.close();
}

void
Slice::Gen::printHeader()
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

    _out << header;
    _out << "//\n";
    _out << "// Ice version " << ICE_STRING_VERSION << "\n";
    _out << "//\n";
}

Slice::Gen::UnitVisitor::UnitVisitor(IceUtilInternal::Output& out) :
    CsVisitor(out)
{
}

bool
Slice::Gen::UnitVisitor::visitUnitStart(const UnitPtr& p)
{
    DefinitionContextPtr dc = p->findDefinitionContext(p->topLevelFile());
    assert(dc);
    StringList globalMetaData = dc->getMetaData();

    static const string attributePrefix = "cs:attribute:";

    bool sep = false;
    for(StringList::const_iterator q = globalMetaData.begin(); q != globalMetaData.end(); ++q)
    {
        string::size_type pos = q->find(attributePrefix);
        if(pos == 0 && q->size() > attributePrefix.size())
        {
            if(!sep)
            {
                _out << sp;
                sep = true;
            }
            string attrib = q->substr(pos + attributePrefix.size());
            _out << nl << '[' << attrib << ']';
        }
    }
    return false;
}

Slice::Gen::CompactIdVisitor::CompactIdVisitor(IceUtilInternal::Output& out) :
    CsVisitor(out)
{
}

bool
Slice::Gen::CompactIdVisitor::visitUnitStart(const UnitPtr&)
{
    _out << sp << nl << "namespace IceCompactId";
    _out << sb;
    return true;
}

void
Slice::Gen::CompactIdVisitor::visitUnitEnd(const UnitPtr&)
{
    _out << eb;
}

bool
Slice::Gen::CompactIdVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->compactId() >= 0)
    {
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public sealed class TypeId_" << p->compactId();
        _out << sb;
        _out << nl << "public const string typeId = \"" << p->scoped() << "\";";
        _out << eb;
    }
    return false;
}

Slice::Gen::TypesVisitor::TypesVisitor(IceUtilInternal::Output& out) :
    CsVisitor(out)
{
}

bool
Slice::Gen::TypesVisitor::visitModuleStart(const ModulePtr& p)
{
    DictionaryList dicts;
    if(p->hasOnlyDictionaries(dicts))
    {
        //
        // If this module contains only dictionaries, we don't need to generate
        // anything for the dictionary types. The early return prevents
        // an empty namespace from being emitted, the namespace will
        // be emitted later by the dictionary helper .
        //
        return false;
    }

    string name = fixId(p->name());
    _out << sp;
    emitAttributes(p);
    _out << nl << "namespace " << name;

    _out << sb;

    return true;
}

void
Slice::Gen::TypesVisitor::visitModuleEnd(const ModulePtr&)
{
    _out << eb;
}

bool
Slice::Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = p->name();
    string scoped = fixId(p->scoped());
    ClassList bases = p->bases();
    bool hasBaseClass = !bases.empty() && !bases.front()->isInterface();

    StringList baseNames;

    _out << sp;
    emitAttributes(p);

    if(p->isDelegate())
    {
        emitComVisibleAttribute();
        OperationPtr o = p->allOperations().front();
        _out << nl << "public delegate " << typeToString(o->returnType(), o->returnIsOptional()) << " ";
        _out << fixId(name) << spar << getParams(o) << epar << ";";
        return false;
    }

    if(p->isInterface())
    {
        emitComVisibleAttribute();
        emitPartialTypeAttributes();
        _out << nl << "public partial interface " << fixId(name);
        if(!p->isLocal())
        {
            baseNames.push_back("Ice.Object");
            baseNames.push_back(name + "Operations_");
        }
        for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
        {
            baseNames.push_back(fixId((*q)->scoped()));
        }
    }
    else
    {
        emitComVisibleAttribute();
        emitPartialTypeAttributes();
        _out << nl << "[_System.Serializable]";
        if(p->allOperations().size() > 0) // See bug 4747
        {
            _out << nl << "[_System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Design\", \"CA1012\")]";
        }
        _out << nl << "public ";
        if(p->isLocal() && p->allOperations().size() > 0) // Don't use isAbstract() here - see bug 3739
        {
            _out << "abstract ";
        }
        _out << "partial class " << fixId(name);

        if(!hasBaseClass)
        {
            if(!p->isLocal())
            {
                baseNames.push_back("Ice.Value");
            }
        }
        else
        {
            baseNames.push_back(fixId(bases.front()->scoped()));
            bases.pop_front();
        }

        if(p->isLocal())
        {
            for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
            {
                if((*q)->isAbstract())
                {
                    baseNames.push_back(fixId((*q)->scoped()));
                }
            }
        }
    }

    //
    // Check for cs:implements metadata.
    //
    const StringList metaData = p->getMetaData();
    static const string prefix = "cs:implements:";
    for(StringList::const_iterator q = metaData.begin(); q != metaData.end(); ++q)
    {
        if(q->find(prefix) == 0)
        {
            baseNames.push_back(q->substr(prefix.size()));
        }
    }

    if(!baseNames.empty())
    {
        _out << " : ";
        for(StringList::const_iterator q = baseNames.begin(); q != baseNames.end(); ++q)
        {
            if(q != baseNames.begin())
            {
                _out << ", ";
            }
            _out << *q;
        }
    }

    _out << sb;

    if(!p->isInterface())
    {
        if(p->hasDataMembers() && (!p->hasOperations() || !p->isLocal()))
        {
            _out << sp << nl << "#region Slice data members";
        }
        else if(p->hasDataMembers())
        {
            _out << sp << nl << "#region Slice data members and operations";
        }
        else if(p->hasOperations() && p->isLocal())
        {
            _out << sp << nl << "#region Slice operations";
        }
    }
    else
    {
        if(p->isLocal() && p->hasOperations())
        {
            _out << sp << nl << "#region Slice operations";
        }
    }

    return true;
}

void
Slice::Gen::TypesVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string name = fixId(p->name());
    DataMemberList classMembers = p->classDataMembers();
    DataMemberList allClassMembers = p->allClassDataMembers();
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList allDataMembers = p->allDataMembers();
    ClassList bases = p->bases();
    bool hasBaseClass = !bases.empty() && !bases.front()->isInterface();

    if(!p->isInterface())
    {
        if(p->hasDataMembers() || (p->hasOperations() && p->isLocal()))
        {
            _out << sp << nl << "#endregion";
        }

        if(!allDataMembers.empty())
        {
            const bool isAbstract = p->isLocal() && p->isAbstract();
            const bool propertyMapping = p->hasMetaData("cs:property");

            _out << sp << nl << "#region Constructors";

            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << (isAbstract ? "protected " : "public ") << name << spar << epar;
            if(hasBaseClass)
            {
                _out << " : base()";
            }
            _out << sb;
            writeDataMemberInitializers(dataMembers, DotNet::ICloneable, propertyMapping);
            _out << eb;

            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << (isAbstract ? "protected " : "public ") << name << spar;
            vector<string> paramDecl;
            for(DataMemberList::const_iterator d = allDataMembers.begin(); d != allDataMembers.end(); ++d)
            {
                string memberName = fixId((*d)->name(), DotNet::ICloneable);
                string memberType = typeToString((*d)->type(), (*d)->optional(), p->isLocal(), (*d)->getMetaData());
                paramDecl.push_back(memberType + " " + memberName);
            }
            _out << paramDecl << epar;
            if(hasBaseClass && allDataMembers.size() != dataMembers.size())
            {
                _out << " : base" << spar;
                vector<string> baseParamNames;
                DataMemberList baseDataMembers = bases.front()->allDataMembers();
                for(DataMemberList::const_iterator d = baseDataMembers.begin(); d != baseDataMembers.end(); ++d)
                {
                    baseParamNames.push_back(fixId((*d)->name(), DotNet::ICloneable));
                }
                _out << baseParamNames << epar;
            }
            _out << sb;
            for(DataMemberList::const_iterator d = dataMembers.begin(); d != dataMembers.end(); ++d)
            {
                _out << nl << "this.";
                const string paramName = fixId((*d)->name(), DotNet::ICloneable);
                if(propertyMapping)
                {
                    _out << "_" + (*d)->name();
                }
                else
                {
                    _out << paramName;
                }
                _out << " = " << paramName << ';';
            }
            _out << eb;

            _out << sp << nl << "#endregion"; // Constructors
        }

        if(p->isLocal())
        {
            writeInheritedOperations(p);
        }
    }
    else
    {
        if(p->isLocal() && p->hasOperations())
        {
            _out << sp << nl << "#endregion"; // Slice operations"
        }
    }

    if(!p->isInterface() && !p->isLocal())
    {
        _out << sp;
        _out << nl << "private const string _id = \""
             << p->scoped() << "\";";

        _out << sp;
        _out << nl << "public static new string ice_staticId()";
        _out << sb;
        _out << nl << "return _id;";
        _out << eb;

        _out << nl << "public override string ice_id()";
        _out << sb;
        _out << nl << "return _id;";
        _out << eb;

        writeMarshaling(p);
    }

    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitOperation(const OperationPtr& p)
{
    ClassDefPtr cl = ClassDefPtr::dynamicCast(p->container());
    bool isLocal = cl->isLocal();
    bool isInterface = cl->isInterface();

    if(isLocal)
    {
        string name = fixId(p->name(), DotNet::ICloneable, true);
        TypePtr ret = p->returnType();
        string retS = typeToString(ret, p->returnIsOptional(), true);

        _out << sp;
        if(isInterface)
        {
            _out << sp;
        }

        writeDocComment(p, getDeprecateReason(p, cl, "operation"));

        emitAttributes(p);
        emitDeprecate(p, cl, _out, "operation");
        emitGeneratedCodeAttribute();
        _out << nl;
        if(!isInterface)
        {
            _out << "public abstract ";
        }
        _out << retS << " " << name << spar << getParams(p) << epar << ";";

        if(cl->hasMetaData("async-oneway") || p->hasMetaData("async-oneway"))
        {
            vector<string> inParams = getInParams(p);
            ParamDeclList inParamDecls = p->inParameters();

            //
            // Task based asynchronous methods
            //
            _out << sp;
            emitAttributes(p);
            emitGeneratedCodeAttribute();
            _out << nl;
            if(!isInterface)
            {
                _out << "public abstract ";
            }
            _out << taskResultType(p);

            string progress = getEscapedParamName(p, "progress");
            string cancel = getEscapedParamName(p, "cancel");

            _out << " " << name << "Async" << spar << inParams
                 << ("_System.IProgress<bool> " + progress + " = null")
                 << ("_System.Threading.CancellationToken " + cancel + " = new _System.Threading.CancellationToken()")
                 << epar << ";";

            //
            // IAsyncResult based asynchronous mehtods
            //
            _out << sp;
            emitAttributes(p);
            emitGeneratedCodeAttribute();
            _out << nl;
            if(!isInterface)
            {
                _out << "public abstract ";
            }
            _out << "Ice.AsyncResult begin_" << name << spar << inParams
                 << "Ice.AsyncCallback " + getEscapedParamName(p, "callback") + " = null"
                 << "object " + getEscapedParamName(p, "cookie") + " = null" << epar << ';';

            _out << sp;
            emitAttributes(p);
            emitGeneratedCodeAttribute();
            _out << nl;
            if(!isInterface)
            {
                _out << "public abstract ";
            }
            _out << retS << " end_" << name << spar << getOutParams(p, false, true)
                 << "Ice.AsyncResult " + getEscapedParamName(p, "asyncResult") << epar << ';';
        }
    }
}

void
Slice::Gen::TypesVisitor::visitSequence(const SequencePtr& p)
{
    //
    // No need to generate anything for sequences.
    //
}

bool
Slice::Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string name = fixId(p->name());
    ExceptionPtr base = p->base();

    _out << sp;
    writeDocComment(p, getDeprecateReason(p, 0, "type"));
    emitDeprecate(p, 0, _out, "type");
    emitAttributes(p);
    emitComVisibleAttribute();
    //
    // Suppress FxCop diagnostic about a missing constructor MyException(String).
    //
    _out << nl << "[_System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Design\", \"CA1032\")]";
    _out << nl << "[_System.Serializable]";

    emitPartialTypeAttributes();
    _out << nl << "public partial class " << name << " : ";
    if(base)
    {
        _out << fixId(base->scoped());
    }
    else
    {
        _out << (p->isLocal() ? "Ice.LocalException" : "Ice.UserException");
    }
    _out << sb;

    if(!p->dataMembers().empty())
    {
        _out << sp << nl << "#region Slice data members";
    }

    return true;
}

void
Slice::Gen::TypesVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    string name = fixId(p->name());

    DataMemberList allDataMembers = p->allDataMembers();
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList allClassMembers = p->allClassDataMembers();
    DataMemberList classMembers = p->classDataMembers();
    DataMemberList optionalMembers = p->orderedOptionalDataMembers();

    vector<string> allParamDecl;
    for(DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name());
        string memberType = typeToString((*q)->type(), (*q)->optional());
        allParamDecl.push_back(memberType + " " + memberName);
    }

    vector<string> paramNames;
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        paramNames.push_back(fixId((*q)->name()));
    }

    vector<string> paramDecl;
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name());
        string memberType = typeToString((*q)->type(), (*q)->optional());
        paramDecl.push_back(memberType + " " + memberName);
    }

    vector<string> baseParamNames;
    DataMemberList baseDataMembers;

    if(p->base())
    {
        baseDataMembers = p->base()->allDataMembers();
        for(DataMemberList::const_iterator q = baseDataMembers.begin(); q != baseDataMembers.end(); ++q)
        {
            baseParamNames.push_back(fixId((*q)->name()));
        }
    }

    if(!dataMembers.empty())
    {
        _out << sp << nl << "#endregion"; // Slice data members
    }

    _out << sp << nl << "#region Constructors";

    const bool hasDataMemberInitializers = requiresDataMemberInitializers(dataMembers);
    if(hasDataMemberInitializers)
    {
        _out << sp << nl << "private void _initDM()";
        _out << sb;
        writeDataMemberInitializers(dataMembers, DotNet::Exception);
        _out << eb;
    }

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public " << name << "()";
    _out << sb;
    if(hasDataMemberInitializers)
    {
        _out << nl << "_initDM();";
    }
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public " << name << "(_System.Exception ex) : base(ex)";
    _out << sb;
    if(hasDataMemberInitializers)
    {
        _out << nl << "_initDM();";
    }
    _out << eb;
    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public " << name << "(_System.Runtime.Serialization.SerializationInfo info, "
         << "_System.Runtime.Serialization.StreamingContext context) : base(info, context)";
    _out << sb;
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string name = fixId((*q)->name(), DotNet::Exception, false);
        writeSerializeDeserializeCode(_out, (*q)->type(), name, (*q)->optional(), (*q)->tag(), false);
    }
    _out << eb;

    if(!allDataMembers.empty())
    {
        if(!dataMembers.empty())
        {
            _out << sp << nl << "private void _initDM" << spar << paramDecl << epar;
            _out << sb;
            for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
            {
                string name = fixId((*q)->name(), DotNet::Exception, false);
                _out << nl << "this." << name << " = " << fixId((*q)->name()) << ';';
            }
            _out << eb;
        }

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public " << name << spar << allParamDecl << epar;
        if(p->base() && allDataMembers.size() != dataMembers.size())
        {
            _out << " : base" << spar << baseParamNames << epar;
        }
        _out << sb;
        if(!dataMembers.empty())
        {
            _out << nl << "_initDM" << spar << paramNames << epar << ';';
        }
        _out << eb;

        string exParam = getEscapedParamName(allDataMembers, "ex");
        vector<string> exceptionParam;
        exceptionParam.push_back(exParam);
        vector<string> exceptionDecl;
        exceptionDecl.push_back("_System.Exception " + exParam);
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public " << name << spar << allParamDecl << exceptionDecl << epar << " : base" << spar;
        if(p->base() && allDataMembers.size() != dataMembers.size())
        {
            _out << baseParamNames;
        }
        _out << exceptionParam << epar;
        _out << sb;
        if(!dataMembers.empty())
        {
            _out << nl << "_initDM" << spar << paramNames << epar << ';';
        }
        _out << eb;
    }

    _out << sp << nl << "#endregion"; // Constructors

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public override string ice_id()";
    _out << sb;
    _out << nl << "return \"" << p->scoped() << "\";";
    _out << eb;

    _out << sp << nl << "#region Object members";

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public override int GetHashCode()";
    _out << sb;
    if(p->base())
    {
        _out << nl << "int h_ = base.GetHashCode();";
    }
    else
    {
        _out << nl << "int h_ = 5381;";
    }
    _out << nl << "IceInternal.HashUtil.hashAdd(ref h_, \"" << p->scoped() << "\");";
    writeMemberHashCode(dataMembers, DotNet::Exception);
    _out << nl << "return h_;";
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public override bool Equals(object other)";
    _out << sb;
    _out << nl << "if(other == null)";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    _out << nl << "if(object.ReferenceEquals(this, other))";
    _out << sb;
    _out << nl << "return true;";
    _out << eb;
    _out << nl << name << " o = other as " << name << ";";
    _out << nl << "if(o == null)";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    if(p->base())
    {
        _out << nl << "if(!base.Equals(other))";
        _out << sb;
        _out << nl << "return false;";
        _out << eb;
    }
    writeMemberEquals(dataMembers, DotNet::Exception);
    _out << nl << "return true;";
    _out << eb;

    if(!dataMembers.empty())
    {
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public override void GetObjectData(_System.Runtime.Serialization.SerializationInfo info, "
             << "_System.Runtime.Serialization.StreamingContext context)";
        _out << sb;
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            string name = fixId((*q)->name(), DotNet::Exception, false);
            writeSerializeDeserializeCode(_out, (*q)->type(), name, (*q)->optional(), (*q)->tag(), true);
        }
        _out << sp << nl << "base.GetObjectData(info, context);";
        _out << eb;
    }

    _out << sp << nl << "#endregion"; // Object members

    _out << sp << nl << "#region Comparison members";

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public static bool operator==(" << name << " lhs, " << name << " rhs)";
    _out << sb;
    _out << nl << "return Equals(lhs, rhs);";
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public static bool operator!=(" << name << " lhs, " << name << " rhs)";
    _out << sb;
    _out << nl << "return !Equals(lhs, rhs);";
    _out << eb;

    _out << sp << nl << "#endregion"; // Comparison members

    if(!p->isLocal())
    {
        _out << sp << nl << "#region Marshaling support";

        string scoped = p->scoped();
        ExceptionPtr base = p->base();

        const bool basePreserved = p->inheritsMetaData("preserve-slice");
        const bool preserved = p->hasMetaData("preserve-slice");

        if(preserved && !basePreserved)
        {
            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << "public override void iceWrite(Ice.OutputStream ostr_)";
            _out << sb;
            _out << nl << "ostr_.startException(slicedData_);";
            _out << nl << "iceWriteImpl(ostr_);";
            _out << nl << "ostr_.endException();";
            _out << eb;

            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << "public override void iceRead(Ice.InputStream istr_)";
            _out << sb;
            _out << nl << "istr_.startException();";
            _out << nl << "iceReadImpl(istr_);";
            _out << nl << "slicedData_ = istr_.endException(true);";
            _out << eb;
        }

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "protected override void iceWriteImpl(Ice.OutputStream ostr_)";
        _out << sb;
        _out << nl << "ostr_.startSlice(\"" << scoped << "\", -1, " << (!base ? "true" : "false") << ");";
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            writeMarshalDataMember(*q, fixId((*q)->name(), DotNet::Exception));
        }
        _out << nl << "ostr_.endSlice();";
        if(base)
        {
            _out << nl << "base.iceWriteImpl(ostr_);";
        }
        _out << eb;

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "protected override void iceReadImpl(Ice.InputStream istr_)";
        _out << sb;
        _out << nl << "istr_.startSlice();";

        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            writeUnmarshalDataMember(*q, fixId((*q)->name(), DotNet::Exception));
        }
        _out << nl << "istr_.endSlice();";
        if(base)
        {
            _out << nl << "base.iceReadImpl(istr_);";
        }
        _out << eb;

        if((!base || (base && !base->usesClasses(false))) && p->usesClasses(false))
        {
            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << "public override bool iceUsesClasses()";
            _out << sb;
            _out << nl << "return true;";
            _out << eb;
        }

        if(preserved && !basePreserved)
        {
            _out << sp << nl << "protected Ice.SlicedData slicedData_;";
        }

        _out << sp << nl << "#endregion"; // Marshalling support
    }

    _out << eb;
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    string name = fixId(p->name());

    _out << sp;

    emitDeprecate(p, 0, _out, "type");

    emitAttributes(p);
    emitPartialTypeAttributes();
    _out << nl << "[_System.Serializable]";
    _out << nl << "public partial " << (isValueType(p) ? "struct" : "class") << ' ' << name;

    StringList baseNames;
    if(!isValueType(p))
    {
        baseNames.push_back("_System.ICloneable");
    }

    //
    // Check for cs:implements metadata.
    //
    const StringList metaData = p->getMetaData();
    static const string prefix = "cs:implements:";
    for(StringList::const_iterator q = metaData.begin(); q != metaData.end(); ++q)
    {
        if(q->find(prefix) == 0)
        {
            baseNames.push_back(q->substr(prefix.size()));
        }
    }

    if(!baseNames.empty())
    {
        _out << " : ";
        for(StringList::const_iterator q = baseNames.begin(); q != baseNames.end(); ++q)
        {
            if(q != baseNames.begin())
            {
                _out << ", ";
            }
            _out << *q;
        }
    }

    _out << sb;

    _out << sp << nl << "#region Slice data members";

    return true;
}

void
Slice::Gen::TypesVisitor::visitStructEnd(const StructPtr& p)
{
    string name = fixId(p->name());

    DataMemberList classMembers = p->classDataMembers();
    DataMemberList dataMembers = p->dataMembers();

    const bool propertyMapping = p->hasMetaData("cs:property");

    _out << sp << nl << "#endregion"; // Slice data members

    const bool isClass = !isValueType(p);

    _out << sp << nl << "#region Constructor";
    if(isClass)
    {
        //
        // Default values for struct data members are only generated if the struct
        // is mapped to a C# class. We cannot generate a parameterless constructor
        // or assign default values to data members if the struct maps to a value
        // type (a C# struct) instead.
        //
        _out << "s";
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public " << name << "()";
        _out << sb;
        writeDataMemberInitializers(dataMembers, DotNet::ICloneable, propertyMapping);
        _out << eb;
    }

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public " << name << spar;
    vector<string> paramDecl;
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name(), isClass ? DotNet::ICloneable : 0);
        string memberType = typeToString((*q)->type(), false, p->isLocal());
        paramDecl.push_back(memberType + " " + memberName);
    }
    _out << paramDecl << epar;
    _out << sb;
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string paramName = fixId((*q)->name(), isClass ? DotNet::ICloneable : 0);
        _out << nl << "this.";
        if(propertyMapping)
        {
            _out << "_" + (*q)->name();
        }
        else
        {
            _out << paramName;
        }
        _out << " = " << paramName << ';';
    }
    _out << eb;

    _out << sp << nl << "#endregion"; // Constructor(s)

    if(isClass)
    {
        _out << sp << nl << "#region ICloneable members";

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public object Clone()";
        _out << sb;
        _out << nl << "return MemberwiseClone();";
        _out << eb;

        _out << sp << nl << "#endregion"; // ICloneable members
    }

    _out << sp << nl << "#region Object members";

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public override int GetHashCode()";
    _out << sb;
    _out << nl << "int h_ = 5381;";
    _out << nl << "IceInternal.HashUtil.hashAdd(ref h_, \"" << p->scoped() << "\");";
    writeMemberHashCode(dataMembers, isClass ? DotNet::ICloneable : 0);
    _out << nl << "return h_;";
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public override bool Equals(object other)";
    _out << sb;
    if(isClass)
    {
        _out << nl << "if(object.ReferenceEquals(this, other))";
        _out << sb;
        _out << nl << "return true;";
        _out << eb;
    }
    if(isClass)
    {
        _out << nl << "if(other == null)";
        _out << sb;
        _out << nl << "return false;";
        _out << eb;
        _out << nl << "if(GetType() != other.GetType())";
    }
    else
    {
        _out << nl << "if(!(other is " << name << "))";
    }
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    if(!dataMembers.empty())
    {
        _out << nl << name << " o = (" << name << ")other;";
    }
    writeMemberEquals(dataMembers, isClass ? DotNet::ICloneable : 0);
    _out << nl << "return true;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Object members

    _out << sp << nl << "#region Comparison members";

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public static bool operator==(" << name << " lhs, " << name << " rhs)";
    _out << sb;
    _out << nl << "return Equals(lhs, rhs);";
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public static bool operator!=(" << name << " lhs, " << name << " rhs)";
    _out << sb;
    _out << nl << "return !Equals(lhs, rhs);";
    _out << eb;

    _out << sp << nl << "#endregion"; // Comparison members

    if(!p->isLocal())
    {
        _out << sp << nl << "#region Marshaling support";

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public void ice_writeMembers(Ice.OutputStream ostr)";
        _out << sb;
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            writeMarshalDataMember(*q, fixId(*q, isClass ? DotNet::ICloneable : 0), true);
        }
        _out << eb;

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public void ice_readMembers(Ice.InputStream istr)";
        _out << sb;
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            writeUnmarshalDataMember(*q, fixId(*q, isClass ? DotNet::ICloneable : 0), true);
        }
        _out << eb;

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public static void ice_write(Ice.OutputStream ostr, " << name << " v)";
        _out << sb;
        if(isClass)
        {
            _out << nl << "if(v == null)";
            _out << sb;
            _out << nl << "_nullMarshalValue.ice_writeMembers(ostr);";
            _out << eb;
            _out << nl << "else";
            _out << sb;
            _out << nl << "v.ice_writeMembers(ostr);";
            _out << eb;
        }
        else
        {
            _out << nl << "v.ice_writeMembers(ostr);";
        }
        _out << eb;

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public static " << name << " ice_read(Ice.InputStream istr)";
        _out << sb;
        _out << nl << "var v = new " << name << "();";
        _out << nl << "v.ice_readMembers(istr);";
        _out << nl << "return v;";
        _out << eb;

        if(isClass)
        {
            _out << sp << nl << "private static readonly " << name << " _nullMarshalValue = new " << name << "();";
        }
        _out << sp << nl << "#endregion"; // Marshalling support
    }

    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitDictionary(const DictionaryPtr& p)
{
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string name = fixId(p->name());
    string scoped = fixId(p->scoped());
    EnumeratorList enumerators = p->enumerators();
    const bool explicitValue = p->explicitValue();

    _out << sp;
    emitDeprecate(p, 0, _out, "type");
    emitAttributes(p);
    emitGeneratedCodeAttribute();
    _out << nl << "public enum " << name;
    _out << sb;
    for(EnumeratorList::const_iterator en = enumerators.begin(); en != enumerators.end(); ++en)
    {
        if(en != enumerators.begin())
        {
            _out << ',';
        }
        _out << nl << fixId((*en)->name());
        if(explicitValue)
        {
            _out << " = " << (*en)->value();
        }
    }
    _out << eb;

    if(!p->isLocal())
    {
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public sealed class " << p->name() << "Helper";
        _out << sb;
        _out << sp;
        _out << nl << "public static void write(Ice.OutputStream ostr, " << name << " v)";
        _out << sb;
        writeMarshalUnmarshalCode(_out, p, "v", true);
        _out << eb;

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public static " << name << " read(Ice.InputStream istr)";
        _out << sb;
        _out << nl << name << " v;";
        writeMarshalUnmarshalCode(_out, p, "v", false);
        _out << nl << "return v;";
        _out << eb;

        _out << eb;
    }
}

void
Slice::Gen::TypesVisitor::visitConst(const ConstPtr& p)
{
    string name = fixId(p->name());
    _out << sp;
    emitAttributes(p);
    emitGeneratedCodeAttribute();
    _out << nl << "public abstract class " << name;
    _out << sb;
    _out << sp << nl << "public const " << typeToString(p->type()) << " value = ";
    writeConstantValue(p->type(), p->valueType(), p->value());
    _out << ";";
    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitDataMember(const DataMemberPtr& p)
{
    int baseTypes = 0;
    bool isClass = false;
    bool isProperty = false;
    bool isValue = false;
    bool isProtected = false;
    const bool isOptional = p->optional();
    ContainedPtr cont = ContainedPtr::dynamicCast(p->container());
    assert(cont);

    bool isLocal;
    StructPtr st = StructPtr::dynamicCast(cont);
    ExceptionPtr ex = ExceptionPtr::dynamicCast(cont);
    ClassDefPtr cl = ClassDefPtr::dynamicCast(cont);
    if(st)
    {
        isLocal = st->isLocal();
        isValue = isValueType(StructPtr::dynamicCast(cont));
        if(!isValue)
        {
            baseTypes = DotNet::ICloneable;
        }
        if(cont->hasMetaData("cs:property"))
        {
            isProperty = true;
        }
    }
    else if(ex)
    {
        isLocal = ex->isLocal();
        baseTypes = DotNet::Exception;
    }
    else
    {
        assert(cl);
        isLocal = cl->isLocal();
        baseTypes = DotNet::ICloneable;
        isClass = true;
        if(cont->hasMetaData("cs:property"))
        {
            isProperty = true;
        }
        isProtected = cont->hasMetaData("protected") || p->hasMetaData("protected");
    }

    _out << sp;

    emitDeprecate(p, cont, _out, "member");

    string type = typeToString(p->type(), isOptional, isLocal, p->getMetaData());
    string propertyName = fixId(p->name(), baseTypes, isClass);
    string dataMemberName;
    if(isProperty)
    {
        dataMemberName = "_" + p->name();
    }
    else
    {
        dataMemberName = propertyName;
    }

    if(isProperty)
    {
        _out << nl << "private";
    }
    else if(isProtected)
    {
        emitAttributes(p);
        emitGeneratedCodeAttribute();
        _out << nl << "protected";
    }
    else
    {
        emitAttributes(p);
        emitGeneratedCodeAttribute();
        _out << nl << "public";
    }

    if(isOptional && isValue)
    {
        _out << ' ' << type << ' ' << dataMemberName << " = new " << type << "();";
    }
    else
    {
        _out << ' ' << type << ' ' << dataMemberName << ';';
    }

    if(isProperty)
    {
        emitAttributes(p);
        emitGeneratedCodeAttribute();
        _out << nl << (isProtected ? "protected" : "public");
        if(!isValue)
        {
            _out << " virtual";
        }
        _out << ' ' << type << ' ' << propertyName;
        _out << sb;
        _out << nl << "get";
        _out << sb;
        _out << nl << "return " << dataMemberName << ';';
        _out << eb;
        _out << nl << "set";
        _out << sb;
        _out << nl << dataMemberName << " = value;";
        _out << eb;
        _out << eb;
    }
}

void
Slice::Gen::TypesVisitor::writeMemberHashCode(const DataMemberList& dataMembers, int baseTypes)
{
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        _out << nl << "IceInternal.HashUtil.hashAdd(ref h_, " << fixId((*q)->name(), baseTypes);
        if((*q)->optional())
        {
            _out << ".Value";
        }
        _out << ");";
    }
}

void
Slice::Gen::TypesVisitor::writeMemberEquals(const DataMemberList& dataMembers, int baseTypes)
{
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name(), baseTypes);
        TypePtr memberType = (*q)->type();
        if(!(*q)->optional() && !isValueType(memberType))
        {
            _out << nl << "if(this." << memberName << " == null)";
            _out << sb;
            _out << nl << "if(o." << memberName << " != null)";
            _out << sb;
            _out << nl << "return false;";
            _out << eb;
            _out << eb;
            _out << nl << "else";
            _out << sb;
            SequencePtr seq = SequencePtr::dynamicCast(memberType);
            if(seq)
            {
                string meta;
                bool isSerializable = seq->findMetaData("cs:serializable:", meta);
                bool isGeneric = seq->findMetaData("cs:generic:", meta);
                bool isArray = !isSerializable && !isGeneric;
                if(isArray)
                {
                    //
                    // Equals() for native arrays does not have value semantics.
                    //
                    _out << nl << "if(!IceUtilInternal.Arrays.Equals(this." << memberName << ", o." << memberName << "))";
                }
                else if(isGeneric)
                {
                    //
                    // Equals() for generic types does not have value semantics.
                    //
                    _out << nl << "if(!IceUtilInternal.Collections.SequenceEquals(this." << memberName << ", o."
                         << memberName << "))";
                }
            }
            else
            {
                DictionaryPtr dict = DictionaryPtr::dynamicCast(memberType);
                if(dict)
                {
                    //
                    // Equals() for generic types does not have value semantics.
                    //
                    _out << nl << "if(!IceUtilInternal.Collections.DictionaryEquals(this." << memberName << ", o."
                            << memberName << "))";
                }
                else
                {
                    _out << nl << "if(!this." << memberName << ".Equals(o." << memberName << "))";
                }
            }
            _out << sb;
            _out << nl << "return false;";
            _out << eb;
            _out << eb;
        }
        else
        {
            _out << nl << "if(!this." << memberName << ".Equals(o." << memberName << "))";
            _out << sb;
            _out << nl << "return false;";
            _out << eb;
        }
    }
}

Slice::Gen::ResultVisitor::ResultVisitor(::IceUtilInternal::Output& out)
    : CsVisitor(out)
{
}

bool
Slice::Gen::ResultVisitor::visitModuleStart(const ModulePtr& p)
{
    ClassList classes = p->classes();
    for(ClassList::const_iterator i = classes.begin(); i != classes.end(); ++i)
    {
        ClassDefPtr cl = *i;
        OperationList operations = cl->operations();
        for(OperationList::const_iterator j = operations.begin(); j != operations.end(); ++j)
        {
            OperationPtr op = *j;
            ParamDeclList outParams = op->outParameters();
            TypePtr ret = op->returnType();
            if(outParams.size() > 1 || (ret && outParams.size() > 0))
            {
                _out << sp << nl << "namespace " << fixId(p->name());
                _out << sb;
                return true;
            }
        }
    }
    return false;
}

void
Slice::Gen::ResultVisitor::visitModuleEnd(const ModulePtr&)
{
    _out << eb;
}

bool
Slice::Gen::ResultVisitor::visitClassDefStart(const ClassDefPtr&)
{
    return true;
}

void
Slice::Gen::ResultVisitor::visitClassDefEnd(const ClassDefPtr&)
{
}

void
Slice::Gen::ResultVisitor::visitOperation(const OperationPtr& p)
{
    ClassDefPtr cl = ClassDefPtr::dynamicCast(p->container());
    ParamDeclList outParams = p->outParameters();
    TypePtr ret = p->returnType();

    if(outParams.size() > 1 || (ret && outParams.size() > 0))
    {
        string name = resultStructName(cl->name(), p->name());

        string retS;
        string retSName;
        if(ret)
        {
            retS = typeToString(ret, p->returnIsOptional());
            retSName = resultStructReturnValueName(outParams);
        }

        _out << sp;
        _out << nl << "public struct " << name;
        _out << sb;

        //
        // One shot constructor
        //
        _out << nl << "public " << name << spar;
        if(ret)
        {
            _out << (retS + " " + retSName);
        }
        for(ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end(); ++i)
        {
            _out << (typeToString((*i)->type(), (*i)->optional()) + " " + fixId((*i)->name()));
        }
        _out << epar;

        _out << sb;

        if(ret)
        {
            _out << nl << "this." << retSName << " = " << retSName << ";";
        }

        for(ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end(); ++i)
        {
            _out << nl << "this." << fixId((*i)->name()) << " = " << fixId((*i)->name()) << ";";
        }

        _out << eb;

        //
        // Data members
        //
        _out << sp;
        if(ret)
        {
            _out << nl << "public " << retS << " " << retSName << ";";
        }

        for(ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end(); ++i)
        {
            _out << nl << "public " << typeToString((*i)->type(), (*i)->optional()) << " " << fixId((*i)->name()) << ";";
        }
        _out << eb;
    }

    if(p->hasMarshaledResult())
    {
        string name = resultStructName(cl->name(), p->name(), true);

        _out << sp;
        _out << nl << "public struct " << name << " : Ice.MarshaledResult";
        _out << sb;

        //
        // One shot constructor
        //
        _out << nl << "public " << name << spar << getOutParams(p, true, false) << "Ice.Current current" << epar;
        _out << sb;
        _out << nl << "_ostr = IceInternal.Incoming.createResponseOutputStream(current);";
        _out << nl << "_ostr.startEncapsulation(current.encoding, " << opFormatTypeToString(p) << ");";
        writeMarshalUnmarshalParams(outParams, p, true, false, true, "_ostr");
        if(p->returnsClasses(false))
        {
            _out << nl << "_ostr.writePendingValues();";
        }
        _out << nl << "_ostr.endEncapsulation();";
        _out << eb;
        _out << sp;
        _out << nl << "public Ice.OutputStream getOutputStream(Ice.Current current)";
        _out << sb;
        _out << nl << "if(_ostr == null)";
        _out << sb;
        _out << nl << "return new " << name << spar;
        if(ret)
        {
            _out << writeValue(ret);
        }
        for(ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end(); ++i)
        {
            _out << writeValue((*i)->type());
        }
        _out << "current" << epar << ".getOutputStream(current);";
        _out << eb;
        _out << nl << "return _ostr;";
        _out << eb;
        _out << sp;
        _out << nl << "private Ice.OutputStream _ostr;";
        _out << eb;
    }
}

Slice::Gen::ProxyVisitor::ProxyVisitor(IceUtilInternal::Output& out) :
    CsVisitor(out)
{
}

bool
Slice::Gen::ProxyVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDecls())
    {
        return false;
    }

    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::ProxyVisitor::visitModuleEnd(const ModulePtr&)
{
    _out << eb;
}

bool
Slice::Gen::ProxyVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal() || (!p->isInterface() && p->allOperations().size() == 0))
    {
        return false;
    }

    string name = p->name();
    ClassList bases = p->bases();

    _out << sp;
    writeDocComment(p, getDeprecateReason(p, 0, p->isInterface() ? "interface" : "class"));
    emitGeneratedCodeAttribute();
    _out << nl << "public interface " << name << "Prx : ";

    vector<string> baseInterfaces;
    for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
    {
        ClassDefPtr def = *q;
        if(def->isInterface() || def->allOperations().size() > 0)
        {
            baseInterfaces.push_back(fixId((*q)->scoped() + "Prx"));
        }
    }

    if(baseInterfaces.empty())
    {
        baseInterfaces.push_back("Ice.ObjectPrx");
    }

    for(vector<string>::const_iterator q = baseInterfaces.begin(); q != baseInterfaces.end();)
    {
        _out << *q;
        if(++q != baseInterfaces.end())
        {
            _out << ", ";
        }
    }
    _out << sb;

    return true;
}

void
Slice::Gen::ProxyVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _out << eb;
}

void
Slice::Gen::ProxyVisitor::visitOperation(const OperationPtr& p)
{
    ClassDefPtr cl = ClassDefPtr::dynamicCast(p->container());
    string name = fixId(p->name(), DotNet::ICloneable, true);
    vector<string> inParams = getInParams(p);
    ParamDeclList inParamDecls = p->inParameters();
    string retS = typeToString(p->returnType(), p->returnIsOptional());
    string deprecateReason = getDeprecateReason(p, cl, "operation");

    {
        //
        // Write the synchronous version of the operation.
        //
        string context = getEscapedParamName(p, "context");
        _out << sp;
        writeDocComment(p, deprecateReason,
            "<param name=\"" + context + " \">The Context map to send with the invocation.</param>");
        if(!deprecateReason.empty())
        {
            _out << nl << "[_System.Obsolete(\"" << deprecateReason << "\")]";
        }
        _out << nl << retS << " " << name << spar << getParams(p)
             << ("Ice.OptionalContext " + context + " = new Ice.OptionalContext()") << epar << ';';
    }

    {
        //
        // Write the async version of the operation (using Async Task API)
        //
        string context = getEscapedParamName(p, "context");
        string cancel = getEscapedParamName(p, "cancel");
        string progress = getEscapedParamName(p, "progress");

        _out << sp;
        writeDocCommentTaskAsyncAMI(p, deprecateReason,
            "<param name=\"" + context + "\">Context map to send with the invocation.</param>",
            "<param name=\"" + progress + "\">Sent progress provider.</param>",
            "<param name=\"" + cancel + "\">A cancellation token that receives the cancellation requests.</param>");
        if(!deprecateReason.empty())
        {
            _out << nl << "[_System.Obsolete(\"" << deprecateReason << "\")]";
        }
        _out << nl << taskResultType(p);
        _out << " " << p->name() << "Async" << spar << inParams
             << ("Ice.OptionalContext " + context + " = new Ice.OptionalContext()")
             << ("_System.IProgress<bool> " + progress + " = null")
             << ("_System.Threading.CancellationToken " + cancel + " = new _System.Threading.CancellationToken()")
             << epar << ";";
    }

    {
        //
        // Write the async versions of the operation (using IAsyncResult API)
        //
        string clScope = fixId(cl->scope());
        string delType = clScope + "Callback_" + cl->name() + "_" + p->name();

        string context = getEscapedParamName(p, "context");
        string callback = getEscapedParamName(p, "callback");
        string cookie = getEscapedParamName(p, "cookie");
        string asyncResult = getEscapedParamName(p, "asyncResult");

        _out << sp;
        writeDocCommentAMI(p, InParam, deprecateReason,
                           "<param name=\"" + context + "\">The Context map to send with the invocation.</param>");
        if(!deprecateReason.empty())
        {
            _out << nl << "[_System.Obsolete(\"" << deprecateReason << "\")]";
        }
        _out << nl << "Ice.AsyncResult<" << delType << "> begin_" << p->name() << spar << inParams
             << ("Ice.OptionalContext " + context + " = new Ice.OptionalContext()") << epar << ';';

        //
        // Type-unsafe begin_ methods.
        //
        _out << sp;
        writeDocCommentAMI(p, InParam, deprecateReason,
           "<param name=\"" + callback + "\">Asynchronous callback invoked when the operation completes.</param>",
           "<param name=\"" + cookie + "\">Application data to store in the asynchronous result object.</param>");
        if(!deprecateReason.empty())
        {
            _out << nl << "[_System.Obsolete(\"" << deprecateReason << "\")]";
        }
        _out << nl << "Ice.AsyncResult begin_" << p->name() << spar << inParams
             << "Ice.AsyncCallback " + callback << "object " + cookie << epar << ';';

        _out << sp;
        writeDocCommentAMI(p, InParam, deprecateReason,
           "<param name=\"" + context + "\">The Context map to send with the invocation.</param>",
           "<param name=\"" + callback + "\">Asynchronous callback invoked when the operation completes.</param>",
           "<param name=\"" + cookie + "\">Application data to store in the asynchronous result object.</param>");
        if(!deprecateReason.empty())
        {
            _out << nl << "[_System.Obsolete(\"" << deprecateReason << "\")]";
        }
        _out << nl << "Ice.AsyncResult begin_" << p->name() << spar << inParams
             << "Ice.OptionalContext " + context << "Ice.AsyncCallback " + callback << "object " + cookie << epar << ';';

        //
        // end_ method.
        //
        _out << sp;
        writeDocCommentAMI(p, OutParam, deprecateReason,
           "<param name=\"" + asyncResult + "\">The asynchronous result object for the invocation.</param>");
        if(!deprecateReason.empty())
        {
            _out << nl << "[_System.Obsolete(\"" << deprecateReason << "\")]";
        }
        _out << nl << retS << " end_" << p->name() << spar << getOutParams(p, false, true)
             << "Ice.AsyncResult " + asyncResult << epar << ';';
    }
}

Slice::Gen::AsyncDelegateVisitor::AsyncDelegateVisitor(IceUtilInternal::Output& out)
    : CsVisitor(out)
{
}

bool
Slice::Gen::AsyncDelegateVisitor::visitModuleStart(const ModulePtr& p)
{
    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::AsyncDelegateVisitor::visitModuleEnd(const ModulePtr&)
{
    _out << eb;
}

bool
Slice::Gen::AsyncDelegateVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    return !p->operations().empty();
}

void
Slice::Gen::AsyncDelegateVisitor::visitClassDefEnd(const ClassDefPtr&)
{
}

void
Slice::Gen::AsyncDelegateVisitor::visitOperation(const OperationPtr& p)
{
    ClassDefPtr cl = ClassDefPtr::dynamicCast(p->container());

    //
    // We also generate delegates for local twoway-style operations marked with "async-oneway" metadata.
    //
    if(cl->isLocal() && (!(cl->hasMetaData("async-oneway") || p->hasMetaData("async-oneway")) || !p->returnsData()))
    {
        return;
    }

    vector<string> paramDeclAMI = getOutParams(p, false, false);
    string retS = typeToString(p->returnType(), p->returnIsOptional());
    string delName = "Callback_" + cl->name() + "_" + p->name();

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public delegate void " << delName << spar;
    if(p->returnType())
    {
        _out << retS + " ret";
    }
    _out << paramDeclAMI << epar << ';';
}

Slice::Gen::OpsVisitor::OpsVisitor(IceUtilInternal::Output& out)
    : CsVisitor(out)
{
}

bool
Slice::Gen::OpsVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasAbstractClassDefs())
    {
        return false;
    }

    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::OpsVisitor::visitModuleEnd(const ModulePtr&)
{
    _out << eb;
}

bool
Slice::Gen::OpsVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    //
    // Don't generate Operations interfaces for non-abstract classes.
    //
    if(!p->isAbstract() || p->isLocal())
    {
        return false;
    }
    string name = p->name();
    string scoped = fixId(p->scoped());
    ClassList bases = p->bases();
    string opIntfName = "Operations";

    _out << sp;
    writeDocComment(p, getDeprecateReason(p, 0, p->isInterface() ? "interface" : "class"));
    emitGeneratedCodeAttribute();
    _out << nl << "public interface " << name << opIntfName << '_';
    if((bases.size() == 1 && bases.front()->isAbstract()) || bases.size() > 1)
    {
        _out << " : ";
        ClassList::const_iterator q = bases.begin();
        bool first = true;
        while(q != bases.end())
        {
            if((*q)->isAbstract())
            {
                if (!first)
                {
                    _out << ", ";
                }
                else
                {
                    first = false;
                }
                string s = (*q)->scoped();
                s += "Operations";
                _out << fixId(s) << '_';
            }
            ++q;
        }
    }
    _out << sb;

    OperationList ops = p->operations();
    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        bool amd = !p->isLocal() && (p->hasMetaData("amd") || op->hasMetaData("amd"));
        string retS;
        vector<string> params, args;
        string name = getDispatchParams(op, retS, params, args);
        _out << sp;
        if(amd)
        {
            writeDocCommentAMD(op,
                "<param name=\"" + args.back() + "\">The Current object for the invocation.</param>");
        }
        else
        {
            writeDocComment(op, getDeprecateReason(op, p, "operation"),
                "<param name=\"" + args.back() + "\">The Current object for the invocation.</param>");
        }
        emitAttributes(op);
        emitDeprecate(op, op, _out, "operation");
        emitGeneratedCodeAttribute();
        _out << nl << retS << " " << name << spar << params << epar << ";";
    }

    _out << eb;
    return false;
}

Slice::Gen::HelperVisitor::HelperVisitor(IceUtilInternal::Output& out) :
    CsVisitor(out)
{
}

bool
Slice::Gen::HelperVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDecls() && !p->hasNonLocalSequences() && !p->hasDictionaries())
    {
        return false;
    }

    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::HelperVisitor::visitModuleEnd(const ModulePtr&)
{
    _out << eb;
}

bool
Slice::Gen::HelperVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal() || (!p->isInterface() && p->allOperations().size() == 0))
    {
        return false;
    }

    string name = p->name();
    ClassList bases = p->bases();

    _out << sp;
    emitComVisibleAttribute();
    emitGeneratedCodeAttribute();
    _out << nl << "[_System.Serializable]";
    _out << nl << "public sealed class " << name << "PrxHelper : Ice.ObjectPrxHelperBase, " << name << "Prx";
    _out << sb;

    _out << sp;
    _out << nl << "public " << name << "PrxHelper()";
    _out << sb;
    _out << eb;

    _out << sp;
    _out << nl << "public " << name << "PrxHelper(_System.Runtime.Serialization.SerializationInfo info, "
         << "_System.Runtime.Serialization.StreamingContext context) : base(info, context)";
    _out << sb;
    _out << eb;

    OperationList ops = p->allOperations();

    if(!ops.empty())
    {
        _out << sp << nl << "#region Synchronous operations";
    }

    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());
        string opName = fixId(op->name(), DotNet::ICloneable, true);
        TypePtr ret = op->returnType();
        string retS = typeToString(ret, op->returnIsOptional());

        vector<string> params = getParams(op);
        vector<string> args = getArgs(op);
        vector<string> argsAMI = getInArgs(op);

        string deprecateReason = getDeprecateReason(op, p, "operation");

        ParamDeclList inParams = op->inParameters();
        ParamDeclList outParams = op->outParameters();

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

        //
        // Arrange exceptions into most-derived to least-derived order. If we don't
        // do this, a base exception handler can appear before a derived exception
        // handler, causing compiler warnings and resulting in the base exception
        // being marshaled instead of the derived exception.
        //
#if defined(__SUNPRO_CC)
        throws.sort(Slice::derivedToBaseCompare);
#else
        throws.sort(Slice::DerivedToBaseCompare());
#endif

        string context = getEscapedParamName(op, "context");

        _out << sp;
        _out << nl << "public " << retS << " " << opName << spar << params
             << ("Ice.OptionalContext " + context + " = new Ice.OptionalContext()") << epar;
        _out << sb;
        _out << nl << "try";
        _out << sb;

        _out << nl;

        if(ret || !outParams.empty())
        {
            if(outParams.empty())
            {
                _out << "return ";
            }
            else if(ret || outParams.size() > 1)
            {
                _out << "var result_ = ";
            }
            else
            {
                _out << fixId(outParams.front()->name()) << " = ";
            }
        }
        _out << "_iceI_" << op->name() << "Async" << spar << argsAMI << context
             << "null" << "_System.Threading.CancellationToken.None" << "true" << epar;

        if(ret || outParams.size() > 0)
        {
            _out << ".Result;";
        }
        else
        {
            _out << ".Wait();";
        }

        if((ret && outParams.size() > 0) || outParams.size() > 1)
        {
            for(ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end(); ++i)
            {
                ParamDeclPtr param = *i;
                _out << nl << fixId(param->name()) << " = result_." << fixId(param->name()) << ";";
            }

            if(ret)
            {
                _out << nl << "return result_." << resultStructReturnValueName(outParams) << ";";
            }
        }
        _out << eb;
        _out << nl << "catch(_System.AggregateException ex_)";
        _out << sb;
        _out << nl << "throw ex_.InnerException;";
        _out << eb;
        _out << eb;
    }

    if(!ops.empty())
    {
        _out << sp << nl << "#endregion"; // Synchronous operations
    }

    //
    // Async Task AMI mapping.
    //
    _out << sp << nl << "#region Async Task operations";
    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;

        ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());
        string clScope = fixId(cl->scope());
        vector<string> paramsAMI = getInParams(op);
        vector<string> argsAMI = getInArgs(op);

        string opName = op->name();

        ParamDeclList inParams = op->inParameters();
        ParamDeclList outParams = op->outParameters();

        string context = getEscapedParamName(op, "context");
        string cancel = getEscapedParamName(op, "cancel");
        string progress = getEscapedParamName(op, "progress");

        TypePtr ret = op->returnType();

        string retS = typeToString(ret, op->returnIsOptional());

        string returnTypeS = resultType(op);

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

        //
        // Arrange exceptions into most-derived to least-derived order. If we don't
        // do this, a base exception handler can appear before a derived exception
        // handler, causing compiler warnings and resulting in the base exception
        // being marshaled instead of the derived exception.
        //
#if defined(__SUNPRO_CC)
        throws.sort(Slice::derivedToBaseCompare);
#else
        throws.sort(Slice::DerivedToBaseCompare());
#endif

        //
        // Write the public Async method.
        //
        _out << sp;
        _out << nl << "public _System.Threading.Tasks.Task";
        if(!returnTypeS.empty())
        {
            _out << "<" << returnTypeS << ">";
        }
        _out << " " << opName << "Async" << spar << paramsAMI
             << ("Ice.OptionalContext " + context + " = new Ice.OptionalContext()")
             << ("_System.IProgress<bool> " + progress + " = null")
             << ("_System.Threading.CancellationToken " + cancel + " = new _System.Threading.CancellationToken()")
             << epar;

        _out << sb;
        _out << nl << "return _iceI_" << opName << "Async" << spar << argsAMI
             << context << progress << cancel << "false" << epar << ";";
        _out << eb;

        //
        // Write the Async method implementation.
        //
        _out << sp;
        _out << nl << "private _System.Threading.Tasks.Task";
        if(!returnTypeS.empty())
        {
            _out << "<" << returnTypeS << ">";
        }
        _out << " _iceI_" << opName << "Async" << spar << getInParams(op, true)
             << "Ice.OptionalContext context"
             << "_System.IProgress<bool> progress"
             << "_System.Threading.CancellationToken cancel"
             << "bool synchronous" << epar;
        _out << sb;

        if(returnTypeS.empty())
        {
            _out << nl << "var completed = "
                 << "new IceInternal.OperationTaskCompletionCallback<object>(progress, cancel);";
        }
        else
        {
            _out << nl << "var completed = "
                 << "new IceInternal.OperationTaskCompletionCallback<" << returnTypeS << ">(progress, cancel);";
        }

        _out << nl << "_iceI_" << opName << spar << getInArgs(op, true) << "context" << "synchronous" << "completed"
             << epar << ";";
        _out << nl << "return completed.Task;";

        _out << eb;

        string flatName = "_" + opName + "_name";
        _out << sp << nl << "private const string " << flatName << " = \"" << op->name() << "\";";

        //
        // Write the common invoke method
        //
        _out << sp << nl;
        _out << "private void _iceI_" << op->name() << spar << getInParams(op, true)
             << "_System.Collections.Generic.Dictionary<string, string> context"
             << "bool synchronous"
             << "IceInternal.OutgoingAsyncCompletionCallback completed" << epar;
        _out << sb;

        if(op->returnsData())
        {
            _out << nl << "iceCheckAsyncTwowayOnly(" << flatName << ");";
        }

        if(returnTypeS.empty())
        {
            _out << nl << "var outAsync = getOutgoingAsync<object>(completed);";
        }
        else
        {
            _out << nl << "var outAsync = getOutgoingAsync<" << returnTypeS << ">(completed);";
        }

        _out << nl << "outAsync.invoke(";
        _out.inc();
        _out << nl << flatName << ",";
        _out << nl << sliceModeToIceMode(op->sendMode()) << ",";
        _out << nl << opFormatTypeToString(op) << ",";
        _out << nl << "context,";
        _out << nl << "synchronous";
        if(!inParams.empty())
        {
            _out << ",";
            _out << nl << "write: (Ice.OutputStream ostr) =>";
            _out << sb;
            writeMarshalUnmarshalParams(inParams, 0, true);
            if(op->sendsClasses(false))
            {
                _out << nl << "ostr.writePendingValues();";
            }
            _out << eb;
        }

        if(!throws.empty())
        {
            _out << ",";
            _out << nl << "userException: (Ice.UserException ex) =>";
            _out << sb;
            _out << nl << "try";
            _out << sb;
            _out << nl << "throw ex;";
            _out << eb;

            //
            // Generate a catch block for each legal user exception.
            //
            for(ExceptionList::const_iterator i = throws.begin(); i != throws.end(); ++i)
            {
                _out << nl << "catch(" << fixId((*i)->scoped()) << ")";
                _out << sb;
                _out << nl << "throw;";
                _out << eb;
            }

            _out << nl << "catch(Ice.UserException)";
            _out << sb;
            _out << eb;

            _out << eb;
        }

        if(ret || !outParams.empty())
        {
            _out << ",";
            _out << nl << "read: (Ice.InputStream istr) =>";
            _out << sb;
            if(outParams.empty())
            {
                _out << nl << returnTypeS << " ret";
                if(!op->returnIsOptional())
                {
                    StructPtr st = StructPtr::dynamicCast(ret);
                    if(st && isValueType(st))
                    {
                        _out << " = " << "new " + returnTypeS + "()";
                    }
                    else if(isClassType(ret) || st)
                    {
                        _out << " = null";
                    }
                }
                else if(isClassType(ret))
                {
                    _out << " = Ice.Util.None";
                }
                _out << ";";
            }
            else if(ret || outParams.size() > 1)
            {
                _out << nl << returnTypeS << " ret = new " << returnTypeS << "();";
            }
            else
            {
                TypePtr t = outParams.front()->type();
                _out << nl << typeToString(t, (outParams.front()->optional())) << " iceP_" << outParams.front()->name();
                if(!outParams.front()->optional())
                {
                    StructPtr st = StructPtr::dynamicCast(t);
                    if(st && isValueType(st))
                    {
                        _out << " = " << "new " << typeToString(t) << "()";
                    }
                    else if(isClassType(t) || st)
                    {
                        _out << " = null";
                    }
                }
                else if(isClassType(t))
                {
                    _out << " = Ice.Util.None";
                }
                _out << ";";
            }

            writeMarshalUnmarshalParams(outParams, op, false, true);
            if(op->returnsClasses(false))
            {
                _out << nl << "istr.readPendingValues();";
            }

            if(!ret && outParams.size() == 1)
            {
                _out << nl << "return iceP_" << outParams.front()->name() << ";";
            }
            else
            {
                _out << nl << "return ret;";
            }
            _out << eb;
        }
        _out << ");";
        _out.dec();
        _out << eb;
    }

    _out << sp << nl << "#endregion"; // Asynchronous Task operations

    //
    // IAsyncResult AMI mapping.
    //
    _out << sp << nl << "#region Asynchronous operations";
    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;

        ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());
        string clScope = fixId(cl->scope());
        vector<string> paramsAMI = getInParams(op);
        vector<string> argsAMI = getInArgs(op);
        string opName = op->name();
        ParamDeclList inParams = op->inParameters();
        ParamDeclList outParams = op->outParameters();

        TypePtr ret = op->returnType();
        string retS = typeToString(ret, op->returnIsOptional());

        string returnTypeS = resultType(op);

        //
        // Write the begin_ methods.
        //
        string delType = clScope + "Callback_" + cl->name() + "_" + op->name();

        string context = getEscapedParamName(op, "context");
        string callback = getEscapedParamName(op, "callback");
        string cookie = getEscapedParamName(op, "cookie");

        _out << sp;
        _out << nl << "public Ice.AsyncResult<" << delType << "> begin_" << opName << spar << paramsAMI
             << "Ice.OptionalContext " + context + " = new Ice.OptionalContext()" << epar;
        _out << sb;
        _out << nl << "return begin_" << opName << spar << argsAMI << context << "null" << "null" << "false"
            << epar << ';';
        _out << eb;

        _out << sp;
        _out << nl << "public Ice.AsyncResult begin_" << opName << spar << paramsAMI
             << "Ice.AsyncCallback " + callback << "object " + cookie << epar;
        _out << sb;
        _out << nl << "return begin_" << opName << spar << argsAMI << "new Ice.OptionalContext()" << callback
            << cookie << "false" << epar << ';';
        _out << eb;

        _out << sp;
        _out << nl << "public Ice.AsyncResult begin_" << opName << spar << paramsAMI
            << "Ice.OptionalContext " + context << "Ice.AsyncCallback " + callback
            << "object " + cookie << epar;
        _out << sb;
        _out << nl << "return begin_" << opName << spar << argsAMI << context << callback
            << cookie << "false" << epar << ';';
        _out << eb;

        //
        // Write the end_ method.
        //
        string flatName = "_" + opName + "_name";
        string asyncResult = getEscapedParamName(op, "asyncResult");

        _out << sp << nl << "public " << retS << " end_" << opName << spar << getOutParams(op, false, true)
            << "Ice.AsyncResult " + asyncResult << epar;
        _out << sb;

        _out << nl << "var resultI_ = IceInternal.AsyncResultI.check(" + asyncResult + ", this, " << flatName << ");";

        if(returnTypeS.empty())
        {
            _out << nl << "((IceInternal.OutgoingAsyncT<object>)resultI_.OutgoingAsync).getResult(resultI_.wait());";
        }
        else
        {
            _out << nl << "var outgoing_ = (IceInternal.OutgoingAsyncT<" << returnTypeS << ">)resultI_.OutgoingAsync;";
            if(outParams.empty())
            {
                _out << nl << "return outgoing_.getResult(resultI_.wait());";
            }
            else if(!ret && outParams.size() == 1)
            {
                _out << nl << fixId(outParams.front()->name()) << " = outgoing_.getResult(resultI_.wait());";
            }
            else
            {
                _out << nl << "var result_ = outgoing_.getResult(resultI_.wait());";
                for(ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end(); ++i)
                {
                    _out << nl << fixId((*i)->name()) << " = result_." << fixId((*i)->name()) << ";";
                }

                if(ret)
                {
                    _out << nl << "return result_." << resultStructReturnValueName(outParams) << ";";
                }
            }
        }
        _out << eb;

        //
        // Write the common begin_ implementation.
        //
        _out << sp;
        _out << nl << "private Ice.AsyncResult<" << delType << "> begin_" << opName << spar << getInParams(op, true)
                << "_System.Collections.Generic.Dictionary<string, string> context"
                << "Ice.AsyncCallback completedCallback" << "object cookie" << "bool synchronous"
                << epar;
        _out << sb;

        _out << nl << "var completed = new IceInternal.OperationAsyncResultCompletionCallback<" << delType;
        _out << ", " << (returnTypeS.empty() ? "object" : returnTypeS);
        _out << ">(";

        //
        // Write the completed callback
        //
        _out.inc();
        _out << nl << "(" << delType << " cb, " << (returnTypeS.empty() ? "object" : returnTypeS) << " ret) =>";
        _out << sb;
        _out << nl << "if(cb != null)";
        _out << sb;
        _out << nl << "cb.Invoke" << spar;
        if(ret && outParams.empty())
        {
            _out << "ret";
        }
        else if(ret || outParams.size() > 1)
        {
            if(ret)
            {
                _out << "ret." + resultStructReturnValueName(outParams);
            }
            for(ParamDeclList::const_iterator pli = outParams.begin(); pli != outParams.end(); ++pli)
            {
                _out << "ret." + fixId((*pli)->name());
            }
        }
        else if(!outParams.empty())
        {
            _out << "ret";
        }
        _out << epar << ';';
        _out << eb;
        _out << eb << ",";
        _out << nl << "this, " << flatName << ", cookie, completedCallback);";

        _out.dec();
        _out << nl << "_iceI_" << op->name() << spar << getInArgs(op, true) << "context" << "synchronous" << "completed" << epar << ";";
        _out << nl << "return completed;";
        _out << eb;
    }
    _out << sp << nl << "#endregion"; // Asynchronous operations
    _out << sp << nl << "#region Checked and unchecked cast operations";

    _out << sp << nl << "public static " << name << "Prx checkedCast(Ice.ObjectPrx b)";
    _out << sb;
    _out << nl << "if(b == null)";
    _out << sb;
    _out << nl << "return null;";
    _out << eb;
    _out << nl << name << "Prx r = b as " << name << "Prx;";
    _out << nl << "if((r == null) && b.ice_isA(ice_staticId()))";
    _out << sb;
    _out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    _out << nl << "h.iceCopyFrom(b);";
    _out << nl << "r = h;";
    _out << eb;
    _out << nl << "return r;";
    _out << eb;

    _out << sp << nl << "public static " << name
         << "Prx checkedCast(Ice.ObjectPrx b, _System.Collections.Generic.Dictionary<string, string> ctx)";
    _out << sb;
    _out << nl << "if(b == null)";
    _out << sb;
    _out << nl << "return null;";
    _out << eb;
    _out << nl << name << "Prx r = b as " << name << "Prx;";
    _out << nl << "if((r == null) && b.ice_isA(ice_staticId(), ctx))";
    _out << sb;
    _out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    _out << nl << "h.iceCopyFrom(b);";
    _out << nl << "r = h;";
    _out << eb;
    _out << nl << "return r;";
    _out << eb;

    _out << sp << nl << "public static " << name << "Prx checkedCast(Ice.ObjectPrx b, string f)";
    _out << sb;
    _out << nl << "if(b == null)";
    _out << sb;
    _out << nl << "return null;";
    _out << eb;
    _out << nl << "Ice.ObjectPrx bb = b.ice_facet(f);";
    _out << nl << "try";
    _out << sb;
    _out << nl << "if(bb.ice_isA(ice_staticId()))";
    _out << sb;
    _out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    _out << nl << "h.iceCopyFrom(bb);";
    _out << nl << "return h;";
    _out << eb;
    _out << eb;
    _out << nl << "catch(Ice.FacetNotExistException)";
    _out << sb;
    _out << eb;
    _out << nl << "return null;";
    _out << eb;

    _out << sp << nl << "public static " << name
         << "Prx checkedCast(Ice.ObjectPrx b, string f, "
         << "_System.Collections.Generic.Dictionary<string, string> ctx)";
    _out << sb;
    _out << nl << "if(b == null)";
    _out << sb;
    _out << nl << "return null;";
    _out << eb;
    _out << nl << "Ice.ObjectPrx bb = b.ice_facet(f);";
    _out << nl << "try";
    _out << sb;
    _out << nl << "if(bb.ice_isA(ice_staticId(), ctx))";
    _out << sb;
    _out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    _out << nl << "h.iceCopyFrom(bb);";
    _out << nl << "return h;";
    _out << eb;
    _out << eb;
    _out << nl << "catch(Ice.FacetNotExistException)";
    _out << sb;
    _out << eb;
    _out << nl << "return null;";
    _out << eb;

    _out << sp << nl << "public static " << name << "Prx uncheckedCast(Ice.ObjectPrx b)";
    _out << sb;
    _out << nl << "if(b == null)";
    _out << sb;
    _out << nl << "return null;";
    _out << eb;
    _out << nl << name << "Prx r = b as " << name << "Prx;";
    _out << nl << "if(r == null)";
    _out << sb;
    _out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    _out << nl << "h.iceCopyFrom(b);";
    _out << nl << "r = h;";
    _out << eb;
    _out << nl << "return r;";
    _out << eb;

    _out << sp << nl << "public static " << name << "Prx uncheckedCast(Ice.ObjectPrx b, string f)";
    _out << sb;
    _out << nl << "if(b == null)";
    _out << sb;
    _out << nl << "return null;";
    _out << eb;
    _out << nl << "Ice.ObjectPrx bb = b.ice_facet(f);";
    _out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    _out << nl << "h.iceCopyFrom(bb);";
    _out << nl << "return h;";
    _out << eb;

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

    //
    // Need static-readonly for arrays in C# (not const)
    //
    _out << sp << nl << "private static readonly string[] _ids =";
    _out << sb;

    {
        StringList::const_iterator q = ids.begin();
        while(q != ids.end())
        {
            _out << nl << '"' << *q << '"';
            if(++q != ids.end())
            {
                _out << ',';
            }
        }
    }
    _out << eb << ";";

    _out << sp << nl << "public static string ice_staticId()";
    _out << sb;
    _out << nl << "return _ids[" << scopedPos << "];";
    _out << eb;

    _out << sp << nl << "#endregion"; // Checked and unchecked cast operations

    _out << sp << nl << "#region Marshaling support";

    _out << sp << nl << "public static void write(Ice.OutputStream ostr, " << name << "Prx v)";
    _out << sb;
    _out << nl << "ostr.writeProxy(v);";
    _out << eb;

    _out << sp << nl << "public static " << name << "Prx read(Ice.InputStream istr)";
    _out << sb;
    _out << nl << "Ice.ObjectPrx proxy = istr.readProxy();";
    _out << nl << "if(proxy != null)";
    _out << sb;
    _out << nl << name << "PrxHelper result = new " << name << "PrxHelper();";
    _out << nl << "result.iceCopyFrom(proxy);";
    _out << nl << "return result;";
    _out << eb;
    _out << nl << "return null;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Marshaling support

    return true;
}

void
Slice::Gen::HelperVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _out << eb;
}

void
Slice::Gen::HelperVisitor::visitSequence(const SequencePtr& p)
{
    //
    // Don't generate helper for sequence of a local type.
    //
    if(p->isLocal())
    {
        return;
    }

    string typeS = typeToString(p);

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public sealed class " << p->name() << "Helper";
    _out << sb;

    _out << sp << nl << "public static void write(Ice.OutputStream ostr, " << typeS << " v)";
    _out << sb;
    writeSequenceMarshalUnmarshalCode(_out, p, "v", true, false);
    _out << eb;

    _out << sp << nl << "public static " << typeS << " read(Ice.InputStream istr)";
    _out << sb;
    _out << nl << typeS << " v;";
    writeSequenceMarshalUnmarshalCode(_out, p, "v", false, false);
    _out << nl << "return v;";
    _out << eb;
    _out << eb;

    string prefix = "cs:generic:";
    string meta;
    if(p->findMetaData(prefix, meta))
    {
        string type = meta.substr(prefix.size());
        if(type == "List" || type == "LinkedList" || type == "Queue" || type == "Stack")
        {
            return;
        }

        if(!isClassType(p->type()))
        {
            return;
        }

        //
        // The sequence is a custom sequence with elements of class type.
        // Emit a dummy class that causes a compile-time error if the
        // custom sequence type does not implement an indexer.
        //
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public class " << p->name() << "_Tester";
        _out << sb;
        _out << nl << p->name() << "_Tester()";
        _out << sb;
        _out << nl << typeS << " test = new " << typeS << "();";
        _out << nl << "test[0] = null;";
        _out << eb;
        _out << eb;
    }
}

void
Slice::Gen::HelperVisitor::visitDictionary(const DictionaryPtr& p)
{
    //
    // Don't generate helper for a dictionary containing a local type
    //
    if(p->isLocal())
    {
        return;
    }

    TypePtr key = p->keyType();
    TypePtr value = p->valueType();

    string meta;

    string prefix = "cs:generic:";
    string genericType;
    if(!p->findMetaData(prefix, meta))
    {
        genericType = "Dictionary";
    }
    else
    {
        genericType = meta.substr(prefix.size());
    }

    string keyS = typeToString(key);
    string valueS = typeToString(value);
    string name = "_System.Collections.Generic." + genericType + "<" + keyS + ", " + valueS + ">";

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public sealed class " << p->name() << "Helper";
    _out << sb;

    _out << sp << nl << "public static void write(";
    _out.useCurrentPosAsIndent();
    _out << "Ice.OutputStream ostr,";
    _out << nl << name << " v)";
    _out.restoreIndent();
    _out << sb;
    _out << nl << "if(v == null)";
    _out << sb;
    _out << nl << "ostr.writeSize(0);";
    _out << eb;
    _out << nl << "else";
    _out << sb;
    _out << nl << "ostr.writeSize(v.Count);";
    _out << nl << "foreach(_System.Collections.";
    _out << "Generic.KeyValuePair<" << keyS << ", " << valueS << ">";
    _out << " e in v)";
    _out << sb;
    writeMarshalUnmarshalCode(_out, key, "e.Key", true);
    writeMarshalUnmarshalCode(_out, value, "e.Value", true);
    _out << eb;
    _out << eb;
    _out << eb;

    _out << sp << nl << "public static " << name << " read(Ice.InputStream istr)";
    _out << sb;
    _out << nl << "int sz = istr.readSize();";
    _out << nl << name << " r = new " << name << "();";
    _out << nl << "for(int i = 0; i < sz; ++i)";
    _out << sb;
    _out << nl << keyS << " k;";
    StructPtr st = StructPtr::dynamicCast(key);
    if(st)
    {
        if(isValueType(st))
        {
            _out << nl << "k = new " << typeToString(key) << "();";
        }
        else
        {
            _out << nl << "k = null;";
        }
    }
    writeMarshalUnmarshalCode(_out, key, "k", false);

    if(isClassType(value))
    {
        ostringstream os;
        os << '(' << typeToString(value) << " v) => { r[k] = v; }";
        writeMarshalUnmarshalCode(_out, value, os.str(), false);
    }
    else
    {
        _out << nl << valueS << " v;";
        StructPtr st = StructPtr::dynamicCast(value);
        if(st)
        {
            if(isValueType(st))
            {
                _out << nl << "v = new " << typeToString(value) << "();";
            }
            else
            {
                _out << nl << "v = null;";
            }
        }
        writeMarshalUnmarshalCode(_out, value, "v", false);
        _out << nl << "r[k] = v;";
    }
    _out << eb;
    _out << nl << "return r;";
    _out << eb;

    _out << eb;
}

Slice::Gen::DispatcherVisitor::DispatcherVisitor(::IceUtilInternal::Output& out, bool tie) :
    CsVisitor(out),
    _tie(tie)
{
}

bool
Slice::Gen::DispatcherVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalClassDecls())
    {
        return false;
    }

    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::DispatcherVisitor::visitModuleEnd(const ModulePtr&)
{
    _out << eb;
}

bool
Slice::Gen::DispatcherVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal() || (!p->isInterface() && p->allOperations().empty()))
    {
        return false;
    }

    ClassList bases = p->bases();
    bool hasBaseClass = !bases.empty() && !bases.front()->isInterface();
    string baseClass = "Ice.ObjectImpl";
    if(hasBaseClass && !bases.front()->allOperations().empty())
    {
        baseClass = fixId(bases.front()->scoped() + "Disp_");
    }

    string name = p->name();

    _out << sp;
    emitComVisibleAttribute();
    emitGeneratedCodeAttribute();
    _out << nl << "public abstract class " << name << "Disp_ : " << baseClass << ", ";

    if(p->isInterface())
    {
        _out << fixId(name);
    }
    else
    {
        _out << name << "Operations_";
    }

    if(!p->isInterface())
    {
        ClassList allBases = bases;
        if(!allBases.empty() && !allBases.front()->isInterface())
        {
            allBases.pop_front();
        }

        for(ClassList::const_iterator i = allBases.begin(); i != allBases.end(); ++i)
        {
            _out << ", " << fixId((*i)->scoped());
        }
    }

    _out << sb;

    OperationList ops = p->operations();
    if(!ops.empty())
    {
        _out << sp << nl << "#region Slice operations";
    }

    for(OperationList::const_iterator i = ops.begin(); i != ops.end(); ++i)
    {
        string retS;
        vector<string> params, args;
        string name = getDispatchParams(*i, retS, params, args);
        _out << sp << nl << "public abstract " << retS << " " << name << spar << params << epar << ';';
    }

    if(!ops.empty())
    {
        _out << sp << nl << "#endregion"; // Slice operations
    }

    writeInheritedOperations(p);
    writeDispatch(p);

    if((_tie || p->hasMetaData("cs:tie")) && !p->isLocal() && p->isAbstract())
    {
        // Need to generate tie

        // close previous class
        _out << eb;

        string opIntfName = "Operations";

        _out << sp;
        emitComVisibleAttribute();
        emitGeneratedCodeAttribute();
        _out << nl << "public class " << name << "Tie_ : " << name << "Disp_, Ice.TieBase";

        _out << sb;

        _out << sp << nl << "public " << name << "Tie_()";
        _out << sb;
        _out << eb;

        _out << sp << nl << "public " << name << "Tie_(" << name << opIntfName << "_ del)";
        _out << sb;
        _out << nl << "_ice_delegate = del;";
        _out << eb;

        _out << sp << nl << "public object ice_delegate()";
        _out << sb;
        _out << nl << "return _ice_delegate;";
        _out << eb;

        _out << sp << nl << "public void ice_delegate(object del)";
        _out << sb;
        _out << nl << "_ice_delegate = (" << name << opIntfName << "_)del;";
        _out << eb;

        _out << sp << nl << "public override int GetHashCode()";
        _out << sb;
        _out << nl << "return _ice_delegate == null ? 0 : _ice_delegate.GetHashCode();";
        _out << eb;

        _out << sp << nl << "public override bool Equals(object rhs)";
        _out << sb;
        _out << nl << "if(object.ReferenceEquals(this, rhs))";
        _out << sb;
        _out << nl << "return true;";
        _out << eb;
        _out << nl << "if(!(rhs is " << name << "Tie_))";
        _out << sb;
        _out << nl << "return false;";
        _out << eb;
        _out << nl << "if(_ice_delegate == null)";
        _out << sb;
        _out << nl << "return ((" << name << "Tie_)rhs)._ice_delegate == null;";
        _out << eb;
        _out << nl << "return _ice_delegate.Equals(((" << name << "Tie_)rhs)._ice_delegate);";
        _out << eb;

        writeTieOperations(p);

        _out << sp << nl << "private " << name << opIntfName << "_ _ice_delegate;";
    }

    return true;
}
void
Slice::Gen::DispatcherVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _out << eb;
}

void
Slice::Gen::DispatcherVisitor::writeTieOperations(const ClassDefPtr& p, NameSet* opNames)
{
    OperationList ops = p->operations();
    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        string retS;
        vector<string> params;
        vector<string> args;
        string opName = getDispatchParams(*r, retS, params, args);
        if(opNames)
        {
            if(opNames->find(opName) != opNames->end())
            {
                continue;
            }
            opNames->insert(opName);
        }

        _out << sp << nl << "public override " << retS << ' ' << opName << spar << params << epar;
        _out << sb;
        _out << nl;
        if(retS != "void")
        {
            _out << "return ";
        }
        _out << "_ice_delegate." << opName << spar << args << epar << ';';
        _out << eb;
    }

    if(!opNames)
    {
        NameSet opNames;
        ClassList bases = p->bases();
        for(ClassList::const_iterator i = bases.begin(); i != bases.end(); ++i)
        {
            writeTieOperations(*i, &opNames);
        }
    }
    else
    {
        ClassList bases = p->bases();
        for(ClassList::const_iterator i = bases.begin(); i != bases.end(); ++i)
        {
            writeTieOperations(*i, opNames);
        }
    }
}

Slice::Gen::BaseImplVisitor::BaseImplVisitor(IceUtilInternal::Output& out)
    : CsVisitor(out)
{
}

void
Slice::Gen::BaseImplVisitor::writeOperation(const OperationPtr& op, bool comment, bool forTie)
{
    ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());
    string opName = op->name();
    TypePtr ret = op->returnType();
    ParamDeclList params = op->parameters();
    ParamDeclList outParams;
    ParamDeclList inParams;
    for(ParamDeclList::const_iterator i = params.begin(); i != params.end(); ++i)
    {
        if((*i)->isOutParam())
        {
            outParams.push_back(*i);
        }
        else
        {
            inParams.push_back(*i);
        }
    }

    if(comment)
    {
        _out << nl << "// ";
    }
    else
    {
        _out << sp << nl;
    }

    if(!cl->isLocal() && (cl->hasMetaData("amd") || op->hasMetaData("amd")))
    {
        ParamDeclList::const_iterator i;
        vector<string> pDecl = getInParams(op);
        string resultType = CsGenerator::resultType(op, true);

        _out << "public ";
        if(!forTie)
        {
            _out << "override ";
        }

        _out << "_System.Threading.Tasks.Task";
        if(!resultType.empty())
        {
            _out << "<" << resultType << ">";
        }
        _out << " " << opName << "Async" << spar << pDecl << "Ice.Current current = null" << epar;

        if(comment)
        {
            _out << ';';
            return;
        }

        _out << sb;
        if(ret)
        {
            _out << nl << typeToString(ret) << " ret = " << writeValue(ret) << ';';
        }
        for(ParamDeclList::const_iterator i = params.begin(); i != params.end(); ++i)
        {
            if((*i)->isOutParam())
            {
                string name = fixId((*i)->name());
                TypePtr type = (*i)->type();
                _out << nl << typeToString(type) << ' ' << name << " = " << writeValue(type) << ';';
            }
        }
        _out << nl << "return _System.Threading.Tasks.Task.FromResult";
        if(resultType.empty())
        {
            _out << "<_System.Object>(null);";
        }
        else
        {

            bool returnStruct = (op->returnType() && !outParams.empty()) || outParams.size() > 1 || op->hasMarshaledResult();

            if(returnStruct)
            {
                _out << "(new " << resultType;
            }
            _out << spar;
            if(ret)
            {
                _out << "ret";
            }

            for(ParamDeclList::const_iterator i = params.begin(); i != params.end(); ++i)
            {
                if((*i)->isOutParam())
                {
                    _out << fixId((*i)->name());
                }
            }

            if(op->hasMarshaledResult())
            {
                _out << "current";
            }

            _out << epar;
            if(returnStruct)
            {
                _out << ")";
            }
            _out << ";";
        }
        _out << eb;
    }
    else
    {
        string retS = op->hasMarshaledResult() ?
              fixId(cl->scope() + resultStructName(cl->name(), op->name(), true)) :
              typeToString(ret);

        vector<string> pDecls = op->hasMarshaledResult() ? getInParams(op) : getParams(op);

        _out << "public ";
        if(!forTie && !cl->isLocal())
        {
            _out << "override ";
        }
        _out << retS << ' ' << fixId(opName, DotNet::ICloneable, true) << spar << pDecls;
        if(!cl->isLocal())
        {
            _out << "Ice.Current current = null";
        }
        _out << epar;
        if(comment)
        {
            _out << ';';
            return;
        }
        _out << sb;
        if(op->hasMarshaledResult())
        {
            _out << nl << "return new " << fixId(cl->scope() + resultStructName(cl->name(), op->name(), true))
                 << "(";
            if(ret)
            {
                _out << writeValue(ret);
            }
            for(ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end(); ++i)
            {
                if(ret || i != outParams.begin())
                {
                    _out << ", ";
                }
                _out << writeValue((*i)->type());
            }
            _out << ", current);";
        }
        else
        {
            for(ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end(); ++i)
            {
                string name = fixId((*i)->name());
                TypePtr type = (*i)->type();
                _out << nl << name << " = " << writeValue(type) << ';';
            }

            if(ret)
            {
                _out << nl << "return " << writeValue(ret) << ';';
            }
        }
        _out << eb;
    }
}

Slice::Gen::ImplVisitor::ImplVisitor(IceUtilInternal::Output& out) :
    BaseImplVisitor(out)
{
}

bool
Slice::Gen::ImplVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasClassDefs())
    {
        return false;
    }

    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;

    return true;
}

void
Slice::Gen::ImplVisitor::visitModuleEnd(const ModulePtr&)
{
    _out << eb;
}

bool
Slice::Gen::ImplVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->allOperations().size() == 0)
    {
        return false;
    }

    string name = p->name();

    _out << sp << nl << "public class " << name << 'I';
    if(p->isInterface())
    {
        if(p->isLocal())
        {
            _out << " : " << fixId(name);
        }
        else
        {
            _out << " : " << name << "Disp_";
        }
    }
    else
    {
        _out << " : " << fixId(name);
    }
    _out << sb;

    OperationList ops = p->allOperations();
    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        writeOperation(*r, false, false);
    }

    return true;
}

void
Slice::Gen::ImplVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _out << eb;
}

Slice::Gen::ImplTieVisitor::ImplTieVisitor(IceUtilInternal::Output& out)
    : BaseImplVisitor(out)
{
}

bool
Slice::Gen::ImplTieVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasClassDefs())
    {
        return false;
    }

    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;

    return true;
}

void
Slice::Gen::ImplTieVisitor::visitModuleEnd(const ModulePtr&)
{
    _out << eb;
}

bool
Slice::Gen::ImplTieVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal() || !p->isAbstract())
    {
        return false;
    }

    string name = p->name();
    ClassList bases = p->bases();

    //
    // Use implementation inheritance in the following situations:
    //
    // * if a class extends another class
    // * if a class implements a single interface
    // * if an interface extends only one interface
    //
    bool inheritImpl = (!p->isInterface() && !bases.empty() && !bases.front()->isInterface()) || (bases.size() == 1);

    _out << sp << nl << "public class " << name << "I : ";
    if(inheritImpl)
    {
        if(bases.front()->isAbstract())
        {
            _out << bases.front()->name() << 'I';
        }
        else
        {
            _out << fixId(bases.front()->name());
        }
        _out << ", ";
    }
    _out << name << "Operations";
    _out << '_';
    _out << sb;

    _out << nl << "public " << name << "I()";
    _out << sb;
    _out << eb;

    OperationList ops = p->allOperations();
    ops.sort();

    OperationList baseOps;
    if(inheritImpl)
    {
        baseOps = bases.front()->allOperations();
        baseOps.sort();
    }

    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        if(inheritImpl && binary_search(baseOps.begin(), baseOps.end(), *r))
        {
            _out << sp;
            _out << nl << "// ";
            _out << nl << "// Implemented by " << bases.front()->name() << 'I';
            _out << nl << "//";
            writeOperation(*r, true, true);
        }
        else
        {
            writeOperation(*r, false, true);
        }
    }

    _out << eb;

    return true;
}
