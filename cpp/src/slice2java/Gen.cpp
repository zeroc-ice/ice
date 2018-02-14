// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Gen.h>
#include <Slice/Checksum.h>
#include <Slice/Util.h>
#include <IceUtil/Functional.h>
#include <IceUtil/Iterator.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/InputUtil.h>
#include <IceUtil/Unicode.h>
#include <cstring>

#include <limits>

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

string
u16CodePoint(unsigned short value)
{
    ostringstream s;
    s << "\\u";
    s << hex;
    s.width(4);
    s.fill('0');
    s << value;
    return s.str();
}

void
writeU8Buffer(const vector<unsigned char>& u8buffer, ::IceUtilInternal::Output& out)
{
    vector<unsigned short> u16buffer;
    IceUtilInternal::ConversionResult result = convertUTF8ToUTF16(u8buffer, u16buffer, IceUtil::lenientConversion);
    switch(result)
    {
        case conversionOK:
            break;
        case sourceExhausted:
            throw IceUtil::IllegalConversionException(__FILE__, __LINE__, "string source exhausted");
        case sourceIllegal:
            throw IceUtil::IllegalConversionException(__FILE__, __LINE__, "string source illegal");
        default:
        {
            assert(0);
            throw IceUtil::IllegalConversionException(__FILE__, __LINE__);
        }
    }

    for(vector<unsigned short>::const_iterator c = u16buffer.begin(); c != u16buffer.end(); ++c)
    {
        out << u16CodePoint(*c);
    }
}

static string
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

static string
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

static string
getDeprecateReason(const ContainedPtr& p1, const ContainedPtr& p2, const string& type)
{
    string deprecateMetadata, deprecateReason;
    if(p1->findMetaData("deprecate", deprecateMetadata) ||
       (p2 != 0 && p2->findMetaData("deprecate", deprecateMetadata)))
    {
        deprecateReason = "This " + type + " has been deprecated.";
        if(deprecateMetadata.find("deprecate:") == 0 && deprecateMetadata.size() > 10)
        {
            deprecateReason = deprecateMetadata.substr(10);
        }
    }
    return deprecateReason;
}

string
initValue(const TypePtr& p)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p);
    if(builtin)
    {
        switch(builtin->kind())
        {
        case Builtin::KindBool:
        {
            return "false";
        }
        case Builtin::KindByte:
        {
            return "(byte)0";
        }
        case Builtin::KindShort:
        {
            return "(short)0";
        }
        case Builtin::KindInt:
        case Builtin::KindLong:
        {
            return "0";
        }
        case Builtin::KindFloat:
        {
            return "(float)0.0";
        }
        case Builtin::KindDouble:
        {
            return "0.0";
        }
        case Builtin::KindString:
        case Builtin::KindObject:
        case Builtin::KindObjectProxy:
        case Builtin::KindLocalObject:
        {
            return "null";
        }
        }
    }
    return "null";
}

void
writeParamList(Output& out, vector<string> params, bool end = true, bool newLine = true)
{
    out << "(";
    out.useCurrentPosAsIndent();
    for(vector<string>::const_iterator i = params.begin(); i != params.end();)
    {
        out << (*i);
        if(++i != params.end() || !end)
        {
            out << ", ";
            if(newLine)
            {
                out << nl;
            }
        }
    }
    if(end)
    {
        out << ")";
        out.restoreIndent();
    }
}

Slice::JavaVisitor::JavaVisitor(const string& dir) :
    JavaGenerator(dir)
{
}

Slice::JavaVisitor::~JavaVisitor()
{
}

ParamDeclList
Slice::JavaVisitor::getOutParams(const OperationPtr& op)
{
    ParamDeclList outParams;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator i = paramList.begin(); i != paramList.end(); ++i)
    {
        if((*i)->isOutParam())
        {
            outParams.push_back(*i);
        }
    }
    return outParams;
}

vector<string>
Slice::JavaVisitor::getParams(const OperationPtr& op, const string& package, bool local, bool optionalMapping)
{
    vector<string> params;

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        StringList metaData = (*q)->getMetaData();
        bool optional = (*q)->optional();
        if(optional && (local || (*q)->isOutParam()))
        {
            optional = optionalMapping;
        }
        string typeString = typeToString((*q)->type(), (*q)->isOutParam() ? TypeModeOut : TypeModeIn, package,
                                         metaData, true, optional);
        params.push_back(typeString + ' ' + fixKwd((*q)->name()));
    }

    return params;
}

vector<string>
Slice::JavaVisitor::getParamsProxy(const OperationPtr& op, const string& package, bool final, bool optionalMapping)
{
    vector<string> params;

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        bool optional;
        if((*q)->optional())
        {
            optional = (*q)->isOutParam() ? true : optionalMapping;
        }
        else
        {
            optional = false;
        }

        StringList metaData = (*q)->getMetaData();
        string typeString = typeToString((*q)->type(), (*q)->isOutParam() ? TypeModeOut : TypeModeIn, package,
                                         metaData, true, optional);
        if(final)
        {
            typeString = "final " + typeString;
        }
        params.push_back(typeString + ' ' + fixKwd((*q)->name()));
    }

    return params;
}

vector<string>
Slice::JavaVisitor::getInOutParams(const OperationPtr& op, const string& package, ParamDir paramType, bool /*proxy*/,
                                   bool optionalMapping)
{
    vector<string> params;

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if((*q)->isOutParam() == (paramType == OutParam))
        {
            bool optional = optionalMapping && (*q)->optional();
            string typeString = typeToString((*q)->type(), paramType == InParam ? TypeModeIn : TypeModeOut, package,
                                             (*q)->getMetaData(), true, optional);
            params.push_back(typeString + ' ' + fixKwd((*q)->name()));
        }
    }

    return params;
}

vector<string>
Slice::JavaVisitor::getParamsAsync(const OperationPtr& op, const string& package, bool amd, bool optionalMapping)
{
    vector<string> params = getInOutParams(op, package, InParam, !amd, optionalMapping);

    string name = op->name();
    ContainerPtr container = op->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    string classNameAsync = getAbsolute(cl, package, amd ? "AMD_" : "AMI_", '_' + name);
    params.insert(params.begin(), classNameAsync + " __cb");

    return params;
}

vector<string>
Slice::JavaVisitor::getParamsAsyncCB(const OperationPtr& op, const string& package, bool /*amd*/, bool optionalMapping)
{
    vector<string> params;

    TypePtr ret = op->returnType();
    if(ret)
    {
        string retS = typeToString(ret, TypeModeIn, package, op->getMetaData(), true,
                                   optionalMapping && op->returnIsOptional());
        params.push_back(retS + " __ret");
    }

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if((*q)->isOutParam())
        {
            string typeString = typeToString((*q)->type(), TypeModeIn, package, (*q)->getMetaData(), true,
                                             optionalMapping && (*q)->optional());
            params.push_back(typeString + ' ' + fixKwd((*q)->name()));
        }
    }

    return params;
}

namespace
{

const char* builtinAsyncCallbackTable[] =
{
    "TwowayCallbackByte",
    "TwowayCallbackBool",
    "TwowayCallbackShort",
    "TwowayCallbackInt",
    "TwowayCallbackLong",
    "TwowayCallbackFloat",
    "TwowayCallbackDouble"
};

}

string
Slice::JavaVisitor::getAsyncCallbackInterface(const OperationPtr& op, const string& package)
{
    TypePtr ret = op->returnType();
    ParamDeclList outParams = getOutParams(op);
    bool throws = !op->throws().empty();
    const string suffix = throws ? "UE" : "";

    if(!ret && outParams.empty())
    {
        return throws ? "Ice.TwowayCallbackVoidUE" : "Ice.OnewayCallback";
    }
    else if((ret && outParams.empty()) || (!ret && outParams.size() == 1))
    {
        TypePtr t = ret ? ret : outParams.front()->type();
        bool optional = ret ? op->returnIsOptional() : outParams.front()->optional();
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(t);
        if(builtin && !optional)
        {
            const string prefix = "Ice.";
            switch(builtin->kind())
            {
                case Builtin::KindByte:
                case Builtin::KindBool:
                case Builtin::KindShort:
                case Builtin::KindInt:
                case Builtin::KindLong:
                case Builtin::KindFloat:
                case Builtin::KindDouble:
                {
                    return prefix + builtinAsyncCallbackTable[builtin->kind()] + suffix;
                }
                default:
                {
                    break;
                }
            }
        }

        return "Ice.TwowayCallbackArg1" + suffix + "<" +
            typeToString(t, TypeModeIn, package, op->getMetaData(), true, optional) + ">";
    }
    else
    {
        ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());
        return getPackage(cl) + "._Callback_" + cl->name() + "_" + op->name();
    }
}

string
Slice::JavaVisitor::getAsyncCallbackBaseClass(const OperationPtr& op, bool functional)
{
    assert(op->returnsData());
    TypePtr ret = op->returnType();
    ParamDeclList outParams = getOutParams(op);

    bool throws = !op->throws().empty();
    const string suffix = throws ? "UE" : "";
    if(!ret && outParams.empty())
    {
        assert(throws);
        return functional ?
            "IceInternal.Functional_TwowayCallbackVoidUE" :
            "IceInternal.TwowayCallback implements Ice.TwowayCallbackVoidUE";
    }
    else if((ret && outParams.empty()) || (!ret && outParams.size() == 1))
    {
        TypePtr t = ret ? ret : outParams.front()->type();
        bool optional = ret ? op->returnIsOptional() : outParams.front()->optional();
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(t);
        if(builtin && !optional)
        {
            switch(builtin->kind())
            {
                case Builtin::KindByte:
                case Builtin::KindBool:
                case Builtin::KindShort:
                case Builtin::KindInt:
                case Builtin::KindLong:
                case Builtin::KindFloat:
                case Builtin::KindDouble:
                {
                    ostringstream os;
                    os << (functional ? "IceInternal.Functional_" : "IceInternal.TwowayCallback implements Ice.")
                       << builtinAsyncCallbackTable[builtin->kind()] + suffix;
                    return os.str();
                }
                default:
                {
                    break;
                }
            }
        }

        ostringstream os;
        if(functional)
        {
            os << "IceInternal.Functional_TwowayCallbackArg1";
        }
        else
        {
            os << "IceInternal.TwowayCallback implements Ice.TwowayCallbackArg1";
        }
        os << suffix << "<" << typeToString(t, TypeModeIn, getPackage(op), op->getMetaData(), true, optional) + ">";
        return os.str();
    }
    else
    {
        ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());
        ostringstream os;
        if(functional)
        {
            os << "IceInternal.Functional_TwowayCallback" << suffix << " implements ";
        }
        else
        {
            os << "IceInternal.TwowayCallback implements ";
        }
        os << getPackage(cl) << "._Callback_" << cl->name() << "_" << op->name();
        return os.str();
    }
}

string
Slice::JavaVisitor::getLambdaResponseCB(const OperationPtr& op, const string& package)
{
    TypePtr ret = op->returnType();
    ParamDeclList outParams = getOutParams(op);
    if(!ret && outParams.empty())
    {
        return "IceInternal.Functional_VoidCallback";
    }
    else if((ret && outParams.empty()) || (!ret && outParams.size() == 1))
    {
        TypePtr t = ret ? ret : outParams.front()->type();
        bool optional = ret ? op->returnIsOptional() : outParams.front()->optional();
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(t);
        if(builtin && !optional)
        {
            static const char* builtinTable[] =
            {
                "IceInternal.Functional_ByteCallback",
                "IceInternal.Functional_BoolCallback",
                "IceInternal.Functional_ShortCallback",
                "IceInternal.Functional_IntCallback",
                "IceInternal.Functional_LongCallback",
                "IceInternal.Functional_FloatCallback",
                "IceInternal.Functional_DoubleCallback"
            };
            switch(builtin->kind())
            {
                case Builtin::KindByte:
                case Builtin::KindBool:
                case Builtin::KindShort:
                case Builtin::KindInt:
                case Builtin::KindLong:
                case Builtin::KindFloat:
                case Builtin::KindDouble:
                {
                    return builtinTable[builtin->kind()];
                }
                default:
                {
                    break;
                }
            }
        }

        return "IceInternal.Functional_GenericCallback1<" +
            typeToString(t, TypeModeIn, package, op->getMetaData(), true, optional) + ">";
    }
    else
    {
        ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());
        return "FunctionalCallback_" + cl->name() + "_" + op->name() + "_Response";
    }
}

vector<string>
Slice::JavaVisitor::getParamsAsyncLambda(const OperationPtr& op, const string& package, bool context, bool sentCB,
                                         bool optionalMapping, bool inParams)
{
    vector<string> params;

    if(inParams)
    {
        params = getInOutParams(op, package, InParam, false, optionalMapping);
    }

    if(context)
    {
        params.push_back("java.util.Map<String, String> __ctx");
    }

    params.push_back(getLambdaResponseCB(op, package) + " __responseCb");

    if(!op->throws().empty())
    {
        params.push_back("IceInternal.Functional_GenericCallback1<Ice.UserException> __userExceptionCb");
    }

    params.push_back("IceInternal.Functional_GenericCallback1<Ice.Exception> __exceptionCb");

    if(sentCB)
    {
        params.push_back("IceInternal.Functional_BoolCallback __sentCb");
    }

    return params;
}

vector<string>
Slice::JavaVisitor::getArgsAsyncLambda(const OperationPtr& op, const string& package, bool context, bool sentCB)
{
    vector<string> args = getInOutArgs(op, InParam);
    args.push_back(context ? "__ctx" : "null");
    args.push_back(context ? "true" : "false");
    args.push_back("false"); // __synchronous
    args.push_back("__responseCb");
    if(!op->throws().empty())
    {
        args.push_back("__userExceptionCb");
    }
    args.push_back("__exceptionCb");
    args.push_back(sentCB ? "__sentCb" : "null");
    return args;
}

vector<string>
Slice::JavaVisitor::getArgs(const OperationPtr& op)
{
    vector<string> args;

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        args.push_back(fixKwd((*q)->name()));
    }

    return args;
}

vector<string>
Slice::JavaVisitor::getInOutArgs(const OperationPtr& op, ParamDir paramType)
{
    vector<string> args;

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if((*q)->isOutParam() == (paramType == OutParam))
        {
            args.push_back(fixKwd((*q)->name()));
        }
    }

    return args;
}

vector<string>
Slice::JavaVisitor::getArgsAsync(const OperationPtr& op)
{
    vector<string> args = getInOutArgs(op, InParam);
    args.insert(args.begin(), "__cb");
    return args;
}

vector<string>
Slice::JavaVisitor::getArgsAsyncCB(const OperationPtr& op)
{
    vector<string> args;

    TypePtr ret = op->returnType();
    if(ret)
    {
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
        if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(ret))
        {
            args.push_back("__ret.value");
        }
        else
        {
            args.push_back("__ret");
        }
    }

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if((*q)->isOutParam())
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast((*q)->type());
            if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast((*q)->type()))
            {
                args.push_back(fixKwd((*q)->name()) + ".value");
            }
            else
            {
                args.push_back(fixKwd((*q)->name()));
            }
        }
    }

    return args;
}

void
Slice::JavaVisitor::writeMarshalUnmarshalParams(Output& out, const string& package, const ParamDeclList& params,
                                                const OperationPtr& op, int& iter, bool marshal, bool optionalMapping,
                                                bool dispatch)
{
    ParamDeclList optionals;
    for(ParamDeclList::const_iterator pli = params.begin(); pli != params.end(); ++pli)
    {
        if((*pli)->optional())
        {
            optionals.push_back(*pli);
        }
        else
        {
            string paramName = fixKwd((*pli)->name());
            bool holder = marshal == dispatch;
            string patchParams;
            if(!marshal)
            {
                patchParams = paramName;
            }
            writeMarshalUnmarshalCode(out, package, (*pli)->type(), OptionalNone, false, 0, paramName, marshal,
                                      iter, holder, (*pli)->getMetaData(), patchParams);
        }
    }

    TypePtr ret;
    bool returnsObject = false;

    if(op && op->returnType())
    {
        ret = op->returnType();
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
        ClassDeclPtr cl = ClassDeclPtr::dynamicCast(ret);
        returnsObject = (builtin && builtin->kind() == Builtin::KindObject) || cl;
        const bool optional = optionalMapping && op->returnIsOptional();

        string retS = typeToString(ret, TypeModeReturn, package, op->getMetaData(), true, optional);
        bool holder = false;

        if(!marshal)
        {
            if(optional)
            {
                out << nl << retS << " __ret = new " << retS << "();";
            }
            else if(returnsObject)
            {
                out << nl << retS << "Holder __ret = new " << retS << "Holder();";
                holder = true;
            }
            else if(StructPtr::dynamicCast(ret))
            {
                out << nl << retS << " __ret = null;";
            }
            else
            {
                out << nl << retS << " __ret;";
            }
        }

        if(!op->returnIsOptional())
        {
            writeMarshalUnmarshalCode(out, package, ret, OptionalNone, false, 0, "__ret", marshal, iter, holder,
                                      op->getMetaData());
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
            writeMarshalUnmarshalCode(out, package, ret, OptionalReturnParam, optionalMapping, op->returnTag(),
                                      "__ret", marshal, iter, false, op->getMetaData());
            checkReturnType = false;
        }

        const bool holder = dispatch && (*pli)->isOutParam() && !optionalMapping;

        writeMarshalUnmarshalCode(out, package, (*pli)->type(),
                                  (*pli)->isOutParam() ? OptionalOutParam : OptionalInParam, optionalMapping,
                                  (*pli)->tag(), fixKwd((*pli)->name()), marshal, iter, holder, (*pli)->getMetaData());
    }

    if(checkReturnType)
    {
        writeMarshalUnmarshalCode(out, package, ret, OptionalReturnParam, optionalMapping, op->returnTag(), "__ret",
                                  marshal, iter, false, op->getMetaData());
    }
}

void
Slice::JavaVisitor::writeThrowsClause(const string& package, const ExceptionList& throws)
{
    Output& out = output();
    if(throws.size() > 0)
    {
        out.inc();
        out << nl << "throws ";
        out.useCurrentPosAsIndent();
        int count = 0;
        for(ExceptionList::const_iterator r = throws.begin(); r != throws.end(); ++r)
        {
            if(count > 0)
            {
                out << "," << nl;
            }
            out << getAbsolute(*r, package);
            count++;
        }
        out.restoreIndent();
        out.dec();
    }
}

void
Slice::JavaVisitor::writeMarshalDataMember(Output& out, const string& package, const DataMemberPtr& member, int& iter)
{
    if(!member->optional())
    {
        writeMarshalUnmarshalCode(out, package, member->type(), OptionalNone, false, 0, fixKwd(member->name()),
                                  true, iter, false, member->getMetaData());
    }
    else
    {
        out << nl << "if(__has_" << member->name() << " && __os.writeOpt(" << member->tag() << ", "
            << getOptionalFormat(member->type()) << "))";
        out << sb;
        writeMarshalUnmarshalCode(out, package, member->type(), OptionalMember, false, 0, fixKwd(member->name()), true,
                                  iter, false, member->getMetaData());
        out << eb;
    }
}

void
Slice::JavaVisitor::writeUnmarshalDataMember(Output& out, const string& package, const DataMemberPtr& member,
                                             int& iter, bool needPatcher, int& patchIter)
{
    string patchParams;
    if(needPatcher)
    {
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(member->type());
        if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(member->type()))
        {
            ostringstream ostr;
            ostr << "new Patcher(" << patchIter++ << ')';
            patchParams = ostr.str();
        }
    }

    if(!member->optional())
    {
        writeMarshalUnmarshalCode(out, package, member->type(), OptionalNone, false, 0, fixKwd(member->name()), false,
                                  iter, false, member->getMetaData(), patchParams);
    }
    else
    {
        out << nl << "if(__has_" << member->name() << " = __is.readOpt(" << member->tag() << ", "
            << getOptionalFormat(member->type()) << "))";
        out << sb;
        writeMarshalUnmarshalCode(out, package, member->type(), OptionalMember, false, 0, fixKwd(member->name()), false,
                                  iter, false, member->getMetaData(), patchParams);
        out << eb;
    }
}

void
Slice::JavaVisitor::writeStreamMarshalDataMember(Output& out, const string& package, const DataMemberPtr& member,
                                                 int& iter)
{
    if(!member->optional())
    {
        writeStreamMarshalUnmarshalCode(out, package, member->type(), false, 0, fixKwd(member->name()), true,
                                        iter, false, member->getMetaData());
    }
    else
    {
        out << nl << "if(__has_" << member->name() << " && __outS.writeOptional(" << member->tag() << ", "
            << getOptionalFormat(member->type()) << "))";
        out << sb;
        writeStreamMarshalUnmarshalCode(out, package, member->type(), true, member->tag(), fixKwd(member->name()),
                                        true, iter, false, member->getMetaData());
        out << eb;
    }
}

void
Slice::JavaVisitor::writeStreamUnmarshalDataMember(Output& out, const string& package, const DataMemberPtr& member,
                                                   int& iter, bool needPatcher, int& patchIter)
{
    string patchParams;
    if(needPatcher)
    {
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(member->type());
        if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(member->type()))
        {
            ostringstream ostr;
            ostr << "new Patcher(" << patchIter++ << ')';
            patchParams = ostr.str();
        }
    }

    if(!member->optional())
    {
        writeStreamMarshalUnmarshalCode(out, package, member->type(), false, 0, fixKwd(member->name()), false,
                                        iter, false, member->getMetaData(), patchParams);
    }
    else
    {
        out << nl << "if(__has_" << member->name() << " = __inS.readOptional(" << member->tag() << ", "
            << getOptionalFormat(member->type()) << "))";
        out << sb;
        writeStreamMarshalUnmarshalCode(out, package, member->type(), true, member->tag(), fixKwd(member->name()),
                                        false, iter, false, member->getMetaData(), patchParams);
        out << eb;
    }
}

void
Slice::JavaVisitor::writePatcher(Output& out, const string& package, const DataMemberList& classMembers,
                                 const DataMemberList& optionalMembers, bool stream)
{
    out << sp << nl << "private class Patcher implements IceInternal.Patcher";
    if(stream)
    {
        out << ", Ice.ReadObjectCallback";
    }
    out << sb;
    if(classMembers.size() > 1)
    {
        out << sp << nl << "Patcher(int member)";
        out << sb;
        out << nl << "__member = member;";
        out << eb;
    }

    out << sp << nl << "public void" << nl << "patch(Ice.Object v)";
    out << sb;
    if(classMembers.size() > 1)
    {
        out << nl << "switch(__member)";
        out << sb;
    }
    int memberCount = 0;
    for(DataMemberList::const_iterator d = classMembers.begin(); d != classMembers.end(); ++d)
    {
        if((*d)->optional())
        {
            continue;
        }

        BuiltinPtr b = BuiltinPtr::dynamicCast((*d)->type());
        if(b)
        {
            assert(b->kind() == Builtin::KindObject);
        }

        if(classMembers.size() > 1)
        {
            out.dec();
            out << nl << "case " << memberCount << ":";
            out.inc();
            if(b)
            {
                out << nl << "__typeId = Ice.ObjectImpl.ice_staticId();";
            }
            else
            {
                out << nl << "__typeId = \"" << (*d)->type()->typeId() << "\";";
            }
        }

        string memberName = fixKwd((*d)->name());
        if(b)
        {
            out << nl << memberName << " = v;";
        }
        else
        {
            string memberType = typeToString((*d)->type(), TypeModeMember, package);
            out << nl << "if(v == null || v instanceof " << memberType << ")";
            out << sb;
            out << nl << memberName << " = (" << memberType << ")v;";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << "IceInternal.Ex.throwUOE(type(), v);";
            out << eb;
        }

        if(classMembers.size() > 1)
        {
            out << nl << "break;";
        }

        memberCount++;
    }

    for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
    {
        BuiltinPtr b = BuiltinPtr::dynamicCast((*d)->type());
        if(b && b->kind() != Builtin::KindObject)
        {
            continue;
        }

        TypePtr paramType = (*d)->type();
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(paramType);
        if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(paramType))
        {

            if(classMembers.size() > 1)
            {
                out.dec();
                out << nl << "case " << memberCount << ":";
                out.inc();
                if(b)
                {
                    out << nl << "__typeId = Ice.ObjectImpl.ice_staticId();";
                }
                else
                {
                    out << nl << "__typeId = \"" << (*d)->type()->typeId() << "\";";
                }
            }

            string capName = (*d)->name();
            capName[0] = toupper(static_cast<unsigned char>(capName[0]));

            if(b)
            {
                out << nl << "set" << capName << "(v);";
            }
            else
            {
                string memberType = typeToString((*d)->type(), TypeModeMember, package);
                out << nl << "if(v == null || v instanceof " << memberType << ")";
                out << sb;
                out << nl << "set" << capName << "((" << memberType << ")v);";
                out << eb;
                out << nl << "else";
                out << sb;
                out << nl << "IceInternal.Ex.throwUOE(type(), v);";
                out << eb;
            }

            if(classMembers.size() > 1)
            {
                out << nl << "break;";
            }

            memberCount++;
        }
    }

    if(classMembers.size() > 1)
    {
        out << eb;
    }
    out << eb;

    out << sp << nl << "public String" << nl << "type()";
    out << sb;
    if(classMembers.size() > 1)
    {
        out << nl << "return __typeId;";
    }
    else
    {
        out << nl << "return \"" << (*classMembers.begin())->type()->typeId() << "\";";
    }
    out << eb;

    if(stream)
    {
        out << sp << nl << "public void" << nl << "invoke(Ice.Object v)";
        out << sb;
        out << nl << "patch(v);";
        out << eb;
    }

    if(classMembers.size() > 1)
    {
        out << sp << nl << "private int __member;";
        out << nl << "private String __typeId;";
    }

    out << eb;
}

void
Slice::JavaVisitor::writeDispatchAndMarshalling(Output& out, const ClassDefPtr& p, bool stream)
{
    string name = fixKwd(p->name());
    string package = getPackage(p);
    string scoped = p->scoped();
    ClassList bases = p->bases();
    ClassDefPtr base;
    if(!bases.empty() && !bases.front()->isInterface())
    {
        base = bases.front();
    }

    ClassList allBases = p->allBases();
    StringList ids;
    transform(allBases.begin(), allBases.end(), back_inserter(ids), constMemFun(&Contained::scoped));
    StringList other;
    other.push_back(scoped);
    other.push_back("::Ice::Object");
    other.sort();
    ids.merge(other);
    ids.unique();
    StringList::const_iterator firstIter = ids.begin();
    StringList::const_iterator scopedIter = find(ids.begin(), ids.end(), scoped);
    assert(scopedIter != ids.end());
    StringList::difference_type scopedPos = ::IceUtilInternal::distance(firstIter, scopedIter);

    out << sp << nl << "public static final String[] __ids =";
    out << sb;

    for(StringList::const_iterator q = ids.begin(); q != ids.end();)
    {
        out << nl << '"' << *q << '"';
        if(++q != ids.end())
        {
            out << ',';
        }
    }
    out << eb << ';';

    out << sp << nl << "public boolean ice_isA(String s)";
    out << sb;
    out << nl << "return java.util.Arrays.binarySearch(__ids, s) >= 0;";
    out << eb;

    out << sp << nl << "public boolean ice_isA(String s, Ice.Current __current)";
    out << sb;
    out << nl << "return java.util.Arrays.binarySearch(__ids, s) >= 0;";
    out << eb;

    out << sp << nl << "public String[] ice_ids()";
    out << sb;
    out << nl << "return __ids;";
    out << eb;

    out << sp << nl << "public String[] ice_ids(Ice.Current __current)";
    out << sb;
    out << nl << "return __ids;";
    out << eb;

    out << sp << nl << "public String ice_id()";
    out << sb;
    out << nl << "return __ids[" << scopedPos << "];";
    out << eb;

    out << sp << nl << "public String ice_id(Ice.Current __current)";
    out << sb;
    out << nl << "return __ids[" << scopedPos << "];";
    out << eb;

    out << sp << nl << "public static String ice_staticId()";
    out << sb;
    out << nl << "return __ids[" << scopedPos << "];";
    out << eb;

    OperationList ops = p->allOperations();

    //
    // Write the "no Current" implementation of each operation.
    //
    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        string opName = op->name();

        ContainerPtr container = op->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        assert(cl);

        string deprecateReason = getDeprecateReason(op, cl, "operation");

        const bool amd = cl->hasMetaData("amd") || op->hasMetaData("amd");
        const bool optionalMapping = useOptionalMapping(op);

        vector<string> params;
        vector<string> args;
        TypePtr ret;

        if(amd)
        {
            opName += "_async";
            params = getParamsAsync(op, package, true, true);
            args = getArgsAsync(op);
        }
        else
        {
            opName = fixKwd(opName);
            ret = op->returnType();
            params = getParams(op, package, false, optionalMapping);
            args = getArgs(op);
        }

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

        //
        // Only generate a "no current" version of the operation if it hasn't been done in a base
        // class already, because the "no current" version is final.
        //
        bool generateOperation = cl == p; // Generate if the operation is defined in this class.
        if(!generateOperation)
        {
            //
            // The operation is not defined in this class.
            //
            if(!bases.empty())
            {
                //
                // Check if the operation is already implemented by a base class.
                //
                bool implementedByBase = false;
                if(!bases.front()->isInterface())
                {
                    OperationList baseOps = bases.front()->allOperations();
                    OperationList::const_iterator i;
                    for(i = baseOps.begin(); i != baseOps.end(); ++i)
                    {
                        if((*i)->name() == op->name())
                        {
                            implementedByBase = true;
                            break;
                        }
                    }
                    if(i == baseOps.end())
                    {
                        generateOperation = true;
                    }
                }
                if(!generateOperation && !implementedByBase)
                {
                     //
                     // No base class defines the operation. Check if one of the
                     // interfaces defines it, in which case this class must provide it.
                     //
                     if(bases.front()->isInterface() || bases.size() > 1)
                     {
                         generateOperation = true;
                     }
                }
            }
        }
        if(generateOperation)
        {
            out << sp;
            if(amd)
            {
                writeDocCommentAsync(out, op, InParam);
            }
            else
            {
                writeDocComment(out, op, deprecateReason);
            }
            out << nl << "public final "
                << typeToString(ret, TypeModeReturn, package, op->getMetaData(), true,
                                optionalMapping && op->returnIsOptional())
                << ' ' << opName << spar << params << epar;
            if(op->hasMetaData("UserException"))
            {
                out.inc();
                out << nl << "throws Ice.UserException";
                out.dec();
            }
            else
            {
                writeThrowsClause(package, throws);
            }
            out << sb << nl;
            if(ret)
            {
                out << "return ";
            }
            out << opName << spar << args << "null" << epar << ';';
            out << eb;
        }
    }

    //
    // Dispatch operations. We only generate methods for operations
    // defined in this ClassDef, because we reuse existing methods
    // for inherited operations.
    //
    ops = p->operations();
    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        StringList opMetaData = op->getMetaData();
        ContainerPtr container = op->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        assert(cl);
        string deprecateReason = getDeprecateReason(op, cl, "operation");

        string opName = op->name();
        out << sp;
        if(!deprecateReason.empty())
        {
            out << nl << "/** @deprecated **/";
        }
        out << nl << "public static Ice.DispatchStatus ___" << opName << '(' << name
            << " __obj, IceInternal.Incoming __inS, Ice.Current __current)";
        out << sb;

        const bool amd = cl->hasMetaData("amd") || op->hasMetaData("amd");
        const bool optionalMapping = useOptionalMapping(op);

        if(!amd)
        {
            TypePtr ret = op->returnType();

            ParamDeclList inParams;
            ParamDeclList outParams;
            ParamDeclList paramList = op->parameters();
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

            int iter;

            out << nl << "__checkMode(" << sliceModeToIceMode(op->mode()) << ", __current.mode);";

            if(!inParams.empty())
            {
                //
                // Unmarshal 'in' parameters.
                //
                out << nl << "IceInternal.BasicStream __is = __inS.startReadParams();";
                for(ParamDeclList::const_iterator pli = inParams.begin(); pli != inParams.end(); ++pli)
                {
                    TypePtr paramType = (*pli)->type();
                    string paramName = fixKwd((*pli)->name());
                    string typeS = typeToString(paramType, TypeModeIn, package, (*pli)->getMetaData(),
                                                true, (*pli)->optional());
                    if((*pli)->optional())
                    {
                        out << nl << typeS << ' ' << paramName << " = new " << typeS << "();";
                    }
                    else
                    {
                        BuiltinPtr builtin = BuiltinPtr::dynamicCast(paramType);
                        if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(paramType))
                        {
                            out << nl << typeS << "Holder " << paramName << " = new " << typeS << "Holder();";
                        }
                        else if(StructPtr::dynamicCast(paramType))
                        {
                            out << nl << typeS << ' ' << paramName << " = null;";
                        }
                        else
                        {
                            out << nl << typeS << ' ' << paramName << ';';
                        }
                    }
                }
                iter = 0;
                writeMarshalUnmarshalParams(out, package, inParams, 0, iter, false, true, true);
                if(op->sendsClasses(false))
                {
                    out << nl << "__is.readPendingObjects();";
                }
                out << nl << "__inS.endReadParams();";
            }
            else
            {
                out << nl << "__inS.readEmptyParams();";
            }

            //
            // Declare 'out' parameters.
            //
            for(ParamDeclList::const_iterator pli = outParams.begin(); pli != outParams.end(); ++pli)
            {
                string typeS = typeToString((*pli)->type(), TypeModeOut, package, (*pli)->getMetaData(), true,
                                            optionalMapping && (*pli)->optional());
                out << nl << typeS << ' ' << fixKwd((*pli)->name()) << " = new " << typeS << "();";
            }

            //
            // Call on the servant.
            //
            if(!throws.empty())
            {
                out << nl << "try";
                out << sb;
            }
            out << nl;
            if(ret)
            {
                string retS = typeToString(ret, TypeModeReturn, package, opMetaData, true,
                                           optionalMapping && op->returnIsOptional());
                out << retS << " __ret = ";
            }
            out << "__obj." << fixKwd(opName) << '(';
            for(ParamDeclList::const_iterator pli = inParams.begin(); pli != inParams.end(); ++pli)
            {
                TypePtr paramType = (*pli)->type();
                out << fixKwd((*pli)->name());
                if(!(*pli)->optional())
                {
                    BuiltinPtr builtin = BuiltinPtr::dynamicCast(paramType);
                    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(paramType))
                    {
                        out << ".value";
                    }
                }
                out << ", ";
            }
            for(ParamDeclList::const_iterator pli = outParams.begin(); pli != outParams.end(); ++pli)
            {
                out << fixKwd((*pli)->name()) << ", ";
            }
            out << "__current);";

            //
            // Marshal 'out' parameters and return value.
            //
            if(!outParams.empty() || ret)
            {
                out << nl << "IceInternal.BasicStream __os = __inS.__startWriteParams("
                    << opFormatTypeToString(op) << ");";
                writeMarshalUnmarshalParams(out, package, outParams, op, iter, true, optionalMapping, true);
                if(op->returnsClasses(false))
                {
                    out << nl << "__os.writePendingObjects();";
                }
                out << nl << "__inS.__endWriteParams(true);";
            }
            else
            {
                out << nl << "__inS.__writeEmptyParams();";
            }
            out << nl << "return Ice.DispatchStatus.DispatchOK;";

            //
            // Handle user exceptions.
            //
            if(!throws.empty())
            {
                out << eb;
                for(ExceptionList::const_iterator t = throws.begin(); t != throws.end(); ++t)
                {
                    string exS = getAbsolute(*t, package);
                    out << nl << "catch(" << exS << " ex)";
                    out << sb;
                    out << nl << "__inS.__writeUserException(ex, " << opFormatTypeToString(op) << ");";
                    out << nl << "return Ice.DispatchStatus.DispatchUserException;";
                    out << eb;
                }
            }

            out << eb;
        }
        else
        {
            ParamDeclList inParams;
            ParamDeclList paramList = op->parameters();
            for(ParamDeclList::const_iterator pli = paramList.begin(); pli != paramList.end(); ++pli)
            {
                if(!(*pli)->isOutParam())
                {
                    inParams.push_back(*pli);
                }
            }

            int iter;

            out << nl << "__checkMode(" << sliceModeToIceMode(op->mode()) << ", __current.mode);";

            if(!inParams.empty())
            {
                //
                // Unmarshal 'in' parameters.
                //
                out << nl << "IceInternal.BasicStream __is = __inS.startReadParams();";
                iter = 0;
                for(ParamDeclList::const_iterator pli = inParams.begin(); pli != inParams.end(); ++pli)
                {
                    TypePtr paramType = (*pli)->type();
                    string paramName = fixKwd((*pli)->name());
                    string typeS = typeToString(paramType, TypeModeIn, package, (*pli)->getMetaData(),
                                                true, (*pli)->optional());
                    if((*pli)->optional())
                    {
                        out << nl << typeS << ' ' << paramName << " = new " << typeS << "();";
                    }
                    else
                    {
                        BuiltinPtr builtin = BuiltinPtr::dynamicCast(paramType);
                        if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(paramType))
                        {
                            out << nl << typeS << "Holder " << paramName << " = new " << typeS << "Holder();";
                        }
                        else if(StructPtr::dynamicCast(paramType))
                        {
                            out << nl << typeS << ' ' << paramName << " = null;";
                        }
                        else
                        {
                            out << nl << typeS << ' ' << paramName << ';';
                        }
                    }
                }
                writeMarshalUnmarshalParams(out, package, inParams, 0, iter, false, true, true);
                if(op->sendsClasses(false))
                {
                    out << nl << "__is.readPendingObjects();";
                }
                out << nl << "__inS.endReadParams();";
            }
            else
            {
                out << nl << "__inS.readEmptyParams();";
            }

            //
            // Call on the servant.
            //
            string classNameAMD = "AMD_" + p->name();
            out << nl << '_' << classNameAMD << '_' << opName << " __cb = new _" << classNameAMD << '_' << opName
                << "(__inS);";
            out << nl << "try";
            out << sb;
            out << nl << "__obj." << (amd ? opName + "_async" : fixKwd(opName)) << (amd ? "(__cb, " : "(");
            for(ParamDeclList::const_iterator pli = inParams.begin(); pli != inParams.end(); ++pli)
            {
                TypePtr paramType = (*pli)->type();
                out << fixKwd((*pli)->name());
                if(!(*pli)->optional())
                {
                    BuiltinPtr builtin = BuiltinPtr::dynamicCast(paramType);
                    if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(paramType))
                    {
                        out << ".value";
                    }
                }
                out << ", ";
            }
            out << "__current);";
            out << eb;
            out << nl << "catch(java.lang.Exception ex)";
            out << sb;
            out << nl << "__cb.ice_exception(ex);";
            out << eb;
            out << nl << "catch(java.lang.Error ex)";
            out << sb;
            out << nl << "__cb.__error(ex);";
            out << eb;
            out << nl << "return Ice.DispatchStatus.DispatchAsync;";

            out << eb;
        }
    }

    OperationList allOps = p->allOperations();
    if(!allOps.empty())
    {
        StringList allOpNames;
        transform(allOps.begin(), allOps.end(), back_inserter(allOpNames), constMemFun(&Contained::name));
        allOpNames.push_back("ice_id");
        allOpNames.push_back("ice_ids");
        allOpNames.push_back("ice_isA");
        allOpNames.push_back("ice_ping");
        allOpNames.sort();
        allOpNames.unique();

        out << sp << nl << "private final static String[] __all =";
        out << sb;
        for(StringList::const_iterator q = allOpNames.begin(); q != allOpNames.end();)
        {
            out << nl << '"' << *q << '"';
            if(++q != allOpNames.end())
            {
                out << ',';
            }
        }
        out << eb << ';';

        out << sp;
        for(OperationList::iterator r = allOps.begin(); r != allOps.end(); ++r)
        {
            //
            // Suppress deprecation warnings if this method dispatches to a deprecated operation.
            //
            OperationPtr op = *r;
            ContainerPtr container = op->container();
            ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
            assert(cl);
            string deprecateReason = getDeprecateReason(op, cl, "operation");
            if(!deprecateReason.empty())
            {
                out << nl << "@SuppressWarnings(\"deprecation\")";
                break;
            }
        }
        out << nl << "public Ice.DispatchStatus __dispatch(IceInternal.Incoming in, Ice.Current __current)";
        out << sb;
        out << nl << "int pos = java.util.Arrays.binarySearch(__all, __current.operation);";
        out << nl << "if(pos < 0)";
        out << sb;
        out << nl << "throw new Ice.OperationNotExistException(__current.id, __current.facet, __current.operation);";
        out << eb;
        out << sp << nl << "switch(pos)";
        out << sb;
        int i = 0;
        for(StringList::const_iterator q = allOpNames.begin(); q != allOpNames.end(); ++q)
        {
            string opName = *q;

            out << nl << "case " << i++ << ':';
            out << sb;
            if(opName == "ice_id")
            {
                out << nl << "return ___ice_id(this, in, __current);";
            }
            else if(opName == "ice_ids")
            {
                out << nl << "return ___ice_ids(this, in, __current);";
            }
            else if(opName == "ice_isA")
            {
                out << nl << "return ___ice_isA(this, in, __current);";
            }
            else if(opName == "ice_ping")
            {
                out << nl << "return ___ice_ping(this, in, __current);";
            }
            else
            {
                //
                // There's probably a better way to do this.
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
                            out << nl << "return ___" << opName << "(this, in, __current);";
                        }
                        else
                        {
                            string base;
                            if(cl->isInterface())
                            {
                                base = getAbsolute(cl, package, "_", "Disp");
                            }
                            else
                            {
                                base = getAbsolute(cl, package);
                            }
                            out << nl << "return " << base << ".___" << opName << "(this, in, __current);";
                        }
                        break;
                    }
                }
            }
            out << eb;
        }
        out << eb;
        out << sp << nl << "assert(false);";
        out << nl << "throw new Ice.OperationNotExistException(__current.id, __current.facet, __current.operation);";
        out << eb;

        //
        // Check if we need to generate ice_operationAttributes()
        //

        map<string, int> attributesMap;
        for(OperationList::iterator r = allOps.begin(); r != allOps.end(); ++r)
        {
            int attributes = (*r)->attributes();
            if(attributes != 0)
            {
                attributesMap.insert(map<string, int>::value_type((*r)->name(), attributes));
            }
        }

        if(!attributesMap.empty())
        {
            out << sp << nl << "private final static int[] __operationAttributes =";
            out << sb;
            for(StringList::const_iterator q = allOpNames.begin(); q != allOpNames.end();)
            {
                int attributes = 0;
                string opName = *q;
                map<string, int>::iterator it = attributesMap.find(opName);
                if(it != attributesMap.end())
                {
                    attributes = it->second;
                }
                out << nl << attributes;
                if(++q != allOpNames.end())
                {
                    out << ',';
                }
                out  << " // " << opName;
            }
            out << eb << ';';

            out << sp << nl << "public int ice_operationAttributes(String operation)";
            out << sb;
            out << nl << "int pos = java.util.Arrays.binarySearch(__all, operation);";
            out << nl << "if(pos < 0)";
            out << sb;
            out << nl << "return -1;";
            out << eb;
            out << sp << nl << "return __operationAttributes[pos];";
            out << eb;
        }
    }

    int iter;
    DataMemberList members = p->dataMembers();
    DataMemberList optionalMembers = p->orderedOptionalDataMembers();
    bool basePreserved = p->inheritsMetaData("preserve-slice");
    bool preserved = p->hasMetaData("preserve-slice");

    if(preserved && !basePreserved)
    {
        out << sp << nl << "public void __write(IceInternal.BasicStream __os)";
        out << sb;
        out << nl << "__os.startWriteObject(__slicedData);";
        out << nl << "__writeImpl(__os);";
        out << nl << "__os.endWriteObject();";
        out << eb;

        out << sp << nl << "public void __read(IceInternal.BasicStream __is)";
        out << sb;
        out << nl << "__is.startReadObject();";
        out << nl << "__readImpl(__is);";
        out << nl << "__slicedData = __is.endReadObject(true);";
        out << eb;

        if(stream)
        {
            out << sp << nl << "public void __write(Ice.OutputStream __outS)";
            out << sb;
            if(preserved)
            {
                out << nl << "__outS.startObject(__slicedData);";
            }
            else
            {
                out << nl << "__outS.startObject(null);";
            }
            out << nl << "__writeImpl(__outS);";
            out << nl << "__outS.endObject();";
            out << eb;

            out << sp << nl << "public void __read(Ice.InputStream __inS)";
            out << sb;
            out << nl << "__inS.startObject();";
            out << nl << "__readImpl(__inS);";
            out << nl << "__slicedData = __inS.endObject(true);";
            out << eb;
        }
    }

    out << sp << nl << "protected void __writeImpl(IceInternal.BasicStream __os)";
    out << sb;
    out << nl << "__os.startWriteSlice(ice_staticId(), " << p->compactId() << (!base ? ", true" : ", false") << ");";
    iter = 0;
    for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
    {
        if(!(*d)->optional())
        {
            writeMarshalDataMember(out, package, *d, iter);
        }
    }
    for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
    {
        writeMarshalDataMember(out, package, *d, iter);
    }
    out << nl << "__os.endWriteSlice();";
    if(base)
    {
        out << nl << "super.__writeImpl(__os);";
    }
    out << eb;

    DataMemberList classMembers = p->classDataMembers();
    DataMemberList allClassMembers = p->allClassDataMembers();

    if(classMembers.size() != 0)
    {
        writePatcher(out, package, classMembers, optionalMembers, stream);
    }

    out << sp << nl << "protected void __readImpl(IceInternal.BasicStream __is)";
    out << sb;
    out << nl << "__is.startReadSlice();";

    int patchIter = 0;
    const bool needCustomPatcher = classMembers.size() > 1;
    iter = 0;
    for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
    {
        if(!(*d)->optional())
        {
            writeUnmarshalDataMember(out, package, *d, iter, needCustomPatcher, patchIter);
        }
    }
    for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
    {
        writeUnmarshalDataMember(out, package, *d, iter, needCustomPatcher, patchIter);
    }
    out << nl << "__is.endReadSlice();";
    if(base)
    {
        out << nl << "super.__readImpl(__is);";
    }
    out << eb;

    if(stream)
    {
        out << sp << nl << "protected void __writeImpl(Ice.OutputStream __outS)";
        out << sb;
        out << nl << "__outS.startSlice(ice_staticId(), " << p->compactId() << (!base ? ", true" : ", false") << ");";
        iter = 0;
        for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
        {
            if(!(*d)->optional())
            {
                writeStreamMarshalDataMember(out, package, *d, iter);
            }
        }
        for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
        {
            writeStreamMarshalDataMember(out, package, *d, iter);
        }
        out << nl << "__outS.endSlice();";
        if(base)
        {
            out << nl << "super.__writeImpl(__outS);";
        }
        out << eb;

        out << sp << nl << "protected void __readImpl(Ice.InputStream __inS)";
        out << sb;
        out << nl << "__inS.startSlice();";
        iter = 0;
        patchIter = 0;
        for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
        {
            if(!(*d)->optional())
            {
                writeStreamUnmarshalDataMember(out, package, *d, iter, needCustomPatcher, patchIter);
            }
        }
        for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
        {
            writeStreamUnmarshalDataMember(out, package, *d, iter, needCustomPatcher, patchIter);
        }
        out << nl << "__inS.endSlice();";
        if(base)
        {
            out << nl << "super.__readImpl(__inS);";
        }
        out << eb;
    }

    if(preserved && !basePreserved)
    {
        out << sp << nl << "protected Ice.SlicedData __slicedData;";
    }
}

void
Slice::JavaVisitor::writeConstantValue(Output& out, const TypePtr& type, const SyntaxTreeBasePtr& valueType,
                                       const string& value, const string& package)
{
    ConstPtr constant = ConstPtr::dynamicCast(valueType);
    if(constant)
    {
        out << getAbsolute(constant, package) << ".value";
    }
    else
    {
        BuiltinPtr bp;
        EnumPtr ep;
        if((bp = BuiltinPtr::dynamicCast(type)))
        {
            switch(bp->kind())
            {
                case Builtin::KindString:
                {
                    //
                    // Expand strings into the basic source character set. We can't use isalpha() and the like
                    // here because they are sensitive to the current locale.
                    //
                    static const string basicSourceChars = "abcdefghijklmnopqrstuvwxyz"
                                                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                                           "0123456789"
                                                           "_{}[]#()<>%:;.?*+-/^&|~!=,\\\"' ";
                    static const set<char> charSet(basicSourceChars.begin(), basicSourceChars.end());
                    out << "\"";

                    vector<unsigned char> u8buffer;                  // Buffer to convert multibyte characters

                    for(size_t i = 0; i < value.size();)
                    {
                        if(charSet.find(value[i]) == charSet.end())
                        {
                            char c = value[i];
                            if(static_cast<unsigned char>(c) < 128) // Single byte character
                            {
                                //
                                // Print as unicode if not in basic source character set
                                //
                                switch(c)
                                {
                                    //
                                    // Java doesn't want '\n' or '\r\n' encoded as universal
                                    // characters, that gives an error "unclosed string literal"
                                    //
                                    case '\r':
                                    {
                                        out << "\\r";
                                        break;
                                    }
                                    case '\n':
                                    {
                                        out << "\\n";
                                        break;
                                    }
                                    default:
                                    {
                                        out << u16CodePoint(c);
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                u8buffer.push_back(value[i]);
                            }
                        }
                        else
                        {
                            //
                            // Write any pedding characters in the utf8 buffer
                            //
                            if(!u8buffer.empty())
                            {
                                writeU8Buffer(u8buffer, out);
                                u8buffer.clear();
                            }
                            switch(value[i])
                            {
                                case '\\':
                                {
                                    string s = "\\";
                                    size_t j = i + 1;
                                    for(; j < value.size(); ++j)
                                    {
                                        if(value[j] != '\\')
                                        {
                                            break;
                                        }
                                        s += "\\";
                                    }

                                    //
                                    // An even number of slash \ will escape the backslash and
                                    // the codepoint will be interpreted as its charaters
                                    //
                                    // \\U00000041  - ['\\', 'U', '0', '0', '0', '0', '0', '0', '4', '1']
                                    // \\\U00000041 - ['\\', 'A'] (41 is the codepoint for 'A')
                                    //
                                    if(s.size() % 2 != 0 && (value[j] == 'U' || value[j] == 'u'))
                                    {
                                        size_t sz = value[j] == 'U' ? 8 : 4;
                                        out << s.substr(0, s.size() - 1);
                                        i = j + 1;

                                        string codepoint = value.substr(j + 1, sz);
                                        assert(codepoint.size() ==  sz);

                                        IceUtil::Int64 v = IceUtilInternal::strToInt64(codepoint.c_str(), 0, 16);


                                        //
                                        // Java doesn't like this special characters encoded as universal characters
                                        //
                                        if(v == 0x5c)
                                        {
                                            out << "\\\\";
                                        }
                                        else if(v == 0xa)
                                        {
                                            out << "\\n";
                                        }
                                        else if(v == 0xd)
                                        {
                                            out << "\\r";
                                        }
                                        else if(v == 0x22)
                                        {
                                            out << "\\\"";
                                        }
                                        //
                                        // Unicode character in the range U+10000 to U+10FFFF is not permitted in a character literal
                                        // and is represented using a Unicode surrogate pair.
                                        //
                                        else if(v > 0xFFFF)
                                        {
                                            unsigned int high = ((static_cast<unsigned int>(v) - 0x10000) / 0x400) + 0xD800;
                                            unsigned int low = ((static_cast<unsigned int>(v) - 0x10000) % 0x400) + 0xDC00;
                                            out << u16CodePoint(high);
                                            out << u16CodePoint(low);
                                        }
                                        else
                                        {
                                            out << u16CodePoint(static_cast<unsigned int>(v));
                                        }

                                        i = j + 1 + sz;
                                    }
                                    else
                                    {
                                        out << s;
                                        i = j;
                                    }
                                    continue;
                                }
                                case '"':
                                {
                                    out << "\\";
                                    break;
                                }
                            }
                            out << value[i];                        // Print normally if in basic source character set
                        }
                        i++;
                    }

                    //
                    // Write any pedding characters in the utf8 buffer
                    //
                    if(!u8buffer.empty())
                    {
                        writeU8Buffer(u8buffer, out);
                        u8buffer.clear();
                    }

                    out << "\"";
                    break;
                }
                case Builtin::KindByte:
                {
                    int i = atoi(value.c_str());
                    if(i > 127)
                    {
                        i -= 256;
                    }
                    out << i; // Slice byte runs from 0-255, Java byte runs from -128 - 127.
                    break;
                }
                case Builtin::KindLong:
                {
                    out << value << "L"; // Need to append "L" modifier for long constants.
                    break;
                }
                case Builtin::KindBool:
                case Builtin::KindShort:
                case Builtin::KindInt:
                case Builtin::KindDouble:
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                {
                    out << value;
                    break;
                }
                case Builtin::KindFloat:
                {
                    out << value << "F";
                    break;
                }
            }

        }
        else if((ep = EnumPtr::dynamicCast(type)))
        {
            string val = value;
            string::size_type pos = val.rfind(':');
            if(pos != string::npos)
            {
                val.erase(0, pos + 1);
            }
            out << getAbsolute(ep, package) << '.' << fixKwd(val);
        }
        else
        {
            out << value;
        }
    }
}

void
Slice::JavaVisitor::writeDataMemberInitializers(Output& out, const DataMemberList& members, const string& package)
{
    for(DataMemberList::const_iterator p = members.begin(); p != members.end(); ++p)
    {
        TypePtr t = (*p)->type();
        if((*p)->defaultValueType())
        {
            if((*p)->optional())
            {
                string capName = (*p)->name();
                capName[0] = toupper(static_cast<unsigned char>(capName[0]));
                out << nl << "set" << capName << '(';
                writeConstantValue(out, t, (*p)->defaultValueType(), (*p)->defaultValue(), package);
                out << ");";
            }
            else
            {
                out << nl << fixKwd((*p)->name()) << " = ";
                writeConstantValue(out, t, (*p)->defaultValueType(), (*p)->defaultValue(), package);
                out << ';';
            }
        }
        else
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(t);
            if(builtin && builtin->kind() == Builtin::KindString)
            {
                out << nl << fixKwd((*p)->name()) << " = \"\";";
            }

            EnumPtr en = EnumPtr::dynamicCast(t);
            if(en)
            {
                string firstEnum = fixKwd(en->getEnumerators().front()->name());
                out << nl << fixKwd((*p)->name()) << " = " << getAbsolute(en, package) << '.' << firstEnum << ';';
            }

            StructPtr st = StructPtr::dynamicCast(t);
            if(st)
            {
                string memberType = typeToString(st, TypeModeMember, package, (*p)->getMetaData());
                out << nl << fixKwd((*p)->name()) << " = new " << memberType << "();";
            }
        }
    }
}

StringList
Slice::JavaVisitor::splitComment(const ContainedPtr& p)
{
    StringList result;

    string comment = p->comment();
    string::size_type pos = 0;
    string::size_type nextPos;
    while((nextPos = comment.find_first_of('\n', pos)) != string::npos)
    {
        result.push_back(string(comment, pos, nextPos - pos));
        pos = nextPos + 1;
    }
    string lastLine = string(comment, pos);
    if(lastLine.find_first_not_of(" \t\n\r") != string::npos)
    {
        result.push_back(lastLine);
    }

    return result;
}

void
Slice::JavaVisitor::writeDocComment(Output& out, const ContainedPtr& p, const string& deprecateReason,
                                    const string& extraParam)
{
    StringList lines = splitComment(p);
    if(lines.empty())
    {
        if(!deprecateReason.empty())
        {
            out << nl << "/**";
            out << nl << " * @deprecated " << deprecateReason;
            out << nl << " **/";
        }
        return;
    }

    out << nl << "/**";

    bool doneExtraParam = false;
    for(StringList::const_iterator i = lines.begin(); i != lines.end(); ++i)
    {
        //
        // @param must precede @return, so emit any extra parameter
        // when @return is seen.
        //
        if(i->find("@return") != string::npos && !extraParam.empty())
        {
            out << nl << " * " << extraParam;
            doneExtraParam = true;
        }
        out << nl << " *";
        if(!(*i).empty())
        {
            out << " " << *i;
        }
    }

    if(!doneExtraParam && !extraParam.empty())
    {
        //
        // Above code doesn't emit the comment for the extra parameter
        // if the operation returns a void or doesn't have an @return.
        //
        out << nl << " * " << extraParam;
    }

    if(!deprecateReason.empty())
    {
        out << nl << " * @deprecated " << deprecateReason;
    }

    out << nl << " **/";
}

void
Slice::JavaVisitor::writeDocComment(Output& out, const string& deprecateReason, const string& summary)
{
    vector<string> lines;
    IceUtilInternal::splitString(summary, "\n", lines);

    out << nl << "/**";
    for(vector<string>::const_iterator i = lines.begin(); i != lines.end(); ++i)
    {
        out << nl << " *";
        if(!(*i).empty())
        {
            out << " " << *i;
        }
    }

    if(!deprecateReason.empty())
    {
        out << nl << " * @deprecated " << deprecateReason;
    }

    out << nl << " **/";
}

void
Slice::JavaVisitor::writeDocCommentOp(Output& out, const OperationPtr& p)
{
    ContainerPtr container = p->container();
    ContainedPtr contained = ContainedPtr::dynamicCast(container);
    string deprecateReason = getDeprecateReason(p, contained, "operation");

    StringList lines = splitComment(p);

    if(lines.empty() && deprecateReason.empty())
    {
        return;
    }


    out << sp << nl << "/**";

    //
    // Output the leading comment block up until the first @tag.
    //
    bool done = false;
    for(StringList::const_iterator i = lines.begin(); i != lines.end() && !done; ++i)
    {
        if((*i)[0] == '@')
        {
            done = true;
        }
        else
        {
            out << nl << " * " << *i;
        }
    }

    if(!deprecateReason.empty())
    {
        out << nl << " * @deprecated " << deprecateReason;
    }

    out << nl << " **/";
}

void
Slice::JavaVisitor::writeDocCommentAsync(Output& out, const OperationPtr& p, ParamDir paramType,
                                         const string& extraParam)
{
    ContainerPtr container = p->container();
    ClassDefPtr contained = ClassDefPtr::dynamicCast(container);
    string deprecateReason = getDeprecateReason(p, contained, "operation");

    StringList lines = splitComment(p);
    if(lines.empty() && deprecateReason.empty())
    {
        return;
    }

    out << nl << "/**";

    if(paramType == OutParam)
    {
        out << nl << " * ice_response indicates that";
        out << nl << " * the operation completed successfully.";

        //
        // Find the comment for the return value (if any) and rewrite that as an @param comment.
        //
        const string returnTag = "@return";
        bool doneReturn = false;
        bool foundReturn = false;
        for(StringList::const_iterator i = lines.begin(); i != lines.end() && !doneReturn; ++i)
        {
            if(!foundReturn)
            {
                if(i->find(returnTag) != string::npos)
                {
                    foundReturn = true;
                    out << nl << " * @param __ret (return value)" << i->substr(returnTag.length());
                }
            }
            else
            {
                if((*i)[0] == '@')
                {
                    doneReturn = true;
                }
                else
                {
                    out << nl << " * " << *i;
                }
            }
        }
    }
    else
    {
        //
        // Output the leading comment block up until the first @tag.
        //
        bool done = false;
        for(StringList::const_iterator i = lines.begin(); i != lines.end() && !done; ++i)
        {
            if((*i)[0] == '@')
            {
                done = true;
            }
            else
            {
                out << nl << " * " << *i;
            }
        }
    }

    //
    // Write the comments for the parameters.
    //
    writeDocCommentParam(out, p, paramType);

    if(!extraParam.empty())
    {
        out << nl << " * " << extraParam;
    }

    if(paramType == InParam)
    {
        if(!deprecateReason.empty())
        {
            out << nl << " * @deprecated " << deprecateReason;
        }
    }

    out << nl << " **/";
}

void
Slice::JavaVisitor::writeDocCommentAMI(Output& out, const OperationPtr& p, ParamDir paramType,
                                       const string& extraParam1, const string& extraParam2, const string& extraParam3,
                                       const string& extraParam4, const string& extraParam5)
{
    ContainerPtr container = p->container();
    ClassDefPtr contained = ClassDefPtr::dynamicCast(container);
    string deprecateReason = getDeprecateReason(p, contained, "operation");

    StringList lines = splitComment(p);
    if(lines.empty() && deprecateReason.empty())
    {
        return;
    }

    out << nl << "/**";

    //
    // Output the leading comment block up until the first @tag.
    //
    bool done = false;
    for(StringList::const_iterator i = lines.begin(); i != lines.end() && !done; ++i)
    {
        if((*i)[0] == '@')
        {
            done = true;
        }
        else
        {
            out << nl << " * " << *i;
        }
    }

    //
    // Write the comments for the parameters.
    //
    writeDocCommentParam(out, p, paramType, false);

    if(!extraParam1.empty())
    {
        out << nl << " * " << extraParam1;
    }

    if(!extraParam2.empty())
    {
        out << nl << " * " << extraParam2;
    }

    if(!extraParam3.empty())
    {
        out << nl << " * " << extraParam3;
    }

    if(!extraParam4.empty())
    {
        out << nl << " * " << extraParam4;
    }

    if(!extraParam5.empty())
    {
        out << nl << " * " << extraParam5;
    }

    if(paramType == InParam)
    {
        out << nl << " * @return The asynchronous result object.";
        if(!deprecateReason.empty())
        {
            out << nl << " * @deprecated " << deprecateReason;
        }
    }
    else
    {
        out << nl << " * @param __result The asynchronous result object.";
        //
        // Print @return, @throws, and @see tags.
        //
        const string returnTag = "@return";
        const string throwsTag = "@throws";
        const string seeTag = "@see";
        bool found = false;
        for(StringList::const_iterator i = lines.begin(); i != lines.end(); ++i)
        {
            if(!found)
            {
                if(i->find(returnTag) != string::npos || i->find(throwsTag) != string::npos ||
                   i->find(seeTag) != string::npos)
                {
                    found = true;
                }
            }

            if(found)
            {
                out << nl << " * " << *i;
            }
        }
    }

    out << nl << " **/";
}

void
Slice::JavaVisitor::writeDocCommentParam(Output& out, const OperationPtr& p, ParamDir paramType, bool cb)
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
    // Print a comment for the callback parameter.
    //
    if(cb && paramType == InParam)
    {
        out << nl << " * @param __cb The callback object for the operation.";
    }

    //
    // Print the comments for all the parameters that appear in the parameter list.
    //
    const string paramTag = "@param";
    StringList lines = splitComment(p);
    StringList::const_iterator i = lines.begin();
    while(i != lines.end())
    {
        string line = *i++;
        if(line.find(paramTag) != string::npos)
        {
            string::size_type paramNamePos = line.find_first_not_of(" \t\n\r", paramTag.length());
            if(paramNamePos != string::npos)
            {
                string::size_type paramNameEndPos = line.find_first_of(" \t", paramNamePos);
                string paramName = line.substr(paramNamePos, paramNameEndPos - paramNamePos);
                if(std::find(params.begin(), params.end(), paramName) != params.end())
                {
                    out << nl << " * " << line;
                    StringList::const_iterator j;
                    if (i == lines.end())
                    {
                        break;
                    }
                    j = i++;
                    while(j != lines.end())
                    {
                        if((*j)[0] != '@')
                        {
                            i = j;
                            out << nl << " * " << *j++;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }
        }
    }
}

Slice::Gen::Gen(const string& /*name*/, const string& base, const vector<string>& includePaths, const string& dir) :
    _base(base),
    _includePaths(includePaths),
    _dir(dir)
{
}

Slice::Gen::~Gen()
{
}

void
Slice::Gen::generate(const UnitPtr& p, bool stream)
{
    JavaGenerator::validateMetaData(p);

    OpsVisitor opsVisitor(_dir);
    p->visit(&opsVisitor, false);

    PackageVisitor packageVisitor(_dir);
    p->visit(&packageVisitor, false);

    TypesVisitor typesVisitor(_dir, stream);
    p->visit(&typesVisitor, false);

    CompactIdVisitor compactIdVisitor(_dir);
    p->visit(&compactIdVisitor, false);

    HolderVisitor holderVisitor(_dir);
    p->visit(&holderVisitor, false);

    HelperVisitor helperVisitor(_dir, stream);
    p->visit(&helperVisitor, false);

    ProxyVisitor proxyVisitor(_dir);
    p->visit(&proxyVisitor, false);

    DispatcherVisitor dispatcherVisitor(_dir, stream);
    p->visit(&dispatcherVisitor, false);

    AsyncVisitor asyncVisitor(_dir);
    p->visit(&asyncVisitor, false);
}

void
Slice::Gen::generateTie(const UnitPtr& p)
{
    TieVisitor tieVisitor(_dir);
    p->visit(&tieVisitor, false);
}

void
Slice::Gen::generateImpl(const UnitPtr& p)
{
    ImplVisitor implVisitor(_dir);
    p->visit(&implVisitor, false);
}

void
Slice::Gen::generateImplTie(const UnitPtr& p)
{
    ImplTieVisitor implTieVisitor(_dir);
    p->visit(&implTieVisitor, false);
}

void
Slice::Gen::writeChecksumClass(const string& checksumClass, const string& dir, const ChecksumMap& m)
{
    //
    // Attempt to open the source file for the checksum class.
    //
    JavaOutput out;
    out.openClass(checksumClass, dir);

    //
    // Get the class name.
    //
    string className;
    string::size_type pos = checksumClass.rfind('.');
    if(pos == string::npos)
    {
        className = checksumClass;
    }
    else
    {
        className = checksumClass.substr(pos + 1);
    }

    //
    // Emit the class.
    //
    out << sp << nl << "public class " << className;
    out << sb;

    //
    // Use a static initializer to populate the checksum map.
    //
    out << sp << nl << "public static final java.util.Map<String, String> checksums;";
    out << sp << nl << "static";
    out << sb;
    out << nl << "java.util.Map<String, String> map = new java.util.HashMap<String, String>();";
    for(ChecksumMap::const_iterator p = m.begin(); p != m.end(); ++p)
    {
        out << nl << "map.put(\"" << p->first << "\", \"";
        ostringstream str;
        str.flags(ios_base::hex);
        str.fill('0');
        for(vector<unsigned char>::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
        {
            str << static_cast<int>(*q);
        }
        out << str.str() << "\");";
    }
    out << nl << "checksums = java.util.Collections.unmodifiableMap(map);";

    out << eb;
    out << eb;
    out << nl;
}

Slice::Gen::OpsVisitor::OpsVisitor(const string& dir) :
    JavaVisitor(dir)
{
}

bool
Slice::Gen::OpsVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    //
    // Don't generate an Operations interface for non-abstract classes
    //
    if(!p->isAbstract())
    {
        return false;
    }

    if(!p->isLocal())
    {
        writeOperations(p, false);
    }
    writeOperations(p, true);

    return false;
}

void
Slice::Gen::OpsVisitor::writeOperations(const ClassDefPtr& p, bool noCurrent)
{
    string name = p->name();
    ClassList bases = p->bases();
    string package = getPackage(p);
    string opIntfName = "Operations";
    if(noCurrent || p->isLocal())
    {
        opIntfName += "NC";
    }
    string absolute = getAbsolute(p, "", "_", opIntfName);

    open(absolute, p->file());

    Output& out = output();

    //
    // Generate the operations interface
    //
    out << sp;
    writeDocComment(out, p, getDeprecateReason(p, 0, p->isInterface() ? "interface" : "class"));
    out << nl << "public interface " << '_' << name << opIntfName;
    if((bases.size() == 1 && bases.front()->isAbstract()) || bases.size() > 1)
    {
        out << " extends ";
        out.useCurrentPosAsIndent();
        bool first = true;
        for(ClassList::const_iterator q = bases.begin(); q != bases.end();)
        {
            if((*q)->isAbstract())
            {
                if(!first)
                {
                    out << ',' << nl;
                }
                else
                {
                    first = false;
                }
                out << getAbsolute(*q, package, "_", opIntfName);
            }
            ++q;
        }
        out.restoreIndent();
    }
    out << sb;

    OperationList ops = p->operations();
    for(OperationList::iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        ContainerPtr container = op->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        string opname = op->name();

        TypePtr ret;
        vector<string> params;

        const bool amd = !p->isLocal() && (cl->hasMetaData("amd") || op->hasMetaData("amd"));
        const bool optionalMapping = useOptionalMapping(op);

        if(amd)
        {
            params = getParamsAsync(op, package, true, true);
        }
        else
        {
            params = getParams(op, package, false, optionalMapping);
            ret = op->returnType();
        }

        string retS = typeToString(ret, TypeModeReturn, package, op->getMetaData(), true,
                                   optionalMapping && op->returnIsOptional());
        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();
        out << sp;

        string deprecateReason = getDeprecateReason(*r, p, "operation");
        string extraCurrent;
        if(!noCurrent && !p->isLocal())
        {
            extraCurrent = "@param __current The Current object for the invocation.";
        }
        if(amd)
        {
            writeDocCommentAsync(out, *r, InParam, extraCurrent);
        }
        else
        {
            writeDocComment(out, *r, deprecateReason, extraCurrent);
        }
        out << nl << retS << ' ' << (amd ? opname + "_async" : fixKwd(opname)) << spar << params;
        if(!noCurrent && !p->isLocal())
        {
            out << "Ice.Current __current";
        }
        out << epar;
        if(op->hasMetaData("UserException"))
        {
            out.inc();
            out << nl << "throws Ice.UserException";
            out.dec();
        }
        else
        {
            writeThrowsClause(package, throws);
        }
        out << ';';
    }

    out << eb;

    close();
}

Slice::Gen::TieVisitor::TieVisitor(const string& dir) :
    JavaVisitor(dir)
{
}

bool
Slice::Gen::TieVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = p->name();
    ClassList bases = p->bases();
    string package = getPackage(p);
    string absolute = getAbsolute(p, "", "_", "Tie");
    string opIntfName = "Operations";
    if(p->isLocal())
    {
        opIntfName += "NC";
    }

    //
    // Don't generate a TIE class for a non-abstract class
    //
    if(!p->isAbstract())
    {
        return false;
    }

    open(absolute, p->file());

    Output& out = output();

    //
    // Generate the TIE class
    //
    out << sp << nl << "public class " << '_' << name << "Tie";
    if(p->isInterface())
    {
        if(p->isLocal())
        {
            out << " implements " << fixKwd(name) << ", Ice.TieBase";
        }
        else
        {
            out << " extends " << '_' << name << "Disp implements Ice.TieBase";
        }
    }
    else
    {
        out << " extends " << fixKwd(name) << " implements Ice.TieBase";
    }

    out << sb;

    out << sp << nl << "public _" << name << "Tie()";
    out << sb;
    out << eb;

    out << sp << nl << "public _" << name << "Tie(" << '_' << name << opIntfName << " delegate)";
    out << sb;
    out << nl << "_ice_delegate = delegate;";
    out << eb;

    out << sp << nl << "public java.lang.Object ice_delegate()";
    out << sb;
    out << nl << "return _ice_delegate;";
    out << eb;

    out << sp << nl << "public void ice_delegate(java.lang.Object delegate)";
    out << sb;
    out << nl << "_ice_delegate = (_" << name << opIntfName << ")delegate;";
    out << eb;

    out << sp << nl << "public boolean equals(java.lang.Object rhs)";
    out << sb;
    out << nl << "if(this == rhs)";
    out << sb;
    out << nl << "return true;";
    out << eb;
    out << nl << "if(!(rhs instanceof " << '_' << name << "Tie))";
    out << sb;
    out << nl << "return false;";
    out << eb;
    out << sp << nl << "return _ice_delegate.equals(((" << '_' << name << "Tie)rhs)._ice_delegate);";
    out << eb;

    out << sp << nl << "public int hashCode()";
    out << sb;
    out << nl << "return _ice_delegate.hashCode();";
    out << eb;

    if(p->isLocal())
    {
        out << sp << nl << "public _" << name << "Tie clone()";
        out.inc();
        out << nl << "throws java.lang.CloneNotSupportedException";
        out.dec();
        out << sb;
        out << nl << "return (_" << name << "Tie)super.clone();";
        out << eb;
    }

    OperationList ops = p->allOperations();
    for(OperationList::iterator r = ops.begin(); r != ops.end(); ++r)
    {
        ContainerPtr container = (*r)->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        const bool hasAMD = cl->hasMetaData("amd") || (*r)->hasMetaData("amd");
        const bool optionalMapping = useOptionalMapping(*r);

        string opName = hasAMD ? (*r)->name() + "_async" : fixKwd((*r)->name());

        TypePtr ret = (*r)->returnType();
        string retS = typeToString(ret, TypeModeReturn, package, (*r)->getMetaData(), true,
                                   optionalMapping && (*r)->returnIsOptional());

        vector<string> params;
        vector<string> args;
        if(hasAMD)
        {
            params = getParamsAsync((*r), package, true, true);
            args = getArgsAsync(*r);
        }
        else
        {
            params = getParams((*r), package, false, optionalMapping);
            args = getArgs(*r);
        }

        string deprecateReason = getDeprecateReason(*r, cl, "operation");

        out << sp;
        if(!deprecateReason.empty())
        {
            out << nl << "@Deprecated";
            out << nl << "@SuppressWarnings(\"deprecation\")";
        }
        out << nl << "public " << (hasAMD ? string("void") : retS) << ' ' << opName << spar << params;
        if(!p->isLocal())
        {
            out << "Ice.Current __current";
        }
        out << epar;

        if((*r)->hasMetaData("UserException"))
        {
            out.inc();
            out << nl << "throws Ice.UserException";
            out.dec();
        }
        else
        {
            ExceptionList throws = (*r)->throws();
            throws.sort();
            throws.unique();
            writeThrowsClause(package, throws);
        }
        out << sb;
        out << nl;
        if(ret && !hasAMD)
        {
            out << "return ";
        }
        out << "_ice_delegate." << opName << spar << args;
        if(!p->isLocal())
        {
            out << "__current";
        }
        out << epar << ';';
        out << eb;
    }

    out << sp << nl << "private " << '_' << name << opIntfName << " _ice_delegate;";
    out << sp << nl << "public static final long serialVersionUID = ";
    string serialVersionUID;
    if(p->findMetaData("java:serialVersionUID", serialVersionUID))
    {
        string::size_type pos = serialVersionUID.rfind(":") + 1;
        if(pos == string::npos)
        {
            ostringstream os;
            os << "ignoring invalid serialVersionUID for class `" << p->scoped() << "'; generating default value";
            emitWarning("", "", os.str());
            out << computeSerialVersionUUID(p);
        }
        else
        {
            Int64 v = 0;
            serialVersionUID = serialVersionUID.substr(pos);
            if(serialVersionUID != "0")
            {
                if(!stringToInt64(serialVersionUID, v)) // conversion error
                {
                    ostringstream os;
                    os << "ignoring invalid serialVersionUID for class `" << p->scoped()
                       << "'; generating default value";
                    emitWarning("", "", os.str());
                    out << computeSerialVersionUUID(p);
                }
            }
            out << v;
        }
    }
    else
    {
        out << computeSerialVersionUUID(p);
    }
    out << "L;";
    out << eb;
    close();

    return false;
}

Slice::Gen::PackageVisitor::PackageVisitor(const string& dir) :
    JavaVisitor(dir)
{
}

bool
Slice::Gen::PackageVisitor::visitModuleStart(const ModulePtr& p)
{
    string prefix = getPackagePrefix(p);
    if(!prefix.empty())
    {
        string markerClass = prefix + "." + fixKwd(p->name()) + "._Marker";
        open(markerClass, p->file());

        Output& out = output();

        out << sp << nl << "interface _Marker";
        out << sb;
        out << eb;

        close();
    }
    return false;
}

Slice::Gen::TypesVisitor::TypesVisitor(const string& dir, bool stream) :
    JavaVisitor(dir), _stream(stream)
{
}

bool
Slice::Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = p->name();
    ClassList bases = p->bases();
    ClassDefPtr baseClass;
    if(!bases.empty() && !bases.front()->isInterface())
    {
        baseClass = bases.front();
    }

    string package = getPackage(p);
    string absolute = getAbsolute(p);
    DataMemberList members = p->dataMembers();
    DataMemberList allDataMembers = p->allDataMembers();

    open(absolute, p->file());

    Output& out = output();

    //
    // Slice interfaces map to Java interfaces.
    //
    out << sp;
    writeDocComment(out, p, getDeprecateReason(p, 0, p->isInterface() ? "interface" : "class"));
    if(p->isInterface())
    {
        out << nl << "public interface " << fixKwd(name);
        if(!p->isLocal())
        {
            out << " extends ";
            out.useCurrentPosAsIndent();
            out << "Ice.Object";
            out << "," << nl << '_' << name;
            out << "Operations, _" << name << "OperationsNC";
        }
        else
        {
            if(!bases.empty())
            {
                out << " extends ";
            }
            out.useCurrentPosAsIndent();
        }

        ClassList::const_iterator q = bases.begin();
        if(p->isLocal() && q != bases.end())
        {
            out << getAbsolute(*q++, package);
        }
        while(q != bases.end())
        {
            out << ',' << nl << getAbsolute(*q, package);
            q++;
        }
        out.restoreIndent();
    }
    else
    {
        out << nl << "public ";
        if(p->allOperations().size() > 0) // Don't use isAbstract() - see bug 3739
        {
            out << "abstract ";
        }
        out << "class " << fixKwd(name);
        out.useCurrentPosAsIndent();

	StringList implements;
        bool implementsOnNewLine = true;

        if(bases.empty() || bases.front()->isInterface())
        {
            if(p->isLocal())
            {
                implementsOnNewLine = false;
		implements.push_back("java.lang.Cloneable");
            }
            else
            {
                out << " extends Ice.ObjectImpl";
            }
        }
        else
        {
            out << " extends ";
            out << getAbsolute(baseClass, package);
            bases.pop_front();
        }

        //
        // Implement interfaces
        //

        if(p->isAbstract())
        {
            if(!p->isLocal())
            {
                implements.push_back("_" + name + "Operations");
                implements.push_back("_" + name + "OperationsNC");
            }
        }
        if(!bases.empty())
        {
            for(ClassList::const_iterator q = bases.begin(); q != bases.end();)
            {
                implements.push_back(getAbsolute(*q, package));
                q++;
            }
        }

        if(!implements.empty())
        {
            if(implementsOnNewLine)
            {
                out << nl;
            }

            out << " implements ";
            out.useCurrentPosAsIndent();

            for(StringList::const_iterator q = implements.begin(); q != implements.end();)
            {
                if(q != implements.begin())
                {
                    out << ',' << nl;
                }
                out << *q;
                q++;
            }

            out.restoreIndent();
        }

        out.restoreIndent();
    }

    out << sb;

    //
    // For local classes and interfaces, we don't use the OperationsNC interface.
    // Instead, we generate the operation signatures directly into the class
    // or interface.
    //
    if(p->isLocal())
    {
        OperationList ops = p->operations();
        for(OperationList::iterator r = ops.begin(); r != ops.end(); ++r)
        {
            OperationPtr op = *r;
            const ContainerPtr container = op->container();
            const ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
            const string opname = op->name();
            const bool optionalMapping = useOptionalMapping(op);

            const TypePtr ret = op->returnType();
            vector<string> params = getParams(op, package, true, optionalMapping);

            string retS = typeToString(ret, TypeModeReturn, package, op->getMetaData(), true,
                                       optionalMapping && op->returnIsOptional());
            ExceptionList throws = op->throws();
            throws.sort();
            throws.unique();
            out << sp;

            writeDocComment(out, *r, getDeprecateReason(*r, p, "operation"));
            out << nl;
            if(!p->isInterface())
            {
                out << "public abstract ";
            }
            out << retS << ' ' << fixKwd(opname) << spar << params << epar;
            if(op->hasMetaData("UserException"))
            {
                out.inc();
                out << nl << "throws Ice.UserException";
                out.dec();
            }
            else
            {
                writeThrowsClause(package, throws);
            }
            out << ';';

            //
            // Generate asynchronous API for local operations marked with "async" metadata.
            //
            if(p->hasMetaData("async") || op->hasMetaData("async"))
            {
                vector<string> inParams = getInOutParams(op, package, InParam, true, true);

                out << sp;
                writeDocCommentAMI(out, op, InParam);
                out << nl;
                if(!p->isInterface())
                {
                    out << "public abstract ";
                }
                out << "Ice.AsyncResult begin_" << opname << spar << inParams << epar << ';';

                out << sp;
                writeDocCommentAMI(out, op, InParam);
                out << nl;
                if(!p->isInterface())
                {
                    out << "public abstract ";
                }
                out << "Ice.AsyncResult begin_" << opname << spar << inParams << "Ice.Callback __cb" << epar << ';';

                out << sp;
                writeDocCommentAMI(out, op, InParam);
                out << nl;
                if(!p->isInterface())
                {
                    out << "public abstract ";
                }
                string cb = "Callback_" + name + "_" + opname + " __cb";
                out << "Ice.AsyncResult begin_" << opname << spar << inParams << cb << epar << ';';


                out << sp;
                writeDocCommentAMI(out, op, InParam);
                out << nl;
                if(!p->isInterface())
                {
                    out << "public abstract ";
                }
                out << nl << "Ice.AsyncResult begin_" << opname;
                writeParamList(out, getParamsAsyncLambda(op, package, false, true));
                out << ';';

                vector<string> outParams = getInOutParams(op, package, OutParam, true, true);
                out << sp;
                writeDocCommentAMI(out, op, OutParam);
                out << nl;
                if(!p->isInterface())
                {
                    out << "public abstract ";
                }

                out << retS << " end_" << opname << spar << outParams << "Ice.AsyncResult __result" << epar << ';';
            }
        }
    }

    if(!p->isInterface() && !allDataMembers.empty())
    {
        bool hasOptionalMembers = false;
        bool hasRequiredMembers = false;

        for(DataMemberList::const_iterator d = allDataMembers.begin(); d != allDataMembers.end(); ++d)
        {
            if((*d)->optional())
            {
                hasOptionalMembers = true;
            }
            else
            {
                hasRequiredMembers = true;
            }
        }

        //
        // Default constructor.
        //
        out << sp;
        out << nl << "public " << fixKwd(name) << "()";
        out << sb;
        if(baseClass)
        {
            out << nl << "super();";
        }
        writeDataMemberInitializers(out, members, package);
        out << eb;

        //
        // A method cannot have more than 255 parameters (including the implicit "this" argument).
        //
        if(allDataMembers.size() < 255)
        {
            DataMemberList baseDataMembers;
            if(baseClass)
            {
                baseDataMembers = baseClass->allDataMembers();
            }

            if(hasRequiredMembers && hasOptionalMembers)
            {
                //
                // Generate a constructor accepting parameters for just the required members.
                //
                out << sp << nl << "public " << fixKwd(name) << spar;
                vector<string> paramDecl;
                for(DataMemberList::const_iterator d = allDataMembers.begin(); d != allDataMembers.end(); ++d)
                {
                    if(!(*d)->optional())
                    {
                        string memberName = fixKwd((*d)->name());
                        string memberType = typeToString((*d)->type(), TypeModeMember, package, (*d)->getMetaData());
                        paramDecl.push_back(memberType + " " + memberName);
                    }
                }
                out << paramDecl << epar;
                out << sb;
                if(!baseDataMembers.empty())
                {
                    bool hasBaseRequired = false;
                    for(DataMemberList::const_iterator d = baseDataMembers.begin(); d != baseDataMembers.end(); ++d)
                    {
                        if(!(*d)->optional())
                        {
                            hasBaseRequired = true;
                            break;
                        }
                    }
                    if(hasBaseRequired)
                    {
                        out << nl << "super" << spar;
                        vector<string> baseParamNames;
                        for(DataMemberList::const_iterator d = baseDataMembers.begin(); d != baseDataMembers.end(); ++d)
                        {
                            if(!(*d)->optional())
                            {
                                baseParamNames.push_back(fixKwd((*d)->name()));
                            }
                        }
                        out << baseParamNames << epar << ';';
                    }
                }

                for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
                {
                    if(!(*d)->optional())
                    {
                        string paramName = fixKwd((*d)->name());
                        out << nl << "this." << paramName << " = " << paramName << ';';
                    }
                }
                writeDataMemberInitializers(out, p->orderedOptionalDataMembers(), package);
                out << eb;
            }

            //
            // Generate a constructor accepting parameters for all members.
            //
            out << sp << nl << "public " << fixKwd(name) << spar;
            vector<string> paramDecl;
            for(DataMemberList::const_iterator d = allDataMembers.begin(); d != allDataMembers.end(); ++d)
            {
                string memberName = fixKwd((*d)->name());
                string memberType = typeToString((*d)->type(), TypeModeMember, package, (*d)->getMetaData());
                paramDecl.push_back(memberType + " " + memberName);
            }
            out << paramDecl << epar;
            out << sb;
            if(baseClass && allDataMembers.size() != members.size())
            {
                out << nl << "super" << spar;
                vector<string> baseParamNames;
                for(DataMemberList::const_iterator d = baseDataMembers.begin(); d != baseDataMembers.end(); ++d)
                {
                    baseParamNames.push_back(fixKwd((*d)->name()));
                }
                out << baseParamNames << epar << ';';
            }
            for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
            {
                string paramName = fixKwd((*d)->name());
                if((*d)->optional())
                {
                    string capName = paramName;
                    capName[0] = toupper(static_cast<unsigned char>(capName[0]));
                    out << nl << "set" << capName << '(' << paramName << ");";
                }
                else
                {
                    out << nl << "this." << paramName << " = " << paramName << ';';
                }
            }
            out << eb;
        }
    }

    //
    // Default factory for non-abstract classes.
    //
    if(!p->isInterface() && p->allOperations().size() == 0 && !p->isLocal())
    {
        out << sp;
        out << nl << "private static class __F implements Ice.ObjectFactory";
        out << sb;
        out << nl << "public Ice.Object create(String type)";
        out << sb;
        out << nl << "assert(type.equals(ice_staticId()));";
        out << nl << "return new " << fixKwd(name) << "();";
        out << eb;
        out << sp << nl << "public void destroy()";
        out << sb;
        out << eb;
        out << eb;
        out << nl << "private static Ice.ObjectFactory _factory = new __F();";
        out << sp;
        out << nl << "public static Ice.ObjectFactory" << nl << "ice_factory()";
        out << sb;
        out << nl << "return _factory;";
        out << eb;
    }

    //
    // Marshalling & dispatch support.
    //
    if(!p->isInterface() && !p->isLocal())
    {
        writeDispatchAndMarshalling(out, p, _stream);
    }

    return true;
}

void
Slice::Gen::TypesVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    Output& out = output();

    ClassList bases = p->bases();
    ClassDefPtr baseClass;
    if(!bases.empty() && !bases.front()->isInterface())
    {
        baseClass = bases.front();
    }

    string name = fixKwd(p->name());

    if(!p->isInterface())
    {
	out << sp << nl << "public " << name << nl << "clone()";
	out << sb;

	if(p->isLocal() && !baseClass)
	{
	    out << nl << name << " c = null;";
	    out << nl << "try";
	    out << sb;
	    out << nl << "c = (" << name << ")super.clone();";
	    out << eb;
	    out << nl << "catch(CloneNotSupportedException ex)";
	    out << sb;
	    out << nl << "assert false; // impossible";
	    out << eb;
	    out << nl << "return c;";

	}
	else
	{
	    out << nl << "return (" << name << ")super.clone();";
	}
	out << eb;
    }

    if(p->isInterface() && !p->isLocal())
    {
        out << sp << nl << "public static final String ice_staticId = \"" << p->scoped() << "\";";
    }

    out << sp << nl << "public static final long serialVersionUID = ";
    string serialVersionUID;
    if(p->findMetaData("java:serialVersionUID", serialVersionUID))
    {
        string::size_type pos = serialVersionUID.rfind(":") + 1;
        if(pos == string::npos)
        {
            ostringstream os;
            os << "ignoring invalid serialVersionUID for class `" << p->scoped() << "'; generating default value";
            emitWarning("", "", os.str());
            out << computeSerialVersionUUID(p);
        }
        else
        {
            Int64 v = 0;
            serialVersionUID = serialVersionUID.substr(pos);
            if(serialVersionUID != "0")
            {
                if(!stringToInt64(serialVersionUID, v)) // conversion error
                {
                    ostringstream os;
                    os << "ignoring invalid serialVersionUID for class `" << p->scoped()
                       << "'; generating default value";
                    emitWarning("", "", os.str());
                    out << computeSerialVersionUUID(p);
                }
            }
            out << v;
        }
    }
    else
    {
        out << computeSerialVersionUUID(p);
    }
    out << "L;";
    out << eb;
    close();
}

bool
Slice::Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string name = fixKwd(p->name());
    string scoped = p->scoped();
    ExceptionPtr base = p->base();
    string package = getPackage(p);
    string absolute = getAbsolute(p);
    DataMemberList members = p->dataMembers();
    DataMemberList allDataMembers = p->allDataMembers();

    open(absolute, p->file());

    Output& out = output();

    out << sp;

    writeDocComment(out, p, getDeprecateReason(p, 0, "type"));

    out << nl << "public class " << name << " extends ";

    if(!base)
    {
        if(p->isLocal())
        {
            out << "Ice.LocalException";
        }
        else
        {
            out << "Ice.UserException";
        }
    }
    else
    {
        out << getAbsolute(base, package);
    }
    out << sb;

    //
    // Default constructor.
    //
    out << sp;
    out << nl << "public " << name << "()";
    out << sb;
    if(base)
    {
        out << nl << "super();";
    }
    writeDataMemberInitializers(out, members, package);
    out << eb;

    out << sp;
    out << nl << "public " << name << "(Throwable __cause)";
    out << sb;
    out << nl << "super(__cause);";
    writeDataMemberInitializers(out, members, package);
    out << eb;

    bool hasOptionalMembers = false;
    bool hasRequiredMembers = false;
    for(DataMemberList::const_iterator d = allDataMembers.begin(); d != allDataMembers.end(); ++d)
    {
        if((*d)->optional())
        {
            hasOptionalMembers = true;
        }
        else
        {
            hasRequiredMembers = true;
        }
    }

    if(!allDataMembers.empty())
    {
        DataMemberList baseDataMembers;
        if(base)
        {
            baseDataMembers = base->allDataMembers();
        }

        //
        // A method cannot have more than 255 parameters (including the implicit "this" argument).
        //
        if(allDataMembers.size() < 255)
        {
            if(hasRequiredMembers && hasOptionalMembers)
            {
                bool hasBaseRequired = false;
                for(DataMemberList::const_iterator d = baseDataMembers.begin(); d != baseDataMembers.end(); ++d)
                {
                    if(!(*d)->optional())
                    {
                        hasBaseRequired = true;
                        break;
                    }
                }

                DataMemberList optionalMembers = p->orderedOptionalDataMembers();

                //
                // Generate a constructor accepting parameters for just the required members.
                //
                out << sp << nl << "public " << name << spar;
                vector<string> paramDecl;
                for(DataMemberList::const_iterator d = allDataMembers.begin(); d != allDataMembers.end(); ++d)
                {
                    if(!(*d)->optional())
                    {
                        string memberName = fixKwd((*d)->name());
                        string memberType = typeToString((*d)->type(), TypeModeMember, package, (*d)->getMetaData());
                        paramDecl.push_back(memberType + " " + memberName);
                    }
                }
                out << paramDecl << epar;
                out << sb;
                if(!baseDataMembers.empty())
                {
                    if(hasBaseRequired)
                    {
                        out << nl << "super" << spar;
                        vector<string> baseParamNames;
                        for(DataMemberList::const_iterator d = baseDataMembers.begin(); d != baseDataMembers.end(); ++d)
                        {
                            if(!(*d)->optional())
                            {
                                baseParamNames.push_back(fixKwd((*d)->name()));
                            }
                        }
                        out << baseParamNames << epar << ';';
                    }
                }

                for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
                {
                    if(!(*d)->optional())
                    {
                        string paramName = fixKwd((*d)->name());
                        out << nl << "this." << paramName << " = " << paramName << ';';
                    }
                }
                writeDataMemberInitializers(out, optionalMembers, package);
                out << eb;

                //
                // Create constructor that takes all data members plus a Throwable.
                //
                if(allDataMembers.size() < 254)
                {
                    paramDecl.push_back("Throwable __cause");
                    out << sp << nl << "public " << name << spar;
                    out << paramDecl << epar;
                    out << sb;
                    if(hasBaseRequired)
                    {
                        out << nl << "super" << spar;
                        vector<string> baseParamNames;
                        for(DataMemberList::const_iterator d = baseDataMembers.begin(); d != baseDataMembers.end(); ++d)
                        {
                            if(!(*d)->optional())
                            {
                                baseParamNames.push_back(fixKwd((*d)->name()));
                            }
                        }
                        baseParamNames.push_back("__cause");
                        out << baseParamNames << epar << ';';
                    }
                    else
                    {
                        out << nl << "super(__cause);";
                    }
                    for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
                    {
                        if(!(*d)->optional())
                        {
                            string paramName = fixKwd((*d)->name());
                            out << nl << "this." << paramName << " = " << paramName << ';';
                        }
                    }
                    writeDataMemberInitializers(out, optionalMembers, package);
                    out << eb;
                }
            }

            out << sp << nl << "public " << name << spar;
            vector<string> paramDecl;
            for(DataMemberList::const_iterator d = allDataMembers.begin(); d != allDataMembers.end(); ++d)
            {
                string memberName = fixKwd((*d)->name());
                string memberType = typeToString((*d)->type(), TypeModeMember, package, (*d)->getMetaData());
                paramDecl.push_back(memberType + " " + memberName);
            }
            out << paramDecl << epar;
            out << sb;
            if(base && allDataMembers.size() != members.size())
            {
                out << nl << "super" << spar;
                vector<string> baseParamNames;
                DataMemberList baseDataMembers = base->allDataMembers();
                for(DataMemberList::const_iterator d = baseDataMembers.begin(); d != baseDataMembers.end(); ++d)
                {
                    baseParamNames.push_back(fixKwd((*d)->name()));
                }

                out << baseParamNames << epar << ';';
            }
            for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
            {
                string paramName = fixKwd((*d)->name());
                if((*d)->optional())
                {
                    string capName = paramName;
                    capName[0] = toupper(static_cast<unsigned char>(capName[0]));
                    out << nl << "set" << capName << '(' << paramName << ");";
                }
                else
                {
                    out << nl << "this." << paramName << " = " << paramName << ';';
                }
            }
            out << eb;

            //
            // Create constructor that takes all data members plus a Throwable
            //
            if(allDataMembers.size() < 254)
            {
                paramDecl.push_back("Throwable __cause");
                out << sp << nl << "public " << name << spar;
                out << paramDecl << epar;
                out << sb;
                if(!base)
                {
                    out << nl << "super(__cause);";
                }
                else
                {
                    out << nl << "super" << spar;
                    vector<string> baseParamNames;
                    DataMemberList baseDataMembers = base->allDataMembers();
                    for(DataMemberList::const_iterator d = baseDataMembers.begin(); d != baseDataMembers.end(); ++d)
                    {
                        baseParamNames.push_back(fixKwd((*d)->name()));
                    }
                    baseParamNames.push_back("__cause");
                    out << baseParamNames << epar << ';';
                }
                for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
                {
                    string paramName = fixKwd((*d)->name());
                    if((*d)->optional())
                    {
                        string capName = paramName;
                        capName[0] = toupper(static_cast<unsigned char>(capName[0]));
                        out << nl << "set" << capName << '(' << paramName << ");";
                    }
                    else
                    {
                        out << nl << "this." << paramName << " = " << paramName << ';';
                    }
                }
                out << eb;
            }
        }
    }

    out << sp << nl << "public String" << nl << "ice_name()";
    out << sb;
    out << nl << "return \"" << scoped.substr(2) << "\";";
    out << eb;

    return true;
}

void
Slice::Gen::TypesVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    Output& out = output();

    if(!p->isLocal())
    {
        string name = fixKwd(p->name());
        string scoped = p->scoped();
        string package = getPackage(p);
        ExceptionPtr base = p->base();
        bool basePreserved = p->inheritsMetaData("preserve-slice");
        bool preserved = p->hasMetaData("preserve-slice");

        DataMemberList members = p->dataMembers();
        DataMemberList optionalMembers = p->orderedOptionalDataMembers();
        int iter;

        if(preserved && !basePreserved)
        {

            out << sp << nl << "public void" << nl << "__write(IceInternal.BasicStream __os)";
            out << sb;
            out << nl << "__os.startWriteException(__slicedData);";
            out << nl << "__writeImpl(__os);";
            out << nl << "__os.endWriteException();";
            out << eb;

            out << sp << nl << "public void" << nl << "__read(IceInternal.BasicStream __is)";
            out << sb;
            out << nl << "__is.startReadException();";
            out << nl << "__readImpl(__is);";
            out << nl << "__slicedData = __is.endReadException(true);";
            out << eb;

            if(_stream)
            {
                out << sp << nl << "public void" << nl << "__write(Ice.OutputStream __outS)";
                out << sb;
                out << nl << "__outS.startException(__slicedData);";
                out << nl << "__writeImpl(__outS);";
                out << nl << "__outS.endException();";
                out << eb;

                out << sp << nl << "public void" << nl << "__read(Ice.InputStream __inS)";
                out << sb;
                out << nl << "__inS.startException();";
                out << nl << "__readImpl(__inS);";
                out << nl << "__slicedData = __inS.endException(true);";
                out << eb;
            }
        }

        out << sp << nl << "protected void" << nl << "__writeImpl(IceInternal.BasicStream __os)";
        out << sb;
        out << nl << "__os.startWriteSlice(\"" << scoped << "\", -1, " << (!base ? "true" : "false") << ");";
        iter = 0;
        for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
        {
            if(!(*d)->optional())
            {
                writeMarshalDataMember(out, package, *d, iter);
            }
        }
        for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
        {
            writeMarshalDataMember(out, package, *d, iter);
        }
        out << nl << "__os.endWriteSlice();";
        if(base)
        {
            out << nl << "super.__writeImpl(__os);";
        }
        out << eb;

        DataMemberList classMembers = p->classDataMembers();
        DataMemberList allClassMembers = p->allClassDataMembers();

        if(classMembers.size() != 0)
        {
            writePatcher(out, package, classMembers, optionalMembers, _stream);
        }
        out << sp << nl << "protected void" << nl << "__readImpl(IceInternal.BasicStream __is)";
        out << sb;
        out << nl << "__is.startReadSlice();";
        iter = 0;
        int patchIter = 0;
        const bool needCustomPatcher = classMembers.size() > 1;
        for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
        {
            if(!(*d)->optional())
            {
                writeUnmarshalDataMember(out, package, *d, iter, needCustomPatcher, patchIter);
            }
        }
        for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
        {
            writeUnmarshalDataMember(out, package, *d, iter, needCustomPatcher, patchIter);
        }
        out << nl << "__is.endReadSlice();";
        if(base)
        {
            out << nl << "super.__readImpl(__is);";
        }
        out << eb;

        if(_stream)
        {
            out << sp << nl << "protected void" << nl << "__writeImpl(Ice.OutputStream __outS)";
            out << sb;
            out << nl << "__outS.startSlice(\"" << scoped << "\", -1, " << (!base ? "true" : "false") << ");";
            iter = 0;
            for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
            {
                if(!(*d)->optional())
                {
                    writeStreamMarshalDataMember(out, package, *d, iter);
                }
            }
            for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
            {
                writeStreamMarshalDataMember(out, package, *d, iter);
            }
            out << nl << "__outS.endSlice();";
            if(base)
            {
                out << nl << "super.__writeImpl(__outS);";
            }
            out << eb;

            out << sp << nl << "protected void" << nl << "__readImpl(Ice.InputStream __inS)";
            out << sb;
            out << nl << "__inS.startSlice();";
            iter = 0;
            patchIter = 0;
            for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
            {
                if(!(*d)->optional())
                {
                    writeStreamUnmarshalDataMember(out, package, *d, iter, needCustomPatcher, patchIter);
                }
            }
            for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
            {
                writeStreamUnmarshalDataMember(out, package, *d, iter, needCustomPatcher, patchIter);
            }
            out << nl << "__inS.endSlice();";
            if(base)
            {
                out << nl << "super.__readImpl(__inS);";
            }
            out << eb;
        }

        if(p->usesClasses(false))
        {
	    if(!base || (base && !base->usesClasses(false)))
            {
                out << sp << nl << "public boolean" << nl << "__usesClasses()";
                out << sb;
                out << nl << "return true;";
                out << eb;
            }
        }

        if(preserved && !basePreserved)
        {
            out << sp << nl << "protected Ice.SlicedData __slicedData;";
        }
    }

    out << sp << nl << "public static final long serialVersionUID = ";
    string serialVersionUID;
    if(p->findMetaData("java:serialVersionUID", serialVersionUID))
    {
        string::size_type pos = serialVersionUID.rfind(":") + 1;
        if(pos == string::npos)
        {
            ostringstream os;
            os << "ignoring invalid serialVersionUID for exception `" << p->scoped() << "'; generating default value";
            emitWarning("", "", os.str());
            out << computeSerialVersionUUID(p);
        }
        else
        {
            Int64 v = 0;
            serialVersionUID = serialVersionUID.substr(pos);
            if(serialVersionUID != "0")
            {
                if(!stringToInt64(serialVersionUID, v)) // conversion error
                {
                    ostringstream os;
                    os << "ignoring invalid serialVersionUID for exception `" << p->scoped()
                       << "'; generating default value";
                    emitWarning("", "", os.str());
                    out << computeSerialVersionUUID(p);
                }
            }
            out << v;
        }
    }
    else
    {
        out << computeSerialVersionUUID(p);
    }
    out << "L;";

    out << eb;
    close();
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    string name = fixKwd(p->name());
    string absolute = getAbsolute(p);

    open(absolute, p->file());

    Output& out = output();

    out << sp;

    writeDocComment(out, p, getDeprecateReason(p, 0, "type"));

    out << nl << "public class " << name << " implements java.lang.Cloneable";
    if(!p->isLocal())
    {
        out << ", java.io.Serializable";
    }
    out << sb;

    return true;
}

void
Slice::Gen::TypesVisitor::visitStructEnd(const StructPtr& p)
{
    string package = getPackage(p);

    Output& out = output();

    DataMemberList members = p->dataMembers();
    int iter;

    string name = fixKwd(p->name());
    string typeS = typeToString(p, TypeModeIn, package);

    out << sp << nl << "public " << name << "()";
    out << sb;
    writeDataMemberInitializers(out, members, package);
    out << eb;

    //
    // A method cannot have more than 255 parameters (including the implicit "this" argument).
    //
    if(members.size() < 255)
    {
        vector<string> paramDecl;
        vector<string> paramNames;
        for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
        {
            string memberName = fixKwd((*d)->name());
            string memberType = typeToString((*d)->type(), TypeModeMember, package, (*d)->getMetaData());
            paramDecl.push_back(memberType + " " + memberName);
            paramNames.push_back(memberName);
        }

        out << sp << nl << "public " << name << spar << paramDecl << epar;
        out << sb;
        for(vector<string>::const_iterator i = paramNames.begin(); i != paramNames.end(); ++i)
        {
            out << nl << "this." << *i << " = " << *i << ';';
        }
        out << eb;
    }

    out << sp << nl << "public boolean" << nl << "equals(java.lang.Object rhs)";
    out << sb;
    out << nl << "if(this == rhs)";
    out << sb;
    out << nl << "return true;";
    out << eb;
    out << nl << typeS << " _r = null;";
    out << nl << "if(rhs instanceof " << typeS << ")";
    out << sb;
    out << nl << "_r = (" << typeS << ")rhs;";
    out << eb;
    out << sp << nl << "if(_r != null)";
    out << sb;
    for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
    {
        string memberName = fixKwd((*d)->name());
        BuiltinPtr b = BuiltinPtr::dynamicCast((*d)->type());
        if(b)
        {
            switch(b->kind())
            {
                case Builtin::KindByte:
                case Builtin::KindBool:
                case Builtin::KindShort:
                case Builtin::KindInt:
                case Builtin::KindLong:
                case Builtin::KindFloat:
                case Builtin::KindDouble:
                {
                    out << nl << "if(" << memberName << " != _r." << memberName << ')';
                    out << sb;
                    out << nl << "return false;";
                    out << eb;
                    break;
                }

                case Builtin::KindString:
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                {
                    out << nl << "if(" << memberName << " != _r." << memberName << ')';
                    out << sb;
                    out << nl << "if(" << memberName << " == null || _r." << memberName << " == null || !"
                        << memberName << ".equals(_r." << memberName << "))";
                    out << sb;
                    out << nl << "return false;";
                    out << eb;
                    out << eb;
                    break;
                }
            }
        }
        else
        {
            //
            // We treat sequences differently because the native equals() method for
            // a Java array does not perform a deep comparison. If the mapped type
            // is not overridden via metadata, we use the helper method
            // java.util.Arrays.equals() to compare native arrays.
            //
            // For all other types, we can use the native equals() method.
            //
            SequencePtr seq = SequencePtr::dynamicCast((*d)->type());
            if(seq)
            {
                if(hasTypeMetaData(seq, (*d)->getMetaData()))
                {
                    out << nl << "if(" << memberName << " != _r." << memberName << ')';
                    out << sb;
                    out << nl << "if(" << memberName << " == null || _r." << memberName << " == null || !"
                        << memberName << ".equals(_r." << memberName << "))";
                    out << sb;
                    out << nl << "return false;";
                    out << eb;
                    out << eb;
                }
                else
                {
                    //
                    // Arrays.equals() handles null values.
                    //
                    out << nl << "if(!java.util.Arrays.equals(" << memberName << ", _r." << memberName << "))";
                    out << sb;
                    out << nl << "return false;";
                    out << eb;
                }
            }
            else
            {
                out << nl << "if(" << memberName << " != _r." << memberName << ')';
                out << sb;
                out << nl << "if(" << memberName << " == null || _r." << memberName << " == null || !"
                    << memberName << ".equals(_r." << memberName << "))";
                out << sb;
                out << nl << "return false;";
                out << eb;
                out << eb;
            }
        }
    }
    out << sp << nl << "return true;";
    out << eb;
    out << sp << nl << "return false;";
    out << eb;

    out << sp << nl << "public int" << nl << "hashCode()";
    out << sb;
    out << nl << "int __h = 5381;";
    out << nl << "__h = IceInternal.HashUtil.hashAdd(__h, \"" << p->scoped() << "\");";
    iter = 0;
    for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
    {
        string memberName = fixKwd((*d)->name());
        out << nl << "__h = IceInternal.HashUtil.hashAdd(__h, " << memberName << ");";
    }
    out << nl << "return __h;";
    out << eb;

    out << sp << nl << "public " << name << nl << "clone()";
    out << sb;
    out << nl << name << " c = null;";
    out << nl << "try";
    out << sb;
    out << nl << "c = (" << name << ")super.clone();";
    out << eb;
    out << nl << "catch(CloneNotSupportedException ex)";
    out << sb;
    out << nl << "assert false; // impossible";
    out << eb;
    out << nl << "return c;";
    out << eb;

    if(!p->isLocal())
    {
        out << sp << nl << "public void" << nl << "__write(IceInternal.BasicStream __os)";
        out << sb;
        iter = 0;
        for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
        {
            writeMarshalDataMember(out, package, *d, iter);
        }
        out << eb;

        DataMemberList classMembers = p->classDataMembers();

        if(classMembers.size() != 0)
        {
            writePatcher(out, package, classMembers, DataMemberList(), _stream);
        }

        out << sp << nl << "public void" << nl << "__read(IceInternal.BasicStream __is)";
        out << sb;
        iter = 0;
        int patchIter = 0;
        const bool needCustomPatcher = classMembers.size() > 1;
        for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
        {
            writeUnmarshalDataMember(out, package, *d, iter, needCustomPatcher, patchIter);
        }
        out << eb;

        if(_stream)
        {
            out << sp << nl << "public void" << nl << "ice_write(Ice.OutputStream __outS)";
            out << sb;
            iter = 0;
            for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
            {
                writeStreamMarshalDataMember(out, package, *d, iter);
            }
            out << eb;

            out << sp << nl << "public void" << nl << "ice_read(Ice.InputStream __inS)";
            out << sb;
            iter = 0;
            patchIter = 0;
            for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
            {
                writeStreamUnmarshalDataMember(out, package, *d, iter, needCustomPatcher, patchIter);
            }
            out << eb;
        }

        out << sp << nl << "static public void" << nl << "__write(IceInternal.BasicStream __os, " << name << " __v)";
        out << sb;
        out << nl << "if(__v == null)";
        out << sb;
        out << nl << "__nullMarshalValue.__write(__os);";
        out << eb;
        out << nl << "else";
        out << sb;
        out << nl << "__v.__write(__os);";
        out << eb;
        out << eb;

        out << sp << nl << "static public " << name << nl << "__read(IceInternal.BasicStream __is, " << name << " __v)";
        out << sb;
        out << nl << "if(__v == null)";
        out << sb;
        out << nl << " __v = new " << name << "();";
        out << eb;
        out << nl << "__v.__read(__is);";
        out << nl << "return __v;";
        out << eb;

        if(_stream)
        {
            out << sp << nl << "static public void" << nl << "ice_write(Ice.OutputStream __outS, " << name << " __v)";
            out << sb;
            out << nl << "if(__v == null)";
            out << sb;
            out << nl << "__nullMarshalValue.ice_write(__outS);";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << "__v.ice_write(__outS);";
            out << eb;
            out << eb;

            out << sp << nl << "static public " << name << nl << "ice_read(Ice.InputStream __inS, " << name << " __v)";
            out << sb;
            out << nl << "if(__v == null)";
            out << sb;
            out << nl << " __v = new " << name << "();";
            out << eb;
            out << nl << "__v.ice_read(__inS);";
            out << nl << "return __v;";
            out << eb;
        }

        out << nl << nl << "private static final " << name << " __nullMarshalValue = new " << name << "();";
    }

    out << sp << nl << "public static final long serialVersionUID = ";
    string serialVersionUID;
    if(p->findMetaData("java:serialVersionUID", serialVersionUID))
    {
        string::size_type pos = serialVersionUID.rfind(":") + 1;
        if(pos == string::npos)
        {
            ostringstream os;
            os << "ignoring invalid serialVersionUID for struct `" << p->scoped() << "'; generating default value";
            emitWarning("", "", os.str());
            out << computeSerialVersionUUID(p);
        }
        else
        {
            Int64 v = 0;
            serialVersionUID = serialVersionUID.substr(pos);
            if(serialVersionUID != "0")
            {
                if(!stringToInt64(serialVersionUID, v)) // conversion error
                {
                    ostringstream os;
                    os << "ignoring invalid serialVersionUID for struct `" << p->scoped()
                       << "'; generating default value";
                    emitWarning("", "", os.str());
                    out << computeSerialVersionUUID(p);
                }
            }
            out << v;
        }
    }
    else
    {
        out << computeSerialVersionUUID(p);
    }
    out << "L;";

    out << eb;
    close();
}

void
Slice::Gen::TypesVisitor::visitDataMember(const DataMemberPtr& p)
{
    string name = fixKwd(p->name());
    ContainerPtr container = p->container();
    ContainedPtr contained = ContainedPtr::dynamicCast(container);
    StringList metaData = p->getMetaData();
    TypePtr type = p->type();
    string s = typeToString(type, TypeModeMember, getPackage(contained), metaData);
    Output& out = output();
    const bool optional = p->optional();
    const bool getSet = p->hasMetaData(_getSetMetaData) || contained->hasMetaData(_getSetMetaData);

    out << sp;

    string deprecateReason = getDeprecateReason(p, contained, "member");
    writeDocComment(out, p, deprecateReason);

    //
    // Access visibility for class data members can be controlled by metadata.
    // If none is specified, the default is public.
    //
    if(contained->containedType() == Contained::ContainedTypeClass &&
       (p->hasMetaData("protected") || contained->hasMetaData("protected")))
    {
        out << nl << "protected " << s << ' ' << name << ';';
    }
    else if(optional)
    {
        out << nl << "private " << s << ' ' << name << ';';
    }
    else
    {
        out << nl << "public " << s << ' ' << name << ';';
    }

    if(optional)
    {
        out << nl << "private boolean __has_" << p->name() << ';';
    }

    //
    // Getter/Setter.
    //
    if(getSet || optional)
    {
        string capName = p->name();
        capName[0] = toupper(static_cast<unsigned char>(capName[0]));

        //
        // If container is a class, get all of its operations so that we can check for conflicts.
        //
        OperationList ops;
        string file, line;
        ClassDefPtr cls = ClassDefPtr::dynamicCast(container);
        if(cls)
        {
            ops = cls->allOperations();
            file = p->file();
            line = p->line();
            if(!validateMethod(ops, "get" + capName, 0, file, line) ||
               !validateMethod(ops, "set" + capName, 1, file, line))
            {
                return;
            }
            if(optional &&
               (!validateMethod(ops, "has" + capName, 0, file, line) ||
                !validateMethod(ops, "clear" + capName, 0, file, line) ||
                !validateMethod(ops, "optional" + capName, 0, file, line)))
            {
                return;
            }
        }

        //
        // Getter.
        //
        out << sp;
        writeDocComment(out, p, deprecateReason);
        out << nl << "public " << s
            << nl << "get" << capName << "()";
        out << sb;
        if(optional)
        {
            out << nl << "if(!__has_" << p->name() << ')';
            out << sb;
            out << nl << "throw new java.lang.IllegalStateException(\"" << name << " is not set\");";
            out << eb;
        }
        out << nl << "return " << name << ';';
        out << eb;

        //
        // Setter.
        //
        out << sp;
        writeDocComment(out, p, deprecateReason);
        out << nl << "public void"
            << nl << "set" << capName << '(' << s << " _" << name << ')';
        out << sb;
        if(optional)
        {
            out << nl << "__has_" << p->name() << " = true;";
        }
        out << nl << name << " = _" << name << ';';
        out << eb;

        //
        // Generate hasFoo and clearFoo for optional member.
        //
        if(optional)
        {
            out << sp;
            writeDocComment(out, p, deprecateReason);
            out << nl << "public boolean"
                << nl << "has" << capName << "()";
            out << sb;
            out << nl << "return __has_" << p->name() << ';';
            out << eb;

            out << sp;
            writeDocComment(out, p, deprecateReason);
            out << nl << "public void"
                << nl << "clear" << capName << "()";
            out << sb;
            out << nl << "__has_" << p->name() << " = false;";
            out << eb;

            const string optType = typeToString(type, TypeModeMember, getPackage(contained), metaData, true, true);

            out << sp;
            writeDocComment(out, p, deprecateReason);
            out << nl << "public void"
                << nl << "optional" << capName << '(' << optType << " __v)";
            out << sb;
            out << nl << "if(__v == null || !__v.isSet())";
            out << sb;
            out << nl << "__has_" << p->name() << " = false;";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << "__has_" << p->name() << " = true;";
            out << nl << name << " = __v.get();";
            out << eb;
            out << eb;

            out << sp;
            writeDocComment(out, p, deprecateReason);
            out << nl << "public " << optType
                << nl << "optional" << capName << "()";
            out << sb;
            out << nl << "if(__has_" << p->name() << ')';
            out << sb;
            out << nl << "return new " << optType << '(' << name << ");";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << "return new " << optType << "();";
            out << eb;
            out << eb;
        }

        //
        // Check for bool type.
        //
        BuiltinPtr b = BuiltinPtr::dynamicCast(type);
        if(b && b->kind() == Builtin::KindBool)
        {
            if(cls && !validateMethod(ops, "is" + capName, 0, file, line))
            {
                return;
            }
            out << sp;
            if(!deprecateReason.empty())
            {
                out << nl << "/**";
                out << nl << " * @deprecated " << deprecateReason;
                out << nl << " **/";
            }
            out << nl << "public boolean";
            out << nl << "is" << capName << "()";
            out << sb;
            if(optional)
            {
                out << nl << "if(!__has_" << p->name() << ')';
                out << sb;
                out << nl << "throw new java.lang.IllegalStateException(\"" << name << " is not set\");";
                out << eb;
            }
            out << nl << "return " << name << ';';
            out << eb;
        }

        //
        // Check for unmodified sequence type and emit indexing methods.
        //
        SequencePtr seq = SequencePtr::dynamicCast(type);
        if(seq)
        {
            if(!hasTypeMetaData(seq, metaData))
            {
                if(cls &&
                   (!validateMethod(ops, "get" + capName, 1, file, line) ||
                    !validateMethod(ops, "set" + capName, 2, file, line)))
                {
                    return;
                }

                string elem = typeToString(seq->type(), TypeModeMember, getPackage(contained));

                //
                // Indexed getter.
                //
                out << sp;
                if(!deprecateReason.empty())
                {
                    out << nl << "/**";
                    out << nl << " * @deprecated " << deprecateReason;
                    out << nl << " **/";
                }
                out << nl << "public " << elem;
                out << nl << "get" << capName << "(int _index)";
                out << sb;
                if(optional)
                {
                    out << nl << "if(!__has_" << p->name() << ')';
                    out << sb;
                    out << nl << "throw new java.lang.IllegalStateException(\"" << name << " is not set\");";
                    out << eb;
                }
                out << nl << "return " << name << "[_index];";
                out << eb;

                //
                // Indexed setter.
                //
                out << sp;
                if(!deprecateReason.empty())
                {
                    out << nl << "/**";
                    out << nl << " * @deprecated " << deprecateReason;
                    out << nl << " **/";
                }
                out << nl << "public void";
                out << nl << "set" << capName << "(int _index, " << elem << " _val)";
                out << sb;
                if(optional)
                {
                    out << nl << "if(!__has_" << p->name() << ')';
                    out << sb;
                    out << nl << "throw new java.lang.IllegalStateException(\"" << name << " is not set\");";
                    out << eb;
                }
                out << nl << name << "[_index] = _val;";
                out << eb;
            }
        }
    }
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string name = fixKwd(p->name());
    string absolute = getAbsolute(p);
    EnumeratorList enumerators = p->getEnumerators();

    open(absolute, p->file());

    Output& out = output();

    out << sp;

    writeDocComment(out, p, getDeprecateReason(p, 0, "type"));

    out << nl << "public enum " << name;
    if(!p->isLocal())
    {
        out << " implements java.io.Serializable";
    }
    out << sb;

    for(EnumeratorList::const_iterator en = enumerators.begin(); en != enumerators.end(); ++en)
    {
        if(en != enumerators.begin())
        {
            out << ',';
        }
        out << nl;
        writeDocComment(out, *en, getDeprecateReason(*en, 0, "enumerator"));
        out << nl << fixKwd((*en)->name()) << '(' << (*en)->value() << ')';
    }
    out << ';';

    out << sp << nl << "public int"
        << nl << "value()";
    out << sb;
    out << nl << "return __value;";
    out << eb;

    out << sp << nl << "public static " << name
        << nl << "valueOf(int __v)";
    out << sb;
    out << nl << "switch(__v)";
    out << sb;
    out.dec();
    for(EnumeratorList::const_iterator en = enumerators.begin(); en != enumerators.end(); ++en)
    {
        out << nl << "case " << (*en)->value() << ':';
        out.inc();
        out << nl << "return " << fixKwd((*en)->name()) << ';';
        out.dec();
    }
    out.inc();
    out << eb;
    out << nl << "return null;";
    out << eb;

    out << sp << nl << "private"
        << nl << name << "(int __v)";
    out << sb;
    out << nl << "__value = __v;";
    out << eb;

    if(!p->isLocal())
    {
        out << sp << nl << "public void" << nl << "__write(IceInternal.BasicStream __os)";
        out << sb;
        out << nl << "__os.writeEnum(value(), " << p->maxValue() << ");";
        out << eb;

        out << sp << nl << "public static void" << nl << "__write(IceInternal.BasicStream __os, " << name << " __v)";
        out << sb;
        out << nl << "if(__v == null)";
        out << sb;
        string firstEnum = fixKwd(enumerators.front()->name());
        out << nl << "__os.writeEnum(" << absolute << '.' << firstEnum << ".value(), " << p->maxValue() << ");";
        out << eb;
        out << nl << "else";
        out << sb;
        out << nl << "__os.writeEnum(__v.value(), " << p->maxValue() << ");";
        out << eb;
        out << eb;

        out << sp << nl << "public static " << name << nl << "__read(IceInternal.BasicStream __is)";
        out << sb;
        out << nl << "int __v = __is.readEnum(" << p->maxValue() << ");";
        out << nl << "return __validate(__v);";
        out << eb;

        if(_stream)
        {
            out << sp << nl << "public void" << nl << "ice_write(Ice.OutputStream __outS)";
            out << sb;
            out << nl << "__outS.writeEnum(value(), " << p->maxValue() << ");";
            out << eb;

            out << sp << nl << "public static void" << nl << "ice_write(Ice.OutputStream __outS, " << name << " __v)";
            out << sb;
            out << nl << "if(__v == null)";
            out << sb;
            out << nl << "__outS.writeEnum(" << absolute << '.' << firstEnum << ".value(), " << p->maxValue() << ");";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << "__outS.writeEnum(__v.value(), " << p->maxValue() << ");";
            out << eb;
            out << eb;

            out << sp << nl << "public static " << name << nl << "ice_read(Ice.InputStream __inS)";
            out << sb;
            out << nl << "int __v = __inS.readEnum(" << p->maxValue() << ");";
            out << nl << "return __validate(__v);";
            out << eb;
        }

        out << sp << nl << "private static " << name
            << nl << "__validate(int __v)";
        out << sb;
        out << nl << "final " << name << " __e = valueOf(__v);";
        out << nl << "if(__e == null)";
        out << sb;
        out << nl << "throw new Ice.MarshalException(\"enumerator value \" + __v + \" is out of range\");";
        out << eb;
        out << nl << "return __e;";
        out << eb;
    }

    out << sp << nl << "private final int __value;";

    out << eb;
    close();
}

void
Slice::Gen::TypesVisitor::visitConst(const ConstPtr& p)
{
    string name = fixKwd(p->name());
    string package = getPackage(p);
    string absolute = getAbsolute(p);
    TypePtr type = p->type();

    open(absolute, p->file());

    Output& out = output();

    out << sp;
    writeDocComment(out, p, getDeprecateReason(p, 0, "constant"));
    out << nl << "public interface " << name;
    out << sb;
    out << nl << typeToString(type, TypeModeIn, package) << " value = ";
    writeConstantValue(out, type, p->valueType(), p->value(), package);
    out << ';' << eb;
    close();
}

bool
Slice::Gen::TypesVisitor::validateMethod(const OperationList& ops, const std::string& name, int numArgs,
                                         const string& file, const string& line)
{
    for(OperationList::const_iterator i = ops.begin(); i != ops.end(); ++i)
    {
        if((*i)->name() == name)
        {
            int numParams = static_cast<int>((*i)->parameters().size());
            if(numArgs >= numParams && numArgs - numParams <= 1)
            {
                ostringstream ostr;
                ostr << "operation `" << name << "' conflicts with method for data member";
                emitError(file, line, ostr.str());
                return false;
            }
            break;
        }
    }
    return true;
}

Slice::Gen::CompactIdVisitor::CompactIdVisitor(const string& dir) :
    JavaVisitor(dir)
{
}

bool
Slice::Gen::CompactIdVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string prefix = getPackagePrefix(p);
    if(!prefix.empty())
    {
        prefix = prefix + ".";
    }
    if(p->compactId() >= 0)
    {
        ostringstream os;
        os << prefix << "IceCompactId.TypeId_" << p->compactId();
        open(os.str(), p->file());

        Output& out = output();
        out << sp << nl << "public class TypeId_" << p->compactId();
        out << sb;
        out << nl << "public final static String typeId = \"" << p->scoped() << "\";";
        out << eb;

        close();
    }
    return false;
}

Slice::Gen::HolderVisitor::HolderVisitor(const string& dir) :
    JavaVisitor(dir)
{
}

bool
Slice::Gen::HolderVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    ClassDeclPtr decl = p->declaration();
    writeHolder(decl);

    if(!p->isLocal())
    {
        string name = p->name();
        string absolute = getAbsolute(p, "", "", "PrxHolder");

        open(absolute, p->file());
        Output& out = output();

        out << sp << nl << "public final class " << name << "PrxHolder";
        out << sb;
        out << sp << nl << "public" << nl << name << "PrxHolder()";
        out << sb;
        out << eb;
        out << sp << nl << "public" << nl << name << "PrxHolder(" << name << "Prx value)";
        out << sb;
        out << nl << "this.value = value;";
        out << eb;
        out << sp << nl << "public " << name << "Prx value;";
        out << eb;
        close();
    }

    return false;
}

bool
Slice::Gen::HolderVisitor::visitStructStart(const StructPtr& p)
{
    writeHolder(p);
    return false;
}

void
Slice::Gen::HolderVisitor::visitSequence(const SequencePtr& p)
{
    if(sequenceHasHolder(p))
    {
        writeHolder(p);
    }
}

void
Slice::Gen::HolderVisitor::visitDictionary(const DictionaryPtr& p)
{
    writeHolder(p);
}

void
Slice::Gen::HolderVisitor::visitEnum(const EnumPtr& p)
{
    writeHolder(p);
}

void
Slice::Gen::HolderVisitor::writeHolder(const TypePtr& p)
{
    ContainedPtr contained = ContainedPtr::dynamicCast(p);
    assert(contained);
    string name = contained->name();
    string absolute = getAbsolute(contained, "", "", "Holder");


    string file;
    if(p->definitionContext())
    {
        file = p->definitionContext()->filename();
    }

    open(absolute, file);
    Output& out = output();

    string typeS = typeToString(p, TypeModeIn, getPackage(contained));
    out << sp << nl << "public final class " << name << "Holder";
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p);
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(p);
    if(!p->isLocal() && ((builtin && builtin->kind() == Builtin::KindObject) || cl))
    {
        out << " extends Ice.ObjectHolderBase<" << typeS << ">";
    }
    else {
        out << " extends Ice.Holder<" << typeS << ">";
    }
    out << sb;
    if(!p->isLocal() && ((builtin && builtin->kind() == Builtin::KindObject) || cl))
    {
        out << sp << nl << "public" << nl << name << "Holder()";
        out << sb;
        out << eb;
        out << sp << nl << "public" << nl << name << "Holder(" << typeS << " value)";
        out << sb;
        out << nl << "this.value = value;";
        out << eb;

        out << sp << nl << "public void";
        out << nl << "patch(Ice.Object v)";
        out << sb;
        out << nl << "if(v == null || v instanceof " << typeS << ")";
        out << sb;
        out << nl << "value = (" << typeS << ")v;";
        out << eb;
        out << nl << "else";
        out << sb;
        out << nl << "IceInternal.Ex.throwUOE(type(), v);";
        out << eb;
        out << eb;
        out << sp << nl << "public String" << nl << "type()";
        out << sb;
        if(cl)
        {
            if(cl->isInterface())
            {
                out << nl << "return _" << cl->name() << "Disp.ice_staticId();";
            }
            else
            {
                out << nl << "return " << typeS << ".ice_staticId();";
            }
        }
        else
        {
            out << nl << "return \"" << p->typeId() << "\";";
        }
        out << eb;
    }
    else
    {
        out << sp << nl << "public" << nl << name << "Holder()";
        out << sb;
        out << eb;
        out << sp << nl << "public" << nl << name << "Holder(" << typeS << " value)";
        out << sb;
        out << nl << "super(value);";
        out << eb;
    }
    out << eb;
    close();
}

Slice::Gen::HelperVisitor::HelperVisitor(const string& dir, bool stream) :
    JavaVisitor(dir), _stream(stream)
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
    string name = p->name();
    string scoped = p->scoped();
    string package = getPackage(p);
    string absolute = getAbsolute(p);

    open(getAbsolute(p, "", "", "PrxHelper"), p->file());
    Output& out = output();

    //
    // A proxy helper class serves two purposes: it implements the
    // proxy interface, and provides static helper methods for use
    // by applications (e.g., checkedCast, etc.)
    //
    out << sp;
    writeDocComment(out, getDeprecateReason(p, 0, p->isInterface() ? "interface" : "class"),
                    "Provides type-specific helper functions.");
    out << nl << "public final class " << name << "PrxHelper extends Ice.ObjectPrxHelperBase implements " << name
        << "Prx";

    out << sb;

    string contextParam = "java.util.Map<String, String> __ctx";
    string explicitContextParam = "boolean __explicitCtx";

    OperationList ops = p->allOperations();
    for(OperationList::iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        const ContainerPtr container = op->container();
        const ClassDefPtr cl = ClassDefPtr::dynamicCast(container);

        out << sp;
        out << nl << "private static final String __" << op->name() << "_name = \"" << op->name() << "\";";

        //
        // Use the optional mapping by default.
        //
        writeOperation(p, package, op, true);

        //
        // If the operation actually sends any optionals, we generated overloaded methods
        // that use the required mapping.
        //
        if(op->sendsOptionals())
        {
            writeOperation(p, package, op, false);
        }

        //
        // End method
        //
        vector<string> outParams = getInOutParams(op, package, OutParam, true, true);
        int iter = 0;
        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();
        const TypePtr ret = op->returnType();
        const string retS = typeToString(ret, TypeModeReturn, package, op->getMetaData(), true, op->returnIsOptional());

        out << sp;
        out << nl << "public " << retS << " end_" << op->name() << spar << outParams << "Ice.AsyncResult __iresult"
            << epar;
        writeThrowsClause(package, throws);
        out << sb;
        if(op->returnsData())
        {
            out << nl << "IceInternal.OutgoingAsync __result = IceInternal.OutgoingAsync.check(__iresult, this, __"
                << op->name() << "_name);";
            out << nl << "try";
            out << sb;

            out << nl << "if(!__result.__wait())";
            out << sb;
            out << nl << "try";
            out << sb;
            out << nl << "__result.throwUserException();";
            out << eb;
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
            for(ExceptionList::const_iterator eli = throws.begin(); eli != throws.end(); ++eli)
            {
                out << nl << "catch(" << getAbsolute(*eli, package) << " __ex)";
                out << sb;
                out << nl << "throw __ex;";
                out << eb;
            }
            out << nl << "catch(Ice.UserException __ex)";
            out << sb;
            out << nl << "throw new Ice.UnknownUserException(__ex.ice_name(), __ex);";
            out << eb;
            out << eb;

            if(ret || !outParams.empty())
            {
                out << nl << "IceInternal.BasicStream __is = __result.startReadParams();";
                const ParamDeclList paramList = op->parameters();
                ParamDeclList pl;
                for(ParamDeclList::const_iterator pli = paramList.begin(); pli != paramList.end(); ++pli)
                {
                    if((*pli)->isOutParam())
                    {
                        pl.push_back(*pli);
                    }
                }
                writeMarshalUnmarshalParams(out, package, pl, op, iter, false, true);
                if(op->returnsClasses(false))
                {
                    out << nl << "__is.readPendingObjects();";
                }
                out << nl << "__result.endReadParams();";
            }
            else
            {
                out << nl << "__result.readEmptyParams();";
            }

            if(ret)
            {
                BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
                if(!op->returnIsOptional() &&
                   ((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(ret)))
                {
                    out << nl << "return __ret.value;";
                }
                else
                {
                    out << nl << "return __ret;";
                }
            }

            out << eb;
            out << nl << "finally";
            out << sb;
            out << nl << "if(__result != null)";
            out << sb;
            out << nl << "__result.cacheMessageBuffers();";
            out << eb;
            out << eb;
        }
        else
        {
            out << nl << "__end(__iresult, __" << op->name() << "_name);";
        }
        out << eb;

        //
        // The async callbacks implementation of __completed method delegate to the static
        // __<op-name>_completed method implemented bellow.
        //
        if(op->returnsData())
        {
            const ParamDeclList paramList = op->parameters();
            ParamDeclList outParams;
            for(ParamDeclList::const_iterator pli = paramList.begin(); pli != paramList.end(); ++pli)
            {
                if((*pli)->isOutParam())
                {
                    outParams.push_back(*pli);
                }
            }

            out << sp << nl << "static public void __" << op->name() << "_completed("
                << getAsyncCallbackInterface(op, package) << " __cb, Ice.AsyncResult __result)";
            out << sb;
            out << nl << getAbsolute(cl, "", "", "Prx") << " __proxy = ("
                << getAbsolute(cl, "", "", "Prx") << ")__result.getProxy();";

            TypePtr ret = op->returnType();
            if(ret)
            {
                out << nl << typeToString(ret, TypeModeIn, package, op->getMetaData(), true,
                                            op->returnIsOptional())
                    << " __ret = " << (op->returnIsOptional() ? "null" : initValue(ret)) << ';';
            }
            for(ParamDeclList::const_iterator pli = outParams.begin(); pli != outParams.end(); ++pli)
            {
                string ts = typeToString((*pli)->type(), TypeModeOut, package, (*pli)->getMetaData(), true,
                                            (*pli)->optional());
                out << nl << ts << ' ' << fixKwd((*pli)->name()) << " = new " << ts << "();";
            }
            out << nl << "try";
            out << sb;
            out << nl;
            if(op->returnType())
            {
                out << "__ret = ";
            }
            out << "__proxy.end_" << op->name() << spar << getInOutArgs(op, OutParam) << "__result" << epar
                << ';';

            out << eb;
            if(!throws.empty())
            {
                out << nl << "catch(Ice.UserException __ex)";
                out << sb;
                out << nl << "__cb.exception(__ex);";
                out << nl << "return;";
                out << eb;
            }
            out << nl << "catch(Ice.LocalException __ex)";
            out << sb;
            out << nl << "__cb.exception(__ex);";
            out << nl << "return;";
            out << eb;
            out << nl << "catch(Ice.SystemException __ex)";
            out << sb;
            out << nl << "__cb.exception(__ex);";
            out << nl << "return;";
            out << eb;

            out << nl << "__cb.response" << spar;
            if(op->returnType())
            {
                out << "__ret";
            }
            for(ParamDeclList::const_iterator pli = outParams.begin(); pli != outParams.end(); ++pli)
            {
                if((*pli)->optional())
                {
                    out << fixKwd((*pli)->name());
                }
                else
                {
                    out << fixKwd((*pli)->name()) + ".value";
                }
            }
            out << epar << ';';

            out << eb;
        }
    }

    out << sp;
    writeDocComment(out, "",
                    "Contacts the remote server to verify that the object implements this type.\n"
                    "Raises a local exception if a communication error occurs.\n"
                    "@param __obj The untyped proxy.\n"
                    "@return A proxy for this type, or null if the object does not support this type.");
    out << nl << "public static " << name << "Prx checkedCast(Ice.ObjectPrx __obj)";
    out << sb;
    out << nl << "return checkedCastImpl(__obj, ice_staticId(), " << name << "Prx.class, "
        << name << "PrxHelper.class);";
    out << eb;

    out << sp;
    writeDocComment(out, "",
                    "Contacts the remote server to verify that the object implements this type.\n"
                    "Raises a local exception if a communication error occurs.\n"
                    "@param __obj The untyped proxy.\n"
                    "@param __ctx The Context map to send with the invocation.\n"
                    "@return A proxy for this type, or null if the object does not support this type.");
    out << nl << "public static " << name << "Prx checkedCast(Ice.ObjectPrx __obj, " << contextParam << ')';
    out << sb;
    out << nl << "return checkedCastImpl(__obj, __ctx, ice_staticId(), " << name
        << "Prx.class, " << name << "PrxHelper.class);";
    out << eb;

    out << sp;
    writeDocComment(out, "",
                    "Contacts the remote server to verify that a facet of the object implements this type.\n"
                    "Raises a local exception if a communication error occurs.\n"
                    "@param __obj The untyped proxy.\n"
                    "@param __facet The name of the desired facet.\n"
                    "@return A proxy for this type, or null if the object does not support this type.");
    out << nl << "public static " << name << "Prx checkedCast(Ice.ObjectPrx __obj, String __facet)";
    out << sb;
    out << nl << "return checkedCastImpl(__obj, __facet, ice_staticId(), " << name
        << "Prx.class, " << name << "PrxHelper.class);";
    out << eb;

    out << sp;
    writeDocComment(out, "",
                    "Contacts the remote server to verify that a facet of the object implements this type.\n"
                    "Raises a local exception if a communication error occurs.\n"
                    "@param __obj The untyped proxy.\n"
                    "@param __facet The name of the desired facet.\n"
                    "@param __ctx The Context map to send with the invocation.\n"
                    "@return A proxy for this type, or null if the object does not support this type.");
    out << nl << "public static " << name << "Prx checkedCast(Ice.ObjectPrx __obj, String __facet, "
        << contextParam << ')';
    out << sb;
    out << nl << "return checkedCastImpl(__obj, __facet, __ctx, ice_staticId(), " << name
        << "Prx.class, " << name << "PrxHelper.class);";
    out << eb;

    out << sp;
    writeDocComment(out, "",
                    "Downcasts the given proxy to this type without contacting the remote server.\n"
                    "@param __obj The untyped proxy.\n"
                    "@return A proxy for this type.");
    out << nl << "public static " << name << "Prx uncheckedCast(Ice.ObjectPrx __obj)";
    out << sb;
    out << nl << "return uncheckedCastImpl(__obj, " << name << "Prx.class, " << name
        << "PrxHelper.class);";
    out << eb;

    out << sp;
    writeDocComment(out, "",
                    "Downcasts the given proxy to this type without contacting the remote server.\n"
                    "@param __obj The untyped proxy.\n"
                    "@param __facet The name of the desired facet.\n"
                    "@return A proxy for this type.");
    out << nl << "public static " << name << "Prx uncheckedCast(Ice.ObjectPrx __obj, String __facet)";
    out << sb;
    out << nl << "return uncheckedCastImpl(__obj, __facet, " << name << "Prx.class, " << name
        << "PrxHelper.class);";
    out << eb;

    ClassList allBases = p->allBases();
    StringList ids;
    transform(allBases.begin(), allBases.end(), back_inserter(ids), constMemFun(&Contained::scoped));
    StringList other;
    other.push_back(scoped);
    other.push_back("::Ice::Object");
    other.sort();
    ids.merge(other);
    ids.unique();
    StringList::const_iterator firstIter = ids.begin();
    StringList::const_iterator scopedIter = find(ids.begin(), ids.end(), scoped);
    assert(scopedIter != ids.end());
    StringList::difference_type scopedPos = ::IceUtilInternal::distance(firstIter, scopedIter);

    out << sp << nl << "public static final String[] __ids =";
    out << sb;

    for(StringList::const_iterator q = ids.begin(); q != ids.end();)
    {
        out << nl << '"' << *q << '"';
        if(++q != ids.end())
        {
            out << ',';
        }
    }

    out << eb << ';';

    out << sp;
    writeDocComment(out, "",
                    "Provides the Slice type ID of this type.\n"
                    "@return The Slice type ID.");
    out << nl << "public static String ice_staticId()";
    out << sb;
    out << nl << "return __ids[" << scopedPos << "];";
    out << eb;

    out << sp << nl << "public static void __write(IceInternal.BasicStream __os, " << name << "Prx v)";
    out << sb;
    out << nl << "__os.writeProxy(v);";
    out << eb;

    out << sp << nl << "public static " << name << "Prx __read(IceInternal.BasicStream __is)";
    out << sb;
    out << nl << "Ice.ObjectPrx proxy = __is.readProxy();";
    out << nl << "if(proxy != null)";
    out << sb;
    out << nl << name << "PrxHelper result = new " << name << "PrxHelper();";
    out << nl << "result.__copyFrom(proxy);";
    out << nl << "return result;";
    out << eb;
    out << nl << "return null;";
    out << eb;

    if(_stream)
    {
        out << sp;
        writeDocComment(out, "",
                        "Writes the proxy to the stream.\n"
                        "@param __outS The output stream.\n"
                        "@param v The proxy to write. A null value is legal.");
        out << nl << "public static void write(Ice.OutputStream __outS, " << name << "Prx v)";
        out << sb;
        out << nl << "__outS.writeProxy(v);";
        out << eb;

        out << sp;
        writeDocComment(out, "",
                        "Reads a proxy from the stream.\n"
                        "@param __inS The input stream.\n"
                        "@return The proxy, which may be null.");
        out << nl << "public static " << name << "Prx read(Ice.InputStream __inS)";
        out << sb;
        out << nl << "Ice.ObjectPrx proxy = __inS.readProxy();";
        out << nl << "if(proxy != null)";
        out << sb;
        out << nl << name << "PrxHelper result = new " << name << "PrxHelper();";
        out << nl << "result.__copyFrom(proxy);";
        out << nl << "return result;";
        out << eb;
        out << nl << "return null;";
        out << eb;

        out << sp;
        writeDocComment(out, "",
                        "Provides the optional format for a proxy of this type.\n"
                        "@return The optional format.");
        out << nl << "public static Ice.OptionalFormat optionalFormat()";
        out << sb;
        out << nl << "return Ice.OptionalFormat.FSize;";
        out << eb;
    }

    //
    // Avoid serialVersionUID warnings for Proxy Helper classes.
    //
    out << sp << nl << "public static final long serialVersionUID = 0L;";
    out << eb;

    close();

    if(_stream)
    {
        //
        // Class helper.
        //
        open(getAbsolute(p, "", "", "Helper"), p->file());

        Output& out2 = output();

        out2 << sp;
        writeDocComment(out2, getDeprecateReason(p, 0, p->isInterface() ? "interface" : "class"),
                        "Provides type-specific helper functions.");
        out2 << nl << "public final class " << name << "Helper";
        out2 << sb;

        out2 << sp;
        writeDocComment(out2, "",
                        "Writes an instance to the stream.\n"
                        "@param __outS The output stream.\n"
                        "@param __v The instance to write. A null value is legal.");
        out2 << nl << "public static void write(Ice.OutputStream __outS, " << fixKwd(name) << " __v)";
        out2 << sb;
        out2 << nl << "__outS.writeObject(__v);";
        out2 << eb;

        out2 << sp;
        writeDocComment(out2, "",
                        "Reads an instance from the stream.\n"
                        "@param __inS The input stream.\n"
                        "@param __h A holder to contain the instance when it is eventually unmarshaled.");
        out2 << nl << "public static void read(Ice.InputStream __inS, " << name << "Holder __h)";
        out2 << sb;
        out2 << nl << "__inS.readObject(__h);";
        out2 << eb;

        out2 << sp;
        writeDocComment(out2, "",
                        "Provides the optional format for an instance of this type.\n"
                        "@return The optional format.");
        out2 << nl << "public static Ice.OptionalFormat optionalFormat()";
        out2 << sb;
        out2 << nl << "return " << getOptionalFormat(p->declaration()) << ';';
        out2 << eb;

        out2 << eb;
        close();
    }

    return false;
}

bool
Slice::Gen::HelperVisitor::visitStructStart(const StructPtr& p)
{
    if(!p->isLocal() && _stream)
    {
        string name = p->name();
        string fixedName = fixKwd(name);

        open(getAbsolute(p, "", "", "Helper"), p->file());

        Output& out = output();

        out << sp << nl << "public final class " << name << "Helper";
        out << sb;

        out << sp << nl << "public static void write(Ice.OutputStream __outS, " << fixedName << " __v)";
        out << sb;
        out << nl << "__v.ice_write(__outS);";
        out << eb;

        out << sp << nl << "public static " << fixedName << " read(Ice.InputStream __inS)";
        out << sb;
        out << nl << fixedName << " __v = new " << fixedName << "();";
        out << nl << "__v.ice_read(__inS);";
        out << nl << "return __v;";
        out << eb;

        out << sp << nl << "public static Ice.OptionalFormat optionalFormat()";
        out << sb;
        out << nl << "return " << getOptionalFormat(p) << ';';
        out << eb;

        out << eb;
        close();
    }

    return false;
}

void
Slice::Gen::HelperVisitor::visitSequence(const SequencePtr& p)
{
    //
    // Don't generate helper for a sequence of a local type.
    //
    if(p->isLocal())
    {
        return;
    }

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p->type());
    if(builtin &&
       (builtin->kind() == Builtin::KindByte || builtin->kind() == Builtin::KindShort ||
        builtin->kind() == Builtin::KindInt || builtin->kind() == Builtin::KindLong ||
        builtin->kind() == Builtin::KindFloat || builtin->kind() == Builtin::KindDouble))
    {
        string prefix = "java:buffer";
        string meta;
        if(p->findMetaData(prefix, meta))
        {
            return; // No holders for buffer types.
        }
    }

    string name = p->name();
    string absolute = getAbsolute(p);
    string helper = getAbsolute(p, "", "", "Helper");
    string package = getPackage(p);
    string typeS = typeToString(p, TypeModeIn, package);

    //
    // We cannot allocate an array of a generic type, such as
    //
    // arr = new Map<String, String>[sz];
    //
    // Attempting to compile this code results in a "generic array creation" error
    // message. This problem can occur when the sequence's element type is a
    // dictionary, or when the element type is a nested sequence that uses a custom
    // mapping.
    //
    // The solution is to rewrite the code as follows:
    //
    // arr = (Map<String, String>[])new Map[sz];
    //
    // Unfortunately, this produces an unchecked warning during compilation, so we
    // annotate the read() method to suppress the warning.
    //
    // A simple test is to look for a "<" character in the content type, which
    // indicates the use of a generic type.
    //
    bool suppressUnchecked = false;

    string instanceType, formalType;
    bool customType = getSequenceTypes(p, "", StringList(), instanceType, formalType);

    if(!customType)
    {
        //
        // Determine sequence depth.
        //
        int depth = 0;
        TypePtr origContent = p->type();
        SequencePtr s = SequencePtr::dynamicCast(origContent);
        while(s)
        {
            //
            // Stop if the inner sequence type has a custom, serializable or protobuf type.
            //
            if(hasTypeMetaData(s))
            {
                break;
            }
            depth++;
            origContent = s->type();
            s = SequencePtr::dynamicCast(origContent);
        }

        string origContentS = typeToString(origContent, TypeModeIn, package);
        suppressUnchecked = origContentS.find('<') != string::npos;
    }

    open(helper, p->file());
    Output& out = output();

    int iter;

    out << sp << nl << "public final class " << name << "Helper";
    out << sb;

    out << nl << "public static void" << nl << "write(IceInternal.BasicStream __os, " << typeS << " __v)";
    out << sb;
    iter = 0;
    writeSequenceMarshalUnmarshalCode(out, package, p, "__v", true, iter, false);
    out << eb;

    out << sp;
    if(suppressUnchecked)
    {
        out << nl << "@SuppressWarnings(\"unchecked\")";
    }
    out << nl << "public static " << typeS << nl << "read(IceInternal.BasicStream __is)";
    out << sb;
    out << nl << typeS << " __v;";
    iter = 0;
    writeSequenceMarshalUnmarshalCode(out, package, p, "__v", false, iter, false);
    out << nl << "return __v;";
    out << eb;

    if(_stream)
    {
        out << sp << nl << "public static void write(Ice.OutputStream __outS, " << typeS << " __v)";
        out << sb;
        iter = 0;
        writeStreamSequenceMarshalUnmarshalCode(out, package, p, "__v", true, iter, false);
        out << eb;

        out << sp;
        if(suppressUnchecked)
        {
            out << nl << "@SuppressWarnings(\"unchecked\")";
        }
        out << nl << "public static " << typeS << " read(Ice.InputStream __inS)";
        out << sb;
        out << nl << typeS << " __v;";
        iter = 0;
        writeStreamSequenceMarshalUnmarshalCode(out, package, p, "__v", false, iter, false);
        out << nl << "return __v;";
        out << eb;

        out << sp << nl << "public static Ice.OptionalFormat optionalFormat()";
        out << sb;
        out << nl << "return " << getOptionalFormat(p) << ';';
        out << eb;
    }

    out << eb;
    close();
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

    string name = p->name();
    string absolute = getAbsolute(p);
    string helper = getAbsolute(p, "", "", "Helper");
    string package = getPackage(p);
    StringList metaData = p->getMetaData();
    string formalType = typeToString(p, TypeModeIn, package, StringList(), true);

    open(helper, p->file());
    Output& out = output();

    int iter;

    out << sp << nl << "public final class " << name << "Helper";
    out << sb;

    out << nl << "public static void" << nl << "write(IceInternal.BasicStream __os, " << formalType << " __v)";
    out << sb;
    iter = 0;
    writeDictionaryMarshalUnmarshalCode(out, package, p, "__v", true, iter, false);
    out << eb;

    out << sp << nl << "public static " << formalType
        << nl << "read(IceInternal.BasicStream __is)";
    out << sb;
    out << nl << formalType << " __v;";
    iter = 0;
    writeDictionaryMarshalUnmarshalCode(out, package, p, "__v", false, iter, false);
    out << nl << "return __v;";
    out << eb;

    if(_stream)
    {
        out << sp << nl << "public static void write(Ice.OutputStream __outS, " << formalType
            << " __v)";
        out << sb;
        iter = 0;
        writeStreamDictionaryMarshalUnmarshalCode(out, package, p, "__v", true, iter, false);
        out << eb;

        out << sp << nl << "public static " << formalType << " read(Ice.InputStream __inS)";
        out << sb;
        out << nl << formalType << " __v;";
        iter = 0;
        writeStreamDictionaryMarshalUnmarshalCode(out, package, p, "__v", false, iter, false);
        out << nl << "return __v;";
        out << eb;

        out << sp << nl << "public static Ice.OptionalFormat optionalFormat()";
        out << sb;
        out << nl << "return " << getOptionalFormat(p) << ';';
        out << eb;
    }

    out << eb;
    close();
}

void
Slice::Gen::HelperVisitor::visitEnum(const EnumPtr& p)
{
    if(!p->isLocal() && _stream)
    {
        string name = p->name();
        string fixedName = fixKwd(name);

        open(getAbsolute(p, "", "", "Helper"), p->file());

        Output& out = output();

        out << sp << nl << "public final class " << name << "Helper";
        out << sb;

        out << sp << nl << "public static void write(Ice.OutputStream __outS, " << fixedName << " __v)";
        out << sb;
        out << nl << "__v.ice_write(__outS);";
        out << eb;

        out << sp << nl << "public static " << fixedName << " read(Ice.InputStream __inS)";
        out << sb;
        out << nl << "return " << fixedName << ".ice_read(__inS);";
        out << eb;

        out << sp << nl << "public static Ice.OptionalFormat optionalFormat()";
        out << sb;
        out << nl << "return " << getOptionalFormat(p) << ';';
        out << eb;

        out << eb;
        close();
    }
}

void
Slice::Gen::HelperVisitor::writeOperation(const ClassDefPtr& p, const string& package, const OperationPtr& op,
                                          bool optionalMapping)
{
    const string name = p->name();
    Output& out = output();

    const string contextParam = "java.util.Map<String, String> __ctx";
    const string explicitContextParam = "boolean __explicitCtx";

    const ContainerPtr container = op->container();
    const ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    const string opName = fixKwd(op->name());
    const TypePtr ret = op->returnType();
    const string retS = typeToString(ret, TypeModeReturn, package, op->getMetaData(), true, op->returnIsOptional());

    vector<string> params = getParamsProxy(op, package, false, optionalMapping);
    vector<string> args = getArgs(op);

    ParamDeclList inParams;
    ParamDeclList outParams;
    ParamDeclList paramList = op->parameters();
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

    ExceptionList throws = op->throws();
    throws.sort();
    throws.unique();

    //
    // Write two synchronous versions of the operation - with and without a
    // context parameter.
    //
    out << sp << nl << "public " << retS << ' ' << opName << spar << params << epar;
    writeThrowsClause(package, throws);
    out << sb;
    out << nl;
    if(ret)
    {
        out << "return ";
    }
    out << opName << spar << args << "null" << "false" << epar << ';';
    out << eb;

    out << sp << nl << "public " << retS << ' ' << opName << spar << params << contextParam << epar;
    writeThrowsClause(package, throws);
    out << sb;
    out << nl;
    if(ret)
    {
        out << "return ";
    }
    out << opName << spar << args << "__ctx" << "true" << epar << ';';
    out << eb;

    out << sp;
    out << nl << "private " << retS << ' ' << opName << spar << params << contextParam
        << explicitContextParam << epar;
    writeThrowsClause(package, throws);
    out << sb;

    // This code replaces the synchronous calls with chained AMI calls.
    if(op->returnsData())
    {
        out << nl << "__checkTwowayOnly(__" << op->name() << "_name);";
    }

    if(ret)
    {
        out << nl << "return ";
    }
    else
    {
        out << nl;
    }

    out << "end_" << op->name() << "(";
    vector<string> inOutArgs = getInOutArgs(op, OutParam);
    if(!inOutArgs.empty()) {
        for(vector<string>::const_iterator p = inOutArgs.begin(); p != inOutArgs.end(); ++p) {
            out << *p << ", ";
        }
    }
    vector<string> inArgs = getInOutArgs(op, InParam);
    out << "begin_" << op->name() << "(";
    if(!inArgs.empty())
    {
        for(vector<string>::const_iterator p = inArgs.begin(); p != inArgs.end(); ++p) {
            out << *p << ", ";
        }
    }
    out << "__ctx, __explicitCtx, true, null));";
    out << eb;

    {
        //
        // Write the asynchronous begin methods.
        //
        vector<string> inParams = getInOutParams(op, package, InParam, true, optionalMapping);
        vector<string> inArgs = getInOutArgs(op, InParam);
        const string callbackParam = "Ice.Callback __cb";
        const ParamDeclList paramList = op->parameters();
        int iter;

        //
        // Type-unsafe begin methods
        //
        out << sp << nl << "public Ice.AsyncResult begin_" << op->name() << spar << inParams << epar;
        out << sb;
        out << nl << "return begin_" << op->name() << spar << inArgs << "null" << "false" << "false" << "null" << epar
            << ';';
        out << eb;

        out << sp << nl << "public Ice.AsyncResult begin_" << op->name() << spar << inParams << contextParam << epar;
        out << sb;
        out << nl << "return begin_" << op->name() << spar << inArgs << "__ctx" << "true" << "false" << "null" << epar
            << ';';
        out << eb;

        out << sp << nl << "public Ice.AsyncResult begin_" << op->name() << spar << inParams << callbackParam << epar;
        out << sb;
        out << nl << "return begin_" << op->name() << spar << inArgs << "null" << "false" << "false" << "__cb" << epar
            << ';';
        out << eb;

        out << sp << nl << "public Ice.AsyncResult begin_" << op->name() << spar << inParams << contextParam
            << callbackParam << epar;
        out << sb;
        out << nl << "return begin_" << op->name() << spar << inArgs << "__ctx" << "true" << "false" << "__cb" << epar
            << ';';
        out << eb;

        //
        // Type-safe begin methods
        //
        string typeSafeCallbackParam;

        //
        // Get the name of the callback using the name of the class in which this
        // operation was defined.
        //
        ContainerPtr container = op->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        string opClassName = getAbsolute(cl, package, "Callback_", '_' + op->name());
        typeSafeCallbackParam = opClassName + " __cb";

        out << sp << nl << "public Ice.AsyncResult begin_" << op->name() << spar << inParams << typeSafeCallbackParam
            << epar;
        out << sb;
        out << nl << "return begin_" << op->name() << spar << inArgs << "null" << "false" << "false" << "__cb" << epar
            << ';';
        out << eb;

        out << sp << nl << "public Ice.AsyncResult begin_" << op->name() << spar << inParams << contextParam
            << typeSafeCallbackParam << epar;
        out << sb;
        out << nl << "return begin_" << op->name() << spar << inArgs << "__ctx" << "true" << "false" << "__cb" << epar
            << ';';
        out << eb;

        //
        // Async methods that accept Java 8 lambda callbacks.
        //
        out << sp;
        out << nl << "public Ice.AsyncResult begin_" << op->name();
        writeParamList(out, getParamsAsyncLambda(op, package, false, false, optionalMapping));
        out << sb;
        out << nl << "return begin_" << op->name() << spar << getArgsAsyncLambda(op, package) << epar << ';';
        out << eb;

        out << sp;
        out << nl << "public Ice.AsyncResult begin_" << op->name();
        writeParamList(out, getParamsAsyncLambda(op, package, false, true, optionalMapping));
        out << sb;
        out << nl << "return begin_" << op->name() << spar << getArgsAsyncLambda(op, package, false, true) << epar
            << ';';
        out << eb;

        out << sp;
        out << nl << "public Ice.AsyncResult begin_" << op->name();
        writeParamList(out, getParamsAsyncLambda(op, package, true, false, optionalMapping));
        out << sb;
        out << nl << "return begin_" << op->name() << spar << getArgsAsyncLambda(op, package, true) << epar << ';';
        out << eb;

        out << sp;
        out << nl << "public Ice.AsyncResult begin_" << op->name();
        writeParamList(out, getParamsAsyncLambda(op, package, true, true, optionalMapping));
        out << sb;
        out << nl << "return begin_" << op->name() << spar << getArgsAsyncLambda(op, package, true, true) << epar
            << ';';
        out << eb;

        vector<string> params = inParams;
        params.push_back(contextParam);
        params.push_back("boolean __explicitCtx");
        params.push_back("boolean __synchronous");
        vector<string> asyncParams = getParamsAsyncLambda(op, package, false, true, optionalMapping, false);
        copy(asyncParams.begin(), asyncParams.end(), back_inserter(params));

        out << sp;
        out << nl << "private Ice.AsyncResult begin_" << op->name();
        writeParamList(out, params);
        out << sb;

        ParamDeclList outParams = getOutParams(op);

        if(!op->returnsData())
        {
            params = getInOutArgs(op, InParam);
            params.push_back("__ctx");
            params.push_back("__explicitCtx");
            params.push_back("__synchronous");
            params.push_back("new IceInternal.Functional_OnewayCallback(__responseCb, __exceptionCb, __sentCb)");
            out << nl << "return begin_" << op->name();
            writeParamList(out, params);
            out << ';';
        }
        else if((ret && !outParams.empty()) || (outParams.size() > 1))
        {
            params.clear();
            params.push_back(getLambdaResponseCB(op, package) + " responseCb");
            if(!throws.empty())
            {
                params.push_back("IceInternal.Functional_GenericCallback1<Ice.UserException> userExceptionCb");
            }
            params.push_back("IceInternal.Functional_GenericCallback1<Ice.Exception> exceptionCb");
            params.push_back("IceInternal.Functional_BoolCallback sentCb");

            out << sp;
            out << nl << "class CB extends " << getAsyncCallbackBaseClass(op, true);
            out << sb;
            out << nl << "public CB";
            writeParamList(out, params);
            out << sb;
            out << nl << "super(responseCb != null, ";
            if(!throws.empty())
            {
                out << "userExceptionCb, ";
            }
            out << "exceptionCb, sentCb);";
            out << nl << "__responseCb = responseCb;";
            out << eb;

            out << sp;
            out << nl << "public void response" << spar << getParamsAsyncCB(op, package, false, true) << epar;
            out << sb;
            out << nl << "if(__responseCb != null)";
            out << sb;
            out << nl << "__responseCb.apply" << spar;
            if(ret)
            {
                out << "__ret";
            }
            out << getInOutArgs(op, OutParam) << epar << ';';
            out << eb;
            out << eb;

            out << sp;
            out << nl << "public final void __completed(Ice.AsyncResult __result)";
            out << sb;
            out << nl << p->name() << "PrxHelper.__" << op->name() << "_completed(this, __result);";
            out << eb;
            out << sp;
            out << nl << "private final " << getLambdaResponseCB(op, package) << " __responseCb;";
            out << eb;

            out << nl << "return begin_" << op->name() << spar << getInOutArgs(op, InParam) << "__ctx"
                << "__explicitCtx"
                << "__synchronous"
                << (throws.empty() ? "new CB(__responseCb, __exceptionCb, __sentCb)" :
                                     "new CB(__responseCb, __userExceptionCb, __exceptionCb, __sentCb)")
                << epar << ';';
        }
        else
        {
            params = getInOutArgs(op, InParam);
            params.push_back("__ctx");
            params.push_back("__explicitCtx");
            params.push_back("__synchronous");

            const string baseClass = getAsyncCallbackBaseClass(op, true);
            out << nl << "return begin_" << op->name();
            writeParamList(out, params, false, false);
            out << nl
                << (throws.empty() ? "new " + baseClass + "(__responseCb, __exceptionCb, __sentCb)" :
                                     "new " + baseClass + "(__responseCb, __userExceptionCb, __exceptionCb, __sentCb)");
            out.inc();
            out << sb;
            out << nl << "public final void __completed(Ice.AsyncResult __result)";
            out << sb;
            out << nl << p->name() << "PrxHelper.__" << op->name() << "_completed(this, __result);";
            out << eb;
            out << eb;
            out << ");";
            out.dec();
            out.restoreIndent();
        }
        out << eb;

        //
        // Implementation of begin method
        //
        params = inParams;
        params.push_back(contextParam);
        params.push_back("boolean __explicitCtx");
        params.push_back("boolean __synchronous");
        params.push_back("IceInternal.CallbackBase __cb");

        out << sp;
        out << nl << "private Ice.AsyncResult begin_" << op->name();
        writeParamList(out, params);
        out << sb;
        if(op->returnsData())
        {
            out << nl << "__checkAsyncTwowayOnly(__" << op->name() << "_name);";
        }
        out << nl << "IceInternal.OutgoingAsync __result = getOutgoingAsync(__" << op->name()
            << "_name, __cb);";
        out << nl << "try";
        out << sb;

        out << nl << "__result.prepare(__" << op->name() << "_name, " << sliceModeToIceMode(op->sendMode())
            << ", __ctx, __explicitCtx, __synchronous);";

        iter = 0;
        if(!inArgs.empty())
        {
            out << nl << "IceInternal.BasicStream __os = __result.startWriteParams("
                << opFormatTypeToString(op) << ");";
            ParamDeclList pl;
            for(ParamDeclList::const_iterator pli = paramList.begin(); pli != paramList.end(); ++pli)
            {
                if(!(*pli)->isOutParam())
                {
                    pl.push_back(*pli);
                }
            }
            writeMarshalUnmarshalParams(out, package, pl, 0, iter, true, optionalMapping);
            if(op->sendsClasses(false))
            {
                out << nl << "__os.writePendingObjects();";
            }
            out << nl << "__result.endWriteParams();";
        }
        else
        {
            out << nl << "__result.writeEmptyParams();";
        }

        out << nl << "__result.invoke();";
        out << eb;
        out << nl << "catch(Ice.Exception __ex)";
        out << sb;
        out << nl << "__result.abort(__ex);";
        out << eb;
        out << nl << "return __result;";
        out << eb;
    }
}

Slice::Gen::ProxyVisitor::ProxyVisitor(const string& dir) :
    JavaVisitor(dir)
{
}

bool
Slice::Gen::ProxyVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
    {
        return false;
    }

    string name = p->name();
    ClassList bases = p->bases();
    string package = getPackage(p);
    string absolute = getAbsolute(p, "", "", "Prx");

    open(absolute, p->file());

    Output& out = output();

    //
    // Generate a Java interface as the user-visible type
    //
    out << sp;
    writeDocComment(out, p, getDeprecateReason(p, 0, p->isInterface() ? "interface" : "class"));
    out << nl << "public interface " << name << "Prx extends ";
    if(bases.empty())
    {
        out << "Ice.ObjectPrx";
    }
    else
    {
        out.useCurrentPosAsIndent();
        for(ClassList::const_iterator q = bases.begin(); q != bases.end();)
        {
            out << getAbsolute(*q, package, "", "Prx");
            if(++q != bases.end())
            {
                out << ',' << nl;
            }
        }
        out.restoreIndent();
    }

    out << sb;

    return true;
}

void
Slice::Gen::ProxyVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    Output& out = output();
    out << eb;
    close();
}

void
Slice::Gen::ProxyVisitor::visitOperation(const OperationPtr& p)
{
    string name = fixKwd(p->name());
    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    string package = getPackage(cl);

    Output& out = output();

    TypePtr ret = p->returnType();
    string retS = typeToString(ret, TypeModeReturn, package, p->getMetaData(), true, p->returnIsOptional());
    vector<string> params = getParamsProxy(p, package, false, true);
    ExceptionList throws = p->throws();
    throws.sort();
    throws.unique();

    string deprecateReason = getDeprecateReason(p, cl, "operation");
    string contextDoc = "@param __ctx The Context map to send with the invocation.";
    string contextParam = "java.util.Map<String, String> __ctx";
    string lambdaResponseDoc = "@param __responseCb The lambda response callback.";
    string lambdaUserExDoc = "@param __userExceptionCb The lambda user exception callback.";
    string lambdaExDoc = "@param __exceptionCb The lambda exception callback.";
    string lambdaSentDoc = "@param __sentCb The lambda sent callback.";

    const bool optional = p->sendsOptionals();

    //
    // Write two synchronous versions of the operation - with and without a context parameter.
    //
    out << sp;
    writeDocComment(out, p, deprecateReason);

    out << nl << "public " << retS << ' ' << name << spar << params << epar;
    writeThrowsClause(package, throws);
    out << ';';

    out << sp;
    writeDocComment(out, p, deprecateReason, contextDoc);

    out << nl << "public " << retS << ' ' << name << spar << params << contextParam << epar;
    writeThrowsClause(package, throws);
    out << ';';

    if(optional)
    {
        //
        // Write overloaded versions of the methods using required params.
        //
        vector<string> reqParams = getParamsProxy(p, package, false, false);

        out << sp;
        writeDocComment(out, p, deprecateReason);
        out << nl << "public " << retS << ' ' << name << spar << reqParams << epar;
        writeThrowsClause(package, throws);
        out << ';';

        out << sp;
        writeDocComment(out, p, deprecateReason, contextDoc);
        out << nl << "public " << retS << ' ' << name << spar << reqParams << contextParam << epar;
        writeThrowsClause(package, throws);
        out << ';';
    }

    {
        //
        // Write the asynchronous begin/end methods.
        //
        // Start with the type-unsafe begin methods.
        //
        vector<string> inParams = getInOutParams(p, package, InParam, true, true);
        string callbackParam = "Ice.Callback __cb";
        string callbackDoc = "@param __cb The asynchronous callback object.";

        out << sp;
        writeDocCommentAMI(out, p, InParam);
        out << nl << "public Ice.AsyncResult begin_" << p->name() << spar << inParams << epar << ';';

        out << sp;
        writeDocCommentAMI(out, p, InParam, contextDoc);
        out << nl << "public Ice.AsyncResult begin_" << p->name() << spar << inParams << contextParam << epar << ';';

        out << sp;
        writeDocCommentAMI(out, p, InParam, callbackDoc);
        out << nl << "public Ice.AsyncResult begin_" << p->name() << spar << inParams << callbackParam << epar << ';';

        out << sp;
        writeDocCommentAMI(out, p, InParam, contextDoc, callbackDoc);
        out << nl << "public Ice.AsyncResult begin_" << p->name() << spar << inParams << contextParam << callbackParam
            << epar << ';';

        //
        // Type-safe begin methods.
        //
        string typeSafeCallbackParam;

        //
        // Get the name of the callback using the name of the class in which this
        // operation was defined.
        //
        ContainerPtr container = p->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        string opClassName = getAbsolute(cl, package, "Callback_", '_' + p->name());
        typeSafeCallbackParam = opClassName + " __cb";

        out << sp;
        writeDocCommentAMI(out, p, InParam, callbackDoc);
        out << nl << "public Ice.AsyncResult begin_" << p->name() << spar << inParams << typeSafeCallbackParam
            << epar << ';';

        out << sp;
        writeDocCommentAMI(out, p, InParam, contextDoc, callbackDoc);
        out << nl << "public Ice.AsyncResult begin_" << p->name() << spar << inParams << contextParam
            << typeSafeCallbackParam << epar << ';';

        //
        // Generate the Callback Response interface if the operation has more than one
        // return parameter. Operations with just one return parameter use one of the
        // builtin async callback interfaces.
        //
        {
            ParamDeclList outParams = getOutParams(p);
            if((ret && !outParams.empty()) || outParams.size() > 1)
            {
                vector<string> params = getParamsAsyncCB(p, package, false, true);
                out << sp;
                out << nl << "public interface " << getLambdaResponseCB(p, package);
                out << sb;
                out << nl << "void apply" << spar << params << epar << ';';
                out << eb;
            }
        }

        //
        // Async methods that accept Java 8 lambda callbacks.
        //
        {
            out << sp;
            writeDocCommentAMI(out, p, InParam, lambdaResponseDoc, throws.empty() ? "" : lambdaUserExDoc, lambdaExDoc);
            out << nl << "public Ice.AsyncResult begin_" << p->name();
            writeParamList(out, getParamsAsyncLambda(p, package, false, false, true));
            out << ';';

            out << sp;
            writeDocCommentAMI(out, p, InParam, lambdaResponseDoc, throws.empty() ? "" : lambdaUserExDoc, lambdaExDoc,
                               lambdaSentDoc);
            out << nl << "public Ice.AsyncResult begin_" << p->name();
            writeParamList(out, getParamsAsyncLambda(p, package, false, true, true));
            out << ';';

            out << sp;
            writeDocCommentAMI(out, p, InParam, contextDoc, lambdaResponseDoc, throws.empty() ? "" : lambdaUserExDoc,
                               lambdaExDoc);
            out << nl << "public Ice.AsyncResult begin_" << p->name();
            writeParamList(out, getParamsAsyncLambda(p, package, true, false, true));
            out << ';';

            out << sp;
            writeDocCommentAMI(out, p, InParam, contextDoc, lambdaResponseDoc, throws.empty() ? "" : lambdaUserExDoc,
                               lambdaExDoc, lambdaSentDoc);
            out << nl << "public Ice.AsyncResult begin_" << p->name();
            writeParamList(out, getParamsAsyncLambda(p, package, true, true, true));
            out << ';';
        }

        vector<string> outParams = getInOutParams(p, package, OutParam, true, true);

        out << sp;
        writeDocCommentAMI(out, p, OutParam);
        out << nl << "public " << retS << " end_" << p->name() << spar << outParams << "Ice.AsyncResult __result"
            << epar;
        writeThrowsClause(package, throws);
        out << ';';
    }

    if(optional)
    {
        //
        // Write overloaded versions of the methods using required params.
        //
        vector<string> inParams = getInOutParams(p, package, InParam, true, false);
        string callbackParam = "Ice.Callback __cb";
        string callbackDoc = "@param __cb The asynchronous callback object.";

        out << sp;
        writeDocCommentAMI(out, p, InParam);
        out << nl << "public Ice.AsyncResult begin_" << p->name() << spar << inParams << epar << ';';

        out << sp;
        writeDocCommentAMI(out, p, InParam, contextDoc);
        out << nl << "public Ice.AsyncResult begin_" << p->name() << spar << inParams << contextParam << epar << ';';

        out << sp;
        writeDocCommentAMI(out, p, InParam, callbackDoc);
        out << nl << "public Ice.AsyncResult begin_" << p->name() << spar << inParams << callbackParam << epar << ';';

        out << sp;
        writeDocCommentAMI(out, p, InParam, contextDoc, callbackDoc);
        out << nl << "public Ice.AsyncResult begin_" << p->name() << spar << inParams << contextParam << callbackParam
            << epar << ';';

        //
        // Async methods that accept Java 8 lambda callbacks.
        //
        {
            out << sp;
            writeDocCommentAMI(out, p, InParam, lambdaResponseDoc, throws.empty() ? "" : lambdaUserExDoc, lambdaExDoc);
            out << nl << "public Ice.AsyncResult begin_" << p->name();
            writeParamList(out, getParamsAsyncLambda(p, package));
            out << ';';

            out << sp;
            writeDocCommentAMI(out, p, InParam, lambdaResponseDoc, throws.empty() ? "" : lambdaUserExDoc, lambdaExDoc,
                               lambdaSentDoc);
            out << nl << "public Ice.AsyncResult begin_" << p->name();
            writeParamList(out, getParamsAsyncLambda(p, package, false, true));
            out << ';';

            out << sp;
            writeDocCommentAMI(out, p, InParam, contextDoc, lambdaResponseDoc, throws.empty() ? "" : lambdaUserExDoc,
                               lambdaExDoc);
            out << nl << "public Ice.AsyncResult begin_" << p->name();
            writeParamList(out, getParamsAsyncLambda(p, package, true));
            out << ';';

            out << sp;
            writeDocCommentAMI(out, p, InParam, contextDoc, lambdaResponseDoc, throws.empty() ? "" : lambdaUserExDoc,
                               lambdaExDoc, lambdaSentDoc);
            out << nl << "public Ice.AsyncResult begin_" << p->name();
            writeParamList(out, getParamsAsyncLambda(p, package, true, true));
            out << ';';
        }

        //
        // Type-safe begin methods.
        //
        string typeSafeCallbackParam;

        //
        // Get the name of the callback using the name of the class in which this
        // operation was defined.
        //
        ContainerPtr container = p->container();
        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
        string opClassName = getAbsolute(cl, package, "Callback_", '_' + p->name());
        typeSafeCallbackParam = opClassName + " __cb";

        out << sp;
        writeDocCommentAMI(out, p, InParam, callbackDoc);
        out << nl << "public Ice.AsyncResult begin_" << p->name() << spar << inParams << typeSafeCallbackParam
            << epar << ';';

        out << sp;
        writeDocCommentAMI(out, p, InParam, contextDoc, callbackDoc);
        out << nl << "public Ice.AsyncResult begin_" << p->name() << spar << inParams << contextParam
            << typeSafeCallbackParam << epar << ';';
    }
}

Slice::Gen::DispatcherVisitor::DispatcherVisitor(const string& dir, bool stream) :
    JavaVisitor(dir), _stream(stream)
{
}

bool
Slice::Gen::DispatcherVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal() || !p->isInterface())
    {
        return false;
    }

    string name = p->name();
    ClassList bases = p->bases();
    string absolute = getAbsolute(p, "", "_", "Disp");

    open(absolute, p->file());

    Output& out = output();

    out << sp;
    writeDocComment(out, p, getDeprecateReason(p, 0, p->isInterface() ? "interface" : "class"));
    out << nl << "public abstract class _" << name << "Disp extends Ice.ObjectImpl implements " << fixKwd(name);
    out << sb;

    out << sp << nl << "protected void" << nl << "ice_copyStateFrom(Ice.Object __obj)";
    out.inc();
    out << nl << "throws java.lang.CloneNotSupportedException";
    out.dec();
    out << sb;
    out << nl << "throw new java.lang.CloneNotSupportedException();";
    out << eb;

    writeDispatchAndMarshalling(out, p, _stream);

    //
    // Avoid serialVersionUID warnings for dispatch classes.
    //
    out << sp << nl << "public static final long serialVersionUID = 0L;";
    out << eb;
    close();

    return false;
}

Slice::Gen::BaseImplVisitor::BaseImplVisitor(const string& dir) :
    JavaVisitor(dir)
{
}

void
Slice::Gen::BaseImplVisitor::writeDecl(Output& out, const string& package, const string& name, const TypePtr& type,
                                       const StringList& metaData, bool optional)
{
    string typeS = typeToString(type, TypeModeIn, package, metaData, true, optional);
    out << nl << typeS << ' ' << name;

    if(optional)
    {
        out << " = new " << typeS << "();";
    }
    else
    {
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
        if(builtin)
        {
            switch(builtin->kind())
            {
                case Builtin::KindBool:
                {
                    out << " = false";
                    break;
                }
                case Builtin::KindByte:
                {
                    out << " = (byte)0";
                    break;
                }
                case Builtin::KindShort:
                {
                    out << " = (short)0";
                    break;
                }
                case Builtin::KindInt:
                case Builtin::KindLong:
                {
                    out << " = 0";
                    break;
                }
                case Builtin::KindFloat:
                {
                    out << " = (float)0.0";
                    break;
                }
                case Builtin::KindDouble:
                {
                    out << " = 0.0";
                    break;
                }
                case Builtin::KindString:
                {
                    out << " = \"\"";
                    break;
                }
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                {
                    out << " = null";
                    break;
                }
            }
        }
        else
        {
            EnumPtr en = EnumPtr::dynamicCast(type);
            if(en)
            {
                EnumeratorList enumerators = en->getEnumerators();
                out << " = " << getAbsolute(en, package) << '.' << fixKwd(enumerators.front()->name());
            }
            else
            {
                out << " = null";
            }
        }

        out << ';';
    }
}

void
Slice::Gen::BaseImplVisitor::writeReturn(Output& out, const TypePtr& type, bool optional)
{
    if(optional)
    {
        out << nl << "return null;";
    }
    else
    {
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
        if(builtin)
        {
            switch(builtin->kind())
            {
                case Builtin::KindBool:
                {
                    out << nl << "return false;";
                    break;
                }
                case Builtin::KindByte:
                {
                    out << nl << "return (byte)0;";
                    break;
                }
                case Builtin::KindShort:
                {
                    out << nl << "return (short)0;";
                    break;
                }
                case Builtin::KindInt:
                case Builtin::KindLong:
                {
                    out << nl << "return 0;";
                    break;
                }
                case Builtin::KindFloat:
                {
                    out << nl << "return (float)0.0;";
                    break;
                }
                case Builtin::KindDouble:
                {
                    out << nl << "return 0.0;";
                    break;
                }
                case Builtin::KindString:
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                {
                    out << nl << "return null;";
                    break;
                }
            }
            return;
        }

        out << nl << "return null;";
    }
}

void
Slice::Gen::BaseImplVisitor::writeOperation(Output& out, const string& package, const OperationPtr& op, bool local)
{
    string opName = op->name();

    const TypePtr ret = op->returnType();
    const bool optionalMapping = useOptionalMapping(op);
    const StringList opMetaData = op->getMetaData();
    const string retS = typeToString(ret, TypeModeReturn, package, opMetaData, true,
                                     optionalMapping && op->returnIsOptional());
    vector<string> params = getParams(op, package, false, optionalMapping);

    ContainerPtr container = op->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);

    if(!local && (cl->hasMetaData("amd") || op->hasMetaData("amd")))
    {
        vector<string> paramsAMD = getParamsAsync(op, package, true, true);

        out << sp << nl << "public void " << opName << "_async" << spar << paramsAMD << "Ice.Current __current"
            << epar;

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
        writeThrowsClause(package, throws);

        out << sb;

        string result = "__r";
        ParamDeclList paramList = op->parameters();
        for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
        {
            if((*q)->name() == result)
            {
                result = "_" + result;
                break;
            }
        }
        if(ret)
        {
            writeDecl(out, package, result, ret, opMetaData, optionalMapping && op->returnIsOptional());
        }
        for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
        {
            if((*q)->isOutParam())
            {
                writeDecl(out, package, fixKwd((*q)->name()), (*q)->type(), (*q)->getMetaData(),
                          optionalMapping && (*q)->optional());
            }
        }

        out << nl << "__cb.ice_response(";
        if(ret)
        {
            out << result;
        }
        bool firstOutParam = true;
        for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
        {
            if((*q)->isOutParam())
            {
                if(ret || !firstOutParam)
                {
                    out << ", ";
                }
                out << fixKwd((*q)->name());
                firstOutParam = false;
            }
        }
        out << ");";

        out << eb;
    }
    else
    {
        out << sp << nl << "public " << retS << nl << fixKwd(opName) << spar << params;
        if(!local)
        {
            out << "Ice.Current __current";
        }
        out << epar;

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

        if(op->hasMetaData("UserException"))
        {
            out.inc();
            out << nl << "throws Ice.UserException";
            out.dec();
        }
        else
        {
            writeThrowsClause(package, throws);
        }

        out << sb;

        //
        // Return value
        //
        if(ret)
        {
            writeReturn(out, ret, optionalMapping && op->returnIsOptional());
        }

        out << eb;
    }
}

Slice::Gen::ImplVisitor::ImplVisitor(const string& dir) :
    BaseImplVisitor(dir)
{
}

bool
Slice::Gen::ImplVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(!p->isAbstract())
    {
        return false;
    }

    string name = p->name();
    ClassList bases = p->bases();
    string package = getPackage(p);
    string absolute = getAbsolute(p, "", "", "I");

    open(absolute, p->file());

    Output& out = output();

    out << sp << nl << "public final class " << name << 'I';
    if(p->isInterface())
    {
        if(p->isLocal())
        {
            out << " implements " << fixKwd(name);
        }
        else
        {
            out << " extends _" << name << "Disp";
        }
    }
    else
    {
        out << " extends " << fixKwd(name);
    }
    out << sb;

    out << nl << "public" << nl << name << "I()";
    out << sb;
    out << eb;

    OperationList ops = p->allOperations();
    for(OperationList::iterator r = ops.begin(); r != ops.end(); ++r)
    {
        writeOperation(out, package, *r, p->isLocal());
    }

    out << eb;
    close();

    return false;
}

Slice::Gen::ImplTieVisitor::ImplTieVisitor(const string& dir) :
    BaseImplVisitor(dir)
{
}

bool
Slice::Gen::ImplTieVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(!p->isAbstract())
    {
        return false;
    }

    string name = p->name();
    ClassList bases = p->bases();
    string package = getPackage(p);
    string absolute = getAbsolute(p, "", "", "I");

    open(absolute, p->file());

    Output& out = output();

    //
    // Use implementation inheritance in the following situations:
    //
    // * if a class extends another class
    // * if a class implements a single interface
    // * if an interface extends only one interface
    //
    bool inheritImpl = (!p->isInterface() && !bases.empty() && !bases.front()->isInterface()) || (bases.size() == 1);

    out << sp << nl << "public class " << name << 'I';
    if(inheritImpl)
    {
        out << " extends ";
        if(bases.front()->isAbstract())
        {
            out << bases.front()->name() << 'I';
        }
        else
        {
            out << fixKwd(bases.front()->name());
        }
    }
    out << " implements " << '_' << name << "Operations";
    if(p->isLocal())
    {
        out << "NC";
    }
    out << sb;

    out << nl << "public" << nl << name << "I()";
    out << sb;
    out << eb;

    OperationList ops = p->allOperations();
    ops.sort();

    OperationList baseOps;
    if(inheritImpl)
    {
        baseOps = bases.front()->allOperations();
        baseOps.sort();
    }

    for(OperationList::iterator r = ops.begin(); r != ops.end(); ++r)
    {
        if(inheritImpl && binary_search(baseOps.begin(), baseOps.end(), *r))
        {
            out << sp;
            out << nl << "/*";
            out << nl << " * Implemented by " << bases.front()->name() << 'I';
            out << nl << " *";
            writeOperation(out, package, *r, p->isLocal());
            out << sp;
            out << nl << "*/";
        }
        else
        {
            writeOperation(out, package, *r, p->isLocal());
        }
    }

    out << eb;
    close();

    return false;
}

Slice::Gen::AsyncVisitor::AsyncVisitor(const string& dir) :
    JavaVisitor(dir)
{
}

void
Slice::Gen::AsyncVisitor::visitOperation(const OperationPtr& p)
{
    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);

    if(cl->isLocal())
    {
        return;
    }

    string name = p->name();
    string classPkg = getPackage(cl);
    StringList opMetaData = p->getMetaData();

    //
    // Generate new-style callback.
    //
    {
        TypePtr ret = p->returnType();
        ParamDeclList outParams;
        ParamDeclList paramList = p->parameters();
        for(ParamDeclList::const_iterator pli = paramList.begin(); pli != paramList.end(); ++pli)
        {
            if((*pli)->isOutParam())
            {
                outParams.push_back(*pli);
            }
        }

        vector<string> params = getParamsAsyncCB(p, classPkg, false, true);
        vector<string> args = getInOutArgs(p, OutParam);
        ExceptionList throws = p->throws();

        //
        // If the operation has more than one return parameter we generate a Callback
        // interface to use in the method signatures.
        //
        if(p->returnsData() && ((ret && !outParams.empty()) || outParams.size() > 1))
        {
            open(getAbsolute(cl, "", "_Callback_", "_" + name), p->file());

            Output& out = output();

            writeDocCommentOp(out, p);
            out << sp << nl << "public interface " << ("_Callback_" + cl->name()) << '_' << name
                << " extends " << (throws.empty() ? "Ice.TwowayCallback" : "Ice.TwowayCallbackUE");
            out << sb;
            out << nl << "public void response" << spar << params << epar << ';';
            out << eb;

            close();
        }

        string classNameAsync = "Callback_" + cl->name();
        string absoluteAsync = getAbsolute(cl, "", "Callback_", "_" + name);

        open(absoluteAsync, p->file());

        Output& out = output();

        writeDocCommentOp(out, p);
        out << sp << nl << "public abstract class " << classNameAsync << '_' << name;

        if(p->returnsData())
        {
            out.inc();
            out << nl << "extends " << getAsyncCallbackBaseClass(p, false);
            out.dec();
            out << sb;

            out << sp << nl << "public final void __completed(Ice.AsyncResult __result)";
            out << sb;
            out << nl << cl->name() << "PrxHelper.__" << p->name() << "_completed(this, __result);";
            out << eb;

            out << eb;
        }
        else
        {
            out << " extends Ice.OnewayCallback";
            out << sb;
            out << eb;
        }

        close();
    }

    if(cl->hasMetaData("amd") || p->hasMetaData("amd"))
    {
        string classNameAMD = "AMD_" + cl->name();
        string absoluteAMD = getAbsolute(cl, "", "AMD_", "_" + name);

        string classNameAMDI = "_AMD_" + cl->name();
        string absoluteAMDI = getAbsolute(cl, "", "_AMD_", "_" + name);

        const bool optionalMapping = useOptionalMapping(p);
        vector<string> paramsAMD = getParamsAsyncCB(p, classPkg, true, optionalMapping);

        {
            open(absoluteAMD, p->file());

            Output& out = output();

            writeDocCommentOp(out, p);
            out << sp << nl << "public interface " << classNameAMD << '_' << name;
            out << " extends Ice.AMDCallback";
            out << sb;
            out << sp;
            writeDocCommentAsync(out, p, OutParam);
            out << nl << "void ice_response" << spar << paramsAMD << epar << ';';

            out << eb;

            close();
        }

        {
            open(absoluteAMDI, p->file());

            Output& out = output();

            TypePtr ret = p->returnType();

            ParamDeclList outParams;
            ParamDeclList paramList = p->parameters();
            for(ParamDeclList::const_iterator pli = paramList.begin(); pli != paramList.end(); ++pli)
            {
                if((*pli)->isOutParam())
                {
                    outParams.push_back(*pli);
                }
            }

            ExceptionList throws = p->throws();
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

            int iter;

            out << sp << nl << "final class " << classNameAMDI << '_' << name
                << " extends IceInternal.IncomingAsync implements " << classNameAMD << '_' << name;
            out << sb;

            out << sp << nl << "public " << classNameAMDI << '_' << name << "(IceInternal.Incoming in)";
            out << sb;
            out << nl << "super(in);";
            out << eb;

            out << sp << nl << "public void ice_response" << spar << paramsAMD << epar;
            out << sb;
            iter = 0;
            out << nl << "if(__validateResponse(true))";
            out << sb;
            if(ret || !outParams.empty())
            {
                out << nl << "try";
                out << sb;
                out << nl << "IceInternal.BasicStream __os = this.__startWriteParams("
                    << opFormatTypeToString(p) << ");";
                writeMarshalUnmarshalParams(out, classPkg, outParams, p, iter, true, optionalMapping, false);
                if(p->returnsClasses(false))
                {
                    out << nl << "__os.writePendingObjects();";
                }
                out << nl << "this.__endWriteParams(true);";
                out << eb;
                out << nl << "catch(Ice.LocalException __ex)";
                out << sb;
                out << nl << "__exception(__ex);";
                out << nl << "return;";
                out << eb;
            }
            else
            {
                out << nl << "__writeEmptyParams();";
            }
            out << nl << "__response();";
            out << eb;
            out << eb;

            if(!throws.empty())
            {
                out << sp << nl << "public void ice_exception(java.lang.Exception ex)";
                out << sb;
                out << nl << "try";
                out << sb;
                out << nl << "throw ex;";
                out << eb;
                for(ExceptionList::const_iterator r = throws.begin(); r != throws.end(); ++r)
                {
                    string exS = getAbsolute(*r, classPkg);
                    out << nl << "catch(" << exS << " __ex)";
                    out << sb;
                    out << nl << "if(__validateResponse(false))";
                    out << sb;
                    out << nl << "__writeUserException(__ex, " << opFormatTypeToString(p) << ");";
                    out << nl << "__response();";
                    out << eb;
                    out << eb;
                }
                out << nl << "catch(java.lang.Exception __ex)";
                out << sb;
                out << nl << "super.ice_exception(__ex);";
                out << eb;
                out << eb;
            }

            out << eb;

            close();
        }
    }
}
