//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Gen.h>
#include <Slice/Util.h>
#include <IceUtil/Functional.h>
#include <IceUtil/Iterator.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/InputUtil.h>
#include <cstring>

#include <limits>

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

namespace
{

string
sliceModeToIceMode(Operation::Mode opMode)
{
    string mode = "com.zeroc.Ice.OperationMode.";
    switch(opMode)
    {
    case Operation::Normal:
        mode = "null"; // shorthand for most common case
        break;
    case Operation::Nonmutating:
        mode += "Nonmutating";
        break;
    case Operation::Idempotent:
        mode += "Idempotent";
        break;
    default:
        assert(false);
        break;
    }
    return mode;
}

string
opFormatTypeToString(const OperationPtr& op)
{
    string format = "com.zeroc.Ice.FormatType.";
    switch(op->format())
    {
    case DefaultFormat:
        format = "null"; // shorthand for most common case
        break;
    case CompactFormat:
        format += "CompactFormat";
        break;
    case SlicedFormat:
        format += "SlicedFormat";
        break;
    default:
        assert(false);
        break;
    }
    return format;
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

bool
isDeprecated(const ContainedPtr& p1, const ContainedPtr& p2)
{
    string deprecateMetadata;
    return p1->findMetaData("deprecate", deprecateMetadata) ||
            (p2 != 0 && p2->findMetaData("deprecate", deprecateMetadata));
}

bool isValue(const TypePtr& type)
{
    BuiltinPtr b = BuiltinPtr::dynamicCast(type);
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    return (b && b->usesClasses()) || cl;
}

}

Slice::JavaVisitor::JavaVisitor(const string& dir) :
    JavaGenerator(dir)
{
}

Slice::JavaVisitor::~JavaVisitor()
{
}

string
Slice::JavaVisitor::getResultType(const OperationPtr& op, const string& package, bool object, bool dispatch)
{
    if(dispatch && op->hasMarshaledResult())
    {
        const ClassDefPtr c = ClassDefPtr::dynamicCast(op->container());
        assert(c);
        string abs;
        if(c->isInterface())
        {
            abs = getUnqualified(c, package);
        }
        else
        {
            abs = getUnqualified(c, package, "", "Disp");
        }
        string name = op->name();
        name[0] = static_cast<char>(toupper(static_cast<unsigned char>(name[0])));
        return abs + "." + name + "MarshaledResult";
    }
    else if(op->returnsMultipleValues())
    {
        const ContainedPtr c = ContainedPtr::dynamicCast(op->container());
        assert(c);
        const string abs = getUnqualified(c, package);
        string name = op->name();
        name[0] = static_cast<char>(toupper(static_cast<unsigned char>(name[0])));
        return abs + "." + name + "Result";
    }
    else
    {
        TypePtr type = op->returnType();
        bool optional = op->returnIsOptional();
        if(!type)
        {
            const ParamDeclList outParams = op->outParameters();
            if(!outParams.empty())
            {
                assert(outParams.size() == 1);
                type = outParams.front()->type();
                optional = outParams.front()->optional();
            }
        }
        if(type)
        {
            ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());
            assert(cl);
            return typeToAnnotatedString(type, TypeModeReturn, package, op->getMetaData(), optional, object);
        }
        else
        {
            return object ? "Void" : "void";
        }
    }
}

void
Slice::JavaVisitor::writeResultType(Output& out, const OperationPtr& op, const string& package, const CommentPtr& dc)
{
    string opName = op->name();
    opName[0] = static_cast<char>(toupper(static_cast<unsigned char>(opName[0])));

    out << sp;
    writeDocComment(out, "Holds the result of operation " + op->name() + ".");
    out << nl << "public static class " << opName << "Result";
    out << sb;

    //
    // Make sure none of the out parameters are named "returnValue".
    //
    string retval = "returnValue";
    const ParamDeclList outParams = op->outParameters();
    for(const auto& p : outParams)
    {
        if(p->name() == "returnValue")
        {
            retval = "_returnValue";
            break;
        }
    }

    const TypePtr ret = op->returnType();
    const ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());
    assert(cl);

    //
    // Default constructor.
    //
    writeDocComment(out, "Default constructor.");
    out << nl << "public " << opName << "Result()";
    out << sb;
    out << eb;

    //
    // One-shot constructor.
    //
    out << sp;
    if(dc)
    {
        //
        // Emit a doc comment for the constructor if necessary.
        //
        out << nl << "/**";
        out << nl << " * This constructor makes shallow copies of the results for operation " << opName << '.';

        if(ret && !dc->returns().empty())
        {
            out << nl << " * @param " << retval << ' ';
            writeDocCommentLines(out, dc->returns());
        }
        map<string, StringList> paramDocs = dc->parameters();
        for(const auto& p : outParams)
        {
            const string name = p->name();
            map<string, StringList>::const_iterator q = paramDocs.find(name);
            if(q != paramDocs.end() && !q->second.empty())
            {
                out << nl << " * @param " << fixKwd(q->first) << ' ';
                writeDocCommentLines(out, q->second);
            }
        }
        out << nl << " **/";
    }

    out << nl << "public " << opName << "Result" << spar;

    if(ret)
    {
        out << (typeToAnnotatedString(ret, TypeModeIn, package, op->getMetaData(), op->returnIsOptional())
                + " " + retval);
    }
    for(const auto& p : outParams)
    {
        out << (typeToAnnotatedString(p->type(), TypeModeIn, package, p->getMetaData(), p->optional())
                + " " + fixKwd(p->name()));
    }
    out << epar;
    out << sb;
    if(ret)
    {
        out << nl << "this." << retval << " = " << retval << ';';
    }
    for(const auto& p : outParams)
    {
        const string name = fixKwd(p->name());
        out << nl << "this." << name << " = " << name << ';';
    }
    out << eb;

    //
    // Members.
    //
    out << sp;
    if(ret)
    {
        if(dc && !dc->returns().empty())
        {
            out << nl << "/**";
            out << nl << " * ";
            writeDocCommentLines(out, dc->returns());
            out << nl << " **/";
        }
        out << nl << "public " << typeToAnnotatedString(ret, TypeModeIn, package, op->getMetaData(),
            op->returnIsOptional(), false) << ' ' << retval << ';';
    }

    for(const auto& p : outParams)
    {
        const string name = p->name();
        if(dc)
        {
            map<string, StringList> paramDocs = dc->parameters();
            map<string, StringList>::const_iterator q = paramDocs.find(name);
            if(q != paramDocs.end() && !q->second.empty())
            {
                out << nl << "/**";
                out << nl << " * ";
                writeDocCommentLines(out, q->second);
                out << nl << " **/";
            }
        }
        out << nl << "public " << typeToAnnotatedString(p->type(), TypeModeIn, package, p->getMetaData(),
            p->optional(), false) << ' ' << fixKwd(name) << ';';
    }

    ParamDeclList required, optional;
    op->outParameters(required, optional);

    out << sp << nl << "public void write(" << getUnqualified("com.zeroc.Ice.OutputStream", package) << " ostr)";
    out << sb;

    int iter = 0;
    for(const auto& pli : required)
    {
        const string paramName = fixKwd(pli->name());
        writeMarshalUnmarshalCode(out, package, pli->type(), OptionalNone, false, 0, "this." + paramName, true,
                                  iter, "", pli->getMetaData());
    }

    bool checkReturnType = op->returnIsOptional();
    if(ret && !checkReturnType)
    {
        writeMarshalUnmarshalCode(out, package, ret, OptionalNone, false, 0, retval, true, iter, "",
                                  op->getMetaData());
    }

    //
    // Handle optional parameters.
    //
    for(const auto& pli : optional)
    {
        if(checkReturnType && op->returnTag() < pli->tag())
        {
            writeMarshalUnmarshalCode(out, package, ret, OptionalReturnParam, true, op->returnTag(), retval, true,
                                      iter, "", op->getMetaData());
            checkReturnType = false;
        }

        const string paramName = fixKwd(pli->name());
        writeMarshalUnmarshalCode(out, package, pli->type(), OptionalOutParam, true, pli->tag(),
                                  "this." + paramName, true, iter, "", pli->getMetaData());
    }

    if(checkReturnType)
    {
        writeMarshalUnmarshalCode(out, package, ret, OptionalReturnParam, true, op->returnTag(), retval, true, iter,
                                  "", op->getMetaData());
    }

    out << eb;

    out << sp << nl << "public void read(" << getUnqualified("com.zeroc.Ice.InputStream", package) << " istr)";
    out << sb;

    iter = 0;
    for(const auto& pli : required)
    {
        const string paramName = fixKwd(pli->name());
        const string patchParams = getPatcher(pli->type(), package, "this." + paramName);
        writeMarshalUnmarshalCode(out, package, pli->type(), OptionalNone, false, 0, "this." + paramName, false,
                                  iter, "", pli->getMetaData(), patchParams);
    }

    checkReturnType = op->returnIsOptional();
    if(ret && !checkReturnType)
    {
        const string patchParams = getPatcher(ret, package, retval);
        writeMarshalUnmarshalCode(out, package, ret, OptionalNone, false, 0, retval, false, iter, "",
                                  op->getMetaData(), patchParams);
    }

    //
    // Handle optional parameters.
    //

    for(const auto& pli : optional)
    {
        if(checkReturnType && op->returnTag() < pli->tag())
        {
            const string patchParams = getPatcher(ret, package, retval);
            writeMarshalUnmarshalCode(out, package, ret, OptionalReturnParam, true, op->returnTag(), retval, false,
                                      iter, "", op->getMetaData(), patchParams);
            checkReturnType = false;
        }

        const string paramName = fixKwd(pli->name());
        const string patchParams = getPatcher(pli->type(), package, paramName);
        writeMarshalUnmarshalCode(out, package, pli->type(), OptionalOutParam, true, pli->tag(),
                                  "this." + paramName, false, iter, "", pli->getMetaData(), patchParams);
    }

    if(checkReturnType)
    {
        const string patchParams = getPatcher(ret, package, retval);
        writeMarshalUnmarshalCode(out, package, ret, OptionalReturnParam, true, op->returnTag(), retval, false,
                                  iter, "", op->getMetaData(), patchParams);
    }

    out << eb;
    out << eb;
}

void
Slice::JavaVisitor::writeMarshaledResultType(Output& out, const OperationPtr& op, const string& package,
                                             const CommentPtr& dc)
{
    string opName = op->name();
    const TypePtr ret = op->returnType();
    const ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());
    assert(cl);
    opName[0] = static_cast<char>(toupper(static_cast<unsigned char>(opName[0])));

    out << sp;
    writeDocComment(out, "Holds the marshaled result of operation " + op->name() + ".");
    out << nl << "public static class " << opName << "MarshaledResult implements "
        << getUnqualified("com.zeroc.Ice.MarshaledResult", package) << sb;

    const ParamDeclList outParams = op->outParameters();
    const string retval = getEscapedParamName(op, "returnValue");
    const string currentParamName = getEscapedParamName(op, "current");
    const string currentParam = getUnqualified("com.zeroc.Ice.Current", package) + " " + currentParamName;

    out << sp;

    //
    // Emit a doc comment for the constructor if necessary.
    //
    if(dc)
    {
        out << nl << "/**";
        out << nl << " * This constructor marshals the results of operation " << op->name() << " immediately.";

        if(ret && !dc->returns().empty())
        {
            out << nl << " * @param " << retval << ' ';
            writeDocCommentLines(out, dc->returns());
        }
        map<string, StringList> paramDocs = dc->parameters();
        for(const auto& p : outParams)
        {
            const string name = p->name();
            map<string, StringList>::const_iterator q = paramDocs.find(name);
            if(q != paramDocs.end() && !q->second.empty())
            {
                out << nl << " * @param " << fixKwd(q->first) << ' ';
                writeDocCommentLines(out, q->second);
            }
        }
        out << nl << " * @param " << currentParamName << " The Current object for the invocation.";
        out << nl << " **/";
    }

    out << nl << "public " << opName << "MarshaledResult" << spar;
    if(ret)
    {
        out << (typeToAnnotatedString(ret, TypeModeIn, package, op->getMetaData(), op->returnIsOptional())
                + " " + retval);
    }
    for(const auto& p : outParams)
    {
        out << (typeToAnnotatedString(p->type(), TypeModeIn, package, p->getMetaData(), p->optional())
                + " " + fixKwd((p->name())));
    }
    out << currentParam << epar;
    out << sb;
    out << nl << "_ostr = " << getUnqualified("com.zeroc.IceInternal.Incoming", package) << ".createResponseOutputStream("
        << currentParamName << ");";
    out << nl << "_ostr.startEncapsulation(" << currentParamName << ".encoding, " << opFormatTypeToString(op) << ");";

    ParamDeclList required, optional;
    op->outParameters(required, optional);
    int iter = 0;
    for(const auto& pli : required)
    {
        const string paramName = fixKwd(pli->name());
        writeMarshalUnmarshalCode(out, package, pli->type(), OptionalNone, false, 0, paramName, true, iter,
                                  "_ostr", pli->getMetaData());
    }

    bool checkReturnType = op->returnIsOptional();
    if(ret && !checkReturnType)
    {
        writeMarshalUnmarshalCode(out, package, ret, OptionalNone, false, 0, retval, true, iter, "_ostr",
                                  op->getMetaData());
    }

    //
    // Handle optional parameters.
    //

    for(const auto& pli : optional)
    {
        if(checkReturnType && op->returnTag() < pli->tag())
        {
            writeMarshalUnmarshalCode(out, package, ret, OptionalReturnParam, true, op->returnTag(), retval, true,
                                      iter, "_ostr", op->getMetaData());
            checkReturnType = false;
        }

        const string paramName = fixKwd(pli->name());
        writeMarshalUnmarshalCode(out, package, pli->type(), OptionalOutParam, true, pli->tag(), paramName,
                                  true, iter, "_ostr", pli->getMetaData());
    }

    if(checkReturnType)
    {
        writeMarshalUnmarshalCode(out, package, ret, OptionalReturnParam, true, op->returnTag(), retval, true, iter,
                                  "_ostr", op->getMetaData());
    }

    if(op->returnsClasses(false))
    {
        out << nl << "_ostr.writePendingValues();";
    }

    out << nl << "_ostr.endEncapsulation();";

    out << eb;

    out << sp;
    out << nl << "@Override"
        << nl << "public " << getUnqualified("com.zeroc.Ice.OutputStream", package) << " getOutputStream()"
        << sb
        << nl << "return _ostr;"
        << eb;

    out << sp;
    out << nl << "private " << getUnqualified("com.zeroc.Ice.OutputStream", package) << " _ostr;";
    out << eb;
}

void
Slice::JavaVisitor::allocatePatcher(Output& out, const TypePtr& type, const string& package, const string& name,
                                    bool optionalMapping)
{
    BuiltinPtr b = BuiltinPtr::dynamicCast(type);
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    assert((b && b->usesClasses()) || cl);

    string clsName;
    if(b || cl->isInterface())
    {
        clsName = getUnqualified("com.zeroc.Ice.Value", package);
    }
    else
    {
        clsName = getUnqualified(cl, package);
    }
    if(optionalMapping)
    {
        clsName = addAnnotation(clsName, "@Nullable");
    }
    out << nl << "final com.zeroc.IceInternal.Holder<" << clsName << "> "
        << name << " = new com.zeroc.IceInternal.Holder<>();";
}

string
Slice::JavaVisitor::getPatcher(const TypePtr& type, const string& package, const string& dest)
{
    BuiltinPtr b = BuiltinPtr::dynamicCast(type);
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    ostringstream ostr;
    if((b && b->usesClasses()) || cl)
    {
        string clsName;
        if(b || cl->isInterface())
        {
            clsName = getUnqualified("com.zeroc.Ice.Value", package);
        }
        else
        {
            clsName = getUnqualified(cl, package);
        }
        ostr << "v -> " << dest << " = v, " << clsName << ".class";
    }
    return ostr.str();
}

string
Slice::JavaVisitor::getFutureType(const OperationPtr& op, const string& package)
{
    if(op->returnType() || op->outParameters().size() > 0)
    {
        return "java.util.concurrent.CompletableFuture<" + getResultType(op, package, true, false) + ">";
    }
    else
    {
        return "java.util.concurrent.CompletableFuture<Void>";
    }
}

string
Slice::JavaVisitor::getFutureImplType(const OperationPtr& op, const string& package)
{
    if(op->returnType() || op->outParameters().size() > 0)
    {
        return "com.zeroc.IceInternal.OutgoingAsync<" + getResultType(op, package, true, false) + ">";
    }
    else
    {
        return "com.zeroc.IceInternal.OutgoingAsync<Void>";
    }
}

vector<string>
Slice::JavaVisitor::getParams(const OperationPtr& op, const string& package)
{
    vector<string> params;

    const ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());
    assert(cl);

    const ParamDeclList paramList = op->inParameters();
    for(const auto& q : paramList)
    {
        const string type = typeToAnnotatedString(q->type(), TypeModeIn, package, q->getMetaData(), q->optional());
        params.push_back(type + ' ' + fixKwd(q->name()));
    }

    return params;
}

vector<string>
Slice::JavaVisitor::getParamsProxy(const OperationPtr& op, const string& package, bool optionalMapping, bool internal)
{
    vector<string> params;

    ParamDeclList inParams = op->inParameters();
    for(const auto& q : inParams)
    {
        const string typeString = typeToAnnotatedString(q->type(), TypeModeIn, package, q->getMetaData(),
                                               optionalMapping && q->optional());
        params.push_back(typeString + ' ' + (internal ? "iceP_" + q->name() : fixKwd(q->name())));
    }

    return params;
}

vector<string>
Slice::JavaVisitor::getArgs(const OperationPtr& op)
{
    vector<string> args;

    ParamDeclList paramList = op->parameters();
    for(const auto& q : paramList)
    {
        args.push_back(fixKwd(q->name()));
    }

    return args;
}

vector<string>
Slice::JavaVisitor::getInArgs(const OperationPtr& op, bool internal)
{
    vector<string> args;

    ParamDeclList paramList = op->inParameters();
    for(const auto& q : paramList)
    {
        string s = internal ? "iceP_" + q->name() : fixKwd(q->name());
        args.push_back(s);
    }

    return args;
}

void
Slice::JavaVisitor::writeMarshalProxyParams(Output& out, const string& package, const OperationPtr& op,
                                            bool optionalMapping)
{
    int iter = 0;
    ParamDeclList required, optional;
    op->inParameters(required, optional);
    for(const auto pli : required)
    {
        string paramName = "iceP_" + pli->name();
        writeMarshalUnmarshalCode(out, package, pli->type(), OptionalNone, false, 0, paramName, true,
                                  iter, "", pli->getMetaData());
    }

    //
    // Handle optional parameters.
    //
    for(const auto& pli : optional)
    {
        writeMarshalUnmarshalCode(out, package, pli->type(), OptionalInParam, optionalMapping,
                                  pli->tag(), "iceP_" + pli->name(), true, iter, "", pli->getMetaData());
    }

    if(op->sendsClasses(false))
    {
        out << nl << "ostr.writePendingValues();";
    }
}

void
Slice::JavaVisitor::writeUnmarshalProxyResults(Output& out, const string& package, const OperationPtr& op)
{
    const ParamDeclList outParams = op->outParameters();
    const TypePtr ret = op->returnType();
    const string name = "ret";

    if(op->returnsMultipleValues())
    {
        string resultType = getResultType(op, package, false, false);
        out << nl << resultType << ' ' << name << " = new " << resultType << "();";
        out << nl << name << ".read(istr);";
        if(op->returnsClasses(false))
        {
            out << nl << "istr.readPendingValues();";
        }
        out << nl << "return " << name << ';';
    }
    else
    {
        string resultType = getResultType(op, package, false, false);

        bool optional;
        TypePtr type;
        int tag;
        StringList metaData;
        if(ret)
        {
            type = ret;
            optional = op->returnIsOptional();
            tag = op->returnTag();
            metaData = op->getMetaData();
        }
        else
        {
            assert(outParams.size() == 1);
            optional = outParams.front()->optional();
            type = outParams.front()->type();
            tag = outParams.front()->tag();
            metaData = outParams.front()->getMetaData();
        }

        const bool val = isValue(type);

        int iter = 0;

        if(val)
        {
            allocatePatcher(out, type, package, name, optional);
        }
        else
        {
            out << nl << resultType << ' ' << name << ';';
        }
        string patchParams = getPatcher(type, package, name + ".value");
        if(optional)
        {
            writeMarshalUnmarshalCode(out, package, type, ret ? OptionalReturnParam : OptionalOutParam, true,
                                      tag, name, false, iter, "", metaData, patchParams);
        }
        else
        {
            writeMarshalUnmarshalCode(out, package, type, OptionalNone, false, 0, name, false, iter, "", metaData,
                                      patchParams);
        }

        if(op->returnsClasses(false))
        {
            out << nl << "istr.readPendingValues();";
        }

        if(val)
        {
            out << nl << "return " << name << ".value;";
        }
        else
        {
            out << nl << "return " << name << ';';
        }
    }
}

void
Slice::JavaVisitor::writeMarshalServantResults(Output& out, const string& package, const OperationPtr& op,
                                               const string& param)
{
    if(op->returnsMultipleValues())
    {
        out << nl << param << ".write(ostr);";
    }
    else
    {
        const ParamDeclList params = op->outParameters();
        bool optional;
        OptionalMode mode;
        TypePtr type;
        int tag;
        StringList metaData;
        if(op->returnType())
        {
            type = op->returnType();
            optional = op->returnIsOptional();
            mode = optional ? OptionalReturnParam : OptionalNone;
            tag = op->returnTag();
            metaData = op->getMetaData();
        }
        else
        {
            assert(params.size() == 1);
            optional = params.front()->optional();
            mode = optional ? OptionalOutParam : OptionalNone;
            type = params.front()->type();
            tag = params.front()->tag();
            metaData = params.front()->getMetaData();
        }

        int iter = 0;
        writeMarshalUnmarshalCode(out, package, type, mode, true, tag, param, true, iter, "", metaData);
    }

    if(op->returnsClasses(false))
    {
        out << nl << "ostr.writePendingValues();";
    }
}

void
Slice::JavaVisitor::writeThrowsClause(const string& package, const ExceptionList& throws, const OperationPtr& op)
{
    Output& out = output();

    if(op && (op->hasMetaData("java:UserException") || op->hasMetaData("UserException")))
    {
        out.inc();
        out << nl << "throws " << getUnqualified("com.zeroc.Ice.UserException", package);
        out.dec();
    }
    else if(throws.size() > 0)
    {
        out.inc();
        out << nl << "throws ";
        out.useCurrentPosAsIndent();
        int count = 0;
        for(const auto& r : throws)
        {
            if(count > 0)
            {
                out << "," << nl;
            }
            out << getUnqualified(r, package);
            count++;
        }
        out.restoreIndent();
        out.dec();
    }
}

void
Slice::JavaVisitor::writeMarshalDataMember(Output& out, const string& package, const DataMemberPtr& member, int& iter,
                                           bool forStruct)
{
    if(member->optional())
    {
        assert(!forStruct);
        out << nl << "if(_" << member->name() << ")";
        out << sb;
        writeMarshalUnmarshalCode(out, package, member->type(), OptionalInParam, false, member->tag(),
                                  fixKwd(member->name()), true, iter, "ostr_", member->getMetaData());
        out << eb;
    }
    else
    {
        string stream = forStruct ? "" : "ostr_";
        string memberName = fixKwd(member->name());
        if(forStruct)
        {
            memberName = "this." + memberName;
        }

        writeMarshalUnmarshalCode(out, package, member->type(), OptionalNone, false, 0, memberName,
                                  true, iter, stream, member->getMetaData());
    }
}

void
Slice::JavaVisitor::writeUnmarshalDataMember(Output& out, const string& package, const DataMemberPtr& member, int& iter,
                                             bool forStruct)
{
    // TBD: Handle passing interface-by-value

    const string patchParams = getPatcher(member->type(), package, fixKwd(member->name()));

    if(member->optional())
    {
        assert(!forStruct);
        out << nl << "if(_" << member->name() << " = istr_.readOptional(" << member->tag() << ", "
            << getOptionalFormat(member->type()) << "))";
        out << sb;
        writeMarshalUnmarshalCode(out, package, member->type(), OptionalMember, false, 0, fixKwd(member->name()), false,
                                  iter, "istr_", member->getMetaData(), patchParams);
        out << eb;
    }
    else
    {
        string stream = forStruct ? "" : "istr_";
        string memberName = fixKwd(member->name());
        if(forStruct)
        {
            memberName = "this." + memberName;
        }

        writeMarshalUnmarshalCode(out, package, member->type(), OptionalNone, false, 0, memberName, false,
                                  iter, stream, member->getMetaData(), patchParams);
    }
}

void
Slice::JavaVisitor::writeDispatch(Output& out, const ClassDefPtr& p)
{
    const string name = fixKwd(p->name());
    const string package = getPackage(p);
    const string scoped = p->scoped();
    const ClassList bases = p->bases();
    ClassDefPtr base;
    if(!bases.empty() && !bases.front()->isInterface())
    {
        base = bases.front();
    }
    const OperationList ops = p->operations();

    for(const auto& op : ops)
    {
        CommentPtr dc = op->parseComment(false);

        //
        // The "MarshaledResult" type is generated in the servant interface.
        //
        if(!p->isInterface() && op->hasMarshaledResult())
        {
            writeMarshaledResultType(out, op, package, dc);
        }

        vector<string> params = getParams(op, package);
        const string currentParam = getUnqualified("com.zeroc.Ice.Current", package) + " " +
            getEscapedParamName(op, "current");

        const bool amd = p->hasMetaData("amd") || op->hasMetaData("amd");

        ExceptionList throws = op->throws();
        throws.sort();
        throws.unique();

        out << sp;
        writeServantDocComment(out, op, package, dc, amd);
        if(dc && dc->isDeprecated())
        {
            out << nl << "@Deprecated";
        }

        if(amd)
        {
            out << nl << "java.util.concurrent.CompletionStage<" << getResultType(op, package, true, true) << "> "
                << op->name() << "Async" << spar << params << currentParam << epar;
            writeThrowsClause(package, throws, op);
            out << ';';
        }
        else
        {
            out << nl << getResultType(op, package, false, true) << ' ' << fixKwd(op->name()) << spar << params
                << currentParam << epar;
            writeThrowsClause(package, throws, op);
            out << ';';
        }
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
#ifndef NDEBUG
    StringList::const_iterator scopedIter = find(ids.begin(), ids.end(), scoped);
    assert(scopedIter != ids.end());
#endif

    out << sp;
    writeHiddenDocComment(out);
    out << nl << "static final String[] _iceIds =";
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

    out << sp << nl << "@Override" << nl << "default String[] ice_ids(" << getUnqualified("com.zeroc.Ice.Current", package)
        << " current)";
    out << sb;
    out << nl << "return _iceIds;";
    out << eb;

    out << sp << nl << "@Override" << nl << "default String ice_id(" << getUnqualified("com.zeroc.Ice.Current", package)
        << " current)";
    out << sb;
    out << nl << "return ice_staticId();";
    out << eb;

    out << sp << nl;
    out << "static String ice_staticId()";
    out << sb;
    if(p->isInterface())
    {
        out << nl << "return \"" << p->scoped() << "\";";
    }
    else
    {
        out << nl << "return " << fixKwd(p->name()) << ".ice_staticId();";
    }
    out << eb;

    //
    // Dispatch methods. We only generate methods for operations
    // defined in this ClassDef, because we reuse existing methods
    // for inherited operations.
    //
    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        StringList opMetaData = op->getMetaData();

        CommentPtr dc = op->parseComment(false);

        string opName = op->name();
        out << sp;

        out << nl << "/**";
        out << nl << " * @hidden";
        out << nl << " * @param obj -";
        out << nl << " * @param inS -";
        out << nl << " * @param current -";
        out << nl << " * @return -";
        if(!op->throws().empty() || op->hasMetaData("java:UserException") || op->hasMetaData("UserException"))
        {
            out << nl << " * @throws " << getUnqualified("com.zeroc.Ice.UserException", package) << " -";
        }
        out << nl << "**/";
        if(dc && dc->isDeprecated())
        {
            out << nl << "@Deprecated";
        }
        out << nl << "static java.util.concurrent.CompletionStage<"
            << getUnqualified("com.zeroc.Ice.OutputStream", package)
            << "> _iceD_" << opName << '(';
        if(p->isInterface())
        {
            out << name;
        }
        else
        {
            out << p->name() << "Disp";
        }
        out << " obj, final " << getUnqualified("com.zeroc.IceInternal.Incoming", package) << " inS, "
            << getUnqualified("com.zeroc.Ice.Current", package) << " current)";
        if(!op->throws().empty() || op->hasMetaData("java:UserException") || op->hasMetaData("UserException"))
        {
            out.inc();
            out << nl << "throws " << getUnqualified("com.zeroc.Ice.UserException", package);
            out.dec();
        }
        out << sb;

        const bool amd = p->hasMetaData("amd") || op->hasMetaData("amd");

        const TypePtr ret = op->returnType();

        const ParamDeclList inParams = op->inParameters();
        const ParamDeclList outParams = op->outParameters();

        out << nl << getUnqualified("com.zeroc.Ice.Object", package)
            << "._iceCheckMode(" << sliceModeToIceMode(op->mode())
            << ", current.mode);";

        if(!inParams.empty())
        {
            ParamDeclList values;

            //
            // Declare 'in' parameters.
            //
            out << nl << getUnqualified("com.zeroc.Ice.InputStream", package) << " istr = inS.startReadParams();";
            for(const auto& pli : inParams)
            {
                const TypePtr paramType = pli->type();
                if(isValue(paramType))
                {
                    allocatePatcher(out, paramType, package, "icePP_" + pli->name(), pli->optional());
                    values.push_back(pli);
                }
                else
                {
                    const string paramName = "iceP_" + pli->name();
                    const string typeS = typeToAnnotatedString(paramType, TypeModeIn, package, pli->getMetaData(),
                                                               pli->optional());
                    out << nl << typeS << ' ' << paramName << ';';
                }
            }

            //
            // Unmarshal 'in' parameters.
            //
            ParamDeclList required, optional;
            op->inParameters(required, optional);
            int iter = 0;
            for(const auto& pli : required)
            {
                const string paramName = isValue(pli->type()) ? ("icePP_" + pli->name()) : "iceP_" + pli->name();
                const string patchParams = getPatcher(pli->type(), package, paramName + ".value");
                writeMarshalUnmarshalCode(out, package, pli->type(), OptionalNone, false, 0, paramName, false,
                                          iter, "", pli->getMetaData(), patchParams);
            }
            for(const auto& pli : optional)
            {
                const string paramName = isValue(pli->type()) ? ("icePP_" + pli->name()) : "iceP_" + pli->name();
                const string patchParams = getPatcher(pli->type(), package, paramName + ".value");
                writeMarshalUnmarshalCode(out, package, pli->type(), OptionalInParam, true, pli->tag(),
                                          paramName, false, iter, "", pli->getMetaData(), patchParams);
            }
            if(op->sendsClasses(false))
            {
                out << nl << "istr.readPendingValues();";
            }
            out << nl << "inS.endReadParams();";

            for(const auto& pli : values)
            {
                const string typeS = typeToAnnotatedString(pli->type(), TypeModeIn, package, pli->getMetaData(),
                                                           pli->optional());
                out << nl << typeS << ' ' << "iceP_" << pli->name() << " = icePP_" << pli->name() << ".value;";
            }
        }
        else
        {
            out << nl << "inS.readEmptyParams();";
        }

        if(op->format() != DefaultFormat)
        {
            out << nl << "inS.setFormat(" << opFormatTypeToString(op) << ");";
        }

        if(amd)
        {
            if(op->hasMarshaledResult())
            {
                out << nl << "return inS.setMarshaledResultFuture(obj." << opName << "Async" << spar
                    << getInArgs(op, true) << "current" << epar << ");";
            }
            else
            {
                out << nl << "return inS.setResultFuture(obj." << opName << "Async" << spar << getInArgs(op, true)
                    << "current" << epar;
                if(ret || !outParams.empty())
                {
                    out << ", (ostr, ret) ->";
                    out.inc();
                    out << sb;
                    writeMarshalServantResults(out, package, op, "ret");
                    out << eb;
                    out.dec();
                }
                out << ");";
            }
        }
        else
        {
            //
            // Call on the servant.
            //
            out << nl;
            if(ret || !outParams.empty())
            {
                out << getResultType(op, package, false, true) << " ret = ";
            }
            out << "obj." << fixKwd(opName) << spar << getInArgs(op, true) << "current" << epar << ';';

            //
            // Marshal 'out' parameters and return value.
            //
            if(op->hasMarshaledResult())
            {
                out << nl << "return inS.setMarshaledResult(ret);";
            }
            else if(ret || !outParams.empty())
            {
                out << nl << getUnqualified("com.zeroc.Ice.OutputStream", package) << " ostr = inS.startWriteParams();";
                writeMarshalServantResults(out, package, op, "ret");
                out << nl << "inS.endWriteParams(ostr);";
                out << nl << "return inS.setResult(ostr);";
            }
            else
            {
                out << nl << "return inS.setResult(inS.writeEmptyParams());";
            }
        }

        out << eb;
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

        out << sp;
        writeHiddenDocComment(out);
        out << nl << "final static String[] _iceOps =";
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
        writeHiddenDocComment(out);
        for(const auto& op : allOps)
        {
            //
            // Suppress deprecation warnings if this method dispatches to a deprecated operation.
            //
            ContainerPtr container = op->container();
            ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
            assert(cl);
            if(isDeprecated(op, cl))
            {
                out << nl << "@SuppressWarnings(\"deprecation\")";
                break;
            }
        }
        out << nl << "@Override" << nl
            << "default java.util.concurrent.CompletionStage<" << getUnqualified("com.zeroc.Ice.OutputStream", package)
            << "> _iceDispatch(" << getUnqualified("com.zeroc.IceInternal.Incoming", package) << " in, "
            << getUnqualified("com.zeroc.Ice.Current", package) << " current)";
        out.inc();
        out << nl << "throws " << getUnqualified("com.zeroc.Ice.UserException", package);
        out.dec();
        out << sb;
        out << nl << "int pos = java.util.Arrays.binarySearch(_iceOps, current.operation);";
        out << nl << "if(pos < 0)";
        out << sb;
        out << nl << "throw new "
            << getUnqualified("com.zeroc.Ice.OperationNotExistException", package)
            << "(current.id, current.facet, current.operation);";
        out << eb;
        out << sp << nl << "switch(pos)";
        out << sb;
        int i = 0;
        for(const auto& opName : allOpNames)
        {
            out << nl << "case " << i++ << ':';
            out << sb;
            if(opName == "ice_id")
            {
                out << nl << "return " << getUnqualified("com.zeroc.Ice.Object", package)
                    << "._iceD_ice_id(this, in, current);";
            }
            else if(opName == "ice_ids")
            {
                out << nl << "return " << getUnqualified("com.zeroc.Ice.Object", package)
                    << "._iceD_ice_ids(this, in, current);";
            }
            else if(opName == "ice_isA")
            {
                out << nl << "return " << getUnqualified("com.zeroc.Ice.Object", package)
                    << "._iceD_ice_isA(this, in, current);";
            }
            else if(opName == "ice_ping")
            {
                out << nl << "return " << getUnqualified("com.zeroc.Ice.Object", package)
                    << "._iceD_ice_ping(this, in, current);";
            }
            else
            {
                //
                // There's probably a better way to do this.
                //
                for(const auto& t : allOps)
                {
                    if(t->name() == opName)
                    {
                        ContainerPtr container = t->container();
                        ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
                        assert(cl);
                        if(cl->scoped() == p->scoped())
                        {
                            out << nl << "return _iceD_" << opName << "(this, in, current);";
                        }
                        else
                        {
                            string baseName;
                            if(cl->isInterface())
                            {
                                baseName = getUnqualified(cl, package);
                            }
                            else
                            {
                                baseName = getUnqualified(cl, package, "", "Disp");
                            }
                            out << nl << "return " << baseName << "._iceD_" << opName << "(this, in, current);";
                        }
                        break;
                    }
                }
            }
            out << eb;
        }
        out << eb;
        out << sp << nl << "assert(false);";
        out << nl << "throw new " << getUnqualified("com.zeroc.Ice.OperationNotExistException", package)
            << "(current.id, current.facet, current.operation);";
        out << eb;
    }
}

void
Slice::JavaVisitor::writeMarshaling(Output& out, const ClassDefPtr& p)
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

    int iter;
    DataMemberList members = p->dataMembers();
    DataMemberList optionalMembers = p->orderedOptionalDataMembers();
    bool basePreserved = p->inheritsMetaData("preserve-slice");
    bool preserved = p->hasMetaData("preserve-slice");

    if(preserved && !basePreserved)
    {
        out << sp;
        writeHiddenDocComment(out);
        out << nl << "@Override";
        out << nl << "public " << getUnqualified("com.zeroc.Ice.SlicedData", package) << " ice_getSlicedData()";
        out << sb;
        out << nl << "return _iceSlicedData;";
        out << eb;

        out << sp;
        writeHiddenDocComment(out);
        out << nl << "@Override";
        out << nl << "public void _iceWrite(" << getUnqualified("com.zeroc.Ice.OutputStream", package) << " ostr)";
        out << sb;
        out << nl << "ostr.startValue(_iceSlicedData);";
        out << nl << "_iceWriteImpl(ostr);";
        out << nl << "ostr.endValue();";
        out << eb;

        out << sp;
        writeHiddenDocComment(out);
        out << nl << "@Override";
        out << nl << "public void _iceRead(" << getUnqualified("com.zeroc.Ice.InputStream", package) << " istr)";
        out << sb;
        out << nl << "istr.startValue();";
        out << nl << "_iceReadImpl(istr);";
        out << nl << "_iceSlicedData = istr.endValue(true);";
        out << eb;
    }

    out << sp;
    writeHiddenDocComment(out);
    out << nl << "@Override";
    out << nl << "protected void _iceWriteImpl(" << getUnqualified("com.zeroc.Ice.OutputStream", package) << " ostr_)";
    out << sb;
    out << nl << "ostr_.startSlice(ice_staticId(), " << p->compactId() << (!base ? ", true" : ", false") << ");";
    iter = 0;
    for(const auto& d : members)
    {
        if(!d->optional())
        {
            writeMarshalDataMember(out, package, d, iter);
        }
    }
    for(const auto& d : optionalMembers)
    {
        writeMarshalDataMember(out, package, d, iter);
    }
    out << nl << "ostr_.endSlice();";
    if(base)
    {
        out << nl << "super._iceWriteImpl(ostr_);";
    }
    out << eb;

    DataMemberList classMembers = p->classDataMembers();
    DataMemberList allClassMembers = p->allClassDataMembers();

    out << sp;
    writeHiddenDocComment(out);
    out << nl << "@Override";
    out << nl << "protected void _iceReadImpl(" << getUnqualified("com.zeroc.Ice.InputStream", package) << " istr_)";
    out << sb;
    out << nl << "istr_.startSlice();";

    iter = 0;
    for(const auto& d : members)
    {
        if(!d->optional())
        {
            writeUnmarshalDataMember(out, package, d, iter);
        }
    }
    for(const auto& d : optionalMembers)
    {
        writeUnmarshalDataMember(out, package, d, iter);
    }

    out << nl << "istr_.endSlice();";
    if(base)
    {
        out << nl << "super._iceReadImpl(istr_);";
    }
    out << eb;

    if(preserved && !basePreserved)
    {
        out << sp;
        writeHiddenDocComment(out);
        out << nl << "protected " << getUnqualified("com.zeroc.Ice.SlicedData", package) << " _iceSlicedData;";
    }
}

void
Slice::JavaVisitor::writeConstantValue(Output& out, const TypePtr& type, const SyntaxTreeBasePtr& valueType,
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
Slice::JavaVisitor::writeDataMemberInitializers(Output& out, const DataMemberList& members, const string& package)
{
    for(const auto& p : members)
    {
        TypePtr t = p->type();
        if(p->defaultValueType())
        {
            if(p->optional())
            {
                string capName = p->name();
                capName[0] = static_cast<char>(toupper(static_cast<unsigned char>(capName[0])));
                out << nl << "set" << capName << '(';
                writeConstantValue(out, t, p->defaultValueType(), p->defaultValue(), package);
                out << ");";
            }
            else
            {
                out << nl << "this." << fixKwd(p->name()) << " = ";
                writeConstantValue(out, t, p->defaultValueType(), p->defaultValue(), package);
                out << ';';
            }
        }
        else
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(t);
            if(builtin && builtin->kind() == Builtin::KindString)
            {
                out << nl << "this." << fixKwd(p->name()) << " = \"\";";
            }

            EnumPtr en = EnumPtr::dynamicCast(t);
            if(en)
            {
                string firstEnum = fixKwd(en->enumerators().front()->name());
                out << nl << "this." << fixKwd(p->name()) << " = " << getUnqualified(en, package) << '.' << firstEnum << ';';
            }

            StructPtr st = StructPtr::dynamicCast(t);
            if(st)
            {
                string memberType = typeToString(st, TypeModeMember, package, p->getMetaData());
                out << nl << "this." << fixKwd(p->name()) << " = new " << memberType << "();";
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
Slice::JavaVisitor::writeHiddenDocComment(Output& out)
{
    out << nl << "/** @hidden */";
}

void
Slice::JavaVisitor::writeDocCommentLines(Output& out, const StringList& lines)
{
    //
    // This method emits a block of text, prepending a leading " * " to the second and
    // subsequent lines. We assume the caller prepended a leading " * " for the first
    // line if necessary.
    //
    assert(!lines.empty());
    StringList l = lines;
    out << l.front();
    l.pop_front();
    for(const auto& p : l)
    {
        out << nl << " *";
        if(!p.empty())
        {
            out << " " << p;
        }
    }
}

void
Slice::JavaVisitor::writeDocCommentLines(Output& out, const string& text)
{
    //
    // This method emits a block of text, prepending a leading " * " to the second and
    // subsequent lines. We assume the caller prepended a leading " * " for the first
    // line if necessary.
    //
    string::size_type start = 0;
    string::size_type pos;
    const string ws = "\n";
    pos = text.find_first_of(ws);
    if(pos == string::npos)
    {
        out << text;
    }
    else
    {
        string s = IceUtilInternal::trim(text.substr(start, pos - start));
        out << s; // Emit the first line.
        start = pos + 1;
        while((pos = text.find_first_of(ws, start)) != string::npos)
        {
            string line = IceUtilInternal::trim(text.substr(start, pos - start));
            if(line.empty())
            {
                out << nl << " *";
            }
            else
            {
                out << nl << " * " << line;
            }
            start = pos + 1;
        }
        if(start < text.size())
        {
            string line = IceUtilInternal::trim(text.substr(start));
            if(line.empty())
            {
                out << nl << " *";
            }
            else
            {
                out << nl << " * " << line;
            }
        }
    }
}

void
Slice::JavaVisitor::writeDocComment(Output& out, const UnitPtr& unt, const CommentPtr& dc)
{
    if(!dc)
    {
        return;
    }

    out << nl << "/**";
    if(!dc->overview().empty())
    {
        out << nl << " * ";
        writeDocCommentLines(out, dc->overview());
    }

    if(!dc->misc().empty())
    {
        out << nl << " * ";
        writeDocCommentLines(out, dc->misc());
    }

    if(!dc->seeAlso().empty())
    {
        out << nl << " *";
        StringList sa = dc->seeAlso();
        for(const auto& p : sa)
        {
            out << nl << " * @see ";
            writeSeeAlso(out, unt, p);
        }
    }

    if(!dc->deprecated().empty())
    {
        out << nl << " * @deprecated ";
        writeDocCommentLines(out, dc->deprecated());
    }
    else if(dc->isDeprecated())
    {
        out << nl << " * @deprecated";
    }

    out << nl << " **/";
}

void
Slice::JavaVisitor::writeDocComment(Output& out, const string& text)
{
    if(!text.empty())
    {
        out << nl << "/**";
        out << nl << " * ";
        writeDocCommentLines(out, text);
        out << nl << " **/";
    }
}

void
Slice::JavaVisitor::writeProxyDocComment(Output& out, const OperationPtr& p, const string& package,
                                         const CommentPtr& dc, bool async, const string& contextParam)
{
    if(!dc)
    {
        return;
    }

    map<string, StringList> paramDocs = dc->parameters();

    out << nl << "/**";
    if(!dc->overview().empty())
    {
        out << nl << " * ";
        writeDocCommentLines(out, dc->overview());
    }

    //
    // Show in-params in order of declaration, but only those with docs.
    //
    const ParamDeclList paramList = p->inParameters();
    for(const auto& i : paramList)
    {
        const string name = i->name();
        map<string, StringList>::const_iterator j = paramDocs.find(name);
        if(j != paramDocs.end() && !j->second.empty())
        {
            out << nl << " * @param " << fixKwd(j->first) << ' ';
            writeDocCommentLines(out, j->second);
        }
    }
    if(!contextParam.empty())
    {
        out << nl << " * " << contextParam;
    }

    //
    // Handle the return value (if any).
    //
    if(p->returnsMultipleValues())
    {
        const string r = getResultType(p, package, true, false);
        if(async)
        {
            out << nl << " * @return A future that will be completed with an instance of " << r << '.';
        }
        else
        {
            out << nl << " * @return An instance of " << r << '.';
        }
    }
    else if(p->returnType())
    {
        if(!dc->returns().empty())
        {
            out << nl << " * @return ";
            writeDocCommentLines(out, dc->returns());
        }
        else if(async)
        {
            out << nl << " * @return A future that will be completed with the result.";
        }
    }
    else if(!p->outParameters().empty())
    {
        assert(p->outParameters().size() == 1);
        const ParamDeclPtr param = p->outParameters().front();
        map<string, StringList>::const_iterator j = paramDocs.find(param->name());
        if(j != paramDocs.end() && !j->second.empty())
        {
            out << nl << " * @return ";
            writeDocCommentLines(out, j->second);
        }
        else if(async)
        {
            out << nl << " * @return A future that will be completed with the result.";
        }
    }
    else if(async)
    {
        //
        // No results but an async proxy operation still returns a future.
        //
        out << nl << " * @return A future that will be completed when the invocation completes.";
    }

    //
    // Async proxy methods don't declare user exceptions.
    //
    if(!async)
    {
        map<string, StringList> exDocs = dc->exceptions();
        for(const auto& i : exDocs)
        {
            out << nl << " * @throws " << fixKwd(i.first) << ' ';
            writeDocCommentLines(out, i.second);
        }
    }

    if(!dc->misc().empty())
    {
        out << nl << " * ";
        writeDocCommentLines(out, dc->misc());
    }

    if(!dc->seeAlso().empty())
    {
        out << nl << " *";
        StringList sa = dc->seeAlso();
        for(StringList::iterator q = sa.begin(); q != sa.end(); ++q)
        {
            out << nl << " * @see ";
            writeSeeAlso(out, p->unit(), *q);
        }
    }

    if(!dc->deprecated().empty())
    {
        out << nl << " * @deprecated ";
        writeDocCommentLines(out, dc->deprecated());
    }
    else if(dc->isDeprecated())
    {
        out << nl << " * @deprecated";
    }

    out << nl << " **/";
}

void
Slice::JavaVisitor::writeHiddenProxyDocComment(Output& out, const OperationPtr& p)
{
    // the _iceI_ methods are all async

    out << nl << "/**";
    out << nl << " * @hidden";

    //
    // Show in-params in order of declaration
    //
    const ParamDeclList paramList = p->inParameters();
    for(const auto& i : paramList)
    {
        const string name = i->name();
        out << nl << " * @param " << "iceP_" << name << " -";
    }
    out << nl << " * @param context -";
    out << nl << " * @param sync -";

    //
    // There is always a return value since it's async
    //
    out << nl << " * @return -";

    // No throws since it's async
    out << nl << " **/";
}

void
Slice::JavaVisitor::writeServantDocComment(Output& out, const OperationPtr& p, const string& package,
                                           const CommentPtr& dc, bool async)
{
    if(!dc)
    {
        return;
    }

    map<string, StringList> paramDocs = dc->parameters();
    const ParamDeclList paramList = p->inParameters();
    const string currentParamName = getEscapedParamName(p, "current");
    const string currentParam = " * @param " + currentParamName + " The Current object for the invocation.";

    out << nl << "/**";
    if(!dc->overview().empty())
    {
        out << nl << " * ";
        writeDocCommentLines(out, dc->overview());
    }

    //
    // Show in-params in order of declaration, but only those with docs.
    //
    for(const auto& i : paramList)
    {
        const string name = i->name();
        map<string, StringList>::const_iterator j = paramDocs.find(name);
        if(j != paramDocs.end() && !j->second.empty())
        {
            out << nl << " * @param " << fixKwd(j->first) << ' ';
            writeDocCommentLines(out, j->second);
        }
    }
    out << nl << currentParam;

    //
    // Handle the return value (if any).
    //
    if(p->returnsMultipleValues())
    {
        const string r = getResultType(p, package, true, false);
        if(async)
        {
            out << nl << " * @return A completion stage that the servant will complete with an instance of " << r
                << '.';
        }
        else
        {
            out << nl << " * @return An instance of " << r << '.';
        }
    }
    else if(p->returnType())
    {
        if(!dc->returns().empty())
        {
            out << nl << " * @return ";
            writeDocCommentLines(out, dc->returns());
        }
        else if(async)
        {
            out << nl << " * @return A completion stage that the servant will complete with the result.";
        }
    }
    else if(!p->outParameters().empty())
    {
        assert(p->outParameters().size() == 1);
        const ParamDeclPtr param = p->outParameters().front();
        map<string, StringList>::const_iterator j = paramDocs.find(param->name());
        if(j != paramDocs.end() && !j->second.empty())
        {
            out << nl << " * @return ";
            writeDocCommentLines(out, j->second);
        }
        else if(async)
        {
            out << nl << " * @return A completion stage that the servant will complete with the result.";
        }
    }
    else if(async)
    {
        //
        // No results but an async operation still returns a completion stage.
        //
        out << nl << " * @return A completion stage that the servant will complete when the invocation completes.";
    }

    if(p->hasMetaData("java:UserException") || p->hasMetaData("UserException"))
    {
        out << nl << " * @throws " << getUnqualified("com.zeroc.Ice.UserException", package);
    }
    else
    {
        map<string, StringList> exDocs = dc->exceptions();
        for(const auto& i : exDocs)
        {
            out << nl << " * @throws " << fixKwd(i.first) << ' ';
            writeDocCommentLines(out, i.second);
        }
    }

    if(!dc->misc().empty())
    {
        out << nl << " * ";
        writeDocCommentLines(out, dc->misc());
    }

    if(!dc->seeAlso().empty())
    {
        out << nl << " *";
        StringList sa = dc->seeAlso();
        for(auto& q : sa)
        {
            out << nl << " * @see ";
            writeSeeAlso(out, p->unit(), q);
        }
    }

    if(!dc->deprecated().empty())
    {
        out << nl << " * @deprecated ";
        writeDocCommentLines(out, dc->deprecated());
    }
    else if(dc->isDeprecated())
    {
        out << nl << " * @deprecated";
    }

    out << nl << " **/";
}

void
Slice::JavaVisitor::writeSeeAlso(Output& out, const UnitPtr& unt, const string& ref)
{
    assert(!ref.empty());

    //
    // Try to look up the referenced type. If we find it, we translate it into a fully-scoped Java type.
    //

    string s = ref;
    string::size_type pos = s.find('#');
    string rest;
    if(pos != string::npos)
    {
        rest = s.substr(pos);
        s = s.substr(0, pos);
    }

    pos = 0;
    while((pos = s.find(".", pos)) != string::npos)
    {
        s.replace(pos, 1, "::");
    }

    //
    // We assume a scoped name should be an absolute name.
    //
    if(s.find(":") != string::npos && s[0] != ':')
    {
        s.insert(0, "::");
    }

    TypeList l = unt->lookupTypeNoBuiltin(s, false, true);
    if(l.empty())
    {
        out << ref;
    }
    else
    {
        ContainedPtr cont = ContainedPtr::dynamicCast(l.front());
        assert(cont);
        out << getUnqualified(cont) << rest;
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
Slice::Gen::generate(const UnitPtr& p)
{
    JavaGenerator::validateMetaData(p);

    PackageVisitor packageVisitor(_dir);
    p->visit(&packageVisitor, false);

    TypesVisitor typesVisitor(_dir);
    p->visit(&typesVisitor, false);

    CompactIdVisitor compactIdVisitor(_dir);
    p->visit(&compactIdVisitor, false);

    HelperVisitor helperVisitor(_dir);
    p->visit(&helperVisitor, false);

    ProxyVisitor proxyVisitor(_dir);
    p->visit(&proxyVisitor, false);

    DispatcherVisitor dispatcherVisitor(_dir);
    p->visit(&dispatcherVisitor, false);
}

void
Slice::Gen::generateImpl(const UnitPtr& p)
{
    ImplVisitor implVisitor(_dir);
    p->visit(&implVisitor, false);
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

Slice::Gen::TypesVisitor::TypesVisitor(const string& dir) :
    JavaVisitor(dir)
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
    string absolute = getUnqualified(p);
    DataMemberList members = p->dataMembers();
    DataMemberList allDataMembers = p->allDataMembers();

    open(absolute, p->file());

    Output& out = output();

    //
    // Check for annotated types to import the corresponding annotations.
    //
    bool hasOptionals = false;
    bool hasRequiredDataMembers = false;
    for(const auto& d : p->allDataMembers())
    {
        if(d->optional())
        {
            hasOptionals = true;
            if(hasRequiredDataMembers)
            {
                break;
            }
        }
        else
        {
            hasRequiredDataMembers = true;
            if(hasOptionals)
            {
                break;
            }
        }
    }
    for(const auto& op : p->allOperations())
    {
        if(op->sendsOptionals() || op->returnIsOptional())
        {
            hasOptionals = true;
            break;
        }

        for(const auto& q : op->outParameters())
        {
            if(q->optional())
            {
                hasOptionals = true;
                break;
            }
        }
    }

    if(hasOptionals || hasRequiredDataMembers)
    {
        out << sp;
        if(hasRequiredDataMembers)
        {
            out << nl << "import org.checkerframework.checker.nullness.qual.MonotonicNonNull;";
        }
        if(hasOptionals)
        {
            out << nl << "import org.checkerframework.checker.nullness.qual.Nullable;";
        }
    }

    //
    // Check for java:implements metadata.
    //
    const StringList metaData = p->getMetaData();
    static const string prefix = "java:implements:";
    StringList implements;
    for(const auto& q : metaData)
    {
        if(q.find(prefix) == 0)
        {
            implements.push_back(q.substr(prefix.size()));
        }
    }

    CommentPtr dc = p->parseComment(false);

    //
    // Slice interfaces map to Java interfaces.
    //
    out << sp;
    writeDocComment(out, p->unit(), dc);
    if(dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }
    if(p->isInterface())
    {
        out << nl << "public interface " << fixKwd(name);
        ClassList::const_iterator q = bases.begin();
        StringList::const_iterator r = implements.begin();

        out << " extends ";
        out.useCurrentPosAsIndent();
        if(bases.empty())
        {
            out << getUnqualified("com.zeroc.Ice.Object", package);
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
        if(!p->allOperations().empty() || !implements.empty())
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
        else
        {
            out << " extends " << getUnqualified("com.zeroc.Ice.Value", package);
        }

        if(!implements.empty())
        {
            if(baseClass)
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

    if(!p->isInterface() && !allDataMembers.empty())
    {
        bool hasOptionalMembers = false;
        bool hasRequiredMembers = false;

        for(const auto& d : allDataMembers)
        {
            if(d->optional())
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
                for(const auto d : allDataMembers)
                {
                    if(!d->optional())
                    {
                        string memberName = fixKwd(d->name());
                        string memberType = typeToString(d->type(), TypeModeMember, package, d->getMetaData(), true);
                        paramDecl.push_back(memberType + " " + memberName);
                    }
                }
                out << paramDecl << epar;
                out << sb;
                if(!baseDataMembers.empty())
                {
                    bool hasBaseRequired = false;
                    for(const auto& d : baseDataMembers)
                    {
                        if(!d->optional())
                        {
                            hasBaseRequired = true;
                            break;
                        }
                    }
                    if(hasBaseRequired)
                    {
                        out << nl << "super" << spar;
                        vector<string> baseParamNames;
                        for(const auto& d : baseDataMembers)
                        {
                            if(!d->optional())
                            {
                                baseParamNames.push_back(fixKwd(d->name()));
                            }
                        }
                        out << baseParamNames << epar << ';';
                    }
                }

                for(const auto& d : members)
                {
                    if(!d->optional())
                    {
                        string paramName = fixKwd(d->name());
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
            for(const auto& d : allDataMembers)
            {
                string memberName = fixKwd(d->name());
                string memberType = typeToAnnotatedString(d->type(), TypeModeMember, package, d->getMetaData(),
                                                          d->optional());
                paramDecl.push_back(memberType + " " + memberName);
            }
            out << paramDecl << epar;
            out << sb;
            if(baseClass && allDataMembers.size() != members.size())
            {
                out << nl << "super" << spar;
                vector<string> baseParamNames;
                for(const auto& d : baseDataMembers)
                {
                    baseParamNames.push_back(fixKwd(d->name()));
                }
                out << baseParamNames << epar << ';';
            }
            for(const auto& d : members)
            {
                string paramName = fixKwd(d->name());
                if(d->optional())
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
        out << sp << nl << "public " << name << " clone()";
        out << sb;
        out << nl << "return (" << name << ")super.clone();";
        out << eb;

        out << sp << nl << "public static String ice_staticId()";
        out << sb;
        out << nl << "return \"" << p->scoped() << "\";";
        out << eb;

        out << sp << nl << "@Override";
        out << nl << "public String ice_id()";
        out << sb;
        out << nl << "return ice_staticId();";
        out << eb;

        out << sp;
        writeHiddenDocComment(out);
        out << nl << "public static final long serialVersionUID = ";
        string serialVersionUID;
        if(p->findMetaData("java:serialVersionUID", serialVersionUID))
        {
            const UnitPtr unt = p->unit();
            const DefinitionContextPtr dc = unt->findDefinitionContext(p->file());
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

        writeMarshaling(out, p);
    }
    else
    {
        writeDispatch(out, p);
    }

    out << eb;
    close();
}

void
Slice::Gen::TypesVisitor::visitOperation(const OperationPtr& p)
{
    //
    // Generate the operation signature for a servant.
    //

    ClassDefPtr cl = ClassDefPtr::dynamicCast(p->container());
    assert(cl);

    const string package = getPackage(cl);

    Output& out = output();

    CommentPtr dc = p->parseComment(false);

    //
    // Generate the "Result" type needed by operations that return multiple values.
    //
    if(p->returnsMultipleValues())
    {
        writeResultType(out, p, package, dc);
    }

    //
    // The "MarshaledResult" type is generated in the servant interface.
    //
    if(cl->isInterface() && p->hasMarshaledResult())
    {
        writeMarshaledResultType(out, p, package, dc);
    }
}

bool
Slice::Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
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

    //
    // Check for annotated types to import the corresponding annotations.
    //
    bool hasOptionalDataMembers = false;
    bool hasRequiredDataMembers = false;
    for(const auto& d : p->allDataMembers())
    {
        if(d->optional())
        {
            hasOptionalDataMembers = true;
            if(hasRequiredDataMembers)
            {
                break;
            }
        }
        else
        {
            hasRequiredDataMembers = true;
            if(hasOptionalDataMembers)
            {
                break;
            }
        }
    }

    if(hasOptionalDataMembers || hasRequiredDataMembers)
    {
        out << sp;
        if(hasRequiredDataMembers)
        {
            out << nl << "import org.checkerframework.checker.nullness.qual.MonotonicNonNull;";
        }
        if(hasOptionalDataMembers)
        {
            out << nl << "import org.checkerframework.checker.nullness.qual.Nullable;";
        }
    }

    out << sp;

    CommentPtr dc = p->parseComment(false);
    writeDocComment(out, p->unit(), dc);
    if(dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }

    out << nl << "public class " << name << " extends ";

    if(!base)
    {
        out << getUnqualified("com.zeroc.Ice.UserException", package);
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
    for(const auto& d : allDataMembers)
    {
        if(d->optional())
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
                for(const auto& d : baseDataMembers)
                {
                    if(!d->optional())
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
                for(const auto& d : allDataMembers)
                {
                    if(!d->optional())
                    {
                        string memberName = fixKwd(d->name());
                        string memberType = typeToString(d->type(), TypeModeMember, package, d->getMetaData(), true);
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
                        for(const auto& d : baseDataMembers)
                        {
                            if(!d->optional())
                            {
                                baseParamNames.push_back(fixKwd(d->name()));
                            }
                        }
                        out << baseParamNames << epar << ';';
                    }
                }

                for(const auto& d : members)
                {
                    if(!d->optional())
                    {
                        string paramName = fixKwd(d->name());
                        out << nl << "this." << paramName << " = " << paramName << ';';
                    }
                }
                writeDataMemberInitializers(out, optionalMembers, package);
                out << eb;

                //
                // A method cannot have more than 255 parameters (including the implicit "this" argument).
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
                        for(const auto& d : baseDataMembers)
                        {
                            if(!d->optional())
                            {
                                baseParamNames.push_back(fixKwd(d->name()));
                            }
                        }
                        baseParamNames.push_back(causeParamName);
                        out << baseParamNames << epar << ';';
                    }
                    else
                    {
                        out << nl << "super(" << causeParamName << ");";
                    }
                    for(const auto& d : members)
                    {
                        if(!d->optional())
                        {
                            string paramName = fixKwd(d->name());
                            out << nl << "this." << paramName << " = " << paramName << ';';
                        }
                    }
                    writeDataMemberInitializers(out, optionalMembers, package);
                    out << eb;
                }
            }

            //
            // Generate a constructor accepting parameters for all members.
            //
            out << sp << nl << "public " << name << spar;
            vector<string> paramDecl;
            for(const auto& d : allDataMembers)
            {
                string memberName = fixKwd(d->name());
                string memberType = typeToAnnotatedString(d->type(), TypeModeMember, package, d->getMetaData(),
                                                          d->optional());
                paramDecl.push_back(memberType + " " + memberName);
            }
            out << paramDecl << epar;
            out << sb;
            if(base && allDataMembers.size() != members.size())
            {
                out << nl << "super" << spar;
                vector<string> baseParamNames;
                DataMemberList baseDataMemberList = base->allDataMembers();
                for(const auto& d : baseDataMemberList)
                {
                    baseParamNames.push_back(fixKwd(d->name()));
                }

                out << baseParamNames << epar << ';';
            }
            for(const auto& d : members)
            {
                string paramName = fixKwd(d->name());
                if(d->optional())
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
            // A method cannot have more than 255 parameters (including the implicit "this" argument).
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
                    DataMemberList baseDataMemberList = base->allDataMembers();
                    for(const auto& d : baseDataMemberList)
                    {
                        baseParamNames.push_back(fixKwd(d->name()));
                    }
                    baseParamNames.push_back(causeParamName);
                    out << baseParamNames << epar << ';';
                }
                for(const auto& d : members)
                {
                    string paramName = fixKwd(d->name());
                    if(d->optional())
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

    out << sp << nl << "public String ice_id()";
    out << sb;
    out << nl << "return \"" << scoped << "\";";
    out << eb;

    return true;
}

void
Slice::Gen::TypesVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    Output& out = output();

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
        writeHiddenDocComment(out);
        out << nl << "@Override";
        out << nl << "public " << getUnqualified("com.zeroc.Ice.SlicedData", package) << " ice_getSlicedData()";
        out << sb;
        out << nl << "return _slicedData;";
        out << eb;

        out << sp;
        writeHiddenDocComment(out);
        out << nl << "@Override";
        out << nl << "public void _write(" << getUnqualified("com.zeroc.Ice.OutputStream", package) << " ostr)";
        out << sb;
        out << nl << "ostr.startException(_slicedData);";
        out << nl << "_writeImpl(ostr);";
        out << nl << "ostr.endException();";
        out << eb;

        out << sp;
        writeHiddenDocComment(out);
        out << nl << "@Override";
        out << nl << "public void _read(" << getUnqualified("com.zeroc.Ice.InputStream", package) << " istr)";
        out << sb;
        out << nl << "istr.startException();";
        out << nl << "_readImpl(istr);";
        out << nl << "_slicedData = istr.endException(true);";
        out << eb;
    }

    out << sp;
    writeHiddenDocComment(out);
    out << nl << "@Override";
    out << nl << "protected void _writeImpl(" << getUnqualified("com.zeroc.Ice.OutputStream", package) << " ostr_)";
    out << sb;
    out << nl << "ostr_.startSlice(\"" << scoped << "\", -1, " << (!base ? "true" : "false") << ");";
    iter = 0;
    for(const auto& d : members)
    {
        if(!d->optional())
        {
            writeMarshalDataMember(out, package, d, iter);
        }
    }
    for(const auto& d : optionalMembers)
    {
        writeMarshalDataMember(out, package, d, iter);
    }
    out << nl << "ostr_.endSlice();";
    if(base)
    {
        out << nl << "super._writeImpl(ostr_);";
    }
    out << eb;

    DataMemberList classMembers = p->classDataMembers();
    DataMemberList allClassMembers = p->allClassDataMembers();

    out << sp;
    writeHiddenDocComment(out);
    out << nl << "@Override";
    out << nl << "protected void _readImpl(" << getUnqualified("com.zeroc.Ice.InputStream", package) << " istr_)";
    out << sb;
    out << nl << "istr_.startSlice();";
    iter = 0;
    for(const auto& d : members)
    {
        if(!d->optional())
        {
            writeUnmarshalDataMember(out, package, d, iter);
        }
    }
    for(const auto& d : optionalMembers)
    {
        writeUnmarshalDataMember(out, package, d, iter);
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
            out << sp;
            writeHiddenDocComment(out);
            out << nl << "@Override";
            out << nl << "public boolean _usesClasses()";
            out << sb;
            out << nl << "return true;";
            out << eb;
        }
    }

    if(preserved && !basePreserved)
    {
        out << sp;
        writeHiddenDocComment(out);
        out << nl << "protected " << getUnqualified("com.zeroc.Ice.SlicedData", package) << " _slicedData;";
    }

    out << sp;
    writeHiddenDocComment(out);
    out << nl << "public static final long serialVersionUID = ";
    string serialVersionUID;
    if(p->findMetaData("java:serialVersionUID", serialVersionUID))
    {
        const UnitPtr unt = p->unit();
        const DefinitionContextPtr dc = unt->findDefinitionContext(p->file());
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
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    string name = fixKwd(p->name());
    string absolute = getUnqualified(p);

    open(absolute, p->file());

    Output& out = output();

    out << sp;
    out << nl << "import org.checkerframework.checker.nullness.qual.Nullable;";

    //
    // Check for java:implements metadata.
    //
    const StringList metaData = p->getMetaData();
    static const string prefix = "java:implements:";
    StringList implements;
    for(const auto& q : metaData)
    {
        if(q.find(prefix) == 0)
        {
            implements.push_back(q.substr(prefix.size()));
        }
    }

    out << sp;

    CommentPtr dc = p->parseComment(false);
    writeDocComment(out, p->unit(), dc);
    if(dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }

    out << nl << "public class " << name << " implements ";
    out.useCurrentPosAsIndent();
    out << "java.lang.Cloneable," << nl << "java.io.Serializable";
    for(const auto& q : implements)
    {
        out << "," << nl << q;
    }
    out.restoreIndent();
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
        for(const auto& d : members)
        {
            string memberName = fixKwd(d->name());
            string memberType = typeToString(d->type(), TypeModeMember, package, d->getMetaData(), true);
            paramDecl.push_back(memberType + " " + memberName);
            paramNames.push_back(memberName);
        }

        out << sp << nl << "public " << name << spar << paramDecl << epar;
        out << sb;
        for(const auto& i : paramNames)
        {
            out << nl << "this." << i << " = " << i << ';';
        }
        out << eb;
    }

    out << sp << nl << "public boolean equals(java.lang.Object rhs)";
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
    for(const auto& d : members)
    {
        string memberName = fixKwd(d->name());
        BuiltinPtr b = BuiltinPtr::dynamicCast(d->type());
        if(b && (b->kind() == Builtin::KindByte || b->kind() == Builtin::KindBool || b->kind() == Builtin::KindShort ||
                 b->kind() == Builtin::KindInt || b->kind() == Builtin::KindLong || b->kind() == Builtin::KindFloat ||
                 b->kind() == Builtin::KindDouble))
        {
            out << nl << "if(this." << memberName << " != r." << memberName << ')';
            out << sb;
            out << nl << "return false;";
            out << eb;
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
            SequencePtr seq = SequencePtr::dynamicCast(d->type());
            if(seq && !hasTypeMetaData(seq, d->getMetaData()))
            {
                //
                // Arrays.equals() handles null values.
                //
                out << nl << "if(!java.util.Arrays.equals(this." << memberName << ", r." << memberName << "))";
                out << sb;
                out << nl << "return false;";
                out << eb;
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

    out << sp << nl << "public int hashCode()";
    out << sb;
    out << nl << "int h_ = 5381;";
    out << nl << "h_ = com.zeroc.IceInternal.HashUtil.hashAdd(h_, \"" << p->scoped() << "\");";
    iter = 0;
    for(const auto& d : members)
    {
        string memberName = fixKwd(d->name());
        out << nl << "h_ = com.zeroc.IceInternal.HashUtil.hashAdd(h_, " << memberName << ");";
    }
    out << nl << "return h_;";
    out << eb;

    out << sp << nl << "public " << name << " clone()";
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

    out << sp << nl << "public void ice_writeMembers(" << getUnqualified("com.zeroc.Ice.OutputStream", package)
        << " ostr)";
    out << sb;
    iter = 0;
    for(const auto& d : members)
    {
        writeMarshalDataMember(out, package, d, iter, true);
    }
    out << eb;

    DataMemberList classMembers = p->classDataMembers();

    out << sp << nl << "public void ice_readMembers(" << getUnqualified("com.zeroc.Ice.InputStream", package)
        << " istr)";
    out << sb;
    iter = 0;
    for(const auto& d : members)
    {
        writeUnmarshalDataMember(out, package, d, iter, true);
    }
    out << eb;

    out << sp << nl << "static public void ice_write(" << getUnqualified("com.zeroc.Ice.OutputStream", package)
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

    out << sp << nl << "static public " << name << " ice_read(" << getUnqualified("com.zeroc.Ice.InputStream", package)
        << " istr)";
    out << sb;
    out << nl << name << " v = new " << name << "();";
    out << nl << "v.ice_readMembers(istr);";
    out << nl << "return v;";
    out << eb;

    out << sp;
    out << nl << "static public void ice_write(" << getUnqualified("com.zeroc.Ice.OutputStream", package)
        << " ostr, int tag, " << addAnnotation(name, "@Nullable") << " v)";
    out << sb;
    out << nl << "if(v != null)";
    out << sb;
    out << nl << "if(ostr.writeOptional(tag, " << getOptionalFormat(p) << "))";
    out << sb;
    if(p->isVariableLength())
    {
        out << nl << "int pos = ostr.startSize();";
        out << nl << "ice_write(ostr, v);";
        out << nl << "ostr.endSize(pos);";
    }
    else
    {
        out << nl << "ostr.writeSize(" << p->minWireSize() << ");";
        out << nl << "ice_write(ostr, v);";
    }
    out << eb;
    out << eb;
    out << eb;

    out << sp;
    out << nl << "static public " << addAnnotation(name, "@Nullable") << " ice_read(" <<
        getUnqualified("com.zeroc.Ice.InputStream", package) << " istr, int tag)";
    out << sb;
    out << nl << "if(istr.readOptional(tag, " << getOptionalFormat(p) << "))";
    out << sb;
    if(p->isVariableLength())
    {
        out << nl << "istr.skip(4);";
    }
    else
    {
        out << nl << "istr.skipSize();";
    }
    out << nl << "return " << typeS << ".ice_read(istr);";
    out << eb;
    out << nl << "else";
    out << sb;
    out << nl << "return null;";
    out << eb;
    out << eb;

    out << sp << nl << "private static final " << name << " _nullMarshalValue = new " << name << "();";

    out << sp;
    writeHiddenDocComment(out);
    out << nl << "public static final long serialVersionUID = ";
    string serialVersionUID;
    if(p->findMetaData("java:serialVersionUID", serialVersionUID))
    {
        const UnitPtr unt = p->unit();
        const DefinitionContextPtr dc = unt->findDefinitionContext(p->file());
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
Slice::Gen::TypesVisitor::visitDataMember(const DataMemberPtr& p)
{
    const ContainerPtr container = p->container();
    const ClassDefPtr cls = ClassDefPtr::dynamicCast(container);
    const StructPtr st = StructPtr::dynamicCast(container);
    const ExceptionPtr ex = ExceptionPtr::dynamicCast(container);
    const ContainedPtr contained = ContainedPtr::dynamicCast(container);

    const string name = fixKwd(p->name());
    const StringList metaData = p->getMetaData();
    const bool getSet = p->hasMetaData(_getSetMetaData) || contained->hasMetaData(_getSetMetaData);
    const bool optional = p->optional();
    const TypePtr type = p->type();
    const BuiltinPtr b = BuiltinPtr::dynamicCast(type);
    const bool classType = isValue(type);

    const string typeS = typeToString(type, TypeModeMember, getPackage(contained), metaData);
    // Optional data members are only allowed in classes and exceptions.
    const string typeN = (cls || ex) ?
                             typeToAnnotatedString(type, TypeModeMember, getPackage(contained), metaData, optional)
                             : typeToString(type, TypeModeMember, getPackage(contained), metaData);

    Output& out = output();

    out << sp;

    CommentPtr dc = p->parseComment(false);
    writeDocComment(out, p->unit(), dc);
    if(dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }

    //
    // Access visibility for class data members can be controlled by metadata.
    // If none is specified, the default is public.
    //
    if(cls && (p->hasMetaData("protected") || contained->hasMetaData("protected")))
    {
        out << nl << "protected " << typeN << ' ' << name << ';';
    }
    else if(optional)
    {
        out << nl << "private " << typeN << ' ' << name << ';';
    }
    else
    {
        out << nl << "public " << typeN << ' ' << name << ';';
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
        writeDocComment(out, p->unit(), dc);
        if(dc && dc->isDeprecated())
        {
            out << nl << "@Deprecated";
        }
        out << nl << "public " << typeS << " get" << capName << "()";
        out << sb;
        if(optional)
        {
            out << nl << "if(!_" << p->name() << ')';
            out << sb;
            out << nl << "throw new java.util.NoSuchElementException(\"" << name << " is not set\");";
            out << eb;
        }
        out << nl << "return " << name << ';';
        out << eb;

        //
        // Setter.
        //
        out << sp;
        writeDocComment(out, p->unit(), dc);
        if(dc && dc->isDeprecated())
        {
            out << nl << "@Deprecated";
        }
        out << nl << "public void set" << capName << '(' << typeS << " " << name << ')';
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
            writeDocComment(out, p->unit(), dc);
            if(dc && dc->isDeprecated())
            {
                out << nl << "@Deprecated";
            }
            out << nl << "public boolean has" << capName << "()";
            out << sb;
            out << nl << "return _" << p->name() << ';';
            out << eb;

            out << sp;
            writeDocComment(out, p->unit(), dc);
            if(dc && dc->isDeprecated())
            {
                out << nl << "@Deprecated";
            }
            out << nl << "public void clear" << capName << "()";
            out << sb;
            out << nl << "_" << p->name() << " = false;";
            out << eb;

            out << sp;
            writeDocComment(out, p->unit(), dc);
            if(dc && dc->isDeprecated())
            {
                out << nl << "@Deprecated";
            }
            out << nl << "public void optional" << capName << '(' << typeN << " v)";
            out << sb;
            out << nl << "if(v == null)";
            out << sb;
            out << nl << "_" << p->name() << " = false;";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << "_" << p->name() << " = true;";
            out << nl << name << " = v;";
            out << eb;
            out << eb;

            out << sp;
            writeDocComment(out, p->unit(), dc);
            if(dc && dc->isDeprecated())
            {
                out << nl << "@Deprecated";
            }
            out << nl << "public " << typeN << " optional" << capName << "()";
            out << sb;
            out << nl << "if(_" << p->name() << ')';
            out << sb;
            out << nl << "return " << name << ";";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << "return null;";

            out << eb;
            out << eb;
        }

        //
        // Check for bool type.
        //
        if(b && b->kind() == Builtin::KindBool)
        {
            if(cls && !validateMethod(ops, "is" + capName, 0, file, line))
            {
                return;
            }
            out << sp;
            if(dc && dc->isDeprecated())
            {
                out << nl << "@Deprecated";
            }
            out << nl << "public boolean is" << capName << "()";
            out << sb;
            if(optional)
            {
                out << nl << "if(!_" << p->name() << ')';
                out << sb;
                out << nl << "throw new java.util.NoSuchElementException(\"" << name << " is not set\");";
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
                if(dc && dc->isDeprecated())
                {
                    out << nl << "@Deprecated";
                }
                out << nl << "public " << elem << " get" << capName << "(int index)";
                out << sb;
                if(optional)
                {
                    out << nl << "if(!_" << p->name() << ')';
                    out << sb;
                    out << nl << "throw new java.util.NoSuchElementException(\"" << name << " is not set\");";
                    out << eb;
                }
                out << nl << "return this." << name << "[index];";
                out << eb;

                //
                // Indexed setter.
                //
                out << sp;
                if(dc && dc->isDeprecated())
                {
                    out << nl << "@Deprecated";
                }
                out << nl << "public void set" << capName << "(int index, " << elem << " val)";
                out << sb;
                if(optional)
                {
                    out << nl << "if(!_" << p->name() << ')';
                    out << sb;
                    out << nl << "throw new java.util.NoSuchElementException(\"" << name << " is not set\");";
                    out << eb;
                }
                out << nl << "this." << name << "[index] = val;";
                out << eb;
            }
        }
    }
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string name = fixKwd(p->name());
    string package = getPackage(p);
    string absolute = getUnqualified(p);
    EnumeratorList enumerators = p->enumerators();

    open(absolute, p->file());

    Output& out = output();

    out << sp;
    out << nl << "import org.checkerframework.checker.nullness.qual.Nullable;";

    out << sp;
    CommentPtr dc = p->parseComment(false);
    writeDocComment(out, p->unit(), dc);
    if(dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }

    out << nl << "public enum " << name << " implements java.io.Serializable";
    out << sb;

    for(EnumeratorList::const_iterator en = enumerators.begin(); en != enumerators.end(); ++en)
    {
        if(en != enumerators.begin())
        {
            out << ',';
        }
        CommentPtr edc = (*en)->parseComment(false);
        writeDocComment(out, p->unit(), edc);
        if(edc && edc->isDeprecated())
        {
            out << nl << "@Deprecated";
        }
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
    for(const auto& en : enumerators)
    {
        out << nl << "case " << en->value() << ':';
        out.inc();
        out << nl << "return " << fixKwd(en->name()) << ';';
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

    out << sp << nl << "public void ice_write(" << getUnqualified("com.zeroc.Ice.OutputStream", package) << " ostr)";
    out << sb;
    out << nl << "ostr.writeEnum(_value, " << p->maxValue() << ");";
    out << eb;

    out << sp << nl << "public static void ice_write(" << getUnqualified("com.zeroc.Ice.OutputStream", package)
        << " ostr, " << name << " v)";
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

    out << sp << nl << "public static " << name << " ice_read(" << getUnqualified("com.zeroc.Ice.InputStream", package)
        << " istr)";
    out << sb;
    out << nl << "int v = istr.readEnum(" << p->maxValue() << ");";
    out << nl << "return validate(v);";
    out << eb;

    out << sp;
    out << nl << "public static void ice_write(" << getUnqualified("com.zeroc.Ice.OutputStream", package)
        << " ostr, int tag, " << addAnnotation(name, "@Nullable") << " v)";
    out << sb;
    out << nl << "if(v != null)";
    out << sb;
    out << nl << "if(ostr.writeOptional(tag, " << getOptionalFormat(p) << "))";
    out << sb;
    out << nl << "ice_write(ostr, v);";
    out << eb;
    out << eb;
    out << eb;

    out << sp;
    out << nl << "public static " << addAnnotation(name, "@Nullable") << " ice_read("
        << getUnqualified("com.zeroc.Ice.InputStream", package) << " istr, int tag)";
    out << sb;
    out << nl << "if(istr.readOptional(tag, " << getOptionalFormat(p) << "))";
    out << sb;
    out << nl << "return ice_read(istr);";
    out << eb;
    out << nl << "else";
    out << sb;
    out << nl << "return null;";
    out << eb;
    out << eb;

    out << sp << nl << "private static " << name << " validate(int v)";
    out << sb;
    out << nl << "final " << name << " e = valueOf(v);";
    out << nl << "if(e == null)";
    out << sb;
    out << nl << "throw new " << getUnqualified("com.zeroc.Ice.MarshalException", package)
        << "(\"enumerator value \" + v + \" is out of range\");";
    out << eb;
    out << nl << "return e;";
    out << eb;

    out << sp << nl << "private final int _value;";

    out << eb;
    close();
}

void
Slice::Gen::TypesVisitor::visitConst(const ConstPtr& p)
{
    string name = fixKwd(p->name());
    string package = getPackage(p);
    string absolute = getUnqualified(p);
    TypePtr type = p->type();

    open(absolute, p->file());

    Output& out = output();

    out << sp;

    CommentPtr dc = p->parseComment(false);
    writeDocComment(out, p->unit(), dc);
    if(dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }

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
    for(const auto& i : ops)
    {
        if(i->name() == name)
        {
            int numParams = static_cast<int>(i->parameters().size());
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
        os << prefix << "com.zeroc.IceCompactId.TypeId_" << p->compactId();
        open(os.str(), p->file());

        Output& out = output();
        out << sp;
        writeHiddenDocComment(out);
        out << nl << "public class TypeId_" << p->compactId();
        out << sb;
        out << nl << "public final static String typeId = \"" << p->scoped() << "\";";
        out << eb;

        close();
    }
    return false;
}

Slice::Gen::HelperVisitor::HelperVisitor(const string& dir) :
    JavaVisitor(dir)
{
}

void
Slice::Gen::HelperVisitor::visitSequence(const SequencePtr& p)
{
    string meta;
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p->type());
    if(!hasTypeMetaData(p) && builtin && builtin->kind() <= Builtin::KindString)
    {
        return; // No helpers for sequence of primitive types
    }
    else if(hasTypeMetaData(p) && !p->findMetaData("java:type", meta))
    {
        return; // No helpers for custom metadata other than java:type
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

    out << sp;
    out << nl << "import org.checkerframework.checker.nullness.qual.Nullable;";

    int iter;

    out << sp;
    writeDocComment(out, "Helper class for marshaling/unmarshaling " + name + ".");
    out << nl << "public final class " << name << "Helper";
    out << sb;

    out << nl << "public static void write(" << getUnqualified("com.zeroc.Ice.OutputStream", package)
        << " ostr, " << typeS << " v)";
    out << sb;
    iter = 0;
    writeSequenceMarshalUnmarshalCode(out, package, p, "v", true, iter, false);
    out << eb;

    out << sp;
    if(suppressUnchecked)
    {
        out << nl << "@SuppressWarnings(\"unchecked\")";
    }
    out << nl << "public static " << typeS << " read(" << getUnqualified("com.zeroc.Ice.InputStream", package)
        << " istr)";
    out << sb;
    out << nl << "final " << typeS << " v;";
    iter = 0;
    writeSequenceMarshalUnmarshalCode(out, package, p, "v", false, iter, false);
    out << nl << "return v;";
    out << eb;

    static const char* builtinTable[] = { "Byte", "Bool", "Short", "Int", "Long", "Float", "Double", "String" };

    out << sp;
    out << nl << "public static void write(" << getUnqualified("com.zeroc.Ice.OutputStream", package)
        << " ostr, int tag, " << addAnnotation(typeS, "@Nullable") << " v)";
    out << sb;
    if(!hasTypeMetaData(p) && builtin && builtin->kind() < Builtin::KindObject)
    {
        out << nl << "ostr.write" << builtinTable[builtin->kind()] << "Seq(tag, v);";
    }
    else
    {
        out << nl << "if(v != null)";
        out << sb;
        out << nl << "if(ostr.writeOptional(tag, " << getOptionalFormat(p) << "))";
        out << sb;
        if(p->type()->isVariableLength())
        {
            out << nl << "int pos = ostr.startSize();";
            writeSequenceMarshalUnmarshalCode(out, package, p, "v", true, iter, true);
            out << nl << "ostr.endSize(pos);";
        }
        else
        {
            //
            // The sequence is an instance of java.util.List<E>, where E is a fixed-size type.
            // If the element type is bool or byte, we do NOT write an extra size.
            //
            const size_t sz = p->type()->minWireSize();
            if(sz > 1)
            {
                string metaData;
                out << nl << "final int optSize = v == null ? 0 : ";
                if(findMetaData("java:buffer", p->getMetaData(), metaData))
                {
                    out << "v.remaining() / " << sz << ";";
                }
                else if(hasTypeMetaData(p))
                {
                    out << "v.size();";
                }
                else
                {
                    out << "v.length;";
                }
                out << nl << "ostr.writeSize(optSize > 254 ? optSize * " << sz << " + 5 : optSize * " << sz << " + 1);";
            }
            writeSequenceMarshalUnmarshalCode(out, package, p, "v", true, iter, true);
        }
        out << eb;
        out << eb;
    }
    out << eb;

    out << sp;
    out << nl << "public static " << addAnnotation(typeS, "@Nullable") << " read(" << getUnqualified("com.zeroc.Ice.InputStream", package)
        << " istr, int tag)";
    out << sb;
    if(!hasTypeMetaData(p) && builtin && builtin->kind() < Builtin::KindObject)
    {
        out << nl << "return istr.read" << builtinTable[builtin->kind()] << "Seq(tag);";
    }
    else
    {
        out << nl << "if(istr.readOptional(tag, " << getOptionalFormat(p) << "))";
        out << sb;
        if(p->type()->isVariableLength())
        {
            out << nl << "istr.skip(4);";
        }
        else if(p->type()->minWireSize() > 1)
        {
            out << nl << "istr.skipSize();";
        }
        out << nl << typeS << " v;";
        writeSequenceMarshalUnmarshalCode(out, package, p, "v", false, iter, true);
        out << nl << "return v;";
        out << eb;
        out << nl << "else";
        out << sb;
        out << nl << "return null;";
        out << eb;
    }
    out << eb;

    out << eb;
    close();
}

void
Slice::Gen::HelperVisitor::visitDictionary(const DictionaryPtr& p)
{
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

    out << sp;
    out << nl << "import org.checkerframework.checker.nullness.qual.Nullable;";

    int iter;

    out << sp;
    writeDocComment(out, "Helper class for marshaling/unmarshaling " + name + ".");
    out << nl << "public final class " << name << "Helper";
    out << sb;

    out << nl << "public static void write(" << getUnqualified("com.zeroc.Ice.OutputStream", package) << " ostr, "
        << formalType << " v)";
    out << sb;
    iter = 0;
    writeDictionaryMarshalUnmarshalCode(out, package, p, "v", true, iter, false);
    out << eb;

    out << sp << nl << "public static " << formalType << " read(" << getUnqualified("com.zeroc.Ice.InputStream", package)
        << " istr)";
    out << sb;
    out << nl << formalType << " v;";
    iter = 0;
    writeDictionaryMarshalUnmarshalCode(out, package, p, "v", false, iter, false);
    out << nl << "return v;";
    out << eb;

    out << sp;
    out << nl << "public static void write(" << getUnqualified("com.zeroc.Ice.OutputStream", package)
        << " ostr, int tag, " << addAnnotation(formalType, "@Nullable") << " v)";
    out << sb;
    out << nl << "if(v != null)";
    out << sb;
    out << nl << "if(ostr.writeOptional(tag, " << getOptionalFormat(p) << "))";
    out << sb;
    TypePtr keyType = p->keyType();
    TypePtr valueType = p->valueType();
    if(keyType->isVariableLength() || valueType->isVariableLength())
    {
        out << nl << "int pos = ostr.startSize();";
        writeDictionaryMarshalUnmarshalCode(out, package, p, "v", true, iter, true);
        out << nl << "ostr.endSize(pos);";
    }
    else
    {
        const size_t sz = keyType->minWireSize() + valueType->minWireSize();
        out << nl << "final int optSize = v == null ? 0 : v.size();";
        out << nl << "ostr.writeSize(optSize > 254 ? optSize * " << sz << " + 5 : optSize * " << sz << " + 1);";
        writeDictionaryMarshalUnmarshalCode(out, package, p, "v", true, iter, true);
    }
    out << eb;
    out << eb;
    out << eb;

    out << sp;
    out << nl << "public static " << addAnnotation(formalType, "@Nullable") << " read("
        << getUnqualified("com.zeroc.Ice.InputStream", package) << " istr, int tag)";
    out << sb;
    out << nl << "if(istr.readOptional(tag, " << getOptionalFormat(p) << "))";
    out << sb;
    if(keyType->isVariableLength() || valueType->isVariableLength())
    {
        out << nl << "istr.skip(4);";
    }
    else
    {
        out << nl << "istr.skipSize();";
    }
    out << nl << formalType << " v;";
    writeDictionaryMarshalUnmarshalCode(out, package, p, "v", false, iter, true);
    out << nl << "return v;";
    out << eb;
    out << nl << "else";
    out << sb;
    out << nl << "return null;";
    out << eb;
    out << eb;

    out << eb;
    close();
}

Slice::Gen::ProxyVisitor::ProxyVisitor(const string& dir) :
    JavaVisitor(dir)
{
}

bool
Slice::Gen::ProxyVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    //
    // Don't generate a proxy interface for a class with no operations.
    //
    const OperationList ops = p->allOperations();
    if(!p->isInterface() && ops.empty())
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
    // Check for optional parameters and import the necessary annotations if they're present.
    //
    for(const auto& op : ops)
    {
        if(op->sendsOptionals() || op->returnIsOptional())
        {
            out << sp << nl << "import org.checkerframework.checker.nullness.qual.Nullable;";
            break;
        }

        for(const auto& q : op->outParameters())
        {
            if(q->optional())
            {
                out << sp << nl << "import org.checkerframework.checker.nullness.qual.Nullable;";
                break;
            }
        }
    }

    //
    // For proxy purposes, we can ignore a base class if it has no operations.
    //
    if(!bases.empty() && !bases.front()->isInterface() && bases.front()->allOperations().empty())
    {
        bases.pop_front();
    }

    CommentPtr dc = p->parseComment(false);

    //
    // Generate a Java interface as the user-visible type
    //
    out << sp;
    writeDocComment(out, p->unit(), dc);
    if(dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }
    out << nl << "public interface " << name << "Prx extends ";
    out.useCurrentPosAsIndent();
    if(bases.empty())
    {
        out << getUnqualified("com.zeroc.Ice.ObjectPrx", package);
    }
    else
    {
        for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
        {
            if(q != bases.begin())
            {
                out << ',' << nl;
            }
            out << getUnqualified(*q, package, "", "Prx");
        }
    }
    out.restoreIndent();

    out << sb;

    return true;
}

void
Slice::Gen::ProxyVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    Output& out = output();

    CommentPtr dc = p->parseComment(false);

    const string package = getPackage(p);
    const string contextParam = "java.util.Map<String, String> context";

    out << sp;
    writeDocComment(out,
                    "Contacts the remote server to verify that the object implements this type.\n"
                    "Raises a local exception if a communication error occurs.\n"
                    "@param obj The untyped proxy.\n"
                    "@return A proxy for this type, or null if the object does not support this type.");
    out << nl << "static " << p->name() << "Prx checkedCast(" << getUnqualified("com.zeroc.Ice.ObjectPrx", package)
        << " obj)";
    out << sb;
    out << nl << "return " << getUnqualified("com.zeroc.Ice.ObjectPrx", package) << "._checkedCast(obj, ice_staticId(), "
        << p->name() << "Prx.class, _" << p->name() << "PrxI.class);";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Contacts the remote server to verify that the object implements this type.\n"
                    "Raises a local exception if a communication error occurs.\n"
                    "@param obj The untyped proxy.\n"
                    "@param context The Context map to send with the invocation.\n"
                    "@return A proxy for this type, or null if the object does not support this type.");
    out << nl << "static " << p->name() << "Prx checkedCast(" << getUnqualified("com.zeroc.Ice.ObjectPrx", package)
        << " obj, " << contextParam << ')';
    out << sb;
    out << nl << "return " << getUnqualified("com.zeroc.Ice.ObjectPrx", package)
        << "._checkedCast(obj, context, ice_staticId(), " << p->name() << "Prx.class, _" << p->name() << "PrxI.class);";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Contacts the remote server to verify that a facet of the object implements this type.\n"
                    "Raises a local exception if a communication error occurs.\n"
                    "@param obj The untyped proxy.\n"
                    "@param facet The name of the desired facet.\n"
                    "@return A proxy for this type, or null if the object does not support this type.");
    out << nl << "static " << p->name() << "Prx checkedCast(" << getUnqualified("com.zeroc.Ice.ObjectPrx", package)
        << " obj, String facet)";
    out << sb;
    out << nl << "return " << getUnqualified("com.zeroc.Ice.ObjectPrx", package)
        << "._checkedCast(obj, facet, ice_staticId(), " << p->name() << "Prx.class, _" << p->name() << "PrxI.class);";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Contacts the remote server to verify that a facet of the object implements this type.\n"
                    "Raises a local exception if a communication error occurs.\n"
                    "@param obj The untyped proxy.\n"
                    "@param facet The name of the desired facet.\n"
                    "@param context The Context map to send with the invocation.\n"
                    "@return A proxy for this type, or null if the object does not support this type.");
    out << nl << "static " << p->name() << "Prx checkedCast(" << getUnqualified("com.zeroc.Ice.ObjectPrx", package)
        << " obj, String facet, " << contextParam << ')';
    out << sb;
    out << nl << "return " << getUnqualified("com.zeroc.Ice.ObjectPrx", package)
        << "._checkedCast(obj, facet, context, ice_staticId(), " << p->name()
        << "Prx.class, _" << p->name() << "PrxI.class);";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Downcasts the given proxy to this type without contacting the remote server.\n"
                    "@param obj The untyped proxy.\n"
                    "@return A proxy for this type.");
    out << nl << "static " << p->name() << "Prx uncheckedCast(" << getUnqualified("com.zeroc.Ice.ObjectPrx", package)
        << " obj)";
    out << sb;
    out << nl << "return " << getUnqualified("com.zeroc.Ice.ObjectPrx", package) << "._uncheckedCast(obj, " << p->name()
        << "Prx.class, _" << p->name() << "PrxI.class);";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Downcasts the given proxy to this type without contacting the remote server.\n"
                    "@param obj The untyped proxy.\n"
                    "@param facet The name of the desired facet.\n"
                    "@return A proxy for this type.");
    out << nl << "static " << p->name() << "Prx uncheckedCast(" << getUnqualified("com.zeroc.Ice.ObjectPrx", package)
        << " obj, String facet)";
    out << sb;
    out << nl << "return " << getUnqualified("com.zeroc.Ice.ObjectPrx", package) << "._uncheckedCast(obj, facet, "
        << p->name() << "Prx.class, _"
        << p->name() << "PrxI.class);";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Returns a proxy that is identical to this proxy, except for the per-proxy context.\n"
                    "@param newContext The context for the new proxy.\n"
                    "@return A proxy with the specified per-proxy context.");
    out << nl << "@Override";
    out << nl << "default " << p->name() << "Prx ice_context(java.util.Map<String, String> newContext)";
    out << sb;
    out << nl << "return (" << p->name() << "Prx)_ice_context(newContext);";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Returns a proxy that is identical to this proxy, except for the adapter ID.\n"
                    "@param newAdapterId The adapter ID for the new proxy.\n"
                    "@return A proxy with the specified adapter ID.");
    out << nl << "@Override";
    out << nl << "default " << p->name() << "Prx ice_adapterId(String newAdapterId)";
    out << sb;
    out << nl << "return (" << p->name() << "Prx)_ice_adapterId(newAdapterId);";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Returns a proxy that is identical to this proxy, except for the endpoints.\n"
                    "@param newEndpoints The endpoints for the new proxy.\n"
                    "@return A proxy with the specified endpoints.");
    out << nl << "@Override";
    out << nl << "default " << p->name() << "Prx ice_endpoints(" << getUnqualified("com.zeroc.Ice.Endpoint", package) << "[] newEndpoints)";
    out << sb;
    out << nl << "return (" << p->name() << "Prx)_ice_endpoints(newEndpoints);";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Returns a proxy that is identical to this proxy, except for the locator cache timeout.\n"
                    "@param newTimeout The new locator cache timeout (in seconds).\n"
                    "@return A proxy with the specified locator cache timeout.");
    out << nl << "@Override";
    out << nl << "default " << p->name() << "Prx ice_locatorCacheTimeout(int newTimeout)";
    out << sb;
    out << nl << "return (" << p->name() << "Prx)_ice_locatorCacheTimeout(newTimeout);";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Returns a proxy that is identical to this proxy, except for the invocation timeout.\n"
                    "@param newTimeout The new invocation timeout (in seconds).\n"
                    "@return A proxy with the specified invocation timeout.");
    out << nl << "@Override";
    out << nl << "default " << p->name() << "Prx ice_invocationTimeout(int newTimeout)";
    out << sb;
    out << nl << "return (" << p->name() << "Prx)_ice_invocationTimeout(newTimeout);";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Returns a proxy that is identical to this proxy, except for connection caching.\n"
                    "@param newCache <code>true</code> if the new proxy should cache connections; <code>false</code> otherwise.\n"
                    "@return A proxy with the specified caching policy.");
    out << nl << "@Override";
    out << nl << "default " << p->name() << "Prx ice_connectionCached(boolean newCache)";
    out << sb;
    out << nl << "return (" << p->name() << "Prx)_ice_connectionCached(newCache);";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Returns a proxy that is identical to this proxy, except for the endpoint selection policy.\n"
                    "@param newType The new endpoint selection policy.\n"
                    "@return A proxy with the specified endpoint selection policy.");
    out << nl << "@Override";
    out << nl << "default " << p->name() << "Prx ice_endpointSelection("
        << getUnqualified("com.zeroc.Ice.EndpointSelectionType", package)
        << " newType)";
    out << sb;
    out << nl << "return (" << p->name() << "Prx)_ice_endpointSelection(newType);";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Returns a proxy that is identical to this proxy, except for how it selects endpoints.\n"
                    "@param b If <code>b</code> is <code>true</code>, only endpoints that use a secure transport are\n"
                    "used by the new proxy. If <code>b</code> is false, the returned proxy uses both secure and\n"
                    "insecure endpoints.\n"
                    "@return A proxy with the specified selection policy.");
    out << nl << "@Override";
    out << nl << "default " << p->name() << "Prx ice_secure(boolean b)";
    out << sb;
    out << nl << "return (" << p->name() << "Prx)_ice_secure(b);";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Returns a proxy that is identical to this proxy, except for the encoding used to marshal parameters.\n"
                    "@param e The encoding version to use to marshal request parameters.\n"
                    "@return A proxy with the specified encoding version.");
    out << nl << "@Override";
    out << nl << "default " << p->name() << "Prx ice_encodingVersion(" << getUnqualified("com.zeroc.Ice.EncodingVersion", package)
        << " e)";
    out << sb;
    out << nl << "return (" << p->name() << "Prx)_ice_encodingVersion(e);";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Returns a proxy that is identical to this proxy, except for its endpoint selection policy.\n"
                    "@param b If <code>b</code> is <code>true</code>, the new proxy will use secure endpoints for invocations\n"
                    "and only use insecure endpoints if an invocation cannot be made via secure endpoints. If <code>b</code> is\n"
                    "<code>false</code>, the proxy prefers insecure endpoints to secure ones.\n"
                    "@return A proxy with the specified selection policy.");
    out << nl << "@Override";
    out << nl << "default " << p->name() << "Prx ice_preferSecure(boolean b)";
    out << sb;
    out << nl << "return (" << p->name() << "Prx)_ice_preferSecure(b);";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Returns a proxy that is identical to this proxy, except for the router.\n"
                    "@param router The router for the new proxy.\n"
                    "@return A proxy with the specified router.");
    out << nl << "@Override";
    out << nl << "default " << p->name() << "Prx ice_router(" << getUnqualified("com.zeroc.Ice.RouterPrx", package) << " router)";
    out << sb;
    out << nl << "return (" << p->name() << "Prx)_ice_router(router);";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Returns a proxy that is identical to this proxy, except for the locator.\n"
                    "@param locator The locator for the new proxy.\n"
                    "@return A proxy with the specified locator.");
    out << nl << "@Override";
    out << nl << "default " << p->name() << "Prx ice_locator(" << getUnqualified("com.zeroc.Ice.LocatorPrx", package)
        << " locator)";
    out << sb;
    out << nl << "return (" << p->name() << "Prx)_ice_locator(locator);";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Returns a proxy that is identical to this proxy, except for collocation optimization.\n"
                    "@param b <code>true</code> if the new proxy enables collocation optimization; <code>false</code> otherwise.\n"
                    "@return A proxy with the specified collocation optimization.");
    out << nl << "@Override";
    out << nl << "default " << p->name() << "Prx ice_collocationOptimized(boolean b)";
    out << sb;
    out << nl << "return (" << p->name() << "Prx)_ice_collocationOptimized(b);";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Returns a proxy that is identical to this proxy, but uses twoway invocations.\n"
                    "@return A proxy that uses twoway invocations.");
    out << nl << "@Override";
    out << nl << "default " << p->name() << "Prx ice_twoway()";
    out << sb;
    out << nl << "return (" << p->name() << "Prx)_ice_twoway();";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Returns a proxy that is identical to this proxy, but uses oneway invocations.\n"
                    "@return A proxy that uses oneway invocations.");
    out << nl << "@Override";
    out << nl << "default " << p->name() << "Prx ice_oneway()";
    out << sb;
    out << nl << "return (" << p->name() << "Prx)_ice_oneway();";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Returns a proxy that is identical to this proxy, but uses batch oneway invocations.\n"
                    "@return A proxy that uses batch oneway invocations.");
    out << nl << "@Override";
    out << nl << "default " << p->name() << "Prx ice_batchOneway()";
    out << sb;
    out << nl << "return (" << p->name() << "Prx)_ice_batchOneway();";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Returns a proxy that is identical to this proxy, but uses datagram invocations.\n"
                    "@return A proxy that uses datagram invocations.");
    out << nl << "@Override";
    out << nl << "default " << p->name() << "Prx ice_datagram()";
    out << sb;
    out << nl << "return (" << p->name() << "Prx)_ice_datagram();";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Returns a proxy that is identical to this proxy, but uses batch datagram invocations.\n"
                    "@return A proxy that uses batch datagram invocations.");
    out << nl << "@Override";
    out << nl << "default " << p->name() << "Prx ice_batchDatagram()";
    out << sb;
    out << nl << "return (" << p->name() << "Prx)_ice_batchDatagram();";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Returns a proxy that is identical to this proxy, except for compression.\n"
                    "@param co <code>true</code> enables compression for the new proxy; <code>false</code> disables compression.\n"
                    "@return A proxy with the specified compression setting.");
    out << nl << "@Override";
    out << nl << "default " << p->name() << "Prx ice_compress(boolean co)";
    out << sb;
    out << nl << "return (" << p->name() << "Prx)_ice_compress(co);";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Returns a proxy that is identical to this proxy, except for its connection timeout setting.\n"
                    "@param t The connection timeout for the proxy in milliseconds.\n"
                    "@return A proxy with the specified timeout.");
    out << nl << "@Override";
    out << nl << "default " << p->name() << "Prx ice_timeout(int t)";
    out << sb;
    out << nl << "return (" << p->name() << "Prx)_ice_timeout(t);";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Returns a proxy that is identical to this proxy, except for its connection ID.\n"
                    "@param connectionId The connection ID for the new proxy. An empty string removes the connection ID.\n"
                    "@return A proxy with the specified connection ID.");
    out << nl << "@Override";
    out << nl << "default " << p->name() << "Prx ice_connectionId(String connectionId)";
    out << sb;
    out << nl << "return (" << p->name() << "Prx)_ice_connectionId(connectionId);";
    out << eb;

    out << sp;
    writeDocComment(out,
                    "Returns a proxy that is identical to this proxy, except it's a fixed proxy bound\n"
                    "the given connection."
                    "@param connection The fixed proxy connection.\n"
                    "@return A fixed proxy bound to the given connection.");
    out << nl << "@Override";
    out << nl << "default " << p->name() << "Prx ice_fixed(com.zeroc.Ice.Connection connection)";
    out << sb;
    out << nl << "return (" << p->name() << "Prx)_ice_fixed(connection);";
    out << eb;

    out << sp;
    out << nl << "static String ice_staticId()";
    out << sb;
    out << nl << "return \"" << p->scoped() << "\";";
    out << eb;

    out << eb;
    close();

    string absolute = getUnqualified(p, "", "_", "PrxI");

    open(absolute, p->file());

    Output& outi = output();

    outi << sp;
    writeHiddenDocComment(outi);
    if(dc && dc->isDeprecated())
    {
        outi << nl << "@Deprecated";
    }
    outi << nl << "public class _" << p->name() << "PrxI extends " << getUnqualified("com.zeroc.Ice._ObjectPrxI", package)
         << " implements " << p->name() << "Prx";
    outi << sb;
    outi << sp;
    writeHiddenDocComment(outi);
    outi << nl << "public static final long serialVersionUID = 0L;";
    outi << eb;
    close();
}

void
Slice::Gen::ProxyVisitor::visitOperation(const OperationPtr& p)
{
    const string name = fixKwd(p->name());
    const ContainerPtr container = p->container();
    const ClassDefPtr cl = ClassDefPtr::dynamicCast(container);
    const string package = getPackage(cl);

    Output& out = output();

    const TypePtr ret = p->returnType();
    const string retS = getResultType(p, package, false, false);
    const bool returnsParams = ret || !p->outParameters().empty();
    const vector<string> params = getParamsProxy(p, package, true);
    const vector<string> args = getInArgs(p);

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

    const string contextParamName = getEscapedParamName(p, "context");
    const string contextDoc = "@param " + contextParamName +" The Context map to send with the invocation.";
    const string contextParam = "java.util.Map<String, String> " + contextParamName;
    const string noExplicitContextArg = "com.zeroc.Ice.ObjectPrx.noExplicitContext";

    CommentPtr dc = p->parseComment(false);

    //
    // Synchronous methods.
    //
    out << sp;
    writeProxyDocComment(out, p, package, dc, false, "");
    if(dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }
    out << nl << "default " << retS << ' ' << name << spar << params << epar;
    writeThrowsClause(package, throws);
    out << sb;
    out << nl;
    if(returnsParams)
    {
        out << "return ";
    }
    out << name << spar << args << noExplicitContextArg << epar << ';';
    out << eb;

    out << sp;
    writeProxyDocComment(out, p, package, dc, false, contextDoc);
    if(dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }
    out << nl << "default " << retS << ' ' << name << spar << params << contextParam << epar;
    writeThrowsClause(package, throws);
    out << sb;
    if(throws.empty())
    {
        out << nl;
        if(returnsParams)
        {
            out << "return ";
        }
        out << "_iceI_" << p->name() << "Async" << spar << args << contextParamName << "true" << epar << ".waitForResponse();";
    }
    else
    {
        out << nl << "try";
        out << sb;
        out << nl;
        if(returnsParams)
        {
            out << "return ";
        }
        out << "_iceI_" << p->name() << "Async" << spar << args << contextParamName << "true" << epar << ".waitForResponseOrUserEx();";
        out << eb;
        for(const auto& t : throws)
        {
            string exS = getUnqualified(t, package);
            out << nl << "catch(" << exS << " ex)";
            out << sb;
            out << nl << "throw ex;";
            out << eb;
        }
        out << nl << "catch(" << getUnqualified("com.zeroc.Ice.UserException", package) << " ex)";
        out << sb;
        out << nl << "throw new " << getUnqualified("com.zeroc.Ice.UnknownUserException", package) << "(ex.ice_id(), ex);";
        out << eb;
    }
    out << eb;

    //
    // Asynchronous methods with required parameters.
    //
    out << sp;
    writeProxyDocComment(out, p, package, dc, true, "");

    const string futureType = getFutureType(p, package);

    if(dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }
    out << nl << "default " << futureType << ' ' << p->name() << "Async" << spar << params << epar;
    out << sb;
    out << nl << "return _iceI_" << p->name() << "Async" << spar << args << noExplicitContextArg << "false" << epar
        << ';';
    out << eb;

    out << sp;
    writeProxyDocComment(out, p, package, dc, true, contextDoc);
    if(dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }
    out << nl << "default " << futureType << ' ' << p->name() << "Async" << spar << params << contextParam << epar;
    out << sb;
    out << nl << "return _iceI_" << p->name() << "Async" << spar << args << contextParamName << "false" << epar << ';';
    out << eb;

    const string futureImpl = getFutureImplType(p, package);

    out << sp;
    writeHiddenProxyDocComment(out, p);
    out << nl << "default " << futureImpl << " _iceI_" << p->name() << "Async" << spar << getParamsProxy(p, package, true, true)
        << "java.util.Map<String, String> context"
        << "boolean sync" << epar;
    out << sb;
    out << nl << futureImpl << " f = new " << getUnqualified("com.zeroc.IceInternal.OutgoingAsync", package)
        << "<>(this, \"" << p->name() << "\", " << sliceModeToIceMode(p->sendMode()) << ", sync, "
        << (throws.empty() ? "null" : "_iceE_" + p->name()) << ");";

    out << nl << "f.invoke(";
    out.useCurrentPosAsIndent();
    out << (p->returnsData() ? "true" : "false") << ", context, " << opFormatTypeToString(p) << ", ";
    if(!p->inParameters().empty())
    {
        out << "ostr -> {";
        out.inc();
        writeMarshalProxyParams(out, package, p, true);
        out.dec();
        out << nl << '}';
    }
    else
    {
        out << "null";
    }
    out << ", ";
    if(returnsParams)
    {
        out << "istr -> {";
        out.inc();
        writeUnmarshalProxyResults(out, package, p);
        out.dec();
        out << nl << "}";
    }
    else
    {
        out << "null";
    }
    out.restoreIndent();
    out << ");";
    out << nl << "return f;";
    out << eb;

    if(!throws.empty())
    {
        out << sp;
        writeHiddenDocComment(out);
        out << nl << "static final Class<?>[] _iceE_" << p->name() << " =";
        out << sb;
        for(ExceptionList::const_iterator t = throws.begin(); t != throws.end(); ++t)
        {
            if(t != throws.begin())
            {
                out << ",";
            }
            out << nl << getUnqualified(*t, package) << ".class";
        }
        out << eb << ';';
    }
}

Slice::Gen::DispatcherVisitor::DispatcherVisitor(const string& dir) :
    JavaVisitor(dir)
{
}

bool
Slice::Gen::DispatcherVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isInterface() || p->allOperations().empty())
    {
        return false;
    }

    const string name = p->name();
    const string absolute = getUnqualified(p, "", "", "Disp");
    const string package = getPackage(p);

    open(absolute, p->file());

    Output& out = output();

    out << sp;
    CommentPtr dc = p->parseComment(false);
    writeDocComment(out, p->unit(), dc);
    if(dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }
    out << nl << "public interface " << name << "Disp";

    //
    // For dispatch purposes, we can ignore a base class if it has no operations.
    //
    ClassList bases = p->bases();
    if(!bases.empty() && !bases.front()->isInterface() && bases.front()->allOperations().empty())
    {
        bases.pop_front();
    }

    if(bases.empty())
    {
        out << " extends " << getUnqualified("com.zeroc.Ice.Object", package);
    }
    else
    {
        out << " extends ";
        out.useCurrentPosAsIndent();
        for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
        {
            if(q != bases.begin())
            {
                out << ',' << nl;
            }
            if(!(*q)->isInterface())
            {
                out << getUnqualified(*q, package, "", "Disp");
            }
            else
            {
                out << getUnqualified(*q, package);
            }
        }
        out.restoreIndent();
    }
    out << sb;

    writeDispatch(out, p);

    out << eb;
    close();

    return false;
}

Slice::Gen::ImplVisitor::ImplVisitor(const string& dir) :
    JavaVisitor(dir)
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
    string absolute = getUnqualified(p, "", "", "I");

    open(absolute, p->file());

    Output& out = output();

    out << sp << nl << "public final class " << name << 'I';
    if(p->isInterface())
    {
        out << " implements " << fixKwd(name);
    }
    else
    {
        out << " implements " << name << "Disp";
    }
    out << sb;

    out << nl << "public " << name << "I()";
    out << sb;
    out << eb;

    OperationList ops = p->allOperations();
    for(OperationList::iterator r = ops.begin(); r != ops.end(); ++r)
    {
        writeOperation(out, package, *r);
    }

    out << eb;
    close();

    return false;
}

string
Slice::Gen::ImplVisitor::getDefaultValue(const string& package, const TypePtr& type, bool optional)
{
    if(!optional)
    {
        const BuiltinPtr b = BuiltinPtr::dynamicCast(type);
        if(b)
        {
            switch(b->kind())
            {
                case Builtin::KindBool:
                {
                    return "false";
                    break;
                }
                case Builtin::KindByte:
                {
                    return "(byte)0";
                    break;
                }
                case Builtin::KindShort:
                {
                    return "(short)0";
                    break;
                }
                case Builtin::KindInt:
                case Builtin::KindLong:
                {
                    return "0";
                    break;
                }
                case Builtin::KindFloat:
                {
                    return "(float)0.0";
                    break;
                }
                case Builtin::KindDouble:
                {
                    return "0.0";
                    break;
                }
                case Builtin::KindString:
                {
                    return "\"\"";
                    break;
                }
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindValue:
                {
                    return "null";
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
                return getUnqualified(en, package) + '.' + fixKwd(enumerators.front()->name());
            }
        }
    }

    return "null";
}

bool
Slice::Gen::ImplVisitor::initResult(Output& out, const string& package, const OperationPtr& op)
{
    const string retS = getResultType(op, package, false, true);

    if(op->hasMarshaledResult())
    {
        out << nl << retS << " r = new " << retS << spar;
        const ParamDeclList outParams = op->outParameters();
        if(op->returnType())
        {
            out << getDefaultValue(package, op->returnType(), op->returnIsOptional());
        }
        for(const auto& p : outParams)
        {
            out << getDefaultValue(package, p->type(), p->optional());
        }
        out << "current" << epar << ';';
    }
    else if(op->returnsMultipleValues())
    {
        out << nl << retS << " r = new " << retS << "();";
        string retval = "returnValue";
        const ParamDeclList outParams = op->outParameters();
        for(const auto& p : outParams)
        {
            out << nl << "r." << fixKwd(p->name()) << " = "
                << getDefaultValue(package, p->type(), p->optional()) << ';';
            if(p->name() == "returnValue")
            {
                retval = "_returnValue";
            }
        }
        if(op->returnType())
        {
            out << nl << "r." << retval << " = "
                << getDefaultValue(package, op->returnType(), op->returnIsOptional()) << ';';
        }
    }
    else
    {
        TypePtr type = op->returnType();
        bool optional = op->returnIsOptional();
        if(!type)
        {
            const ParamDeclList outParams = op->outParameters();
            if(!outParams.empty())
            {
                assert(outParams.size() == 1);
                type = outParams.front()->type();
                optional = outParams.front()->optional();
            }
        }
        if(type)
        {
            out << nl << retS << " r = " << getDefaultValue(package, type, optional) << ';';
        }
        else
        {
            return false;
        }
    }

    return true;
}

void
Slice::Gen::ImplVisitor::writeOperation(Output& out, const string& package, const OperationPtr& op)
{
    string opName = op->name();

    ExceptionList throws = op->throws();
    throws.sort();
    throws.unique();

    const ContainerPtr container = op->container();
    const ClassDefPtr cl = ClassDefPtr::dynamicCast(container);

    const vector<string> params = getParams(op, package);
    const string currentParam = getUnqualified("com.zeroc.Ice.Current", package) + " " + getEscapedParamName(op, "current");

    const bool amd = cl->hasMetaData("amd") || op->hasMetaData("amd");

    if(amd)
    {
        const string retS = getResultType(op, package, true, true);
        out << sp;
        out << nl << "@Override";
        out << nl << "public java.util.concurrent.CompletionStage<" << retS << "> " << opName << "Async" << spar
            << params << currentParam << epar;
        writeThrowsClause(package, throws, op);
        out << sb;
        if(initResult(out, package, op))
        {
            out << nl << "return java.util.concurrent.CompletableFuture.completedFuture(r);";
        }
        else
        {
            out << nl << "return java.util.concurrent.CompletableFuture.completedFuture((Void)null);";
        }
        out << eb;
    }
    else
    {
        out << sp;
        out << nl << "@Override";
        out << nl << "public " << getResultType(op, package, false, true) << ' ' << fixKwd(opName) << spar << params
            << currentParam << epar;
        writeThrowsClause(package, throws, op);
        out << sb;
        if(initResult(out, package, op))
        {
            out << nl << "return r;";
        }
        out << eb;
    }
}
