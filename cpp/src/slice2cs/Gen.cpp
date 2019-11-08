//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
sliceModeToIceMode(Operation::Mode opMode, string ns)
{
    string mode;
    switch(opMode)
    {
        case Operation::Normal:
        {
            mode = CsGenerator::getUnqualified("Ice.OperationMode.Normal", ns);
            break;
        }
        case Operation::Nonmutating:
        {
            mode = CsGenerator::getUnqualified("Ice.OperationMode.Nonmutating", ns);
            break;
        }
        case Operation::Idempotent:
        {
            mode = CsGenerator::getUnqualified("Ice.OperationMode.Idempotent", ns);
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
opFormatTypeToString(const OperationPtr& op, string ns)
{
    switch (op->format())
    {
        case DefaultFormat:
        {
            return CsGenerator::getUnqualified("Ice.FormatType.DefaultFormat", ns);
        }
        case CompactFormat:
        {
            return CsGenerator::getUnqualified("Ice.FormatType.CompactFormat", ns);
        }
        case SlicedFormat:
        {
            return CsGenerator::getUnqualified("Ice.FormatType.SlicedFormat", ns);
        }
        default:
        {
            assert(false);
        }
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
        out << nl << "[global::System.Obsolete(\"" << reason << "\")]";
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
                                              const string& ns, bool resultStruct, bool publicNames,
                                              const string& customStream)
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
            os << '(' << typeToString(type, ns) << " v) => {" << paramPrefix << param << " = v; }";
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
            writeMarshalUnmarshalCode(_out, type, ns, param, marshal, customStream);
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
            os << '(' << typeToString(ret, ns) << " v) => {" << paramPrefix << returnValueS << " = v; }";
            param = os.str();
        }
        else
        {
            param = paramPrefix + returnValueS;
        }

        if(!op->returnIsOptional())
        {
            writeMarshalUnmarshalCode(_out, ret, ns, param, marshal, customStream);
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
                os << '(' << typeToString(ret, ns) << " v) => {" << paramPrefix << returnValueS << " = v; }";
                param = os.str();
            }
            else
            {
                param = paramPrefix + returnValueS;
            }
            writeOptionalMarshalUnmarshalCode(_out, ret, ns, param, op->returnTag(), marshal, customStream);
            checkReturnType = false;
        }

        string param = paramPrefix.empty() && !publicNames ? "iceP_" + (*pli)->name() : fixId((*pli)->name());
        TypePtr type = (*pli)->type();
        if(!marshal && isClassType(type))
        {
            ostringstream os;
            os << '(' << typeToString(type, ns) << " v) => {" << paramPrefix << param << " = v; }";
            param = os.str();
        }
        else
        {
            param = paramPrefix + param;
        }

        writeOptionalMarshalUnmarshalCode(_out, type, ns, param, (*pli)->tag(), marshal, customStream);
    }

    if(checkReturnType)
    {
        string param;
        if(!marshal && isClassType(ret))
        {
            ostringstream os;
            os << '(' << typeToString(ret, ns) << " v) => {" << paramPrefix << returnValueS << " = v; }";
            param = os.str();
        }
        else
        {
            param = paramPrefix + returnValueS;
        }
        writeOptionalMarshalUnmarshalCode(_out, ret, ns, param, op->returnTag(), marshal, customStream);
    }
}

void
Slice::CsVisitor::writeMarshalDataMember(const DataMemberPtr& member, const string& name, const string& ns,
                                         bool forStruct)
{
    if(member->optional())
    {
        assert(!forStruct);
        writeOptionalMarshalUnmarshalCode(_out, member->type(), ns, name, member->tag(), true, "ostr_");
    }
    else
    {
        string stream = forStruct ? "" : "ostr_";
        string memberName = name;
        if(forStruct)
        {
            memberName = "this." + memberName;
        }

        writeMarshalUnmarshalCode(_out, member->type(), ns, memberName, true, stream);
    }
}

void
Slice::CsVisitor::writeUnmarshalDataMember(const DataMemberPtr& member, const string& name, const string& ns,
                                           bool forStruct)
{
    string param = name;
    if(isClassType(member->type()))
    {
        ostringstream os;
        os << '(' << typeToString(member->type(), ns) << " v) => { this." << name << " = v; }";
        param = os.str();
    }
    else if(forStruct)
    {
        param = "this." + name;
    }

    if(member->optional())
    {
        assert(!forStruct);
        writeOptionalMarshalUnmarshalCode(_out, member->type(), ns, param, member->tag(), false, "istr_");
    }
    else
    {
        writeMarshalUnmarshalCode(_out, member->type(), ns, param, false, forStruct ? "" : "istr_");
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
            string ns = getNamespace(p);
            string name = getDispatchParams(*i, retS, params, args, ns);
            _out << sp << nl << "public abstract " << retS << " " << name << spar << params << epar << ';';
        }
    }
}

void
Slice::CsVisitor::writeDispatch(const ClassDefPtr& p)
{
    string name = fixId(p->name());
    string scoped = p->scoped();
    string ns = getNamespace(p);
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
    _out << nl << "public override bool ice_isA(string s, " << getUnqualified("Ice.Current", ns) << " current = null)";
    _out << sb;
    _out << nl << "return global::System.Array.BinarySearch(_ids, s, IceUtilInternal.StringUtil.OrdinalStringComparer) >= 0;";
    _out << eb;

    _out << sp;
    if(!p->isInterface())
    {
        emitGeneratedCodeAttribute();
    }
    _out << nl << "public override string[] ice_ids(" << getUnqualified("Ice.Current", ns) << " current = null)";
    _out << sb;
    _out << nl << "return _ids;";
    _out << eb;

    _out << sp;
    if(!p->isInterface())
    {
        emitGeneratedCodeAttribute();
    }
    _out << nl << "public override string ice_id(" << getUnqualified("Ice.Current", ns) << " current = null)";
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

    OperationList ops = p->operations();

    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        ContainerPtr container = op->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        assert(cl);

        string opName = op->name();
        _out << sp;
        _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Design\", \"CA1011\")]";
        if(!p->isInterface())
        {
            emitGeneratedCodeAttribute();
        }
        _out << nl << "public static global::System.Threading.Tasks.Task<" << getUnqualified("Ice.OutputStream", ns) << ">";
        _out << nl << "iceD_" << opName << "(" << name << (p->isInterface() ? "" : "Disp_") << " obj, "
             <<  "global::IceInternal.Incoming inS, " << getUnqualified("Ice.Current", ns) << " current)";
        _out << sb;

        TypePtr ret = op->returnType();
        ParamDeclList inParams = op->inParameters();
        ParamDeclList outParams = op->outParameters();

        _out << nl << getUnqualified("Ice.ObjectImpl", ns) << ".iceCheckMode(" << sliceModeToIceMode(op->mode(), ns)
             << ", current.mode);";
        if(!inParams.empty())
        {
            //
            // Unmarshal 'in' parameters.
            //
            _out << nl << "var istr = inS.startReadParams();";
            for(ParamDeclList::const_iterator pli = inParams.begin(); pli != inParams.end(); ++pli)
            {
                string param = "iceP_" + (*pli)->name();
                string typeS = typeToString((*pli)->type(), ns, (*pli)->optional());
                const bool isClass = isClassType((*pli)->type());

                if((*pli)->optional())
                {
                    _out << nl << typeS << ' ' << param;
                    if(isClass)
                    {
                        _out << " = " << getUnqualified("Ice.Util", ns) << ".None";
                    }
                    _out << ';';
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
            writeMarshalUnmarshalParams(inParams, 0, false, ns);
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
            _out << nl << "inS.setFormat(" << opFormatTypeToString(op, ns) << ");";
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
            string retS = resultType(op, ns);
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
                writeMarshalUnmarshalParams(outParams, op, true, ns, true);
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
                string typeS = typeToString((*pli)->type(), ns, (*pli)->optional());
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
                writeMarshalUnmarshalParams(outParams, op, true, ns);
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
        _out << nl << "public override global::System.Threading.Tasks.Task<"
             << getUnqualified("Ice.OutputStream", ns) << ">";
        _out << nl << "iceDispatch(global::IceInternal.Incoming inS, "
             << getUnqualified("Ice.Current", ns) << " current)";
        _out << sb;
        _out << nl << "int pos = global::System.Array.BinarySearch(_all, current.operation, "
             << "global::IceUtilInternal.StringUtil.OrdinalStringComparer);";
        _out << nl << "if(pos < 0)";
        _out << sb;
        _out << nl << "throw new " << getUnqualified("Ice.OperationNotExistException", ns)
             << "(current.id, current.facet, current.operation);";
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
                _out << nl << "return " << getUnqualified("Ice.ObjectImpl", ns)
                     << ".iceD_ice_id(this, inS, current);";
            }
            else if(opName == "ice_ids")
            {
                _out << nl << "return " << getUnqualified("Ice.ObjectImpl", ns)
                     << ".iceD_ice_ids(this, inS, current);";
            }
            else if(opName == "ice_isA")
            {
                _out << nl << "return " << getUnqualified("Ice.ObjectImpl", ns)
                     << ".iceD_ice_isA(this, inS, current);";
            }
            else if(opName == "ice_ping")
            {
                _out << nl << "return " << getUnqualified("Ice.ObjectImpl", ns)
                     << ".iceD_ice_ping(this, inS, current);";
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
                            _out << nl << "return " << getUnqualified(cl, ns, "", "Disp_")
                                 << ".iceD_" << opName << "(this, inS, current);";
                        }
                        break;
                    }
                }
            }
            _out << eb;
        }
        _out << eb;
        _out << sp << nl << "global::System.Diagnostics.Debug.Assert(false);";
        _out << nl << "throw new " << getUnqualified("Ice.OperationNotExistException", ns)
             << "(current.id, current.facet, current.operation);";
        _out << eb;
    }
}

void
Slice::CsVisitor::writeMarshaling(const ClassDefPtr& p)
{
    string name = fixId(p->name());
    string scoped = p->scoped();
    string ns = getNamespace(p);
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

    if(preserved && !basePreserved)
    {
        _out << sp;
        if(!p->isInterface())
        {
            emitGeneratedCodeAttribute();
        }
        _out << nl << "public override " << getUnqualified("Ice.SlicedData", ns) << " ice_getSlicedData()";
        _out << sb;
        _out << nl << "return iceSlicedData_;";
        _out << eb;

        _out << sp;
        if(!p->isInterface())
        {
            emitGeneratedCodeAttribute();
        }
        _out << nl << "public override void iceWrite(" << getUnqualified("Ice.OutputStream", ns) << " ostr_)";
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
        _out << nl << "public override void iceRead(" << getUnqualified("Ice.InputStream", ns) << " istr_)";
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
    _out << nl << "protected override void iceWriteImpl(" << getUnqualified("Ice.OutputStream", ns) << " ostr_)";
    _out << sb;
    _out << nl << "ostr_.startSlice(ice_staticId(), " << p->compactId() << (!base ? ", true" : ", false") << ");";
    for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
    {
        if(!(*d)->optional())
        {
            writeMarshalDataMember(*d, fixId(*d, DotNet::ICloneable, true), ns);
        }
    }
    for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
    {
        writeMarshalDataMember(*d, fixId(*d, DotNet::ICloneable, true), ns);
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
    _out << nl << "protected override void iceReadImpl(" << getUnqualified("Ice.InputStream", ns) << " istr_)";
    _out << sb;
    _out << nl << "istr_.startSlice();";
    for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
    {
        if(!(*d)->optional())
        {
            writeUnmarshalDataMember(*d, fixId(*d, DotNet::ICloneable, true), ns);
        }
    }
    for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
    {
        writeUnmarshalDataMember(*d, fixId(*d, DotNet::ICloneable, true), ns);
    }
    _out << nl << "istr_.endSlice();";
    if(base)
    {
        _out << nl << "base.iceReadImpl(istr_);";
    }
    _out << eb;

    if(preserved && !basePreserved)
    {
        _out << sp << nl << "protected " << getUnqualified("Ice.SlicedData", ns) << " iceSlicedData_;";
    }
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
Slice::CsVisitor::getParams(const OperationPtr& op, const string& ns)
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
        param += typeToString((*q)->type(), ns, (*q)->optional()) + " " + fixId((*q)->name());
        params.push_back(param);
    }
    return params;
}

vector<string>
Slice::CsVisitor::getInParams(const OperationPtr& op, const string& ns, bool internal)
{
    vector<string> params;

    string name = fixId(op->name());
    ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container()); // Get the class containing the op.
    ParamDeclList paramList = op->inParameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        params.push_back(getParamAttributes(*q) + typeToString((*q)->type(), ns, (*q)->optional())
                         + " " + (internal ? "iceP_" + (*q)->name() : fixId((*q)->name())));
    }
    return params;
}

vector<string>
Slice::CsVisitor::getOutParams(const OperationPtr& op, const string& ns, bool returnParam, bool outKeyword)
{
    vector<string> params;
    if(returnParam)
    {
        TypePtr ret = op->returnType();
        if(ret)
        {
            params.push_back(typeToString(ret, ns, op->returnIsOptional()) + " ret");
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
        s += typeToString((*q)->type(), ns, (*q)->optional()) + ' ' + fixId((*q)->name());
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
Slice::CsVisitor::getDispatchParams(const OperationPtr& op, string& retS, vector<string>& params, vector<string>& args,
                                    const string& ns)
{
    string name;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container()); // Get the class containing the op.
    ParamDeclList paramDecls;

    if(cl->hasMetaData("amd") || op->hasMetaData("amd"))
    {
        name = op->name() + "Async";
        params = getInParams(op, ns);
        args = getInArgs(op);
        paramDecls = op->inParameters();
        retS = taskResultType(op, ns, true);
    }
    else if(op->hasMarshaledResult())
    {
        name = fixId(op->name(), DotNet::ICloneable, true);
        params = getInParams(op, ns);
        args = getInArgs(op);
        paramDecls = op->inParameters();
        retS = resultType(op, ns, true);
    }
    else
    {
        name = fixId(op->name(), DotNet::ICloneable, true);
        params = getParams(op, ns);
        args = getArgs(op);
        paramDecls = op->parameters();
        retS = typeToString(op->returnType(), ns, op->returnIsOptional());
    }

    string currentParamName = getEscapedParamName(op, "current");
    params.push_back(getUnqualified("Ice.Current", ns) + " " + currentParamName + " = null");
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
    _out << nl << "[global::System.Runtime.InteropServices.ComVisible(false)]";
}

void
Slice::CsVisitor::emitGeneratedCodeAttribute()
{
    _out << nl << "[global::System.CodeDom.Compiler.GeneratedCodeAttribute(\"slice2cs\", \"" << ICE_STRING_VERSION << "\")]";
}

void
Slice::CsVisitor::emitPartialTypeAttributes()
{
    //
    // We are not supposed to mark an entire partial type with GeneratedCodeAttribute, therefore
    // FxCop may complain about naming convention violations. These attributes suppress those
    // warnings, but only when the generated code is compiled with /define:CODE_ANALYSIS.
    //
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1704\")]";
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1707\")]";
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1709\")]";
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1710\")]";
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1711\")]";
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1715\")]";
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1716\")]";
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1720\")]";
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1722\")]";
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Naming\", \"CA1724\")]";
}

string
Slice::CsVisitor::writeValue(const TypePtr& type, const string& ns)
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
        return typeToString(type, ns) + "." + fixId((*en->enumerators().begin())->name());
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(st->hasMetaData("cs:class"))
        {
            return "null";
        }
        else
        {
            return "new " + typeToString(type, ns) + "()";
        }
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
Slice::CsVisitor::writeDataMemberInitializers(const DataMemberList& members, const string& ns, unsigned int baseTypes,
                                              bool propertyMapping)
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
            _out << nl << "this." << fixId((*p)->name(), baseTypes) << " = new "
                 << typeToString((*p)->type(), ns, true) << "();";
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
                _out << nl << "this." << fixId((*p)->name(), baseTypes) << " = new " << typeToString(st, ns, false)
                     << "();";
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

    const string paramTag = "@param";
    const string throwsTag = "@throws";
    const string exceptionTag = "@exception";
    const string returnTag = "@return";

    unsigned int i;

    for(i = 0; i < s.size(); ++i)
    {
        if(s[i] == '.' && (i + 1 >= s.size() || isspace(static_cast<unsigned char>(s[i + 1]))))
        {
            ++i;
            break;
        }
        else if(s[i] == '@' && (s.substr(i, paramTag.size()) == paramTag ||
                                s.substr(i, throwsTag.size()) == throwsTag ||
                                s.substr(i, exceptionTag.size()) == exceptionTag ||
                                s.substr(i, returnTag.size()) == returnTag))
        {
            break;
        }
    }

    summaryLines = splitIntoLines(trim(s.substr(0, i)));
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
Slice::CsVisitor::writeDocCommentParam(const OperationPtr& p, ParamDir paramType, bool /*amd*/)
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

void
Slice::CsVisitor::moduleStart(const ModulePtr& p)
{
    if(!ContainedPtr::dynamicCast(p->container()))
    {
        string ns = getNamespacePrefix(p);
        string name = fixId(p->name());
        if(!ns.empty())
        {
            _out << sp;
            _out << nl << "namespace " << ns;
            _out << sb;
        }
    }
}

void
Slice::CsVisitor::moduleEnd(const ModulePtr& p)
{
    if(!ContainedPtr::dynamicCast(p->container()))
    {
        if(!getNamespacePrefix(p).empty())
        {
            _out << eb;
        }
    }
}

Slice::Gen::Gen(const string& base, const vector<string>& includePaths, const string& dir, bool impl) :
    _includePaths(includePaths)
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
        os << "cannot open `" << file << "': " << IceUtilInternal::errorToString(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }
    FileTracker::instance()->addFile(file);
    printHeader();

    printGeneratedHeader(_out, fileBase + ".ice");

    _out << sp << nl << "using _System = global::System;";

    _out << sp << nl << "#pragma warning disable 1591"; // See bug 3654

    if(impl)
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
            os << ": cannot open `" << fileImpl << "': " << IceUtilInternal::errorToString(errno);
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

    TypeIdVisitor typeIdVisitor(_out);
    p->visit(&typeIdVisitor, false);

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

    DispatcherVisitor dispatcherVisitor(_out);
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
Slice::Gen::closeOutput()
{
    _out.close();
    _impl.close();
}

void
Slice::Gen::printHeader()
{
    static const char* header =
"//\n"
"// Copyright (c) ZeroC, Inc. All rights reserved.\n"
"//\n"
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
Slice::Gen::CompactIdVisitor::visitUnitStart(const UnitPtr& p)
{
    if(p->hasCompactTypeId())
    {
        string typeIdNs = getCustomTypeIdNamespace(p);

        if(typeIdNs.empty())
        {
            // TODO: replace by namespace Ice.TypeId, see issue #239
            //
            _out << sp << nl << "namespace IceCompactId";
        }
        else
        {
            _out << sp << nl << "namespace " << typeIdNs;
        }

        _out << sb;
        return true;
    }
    return false;
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
        //
        // TODO: rename to class Compact_Xxx, see issue #239
        //
        _out << nl << "public sealed class TypeId_" << p->compactId();
        _out << sb;
        _out << nl << "public const string typeId = \"" << p->scoped() << "\";";
        _out << eb;
    }
    return false;
}

Slice::Gen::TypeIdVisitor::TypeIdVisitor(IceUtilInternal::Output& out) :
    CsVisitor(out)
{
}

bool
Slice::Gen::TypeIdVisitor::visitModuleStart(const ModulePtr& p)
{
    string ns = getNamespacePrefix(p);

    if(!ns.empty() && (p->hasValueDefs() || p->hasNonLocalExceptions()))
    {
        string name = fixId(p->name());
        if(!ContainedPtr::dynamicCast(p->container()))
        {
            // Top-level module
            //
            string typeIdNs = getCustomTypeIdNamespace(p->unit());
            if(typeIdNs.empty())
            {
                typeIdNs = "Ice.TypeId";
            }

            name = typeIdNs + "." + name;
        }
        _out << sp << nl << "namespace " << name;
        _out << sb;
        return true;
    }
    return false;
}

void
Slice::Gen::TypeIdVisitor::visitModuleEnd(const ModulePtr&)
{
    _out << eb;
}

bool
Slice::Gen::TypeIdVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(!p->isInterface())
    {
        generateHelperClass(p);
    }
    return false;
}

bool
Slice::Gen::TypeIdVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    generateHelperClass(p);
    return false;
}

void
Slice::Gen::TypeIdVisitor::generateHelperClass(const ContainedPtr& p)
{
    string name = fixId(p->name());
    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public abstract class " << name;
    _out << sb;
    _out << nl << "public abstract global::" << getNamespace(p) << "." << name << " targetClass { get; }";
    _out << eb;
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

    moduleStart(p);
    string name = fixId(p->name());
    _out << sp;
    emitAttributes(p);
    _out << nl << "namespace " << name;

    _out << sb;

    return true;
}

void
Slice::Gen::TypesVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
    moduleEnd(p);
}

bool
Slice::Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = p->name();
    string scoped = fixId(p->scoped());
    string ns = getNamespace(p);
    ClassList bases = p->bases();
    bool hasBaseClass = !bases.empty() && !bases.front()->isInterface();

    StringList baseNames;

    _out << sp;
    emitAttributes(p);

    if(p->isInterface())
    {
        emitComVisibleAttribute();
        emitPartialTypeAttributes();
        _out << nl << "public partial interface " << fixId(name);
        baseNames.push_back(getUnqualified("Ice.Object", ns));
        baseNames.push_back(name + "Operations_");
        for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
        {
            baseNames.push_back(getUnqualified(*q, ns));
        }
    }
    else
    {
        emitComVisibleAttribute();
        emitPartialTypeAttributes();
        _out << nl << "[global::System.Serializable]";
        if(p->allOperations().size() > 0) // See bug 4747
        {
            _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Design\", \"CA1012\")]";
        }
        _out << nl << "public ";
        if(p->allOperations().size() > 0) // Don't use isAbstract() here - see bug 3739
        {
            _out << "abstract ";
        }
        _out << "partial class " << fixId(name);

        if(!hasBaseClass)
        {
            baseNames.push_back(getUnqualified("Ice.Value", ns));
        }
        else
        {
            baseNames.push_back(getUnqualified(bases.front(), ns));
            bases.pop_front();
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
    return true;
}

void
Slice::Gen::TypesVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
    DataMemberList classMembers = p->classDataMembers();
    DataMemberList allClassMembers = p->allClassDataMembers();
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList allDataMembers = p->allDataMembers();
    ClassList bases = p->bases();
    bool hasBaseClass = !bases.empty() && !bases.front()->isInterface();

    if(!p->isInterface())
    {
        const bool isAbstract = p->isAbstract();

        _out << sp << nl << "partial void ice_initialize();";
        if(allDataMembers.empty())
        {
            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << (isAbstract ? "protected " : "public ") << name << spar << epar;
            _out << sb;
            _out << nl << "ice_initialize();";
            _out << eb;
        }
        else
        {
            const bool propertyMapping = p->hasMetaData("cs:property");

            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << (isAbstract ? "protected " : "public ") << name << spar << epar;
            if(hasBaseClass)
            {
                _out << " : base()";
            }
            _out << sb;
            writeDataMemberInitializers(dataMembers, ns, DotNet::ICloneable, propertyMapping);
            _out << nl << "ice_initialize();";
            _out << eb;

            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << (isAbstract ? "protected " : "public ") << name << spar;
            vector<string> paramDecl;
            for(DataMemberList::const_iterator d = allDataMembers.begin(); d != allDataMembers.end(); ++d)
            {
                string memberName = fixId((*d)->name(), DotNet::ICloneable);
                string memberType = typeToString((*d)->type(), ns, (*d)->optional());
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
            _out << nl << "ice_initialize();";
            _out << eb;
        }
    }

    if(!p->isInterface())
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
Slice::Gen::TypesVisitor::visitSequence(const SequencePtr&)
{
    //
    // No need to generate anything for sequences.
    //
}

bool
Slice::Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
    ExceptionPtr base = p->base();

    _out << sp;
    writeDocComment(p, getDeprecateReason(p, 0, "type"));
    emitDeprecate(p, 0, _out, "type");
    emitAttributes(p);
    emitComVisibleAttribute();
    //
    // Suppress FxCop diagnostic about a missing constructor MyException(String).
    //
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Design\", \"CA1032\")]";
    _out << nl << "[global::System.Serializable]";

    emitPartialTypeAttributes();
    _out << nl << "public partial class " << name << " : ";
    if(base)
    {
        _out << getUnqualified(base, ns);
    }
    else
    {
        _out << getUnqualified("Ice.UserException", ns);
    }
    _out << sb;
    return true;
}

void
Slice::Gen::TypesVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
    DataMemberList allDataMembers = p->allDataMembers();
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList allClassMembers = p->allClassDataMembers();
    DataMemberList classMembers = p->classDataMembers();
    DataMemberList optionalMembers = p->orderedOptionalDataMembers();

    vector<string> allParamDecl;
    for(DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name());
        string memberType = typeToString((*q)->type(), ns, (*q)->optional());
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
        string memberType = typeToString((*q)->type(), ns, (*q)->optional());
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

    const bool hasDataMemberInitializers = requiresDataMemberInitializers(dataMembers);
    if(hasDataMemberInitializers)
    {
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "private void _initDM()";
        _out << sb;
        writeDataMemberInitializers(dataMembers, ns, DotNet::Exception);
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
    _out << nl << "public " << name << "(global::System.Exception ex) : base(ex)";
    _out << sb;
    if(hasDataMemberInitializers)
    {
        _out << nl << "_initDM();";
    }
    _out << eb;
    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public " << name << "(global::System.Runtime.Serialization.SerializationInfo info, "
         << "global::System.Runtime.Serialization.StreamingContext context) : base(info, context)";
    _out << sb;
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name(), DotNet::Exception, false);
        writeSerializeDeserializeCode(_out, (*q)->type(), ns, memberName, (*q)->optional(), (*q)->tag(), false);
    }
    _out << eb;

    if(!allDataMembers.empty())
    {
        if(!dataMembers.empty())
        {
            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << "private void _initDM" << spar << paramDecl << epar;
            _out << sb;
            for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
            {
                string memberName = fixId((*q)->name(), DotNet::Exception, false);
                _out << nl << "this." << memberName << " = " << fixId((*q)->name()) << ';';
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
        exceptionDecl.push_back("global::System.Exception " + exParam);
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

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public override string ice_id()";
    _out << sb;
    _out << nl << "return \"" << p->scoped() << "\";";
    _out << eb;

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
    _out << nl << "global::IceInternal.HashUtil.hashAdd(ref h_, \"" << p->scoped() << "\");";
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
        _out << nl << "public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, "
             << "global::System.Runtime.Serialization.StreamingContext context)";
        _out << sb;
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            string memberName = fixId((*q)->name(), DotNet::Exception, false);
            writeSerializeDeserializeCode(_out, (*q)->type(), ns, memberName, (*q)->optional(), (*q)->tag(), true);
        }
        _out << sp << nl << "base.GetObjectData(info, context);";
        _out << eb;
    }

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

    string scoped = p->scoped();
    ExceptionPtr base = p->base();

    const bool basePreserved = p->inheritsMetaData("preserve-slice");
    const bool preserved = p->hasMetaData("preserve-slice");

    if(preserved && !basePreserved)
    {
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public override " << getUnqualified("Ice.SlicedData", ns) << " ice_getSlicedData()";
        _out << sb;
        _out << nl << "return slicedData_;";
        _out << eb;

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public override void iceWrite(" << getUnqualified("Ice.OutputStream", ns) << " ostr_)";
        _out << sb;
        _out << nl << "ostr_.startException(slicedData_);";
        _out << nl << "iceWriteImpl(ostr_);";
        _out << nl << "ostr_.endException();";
        _out << eb;

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public override void iceRead(" << getUnqualified("Ice.InputStream", ns) << " istr_)";
        _out << sb;
        _out << nl << "istr_.startException();";
        _out << nl << "iceReadImpl(istr_);";
        _out << nl << "slicedData_ = istr_.endException(true);";
        _out << eb;
    }

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "protected override void iceWriteImpl(" << getUnqualified("Ice.OutputStream", ns) << " ostr_)";
    _out << sb;
    _out << nl << "ostr_.startSlice(\"" << scoped << "\", -1, " << (!base ? "true" : "false") << ");";
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        writeMarshalDataMember(*q, fixId((*q)->name(), DotNet::Exception), ns);
    }
    _out << nl << "ostr_.endSlice();";
    if(base)
    {
        _out << nl << "base.iceWriteImpl(ostr_);";
    }
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "protected override void iceReadImpl(" << getUnqualified("Ice.InputStream", ns) << " istr_)";
    _out << sb;
    _out << nl << "istr_.startSlice();";

    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        writeUnmarshalDataMember(*q, fixId((*q)->name(), DotNet::Exception), ns);
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
        _out << sp << nl << "protected " << getUnqualified("Ice.SlicedData", ns) << " slicedData_;";
    }

    _out << eb;
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
    _out << sp;

    emitDeprecate(p, 0, _out, "type");

    emitAttributes(p);
    emitPartialTypeAttributes();
    _out << nl << "[global::System.Serializable]";
    _out << nl << "public partial " << (isValueType(p) ? "struct" : "class") << ' ' << name;

    StringList baseNames;
    if(!isValueType(p))
    {
        baseNames.push_back("System.ICloneable");
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
            _out << getUnqualified(*q, ns);
        }
    }

    _out << sb;
    return true;
}

void
Slice::Gen::TypesVisitor::visitStructEnd(const StructPtr& p)
{
    string name = fixId(p->name());
    string scope = fixId(p->scope());
    string ns = getNamespace(p);
    DataMemberList classMembers = p->classDataMembers();
    DataMemberList dataMembers = p->dataMembers();

    const bool propertyMapping = p->hasMetaData("cs:property");
    const bool isClass = !isValueType(p);
    _out << sp << nl << "partial void ice_initialize();";
    if(isClass)
    {
        //
        // Default values for struct data members are only generated if the struct
        // is mapped to a C# class. We cannot generate a parameterless constructor
        // or assign default values to data members if the struct maps to a value
        // type (a C# struct) instead.
        //
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public " << name << "()";
        _out << sb;
        writeDataMemberInitializers(dataMembers, ns, DotNet::ICloneable, propertyMapping);
        _out << nl << "ice_initialize();";
        _out << eb;
    }

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public " << name << spar;
    vector<string> paramDecl;
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name(), isClass ? DotNet::ICloneable : 0);
        string memberType = typeToString((*q)->type(), ns);
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
    _out << nl << "ice_initialize();";
    _out << eb;

    if(isClass)
    {
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public object Clone()";
        _out << sb;
        _out << nl << "return MemberwiseClone();";
        _out << eb;
    }

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public override int GetHashCode()";
    _out << sb;
    _out << nl << "int h_ = 5381;";
    _out << nl << "global::IceInternal.HashUtil.hashAdd(ref h_, \"" << p->scoped() << "\");";
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

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public void ice_writeMembers(" << getUnqualified("Ice.OutputStream", ns) << " ostr)";
    _out << sb;
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        writeMarshalDataMember(*q, fixId(*q, isClass ? DotNet::ICloneable : 0), ns, true);
    }
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public void ice_readMembers(" << getUnqualified("Ice.InputStream", ns) << " istr)";
    _out << sb;
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        writeUnmarshalDataMember(*q, fixId(*q, isClass ? DotNet::ICloneable : 0), ns, true);
    }
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public static void ice_write(" << getUnqualified("Ice.OutputStream", ns) << " ostr, " << name
         << " v)";
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
    _out << nl << "public static " << name << " ice_read(" << getUnqualified("Ice.InputStream", ns) << " istr)";
    _out << sb;
    _out << nl << "var v = new " << name << "();";
    _out << nl << "v.ice_readMembers(istr);";
    _out << nl << "return v;";
    _out << eb;

    if(isClass)
    {
        _out << sp << nl << "private static readonly " << name << " _nullMarshalValue = new " << name << "();";
    }

    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitDictionary(const DictionaryPtr&)
{
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
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

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public sealed class " << p->name() << "Helper";
    _out << sb;
    _out << sp;
    _out << nl << "public static void write(" << getUnqualified("Ice.OutputStream", ns) << " ostr, " << name
         << " v)";
    _out << sb;
    writeMarshalUnmarshalCode(_out, p, ns, "v", true);
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public static " << name << " read(" << getUnqualified("Ice.InputStream", ns) << " istr)";
    _out << sb;
    _out << nl << name << " v;";
    writeMarshalUnmarshalCode(_out, p, ns, "v", false);
    _out << nl << "return v;";
    _out << eb;

    _out << eb;
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
    _out << sp << nl << "public const " << typeToString(p->type(), "") << " value = ";
    writeConstantValue(p->type(), p->valueType(), p->value());
    _out << ";";
    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitDataMember(const DataMemberPtr& p)
{
    unsigned int baseTypes = 0;
    bool isClass = false;
    bool isProperty = false;
    bool isValue = false;
    bool isProtected = false;
    bool isPrivate = false;
    const bool isOptional = p->optional();
    ContainedPtr cont = ContainedPtr::dynamicCast(p->container());
    assert(cont);

    StructPtr st = StructPtr::dynamicCast(cont);
    ExceptionPtr ex = ExceptionPtr::dynamicCast(cont);
    ClassDefPtr cl = ClassDefPtr::dynamicCast(cont);
    string ns = getNamespace(cont);
    if(st)
    {
        isValue = isValueType(st);
        if(!isValue)
        {
            baseTypes = DotNet::ICloneable;
        }
        if(cont->hasMetaData("cs:property"))
        {
            isProperty = true;
        }
        //
        // C# structs are implicit sealed and cannot use `protected' modifier,
        // we must use either public or private. For Slice structs using the
        // class mapping we can still use protected modifier.
        //
        if(cont->hasMetaData("protected") || p->hasMetaData("protected"))
        {
            if(isValue)
            {
                isPrivate = true;
            }
            else
            {
                isProtected = true;
            }
        }
    }
    else if(ex)
    {
        baseTypes = DotNet::Exception;
    }
    else
    {
        assert(cl);
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

    string type = typeToString(p->type(), ns, isOptional);
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
    else
    {
        emitAttributes(p);
        emitGeneratedCodeAttribute();
        if(isPrivate)
        {
            _out << nl << "private";
        }
        else if(isProtected)
        {
            _out << nl << "protected";
        }
        else
        {
            _out << nl << "public";
        }
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
        if(isPrivate)
        {
            _out << nl << "private";
        }
        else if(isProtected)
        {
            _out << nl << "protected";
        }
        else
        {
            _out << nl << "public";
        }

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
Slice::Gen::TypesVisitor::writeMemberHashCode(const DataMemberList& dataMembers, unsigned int baseTypes)
{
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        _out << nl << "global::IceInternal.HashUtil.hashAdd(ref h_, " << fixId((*q)->name(), baseTypes);
        if((*q)->optional())
        {
            _out << ".Value";
        }
        _out << ");";
    }
}

void
Slice::Gen::TypesVisitor::writeMemberEquals(const DataMemberList& dataMembers, unsigned int baseTypes)
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
                    _out << nl << "if(!global::IceUtilInternal.Collections.SequenceEquals(this." << memberName << ", o."
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
                    _out << nl << "if(!global::IceUtilInternal.Collections.DictionaryEquals(this." << memberName << ", o."
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

namespace
{

bool
hasResultType(const ModulePtr& p)
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
                return true;
            }
        }
    }

    ModuleList modules = p->modules();
    for(ModuleList::const_iterator i = modules.begin(); i != modules.end(); ++i)
    {
        if(hasResultType(*i))
        {
            return true;
        }
    }

    return false;
}

}

bool
Slice::Gen::ResultVisitor::visitModuleStart(const ModulePtr& p)
{
    if(hasResultType(p))
    {
        moduleStart(p);
        _out << sp << nl << "namespace " << fixId(p->name());
        _out << sb;
        return true;
    }
    return false;
}

void
Slice::Gen::ResultVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
    moduleEnd(p);
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
    string ns = getNamespace(cl);
    ParamDeclList outParams = p->outParameters();
    TypePtr ret = p->returnType();

    if(outParams.size() > 1 || (ret && outParams.size() > 0))
    {
        string name = resultStructName(cl->name(), p->name());

        string retS;
        string retSName;
        if(ret)
        {
            retS = typeToString(ret, ns, p->returnIsOptional());
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
            _out << (typeToString((*i)->type(), ns, (*i)->optional()) + " " + fixId((*i)->name()));
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
            _out << nl << "public " << typeToString((*i)->type(), ns, (*i)->optional()) << " " << fixId((*i)->name())
                 << ";";
        }
        _out << eb;
    }

    if(p->hasMarshaledResult())
    {
        string name = resultStructName(cl->name(), p->name(), true);

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public struct " << name << " : " << getUnqualified("Ice.MarshaledResult", ns);
        _out << sb;

        //
        // One shot constructor
        //
        _out << nl << "public " << name << spar << getOutParams(p, ns, true, false)
             << getUnqualified("Ice.Current", ns) + " current" << epar;
        _out << sb;
        _out << nl << "_ostr = global::IceInternal.Incoming.createResponseOutputStream(current);";
        _out << nl << "_ostr.startEncapsulation(current.encoding, " << opFormatTypeToString(p, ns) << ");";
        writeMarshalUnmarshalParams(outParams, p, true, ns, false, true, "_ostr");
        if(p->returnsClasses(false))
        {
            _out << nl << "_ostr.writePendingValues();";
        }
        _out << nl << "_ostr.endEncapsulation();";
        _out << eb;
        _out << sp;
        _out << nl << "public " << getUnqualified("Ice.OutputStream", ns) << " getOutputStream("
             << getUnqualified("Ice.Current", ns) << " current)";
        _out << sb;
        _out << nl << "if(_ostr == null)";
        _out << sb;
        _out << nl << "return new " << name << spar;
        if(ret)
        {
            _out << writeValue(ret, ns);
        }
        for(ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end(); ++i)
        {
            _out << writeValue((*i)->type(), ns);
        }
        _out << "current" << epar << ".getOutputStream(current);";
        _out << eb;
        _out << nl << "return _ostr;";
        _out << eb;
        _out << sp;
        _out << nl << "private " << getUnqualified("Ice.OutputStream", ns) << " _ostr;";
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
    if(!p->hasNonLocalAbstractClassDefs())
    {
        return false;
    }

    moduleStart(p);
    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::ProxyVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
    moduleEnd(p);
}

bool
Slice::Gen::ProxyVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(!p->isInterface() && p->allOperations().size() == 0)
    {
        return false;
    }

    string name = p->name();
    string ns = getNamespace(p);
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
            baseInterfaces.push_back(getUnqualified(*q, ns, "", "Prx"));
        }
    }

    if(baseInterfaces.empty())
    {
        baseInterfaces.push_back(getUnqualified("Ice.ObjectPrx", ns));
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
    string ns = getNamespace(cl);
    string name = fixId(p->name(), DotNet::ICloneable, true);
    vector<string> inParams = getInParams(p, ns);
    ParamDeclList inParamDecls = p->inParameters();
    string retS = typeToString(p->returnType(), ns, p->returnIsOptional());
    string deprecateReason = getDeprecateReason(p, cl, "operation");

    {
        //
        // Write the synchronous version of the operation.
        //
        string context = getEscapedParamName(p, "context");
        _out << sp;
        writeDocComment(p, deprecateReason,
            "<param name=\"" + context + "\">The Context map to send with the invocation.</param>");
        if(!deprecateReason.empty())
        {
            _out << nl << "[global::System.Obsolete(\"" << deprecateReason << "\")]";
        }
        _out << nl << retS << " " << name << spar << getParams(p, ns)
             << (getUnqualified("Ice.OptionalContext", ns) + " " + context + " = new " +
                 getUnqualified("Ice.OptionalContext", ns) + "()") << epar << ';';
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
            _out << nl << "[global::System.Obsolete(\"" << deprecateReason << "\")]";
        }
        _out << nl << taskResultType(p, ns);
        _out << " " << p->name() << "Async" << spar << inParams
             << (getUnqualified("Ice.OptionalContext", ns) + " " + context + " = new " +
                 getUnqualified("Ice.OptionalContext", ns) + "()")
             << ("global::System.IProgress<bool> " + progress + " = null")
             << ("global::System.Threading.CancellationToken " + cancel + " = new global::System.Threading.CancellationToken()")
             << epar << ";";
    }
}

Slice::Gen::AsyncDelegateVisitor::AsyncDelegateVisitor(IceUtilInternal::Output& out)
    : CsVisitor(out)
{
}

bool
Slice::Gen::AsyncDelegateVisitor::visitModuleStart(const ModulePtr& p)
{
    if(p->hasOperations())
    {
        moduleStart(p);
        _out << sp << nl << "namespace " << fixId(p->name());
        _out << sb;
        return true;
    }
    return false;
}

void
Slice::Gen::AsyncDelegateVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
    moduleEnd(p);
}

bool
Slice::Gen::AsyncDelegateVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    return p->hasOperations();
}

void
Slice::Gen::AsyncDelegateVisitor::visitClassDefEnd(const ClassDefPtr&)
{
}

void
Slice::Gen::AsyncDelegateVisitor::visitOperation(const OperationPtr& p)
{
    ClassDefPtr cl = ClassDefPtr::dynamicCast(p->container());

    string ns = getNamespace(cl);
    vector<string> paramDeclAMI = getOutParams(p, ns, false, false);
    string retS = typeToString(p->returnType(), ns, p->returnIsOptional());
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
    if(!p->hasNonLocalAbstractClassDefs())
    {
        return false;
    }
    moduleStart(p);
    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::OpsVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
    moduleEnd(p);
}

bool
Slice::Gen::OpsVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    //
    // Don't generate Operations interfaces for non-abstract classes.
    //
    if(!p->isAbstract())
    {
        return false;
    }
    string name = p->name();
    string ns = getNamespace(p);
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
                _out << getUnqualified(*q, ns, "", "Operations_");
            }
            ++q;
        }
    }
    _out << sb;

    OperationList ops = p->operations();
    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        bool amd = (p->hasMetaData("amd") || op->hasMetaData("amd"));
        string retS;
        vector<string> params, args;
        string opName = getDispatchParams(op, retS, params, args, ns);
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
        _out << nl << retS << " " << opName << spar << params << epar << ";";
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
    if(!p->hasNonLocalAbstractClassDefs() && !p->hasNonLocalSequences() && !p->hasDictionaries())
    {
        return false;
    }

    moduleStart(p);
    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::HelperVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
    moduleEnd(p);
}

bool
Slice::Gen::HelperVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(!p->isInterface() && p->allOperations().size() == 0)
    {
        return false;
    }

    string name = p->name();
    string ns = getNamespace(p);
    ClassList bases = p->bases();

    _out << sp;
    emitComVisibleAttribute();
    emitGeneratedCodeAttribute();
    _out << nl << "[global::System.Serializable]";
    _out << nl << "public sealed class " << name << "PrxHelper : " << getUnqualified("Ice.ObjectPrxHelperBase", ns)
         << ", " << name << "Prx";
    _out << sb;

    _out << sp;
    _out << nl << "public " << name << "PrxHelper()";
    _out << sb;
    _out << eb;

    _out << sp;
    _out << nl << "public " << name << "PrxHelper(global::System.Runtime.Serialization.SerializationInfo info, "
         << "global::System.Runtime.Serialization.StreamingContext context) : base(info, context)";
    _out << sb;
    _out << eb;

    OperationList ops = p->allOperations();

    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());
        string opName = fixId(op->name(), DotNet::ICloneable, true);
        TypePtr ret = op->returnType();
        string retS = typeToString(ret, ns, op->returnIsOptional());

        vector<string> params = getParams(op, ns);
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
             << (getUnqualified("Ice.OptionalContext", ns) + " " + context + " = new " +
                 getUnqualified("Ice.OptionalContext", ns) + "()") << epar;
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
             << "null" << "global::System.Threading.CancellationToken.None" << "true" << epar;

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
        _out << nl << "catch(global::System.AggregateException ex_)";
        _out << sb;
        _out << nl << "throw ex_.InnerException;";
        _out << eb;
        _out << eb;
    }

    //
    // Async Task AMI mapping.
    //
    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;

        ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());
        vector<string> paramsAMI = getInParams(op, ns);
        vector<string> argsAMI = getInArgs(op);

        string opName = op->name();

        ParamDeclList inParams = op->inParameters();
        ParamDeclList outParams = op->outParameters();

        string context = getEscapedParamName(op, "context");
        string cancel = getEscapedParamName(op, "cancel");
        string progress = getEscapedParamName(op, "progress");

        TypePtr ret = op->returnType();

        string retS = typeToString(ret, ns, op->returnIsOptional());

        string returnTypeS = resultType(op, ns);

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
        _out << nl << "public global::System.Threading.Tasks.Task";
        if(!returnTypeS.empty())
        {
            _out << "<" << returnTypeS << ">";
        }
        _out << " " << opName << "Async" << spar << paramsAMI
             << (getUnqualified("Ice.OptionalContext", ns) + " " + context + " = new " +
                 getUnqualified("Ice.OptionalContext", ns) + "()")
             << ("global::System.IProgress<bool> " + progress + " = null")
             << ("global::System.Threading.CancellationToken " + cancel + " = new global::System.Threading.CancellationToken()")
             << epar;

        _out << sb;
        _out << nl << "return _iceI_" << opName << "Async" << spar << argsAMI
             << context << progress << cancel << "false" << epar << ";";
        _out << eb;

        //
        // Write the Async method implementation.
        //
        _out << sp;
        _out << nl << "private global::System.Threading.Tasks.Task";
        if(!returnTypeS.empty())
        {
            _out << "<" << returnTypeS << ">";
        }
        _out << " _iceI_" << opName << "Async" << spar << getInParams(op, ns, true)
             << getUnqualified("Ice.OptionalContext", ns) + " context"
             << "global::System.IProgress<bool> progress"
             << "global::System.Threading.CancellationToken cancel"
             << "bool synchronous" << epar;
        _out << sb;

        string flatName = "_" + opName + "_name";
        if(op->returnsData())
        {
            _out << nl << "iceCheckTwowayOnly(" << flatName << ");";
        }
        if(returnTypeS.empty())
        {
            _out << nl << "var completed = "
                 << "new global::IceInternal.OperationTaskCompletionCallback<object>(progress, cancel);";
        }
        else
        {
            _out << nl << "var completed = "
                 << "new global::IceInternal.OperationTaskCompletionCallback<" << returnTypeS << ">(progress, cancel);";
        }

        _out << nl << "_iceI_" << opName << spar << getInArgs(op, true) << "context" << "synchronous" << "completed"
             << epar << ";";
        _out << nl << "return completed.Task;";

        _out << eb;

        _out << sp << nl << "private const string " << flatName << " = \"" << op->name() << "\";";

        //
        // Write the common invoke method
        //
        _out << sp << nl;
        _out << "private void _iceI_" << op->name() << spar << getInParams(op, ns, true)
             << "global::System.Collections.Generic.Dictionary<string, string> context"
             << "bool synchronous"
             << "global::IceInternal.OutgoingAsyncCompletionCallback completed" << epar;
        _out << sb;

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
        _out << nl << sliceModeToIceMode(op->sendMode(), ns) << ",";
        _out << nl << opFormatTypeToString(op, ns) << ",";
        _out << nl << "context,";
        _out << nl << "synchronous";
        if(!inParams.empty())
        {
            _out << ",";
            _out << nl << "write: (" << getUnqualified("Ice.OutputStream", ns) << " ostr) =>";
            _out << sb;
            writeMarshalUnmarshalParams(inParams, 0, true, ns);
            if(op->sendsClasses(false))
            {
                _out << nl << "ostr.writePendingValues();";
            }
            _out << eb;
        }

        if(!throws.empty())
        {
            _out << ",";
            _out << nl << "userException: (" << getUnqualified("Ice.UserException", ns) << " ex) =>";
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
                _out << nl << "catch(" << getUnqualified(*i, ns) << ")";
                _out << sb;
                _out << nl << "throw;";
                _out << eb;
            }

            _out << nl << "catch(" << getUnqualified("Ice.UserException", ns) << ")";
            _out << sb;
            _out << eb;

            _out << eb;
        }

        if(ret || !outParams.empty())
        {
            _out << ",";
            _out << nl << "read: (" << getUnqualified("Ice.InputStream", ns) << " istr) =>";
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
                    _out << " = " << getUnqualified("Ice.Util", ns) << ".None";
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
                _out << nl << typeToString(t, ns, (outParams.front()->optional())) << " iceP_"
                     << outParams.front()->name();
                if(!outParams.front()->optional())
                {
                    StructPtr st = StructPtr::dynamicCast(t);
                    if(st && isValueType(st))
                    {
                        _out << " = " << "new " << typeToString(t, ns) << "()";
                    }
                    else if(isClassType(t) || st)
                    {
                        _out << " = null";
                    }
                }
                else if(isClassType(t))
                {
                    _out << " = " << getUnqualified("Ice.Util", ns) << ".None";
                }
                _out << ";";
            }

            writeMarshalUnmarshalParams(outParams, op, false, ns, true);
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

    _out << sp << nl << "public static " << name << "Prx checkedCast(" << getUnqualified("Ice.ObjectPrx", ns) << " b)";
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
         << "Prx checkedCast(" << getUnqualified("Ice.ObjectPrx", ns)
         << " b, global::System.Collections.Generic.Dictionary<string, string> ctx)";
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

    _out << sp << nl << "public static " << name << "Prx checkedCast(" << getUnqualified("Ice.ObjectPrx", ns)
         << " b, string f)";
    _out << sb;
    _out << nl << "if(b == null)";
    _out << sb;
    _out << nl << "return null;";
    _out << eb;
    _out << nl << getUnqualified("Ice.ObjectPrx", ns) << " bb = b.ice_facet(f);";
    _out << nl << "try";
    _out << sb;
    _out << nl << "if(bb.ice_isA(ice_staticId()))";
    _out << sb;
    _out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    _out << nl << "h.iceCopyFrom(bb);";
    _out << nl << "return h;";
    _out << eb;
    _out << eb;
    _out << nl << "catch(" << getUnqualified("Ice.FacetNotExistException", ns) << ")";
    _out << sb;
    _out << eb;
    _out << nl << "return null;";
    _out << eb;

    _out << sp << nl << "public static " << name
         << "Prx checkedCast(" << getUnqualified("Ice.ObjectPrx", ns) << " b, string f, "
         << "global::System.Collections.Generic.Dictionary<string, string> ctx)";
    _out << sb;
    _out << nl << "if(b == null)";
    _out << sb;
    _out << nl << "return null;";
    _out << eb;
    _out << nl << getUnqualified("Ice.ObjectPrx", ns) << " bb = b.ice_facet(f);";
    _out << nl << "try";
    _out << sb;
    _out << nl << "if(bb.ice_isA(ice_staticId(), ctx))";
    _out << sb;
    _out << nl << name << "PrxHelper h = new " << name << "PrxHelper();";
    _out << nl << "h.iceCopyFrom(bb);";
    _out << nl << "return h;";
    _out << eb;
    _out << eb;
    _out << nl << "catch(" << getUnqualified("Ice.FacetNotExistException", ns) << ")";
    _out << sb;
    _out << eb;
    _out << nl << "return null;";
    _out << eb;

    _out << sp << nl << "public static " << name << "Prx uncheckedCast(" << getUnqualified("Ice.ObjectPrx", ns) << " b)";
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

    _out << sp << nl << "public static " << name << "Prx uncheckedCast(" << getUnqualified("Ice.ObjectPrx", ns)
         << " b, string f)";
    _out << sb;
    _out << nl << "if(b == null)";
    _out << sb;
    _out << nl << "return null;";
    _out << eb;
    _out << nl << getUnqualified("Ice.ObjectPrx", ns) << " bb = b.ice_facet(f);";
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

    _out << sp << nl << "public static void write(" << getUnqualified("Ice.OutputStream", ns) << " ostr, " << name
         << "Prx v)";
    _out << sb;
    _out << nl << "ostr.writeProxy(v);";
    _out << eb;

    _out << sp << nl << "public static " << name << "Prx read(" << getUnqualified("Ice.InputStream", ns) << " istr)";
    _out << sb;
    _out << nl << getUnqualified("Ice.ObjectPrx", ns) << " proxy = istr.readProxy();";
    _out << nl << "if(proxy != null)";
    _out << sb;
    _out << nl << name << "PrxHelper result = new " << name << "PrxHelper();";
    _out << nl << "result.iceCopyFrom(proxy);";
    _out << nl << "return result;";
    _out << eb;
    _out << nl << "return null;";
    _out << eb;
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
    string ns = getNamespace(p);
    string typeS = typeToString(p, ns);
    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public sealed class " << p->name() << "Helper";
    _out << sb;

    _out << sp << nl << "public static void write(" << getUnqualified("Ice.OutputStream", ns) << " ostr, " << typeS
         << " v)";
    _out << sb;
    writeSequenceMarshalUnmarshalCode(_out, p, ns, "v", true, false);
    _out << eb;

    _out << sp << nl << "public static " << typeS << " read(" << getUnqualified("Ice.InputStream", ns) << " istr)";
    _out << sb;
    _out << nl << typeS << " v;";
    writeSequenceMarshalUnmarshalCode(_out, p, ns, "v", false, false);
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

    string ns = getNamespace(p);
    string keyS = typeToString(key, ns);
    string valueS = typeToString(value, ns);
    string name = "global::System.Collections.Generic." + genericType + "<" + keyS + ", " + valueS + ">";

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public sealed class " << p->name() << "Helper";
    _out << sb;

    _out << sp << nl << "public static void write(";
    _out.useCurrentPosAsIndent();
    _out << getUnqualified("Ice.OutputStream", ns) << " ostr,";
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
    _out << nl << "foreach(global::System.Collections.";
    _out << "Generic.KeyValuePair<" << keyS << ", " << valueS << ">";
    _out << " e in v)";
    _out << sb;
    writeMarshalUnmarshalCode(_out, key, ns, "e.Key", true);
    writeMarshalUnmarshalCode(_out, value, ns, "e.Value", true);
    _out << eb;
    _out << eb;
    _out << eb;

    _out << sp << nl << "public static " << name << " read(" << getUnqualified("Ice.InputStream", ns) << " istr)";
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
            _out << nl << "k = new " << typeToString(key, ns) << "();";
        }
        else
        {
            _out << nl << "k = null;";
        }
    }
    writeMarshalUnmarshalCode(_out, key, ns, "k", false);

    if(isClassType(value))
    {
        ostringstream os;
        os << '(' << typeToString(value, ns) << " v) => { r[k] = v; }";
        writeMarshalUnmarshalCode(_out, value, ns, os.str(), false);
    }
    else
    {
        _out << nl << valueS << " v;";
        StructPtr stv = StructPtr::dynamicCast(value);
        if(stv)
        {
            if(isValueType(stv))
            {
                _out << nl << "v = new " << typeToString(value, ns) << "();";
            }
            else
            {
                _out << nl << "v = null;";
            }
        }
        writeMarshalUnmarshalCode(_out, value, ns, "v", false);
        _out << nl << "r[k] = v;";
    }
    _out << eb;
    _out << nl << "return r;";
    _out << eb;

    _out << eb;
}

Slice::Gen::DispatcherVisitor::DispatcherVisitor(::IceUtilInternal::Output& out) :
    CsVisitor(out)
{
}

bool
Slice::Gen::DispatcherVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasNonLocalAbstractClassDefs())
    {
        return false;
    }

    moduleStart(p);
    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::DispatcherVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
    moduleEnd(p);
}

bool
Slice::Gen::DispatcherVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(!p->isInterface() && p->allOperations().empty())
    {
        return false;
    }

    ClassList bases = p->bases();
    bool hasBaseClass = !bases.empty() && !bases.front()->isInterface();
    string name = p->name();
    string ns = getNamespace(p);
    string baseClass = getUnqualified("Ice.ObjectImpl", ns);
    if(hasBaseClass && !bases.front()->allOperations().empty())
    {
        baseClass = getUnqualified(bases.front(), ns, "", "Disp_");
    }

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
            _out << ", " << getUnqualified(*i, ns);
        }
    }

    _out << sb;

    OperationList ops = p->operations();
    for(OperationList::const_iterator i = ops.begin(); i != ops.end(); ++i)
    {
        string retS;
        vector<string> params, args;
        string opName = getDispatchParams(*i, retS, params, args, ns);
        _out << sp << nl << "public abstract " << retS << " " << opName << spar << params << epar << ';';
    }

    writeInheritedOperations(p);
    writeDispatch(p);
    return true;
}
void
Slice::Gen::DispatcherVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _out << eb;
}

Slice::Gen::BaseImplVisitor::BaseImplVisitor(IceUtilInternal::Output& out)
    : CsVisitor(out)
{
}

void
Slice::Gen::BaseImplVisitor::writeOperation(const OperationPtr& op, bool comment)
{
    ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());
    string ns = getNamespace(cl);
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

    if(cl->hasMetaData("amd") || op->hasMetaData("amd"))
    {
        vector<string> pDecl = getInParams(op, ns);
        string resultType = CsGenerator::resultType(op, ns, true);

        _out << "public override ";

        _out << "global::System.Threading.Tasks.Task";
        if(!resultType.empty())
        {
            _out << "<" << resultType << ">";
        }
        _out << " " << opName << "Async" << spar << pDecl << getUnqualified("Ice.Current", ns) + " current = null"
             << epar;

        if(comment)
        {
            _out << ';';
            return;
        }

        _out << sb;
        if(ret)
        {
            _out << nl << typeToString(ret, ns) << " ret = " << writeValue(ret, ns) << ';';
        }
        for(ParamDeclList::const_iterator i = params.begin(); i != params.end(); ++i)
        {
            if((*i)->isOutParam())
            {
                string name = fixId((*i)->name());
                TypePtr type = (*i)->type();
                _out << nl << typeToString(type, ns) << ' ' << name << " = " << writeValue(type, ns) << ';';
            }
        }
        _out << nl << "return global::System.Threading.Tasks.Task.FromResult";
        if(resultType.empty())
        {
            _out << "<global::System.Object>(null);";
        }
        else
        {

            bool returnStruct = (op->returnType() && !outParams.empty()) || outParams.size() > 1 ||
                op->hasMarshaledResult();

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
            typeToString(ret, ns);

        vector<string> pDecls = op->hasMarshaledResult() ? getInParams(op, ns) : getParams(op, ns);

        _out << "public override ";
        _out << retS << ' ' << fixId(opName, DotNet::ICloneable, true) << spar << pDecls;
        _out << getUnqualified("Ice.Current", ns) + " current = null";
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
                _out << writeValue(ret, ns);
            }
            for(ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end(); ++i)
            {
                if(ret || i != outParams.begin())
                {
                    _out << ", ";
                }
                _out << writeValue((*i)->type(), ns);
            }
            _out << ", current);";
        }
        else
        {
            for(ParamDeclList::const_iterator i = outParams.begin(); i != outParams.end(); ++i)
            {
                string name = fixId((*i)->name());
                TypePtr type = (*i)->type();
                _out << nl << name << " = " << writeValue(type, ns) << ';';
            }

            if(ret)
            {
                _out << nl << "return " << writeValue(ret, ns) << ';';
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

    moduleStart(p);
    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;

    return true;
}

void
Slice::Gen::ImplVisitor::visitModuleEnd(const ModulePtr& p)
{
    _out << eb;
    moduleEnd(p);
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
        _out << " : " << name << "Disp_";
    }
    else
    {
        _out << " : " << fixId(name);
    }
    _out << sb;

    OperationList ops = p->allOperations();
    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        writeOperation(*r, false);
    }

    return true;
}

void
Slice::Gen::ImplVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _out << eb;
}
