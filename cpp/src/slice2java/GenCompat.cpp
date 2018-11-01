// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <GenCompat.h>
#include <Slice/Checksum.h>
#include <Slice/Util.h>
#include <IceUtil/Functional.h>
#include <IceUtil/Iterator.h>
#include <IceUtil/InputUtil.h>
#include <IceUtil/StringUtil.h>
#include <cstring>

#include <limits>

// TODO: fix this warning!
#if defined(_MSC_VER)
#   pragma warning(disable:4456) // shadow
#   pragma warning(disable:4457) // shadow
#   pragma warning(disable:4459) // shadow
#elif defined(__clang__)
#   pragma clang diagnostic ignored "-Wshadow"
#elif defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wshadow"
#endif

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

bool
writeSuppressDeprecation(Output& out, const ContainedPtr& p1, const ContainedPtr& p2 = 0)
{
    string deprecateMetadata;
    if(p1->findMetaData("deprecate", deprecateMetadata) ||
       (p2 != 0 && p2->findMetaData("deprecate", deprecateMetadata)))
    {
        out << nl << "@SuppressWarnings(\"deprecation\")";
        return true;
    }
    return false;
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
        case Builtin::KindValue:
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
            if(newLine)
            {
                out << "," << nl;
            }
            else
            {
                out << ", ";
            }
        }
    }
    if(end)
    {
        out << ")";
        out.restoreIndent();
    }
}

}

Slice::JavaCompatVisitor::JavaCompatVisitor(const string& dir) :
    JavaCompatGenerator(dir)
{
}

Slice::JavaCompatVisitor::~JavaCompatVisitor()
{
}

ParamDeclList
Slice::JavaCompatVisitor::getOutParams(const OperationPtr& op)
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
Slice::JavaCompatVisitor::getParams(const OperationPtr& op, const string& package, bool local, bool optionalMapping)
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
Slice::JavaCompatVisitor::getParamsProxy(const OperationPtr& op, const string& package, bool final, bool optionalMapping,
                                         bool internal)
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
        params.push_back(typeString + ' ' + (internal ? "iceP_" + (*q)->name() : fixKwd((*q)->name())));
    }

    return params;
}

vector<string>
Slice::JavaCompatVisitor::getInOutParams(const OperationPtr& op, const string& package, ParamDir paramType, bool /*proxy*/,
                                         bool optionalMapping, bool internal)
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
            params.push_back(typeString + ' ' + (internal ? "iceP_" + (*q)->name() : fixKwd((*q)->name())));
        }
    }

    return params;
}

vector<string>
Slice::JavaCompatVisitor::getParamsAsync(const OperationPtr& op, const string& package, bool amd, bool optionalMapping)
{
    vector<string> params = getInOutParams(op, package, InParam, !amd, optionalMapping);

    string name = op->name();
    ContainerPtr container = op->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    string classNameAsync = getUnqualified(cl, package, amd ? "AMD_" : "AMI_", '_' + name);
    params.insert(params.begin(), classNameAsync + " " + getEscapedParamName(op, "cb"));

    return params;
}

vector<string>
Slice::JavaCompatVisitor::getParamsAsyncCB(const OperationPtr& op, const string& package, bool /*amd*/, bool optionalMapping)
{
    vector<string> params;

    TypePtr ret = op->returnType();
    if(ret)
    {
        string retS = typeToString(ret, TypeModeIn, package, op->getMetaData(), true,
                                   optionalMapping && op->returnIsOptional());
        params.push_back(retS + " ret");
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
Slice::JavaCompatVisitor::getAsyncCallbackInterface(const OperationPtr& op, const string& package)
{
    TypePtr ret = op->returnType();
    ParamDeclList outParams = getOutParams(op);
    bool throws = !op->throws().empty();
    const string suffix = throws ? "UE" : "";

    if(!ret && outParams.empty())
    {
        return throws ? getUnqualified("Ice.TwowayCallbackVoidUE", package) : getUnqualified("Ice.OnewayCallback", package);
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
                    return getUnqualified(prefix + builtinAsyncCallbackTable[builtin->kind()] + suffix, package);
                }
                default:
                {
                    break;
                }
            }
        }

        return getUnqualified("Ice.TwowayCallbackArg1" + suffix, package) + "<" +
            typeToString(t, TypeModeIn, package, op->getMetaData(), true, optional) + ">";
    }
    else
    {
        ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());
        return getUnqualified(getPackage(cl) + "._Callback_" + cl->name(), package) + "_" + op->name();
    }
}

string
Slice::JavaCompatVisitor::getAsyncCallbackBaseClass(const OperationPtr& op, bool functional)
{
    assert(op->returnsData());
    TypePtr ret = op->returnType();
    ParamDeclList outParams = getOutParams(op);
    ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());
    const string package = getPackage(cl);
    bool throws = !op->throws().empty();
    const string suffix = throws ? "UE" : "";
    if(!ret && outParams.empty())
    {
        assert(throws);
        return functional ?
            "IceInternal.Functional_TwowayCallbackVoidUE" :
            "IceInternal.TwowayCallback implements " + getUnqualified("Ice.TwowayCallbackVoidUE", package);
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
                    if(functional)
                    {
                        os << "IceInternal.Functional_" << builtinAsyncCallbackTable[builtin->kind()] << suffix;
                    }
                    else
                    {
                        os << "IceInternal.TwowayCallback implements "
                           << getUnqualified(string("Ice.") + builtinAsyncCallbackTable[builtin->kind()] + suffix, package);
                    }
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
            os << "IceInternal.TwowayCallback implements " << getUnqualified("Ice.TwowayCallbackArg1", package);
        }
        os << suffix << "<" << typeToString(t, TypeModeIn, getPackage(op), op->getMetaData(), true, optional) + ">";
        return os.str();
    }
    else
    {
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
Slice::JavaCompatVisitor::getLambdaResponseCB(const OperationPtr& op, const string& package)
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
Slice::JavaCompatVisitor::getParamsAsyncLambda(const OperationPtr& op, const string& package, bool context, bool sentCB,
                                               bool optionalMapping, bool inParams, bool internal)
{
    vector<string> params;
    string contextParamName = "context";
    string responseCbParamName = "responseCb";
    string userExceptionCbParamName = "userExceptionCb";
    string exceptionCbParamName = "exceptionCb";
    string sentCbParamName = "sentCb";

    if(inParams)
    {
        params = getInOutParams(op, package, InParam, false, optionalMapping, internal);
        if(!internal)
        {
            contextParamName = getEscapedParamName(op, contextParamName);
            responseCbParamName = getEscapedParamName(op, responseCbParamName);
            userExceptionCbParamName = getEscapedParamName(op, userExceptionCbParamName);
            exceptionCbParamName = getEscapedParamName(op, exceptionCbParamName);
            sentCbParamName = getEscapedParamName(op, sentCbParamName);
        }
    }

    if(context)
    {
        params.push_back("java.util.Map<String, String> " + contextParamName);
    }

    params.push_back(getLambdaResponseCB(op, package) + " " + responseCbParamName);

    if(!op->throws().empty())
    {
        params.push_back("IceInternal.Functional_GenericCallback1<" + getUnqualified("Ice.UserException", package) + "> " +
                         userExceptionCbParamName);
    }

    params.push_back("IceInternal.Functional_GenericCallback1<" + getUnqualified("Ice.Exception", package) + "> " +
                     exceptionCbParamName);

    if(sentCB)
    {
        params.push_back("IceInternal.Functional_BoolCallback " + sentCbParamName);
    }

    return params;
}

vector<string>
Slice::JavaCompatVisitor::getArgsAsyncLambda(const OperationPtr& op, const string& /*package*/, bool context, bool sentCB)
{
    vector<string> args = getInOutArgs(op, InParam);

    // "internal" is always false for this function

    const string contextParamName = getEscapedParamName(op, "context");
    const string responseCbParamName = getEscapedParamName(op, "responseCb");
    const string userExceptionCbParamName = getEscapedParamName(op, "userExceptionCb");
    const string exceptionCbParamName = getEscapedParamName(op, "exceptionCb");
    const string sentCbParamName = getEscapedParamName(op, "sentCb");

    args.push_back(context ? contextParamName : "null");
    args.push_back(context ? "true" : "false");
    args.push_back("false"); // synchronous param
    args.push_back(responseCbParamName);
    if(!op->throws().empty())
    {
        args.push_back(userExceptionCbParamName);
    }
    args.push_back(exceptionCbParamName);
    args.push_back(sentCB ? sentCbParamName : "null");
    return args;
}

vector<string>
Slice::JavaCompatVisitor::getArgs(const OperationPtr& op)
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
Slice::JavaCompatVisitor::getInOutArgs(const OperationPtr& op, ParamDir paramType, bool internal)
{
    vector<string> args;

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if((*q)->isOutParam() == (paramType == OutParam))
        {
            args.push_back(internal ? "iceP_" + (*q)->name() : fixKwd((*q)->name()));
        }
    }

    return args;
}

vector<string>
Slice::JavaCompatVisitor::getArgsAsync(const OperationPtr& op)
{
    vector<string> args = getInOutArgs(op, InParam);
    args.insert(args.begin(), getEscapedParamName(op, "cb"));
    return args;
}

vector<string>
Slice::JavaCompatVisitor::getArgsAsyncCB(const OperationPtr& op)
{
    vector<string> args;

    TypePtr ret = op->returnType();
    if(ret)
    {
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
        if((builtin && (builtin->kind() == Builtin::KindObject || builtin->kind() == Builtin::KindValue)) ||
           ClassDeclPtr::dynamicCast(ret))
        {
            args.push_back("ret.value");
        }
        else
        {
            args.push_back("ret");
        }
    }

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if((*q)->isOutParam())
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast((*q)->type());
            if((builtin && (builtin->kind() == Builtin::KindObject || builtin->kind() == Builtin::KindValue)) ||
               ClassDeclPtr::dynamicCast((*q)->type()))
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
Slice::JavaCompatVisitor::writeMarshalUnmarshalParams(Output& out, const string& package, const ParamDeclList& params,
                                                      const OperationPtr& op, int& iter, bool marshal, bool optionalMapping,
                                                      bool internal, const string& customRetName, bool dispatch)
{
    string stream = "";
    if(!internal)
    {
        stream = marshal ? "ostr_" : "istr_";
    }

    ParamDeclList optionals;
    for(ParamDeclList::const_iterator pli = params.begin(); pli != params.end(); ++pli)
    {
        if((*pli)->optional())
        {
            optionals.push_back(*pli);
        }
        else
        {
            string paramName = internal ? "iceP_" + (*pli)->name() : fixKwd((*pli)->name());
            bool holder = marshal == dispatch;
            string patchParams;
            if(!marshal)
            {
                patchParams = paramName;
            }
            writeMarshalUnmarshalCode(out, package, (*pli)->type(), OptionalNone, false, 0, paramName, marshal,
                                      iter, holder, stream, (*pli)->getMetaData(), patchParams);
        }
    }

    string retName = customRetName;
    if(retName.empty())
    {
        retName = internal ? "ret" : "ret_";
    }

    TypePtr ret;
    bool returnsObject = false;

    if(op && op->returnType())
    {
        ret = op->returnType();
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
        ClassDeclPtr cl = ClassDeclPtr::dynamicCast(ret);
        returnsObject = (builtin && (builtin->kind() == Builtin::KindObject ||
                                     builtin->kind() == Builtin::KindValue)) || cl;
        const bool optional = optionalMapping && op->returnIsOptional();

        string retS = typeToString(ret, TypeModeReturn, package, op->getMetaData(), true, optional);
        bool holder = false;

        if(!marshal)
        {
            if(optional)
            {
                out << nl << retS << " " << retName << " = new " << retS << "();";
            }
            else if(returnsObject)
            {
                out << nl << retS << "Holder " << retName << " = new " << retS << "Holder();";
                holder = true;
            }
            else if(StructPtr::dynamicCast(ret))
            {
                out << nl << retS << " " << retName << " = null;";
            }
            else
            {
                out << nl << retS << " " << retName << ";";
            }
        }

        if(!op->returnIsOptional())
        {
            writeMarshalUnmarshalCode(out, package, ret, OptionalNone, false, 0, retName, marshal, iter, holder,
                                      stream, op->getMetaData());
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
                                      retName, marshal, iter, false, stream, op->getMetaData());
            checkReturnType = false;
        }

        const bool holder = dispatch && (*pli)->isOutParam() && !optionalMapping;

        writeMarshalUnmarshalCode(out, package, (*pli)->type(),
                                  (*pli)->isOutParam() ? OptionalOutParam : OptionalInParam, optionalMapping,
                                  (*pli)->tag(), internal ? "iceP_" + (*pli)->name() : fixKwd((*pli)->name()),
                                  marshal, iter, holder, stream, (*pli)->getMetaData());
    }

    if(checkReturnType)
    {
        writeMarshalUnmarshalCode(out, package, ret, OptionalReturnParam, optionalMapping, op->returnTag(), retName,
                                  marshal, iter, false, stream, op->getMetaData());
    }
}

void
Slice::JavaCompatVisitor::writeThrowsClause(const string& package, const ExceptionList& throws, const OperationPtr& op)
{
    Output& out = output();

    if(op && (op->hasMetaData("java:UserException") || op->hasMetaData("UserException")))
    {
        ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());
        out.inc();
        out << nl << "throws " << getUnqualified("Ice.UserException", getPackage(cl));
        out.dec();
    }
    else if(throws.size() > 0)
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
            out << getUnqualified(*r, package);
            count++;
        }
        out.restoreIndent();
        out.dec();
    }
}

void
Slice::JavaCompatVisitor::writeMarshalDataMember(Output& out, const string& package, const DataMemberPtr& member,
                                                 int& iter, bool forStruct)
{
    if(!member->optional())
    {
        string stream = forStruct ? "" : "ostr_";
        string memberName = fixKwd(member->name());
        if(forStruct)
        {
            memberName = "this." + memberName;
        }

        writeMarshalUnmarshalCode(out, package, member->type(), OptionalNone, false, 0, memberName,
                                  true, iter, false, stream, member->getMetaData());
    }
    else
    {
        assert(!forStruct);
        out << nl << "if(_" << member->name() << " && ostr_.writeOptional(" << member->tag() << ", "
            << getOptionalFormat(member->type()) << "))";
        out << sb;
        writeMarshalUnmarshalCode(out, package, member->type(), OptionalMember, false, 0, fixKwd(member->name()), true,
                                  iter, false, "ostr_", member->getMetaData());
        out << eb;
    }
}

void
Slice::JavaCompatVisitor::writeUnmarshalDataMember(Output& out, const string& package, const DataMemberPtr& member,
                                                   int& iter, bool needPatcher, int& patchIter, bool forStruct)
{
    string patchParams;
    if(needPatcher)
    {
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(member->type());
        if((builtin && (builtin->kind() == Builtin::KindObject || builtin->kind() == Builtin::KindValue)) ||
           ClassDeclPtr::dynamicCast(member->type()))
        {
            ostringstream ostr;
            ostr << "new Patcher(" << patchIter++ << ')';
            patchParams = ostr.str();
        }
    }

    if(!member->optional())
    {
        string stream = forStruct ? "" : "istr_";
        string memberName = fixKwd(member->name());
        if(forStruct)
        {
            memberName = "this." + memberName;
        }

        writeMarshalUnmarshalCode(out, package, member->type(), OptionalNone, false, 0, memberName, false,
                                  iter, false, stream, member->getMetaData(), patchParams);
    }
    else
    {
        assert(!forStruct);
        out << nl << "if(_" << member->name() << " = istr_.readOptional(" << member->tag() << ", "
            << getOptionalFormat(member->type()) << "))";
        out << sb;
        writeMarshalUnmarshalCode(out, package, member->type(), OptionalMember, false, 0, fixKwd(member->name()), false,
                                  iter, false, "istr_", member->getMetaData(), patchParams);
        out << eb;
    }
}

void
Slice::JavaCompatVisitor::writePatcher(Output& out, const string& package, const DataMemberList& classMembers,
                                       const DataMemberList& optionalMembers)
{
    out << sp << nl << "private class Patcher implements " << getUnqualified("Ice.ReadValueCallback", package);
    out << sb;
    if(classMembers.size() > 1)
    {
        out << sp << nl << "Patcher(int member)";
        out << sb;
        out << nl << "_member = member;";
        out << eb;
    }

    out << sp << nl << "public void" << nl << "valueReady(" << getUnqualified("Ice.Object", package) << " v)";
    out << sb;
    if(classMembers.size() > 1)
    {
        out << nl << "switch(_member)";
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
            assert(b->kind() == Builtin::KindObject || b->kind() == Builtin::KindValue);
        }

        if(classMembers.size() > 1)
        {
            out.dec();
            out << nl << "case " << memberCount << ":";
            out.inc();
            if(b)
            {
                out << nl << "_typeId = " << getUnqualified("Ice.ObjectImpl", package) << ".ice_staticId();";
            }
            else
            {
                out << nl << "_typeId = \"" << (*d)->type()->typeId() << "\";";
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
        if(b && b->kind() != Builtin::KindObject && b->kind() != Builtin::KindValue)
        {
            continue;
        }

        TypePtr paramType = (*d)->type();
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(paramType);
        if((builtin && (builtin->kind() == Builtin::KindObject || builtin->kind() == Builtin::KindValue)) ||
           ClassDeclPtr::dynamicCast(paramType))
        {

            if(classMembers.size() > 1)
            {
                out.dec();
                out << nl << "case " << memberCount << ":";
                out.inc();
                if(b)
                {
                    out << nl << "_typeId = " << getUnqualified("Ice.ObjectImpl", package) << ".ice_staticId();";
                }
                else
                {
                    out << nl << "_typeId = \"" << (*d)->type()->typeId() << "\";";
                }
            }

            string capName = (*d)->name();
            capName[0] = static_cast<char>(toupper(static_cast<unsigned char>(capName[0])));

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
        out << nl << "return _typeId;";
    }
    else
    {
        out << nl << "return \"" << (*classMembers.begin())->type()->typeId() << "\";";
    }
    out << eb;

    if(classMembers.size() > 1)
    {
        out << sp << nl << "private int _member;";
        out << nl << "private String _typeId;";
    }

    out << eb;
}

void
Slice::JavaCompatVisitor::writeDispatchAndMarshalling(Output& out, const ClassDefPtr& p)
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

    out << sp << nl << "private static final String[] _ids =";
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
    out << nl << "return java.util.Arrays.binarySearch(_ids, s) >= 0;";
    out << eb;

    out << sp << nl << "public boolean ice_isA(String s, " << getUnqualified("Ice.Current", package) << " current)";
    out << sb;
    out << nl << "return java.util.Arrays.binarySearch(_ids, s) >= 0;";
    out << eb;

    out << sp << nl << "public String[] ice_ids()";
    out << sb;
    out << nl << "return _ids;";
    out << eb;

    out << sp << nl << "public String[] ice_ids(" << getUnqualified("Ice.Current", package) << " current)";
    out << sb;
    out << nl << "return _ids;";
    out << eb;

    out << sp << nl << "public String ice_id()";
    out << sb;
    out << nl << "return _ids[" << scopedPos << "];";
    out << eb;

    out << sp << nl << "public String ice_id(" << getUnqualified("Ice.Current", package) << " current)";
    out << sb;
    out << nl << "return _ids[" << scopedPos << "];";
    out << eb;

    out << sp << nl << "public static String ice_staticId()";
    out << sb;
    out << nl << "return _ids[" << scopedPos << "];";
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
            writeSuppressDeprecation(out, op, cl);
            out << nl << "public final "
                << typeToString(ret, TypeModeReturn, package, op->getMetaData(), true,
                                optionalMapping && op->returnIsOptional())
                << ' ' << opName << spar << params << epar;
            writeThrowsClause(package, throws, op);

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

        string opName = op->name();
        out << sp;
        writeSuppressDeprecation(out, op);
        out << nl << "public static boolean _iceD_" << opName << '(' << name
            << " obj, IceInternal.Incoming inS, " << getUnqualified("Ice.Current", package) << " current)";
        out.inc();
        out << nl << "throws " << getUnqualified("Ice.UserException", package);
        out.dec();

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

            int iter;

            out << nl << "_iceCheckMode(" << sliceModeToIceMode(op->mode()) << ", current.mode);";

            if(!inParams.empty())
            {
                //
                // Unmarshal 'in' parameters.
                //
                out << nl << getUnqualified("Ice.InputStream", package) << " istr = inS.startReadParams();";
                for(ParamDeclList::const_iterator pli = inParams.begin(); pli != inParams.end(); ++pli)
                {
                    TypePtr paramType = (*pli)->type();
                    string paramName = "iceP_" + (*pli)->name();
                    string typeS = typeToString(paramType, TypeModeIn, package, (*pli)->getMetaData(),
                                                true, (*pli)->optional());
                    if((*pli)->optional())
                    {
                        out << nl << typeS << ' ' << paramName << " = new " << typeS << "();";
                    }
                    else
                    {
                        BuiltinPtr builtin = BuiltinPtr::dynamicCast(paramType);
                        if((builtin && (builtin->kind() == Builtin::KindObject ||
                                        builtin->kind() == Builtin::KindValue)) || ClassDeclPtr::dynamicCast(paramType))
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
                writeMarshalUnmarshalParams(out, package, inParams, 0, iter, false, true, true, "", true);
                if(op->sendsClasses(false))
                {
                    out << nl << "istr.readPendingValues();";
                }
                out << nl << "inS.endReadParams();";
            }
            else
            {
                out << nl << "inS.readEmptyParams();";
            }
            if(op->format() != DefaultFormat)
            {
                out << nl << "inS.setFormat(" << opFormatTypeToString(op) << ");";
            }

            //
            // Declare 'out' parameters.
            //
            for(ParamDeclList::const_iterator pli = outParams.begin(); pli != outParams.end(); ++pli)
            {
                string typeS = typeToString((*pli)->type(), TypeModeOut, package, (*pli)->getMetaData(), true,
                                            optionalMapping && (*pli)->optional());
                out << nl << typeS << " iceP_" << (*pli)->name() << " = new " << typeS << "();";
            }

            //
            // Call on the servant.
            //
            out << nl;
            if(ret)
            {
                string retS = typeToString(ret, TypeModeReturn, package, opMetaData, true,
                                           optionalMapping && op->returnIsOptional());
                out << retS << " ret = ";
            }
            out << "obj." << fixKwd(opName) << '(';
            for(ParamDeclList::const_iterator pli = inParams.begin(); pli != inParams.end(); ++pli)
            {
                TypePtr paramType = (*pli)->type();
                out << "iceP_" << (*pli)->name();
                if(!(*pli)->optional())
                {
                    BuiltinPtr builtin = BuiltinPtr::dynamicCast(paramType);
                    if((builtin && (builtin->kind() == Builtin::KindObject ||
                                    builtin->kind() == Builtin::KindValue)) || ClassDeclPtr::dynamicCast(paramType))
                    {
                        out << ".value";
                    }
                }
                out << ", ";
            }
            for(ParamDeclList::const_iterator pli = outParams.begin(); pli != outParams.end(); ++pli)
            {
                out << "iceP_" << (*pli)->name() << ", ";
            }
            out << "current);";

            //
            // Marshal 'out' parameters and return value.
            //
            if(!outParams.empty() || ret)
            {
                out << nl << getUnqualified("Ice.OutputStream", package) << " ostr = inS.startWriteParams();";
                writeMarshalUnmarshalParams(out, package, outParams, op, iter, true, optionalMapping, true, "", true);
                if(op->returnsClasses(false))
                {
                    out << nl << "ostr.writePendingValues();";
                }
                out << nl << "inS.endWriteParams();";
            }
            else
            {
                out << nl << "inS.writeEmptyParams();";
            }
            out << nl << "return true;";

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

            out << nl << "_iceCheckMode(" << sliceModeToIceMode(op->mode()) << ", current.mode);";

            if(!inParams.empty())
            {
                //
                // Unmarshal 'in' parameters.
                //
                out << nl << getUnqualified("Ice.InputStream", package) << " istr = inS.startReadParams();";
                iter = 0;
                for(ParamDeclList::const_iterator pli = inParams.begin(); pli != inParams.end(); ++pli)
                {
                    TypePtr paramType = (*pli)->type();
                    string paramName = "iceP_" + (*pli)->name();
                    string typeS = typeToString(paramType, TypeModeIn, package, (*pli)->getMetaData(),
                                                true, (*pli)->optional());
                    if((*pli)->optional())
                    {
                        out << nl << typeS << ' ' << paramName << " = new " << typeS << "();";
                    }
                    else
                    {
                        BuiltinPtr builtin = BuiltinPtr::dynamicCast(paramType);
                        if((builtin && (builtin->kind() == Builtin::KindObject ||
                                        builtin->kind() == Builtin::KindValue)) || ClassDeclPtr::dynamicCast(paramType))
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
                writeMarshalUnmarshalParams(out, package, inParams, 0, iter, false, true, true, "", true);
                if(op->sendsClasses(false))
                {
                    out << nl << "istr.readPendingValues();";
                }
                out << nl << "inS.endReadParams();";
            }
            else
            {
                out << nl << "inS.readEmptyParams();";
            }
            if(op->format() != DefaultFormat)
            {
                out << nl << "inS.setFormat(" << opFormatTypeToString(op) << ");";
            }

            //
            // Call on the servant.
            //
            string classNameAMD = "AMD_" + p->name();
            out << nl << classNameAMD << '_' << opName << " cb = new _" << classNameAMD << '_' << opName
                << "(inS);";
            out << nl << "obj." << (amd ? opName + "_async" : fixKwd(opName)) << (amd ? "(cb, " : "(");
            for(ParamDeclList::const_iterator pli = inParams.begin(); pli != inParams.end(); ++pli)
            {
                TypePtr paramType = (*pli)->type();
                out << "iceP_" << (*pli)->name();
                if(!(*pli)->optional())
                {
                    BuiltinPtr builtin = BuiltinPtr::dynamicCast(paramType);
                    if((builtin && (builtin->kind() == Builtin::KindObject || builtin->kind() == Builtin::KindValue)) ||
                       ClassDeclPtr::dynamicCast(paramType))
                    {
                        out << ".value";
                    }
                }
                out << ", ";
            }
            out << "current);";
            out << nl << "return false;";

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

        out << sp << nl << "private final static String[] _all =";
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
        out << nl << "public boolean _iceDispatch(IceInternal.Incoming in, Ice.Current current)";
        out.inc();
        out << nl << "throws Ice.UserException";
        out.dec();
        out << sb;
        out << nl << "int pos = java.util.Arrays.binarySearch(_all, current.operation);";
        out << nl << "if(pos < 0)";
        out << sb;
        out << nl << "throw new " << getUnqualified("Ice.OperationNotExistException", package)
            << "(current.id, current.facet, current.operation);";
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
                out << nl << "return _iceD_ice_id(this, in, current);";
            }
            else if(opName == "ice_ids")
            {
                out << nl << "return _iceD_ice_ids(this, in, current);";
            }
            else if(opName == "ice_isA")
            {
                out << nl << "return _iceD_ice_isA(this, in, current);";
            }
            else if(opName == "ice_ping")
            {
                out << nl << "return _iceD_ice_ping(this, in, current);";
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
                            out << nl << "return _iceD_" << opName << "(this, in, current);";
                        }
                        else
                        {
                            string base;
                            if(cl->isInterface())
                            {
                                base = getUnqualified(cl, package, "_", "Disp");
                            }
                            else
                            {
                                base = getUnqualified(cl, package);
                            }
                            out << nl << "return " << base << "._iceD_" << opName << "(this, in, current);";
                        }
                        break;
                    }
                }
            }
            out << eb;
        }
        out << eb;
        out << sp << nl << "assert(false);";
        out << nl << "throw new " << getUnqualified("Ice.OperationNotExistException", package)
            << "(current.id, current.facet, current.operation);";
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
            out << sp << nl << "private final static int[] _operationAttributes =";
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
            out << nl << "int pos = java.util.Arrays.binarySearch(_all, operation);";
            out << nl << "if(pos < 0)";
            out << sb;
            out << nl << "return -1;";
            out << eb;
            out << sp << nl << "return _operationAttributes[pos];";
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
        out << sp << nl << "public " << getUnqualified("Ice.SlicedData", package) << " ice_getSlicedData()";
        out << sb;
        out << nl << "return _iceSlicedData;";
        out << eb;

        out << sp << nl << "public void _iceWrite(" << getUnqualified("Ice.OutputStream", package) << " ostr)";
        out << sb;
        out << nl << "ostr.startValue(_iceSlicedData);";
        out << nl << "_iceWriteImpl(ostr);";
        out << nl << "ostr.endValue();";
        out << eb;

        out << sp << nl << "public void _iceRead(" << getUnqualified("Ice.InputStream", package) << " istr)";
        out << sb;
        out << nl << "istr.startValue();";
        out << nl << "_iceReadImpl(istr);";
        out << nl << "_iceSlicedData = istr.endValue(true);";
        out << eb;
    }

    out << sp << nl << "protected void _iceWriteImpl(" << getUnqualified("Ice.OutputStream", package) << " ostr_)";
    out << sb;
    out << nl << "ostr_.startSlice(ice_staticId(), " << p->compactId() << (!base ? ", true" : ", false") << ");";
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
    out << nl << "ostr_.endSlice();";
    if(base)
    {
        out << nl << "super._iceWriteImpl(ostr_);";
    }
    out << eb;

    DataMemberList classMembers = p->classDataMembers();
    DataMemberList allClassMembers = p->allClassDataMembers();

    if(classMembers.size() != 0)
    {
        writePatcher(out, package, classMembers, optionalMembers);
    }

    out << sp << nl << "protected void _iceReadImpl(" << getUnqualified("Ice.InputStream", package) << " istr_)";
    out << sb;
    out << nl << "istr_.startSlice();";

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
    out << nl << "istr_.endSlice();";
    if(base)
    {
        out << nl << "super._iceReadImpl(istr_);";
    }
    out << eb;

    if(preserved && !basePreserved)
    {
        out << sp << nl << "protected " << getUnqualified("Ice.SlicedData", package) << " _iceSlicedData;";
    }
}

void
Slice::JavaCompatVisitor::writeConstantValue(Output& out, const TypePtr& type, const SyntaxTreeBasePtr& valueType,
                                       const string& value, const string& package)
{
    ConstPtr constant = ConstPtr::dynamicCast(valueType);
    if(constant)
    {
        out << getUnqualified(constant, package) << ".value";
    }
    else
    {
        BuiltinPtr bp;
        if((bp = BuiltinPtr::dynamicCast(type)))
        {
            switch(bp->kind())
            {
                case Builtin::KindString:
                {
                    out << "\"" << toStringLiteral(value, "\b\f\n\r\t", "", ShortUCN, 0) << "\"";
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
                case Builtin::KindValue:
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
        else if(EnumPtr::dynamicCast(type))
        {
            EnumeratorPtr lte = EnumeratorPtr::dynamicCast(valueType);
            assert(lte);
            out << getUnqualified(lte, package);
        }
        else
        {
            out << value;
        }
    }
}

void
Slice::JavaCompatVisitor::writeDataMemberInitializers(Output& out, const DataMemberList& members, const string& package)
{
    for(DataMemberList::const_iterator p = members.begin(); p != members.end(); ++p)
    {
        TypePtr t = (*p)->type();
        if((*p)->defaultValueType())
        {
            if((*p)->optional())
            {
                string capName = (*p)->name();
                capName[0] = static_cast<char>(toupper(static_cast<unsigned char>(capName[0])));
                out << nl << "set" << capName << '(';
                writeConstantValue(out, t, (*p)->defaultValueType(), (*p)->defaultValue(), package);
                out << ");";
            }
            else
            {
                out << nl << "this." << fixKwd((*p)->name()) << " = ";
                writeConstantValue(out, t, (*p)->defaultValueType(), (*p)->defaultValue(), package);
                out << ';';
            }
        }
        else
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(t);
            if(builtin && builtin->kind() == Builtin::KindString)
            {
                out << nl << "this." << fixKwd((*p)->name()) << " = \"\";";
            }

            EnumPtr en = EnumPtr::dynamicCast(t);
            if(en)
            {
                string firstEnum = fixKwd(en->enumerators().front()->name());
                out << nl << "this." << fixKwd((*p)->name()) << " = " << getUnqualified(en, package) << '.' << firstEnum
                    << ';';
            }

            StructPtr st = StructPtr::dynamicCast(t);
            if(st)
            {
                string memberType = typeToString(st, TypeModeMember, package, (*p)->getMetaData());
                out << nl << "this." << fixKwd((*p)->name()) << " = new " << memberType << "();";
            }
        }
    }
}

StringList
Slice::JavaCompatVisitor::splitComment(const ContainedPtr& p)
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
Slice::JavaCompatVisitor::writeDocComment(Output& out, const ContainedPtr& p, const string& deprecateReason,
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
Slice::JavaCompatVisitor::writeDocComment(Output& out, const string& deprecateReason, const string& summary)
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
Slice::JavaCompatVisitor::writeDocCommentOp(Output& out, const OperationPtr& p)
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
        if((*i).empty())
        {
            out << nl << " *";
        }
        else if((*i)[0] == '@')
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
Slice::JavaCompatVisitor::writeDocCommentAsync(Output& out, const OperationPtr& p, ParamDir paramType,
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
                    out << nl << " * @param ret (return value)" << i->substr(returnTag.length());
                }
            }
            else
            {
                if((*i).empty())
                {
                    out << nl << " *";
                }
                else if((*i)[0] == '@')
                {
                    doneReturn = true;
                }
                else
                {
                    out << nl << *i;
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
            if((*i).empty())
            {
                out << nl << " *";
            }
            else if((*i)[0] == '@')
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
        //
        // Print @throws tags
        //
        const string throwsTag = "@throws";
        bool found = false;
        for(StringList::const_iterator i = lines.begin(); i != lines.end(); ++i)
        {
            if(found && i->find("@") == 0)
            {
                found = false; // reset
            }

            if(!found)
            {
                if(i->find(throwsTag) != string::npos)
                {
                    found = true;
                }
            }

            if(found)
            {
                if((*i).empty())
                {
                    out << nl << " *";
                }
                else
                {
                    out << nl << " * " << *i;
                }
            }
        }

        if(!deprecateReason.empty())
        {
            out << nl << " * @deprecated " << deprecateReason;
        }
    }

    out << nl << " **/";
}

void
Slice::JavaCompatVisitor::writeDocCommentAMI(Output& out, const OperationPtr& p, ParamDir paramType,
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
        if((*i).empty())
        {
            out << nl << " *";
        }
        else if((*i)[0] == '@')
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
        out << nl << " * @param result The asynchronous result object.";
        //
        // Print @return, @throws, and @see tags.
        //
        const string returnTag = "@return";
        const string throwsTag = "@throws";
        const string seeTag = "@see";
        bool found = false;
        for(StringList::const_iterator i = lines.begin(); i != lines.end(); ++i)
        {
            if(found && i->find("@") == 0)
            {
                found = false; // reset
            }

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
                if((*i).empty())
                {
                    out << nl << " *";
                }
                else
                {
                    out << nl << " * " << *i;
                }
            }
        }
    }

    out << nl << " **/";
}

void
Slice::JavaCompatVisitor::writeDocCommentParam(Output& out, const OperationPtr& p, ParamDir paramType, bool cb)
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
        out << nl << " * @param cb The callback object for the operation.";
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
                    if(i == lines.end())
                    {
                        break;
                    }
                    j = i++;
                    while(j != lines.end())
                    {
                        if((*j)[0] != '@')
                        {
                            i = j;
                            if((*j).empty())
                            {
                                out << nl << " *";
                            }
                            else
                            {
                                out << nl << " * " << *j;
                            }
                            j++;
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

Slice::GenCompat::GenCompat(const string& /*name*/, const string& base, const vector<string>& includePaths,
                            const string& dir, bool tie) :
    _base(base),
    _includePaths(includePaths),
    _dir(dir),
    _tie(tie)
{
}

Slice::GenCompat::~GenCompat()
{
}

void
Slice::GenCompat::generate(const UnitPtr& p)
{
    JavaGenerator::validateMetaData(p);

    OpsVisitor opsVisitor(_dir);
    p->visit(&opsVisitor, false);

    PackageVisitor packageVisitor(_dir);
    p->visit(&packageVisitor, false);

    TypesVisitor typesVisitor(_dir);
    p->visit(&typesVisitor, false);

    CompactIdVisitor compactIdVisitor(_dir);
    p->visit(&compactIdVisitor, false);

    HolderVisitor holderVisitor(_dir);
    p->visit(&holderVisitor, false);

    HelperVisitor helperVisitor(_dir);
    p->visit(&helperVisitor, false);

    ProxyVisitor proxyVisitor(_dir);
    p->visit(&proxyVisitor, false);

    DispatcherVisitor dispatcherVisitor(_dir, _tie);
    p->visit(&dispatcherVisitor, false);

    AsyncVisitor asyncVisitor(_dir);
    p->visit(&asyncVisitor, false);
}

void
Slice::GenCompat::generateImpl(const UnitPtr& p)
{
    ImplVisitor implVisitor(_dir);
    p->visit(&implVisitor, false);
}

void
Slice::GenCompat::generateImplTie(const UnitPtr& p)
{
    ImplTieVisitor implTieVisitor(_dir);
    p->visit(&implTieVisitor, false);
}

void
Slice::GenCompat::writeChecksumClass(const string& checksumClass, const string& dir, const ChecksumMap& m)
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

Slice::GenCompat::OpsVisitor::OpsVisitor(const string& dir) :
    JavaCompatVisitor(dir)
{
}

bool
Slice::GenCompat::OpsVisitor::visitClassDefStart(const ClassDefPtr& p)
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
Slice::GenCompat::OpsVisitor::writeOperations(const ClassDefPtr& p, bool noCurrent)
{
    string name = p->name();
    ClassList bases = p->bases();
    string package = getPackage(p);
    string opIntfName = "Operations";
    if(noCurrent || p->isLocal())
    {
        opIntfName += "NC";
    }
    string absolute = getUnqualified(p, "", "_", opIntfName);

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
                out << getUnqualified(*q, package, "_", opIntfName);
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
        string currentParamName = getEscapedParamName(op, "current");

        if(!noCurrent && !p->isLocal())
        {
            extraCurrent = "@param " + currentParamName + " The Current object for the invocation.";
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
            out << getUnqualified("Ice.Current", package) + " " + currentParamName;
        }
        out << epar;
        writeThrowsClause(package, throws, op);
        out << ';';
    }

    out << eb;

    close();
}

Slice::GenCompat::PackageVisitor::PackageVisitor(const string& dir) :
    JavaCompatVisitor(dir)
{
}

bool
Slice::GenCompat::PackageVisitor::visitModuleStart(const ModulePtr& p)
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

Slice::GenCompat::TypesVisitor::TypesVisitor(const string& dir) :
    JavaCompatVisitor(dir)
{
}

bool
Slice::GenCompat::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = p->name();
    ClassList bases = p->bases();
    ClassDefPtr baseClass;
    if(!bases.empty() && !bases.front()->isInterface())
    {
        baseClass = bases.front();
    }

    string package = getPackage(p);
    string absolute = getUnqualified(p);
    DataMemberList members = p->dataMembers();
    DataMemberList allDataMembers = p->allDataMembers();

    open(absolute, p->file());

    Output& out = output();

    //
    // Check for java:implements metadata.
    //
    const StringList metaData = p->getMetaData();
    static const string prefix = "java:implements:";
    StringList implements;
    for(StringList::const_iterator q = metaData.begin(); q != metaData.end(); ++q)
    {
        if(q->find(prefix) == 0)
        {
            implements.push_back(q->substr(prefix.size()));
        }
    }

    //
    // Slice interfaces map to Java interfaces.
    //
    out << sp;
    writeDocComment(out, p, getDeprecateReason(p, 0, p->isInterface() ? "interface" : "class"));
    if(p->isInterface())
    {
        out << nl << "public interface " << fixKwd(name);
        ClassList::const_iterator q = bases.begin();
        StringList::const_iterator r = implements.begin();
        if(!p->isLocal() || !bases.empty() || !implements.empty())
        {
            out << " extends ";
        }
        out.useCurrentPosAsIndent();
        if(!p->isLocal())
        {
            out << '_' << name << "Operations, _" << name << "OperationsNC";
            if(bases.empty())
            {
                out << "," << nl << getUnqualified("Ice.Object", package);
            }
        }
        else if(q != bases.end())
        {
            out << getUnqualified(*q++, package);
        }
        else if(r != implements.end())
        {
            out << *r++;
        }

        for(;q != bases.end(); ++q)
        {
            out << ',' << nl << getUnqualified(*q, package);
        }
        for(; r != implements.end(); ++r)
        {
            out << ',' << nl << *r;
        }
        out.restoreIndent();
    }
    else
    {
        out << nl << "public ";
        if(p->allOperations().size() > 0 || !implements.empty()) // Don't use isAbstract() - see bug 3739
        {
            out << "abstract ";
        }
        out << "class " << fixKwd(name);
        out.useCurrentPosAsIndent();

        if(baseClass)
        {
            out << " extends " << getUnqualified(baseClass, package);
            bases.pop_front();
        }
        else if(!p->isLocal())
        {
            out << " extends " << getUnqualified("Ice.ObjectImpl", package);
        }
        else
        {
            implements.push_back("java.lang.Cloneable");
        }

        if(p->isAbstract() && !p->isLocal())
        {
            implements.push_back("_" + name + "Operations");
            implements.push_back("_" + name + "OperationsNC");
        }
        for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
        {
            implements.push_back(getUnqualified(*q, package));
        }

        if(!implements.empty())
        {
            if(baseClass || !p->isLocal())
            {
                out << nl;
            }

            out << " implements ";
            out.useCurrentPosAsIndent();
            for(StringList::const_iterator q = implements.begin(); q != implements.end(); ++q)
            {
                if(q != implements.begin())
                {
                    out << ',' << nl;
                }
                out << *q;
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
            writeThrowsClause(package, throws, op);

            out << ';';

            //
            // Generate asynchronous API for local operations marked with "async-oneway" metadata.
            //
            if(p->hasMetaData("async-oneway") || op->hasMetaData("async-oneway"))
            {
                vector<string> inParams = getInOutParams(op, package, InParam, true, true);

                out << sp;
                writeDocCommentAMI(out, op, InParam);
                out << nl;
                if(!p->isInterface())
                {
                    out << "public abstract ";
                }
                out << getUnqualified("Ice.AsyncResult", package) << " begin_" << opname << spar << inParams << epar << ';';

                out << sp;
                writeDocCommentAMI(out, op, InParam, "@param " + getEscapedParamName(op, "cb") + " A generic callback.");
                out << nl;
                if(!p->isInterface())
                {
                    out << "public abstract ";
                }
                out << getUnqualified("Ice.AsyncResult", package) << " begin_" << opname << spar << inParams
                    << ("Ice.Callback " + getEscapedParamName(op, "cb")) << epar << ';';

                out << sp;
                writeDocCommentAMI(out, op, InParam, "@param " + getEscapedParamName(op, "cb") + " A typed callback.");
                out << nl;
                if(!p->isInterface())
                {
                    out << "public abstract ";
                }
                string cb = "Callback_" + name + "_" + opname + " " + getEscapedParamName(op, "cb");
                out << getUnqualified("Ice.AsyncResult", package) << " begin_" << opname << spar << inParams << cb << epar
                    << ';';

                out << sp;
                writeDocCommentAMI(out, op, InParam,
                                   "@param " + getEscapedParamName(op, "responseCb") + " The response callback.",
                                   "@param " + getEscapedParamName(op, "exceptionCb") + " The exception callback.",
                                   "@param " + getEscapedParamName(op, "sentCb") + " The sent callback.");
                out << nl;
                if(!p->isInterface())
                {
                    out << "public abstract ";
                }
                out << getUnqualified("Ice.AsyncResult", package) << " begin_" << opname;
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

                out << retS << " end_" << opname << spar << outParams
                    << (getUnqualified("Ice.AsyncResult", package) + " " + getEscapedParamName(op, "result")) << epar << ';';
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
                        string memberType = typeToString((*d)->type(), TypeModeMember, package, (*d)->getMetaData(),
                                                         true, false, p->isLocal());
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
                string memberType = typeToString((*d)->type(), TypeModeMember, package, (*d)->getMetaData(), true,
                                                 false, p->isLocal());
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
                    capName[0] = static_cast<char>(toupper(static_cast<unsigned char>(capName[0])));
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
        out << nl << "private static class _F implements " << getUnqualified("Ice.ValueFactory", package);
        out << sb;
        out << nl << "public " << getUnqualified("Ice.Object", package) << " create(String type)";
        out << sb;
        out << nl << "assert(type.equals(ice_staticId()));";
        out << nl << "return new " << fixKwd(name) << "();";
        out << eb;
        out << eb;
        out << nl << "private static " << getUnqualified("Ice.ValueFactory", package) << " _factory = new _F();";
        out << sp;
        out << nl << "public static " << getUnqualified("Ice.ValueFactory", package) << nl << "ice_factory()";
        out << sb;
        out << nl << "return _factory;";
        out << eb;
    }

    //
    // Marshalling & dispatch support.
    //
    if(!p->isInterface() && !p->isLocal())
    {
        writeDispatchAndMarshalling(out, p);
    }

    return true;
}

void
Slice::GenCompat::TypesVisitor::visitClassDefEnd(const ClassDefPtr& p)
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
        const UnitPtr unit = p->unit();
        const DefinitionContextPtr dc = unit->findDefinitionContext(p->file());
        assert(dc);
        string::size_type pos = serialVersionUID.rfind(":") + 1;
        if(pos == string::npos)
        {
            ostringstream os;
            os << "ignoring invalid serialVersionUID for class `" << p->scoped() << "'; generating default value";
            dc->warning(InvalidMetaData, "", "", os.str());
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
                    dc->warning(InvalidMetaData, "", "", os.str());
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
Slice::GenCompat::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string name = fixKwd(p->name());
    string scoped = p->scoped();
    ExceptionPtr base = p->base();
    string package = getPackage(p);
    string absolute = getUnqualified(p);
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
            out << getUnqualified("Ice.LocalException", package);
        }
        else
        {
            out << getUnqualified("Ice.UserException", package);
        }
    }
    else
    {
        out << getUnqualified(base, package);
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
    out << nl << "public " << name << "(Throwable cause)";
    out << sb;
    out << nl << "super(cause);";
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
                    const string causeParamName = getEscapedParamName(allDataMembers, "cause");
                    paramDecl.push_back("Throwable " + causeParamName);
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
                        baseParamNames.push_back(causeParamName);
                        out << baseParamNames << epar << ';';
                    }
                    else
                    {
                        out << nl << "super(" << causeParamName << ");";
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
                    capName[0] = static_cast<char>(toupper(static_cast<unsigned char>(capName[0])));
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
                const string causeParamName = getEscapedParamName(allDataMembers, "cause");
                paramDecl.push_back("Throwable " + causeParamName);
                out << sp << nl << "public " << name << spar;
                out << paramDecl << epar;
                out << sb;
                if(!base)
                {
                    out << nl << "super(" << causeParamName << ");";
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
                    baseParamNames.push_back(causeParamName);
                    out << baseParamNames << epar << ';';
                }
                for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
                {
                    string paramName = fixKwd((*d)->name());
                    if((*d)->optional())
                    {
                        string capName = paramName;
                        capName[0] = static_cast<char>(toupper(static_cast<unsigned char>(capName[0])));
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

    out << sp << nl << "public String" << nl << "ice_id()";
    out << sb;
    out << nl << "return \"" << scoped << "\";";
    out << eb;

    return true;
}

void
Slice::GenCompat::TypesVisitor::visitExceptionEnd(const ExceptionPtr& p)
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
            out << sp;
            out << nl << "public " << getUnqualified("Ice.SlicedData", package) << " " << nl << "ice_getSlicedData()";
            out << sb;
            out << nl << "return _slicedData;";
            out << eb;

            out << sp << nl << "public void" << nl << "_write(" << getUnqualified("Ice.OutputStream", package) << " ostr)";
            out << sb;
            out << nl << "ostr.startException(_slicedData);";
            out << nl << "_writeImpl(ostr);";
            out << nl << "ostr.endException();";
            out << eb;

            out << sp << nl << "public void" << nl << "_read(" << getUnqualified("Ice.InputStream", package) << " istr)";
            out << sb;
            out << nl << "istr.startException();";
            out << nl << "_readImpl(istr);";
            out << nl << "_slicedData = istr.endException(true);";
            out << eb;
        }

        out << sp << nl << "protected void" << nl << "_writeImpl(" << getUnqualified("Ice.OutputStream", package)
            << " ostr_)";
        out << sb;
        out << nl << "ostr_.startSlice(\"" << scoped << "\", -1, " << (!base ? "true" : "false") << ");";
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
        out << nl << "ostr_.endSlice();";
        if(base)
        {
            out << nl << "super._writeImpl(ostr_);";
        }
        out << eb;

        DataMemberList classMembers = p->classDataMembers();
        DataMemberList allClassMembers = p->allClassDataMembers();

        if(classMembers.size() != 0)
        {
            writePatcher(out, package, classMembers, optionalMembers);
        }
        out << sp << nl << "protected void" << nl << "_readImpl(" << getUnqualified("Ice.InputStream", package)
            << " istr_)";
        out << sb;
        out << nl << "istr_.startSlice();";
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
        out << nl << "istr_.endSlice();";
        if(base)
        {
            out << nl << "super._readImpl(istr_);";
        }
        out << eb;

        if(p->usesClasses(false))
        {
            if(!base || (base && !base->usesClasses(false)))
            {
                out << sp << nl << "public boolean" << nl << "_usesClasses()";
                out << sb;
                out << nl << "return true;";
                out << eb;
            }
        }

        if(preserved && !basePreserved)
        {
            out << sp << nl << "protected " << getUnqualified("Ice.SlicedData", package) << " _slicedData;";
        }
    }

    out << sp << nl << "public static final long serialVersionUID = ";
    string serialVersionUID;
    if(p->findMetaData("java:serialVersionUID", serialVersionUID))
    {
        const UnitPtr unit = p->unit();
        const DefinitionContextPtr dc = unit->findDefinitionContext(p->file());
        assert(dc);
        string::size_type pos = serialVersionUID.rfind(":") + 1;
        if(pos == string::npos)
        {
            ostringstream os;
            os << "ignoring invalid serialVersionUID for exception `" << p->scoped() << "'; generating default value";
            dc->warning(InvalidMetaData, "", "", os.str());
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
                    dc->warning(InvalidMetaData, "", "", os.str());
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
Slice::GenCompat::TypesVisitor::visitStructStart(const StructPtr& p)
{
    string name = fixKwd(p->name());
    string absolute = getUnqualified(p);

    open(absolute, p->file());

    Output& out = output();

    //
    // Check for java:implements metadata.
    //
    const StringList metaData = p->getMetaData();
    static const string prefix = "java:implements:";
    StringList implements;
    for(StringList::const_iterator q = metaData.begin(); q != metaData.end(); ++q)
    {
        if(q->find(prefix) == 0)
        {
            implements.push_back(q->substr(prefix.size()));
        }
    }

    out << sp;

    writeDocComment(out, p, getDeprecateReason(p, 0, "type"));

    out << nl << "public class " << name << " implements ";
    out.useCurrentPosAsIndent();
    out << "java.lang.Cloneable";
    if(!p->isLocal())
    {
        out << "," << nl << "java.io.Serializable";
    }
    for(StringList::const_iterator q = implements.begin(); q != implements.end(); ++q)
    {
        out << "," << nl << *q;
    }
    out.restoreIndent();

    out << sb;

    return true;
}

void
Slice::GenCompat::TypesVisitor::visitStructEnd(const StructPtr& p)
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
    out << nl << typeS << " r = null;";
    out << nl << "if(rhs instanceof " << typeS << ")";
    out << sb;
    out << nl << "r = (" << typeS << ")rhs;";
    out << eb;
    out << sp << nl << "if(r != null)";
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
                    out << nl << "if(this." << memberName << " != r." << memberName << ')';
                    out << sb;
                    out << nl << "return false;";
                    out << eb;
                    break;
                }

                case Builtin::KindString:
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                case Builtin::KindValue:
                {
                    out << nl << "if(this." << memberName << " != r." << memberName << ')';
                    out << sb;
                    out << nl << "if(this." << memberName << " == null || r." << memberName << " == null || !this."
                        << memberName << ".equals(r." << memberName << "))";
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
                    out << nl << "if(this." << memberName << " != r." << memberName << ')';
                    out << sb;
                    out << nl << "if(this." << memberName << " == null || r." << memberName << " == null || !this."
                        << memberName << ".equals(r." << memberName << "))";
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
                    out << nl << "if(!java.util.Arrays.equals(this." << memberName << ", r." << memberName << "))";
                    out << sb;
                    out << nl << "return false;";
                    out << eb;
                }
            }
            else
            {
                out << nl << "if(this." << memberName << " != r." << memberName << ')';
                out << sb;
                out << nl << "if(this." << memberName << " == null || r." << memberName << " == null || !this."
                    << memberName << ".equals(r." << memberName << "))";
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
    out << nl << "int h_ = 5381;";
    out << nl << "h_ = IceInternal.HashUtil.hashAdd(h_, \"" << p->scoped() << "\");";
    iter = 0;
    for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
    {
        string memberName = fixKwd((*d)->name());
        out << nl << "h_ = IceInternal.HashUtil.hashAdd(h_, " << memberName << ");";
    }
    out << nl << "return h_;";
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
        out << sp << nl << "public void" << nl << "ice_writeMembers(" << getUnqualified("Ice.OutputStream", package)
            << " ostr)";
        out << sb;
        iter = 0;
        for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
        {
            writeMarshalDataMember(out, package, *d, iter, true);
        }
        out << eb;

        DataMemberList classMembers = p->classDataMembers();

        if(classMembers.size() != 0)
        {
            writePatcher(out, package, classMembers, DataMemberList());
        }

        out << sp << nl << "public void" << nl << "ice_readMembers(" << getUnqualified("Ice.InputStream", package)
            << " istr)";
        out << sb;
        iter = 0;
        int patchIter = 0;
        const bool needCustomPatcher = classMembers.size() > 1;
        for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
        {
            writeUnmarshalDataMember(out, package, *d, iter, needCustomPatcher, patchIter, true);
        }
        out << eb;

        out << sp << nl << "static public void" << nl << "ice_write(" << getUnqualified("Ice.OutputStream", package)
            << " ostr, " << name << " v)";
        out << sb;
        out << nl << "if(v == null)";
        out << sb;
        out << nl << "_nullMarshalValue.ice_writeMembers(ostr);";
        out << eb;
        out << nl << "else";
        out << sb;
        out << nl << "v.ice_writeMembers(ostr);";
        out << eb;
        out << eb;

        out << sp << nl << "static public " << name << nl << "ice_read(" << getUnqualified("Ice.InputStream", package)
            << " istr)";
        out << sb;
        out << nl << name << " v = new " << name << "();";
        out << nl << "v.ice_readMembers(istr);";
        out << nl << "return v;";
        out << eb;

        out << sp << nl << "private static final " << name << " _nullMarshalValue = new " << name << "();";
    }

    out << sp << nl << "public static final long serialVersionUID = ";
    string serialVersionUID;
    if(p->findMetaData("java:serialVersionUID", serialVersionUID))
    {
        const UnitPtr unit = p->unit();
        const DefinitionContextPtr dc = unit->findDefinitionContext(p->file());
        assert(dc);
        string::size_type pos = serialVersionUID.rfind(":") + 1;
        if(pos == string::npos)
        {
            ostringstream os;
            os << "ignoring invalid serialVersionUID for struct `" << p->scoped() << "'; generating default value";
            dc->warning(InvalidMetaData, "", "", os.str());
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
                    dc->warning(InvalidMetaData, "", "", os.str());
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
Slice::GenCompat::TypesVisitor::visitDataMember(const DataMemberPtr& p)
{
    string name = fixKwd(p->name());
    const ContainerPtr container = p->container();
    const ClassDefPtr cls = ClassDefPtr::dynamicCast(container);
    const StructPtr st = StructPtr::dynamicCast(container);
    const ExceptionPtr ex = ExceptionPtr::dynamicCast(container);
    const ContainedPtr contained = ContainedPtr::dynamicCast(container);
    StringList metaData = p->getMetaData();
    TypePtr type = p->type();

    bool local;
    if(cls)
    {
        local = cls->isLocal();
    }
    else if(st)
    {
        local = st->isLocal();
    }
    else
    {
        assert(ex);
        local = ex->isLocal();
    }

    string s = typeToString(type, TypeModeMember, getPackage(contained), metaData, true, false, local);
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
        out << nl << "private boolean _" << p->name() << ';';
    }

    //
    // Getter/Setter.
    //
    if(getSet || optional)
    {
        string capName = p->name();
        capName[0] = static_cast<char>(toupper(static_cast<unsigned char>(capName[0])));

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
            out << nl << "if(!_" << p->name() << ')';
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
            << nl << "set" << capName << '(' << s << " " << name << ')';
        out << sb;
        if(optional)
        {
            out << nl << "_" << p->name() << " = true;";
        }
        out << nl << "this." << name << " = " << name << ';';
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
            out << nl << "return _" << p->name() << ';';
            out << eb;

            out << sp;
            writeDocComment(out, p, deprecateReason);
            out << nl << "public void"
                << nl << "clear" << capName << "()";
            out << sb;
            out << nl << "_" << p->name() << " = false;";
            out << eb;

            const string optType = typeToString(type, TypeModeMember, getPackage(contained), metaData, true, true);

            out << sp;
            writeDocComment(out, p, deprecateReason);
            out << nl << "public void"
                << nl << "optional" << capName << '(' << optType << " v)";
            out << sb;
            out << nl << "if(v == null || !v.isSet())";
            out << sb;
            out << nl << "_" << p->name() << " = false;";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << "_" << p->name() << " = true;";
            out << nl << name << " = v.get();";
            out << eb;
            out << eb;

            out << sp;
            writeDocComment(out, p, deprecateReason);
            out << nl << "public " << optType
                << nl << "optional" << capName << "()";
            out << sb;
            out << nl << "if(_" << p->name() << ')';
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
                out << nl << "if(!_" << p->name() << ')';
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
                out << nl << "get" << capName << "(int index)";
                out << sb;
                if(optional)
                {
                    out << nl << "if(!_" << p->name() << ')';
                    out << sb;
                    out << nl << "throw new java.lang.IllegalStateException(\"" << name << " is not set\");";
                    out << eb;
                }
                out << nl << "return this." << name << "[index];";
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
                out << nl << "set" << capName << "(int index, " << elem << " val)";
                out << sb;
                if(optional)
                {
                    out << nl << "if(!_" << p->name() << ')';
                    out << sb;
                    out << nl << "throw new java.lang.IllegalStateException(\"" << name << " is not set\");";
                    out << eb;
                }
                out << nl << "this." << name << "[index] = val;";
                out << eb;
            }
        }
    }
}

void
Slice::GenCompat::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string name = fixKwd(p->name());
    string absolute = getUnqualified(p);
    string package = getPackage(p);
    EnumeratorList enumerators = p->enumerators();

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
        out << sp;
        writeDocComment(out, *en, getDeprecateReason(*en, 0, "enumerator"));
        out << nl << fixKwd((*en)->name()) << '(' << (*en)->value() << ')';
    }
    out << ';';

    out << sp << nl << "public int value()";
    out << sb;
    out << nl << "return _value;";
    out << eb;

    out << sp << nl << "public static " << name << " valueOf(int v)";
    out << sb;
    out << nl << "switch(v)";
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

    out << sp << nl << "private " << name << "(int v)";
    out << sb;
    out << nl << "_value = v;";
    out << eb;

    if(!p->isLocal())
    {
        out << sp << nl << "public void ice_write(" << getUnqualified("Ice.OutputStream", package) << " ostr)";
        out << sb;
        out << nl << "ostr.writeEnum(_value, " << p->maxValue() << ");";
        out << eb;

        out << sp << nl << "public static void ice_write(" << getUnqualified("Ice.OutputStream", package) << " ostr, "
            << name << " v)";
        out << sb;
        out << nl << "if(v == null)";
        out << sb;
        string firstEnum = fixKwd(enumerators.front()->name());
        out << nl << "ostr.writeEnum(" << absolute << '.' << firstEnum << ".value(), " << p->maxValue() << ");";
        out << eb;
        out << nl << "else";
        out << sb;
        out << nl << "ostr.writeEnum(v.value(), " << p->maxValue() << ");";
        out << eb;
        out << eb;

        out << sp << nl << "public static " << name << " ice_read(" << getUnqualified("Ice.InputStream", package)
            << " istr)";
        out << sb;
        out << nl << "int v = istr.readEnum(" << p->maxValue() << ");";
        out << nl << "return validate(v);";
        out << eb;

        out << sp << nl << "private static " << name << " validate(int v)";
        out << sb;
        out << nl << "final " << name << " e = valueOf(v);";
        out << nl << "if(e == null)";
        out << sb;
        out << nl << "throw new " << getUnqualified("Ice.MarshalException", package)
            << "(\"enumerator value \" + v + \" is out of range\");";
        out << eb;
        out << nl << "return e;";
        out << eb;
    }

    out << sp << nl << "private final int _value;";

    out << eb;
    close();
}

void
Slice::GenCompat::TypesVisitor::visitConst(const ConstPtr& p)
{
    string name = fixKwd(p->name());
    string package = getPackage(p);
    string absolute = getUnqualified(p);
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
Slice::GenCompat::TypesVisitor::validateMethod(const OperationList& ops, const std::string& name, int numArgs,
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

Slice::GenCompat::CompactIdVisitor::CompactIdVisitor(const string& dir) :
    JavaCompatVisitor(dir)
{
}

bool
Slice::GenCompat::CompactIdVisitor::visitClassDefStart(const ClassDefPtr& p)
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

Slice::GenCompat::HolderVisitor::HolderVisitor(const string& dir) :
    JavaCompatVisitor(dir)
{
}

bool
Slice::GenCompat::HolderVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    ClassDeclPtr decl = p->declaration();
    writeHolder(decl);

    if(!p->isLocal())
    {
        string name = p->name();
        string absolute = getUnqualified(p, "", "", "PrxHolder");

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
Slice::GenCompat::HolderVisitor::visitStructStart(const StructPtr& p)
{
    writeHolder(p);
    return false;
}

void
Slice::GenCompat::HolderVisitor::visitSequence(const SequencePtr& p)
{
    if(sequenceHasHolder(p))
    {
        writeHolder(p);
    }
}

void
Slice::GenCompat::HolderVisitor::visitDictionary(const DictionaryPtr& p)
{
    writeHolder(p);
}

void
Slice::GenCompat::HolderVisitor::visitEnum(const EnumPtr& p)
{
    writeHolder(p);
}

void
Slice::GenCompat::HolderVisitor::writeHolder(const TypePtr& p)
{
    ContainedPtr contained = ContainedPtr::dynamicCast(p);
    assert(contained);
    string name = contained->name();
    string package = getPackage(contained);
    string absolute = getUnqualified(contained, "", "", "Holder");

    string file;
    if(p->definitionContext())
    {
        file = p->definitionContext()->filename();
    }

    open(absolute, file);
    Output& out = output();
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p);
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(p);

    out << sp;
    if(cl)
    {
        writeSuppressDeprecation(out, cl->definition());
    }

    string typeS = typeToString(p, TypeModeIn, getPackage(contained));
    out << nl << "public final class " << name << "Holder";
    if(!p->isLocal() && ((builtin && (builtin->kind() == Builtin::KindObject ||
                                      builtin->kind() == Builtin::KindValue)) || cl))
    {
        out << " extends " << getUnqualified("Ice.ObjectHolderBase", package) << "<" << typeS << ">";
    }
    else {
        out << " extends " << getUnqualified("Ice.Holder", package) << "<" << typeS << ">";
    }
    out << sb;
    if(!p->isLocal() && ((builtin && (builtin->kind() == Builtin::KindObject ||
                                      builtin->kind() == Builtin::KindValue)) || cl))
    {
        out << sp << nl << "public" << nl << name << "Holder()";
        out << sb;
        out << eb;
        out << sp << nl << "public" << nl << name << "Holder(" << typeS << " value)";
        out << sb;
        out << nl << "this.value = value;";
        out << eb;

        out << sp << nl << "public void";
        out << nl << "valueReady(" << getUnqualified("Ice.Object", package) << " v)";
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

Slice::GenCompat::HelperVisitor::HelperVisitor(const string& dir) :
    JavaCompatVisitor(dir)
{
}

bool
Slice::GenCompat::HelperVisitor::visitClassDefStart(const ClassDefPtr& p)
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
    string absolute = getUnqualified(p);

    open(getUnqualified(p, "", "", "PrxHelper"), p->file());
    Output& out = output();
    OperationList ops = p->allOperations();

    //
    // A proxy helper class serves two purposes: it implements the
    // proxy interface, and provides static helper methods for use
    // by applications (e.g., checkedCast, etc.)
    //
    out << sp;
    writeDocComment(out, getDeprecateReason(p, 0, p->isInterface() ? "interface" : "class"),
                    "Provides type-specific helper functions.");

    if(!writeSuppressDeprecation(out, p))
    {
        for(OperationList::iterator r = ops.begin(); r != ops.end(); ++r)
        {
            if(writeSuppressDeprecation(out, (*r)))
            {
                break;
            }
        }
    }

    out << nl << "public final class " << name << "PrxHelper extends "
        << getUnqualified("Ice.ObjectPrxHelperBase", package) << " implements " << name << "Prx";

    out << sb;

    string contextParam = "java.util.Map<String, String> context";
    string explicitContextParam = "boolean explicitCtx";

    for(OperationList::iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        const ContainerPtr container = op->container();
        const ClassDefPtr cl = ClassDefPtr::dynamicCast(container);

        out << sp;
        out << nl << "private static final String _" << op->name() << "_name = \"" << op->name() << "\";";

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
        const string iresultParamName = getEscapedParamName(op, "iresult");
        out << nl << "public " << retS << " end_" << op->name() << spar << outParams
            << (getUnqualified("Ice.AsyncResult", package) + " " + iresultParamName) << epar;
        writeThrowsClause(package, throws);
        out << sb;
        if(op->returnsData())
        {
            out << nl << "IceInternal.OutgoingAsync result_ = IceInternal.OutgoingAsync.check("
                << iresultParamName << ", this, _"  << op->name() << "_name);";
            out << nl << "try";
            out << sb;

            out << nl << "if(!result_.waitForResponseOrUserEx())";
            out << sb;
            out << nl << "try";
            out << sb;
            out << nl << "result_.throwUserException();";
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
                out << nl << "catch(" << getUnqualified(*eli, package) << " ex_)";
                out << sb;
                out << nl << "throw ex_;";
                out << eb;
            }
            out << nl << "catch(" << getUnqualified("Ice.UserException", package) << " ex_)";
            out << sb;
            out << nl << "throw new " << getUnqualified("Ice.UnknownUserException", package) << "(ex_.ice_id(), ex_);";
            out << eb;
            out << eb;

            if(ret || !outParams.empty())
            {
                out << nl << getUnqualified("Ice.InputStream", package) << " istr_ = result_.startReadParams();";
                const ParamDeclList paramList = op->parameters();
                ParamDeclList pl;
                for(ParamDeclList::const_iterator pli = paramList.begin(); pli != paramList.end(); ++pli)
                {
                    if((*pli)->isOutParam())
                    {
                        pl.push_back(*pli);
                    }
                }
                writeMarshalUnmarshalParams(out, package, pl, op, iter, false, true, false);
                if(op->returnsClasses(false))
                {
                    out << nl << "istr_.readPendingValues();";
                }
                out << nl << "result_.endReadParams();";
            }
            else
            {
                out << nl << "result_.readEmptyParams();";
            }

            if(ret)
            {
                BuiltinPtr builtin = BuiltinPtr::dynamicCast(ret);
                if(!op->returnIsOptional() &&
                   ((builtin && (builtin->kind() == Builtin::KindObject ||
                                 builtin->kind() == Builtin::KindValue)) || ClassDeclPtr::dynamicCast(ret)))
                {
                    out << nl << "return ret_.value;";
                }
                else
                {
                    out << nl << "return ret_;";
                }
            }

            out << eb;
            out << nl << "finally";
            out << sb;
            out << nl << "if(result_ != null)";
            out << sb;
            out << nl << "result_.cacheMessageBuffers();";
            out << eb;
            out << eb;
        }
        else
        {
            out << nl << "_end(" << iresultParamName << ", _" << op->name() << "_name);";
        }
        out << eb;

        //
        // The async callbacks implementation of _iceCompleted method delegate to the static
        // _iceI_<op-name>_completed method implemented bellow.
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

            out << sp << nl << "static public void _iceI_" << op->name() << "_completed("
                << getAsyncCallbackInterface(op, package) << " cb, Ice.AsyncResult result)";
            out << sb;
            out << nl << getUnqualified(cl, "", "", "Prx") << " _proxy = ("
                << getUnqualified(cl, "", "", "Prx") << ")result.getProxy();";

            TypePtr ret = op->returnType();
            if(ret)
            {
                out << nl << typeToString(ret, TypeModeIn, package, op->getMetaData(), true,
                                            op->returnIsOptional())
                    << " ret = " << (op->returnIsOptional() ? "null" : initValue(ret)) << ';';
            }
            for(ParamDeclList::const_iterator pli = outParams.begin(); pli != outParams.end(); ++pli)
            {
                string ts = typeToString((*pli)->type(), TypeModeOut, package, (*pli)->getMetaData(), true,
                                            (*pli)->optional());
                out << nl << ts << " iceP_" << (*pli)->name() << " = new " << ts << "();";
            }
            out << nl << "try";
            out << sb;
            out << nl;
            if(op->returnType())
            {
                out << "ret = ";
            }
            out << "_proxy.end_" << op->name() << spar << getInOutArgs(op, OutParam, true) << "result" << epar
                << ';';

            out << eb;
            if(!throws.empty())
            {
                out << nl << "catch(" << getUnqualified("Ice.UserException", package) << " ex)";
                out << sb;
                out << nl << "cb.exception(ex);";
                out << nl << "return;";
                out << eb;
            }
            out << nl << "catch(" << getUnqualified("Ice.LocalException", package) << " ex)";
            out << sb;
            out << nl << "cb.exception(ex);";
            out << nl << "return;";
            out << eb;
            out << nl << "catch(" << getUnqualified("Ice.SystemException", package) << " ex)";
            out << sb;
            out << nl << "cb.exception(ex);";
            out << nl << "return;";
            out << eb;

            out << nl << "cb.response" << spar;
            if(op->returnType())
            {
                out << "ret";
            }
            for(ParamDeclList::const_iterator pli = outParams.begin(); pli != outParams.end(); ++pli)
            {
                if((*pli)->optional())
                {
                    out << "iceP_" + (*pli)->name();
                }
                else
                {
                    out << "iceP_" + (*pli)->name() + ".value";
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
                    "@param obj The untyped proxy.\n"
                    "@return A proxy for this type, or null if the object does not support this type.");
    out << nl << "public static " << name << "Prx checkedCast(" << getUnqualified("Ice.ObjectPrx", package) << " obj)";
    out << sb;
    out << nl << "return checkedCastImpl(obj, ice_staticId(), " << name << "Prx.class, "
        << name << "PrxHelper.class);";
    out << eb;

    out << sp;
    writeDocComment(out, "",
                    "Contacts the remote server to verify that the object implements this type.\n"
                    "Raises a local exception if a communication error occurs.\n"
                    "@param obj The untyped proxy.\n"
                    "@param context The Context map to send with the invocation.\n"
                    "@return A proxy for this type, or null if the object does not support this type.");
    out << nl << "public static " << name << "Prx checkedCast(" << getUnqualified("Ice.ObjectPrx", package) << " obj, "
        << contextParam << ')';
    out << sb;
    out << nl << "return checkedCastImpl(obj, context, ice_staticId(), " << name
        << "Prx.class, " << name << "PrxHelper.class);";
    out << eb;

    out << sp;
    writeDocComment(out, "",
                    "Contacts the remote server to verify that a facet of the object implements this type.\n"
                    "Raises a local exception if a communication error occurs.\n"
                    "@param obj The untyped proxy.\n"
                    "@param facet The name of the desired facet.\n"
                    "@return A proxy for this type, or null if the object does not support this type.");
    out << nl << "public static " << name << "Prx checkedCast(" << getUnqualified("Ice.ObjectPrx", package)
        << " obj, String facet)";
    out << sb;
    out << nl << "return checkedCastImpl(obj, facet, ice_staticId(), " << name
        << "Prx.class, " << name << "PrxHelper.class);";
    out << eb;

    out << sp;
    writeDocComment(out, "",
                    "Contacts the remote server to verify that a facet of the object implements this type.\n"
                    "Raises a local exception if a communication error occurs.\n"
                    "@param obj The untyped proxy.\n"
                    "@param facet The name of the desired facet.\n"
                    "@param context The Context map to send with the invocation.\n"
                    "@return A proxy for this type, or null if the object does not support this type.");
    out << nl << "public static " << name << "Prx checkedCast(" << getUnqualified("Ice.ObjectPrx", package)
        << " obj, String facet, " << contextParam << ')';
    out << sb;
    out << nl << "return checkedCastImpl(obj, facet, context, ice_staticId(), " << name
        << "Prx.class, " << name << "PrxHelper.class);";
    out << eb;

    out << sp;
    writeDocComment(out, "",
                    "Downcasts the given proxy to this type without contacting the remote server.\n"
                    "@param obj The untyped proxy.\n"
                    "@return A proxy for this type.");
    out << nl << "public static " << name << "Prx uncheckedCast(" << getUnqualified("Ice.ObjectPrx", package) << " obj)";
    out << sb;
    out << nl << "return uncheckedCastImpl(obj, " << name << "Prx.class, " << name
        << "PrxHelper.class);";
    out << eb;

    out << sp;
    writeDocComment(out, "",
                    "Downcasts the given proxy to this type without contacting the remote server.\n"
                    "@param obj The untyped proxy.\n"
                    "@param facet The name of the desired facet.\n"
                    "@return A proxy for this type.");
    out << nl << "public static " << name << "Prx uncheckedCast(" << getUnqualified("Ice.ObjectPrx", package)
        << " obj, String facet)";
    out << sb;
    out << nl << "return uncheckedCastImpl(obj, facet, " << name << "Prx.class, " << name
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

    out << sp << nl << "private static final String[] _ids =";
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
    out << nl << "return _ids[" << scopedPos << "];";
    out << eb;

    out << sp << nl << "public static void write(" << getUnqualified("Ice.OutputStream", package) << " ostr, " << name
        << "Prx v)";
    out << sb;
    out << nl << "ostr.writeProxy(v);";
    out << eb;

    out << sp << nl << "public static " << name << "Prx read(" << getUnqualified("Ice.InputStream", package) << " istr)";
    out << sb;
    out << nl << getUnqualified("Ice.ObjectPrx", package) << " proxy = istr.readProxy();";
    out << nl << "if(proxy != null)";
    out << sb;
    out << nl << name << "PrxHelper result = new " << name << "PrxHelper();";
    out << nl << "result._copyFrom(proxy);";
    out << nl << "return result;";
    out << eb;
    out << nl << "return null;";
    out << eb;

    //
    // Avoid serialVersionUID warnings for Proxy Helper classes.
    //
    out << sp << nl << "public static final long serialVersionUID = 0L;";
    out << eb;

    close();

    return false;
}

void
Slice::GenCompat::HelperVisitor::visitSequence(const SequencePtr& p)
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
    string absolute = getUnqualified(p);
    string helper = getUnqualified(p, "", "", "Helper");
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

    out << nl << "public static void" << nl << "write(" << getUnqualified("Ice.OutputStream", package) << " ostr, "
        << typeS << " v)";
    out << sb;
    iter = 0;
    writeSequenceMarshalUnmarshalCode(out, package, p, "v", true, iter, false);
    out << eb;

    out << sp;
    if(suppressUnchecked)
    {
        out << nl << "@SuppressWarnings(\"unchecked\")";
    }
    out << nl << "public static " << typeS << nl << "read(" << getUnqualified("Ice.InputStream", package) << " istr)";
    out << sb;
    out << nl << typeS << " v;";
    iter = 0;
    writeSequenceMarshalUnmarshalCode(out, package, p, "v", false, iter, false);
    out << nl << "return v;";
    out << eb;

    out << eb;
    close();
}

void
Slice::GenCompat::HelperVisitor::visitDictionary(const DictionaryPtr& p)
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
    string absolute = getUnqualified(p);
    string helper = getUnqualified(p, "", "", "Helper");
    string package = getPackage(p);
    StringList metaData = p->getMetaData();
    string formalType = typeToString(p, TypeModeIn, package, StringList(), true);

    open(helper, p->file());
    Output& out = output();

    int iter;

    out << sp << nl << "public final class " << name << "Helper";
    out << sb;

    out << nl << "public static void" << nl << "write(" << getUnqualified("Ice.OutputStream", package) << " ostr, "
        << formalType << " v)";
    out << sb;
    iter = 0;
    writeDictionaryMarshalUnmarshalCode(out, package, p, "v", true, iter, false);
    out << eb;

    out << sp << nl << "public static " << formalType
        << nl << "read(" << getUnqualified("Ice.InputStream", package) << " istr)";
    out << sb;
    out << nl << formalType << " v;";
    iter = 0;
    writeDictionaryMarshalUnmarshalCode(out, package, p, "v", false, iter, false);
    out << nl << "return v;";
    out << eb;

    out << eb;
    close();
}

void
Slice::GenCompat::HelperVisitor::writeOperation(const ClassDefPtr& p, const string& package, const OperationPtr& op,
                                                bool optionalMapping)
{
    const string name = p->name();
    Output& out = output();

    const string contextParamName = getEscapedParamName(op, "context");
    const string contextParam = "java.util.Map<String, String> " + contextParamName;
    const string contextParamInternal = "java.util.Map<String, String> context";
    const string explicitContextParam = "boolean explicitCtx";

    const ContainerPtr container = op->container();
    const ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    const string opName = fixKwd(op->name());
    const TypePtr ret = op->returnType();
    const string retS = typeToString(ret, TypeModeReturn, package, op->getMetaData(), true, op->returnIsOptional());

    vector<string> params = getParamsProxy(op, package, false, optionalMapping);
    vector<string> paramsInternal = getParamsProxy(op, package, false, optionalMapping, true);
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
    out << "_iceI_" << op->name() << spar << args << "null" << "false" << epar << ';';
    out << eb;

    out << sp << nl << "public " << retS << ' ' << opName << spar << params << contextParam << epar;
    writeThrowsClause(package, throws);
    out << sb;
    out << nl;
    if(ret)
    {
        out << "return ";
    }
    out << "_iceI_" << op->name() << spar << args << contextParamName << "true" << epar << ';';
    out << eb;

    out << sp;
    out << nl << "private " << retS << " _iceI_" << op->name() << spar << paramsInternal << contextParamInternal
        << explicitContextParam << epar;
    writeThrowsClause(package, throws);
    out << sb;

    // This code replaces the synchronous calls with chained AMI calls.
    if(op->returnsData())
    {
        out << nl << "_checkTwowayOnly(_" << op->name() << "_name);";
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
    vector<string> inOutArgs = getInOutArgs(op, OutParam, true);
    if(!inOutArgs.empty()) {
        for(vector<string>::const_iterator p = inOutArgs.begin(); p != inOutArgs.end(); ++p) {
            out << *p << ", ";
        }
    }
    vector<string> inArgs = getInOutArgs(op, InParam, true);
    out << "_iceI_begin_" << op->name() << "(";
    if(!inArgs.empty())
    {
        for(vector<string>::const_iterator p = inArgs.begin(); p != inArgs.end(); ++p) {
            out << *p << ", ";
        }
    }
    out << "context, explicitCtx, true, null));";
    out << eb;

    {
        //
        // Write the asynchronous begin methods.
        //
        vector<string> inParams = getInOutParams(op, package, InParam, true, optionalMapping);
        vector<string> inArgs = getInOutArgs(op, InParam);
        const string callbackParamName = getEscapedParamName(op, "cb");
        const string callbackParam = getUnqualified("Ice.Callback", package) + " " + callbackParamName;
        const ParamDeclList paramList = op->parameters();
        int iter;

        //
        // Type-unsafe begin methods
        //
        out << sp << nl << "public " << getUnqualified("Ice.AsyncResult", package) << " begin_" << op->name() << spar
            << inParams << epar;
        out << sb;
        out << nl << "return _iceI_begin_" << op->name() << spar << inArgs << "null" << "false" << "false" << "null"
            << epar << ';';
        out << eb;

        out << sp << nl << "public " << getUnqualified("Ice.AsyncResult", package) << " begin_" << op->name() << spar
            << inParams << contextParam << epar;
        out << sb;
        out << nl << "return _iceI_begin_" << op->name() << spar << inArgs << contextParamName << "true" << "false"
            << "null" << epar << ';';
        out << eb;

        out << sp << nl << "public " << getUnqualified("Ice.AsyncResult", package) << " begin_" << op->name() << spar
            << inParams << callbackParam << epar;
        out << sb;
        out << nl << "return _iceI_begin_" << op->name() << spar << inArgs << "null" << "false" << "false"
            << callbackParamName << epar << ';';
        out << eb;

        out << sp << nl << "public " << getUnqualified("Ice.AsyncResult", package) << " begin_" << op->name() << spar
            << inParams << contextParam << callbackParam << epar;
        out << sb;
        out << nl << "return _iceI_begin_" << op->name() << spar << inArgs << contextParamName << "true" << "false"
            << callbackParamName << epar << ';';
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
        string opClassName = getUnqualified(cl, package, "Callback_", '_' + op->name());
        typeSafeCallbackParam = opClassName + " " + getEscapedParamName(op, "cb");

        out << sp << nl << "public " << getUnqualified("Ice.AsyncResult", package) << " begin_" << op->name() << spar
            << inParams << typeSafeCallbackParam << epar;
        out << sb;
        out << nl << "return _iceI_begin_" << op->name() << spar << inArgs << "null" << "false" << "false"
            << callbackParamName << epar << ';';
        out << eb;

        out << sp << nl << "public " << getUnqualified("Ice.AsyncResult", package) << " begin_" << op->name() << spar
            << inParams << contextParam << typeSafeCallbackParam << epar;
        out << sb;
        out << nl << "return _iceI_begin_" << op->name() << spar << inArgs << contextParamName << "true" << "false"
            << callbackParamName << epar << ';';
        out << eb;

        //
        // Async methods that accept Java 8 lambda callbacks.
        //
        out << sp;
        out << nl << "public " << getUnqualified("Ice.AsyncResult", package) << " begin_" << op->name();
        writeParamList(out, getParamsAsyncLambda(op, package, false, false, optionalMapping));
        out << sb;
        out << nl << "return _iceI_begin_" << op->name() << spar << getArgsAsyncLambda(op, package) << epar << ';';
        out << eb;

        out << sp;
        out << nl << "public " << getUnqualified("Ice.AsyncResult", package) << " begin_" << op->name();
        writeParamList(out, getParamsAsyncLambda(op, package, false, true, optionalMapping));
        out << sb;
        out << nl << "return _iceI_begin_" << op->name() << spar << getArgsAsyncLambda(op, package, false, true) << epar
            << ';';
        out << eb;

        out << sp;
        out << nl << "public " << getUnqualified("Ice.AsyncResult", package) << " begin_" << op->name();
        writeParamList(out, getParamsAsyncLambda(op, package, true, false, optionalMapping));
        out << sb;
        out << nl << "return _iceI_begin_" << op->name() << spar << getArgsAsyncLambda(op, package, true) << epar << ';';
        out << eb;

        out << sp;
        out << nl << "public " << getUnqualified("Ice.AsyncResult", package) << " begin_" << op->name();
        writeParamList(out, getParamsAsyncLambda(op, package, true, true, optionalMapping));
        out << sb;
        out << nl << "return _iceI_begin_" << op->name() << spar << getArgsAsyncLambda(op, package, true, true) << epar
            << ';';
        out << eb;

        vector<string> params = getInOutParams(op, package, InParam, true, optionalMapping, true);
        params.push_back("java.util.Map<String, String> context");
        params.push_back("boolean explicitCtx");
        params.push_back("boolean synchronous");
        vector<string> asyncParams = getParamsAsyncLambda(op, package, false, true, optionalMapping, false, true);
        copy(asyncParams.begin(), asyncParams.end(), back_inserter(params));

        out << sp;
        out << nl << "private " << getUnqualified("Ice.AsyncResult", package) << " _iceI_begin_" << op->name();
        writeParamList(out, params);
        out << sb;

        ParamDeclList outParams = getOutParams(op);

        if(!op->returnsData())
        {
            params = getInOutArgs(op, InParam, true);
            params.push_back("context");
            params.push_back("explicitCtx");
            params.push_back("synchronous");
            params.push_back("new IceInternal.Functional_OnewayCallback(responseCb, exceptionCb, sentCb)");
            out << nl << "return _iceI_begin_" << op->name();
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
            out << nl << "_responseCb = responseCb;";
            out << eb;

            out << sp;
            out << nl << "public void response" << spar << getParamsAsyncCB(op, package, false, true) << epar;
            out << sb;
            out << nl << "if(_responseCb != null)";
            out << sb;
            out << nl << "_responseCb.apply" << spar;
            if(ret)
            {
                out << "ret";
            }
            out << getInOutArgs(op, OutParam) << epar << ';';
            out << eb;
            out << eb;

            out << sp;
            out << nl << "public final void _iceCompleted(" << getUnqualified("Ice.AsyncResult", package) << " result)";
            out << sb;
            out << nl << p->name() << "PrxHelper._iceI_" << op->name() << "_completed(this, result);";
            out << eb;
            out << sp;
            out << nl << "private final " << getLambdaResponseCB(op, package) << " _responseCb;";
            out << eb;

            out << nl << "return _iceI_begin_" << op->name() << spar << getInOutArgs(op, InParam, true) << "context"
                << "explicitCtx"
                << "synchronous"
                << (throws.empty() ? "new CB(responseCb, exceptionCb, sentCb)" :
                                     "new CB(responseCb, userExceptionCb, exceptionCb, sentCb)")
                << epar << ';';
        }
        else
        {
            params = getInOutArgs(op, InParam, true);
            params.push_back("context");
            params.push_back("explicitCtx");
            params.push_back("synchronous");

            const string baseClass = getAsyncCallbackBaseClass(op, true);
            out << nl << "return _iceI_begin_" << op->name();
            writeParamList(out, params, false);
            out << (throws.empty() ? "new " + baseClass + "(responseCb, exceptionCb, sentCb)" :
                    "new " + baseClass + "(responseCb, userExceptionCb, exceptionCb, sentCb)");
            out.inc();
            out << sb;
            out << nl << "public final void _iceCompleted(" << getUnqualified("Ice.AsyncResult", package) << " result)";
            out << sb;
            out << nl << p->name() << "PrxHelper._iceI_" << op->name() << "_completed(this, result);";
            out << eb;
            out << eb;
            out << ");";
            out.dec();
            out.restoreIndent();
        }
        out << eb;

        //
        // Implementation of _iceI_begin method
        //
        params = getInOutParams(op, package, InParam, true, optionalMapping, true);
        params.push_back("java.util.Map<String, String> context");
        params.push_back("boolean explicitCtx");
        params.push_back("boolean synchronous");
        params.push_back("IceInternal.CallbackBase cb");

        out << sp;
        out << nl << "private " << getUnqualified("Ice.AsyncResult", package) << " _iceI_begin_" << op->name();
        writeParamList(out, params);
        out << sb;
        if(op->returnsData())
        {
            out << nl << "_checkAsyncTwowayOnly(_" << op->name() << "_name);";
        }
        out << nl << "IceInternal.OutgoingAsync result = getOutgoingAsync(_" << op->name()
            << "_name, cb);";
        out << nl << "try";
        out << sb;

        out << nl << "result.prepare(_" << op->name() << "_name, " << sliceModeToIceMode(op->sendMode())
            << ", context, explicitCtx, synchronous);";

        iter = 0;
        if(!inArgs.empty())
        {
            out << nl << getUnqualified("Ice.OutputStream", package) << " ostr = result.startWriteParams("
                << opFormatTypeToString(op) << ");";
            ParamDeclList pl;
            for(ParamDeclList::const_iterator pli = paramList.begin(); pli != paramList.end(); ++pli)
            {
                if(!(*pli)->isOutParam())
                {
                    pl.push_back(*pli);
                }
            }
            writeMarshalUnmarshalParams(out, package, pl, 0, iter, true, optionalMapping, true);
            if(op->sendsClasses(false))
            {
                out << nl << "ostr.writePendingValues();";
            }
            out << nl << "result.endWriteParams();";
        }
        else
        {
            out << nl << "result.writeEmptyParams();";
        }

        out << nl << "result.invoke();";
        out << eb;
        out << nl << "catch(" << getUnqualified("Ice.Exception", package) << " ex)";
        out << sb;
        out << nl << "result.abort(ex);";
        out << eb;
        out << nl << "return result;";
        out << eb;
    }
}

Slice::GenCompat::ProxyVisitor::ProxyVisitor(const string& dir) :
    JavaCompatVisitor(dir)
{
}

bool
Slice::GenCompat::ProxyVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal())
    {
        return false;
    }

    string name = p->name();
    ClassList bases = p->bases();
    string package = getPackage(p);
    string absolute = getUnqualified(p, "", "", "Prx");

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
        out << getUnqualified("Ice.ObjectPrx", package);
    }
    else
    {
        out.useCurrentPosAsIndent();
        for(ClassList::const_iterator q = bases.begin(); q != bases.end();)
        {
            out << getUnqualified(*q, package, "", "Prx");
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
Slice::GenCompat::ProxyVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    Output& out = output();
    out << eb;
    close();
}

void
Slice::GenCompat::ProxyVisitor::visitOperation(const OperationPtr& p)
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
    string contextDoc = "@param context The Context map to send with the invocation.";
    string contextParamName = getEscapedParamName(p, "context");
    string contextParam = "java.util.Map<String, String> " + contextParamName;
    string lambdaResponseDoc = "@param responseCb The lambda response callback.";
    string lambdaUserExDoc = "@param userExceptionCb The lambda user exception callback.";
    string lambdaExDoc = "@param exceptionCb The lambda exception callback.";
    string lambdaSentDoc = "@param sentCb The lambda sent callback.";

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
        string callbackParam = getUnqualified("Ice.Callback", package) + " " + getEscapedParamName(p, "cb");
        string callbackDoc = "@param cb The asynchronous callback object.";

        out << sp;
        writeDocCommentAMI(out, p, InParam);
        out << nl << "public " << getUnqualified("Ice.AsyncResult", package) << " begin_" << p->name() << spar << inParams
            << epar << ';';

        out << sp;
        writeDocCommentAMI(out, p, InParam, contextDoc);
        out << nl << "public " << getUnqualified("Ice.AsyncResult", package) << " begin_" << p->name() << spar << inParams
            << contextParam << epar << ';';

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
        string opClassName = getUnqualified(cl, package, "Callback_", '_' + p->name());
        typeSafeCallbackParam = opClassName + " " + getEscapedParamName(p, "cb");

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
        out << nl << "public " << retS << " end_" << p->name() << spar << outParams
            << "Ice.AsyncResult " + getEscapedParamName(p, "result")
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
        string callbackParam = "Ice.Callback " + getEscapedParamName(p, "cb");
        string callbackDoc = "@param cb The asynchronous callback object.";

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
        string opClassName = getUnqualified(cl, package, "Callback_", '_' + p->name());
        typeSafeCallbackParam = opClassName + " " + getEscapedParamName(p, "cb");

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

Slice::GenCompat::DispatcherVisitor::DispatcherVisitor(const string& dir, bool tie) :
    JavaCompatVisitor(dir),
    _tie(tie)
{
}

bool
Slice::GenCompat::DispatcherVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = p->name();
    ClassList bases = p->bases();

    if(!p->isLocal() && p->isInterface())
    {
        string absolute = getUnqualified(p, "", "_", "Disp");

        open(absolute, p->file());

        Output& out = output();

        out << sp;
        out << nl << "public abstract class _" << name << "Disp extends Ice.ObjectImpl implements " << fixKwd(name);
        out << sb;

        writeDispatchAndMarshalling(out, p);

        //
        // Avoid serialVersionUID warnings for dispatch classes.
        //
        out << sp << nl << "public static final long serialVersionUID = 0L;";
        out << eb;
        close();
    }

    if(_tie || p->hasMetaData("java:tie"))
    {
        // Tie class

        string package = getPackage(p);
        string absolute = getUnqualified(p, "", "_", "Tie");
        string opIntfName = "Operations";
        if(p->isLocal())
        {
            opIntfName += "NC";
        }

        //
        // Don't generate a tie class for a non-abstract class
        //
        if(!p->isAbstract())
        {
            return false;
        }

        open(absolute, p->file());

        Output& out = output();

        //
        // Generate the tie class
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

            const string currentParamName = getEscapedParamName((*r), "current");

            if(!p->isLocal())
            {
                out << "Ice.Current " + currentParamName;
            }
            out << epar;

            ExceptionList throws = (*r)->throws();
            throws.sort();
            throws.unique();
            writeThrowsClause(package, throws, *r);

            out << sb;
            out << nl;
            if(ret && !hasAMD)
            {
                out << "return ";
            }
            out << "_ice_delegate." << opName << spar << args;
            if(!p->isLocal())
            {
                out << currentParamName;
            }
            out << epar << ';';
            out << eb;
        }

        out << sp << nl << "private " << '_' << name << opIntfName << " _ice_delegate;";
        out << sp << nl << "public static final long serialVersionUID = ";
        string serialVersionUID;
        if(p->findMetaData("java:serialVersionUID", serialVersionUID))
        {
            const UnitPtr unit = p->unit();
            const DefinitionContextPtr dc = unit->findDefinitionContext(p->file());
            assert(dc);
            string::size_type pos = serialVersionUID.rfind(":") + 1;
            if(pos == string::npos)
            {
                ostringstream os;
                os << "ignoring invalid serialVersionUID for class `" << p->scoped() << "'; generating default value";
                dc->warning(InvalidMetaData, "", "", os.str());
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
                        dc->warning(InvalidMetaData, "", "", os.str());
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

    return false;
}

Slice::GenCompat::BaseImplVisitor::BaseImplVisitor(const string& dir) :
    JavaCompatVisitor(dir)
{
}

void
Slice::GenCompat::BaseImplVisitor::writeDecl(Output& out, const string& package, const string& name, const TypePtr& type,
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
                case Builtin::KindValue:
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
                EnumeratorList enumerators = en->enumerators();
                out << " = " << getUnqualified(en, package) << '.' << fixKwd(enumerators.front()->name());
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
Slice::GenCompat::BaseImplVisitor::writeReturn(Output& out, const TypePtr& type, bool optional)
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
                case Builtin::KindValue:
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
Slice::GenCompat::BaseImplVisitor::writeOperation(Output& out, const string& package, const OperationPtr& op, bool local)
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

        out << sp << nl << "public void " << opName << "_async" << spar << paramsAMD
            << "Ice.Current " + getEscapedParamName(op, "current")
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
        writeThrowsClause(package, throws, op);

        out << sb;

        string result = "r";
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

        out << nl << "cb.ice_response(";
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
            out << "Ice.Current " + getEscapedParamName(op, "current");
        }
        out << epar;

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();
        writeThrowsClause(package, throws, op);

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

Slice::GenCompat::ImplVisitor::ImplVisitor(const string& dir) :
    BaseImplVisitor(dir)
{
}

bool
Slice::GenCompat::ImplVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(!p->isAbstract())
    {
        return false;
    }

    string name = p->name();
    ClassList bases = p->bases();
    string package = getPackage(p);
    string absolute = getUnqualified(p, "", "", "I");

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

Slice::GenCompat::ImplTieVisitor::ImplTieVisitor(const string& dir) :
    BaseImplVisitor(dir)
{
}

bool
Slice::GenCompat::ImplTieVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(!p->isAbstract())
    {
        return false;
    }

    string name = p->name();
    ClassList bases = p->bases();
    string package = getPackage(p);
    string absolute = getUnqualified(p, "", "", "I");

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

Slice::GenCompat::AsyncVisitor::AsyncVisitor(const string& dir) :
    JavaCompatVisitor(dir)
{
}

void
Slice::GenCompat::AsyncVisitor::visitOperation(const OperationPtr& p)
{
    ContainerPtr container = p->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    const string package = getPackage(cl);
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
            open(getUnqualified(cl, "", "_Callback_", "_" + name), p->file());

            Output& out = output();

            writeDocCommentOp(out, p);
            out << sp << nl << "public interface " << ("_Callback_" + cl->name()) << '_' << name
                << " extends " << getUnqualified(throws.empty() ? "Ice.TwowayCallback" : "Ice.TwowayCallbackUE", package);
            out << sb;
            out << nl << "public void response" << spar << params << epar << ';';
            out << eb;

            close();
        }

        string classNameAsync = "Callback_" + cl->name();
        string absoluteAsync = getUnqualified(cl, "", "Callback_", "_" + name);

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

            out << sp << nl << "public final void _iceCompleted(" << getUnqualified("Ice.AsyncResult", package)
                << " result)";
            out << sb;
            out << nl << cl->name() << "PrxHelper._iceI_" << p->name() << "_completed(this, result);";
            out << eb;

            out << eb;
        }
        else
        {
            out << " extends " << getUnqualified("Ice.OnewayCallback", package);
            out << sb;
            out << eb;
        }

        close();
    }

    if(cl->hasMetaData("amd") || p->hasMetaData("amd"))
    {
        string classNameAMD = "AMD_" + cl->name();
        string absoluteAMD = getUnqualified(cl, "", "AMD_", "_" + name);

        string classNameAMDI = "_AMD_" + cl->name();
        string absoluteAMDI = getUnqualified(cl, "", "_AMD_", "_" + name);

        const bool optionalMapping = useOptionalMapping(p);
        vector<string> paramsAMD = getParamsAsyncCB(p, classPkg, true, optionalMapping);

        {
            open(absoluteAMD, p->file());

            Output& out = output();

            writeDocCommentOp(out, p);
            out << sp << nl << "public interface " << classNameAMD << '_' << name;
            out << " extends " << getUnqualified("Ice.AMDCallback", package);
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

            int iter;
            out << sp;
            writeSuppressDeprecation(out, p);
            out << nl << "final class " << classNameAMDI << '_' << name
                << " extends IceInternal.IncomingAsync implements " << classNameAMD << '_' << name;
            out << sb;

            out << sp << nl << "public " << classNameAMDI << '_' << name << "(IceInternal.Incoming in)";
            out << sb;
            out << nl << "super(in);";
            out << eb;

            out << sp << nl << "public void ice_response" << spar << paramsAMD << epar;
            out << sb;
            iter = 0;
            if(ret || !outParams.empty())
            {
                out << nl << getUnqualified("Ice.OutputStream", package) << " ostr_ = this.startWriteParams();";
                writeMarshalUnmarshalParams(out, classPkg, outParams, p, iter, true, optionalMapping, false, "ret", false);
                if(p->returnsClasses(false))
                {
                    out << nl << "ostr_.writePendingValues();";
                }
                out << nl << "this.endWriteParams();";
            }
            else
            {
                out << nl << "this.writeEmptyParams();";
            }
            out << nl << "this.completed();";
            out << eb;

            out << eb;

            close();
        }
    }
}
