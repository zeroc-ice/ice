//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Gen.h"
#include "../Slice/Util.h"
#include "Ice/StringUtil.h"
#include <cstring>

#include <algorithm>
#include <cassert>
#include <limits>

using namespace std;
using namespace Slice;
using namespace IceInternal;

namespace
{
    string sliceModeToIceMode(Operation::Mode opMode)
    {
        string mode = "com.zeroc.Ice.OperationMode.";
        switch (opMode)
        {
            case Operation::Normal:
                mode = "null"; // shorthand for most common case
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

    string opFormatTypeToString(const OperationPtr& op)
    {
        optional<FormatType> opFormat = op->format();
        if (opFormat)
        {
            switch (*opFormat)
            {
                case CompactFormat:
                    return "com.zeroc.Ice.FormatType.CompactFormat";
                case SlicedFormat:
                    return "com.zeroc.Ice.FormatType.SlicedFormat";
                default:
                    assert(false);
                    return "???";
            }
        }
        else
        {
            return "null";
        }
    }

    string getEscapedParamName(const OperationPtr& p, const string& name)
    {
        ParameterList params = p->parameters();

        for (ParameterList::const_iterator i = params.begin(); i != params.end(); ++i)
        {
            if ((*i)->name() == name)
            {
                return name + "_";
            }
        }
        return name;
    }

    // Returns java.util.OptionalXXX.ofYYY depending on the type
    string ofFactory(const TypePtr& type)
    {
        const BuiltinPtr b = dynamic_pointer_cast<Builtin>(type);

        if (b)
        {
            if (b->kind() == Builtin::KindInt)
            {
                return "java.util.OptionalInt.of";
            }
            else if (b->kind() == Builtin::KindLong)
            {
                return "java.util.OptionalLong.of";
            }
            else if (b->kind() == Builtin::KindDouble)
            {
                return "java.util.OptionalDouble.of";
            }
            else if (b->kind() < Builtin::KindString)
            {
                return "java.util.Optional.of";
            }
        }

        return "java.util.Optional.ofNullable";
    }

    /// Returns a javadoc formatted link to the provided Slice identifier.
    string javaLinkFormatter(string identifier, string memberComponent)
    {
        string result = "{@link ";
        if (!identifier.empty())
        {
            result += Slice::JavaGenerator::fixKwd(identifier);
        }
        if (!memberComponent.empty())
        {
            result += "#" + Slice::JavaGenerator::fixKwd(memberComponent);
        }
        return result + "}";
    }
}

Slice::JavaVisitor::JavaVisitor(const string& dir) : JavaGenerator(dir) {}

Slice::JavaVisitor::~JavaVisitor() = default;

string
Slice::JavaVisitor::getResultType(const OperationPtr& op, const string& package, bool object, bool dispatch)
{
    if (dispatch && op->hasMarshaledResult())
    {
        const InterfaceDefPtr interface = op->interface();
        assert(interface);
        string abs = getUnqualified(interface, package);
        string name = op->name();
        name[0] = static_cast<char>(toupper(static_cast<unsigned char>(name[0])));
        return abs + "." + name + "MarshaledResult";
    }
    else if (op->returnsMultipleValues())
    {
        const ContainedPtr c = dynamic_pointer_cast<Contained>(op->container());
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
        if (!type)
        {
            const ParameterList outParams = op->outParameters();
            if (!outParams.empty())
            {
                assert(outParams.size() == 1);
                type = outParams.front()->type();
                optional = outParams.front()->optional();
            }
        }
        if (type)
        {
            if (optional)
            {
                return typeToString(type, TypeModeReturn, package, op->getMetadata(), true, true);
            }
            else if (object)
            {
                return typeToObjectString(type, TypeModeReturn, package, op->getMetadata(), true);
            }
            else
            {
                return typeToString(type, TypeModeReturn, package, op->getMetadata(), true, false);
            }
        }
        else
        {
            return object ? "Void" : "void";
        }
    }
}

void
Slice::JavaVisitor::writeResultType(Output& out, const OperationPtr& op, const string& package, const DocCommentPtr& dc)
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
    const ParameterList outParams = op->outParameters();
    for (ParameterList::const_iterator p = outParams.begin(); p != outParams.end(); ++p)
    {
        if ((*p)->name() == "returnValue")
        {
            retval = "_returnValue";
            break;
        }
    }

    const TypePtr ret = op->returnType();

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

    bool needMandatoryOnly = false;
    bool generateMandatoryOnly = false;

    do
    {
        out << sp;

        if (needMandatoryOnly)
        {
            generateMandatoryOnly = true;
            needMandatoryOnly = false;
        }

        if (dc)
        {
            //
            // Emit a doc comment for the constructor if necessary.
            //
            out << nl << "/**";
            out << nl << " * This constructor makes shallow copies of the results for operation " << opName;
            if (generateMandatoryOnly)
            {
                out << " (overload without Optional parameters).";
            }
            else
            {
                out << '.';
            }

            if (ret && !dc->returns().empty())
            {
                out << nl << " * @param " << retval << ' ';
                writeDocCommentLines(out, dc->returns());
            }
            map<string, StringList> paramDocs = dc->parameters();
            for (ParameterList::const_iterator p = outParams.begin(); p != outParams.end(); ++p)
            {
                const string name = (*p)->name();
                map<string, StringList>::const_iterator q = paramDocs.find(name);
                if (q != paramDocs.end() && !q->second.empty())
                {
                    out << nl << " * @param " << fixKwd(q->first) << ' ';
                    writeDocCommentLines(out, q->second);
                }
            }
            out << nl << " **/";
        }

        out << nl << "public " << opName << "Result" << spar;

        if (ret)
        {
            out
                << (typeToString(
                        ret,
                        TypeModeIn,
                        package,
                        op->getMetadata(),
                        true,
                        !generateMandatoryOnly && op->returnIsOptional()) +
                    " " + retval);
            needMandatoryOnly = !generateMandatoryOnly && op->returnIsOptional();
        }
        for (ParameterList::const_iterator p = outParams.begin(); p != outParams.end(); ++p)
        {
            out
                << (typeToString(
                        (*p)->type(),
                        TypeModeIn,
                        package,
                        (*p)->getMetadata(),
                        true,
                        !generateMandatoryOnly && (*p)->optional()) +
                    " " + fixKwd((*p)->name()));
            if (!generateMandatoryOnly)
            {
                needMandatoryOnly = needMandatoryOnly || (*p)->optional();
            }
        }
        out << epar;
        out << sb;
        if (ret)
        {
            out << nl << "this." << retval << " = ";
            if (op->returnIsOptional() && generateMandatoryOnly)
            {
                out << ofFactory(ret) << "(" << retval << ");";
            }
            else
            {
                out << retval << ';';
            }
        }
        for (ParameterList::const_iterator p = outParams.begin(); p != outParams.end(); ++p)
        {
            const string name = fixKwd((*p)->name());
            out << nl << "this." << name << " = ";
            if ((*p)->optional() && generateMandatoryOnly)
            {
                out << ofFactory((*p)->type()) << "(" << name << ");";
            }
            else
            {
                out << name << ';';
            }
        }
        out << eb;
    } while (needMandatoryOnly);

    //
    // Members.
    //
    out << sp;
    if (ret)
    {
        if (dc && !dc->returns().empty())
        {
            out << nl << "/**";
            out << nl << " * ";
            writeDocCommentLines(out, dc->returns());
            out << nl << " **/";
        }
        out << nl << "public "
            << typeToString(ret, TypeModeIn, package, op->getMetadata(), true, op->returnIsOptional()) << ' ' << retval
            << ';';
    }

    for (ParameterList::const_iterator p = outParams.begin(); p != outParams.end(); ++p)
    {
        if (dc)
        {
            const string name = (*p)->name();
            map<string, StringList> paramDocs = dc->parameters();
            map<string, StringList>::const_iterator q = paramDocs.find(name);
            if (q != paramDocs.end() && !q->second.empty())
            {
                out << nl << "/**";
                out << nl << " * ";
                writeDocCommentLines(out, q->second);
                out << nl << " **/";
            }
        }
        out << nl << "public "
            << typeToString((*p)->type(), TypeModeIn, package, (*p)->getMetadata(), true, (*p)->optional()) << ' '
            << fixKwd((*p)->name()) << ';';
    }

    ParameterList required, optional;
    op->outParameters(required, optional);

    out << sp << nl << "public void write(com.zeroc.Ice.OutputStream ostr)";
    out << sb;

    int iter = 0;
    for (ParameterList::const_iterator pli = required.begin(); pli != required.end(); ++pli)
    {
        const string paramName = fixKwd((*pli)->name());
        writeMarshalUnmarshalCode(
            out,
            package,
            (*pli)->type(),
            OptionalNone,
            false,
            0,
            "this." + paramName,
            true,
            iter,
            "",
            (*pli)->getMetadata());
    }

    if (ret && !op->returnIsOptional())
    {
        writeMarshalUnmarshalCode(out, package, ret, OptionalNone, false, 0, retval, true, iter, "", op->getMetadata());
    }

    //
    // Handle optional parameters.
    //
    bool checkReturnType = op->returnIsOptional();

    for (ParameterList::const_iterator pli = optional.begin(); pli != optional.end(); ++pli)
    {
        if (checkReturnType && op->returnTag() < (*pli)->tag())
        {
            writeMarshalUnmarshalCode(
                out,
                package,
                ret,
                OptionalReturnParam,
                true,
                op->returnTag(),
                retval,
                true,
                iter,
                "",
                op->getMetadata());
            checkReturnType = false;
        }

        const string paramName = fixKwd((*pli)->name());
        writeMarshalUnmarshalCode(
            out,
            package,
            (*pli)->type(),
            OptionalOutParam,
            true,
            (*pli)->tag(),
            "this." + paramName,
            true,
            iter,
            "",
            (*pli)->getMetadata());
    }

    if (checkReturnType)
    {
        writeMarshalUnmarshalCode(
            out,
            package,
            ret,
            OptionalReturnParam,
            true,
            op->returnTag(),
            retval,
            true,
            iter,
            "",
            op->getMetadata());
    }

    out << eb;

    out << sp << nl << "public void read(com.zeroc.Ice.InputStream istr)";
    out << sb;

    iter = 0;
    for (ParameterList::const_iterator pli = required.begin(); pli != required.end(); ++pli)
    {
        const string paramName = fixKwd((*pli)->name());
        const string patchParams = getPatcher((*pli)->type(), package, "this." + paramName);
        writeMarshalUnmarshalCode(
            out,
            package,
            (*pli)->type(),
            OptionalNone,
            false,
            0,
            "this." + paramName,
            false,
            iter,
            "",
            (*pli)->getMetadata(),
            patchParams);
    }

    if (ret && !op->returnIsOptional())
    {
        const string patchParams = getPatcher(ret, package, retval);
        writeMarshalUnmarshalCode(
            out,
            package,
            ret,
            OptionalNone,
            false,
            0,
            retval,
            false,
            iter,
            "",
            op->getMetadata(),
            patchParams);
    }

    //
    // Handle optional parameters.
    //
    checkReturnType = op->returnIsOptional();

    for (ParameterList::const_iterator pli = optional.begin(); pli != optional.end(); ++pli)
    {
        if (checkReturnType && op->returnTag() < (*pli)->tag())
        {
            const string patchParams = getPatcher(ret, package, retval);
            writeMarshalUnmarshalCode(
                out,
                package,
                ret,
                OptionalReturnParam,
                true,
                op->returnTag(),
                retval,
                false,
                iter,
                "",
                op->getMetadata(),
                patchParams);
            checkReturnType = false;
        }

        const string paramName = fixKwd((*pli)->name());
        const string patchParams = getPatcher((*pli)->type(), package, paramName);
        writeMarshalUnmarshalCode(
            out,
            package,
            (*pli)->type(),
            OptionalOutParam,
            true,
            (*pli)->tag(),
            "this." + paramName,
            false,
            iter,
            "",
            (*pli)->getMetadata(),
            patchParams);
    }

    if (checkReturnType)
    {
        const string patchParams = getPatcher(ret, package, retval);
        writeMarshalUnmarshalCode(
            out,
            package,
            ret,
            OptionalReturnParam,
            true,
            op->returnTag(),
            retval,
            false,
            iter,
            "",
            op->getMetadata(),
            patchParams);
    }

    out << eb;

    out << eb;
}

void
Slice::JavaVisitor::writeMarshaledResultType(
    Output& out,
    const OperationPtr& op,
    const string& package,
    const DocCommentPtr& dc)
{
    string opName = op->name();
    const TypePtr ret = op->returnType();
    opName[0] = static_cast<char>(toupper(static_cast<unsigned char>(opName[0])));

    out << sp;
    writeDocComment(out, "Holds the marshaled result of operation " + op->name() + ".");
    out << nl << "public static class " << opName << "MarshaledResult implements com.zeroc.Ice.MarshaledResult" << sb;

    const ParameterList outParams = op->outParameters();
    const string retval = getEscapedParamName(op, "returnValue");
    const string currentParamName = getEscapedParamName(op, "current");
    const string currentParam = "com.zeroc.Ice.Current " + currentParamName;

    out << sp;

    //
    // Emit a doc comment for the constructor if necessary.
    //
    if (dc)
    {
        out << nl << "/**";
        out << nl << " * This constructor marshals the results of operation " << op->name() << " immediately.";

        if (ret && !dc->returns().empty())
        {
            out << nl << " * @param " << retval << ' ';
            writeDocCommentLines(out, dc->returns());
        }
        map<string, StringList> paramDocs = dc->parameters();
        for (ParameterList::const_iterator p = outParams.begin(); p != outParams.end(); ++p)
        {
            const string name = (*p)->name();
            map<string, StringList>::const_iterator q = paramDocs.find(name);
            if (q != paramDocs.end() && !q->second.empty())
            {
                out << nl << " * @param " << fixKwd(q->first) << ' ';
                writeDocCommentLines(out, q->second);
            }
        }
        out << nl << " * @param " << currentParamName << " The Current object for the invocation.";
        out << nl << " **/";
    }

    bool hasOpt = false;
    out << nl << "public " << opName << "MarshaledResult" << spar;
    if (ret)
    {
        out << (typeToString(ret, TypeModeIn, package, op->getMetadata(), true, op->returnIsOptional()) + " " + retval);
        hasOpt = op->returnIsOptional();
    }
    for (ParameterList::const_iterator p = outParams.begin(); p != outParams.end(); ++p)
    {
        out
            << (typeToString((*p)->type(), TypeModeIn, package, (*p)->getMetadata(), true, (*p)->optional()) + " " +
                fixKwd((*p)->name()));

        hasOpt = hasOpt || (*p)->optional();
    }
    out << currentParam << epar;
    out << sb;
    out << nl << "_ostr = " << currentParamName << ".startReplyStream();";
    out << nl << "_ostr.startEncapsulation(" << currentParamName << ".encoding, " << opFormatTypeToString(op) << ");";

    ParameterList required, optional;
    op->outParameters(required, optional);
    int iter = 0;
    for (ParameterList::const_iterator pli = required.begin(); pli != required.end(); ++pli)
    {
        const string paramName = fixKwd((*pli)->name());
        writeMarshalUnmarshalCode(
            out,
            package,
            (*pli)->type(),
            OptionalNone,
            false,
            0,
            paramName,
            true,
            iter,
            "_ostr",
            (*pli)->getMetadata());
    }

    if (ret && !op->returnIsOptional())
    {
        writeMarshalUnmarshalCode(
            out,
            package,
            ret,
            OptionalNone,
            false,
            0,
            retval,
            true,
            iter,
            "_ostr",
            op->getMetadata());
    }

    //
    // Handle optional parameters.
    //
    bool checkReturnType = op->returnIsOptional();

    for (ParameterList::const_iterator pli = optional.begin(); pli != optional.end(); ++pli)
    {
        if (checkReturnType && op->returnTag() < (*pli)->tag())
        {
            writeMarshalUnmarshalCode(
                out,
                package,
                ret,
                OptionalReturnParam,
                true,
                op->returnTag(),
                retval,
                true,
                iter,
                "_ostr",
                op->getMetadata());
            checkReturnType = false;
        }

        const string paramName = fixKwd((*pli)->name());
        writeMarshalUnmarshalCode(
            out,
            package,
            (*pli)->type(),
            OptionalOutParam,
            true,
            (*pli)->tag(),
            paramName,
            true,
            iter,
            "_ostr",
            (*pli)->getMetadata());
    }

    if (checkReturnType)
    {
        writeMarshalUnmarshalCode(
            out,
            package,
            ret,
            OptionalReturnParam,
            true,
            op->returnTag(),
            retval,
            true,
            iter,
            "_ostr",
            op->getMetadata());
    }

    if (op->returnsClasses())
    {
        out << nl << "_ostr.writePendingValues();";
    }

    out << nl << "_ostr.endEncapsulation();";

    out << eb;

    if (hasOpt)
    {
        out << sp;

        //
        // Emit a doc comment for the constructor if necessary.
        //
        if (dc)
        {
            out << nl << "/**";
            out << nl << " * This constructor marshals the results of operation " << op->name()
                << " immediately (overload without Optional parameters).";

            if (ret && !dc->returns().empty())
            {
                out << nl << " * @param " << retval << ' ';
                writeDocCommentLines(out, dc->returns());
            }
            map<string, StringList> paramDocs = dc->parameters();
            for (ParameterList::const_iterator p = outParams.begin(); p != outParams.end(); ++p)
            {
                const string name = (*p)->name();
                map<string, StringList>::const_iterator q = paramDocs.find(name);
                if (q != paramDocs.end() && !q->second.empty())
                {
                    out << nl << " * @param " << fixKwd(q->first) << ' ';
                    writeDocCommentLines(out, q->second);
                }
            }
            out << nl << " * @param " << currentParamName << " The Current object for the invocation.";
            out << nl << " **/";
        }

        out << nl << "public " << opName << "MarshaledResult" << spar;
        if (ret)
        {
            out << (typeToString(ret, TypeModeIn, package, op->getMetadata(), true, false) + " " + retval);
        }
        for (ParameterList::const_iterator p = outParams.begin(); p != outParams.end(); ++p)
        {
            out
                << (typeToString((*p)->type(), TypeModeIn, package, (*p)->getMetadata(), true, false) + " " +
                    fixKwd((*p)->name()));
        }

        out << currentParam << epar;
        out << sb;
        out << nl << "this" << spar;
        if (ret)
        {
            if (op->returnIsOptional())
            {
                out << ofFactory(ret) + "(" + retval + ")";
            }
            else
            {
                out << retval;
            }
        }
        for (ParameterList::const_iterator p = outParams.begin(); p != outParams.end(); ++p)
        {
            if ((*p)->optional())
            {
                out << ofFactory((*p)->type()) + "(" + fixKwd((*p)->name()) + ")";
            }
            else
            {
                out << fixKwd((*p)->name());
            }
        }

        out << currentParamName << epar << ';';
        out << eb;
    }

    out << sp;
    out << nl << "@Override" << nl << "public com.zeroc.Ice.OutputStream getOutputStream()" << sb << nl
        << "return _ostr;" << eb;

    out << sp;
    out << nl << "private com.zeroc.Ice.OutputStream _ostr;";
    out << eb;
}

void
Slice::JavaVisitor::allocatePatcher(Output& out, const TypePtr& type, const string& package, const string& name)
{
    BuiltinPtr b = dynamic_pointer_cast<Builtin>(type);
    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    assert((b && b->usesClasses()) || cl);

    string clsName;
    if (b)
    {
        clsName = "com.zeroc.Ice.Value";
    }
    else
    {
        clsName = getUnqualified(cl, package);
    }
    out << nl << "final com.zeroc.Ice.Holder<" << clsName << "> " << name << " = new com.zeroc.Ice.Holder<>();";
}

string
Slice::JavaVisitor::getPatcher(const TypePtr& type, const string& package, const string& dest)
{
    BuiltinPtr b = dynamic_pointer_cast<Builtin>(type);
    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    ostringstream ostr;
    if ((b && b->usesClasses()) || cl)
    {
        string clsName;
        if (b)
        {
            clsName = "com.zeroc.Ice.Value";
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
    if (op->returnType() || op->outParameters().size() > 0)
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
    if (op->returnType() || op->outParameters().size() > 0)
    {
        return "com.zeroc.Ice.OutgoingAsync<" + getResultType(op, package, true, false) + ">";
    }
    else
    {
        return "com.zeroc.Ice.OutgoingAsync<Void>";
    }
}

vector<string>
Slice::JavaVisitor::getParams(const OperationPtr& op, const string& package)
{
    vector<string> params;

    const ParameterList paramList = op->inParameters();
    for (ParameterList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        const string type =
            typeToString((*q)->type(), TypeModeIn, package, (*q)->getMetadata(), true, (*q)->optional());
        params.push_back(type + ' ' + fixKwd((*q)->name()));
    }

    return params;
}

vector<string>
Slice::JavaVisitor::getParamsProxy(const OperationPtr& op, const string& package, bool optionalMapping, bool internal)
{
    vector<string> params;

    ParameterList inParams = op->inParameters();
    for (ParameterList::const_iterator q = inParams.begin(); q != inParams.end(); ++q)
    {
        const string typeString = typeToString(
            (*q)->type(),
            TypeModeIn,
            package,
            (*q)->getMetadata(),
            true,
            optionalMapping && (*q)->optional());
        params.push_back(typeString + ' ' + (internal ? "iceP_" + (*q)->name() : fixKwd((*q)->name())));
    }

    return params;
}

vector<string>
Slice::JavaVisitor::getArgs(const OperationPtr& op)
{
    vector<string> args;

    ParameterList paramList = op->parameters();
    for (ParameterList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        args.push_back(fixKwd((*q)->name()));
    }

    return args;
}

vector<string>
Slice::JavaVisitor::getInArgs(const OperationPtr& op, bool internal)
{
    vector<string> args;

    ParameterList paramList = op->inParameters();
    for (ParameterList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        string s = internal ? "iceP_" + (*q)->name() : fixKwd((*q)->name());
        args.push_back(s);
    }

    return args;
}

void
Slice::JavaVisitor::writeMarshalProxyParams(
    Output& out,
    const string& package,
    const OperationPtr& op,
    bool optionalMapping)
{
    int iter = 0;
    ParameterList required, optional;
    op->inParameters(required, optional);
    for (ParameterList::const_iterator pli = required.begin(); pli != required.end(); ++pli)
    {
        string paramName = "iceP_" + (*pli)->name();
        writeMarshalUnmarshalCode(
            out,
            package,
            (*pli)->type(),
            OptionalNone,
            false,
            0,
            paramName,
            true,
            iter,
            "",
            (*pli)->getMetadata());
    }

    //
    // Handle optional parameters.
    //
    for (ParameterList::const_iterator pli = optional.begin(); pli != optional.end(); ++pli)
    {
        writeMarshalUnmarshalCode(
            out,
            package,
            (*pli)->type(),
            OptionalInParam,
            optionalMapping,
            (*pli)->tag(),
            "iceP_" + (*pli)->name(),
            true,
            iter,
            "",
            (*pli)->getMetadata());
    }

    if (op->sendsClasses())
    {
        out << nl << "ostr.writePendingValues();";
    }
}

void
Slice::JavaVisitor::writeUnmarshalProxyResults(Output& out, const string& package, const OperationPtr& op)
{
    const ParameterList outParams = op->outParameters();
    const TypePtr ret = op->returnType();
    const string name = "ret";

    if (op->returnsMultipleValues())
    {
        string resultType = getResultType(op, package, false, false);
        out << nl << resultType << ' ' << name << " = new " << resultType << "();";
        out << nl << name << ".read(istr);";
        if (op->returnsClasses())
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
        MetadataList metadata;
        if (ret)
        {
            type = ret;
            optional = op->returnIsOptional();
            tag = op->returnTag();
            metadata = op->getMetadata();
        }
        else
        {
            assert(outParams.size() == 1);
            optional = outParams.front()->optional();
            type = outParams.front()->type();
            tag = outParams.front()->tag();
            metadata = outParams.front()->getMetadata();
        }

        const bool val = type->isClassType();

        int iter = 0;

        if (val)
        {
            assert(!optional); // Optional classes are disallowed by the parser.
            allocatePatcher(out, type, package, name);
        }
        else
        {
            out << nl << resultType << ' ' << name << ';';
        }
        string patchParams = getPatcher(type, package, name + ".value");
        if (optional)
        {
            writeMarshalUnmarshalCode(
                out,
                package,
                type,
                ret ? OptionalReturnParam : OptionalOutParam,
                true,
                tag,
                name,
                false,
                iter,
                "",
                metadata,
                patchParams);
        }
        else
        {
            writeMarshalUnmarshalCode(
                out,
                package,
                type,
                OptionalNone,
                false,
                0,
                name,
                false,
                iter,
                "",
                metadata,
                patchParams);
        }

        if (op->returnsClasses())
        {
            out << nl << "istr.readPendingValues();";
        }

        if (val)
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
Slice::JavaVisitor::writeMarshalServantResults(
    Output& out,
    const string& package,
    const OperationPtr& op,
    const string& param)
{
    if (op->returnsMultipleValues())
    {
        out << nl << param << ".write(ostr);";
    }
    else
    {
        const ParameterList params = op->outParameters();
        bool optional;
        OptionalMode mode;
        TypePtr type;
        int tag;
        MetadataList metadata;
        if (op->returnType())
        {
            type = op->returnType();
            optional = op->returnIsOptional();
            mode = optional ? OptionalReturnParam : OptionalNone;
            tag = op->returnTag();
            metadata = op->getMetadata();
        }
        else
        {
            assert(params.size() == 1);
            optional = params.front()->optional();
            mode = optional ? OptionalOutParam : OptionalNone;
            type = params.front()->type();
            tag = params.front()->tag();
            metadata = params.front()->getMetadata();
        }

        int iter = 0;
        writeMarshalUnmarshalCode(out, package, type, mode, true, tag, param, true, iter, "", metadata);
    }

    if (op->returnsClasses())
    {
        out << nl << "ostr.writePendingValues();";
    }
}

void
Slice::JavaVisitor::writeThrowsClause(const string& package, const ExceptionList& throws, const OperationPtr& op)
{
    Output& out = output();

    if (op && (op->hasMetadata("java:UserException") || op->hasMetadata("UserException")))
    {
        out.inc();
        out << nl << "throws com.zeroc.Ice.UserException";
        out.dec();
    }
    else if (throws.size() > 0)
    {
        out.inc();
        out << nl << "throws ";
        out.useCurrentPosAsIndent();
        int count = 0;
        for (ExceptionList::const_iterator r = throws.begin(); r != throws.end(); ++r)
        {
            if (count > 0)
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
Slice::JavaVisitor::writeMarshalDataMember(
    Output& out,
    const string& package,
    const DataMemberPtr& member,
    int& iter,
    bool forStruct)
{
    if (member->optional())
    {
        assert(!forStruct);
        out << nl << "if(_" << member->name() << ")";
        out << sb;
        writeMarshalUnmarshalCode(
            out,
            package,
            member->type(),
            OptionalInParam,
            false,
            member->tag(),
            fixKwd(member->name()),
            true,
            iter,
            "ostr_",
            member->getMetadata());
        out << eb;
    }
    else
    {
        string stream = forStruct ? "" : "ostr_";
        string memberName = fixKwd(member->name());
        if (forStruct)
        {
            memberName = "this." + memberName;
        }

        writeMarshalUnmarshalCode(
            out,
            package,
            member->type(),
            OptionalNone,
            false,
            0,
            memberName,
            true,
            iter,
            stream,
            member->getMetadata());
    }
}

void
Slice::JavaVisitor::writeUnmarshalDataMember(
    Output& out,
    const string& package,
    const DataMemberPtr& member,
    int& iter,
    bool forStruct)
{
    const string patchParams = getPatcher(member->type(), package, fixKwd(member->name()));

    if (member->optional())
    {
        assert(!forStruct);
        out << nl << "if(_" << member->name() << " = istr_.readOptional(" << member->tag() << ", "
            << getOptionalFormat(member->type()) << "))";
        out << sb;
        writeMarshalUnmarshalCode(
            out,
            package,
            member->type(),
            OptionalMember,
            false,
            0,
            fixKwd(member->name()),
            false,
            iter,
            "istr_",
            member->getMetadata(),
            patchParams);
        out << eb;
    }
    else
    {
        string stream = forStruct ? "" : "istr_";
        string memberName = fixKwd(member->name());
        if (forStruct)
        {
            memberName = "this." + memberName;
        }

        writeMarshalUnmarshalCode(
            out,
            package,
            member->type(),
            OptionalNone,
            false,
            0,
            memberName,
            false,
            iter,
            stream,
            member->getMetadata(),
            patchParams);
    }
}

void
Slice::JavaVisitor::writeDispatch(Output& out, const InterfaceDefPtr& p)
{
    const string name = fixKwd(p->name());
    const string package = getPackage(p);
    const string scoped = p->scoped();
    const InterfaceList bases = p->bases();
    const OperationList ops = p->operations();

    for (const auto& op : ops)
    {
        DocCommentPtr dc = op->parseDocComment(javaLinkFormatter);
        vector<string> params = getParams(op, package);
        const string currentParam = "com.zeroc.Ice.Current " + getEscapedParamName(op, "current");

        const bool amd = p->hasMetadata("amd") || op->hasMetadata("amd");

        ExceptionList throws = op->throws();

        out << sp;
        writeServantDocComment(out, op, package, dc, amd);

        if (amd)
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

    StringList ids = p->ids();

    out << sp;
    writeHiddenDocComment(out);
    out << nl << "static final String[] _iceIds =";
    out << sb;

    for (StringList::const_iterator q = ids.begin(); q != ids.end();)
    {
        out << nl << '"' << *q << '"';
        if (++q != ids.end())
        {
            out << ',';
        }
    }
    out << eb << ';';

    out << sp << nl << "@Override" << nl << "default String[] ice_ids(com.zeroc.Ice.Current current)";
    out << sb;
    out << nl << "return _iceIds;";
    out << eb;

    out << sp << nl << "@Override" << nl << "default String ice_id(com.zeroc.Ice.Current current)";
    out << sb;
    out << nl << "return ice_staticId();";
    out << eb;

    out << sp << nl;
    out << "static String ice_staticId()";
    out << sb;

    out << nl << "return \"" << p->scoped() << "\";";
    out << eb;

    // Dispatch methods. We only generate methods for operations
    // defined in this InterfaceDef, because we reuse existing methods
    // for inherited operations.
    for (const auto& op : ops)
    {
        string opName = op->name();
        out << sp;

        writeHiddenDocComment(out);
        out << nl << "static java.util.concurrent.CompletionStage<com.zeroc.Ice.OutgoingResponse> _iceD_" << opName
            << '(' << name << " obj, com.zeroc.Ice.IncomingRequest request)";
        if (!op->throws().empty() || op->hasMetadata("java:UserException") || op->hasMetadata("UserException"))
        {
            out.inc();
            out << nl << "throws com.zeroc.Ice.UserException";
            out.dec();
        }
        out << sb;

        const bool amd = p->hasMetadata("amd") || op->hasMetadata("amd");

        const TypePtr ret = op->returnType();
        const ParameterList inParams = op->inParameters();
        const ParameterList outParams = op->outParameters();

        out << nl << "com.zeroc.Ice.Object._iceCheckMode(" << sliceModeToIceMode(op->mode())
            << ", request.current.mode);";

        if (!inParams.empty())
        {
            ParameterList values;

            //
            // Declare 'in' parameters.
            //
            out << nl << "com.zeroc.Ice.InputStream istr = request.inputStream;";
            out << nl << "istr.startEncapsulation();";
            for (const auto& param : inParams)
            {
                const TypePtr paramType = param->type();
                if (paramType->isClassType())
                {
                    assert(!param->optional()); // Optional classes are disallowed by the parser.
                    allocatePatcher(out, paramType, package, "icePP_" + param->name());
                    values.push_back(param);
                }
                else
                {
                    const string paramName = "iceP_" + param->name();
                    const string typeS =
                        typeToString(paramType, TypeModeIn, package, param->getMetadata(), true, param->optional());
                    out << nl << typeS << ' ' << paramName << ';';
                }
            }

            //
            // Unmarshal 'in' parameters.
            //
            ParameterList required, optional;
            op->inParameters(required, optional);
            int iter = 0;
            for (const auto& param : required)
            {
                const string paramName =
                    param->type()->isClassType() ? ("icePP_" + param->name()) : "iceP_" + param->name();
                const string patchParams = getPatcher(param->type(), package, paramName + ".value");
                writeMarshalUnmarshalCode(
                    out,
                    package,
                    param->type(),
                    OptionalNone,
                    false,
                    0,
                    paramName,
                    false,
                    iter,
                    "",
                    param->getMetadata(),
                    patchParams);
            }
            for (const auto& param : optional)
            {
                const string paramName =
                    param->type()->isClassType() ? ("icePP_" + param->name()) : "iceP_" + param->name();
                const string patchParams = getPatcher(param->type(), package, paramName + ".value");
                writeMarshalUnmarshalCode(
                    out,
                    package,
                    param->type(),
                    OptionalInParam,
                    true,
                    param->tag(),
                    paramName,
                    false,
                    iter,
                    "",
                    param->getMetadata(),
                    patchParams);
            }
            if (op->sendsClasses())
            {
                out << nl << "istr.readPendingValues();";
            }
            out << nl << "istr.endEncapsulation();";

            for (ParameterList::const_iterator pli = values.begin(); pli != values.end(); ++pli)
            {
                const string typeS =
                    typeToString((*pli)->type(), TypeModeIn, package, (*pli)->getMetadata(), true, (*pli)->optional());
                out << nl << typeS << ' ' << "iceP_" << (*pli)->name() << " = icePP_" << (*pli)->name() << ".value;";
            }
        }
        else
        {
            out << nl << "request.inputStream.skipEmptyEncapsulation();";
        }

        vector<string> inArgs;
        for (const auto& pli : inParams)
        {
            inArgs.push_back("iceP_" + pli->name());
        }

        string retS = getResultType(op, package, false, true);

        if (op->hasMarshaledResult())
        {
            if (amd)
            {
                out << nl << "var result = obj." << opName << "Async" << spar << inArgs << "request.current" << epar
                    << ";";
                out << nl << "return result.thenApply(r -> new com.zeroc.Ice.OutgoingResponse(r.getOutputStream()));";
            }
            else
            {
                out << nl << "var result = obj." << fixKwd(opName) << spar << inArgs << "request.current" << epar
                    << ";";
                out << nl
                    << "return java.util.concurrent.CompletableFuture.completedFuture(new "
                       "com.zeroc.Ice.OutgoingResponse(result.getOutputStream()));";
            }
        }
        else if (amd)
        {
            out << nl << "var result = obj." << opName << "Async" << spar << inArgs << "request.current" << epar << ";";
            if (retS == "void")
            {
                out << nl << "return result.thenApply(r -> request.current.createEmptyOutgoingResponse());";
            }
            else
            {
                out << nl << "return result.thenApply(r -> request.current.createOutgoingResponse(";
                out.inc();
                out << nl << "r,";
                out << nl << "(ostr, value) -> ";
                out << sb;
                writeMarshalServantResults(out, package, op, "value");
                out << eb;
                out << ",";
                out << nl << opFormatTypeToString(op) << "));";
                out.dec();
            }
        }
        else
        {
            out << nl;
            if (ret || !outParams.empty())
            {
                out << retS << " ret = ";
            }
            out << "obj." << fixKwd(opName) << spar << inArgs << "request.current" << epar << ';';

            if (ret || !outParams.empty())
            {
                out << nl << "var ostr = request.current.startReplyStream();";
                out << nl << "ostr.startEncapsulation(request.current.encoding, " << opFormatTypeToString(op) << ");";
                writeMarshalServantResults(out, package, op, "ret");
                out << nl << "ostr.endEncapsulation();";
                out << nl
                    << "return java.util.concurrent.CompletableFuture.completedFuture(new "
                       "com.zeroc.Ice.OutgoingResponse(ostr));";
            }
            else
            {
                out << nl
                    << "return "
                       "java.util.concurrent.CompletableFuture.completedFuture(request.current."
                       "createEmptyOutgoingResponse());";
            }
        }
        out << eb;
    }

    OperationList allOps = p->allOperations();
    if (!allOps.empty())
    {
        out << sp;
        out << nl << "@Override" << nl
            << "default java.util.concurrent.CompletionStage<com.zeroc.Ice.OutgoingResponse> dispatch("
            << "com.zeroc.Ice.IncomingRequest request)";
        out.inc();
        out << nl << "throws com.zeroc.Ice.UserException";
        out.dec();
        out << sb;
        out << nl << "return switch (request.current.operation)";
        out << sb;
        for (const auto& op : allOps)
        {
            out << nl << "case \"" << op->name() << "\" -> " << getUnqualified(op->interface(), package) << "._iceD_"
                << op->name() << "(this, request);";
        }
        for (const auto& opName : {"ice_id", "ice_ids", "ice_isA", "ice_ping"})
        {
            out << nl << "case \"" << opName << "\" -> com.zeroc.Ice.Object._iceD_" << opName << "(this, request);";
        }
        out << nl << "default -> throw new com.zeroc.Ice.OperationNotExistException();";
        out << eb;
        out << ";";
        out << eb;
    }
}

void
Slice::JavaVisitor::writeMarshaling(Output& out, const ClassDefPtr& p)
{
    string name = fixKwd(p->name());
    string package = getPackage(p);
    string scoped = p->scoped();
    ClassDefPtr base = p->base();

    int iter;
    DataMemberList members = p->dataMembers();
    DataMemberList optionalMembers = p->orderedOptionalDataMembers();

    out << sp;
    writeHiddenDocComment(out);
    out << nl << "@Override";
    out << nl << "protected void _iceWriteImpl(com.zeroc.Ice.OutputStream ostr_)";
    out << sb;
    out << nl << "ostr_.startSlice(ice_staticId(), " << p->compactId() << (!base ? ", true" : ", false") << ");";
    iter = 0;
    for (DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
    {
        if (!(*d)->optional())
        {
            writeMarshalDataMember(out, package, *d, iter);
        }
    }
    for (DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
    {
        writeMarshalDataMember(out, package, *d, iter);
    }
    out << nl << "ostr_.endSlice();";
    if (base)
    {
        out << nl << "super._iceWriteImpl(ostr_);";
    }
    out << eb;

    DataMemberList classMembers = p->classDataMembers();
    DataMemberList allClassMembers = p->allClassDataMembers();

    out << sp;
    writeHiddenDocComment(out);
    out << nl << "@Override";
    out << nl << "protected void _iceReadImpl(com.zeroc.Ice.InputStream istr_)";
    out << sb;
    out << nl << "istr_.startSlice();";

    iter = 0;
    for (DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
    {
        if (!(*d)->optional())
        {
            writeUnmarshalDataMember(out, package, *d, iter);
        }
    }
    for (DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
    {
        writeUnmarshalDataMember(out, package, *d, iter);
    }

    out << nl << "istr_.endSlice();";
    if (base)
    {
        out << nl << "super._iceReadImpl(istr_);";
    }
    out << eb;
}

void
Slice::JavaVisitor::writeConstantValue(
    Output& out,
    const TypePtr& type,
    const SyntaxTreeBasePtr& valueType,
    const string& value,
    const string& package)
{
    ConstPtr constant = dynamic_pointer_cast<Const>(valueType);
    if (constant)
    {
        out << getUnqualified(constant, package) << ".value";
    }
    else
    {
        BuiltinPtr bp;
        if ((bp = dynamic_pointer_cast<Builtin>(type)))
        {
            switch (bp->kind())
            {
                case Builtin::KindString:
                {
                    out << "\"" << toStringLiteral(value, "\b\f\n\r\t", "", ShortUCN, 0) << "\"";
                    break;
                }
                case Builtin::KindByte:
                {
                    int i = atoi(value.c_str());
                    if (i > 127)
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
        else if (dynamic_pointer_cast<Enum>(type))
        {
            EnumeratorPtr lte = dynamic_pointer_cast<Enumerator>(valueType);
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
    for (DataMemberList::const_iterator p = members.begin(); p != members.end(); ++p)
    {
        TypePtr t = (*p)->type();
        if ((*p)->defaultValueType())
        {
            if ((*p)->optional())
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
            BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(t);
            if (builtin && builtin->kind() == Builtin::KindString)
            {
                out << nl << "this." << fixKwd((*p)->name()) << " = \"\";";
            }

            EnumPtr en = dynamic_pointer_cast<Enum>(t);
            if (en)
            {
                string firstEnum = fixKwd(en->enumerators().front()->name());
                out << nl << "this." << fixKwd((*p)->name()) << " = " << getUnqualified(en, package) << '.' << firstEnum
                    << ';';
            }

            StructPtr st = dynamic_pointer_cast<Struct>(t);
            if (st)
            {
                string memberType = typeToString(st, TypeModeMember, package, (*p)->getMetadata());
                out << nl << "this." << fixKwd((*p)->name()) << " = new " << memberType << "();";
            }
        }
    }
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
    for (StringList::const_iterator p = l.begin(); p != l.end(); ++p)
    {
        out << nl << " *";
        if (!p->empty())
        {
            out << " " << *p;
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
    if (pos == string::npos)
    {
        out << text;
    }
    else
    {
        string s = IceInternal::trim(text.substr(start, pos - start));
        out << s; // Emit the first line.
        start = pos + 1;
        while ((pos = text.find_first_of(ws, start)) != string::npos)
        {
            string line = IceInternal::trim(text.substr(start, pos - start));
            if (line.empty())
            {
                out << nl << " *";
            }
            else
            {
                out << nl << " * " << line;
            }
            start = pos + 1;
        }
        if (start < text.size())
        {
            string line = IceInternal::trim(text.substr(start));
            if (line.empty())
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
Slice::JavaVisitor::writeDocComment(Output& out, const UnitPtr& unt, const DocCommentPtr& dc)
{
    if (!dc)
    {
        return;
    }

    out << nl << "/**";
    if (!dc->overview().empty())
    {
        out << nl << " * ";
        writeDocCommentLines(out, dc->overview());
    }

    if (!dc->seeAlso().empty())
    {
        out << nl << " *";
        StringList sa = dc->seeAlso();
        for (StringList::iterator p = sa.begin(); p != sa.end(); ++p)
        {
            out << nl << " * @see ";
            writeSeeAlso(out, unt, *p);
        }
    }

    if (!dc->deprecated().empty())
    {
        out << nl << " * @deprecated ";
        writeDocCommentLines(out, dc->deprecated());
    }
    else if (dc->isDeprecated())
    {
        out << nl << " * @deprecated";
    }

    out << nl << " **/";
}

void
Slice::JavaVisitor::writeDocComment(Output& out, const string& text)
{
    if (!text.empty())
    {
        out << nl << "/**";
        out << nl << " * ";
        writeDocCommentLines(out, text);
        out << nl << " **/";
    }
}

void
Slice::JavaVisitor::writeProxyDocComment(
    Output& out,
    const OperationPtr& p,
    const string& package,
    const DocCommentPtr& dc,
    bool async,
    const string& contextParam)
{
    if (!dc)
    {
        return;
    }

    map<string, StringList> paramDocs = dc->parameters();

    out << nl << "/**";
    if (!dc->overview().empty())
    {
        out << nl << " * ";
        writeDocCommentLines(out, dc->overview());
    }

    //
    // Show in-params in order of declaration, but only those with docs.
    //
    const ParameterList paramList = p->inParameters();
    for (ParameterList::const_iterator i = paramList.begin(); i != paramList.end(); ++i)
    {
        const string name = (*i)->name();
        map<string, StringList>::const_iterator j = paramDocs.find(name);
        if (j != paramDocs.end() && !j->second.empty())
        {
            out << nl << " * @param " << fixKwd(j->first) << ' ';
            writeDocCommentLines(out, j->second);
        }
    }
    if (!contextParam.empty())
    {
        out << nl << " * " << contextParam;
    }

    //
    // Handle the return value (if any).
    //
    if (p->returnsMultipleValues())
    {
        const string r = getResultType(p, package, true, false);
        if (async)
        {
            out << nl << " * @return A future that will be completed with an instance of " << r << '.';
        }
        else
        {
            out << nl << " * @return An instance of " << r << '.';
        }
    }
    else if (p->returnType())
    {
        if (!dc->returns().empty())
        {
            out << nl << " * @return ";
            writeDocCommentLines(out, dc->returns());
        }
        else if (async)
        {
            out << nl << " * @return A future that will be completed with the result.";
        }
    }
    else if (!p->outParameters().empty())
    {
        assert(p->outParameters().size() == 1);
        const ParameterPtr param = p->outParameters().front();
        map<string, StringList>::const_iterator j = paramDocs.find(param->name());
        if (j != paramDocs.end() && !j->second.empty())
        {
            out << nl << " * @return ";
            writeDocCommentLines(out, j->second);
        }
        else if (async)
        {
            out << nl << " * @return A future that will be completed with the result.";
        }
    }
    else if (async)
    {
        //
        // No results but an async proxy operation still returns a future.
        //
        out << nl << " * @return A future that will be completed when the invocation completes.";
    }

    //
    // Async proxy methods don't declare user exceptions.
    //
    if (!async)
    {
        map<string, StringList> exDocs = dc->exceptions();
        for (map<string, StringList>::const_iterator i = exDocs.begin(); i != exDocs.end(); ++i)
        {
            out << nl << " * @throws " << fixKwd(i->first) << ' ';
            writeDocCommentLines(out, i->second);
        }
    }

    if (!dc->seeAlso().empty())
    {
        out << nl << " *";
        StringList sa = dc->seeAlso();
        for (StringList::iterator q = sa.begin(); q != sa.end(); ++q)
        {
            out << nl << " * @see ";
            writeSeeAlso(out, p->unit(), *q);
        }
    }

    if (!dc->deprecated().empty())
    {
        out << nl << " * @deprecated ";
        writeDocCommentLines(out, dc->deprecated());
    }
    else if (dc->isDeprecated())
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
    const ParameterList paramList = p->inParameters();
    for (ParameterList::const_iterator i = paramList.begin(); i != paramList.end(); ++i)
    {
        const string name = (*i)->name();
        out << nl << " * @param "
            << "iceP_" << name << " -";
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
Slice::JavaVisitor::writeServantDocComment(
    Output& out,
    const OperationPtr& p,
    const string& package,
    const DocCommentPtr& dc,
    bool async)
{
    if (!dc)
    {
        return;
    }

    map<string, StringList> paramDocs = dc->parameters();
    const ParameterList paramList = p->inParameters();
    const string currentParamName = getEscapedParamName(p, "current");
    const string currentParam = " * @param " + currentParamName + " The Current object for the invocation.";

    out << nl << "/**";
    if (!dc->overview().empty())
    {
        out << nl << " * ";
        writeDocCommentLines(out, dc->overview());
    }

    //
    // Show in-params in order of declaration, but only those with docs.
    //
    for (ParameterList::const_iterator i = paramList.begin(); i != paramList.end(); ++i)
    {
        const string name = (*i)->name();
        map<string, StringList>::const_iterator j = paramDocs.find(name);
        if (j != paramDocs.end() && !j->second.empty())
        {
            out << nl << " * @param " << fixKwd(j->first) << ' ';
            writeDocCommentLines(out, j->second);
        }
    }
    out << nl << currentParam;

    //
    // Handle the return value (if any).
    //
    if (p->returnsMultipleValues())
    {
        const string r = getResultType(p, package, true, false);
        if (async)
        {
            out << nl << " * @return A completion stage that the servant will complete with an instance of " << r
                << '.';
        }
        else
        {
            out << nl << " * @return An instance of " << r << '.';
        }
    }
    else if (p->returnType())
    {
        if (!dc->returns().empty())
        {
            out << nl << " * @return ";
            writeDocCommentLines(out, dc->returns());
        }
        else if (async)
        {
            out << nl << " * @return A completion stage that the servant will complete with the result.";
        }
    }
    else if (!p->outParameters().empty())
    {
        assert(p->outParameters().size() == 1);
        const ParameterPtr param = p->outParameters().front();
        map<string, StringList>::const_iterator j = paramDocs.find(param->name());
        if (j != paramDocs.end() && !j->second.empty())
        {
            out << nl << " * @return ";
            writeDocCommentLines(out, j->second);
        }
        else if (async)
        {
            out << nl << " * @return A completion stage that the servant will complete with the result.";
        }
    }
    else if (async)
    {
        //
        // No results but an async operation still returns a completion stage.
        //
        out << nl << " * @return A completion stage that the servant will complete when the invocation completes.";
    }

    if (p->hasMetadata("java:UserException") || p->hasMetadata("UserException"))
    {
        out << nl << " * @throws com.zeroc.Ice.UserException";
    }
    else
    {
        map<string, StringList> exDocs = dc->exceptions();
        for (map<string, StringList>::const_iterator i = exDocs.begin(); i != exDocs.end(); ++i)
        {
            out << nl << " * @throws " << fixKwd(i->first) << ' ';
            writeDocCommentLines(out, i->second);
        }
    }

    if (!dc->seeAlso().empty())
    {
        out << nl << " *";
        StringList sa = dc->seeAlso();
        for (StringList::iterator q = sa.begin(); q != sa.end(); ++q)
        {
            out << nl << " * @see ";
            writeSeeAlso(out, p->unit(), *q);
        }
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
    if (pos != string::npos)
    {
        rest = s.substr(pos);
        s = s.substr(0, pos);
    }

    pos = 0;
    while ((pos = s.find(".", pos)) != string::npos)
    {
        s.replace(pos, 1, "::");
    }

    //
    // We assume a scoped name should be an absolute name.
    //
    if (s.find(":") != string::npos && s[0] != ':')
    {
        s.insert(0, "::");
    }

    TypeList l = unt->lookupTypeNoBuiltin(s, false, true);
    if (l.empty())
    {
        out << ref;
    }
    else
    {
        ContainedPtr cont = dynamic_pointer_cast<Contained>(l.front());
        assert(cont);
        out << getUnqualified(cont) << rest;
    }
}

Slice::Gen::Gen(const string& /*name*/, string base, const vector<string>& includePaths, string dir)
    : _base(std::move(base)),
      _includePaths(includePaths),
      _dir(std::move(dir))
{
}

Slice::Gen::~Gen() = default;

void
Slice::Gen::generate(const UnitPtr& p)
{
    JavaGenerator::validateMetadata(p);

    PackageVisitor packageVisitor(_dir);
    p->visit(&packageVisitor);

    TypesVisitor typesVisitor(_dir);
    p->visit(&typesVisitor);

    CompactIdVisitor compactIdVisitor(_dir);
    p->visit(&compactIdVisitor);

    HelperVisitor helperVisitor(_dir);
    p->visit(&helperVisitor);

    ProxyVisitor proxyVisitor(_dir);
    p->visit(&proxyVisitor);
}

Slice::Gen::PackageVisitor::PackageVisitor(const string& dir) : JavaVisitor(dir) {}

bool
Slice::Gen::PackageVisitor::visitModuleStart(const ModulePtr& p)
{
    string prefix = getPackagePrefix(p);
    if (!prefix.empty())
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

Slice::Gen::TypesVisitor::TypesVisitor(const string& dir) : JavaVisitor(dir) {}

bool
Slice::Gen::TypesVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string name = p->name();
    ClassDefPtr baseClass = p->base();
    string package = getPackage(p);
    string absolute = getUnqualified(p);
    DataMemberList members = p->dataMembers();
    DataMemberList allDataMembers = p->allDataMembers();

    open(absolute, p->file());

    Output& out = output();

    DocCommentPtr dc = p->parseDocComment(javaLinkFormatter);

    //
    // Slice interfaces map to Java interfaces.
    //
    out << sp;
    writeDocComment(out, p->unit(), dc);
    if (dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }

    out << nl << "public class " << fixKwd(name);
    out.useCurrentPosAsIndent();

    if (baseClass)
    {
        out << " extends " << getUnqualified(baseClass, package);
    }
    else
    {
        out << " extends com.zeroc.Ice.Value";
    }

    out.restoreIndent();

    out << sb;

    if (!allDataMembers.empty())
    {
        bool hasOptionalMembers = false;
        bool hasRequiredMembers = false;

        for (DataMemberList::const_iterator d = allDataMembers.begin(); d != allDataMembers.end(); ++d)
        {
            if ((*d)->optional())
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
        if (baseClass)
        {
            out << nl << "super();";
        }
        writeDataMemberInitializers(out, members, package);
        out << eb;

        //
        // Generate constructor if the parameter list is not too large.
        //
        if (isValidMethodParameterList(allDataMembers))
        {
            DataMemberList baseDataMembers;
            if (baseClass)
            {
                baseDataMembers = baseClass->allDataMembers();
            }

            if (hasRequiredMembers && hasOptionalMembers)
            {
                //
                // Generate a constructor accepting parameters for just the required members.
                //
                out << sp << nl << "public " << fixKwd(name) << spar;
                vector<string> parameters;
                for (DataMemberList::const_iterator d = allDataMembers.begin(); d != allDataMembers.end(); ++d)
                {
                    if (!(*d)->optional())
                    {
                        string memberName = fixKwd((*d)->name());
                        string memberType =
                            typeToString((*d)->type(), TypeModeMember, package, (*d)->getMetadata(), true, false);
                        parameters.push_back(memberType + " " + memberName);
                    }
                }
                out << parameters << epar;
                out << sb;
                if (!baseDataMembers.empty())
                {
                    bool hasBaseRequired = false;
                    for (DataMemberList::const_iterator d = baseDataMembers.begin(); d != baseDataMembers.end(); ++d)
                    {
                        if (!(*d)->optional())
                        {
                            hasBaseRequired = true;
                            break;
                        }
                    }
                    if (hasBaseRequired)
                    {
                        out << nl << "super" << spar;
                        vector<string> baseParamNames;
                        for (DataMemberList::const_iterator d = baseDataMembers.begin(); d != baseDataMembers.end();
                             ++d)
                        {
                            if (!(*d)->optional())
                            {
                                baseParamNames.push_back(fixKwd((*d)->name()));
                            }
                        }
                        out << baseParamNames << epar << ';';
                    }
                }

                for (DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
                {
                    if (!(*d)->optional())
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
            vector<string> parameters;
            for (DataMemberList::const_iterator d = allDataMembers.begin(); d != allDataMembers.end(); ++d)
            {
                string memberName = fixKwd((*d)->name());
                string memberType =
                    typeToString((*d)->type(), TypeModeMember, package, (*d)->getMetadata(), true, false);
                parameters.push_back(memberType + " " + memberName);
            }
            out << parameters << epar;
            out << sb;
            if (baseClass && allDataMembers.size() != members.size())
            {
                out << nl << "super" << spar;
                vector<string> baseParamNames;
                for (DataMemberList::const_iterator d = baseDataMembers.begin(); d != baseDataMembers.end(); ++d)
                {
                    baseParamNames.push_back(fixKwd((*d)->name()));
                }
                out << baseParamNames << epar << ';';
            }
            for (DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
            {
                string paramName = fixKwd((*d)->name());
                if ((*d)->optional())
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
    ClassDefPtr baseClass = p->base();

    string name = fixKwd(p->name());

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
    out << nl << getSerialVersionUID(p);

    writeMarshaling(out, p);

    out << eb;
    close();
}

bool
Slice::Gen::TypesVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    string name = p->name();
    InterfaceList bases = p->bases();

    string package = getPackage(p);
    string absolute = getUnqualified(p);
    open(absolute, p->file());

    Output& out = output();
    DocCommentPtr dc = p->parseDocComment(javaLinkFormatter);

    bool hasOptionals = false;
    for (const auto& op : p->allOperations())
    {
        if (op->returnIsOptional())
        {
            hasOptionals = true;
            break;
        }

        for (const auto& q : op->parameters())
        {
            if (q->optional())
            {
                hasOptionals = true;
                break;
            }
        }
        if (hasOptionals)
        {
            break;
        }
    }

    out << sp;
    writeDocComment(out, p->unit(), dc);

    out << nl << "public interface " << fixKwd(name) << " extends ";
    InterfaceList::const_iterator q = bases.begin();
    out.useCurrentPosAsIndent();
    if (bases.empty())
    {
        out << "com.zeroc.Ice.Object";
    }
    else if (q != bases.end())
    {
        out << getUnqualified(*q++, package);
    }

    for (; q != bases.end(); ++q)
    {
        out << ',' << nl << getUnqualified(*q, package);
    }
    out.restoreIndent();
    out << sb;
    return true;
}

void
Slice::Gen::TypesVisitor::visitInterfaceDefEnd(const InterfaceDefPtr& p)
{
    Output& out = output();
    writeDispatch(out, p);
    out << eb;
    close();
}

void
Slice::Gen::TypesVisitor::visitOperation(const OperationPtr& p)
{
    //
    // Generate the operation signature for a servant.
    //

    InterfaceDefPtr interface = dynamic_pointer_cast<InterfaceDef>(p->container());
    assert(interface);

    const string package = getPackage(interface);

    Output& out = output();

    DocCommentPtr dc = p->parseDocComment(javaLinkFormatter);

    //
    // Generate the "Result" type needed by operations that return multiple values.
    //
    if (p->returnsMultipleValues())
    {
        writeResultType(out, p, package, dc);
    }

    //
    // The "MarshaledResult" type is generated in the servant interface.
    //
    if (p->hasMarshaledResult())
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
    DataMemberList baseDataMembers;
    if (base)
    {
        baseDataMembers = base->allDataMembers();
    }

    open(absolute, p->file());

    Output& out = output();

    out << sp;

    DocCommentPtr dc = p->parseDocComment(javaLinkFormatter);
    writeDocComment(out, p->unit(), dc);
    if (dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }

    out << nl << "public class " << name << " extends ";

    if (!base)
    {
        out << "com.zeroc.Ice.UserException";
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
    writeDataMemberInitializers(out, members, package);
    out << eb;

    if (!allDataMembers.empty())
    {
        // Only generate additional constructors if the parameter list is not too large.
        if (isValidMethodParameterList(allDataMembers))
        {
            bool hasOptionalMembers = false;
            bool hasRequiredMembers = false;
            for (const auto& member : allDataMembers)
            {
                if (member->optional())
                {
                    hasOptionalMembers = true;
                }
                else
                {
                    hasRequiredMembers = true;
                }
            }
            if (hasRequiredMembers && hasOptionalMembers)
            {
                bool hasBaseRequired = false;
                for (const auto& member : baseDataMembers)
                {
                    if (!member->optional())
                    {
                        hasBaseRequired = true;
                        break;
                    }
                }

                DataMemberList optionalMembers = p->orderedOptionalDataMembers();

                // Generate a constructor accepting parameters for just the required members.
                out << sp << nl << "public " << name << spar;
                vector<string> parameters;
                for (const auto& member : allDataMembers)
                {
                    if (!member->optional())
                    {
                        string memberName = fixKwd(member->name());
                        string memberType =
                            typeToString(member->type(), TypeModeMember, package, member->getMetadata(), true, false);
                        parameters.push_back(memberType + " " + memberName);
                    }
                }
                out << parameters << epar;
                out << sb;
                if (!baseDataMembers.empty())
                {
                    if (hasBaseRequired)
                    {
                        out << nl << "super" << spar;
                        vector<string> baseParamNames;
                        for (const auto& member : baseDataMembers)
                        {
                            if (!member->optional())
                            {
                                baseParamNames.push_back(fixKwd(member->name()));
                            }
                        }
                        out << baseParamNames << epar << ';';
                    }
                }

                for (const auto& member : members)
                {
                    if (!member->optional())
                    {
                        string paramName = fixKwd(member->name());
                        out << nl << "this." << paramName << " = " << paramName << ';';
                    }
                }
                writeDataMemberInitializers(out, optionalMembers, package);
                out << eb;
            }

            //
            // Primary constructor which takes all data members.
            //
            out << sp << nl << "public " << name << spar;
            vector<string> parameters;
            for (const auto& member : allDataMembers)
            {
                string memberName = fixKwd(member->name());
                string memberType =
                    typeToString(member->type(), TypeModeMember, package, member->getMetadata(), true, false);
                parameters.push_back(memberType + " " + memberName);
            }
            out << parameters << epar;
            out << sb;
            // Set any base members by calling the super-constructor (if necessary).
            if (base && allDataMembers.size() != members.size())
            {
                out << nl << "super" << spar;
                vector<string> baseParamNames;
                DataMemberList baseDataMemberList = base->allDataMembers();
                for (const auto& member : baseDataMemberList)
                {
                    baseParamNames.push_back(fixKwd(member->name()));
                }

                out << baseParamNames << epar << ';';
            }
            // Set any non-base members in the constructor body.
            for (const auto& member : members)
            {
                string paramName = fixKwd(member->name());
                if (member->optional())
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

    DataMemberList members = p->dataMembers();
    DataMemberList optionalMembers = p->orderedOptionalDataMembers();
    int iter;

    out << sp;
    writeHiddenDocComment(out);
    out << nl << "@Override";
    out << nl << "protected void _writeImpl(com.zeroc.Ice.OutputStream ostr_)";
    out << sb;
    out << nl << "ostr_.startSlice(\"" << scoped << "\", -1, " << (!base ? "true" : "false") << ");";
    iter = 0;
    for (DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
    {
        if (!(*d)->optional())
        {
            writeMarshalDataMember(out, package, *d, iter);
        }
    }
    for (DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
    {
        writeMarshalDataMember(out, package, *d, iter);
    }
    out << nl << "ostr_.endSlice();";
    if (base)
    {
        out << nl << "super._writeImpl(ostr_);";
    }
    out << eb;

    DataMemberList classMembers = p->classDataMembers();
    DataMemberList allClassMembers = p->allClassDataMembers();

    out << sp;
    writeHiddenDocComment(out);
    out << nl << "@Override";
    out << nl << "protected void _readImpl(com.zeroc.Ice.InputStream istr_)";
    out << sb;
    out << nl << "istr_.startSlice();";
    iter = 0;
    for (DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
    {
        if (!(*d)->optional())
        {
            writeUnmarshalDataMember(out, package, *d, iter);
        }
    }
    for (DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
    {
        writeUnmarshalDataMember(out, package, *d, iter);
    }
    out << nl << "istr_.endSlice();";
    if (base)
    {
        out << nl << "super._readImpl(istr_);";
    }
    out << eb;

    if (p->usesClasses() && !(base && base->usesClasses()))
    {
        out << sp;
        writeHiddenDocComment(out);
        out << nl << "@Override";
        out << nl << "public boolean _usesClasses()";
        out << sb;
        out << nl << "return true;";
        out << eb;
    }

    out << sp;
    writeHiddenDocComment(out);
    out << nl << getSerialVersionUID(p);

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

    DocCommentPtr dc = p->parseDocComment(javaLinkFormatter);
    writeDocComment(out, p->unit(), dc);
    if (dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }

    out << nl << "public class " << name << " implements ";
    out.useCurrentPosAsIndent();
    out << "java.lang.Cloneable";
    out << "," << nl << "java.io.Serializable";
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
    // Generate constructor if the parameter list is not too large.
    //
    if (isValidMethodParameterList(members))
    {
        vector<string> parameters;
        vector<string> paramNames;
        for (DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
        {
            string memberName = fixKwd((*d)->name());
            string memberType = typeToString((*d)->type(), TypeModeMember, package, (*d)->getMetadata(), true, false);
            parameters.push_back(memberType + " " + memberName);
            paramNames.push_back(memberName);
        }

        out << sp << nl << "public " << name << spar << parameters << epar;
        out << sb;
        for (vector<string>::const_iterator i = paramNames.begin(); i != paramNames.end(); ++i)
        {
            out << nl << "this." << *i << " = " << *i << ';';
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
    for (DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
    {
        string memberName = fixKwd((*d)->name());
        BuiltinPtr b = dynamic_pointer_cast<Builtin>((*d)->type());
        if (b)
        {
            switch (b->kind())
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
            SequencePtr seq = dynamic_pointer_cast<Sequence>((*d)->type());
            if (seq)
            {
                if (hasTypeMetadata(seq, (*d)->getMetadata()))
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

    out << sp << nl << "public int hashCode()";
    out << sb;
    out << nl << "int h_ = 5381;";
    out << nl << "h_ = com.zeroc.Ice.HashUtil.hashAdd(h_, \"" << p->scoped() << "\");";
    iter = 0;
    for (const auto& member : members)
    {
        string memberName = fixKwd(member->name());
        out << nl << "h_ = com.zeroc.Ice.HashUtil.hashAdd(h_, " << memberName << ");";
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

    out << sp << nl << "public void ice_writeMembers(com.zeroc.Ice.OutputStream ostr)";
    out << sb;
    iter = 0;
    for (DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
    {
        writeMarshalDataMember(out, package, *d, iter, true);
    }
    out << eb;

    DataMemberList classMembers = p->classDataMembers();

    out << sp << nl << "public void ice_readMembers(com.zeroc.Ice.InputStream istr)";
    out << sb;
    iter = 0;
    for (DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
    {
        writeUnmarshalDataMember(out, package, *d, iter, true);
    }
    out << eb;

    out << sp << nl << "static public void ice_write(com.zeroc.Ice.OutputStream ostr, " << name << " v)";
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

    out << sp << nl << "static public " << name << " ice_read(com.zeroc.Ice.InputStream istr)";
    out << sb;
    out << nl << name << " v = new " << name << "();";
    out << nl << "v.ice_readMembers(istr);";
    out << nl << "return v;";
    out << eb;

    string optName = "java.util.Optional<" + name + ">";
    out << sp;
    out << nl << "static public void ice_write(com.zeroc.Ice.OutputStream ostr, int tag, " << optName << " v)";
    out << sb;
    out << nl << "if(v != null && v.isPresent())";
    out << sb;
    out << nl << "ice_write(ostr, tag, v.get());";
    out << eb;
    out << eb;

    out << sp;
    out << nl << "static public void ice_write(com.zeroc.Ice.OutputStream ostr, int tag, " << name << " v)";
    out << sb;
    out << nl << "if(ostr.writeOptional(tag, " << getOptionalFormat(p) << "))";
    out << sb;
    if (p->isVariableLength())
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

    out << sp;
    out << nl << "static public " << optName << " ice_read(com.zeroc.Ice.InputStream istr, int tag)";
    out << sb;
    out << nl << "if(istr.readOptional(tag, " << getOptionalFormat(p) << "))";
    out << sb;
    if (p->isVariableLength())
    {
        out << nl << "istr.skip(4);";
    }
    else
    {
        out << nl << "istr.skipSize();";
    }
    out << nl << "return java.util.Optional.of(" << typeS << ".ice_read(istr));";
    out << eb;
    out << nl << "else";
    out << sb;
    out << nl << "return java.util.Optional.empty();";
    out << eb;
    out << eb;

    out << sp << nl << "private static final " << name << " _nullMarshalValue = new " << name << "();";

    out << sp;
    writeHiddenDocComment(out);
    out << nl << getSerialVersionUID(p);

    out << eb;
    close();
}

void
Slice::Gen::TypesVisitor::visitDataMember(const DataMemberPtr& p)
{
    const ContainerPtr container = p->container();
    const ClassDefPtr cls = dynamic_pointer_cast<ClassDef>(container);
    const StructPtr st = dynamic_pointer_cast<Struct>(container);
    const ExceptionPtr ex = dynamic_pointer_cast<Exception>(container);
    const ContainedPtr contained = dynamic_pointer_cast<Contained>(container);

    const string name = fixKwd(p->name());
    const bool getSet = p->hasMetadata("java:getset") || contained->hasMetadata("java:getset");
    const bool optional = p->optional();
    const TypePtr type = p->type();
    const BuiltinPtr b = dynamic_pointer_cast<Builtin>(type);
    const bool classType = type->isClassType();

    const MetadataList metadata = p->getMetadata();
    const string s = typeToString(type, TypeModeMember, getPackage(contained), metadata, true, false);

    Output& out = output();

    out << sp;

    DocCommentPtr dc = p->parseDocComment(javaLinkFormatter);
    writeDocComment(out, p->unit(), dc);
    if (dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }

    // Access visibility for class data members can be controlled by metadata.
    // If none is specified, the default is public.
    if (cls && (p->hasMetadata("protected") || contained->hasMetadata("protected")))
    {
        out << nl << "protected " << s << ' ' << name << ';';
    }
    else if (optional)
    {
        out << nl << "private " << s << ' ' << name << ';';
    }
    else
    {
        out << nl << "public " << s << ' ' << name << ';';
    }

    if (optional)
    {
        out << nl << "private boolean _" << p->name() << ';';
    }

    //
    // Getter/Setter.
    //
    if (getSet || optional)
    {
        string capName = p->name();
        capName[0] = static_cast<char>(toupper(static_cast<unsigned char>(capName[0])));

        //
        // Getter.
        //
        out << sp;
        writeDocComment(out, p->unit(), dc);
        if (dc && dc->isDeprecated())
        {
            out << nl << "@Deprecated";
        }
        out << nl << "public " << s << " get" << capName << "()";
        out << sb;
        if (optional)
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
        if (dc && dc->isDeprecated())
        {
            out << nl << "@Deprecated";
        }
        out << nl << "public void set" << capName << '(' << s << " " << name << ')';
        out << sb;
        if (optional)
        {
            out << nl << "_" << p->name() << " = true;";
        }
        out << nl << "this." << name << " = " << name << ';';
        out << eb;

        //
        // Generate hasFoo and clearFoo for optional member.
        //
        if (optional)
        {
            out << sp;
            writeDocComment(out, p->unit(), dc);
            if (dc && dc->isDeprecated())
            {
                out << nl << "@Deprecated";
            }
            out << nl << "public boolean has" << capName << "()";
            out << sb;
            out << nl << "return _" << p->name() << ';';
            out << eb;

            out << sp;
            writeDocComment(out, p->unit(), dc);
            if (dc && dc->isDeprecated())
            {
                out << nl << "@Deprecated";
            }
            out << nl << "public void clear" << capName << "()";
            out << sb;
            out << nl << "_" << p->name() << " = false;";
            out << eb;

            const string optType = typeToString(type, TypeModeMember, getPackage(contained), metadata, true, true);

            out << sp;
            writeDocComment(out, p->unit(), dc);
            if (dc && dc->isDeprecated())
            {
                out << nl << "@Deprecated";
            }
            out << nl << "public void optional" << capName << '(' << optType << " v)";
            out << sb;
            out << nl << "if(v == null || !v.isPresent())";
            out << sb;
            out << nl << "_" << p->name() << " = false;";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << "_" << p->name() << " = true;";
            if (b && b->kind() == Builtin::KindInt)
            {
                out << nl << name << " = v.getAsInt();";
            }
            else if (b && b->kind() == Builtin::KindLong)
            {
                out << nl << name << " = v.getAsLong();";
            }
            else if (b && b->kind() == Builtin::KindDouble)
            {
                out << nl << name << " = v.getAsDouble();";
            }
            else
            {
                out << nl << name << " = v.get();";
            }
            out << eb;
            out << eb;

            out << sp;
            writeDocComment(out, p->unit(), dc);
            if (dc && dc->isDeprecated())
            {
                out << nl << "@Deprecated";
            }
            out << nl << "public " << optType << " optional" << capName << "()";
            out << sb;
            out << nl << "if(_" << p->name() << ')';
            out << sb;
            if (classType)
            {
                out << nl << "return java.util.Optional.ofNullable(" << name << ");";
            }
            else if (b && b->kind() == Builtin::KindInt)
            {
                out << nl << "return java.util.OptionalInt.of(" << name << ");";
            }
            else if (b && b->kind() == Builtin::KindLong)
            {
                out << nl << "return java.util.OptionalLong.of(" << name << ");";
            }
            else if (b && b->kind() == Builtin::KindDouble)
            {
                out << nl << "return java.util.OptionalDouble.of(" << name << ");";
            }
            else
            {
                out << nl << "return java.util.Optional.of(" << name << ");";
            }
            out << eb;
            out << nl << "else";
            out << sb;
            if (b && b->kind() == Builtin::KindInt)
            {
                out << nl << "return java.util.OptionalInt.empty();";
            }
            else if (b && b->kind() == Builtin::KindLong)
            {
                out << nl << "return java.util.OptionalLong.empty();";
            }
            else if (b && b->kind() == Builtin::KindDouble)
            {
                out << nl << "return java.util.OptionalDouble.empty();";
            }
            else
            {
                out << nl << "return java.util.Optional.empty();";
            }
            out << eb;
            out << eb;
        }

        //
        // Check for bool type.
        //
        if (b && b->kind() == Builtin::KindBool)
        {
            out << sp;
            if (dc && dc->isDeprecated())
            {
                out << nl << "@Deprecated";
            }
            out << nl << "public boolean is" << capName << "()";
            out << sb;
            if (optional)
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
        SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
        if (seq)
        {
            if (!hasTypeMetadata(seq, metadata))
            {
                string elem =
                    typeToString(seq->type(), TypeModeMember, getPackage(contained), MetadataList(), true, false);

                //
                // Indexed getter.
                //
                out << sp;
                if (dc && dc->isDeprecated())
                {
                    out << nl << "@Deprecated";
                }
                out << nl << "public " << elem << " get" << capName << "(int index)";
                out << sb;
                if (optional)
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
                if (dc && dc->isDeprecated())
                {
                    out << nl << "@Deprecated";
                }
                out << nl << "public void set" << capName << "(int index, " << elem << " val)";
                out << sb;
                if (optional)
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

    DocCommentPtr dc = p->parseDocComment(javaLinkFormatter);
    writeDocComment(out, p->unit(), dc);
    if (dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }

    out << nl << "public enum " << name;
    out << sb;

    for (EnumeratorList::const_iterator en = enumerators.begin(); en != enumerators.end(); ++en)
    {
        if (en != enumerators.begin())
        {
            out << ',';
        }
        DocCommentPtr edc = (*en)->parseDocComment(javaLinkFormatter);
        writeDocComment(out, p->unit(), edc);
        if (edc && edc->isDeprecated())
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
    for (EnumeratorList::const_iterator en = enumerators.begin(); en != enumerators.end(); ++en)
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

    out << sp << nl << "public void ice_write(com.zeroc.Ice.OutputStream ostr)";
    out << sb;
    out << nl << "ostr.writeEnum(_value, " << p->maxValue() << ");";
    out << eb;

    out << sp << nl << "public static void ice_write(com.zeroc.Ice.OutputStream ostr, " << name << " v)";
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

    out << sp << nl << "public static " << name << " ice_read(com.zeroc.Ice.InputStream istr)";
    out << sb;
    out << nl << "int v = istr.readEnum(" << p->maxValue() << ");";
    out << nl << "return validate(v);";
    out << eb;

    string optName = "java.util.Optional<" + name + ">";
    out << sp;
    out << nl << "public static void ice_write(com.zeroc.Ice.OutputStream ostr, int tag, " << optName << " v)";
    out << sb;
    out << nl << "if(v != null && v.isPresent())";
    out << sb;
    out << nl << "ice_write(ostr, tag, v.get());";
    out << eb;
    out << eb;

    out << sp;
    out << nl << "public static void ice_write(com.zeroc.Ice.OutputStream ostr, int tag, " << name << " v)";
    out << sb;
    out << nl << "if(ostr.writeOptional(tag, " << getOptionalFormat(p) << "))";
    out << sb;
    out << nl << "ice_write(ostr, v);";
    out << eb;
    out << eb;

    out << sp;
    out << nl << "public static " << optName << " ice_read(com.zeroc.Ice.InputStream istr, int tag)";
    out << sb;
    out << nl << "if(istr.readOptional(tag, " << getOptionalFormat(p) << "))";
    out << sb;
    out << nl << "return java.util.Optional.of(ice_read(istr));";
    out << eb;
    out << nl << "else";
    out << sb;
    out << nl << "return java.util.Optional.empty();";
    out << eb;
    out << eb;

    out << sp << nl << "private static " << name << " validate(int v)";
    out << sb;
    out << nl << "final " << name << " e = valueOf(v);";
    out << nl << "if(e == null)";
    out << sb;
    out << nl << "throw new com.zeroc.Ice.MarshalException(\"enumerator value \" + v + \" is out of range\");";
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

    DocCommentPtr dc = p->parseDocComment(javaLinkFormatter);
    writeDocComment(out, p->unit(), dc);
    if (dc && dc->isDeprecated())
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

Slice::Gen::CompactIdVisitor::CompactIdVisitor(const string& dir) : JavaVisitor(dir) {}

bool
Slice::Gen::CompactIdVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    string prefix = getPackagePrefix(p);
    if (!prefix.empty())
    {
        prefix = prefix + ".";
    }
    if (p->compactId() >= 0)
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

Slice::Gen::HelperVisitor::HelperVisitor(const string& dir) : JavaVisitor(dir) {}

void
Slice::Gen::HelperVisitor::visitSequence(const SequencePtr& p)
{
    bool mappedToCustomType = p->hasMetadata("java:type");
    if (mapsToJavaBuiltinType(p->type()) && !mappedToCustomType)
    {
        return; // No helpers for sequences of primitive types (that aren't re-mapped with 'java:type').
    }

    string name = p->name();
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
    bool customType = getSequenceTypes(p, "", MetadataList(), instanceType, formalType);

    if (!customType)
    {
        //
        // Determine sequence depth.
        //
        TypePtr origContent = p->type();
        SequencePtr s = dynamic_pointer_cast<Sequence>(origContent);
        while (s)
        {
            //
            // Stop if the inner sequence type has a custom or serializable type.
            //
            if (hasTypeMetadata(s))
            {
                break;
            }
            origContent = s->type();
            s = dynamic_pointer_cast<Sequence>(origContent);
        }

        string origContentS = typeToString(origContent, TypeModeIn, package);
        suppressUnchecked = origContentS.find('<') != string::npos;
    }

    open(helper, p->file());
    Output& out = output();

    int iter;

    out << sp;
    writeDocComment(out, "Helper class for marshaling/unmarshaling " + name + ".");
    out << nl << "public final class " << name << "Helper";
    out << sb;

    out << nl << "public static void write(com.zeroc.Ice.OutputStream ostr, " << typeS << " v)";
    out << sb;
    iter = 0;
    writeSequenceMarshalUnmarshalCode(out, package, p, "v", true, iter, false);
    out << eb;

    out << sp;
    if (suppressUnchecked)
    {
        out << nl << "@SuppressWarnings(\"unchecked\")";
    }
    out << nl << "public static " << typeS << " read(com.zeroc.Ice.InputStream istr)";
    out << sb;
    out << nl << "final " << typeS << " v;";
    iter = 0;
    writeSequenceMarshalUnmarshalCode(out, package, p, "v", false, iter, false);
    out << nl << "return v;";
    out << eb;

    string optTypeS = "java.util.Optional<" + typeS + ">";
    out << sp;
    out << nl << "public static void write(com.zeroc.Ice.OutputStream ostr, int tag, " << optTypeS << " v)";
    out << sb;
    out << nl << "if(v != null && v.isPresent())";
    out << sb;
    out << nl << "write(ostr, tag, v.get());";
    out << eb;
    out << eb;

    out << sp;
    out << nl << "public static void write(com.zeroc.Ice.OutputStream ostr, int tag, " << typeS << " v)";
    out << sb;
    out << nl << "if(ostr.writeOptional(tag, " << getOptionalFormat(p) << "))";
    out << sb;
    if (p->type()->isVariableLength())
    {
        out << nl << "int pos = ostr.startSize();";
        writeSequenceMarshalUnmarshalCode(out, package, p, "v", true, iter, true);
        out << nl << "ostr.endSize(pos);";
    }
    else
    {
        // The sequence is an instance of java.util.List<E>, where E is a fixed-size type.
        // If the element type is bool or byte, we do NOT write an extra size.
        const size_t sz = p->type()->minWireSize();
        if (sz > 1)
        {
            out << nl << "final int optSize = v == null ? 0 : ";
            out << (mappedToCustomType ? "v.size();" : "v.length;");
            out << nl << "ostr.writeSize(optSize > 254 ? optSize * " << sz << " + 5 : optSize * " << sz << " + 1);";
        }
        writeSequenceMarshalUnmarshalCode(out, package, p, "v", true, iter, true);
    }
    out << eb;
    out << eb;

    out << sp;
    out << nl << "public static " << optTypeS << " read(com.zeroc.Ice.InputStream istr, int tag)";
    out << sb;
    out << nl << "if(istr.readOptional(tag, " << getOptionalFormat(p) << "))";
    out << sb;
    if (p->type()->isVariableLength())
    {
        out << nl << "istr.skip(4);";
    }
    else if (p->type()->minWireSize() > 1)
    {
        out << nl << "istr.skipSize();";
    }
    out << nl << typeS << " v;";
    writeSequenceMarshalUnmarshalCode(out, package, p, "v", false, iter, true);
    out << nl << "return java.util.Optional.of(v);";
    out << eb;
    out << nl << "else";
    out << sb;
    out << nl << "return java.util.Optional.empty();";
    out << eb;
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
    string formalType = typeToString(p, TypeModeIn, package, MetadataList(), true);

    open(helper, p->file());
    Output& out = output();

    int iter;

    out << sp;
    writeDocComment(out, "Helper class for marshaling/unmarshaling " + name + ".");
    out << nl << "public final class " << name << "Helper";
    out << sb;

    out << nl << "public static void write(com.zeroc.Ice.OutputStream ostr, " << formalType << " v)";
    out << sb;
    iter = 0;
    writeDictionaryMarshalUnmarshalCode(out, package, p, "v", true, iter, false);
    out << eb;

    out << sp << nl << "public static " << formalType << " read(com.zeroc.Ice.InputStream istr)";
    out << sb;
    out << nl << formalType << " v;";
    iter = 0;
    writeDictionaryMarshalUnmarshalCode(out, package, p, "v", false, iter, false);
    out << nl << "return v;";
    out << eb;

    string optTypeS = "java.util.Optional<" + formalType + ">";
    out << sp;
    out << nl << "public static void write(com.zeroc.Ice.OutputStream ostr, int tag, " << optTypeS << " v)";
    out << sb;
    out << nl << "if(v != null && v.isPresent())";
    out << sb;
    out << nl << "write(ostr, tag, v.get());";
    out << eb;
    out << eb;

    out << sp;
    out << nl << "public static void write(com.zeroc.Ice.OutputStream ostr, int tag, " << formalType << " v)";
    out << sb;
    out << nl << "if(ostr.writeOptional(tag, " << getOptionalFormat(p) << "))";
    out << sb;
    TypePtr keyType = p->keyType();
    TypePtr valueType = p->valueType();
    if (keyType->isVariableLength() || valueType->isVariableLength())
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

    out << sp;
    out << nl << "public static " << optTypeS << " read(com.zeroc.Ice.InputStream istr, int tag)";
    out << sb;
    out << nl << "if(istr.readOptional(tag, " << getOptionalFormat(p) << "))";
    out << sb;
    if (keyType->isVariableLength() || valueType->isVariableLength())
    {
        out << nl << "istr.skip(4);";
    }
    else
    {
        out << nl << "istr.skipSize();";
    }
    out << nl << formalType << " v;";
    writeDictionaryMarshalUnmarshalCode(out, package, p, "v", false, iter, true);
    out << nl << "return java.util.Optional.of(v);";
    out << eb;
    out << nl << "else";
    out << sb;
    out << nl << "return java.util.Optional.empty();";
    out << eb;
    out << eb;

    out << eb;
    close();
}

Slice::Gen::ProxyVisitor::ProxyVisitor(const string& dir) : JavaVisitor(dir) {}

bool
Slice::Gen::ProxyVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    string name = p->name();
    InterfaceList bases = p->bases();
    string package = getPackage(p);
    string absolute = getUnqualified(p, "", "", "Prx");

    open(absolute, p->file());

    Output& out = output();
    DocCommentPtr dc = p->parseDocComment(javaLinkFormatter);

    //
    // Generate a Java interface as the user-visible type
    //
    out << sp;
    writeDocComment(out, p->unit(), dc);
    if (dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }
    out << nl << "public interface " << name << "Prx extends ";
    out.useCurrentPosAsIndent();
    if (bases.empty())
    {
        out << "com.zeroc.Ice.ObjectPrx";
    }
    else
    {
        for (InterfaceList::const_iterator q = bases.begin(); q != bases.end(); ++q)
        {
            if (q != bases.begin())
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
Slice::Gen::ProxyVisitor::visitInterfaceDefEnd(const InterfaceDefPtr& p)
{
    Output& out = output();

    DocCommentPtr dc = p->parseDocComment(javaLinkFormatter);

    const string package = getPackage(p);
    const string contextParam = "java.util.Map<String, String> context";
    const string prxName = p->name() + "Prx";
    const string prxIName = "_" + prxName + "I";

    out << sp;
    writeDocComment(
        out,
        "Creates a new proxy that implements {@link " + prxName +
            "}.\n"
            "@param communicator The communicator of the new proxy.\n"
            "@param proxyString The string representation of the proxy.\n"
            "@return The new proxy.");
    out << nl << "public static " << prxName
        << " createProxy(com.zeroc.Ice.Communicator communicator, String proxyString)";
    out << sb;
    out << nl << "return new " << prxIName << "(com.zeroc.Ice.ObjectPrx.createProxy(communicator, proxyString));";
    out << eb;

    out << sp;
    writeDocComment(
        out,
        "Contacts the remote server to verify that the object implements this type.\n"
        "Raises a local exception if a communication error occurs.\n"
        "@param obj The untyped proxy.\n"
        "@return A proxy for this type, or null if the object does not support this type.");
    out << nl << "static " << prxName << " checkedCast(com.zeroc.Ice.ObjectPrx obj)";
    out << sb;
    out << nl << "return checkedCast(obj, noExplicitContext);";
    out << eb;

    out << sp;
    writeDocComment(
        out,
        "Contacts the remote server to verify that the object implements this type.\n"
        "Raises a local exception if a communication error occurs.\n"
        "@param obj The untyped proxy.\n"
        "@param context The Context map to send with the invocation.\n"
        "@return A proxy for this type, or null if the object does not support this type.");
    out << nl << "static " << prxName << " checkedCast(com.zeroc.Ice.ObjectPrx obj, " << contextParam << ')';
    out << sb;
    out << nl << "return (obj != null && obj.ice_isA(ice_staticId(), context)) ? new " << prxIName << "(obj) : null;";
    out << eb;

    out << sp;
    writeDocComment(
        out,
        "Contacts the remote server to verify that a facet of the object implements this type.\n"
        "Raises a local exception if a communication error occurs.\n"
        "@param obj The untyped proxy.\n"
        "@param facet The name of the desired facet.\n"
        "@return A proxy for this type, or null if the object does not support this type.");
    out << nl << "static " << prxName << " checkedCast(com.zeroc.Ice.ObjectPrx obj, String facet)";
    out << sb;
    out << nl << "return checkedCast(obj, facet, noExplicitContext);";
    out << eb;

    out << sp;
    writeDocComment(
        out,
        "Contacts the remote server to verify that a facet of the object implements this type.\n"
        "Raises a local exception if a communication error occurs.\n"
        "@param obj The untyped proxy.\n"
        "@param facet The name of the desired facet.\n"
        "@param context The Context map to send with the invocation.\n"
        "@return A proxy for this type, or null if the object does not support this type.");
    out << nl << "static " << prxName << " checkedCast(com.zeroc.Ice.ObjectPrx obj, String facet, " << contextParam
        << ')';
    out << sb;
    out << nl << "return (obj == null) ? null : checkedCast(obj.ice_facet(facet), context);";
    out << eb;

    out << sp;
    writeDocComment(
        out,
        "Downcasts the given proxy to this type without contacting the remote server.\n"
        "@param obj The untyped proxy.\n"
        "@return A proxy for this type.");
    out << nl << "static " << prxName << " uncheckedCast(com.zeroc.Ice.ObjectPrx obj)";
    out << sb;
    out << nl << "return (obj == null) ? null : new " << prxIName << "(obj);";
    out << eb;

    out << sp;
    writeDocComment(
        out,
        "Downcasts the given proxy to this type without contacting the remote server.\n"
        "@param obj The untyped proxy.\n"
        "@param facet The name of the desired facet.\n"
        "@return A proxy for this type.");
    out << nl << "static " << prxName << " uncheckedCast(com.zeroc.Ice.ObjectPrx obj, String facet)";
    out << sb;
    out << nl << "return (obj == null) ? null : new " << prxIName << "(obj.ice_facet(facet));";
    out << eb;

    // Generate overrides for all the methods on `ObjectPrx` with covariant return types.
    static constexpr string_view objectPrxMethods[] = {
        "ice_context(java.util.Map<String, String> newContext)",
        "ice_adapterId(String newAdapterId)",
        "ice_endpoints(com.zeroc.Ice.Endpoint[] newEndpoints)",
        "ice_locatorCacheTimeout(int newTimeout)",
        "ice_invocationTimeout(int newTimeout)",
        "ice_connectionCached(boolean newCache)",
        "ice_endpointSelection(com.zeroc.Ice.EndpointSelectionType newType)",
        "ice_secure(boolean b)",
        "ice_encodingVersion(com.zeroc.Ice.EncodingVersion e)",
        "ice_preferSecure(boolean b)",
        "ice_router(com.zeroc.Ice.RouterPrx router)",
        "ice_locator(com.zeroc.Ice.LocatorPrx locator)",
        "ice_collocationOptimized(boolean b)",
        "ice_twoway()",
        "ice_oneway()",
        "ice_batchOneway()",
        "ice_datagram()",
        "ice_batchDatagram()",
        "ice_compress(boolean co)",
        "ice_connectionId(String connectionId)",
        "ice_fixed(com.zeroc.Ice.Connection connection)",
    };
    for (const auto& method : objectPrxMethods)
    {
        out << sp;
        out << nl << "@Override";
        out << nl << prxName << " " << method << ";";
    }

    out << sp;
    out << nl << "static String ice_staticId()";
    out << sb;
    out << nl << "return \"" << p->scoped() << "\";";
    out << eb;

    out << sp;
    writeDocComment(out, "@hidden");
    out << nl << "@Override";
    out << nl << "default " << prxName << " _newInstance(com.zeroc.Ice.Reference ref)";
    out << sb;
    out << nl << "return new " << prxIName << "(ref);";
    out << eb;

    out << eb;
    close();

    string absolute = getUnqualified(p, "", "_", "PrxI");

    open(absolute, p->file());

    Output& outi = output();

    outi << sp;
    writeHiddenDocComment(outi);
    if (dc && dc->isDeprecated())
    {
        outi << nl << "@Deprecated";
    }
    outi << nl << "public class " << prxIName;
    outi << " extends com.zeroc.Ice._ObjectPrxFactoryMethods<" << prxName << ">";
    outi << " implements " << prxName;
    outi << sb;

    // Constructor which directly takes a Reference.
    outi << sp;
    outi << nl << prxIName << "(com.zeroc.Ice.Reference ref)";
    outi << sb;
    outi << nl << "super(ref);";
    outi << eb;

    // Copy constructor
    outi << sp;
    outi << nl << prxIName << "(com.zeroc.Ice.ObjectPrx obj)";
    outi << sb;
    outi << nl << "super(obj);";
    outi << eb;

    outi << sp;
    outi << nl << "private static final long serialVersionUID = 0L;";
    outi << eb;
    close();
}

void
Slice::Gen::ProxyVisitor::visitOperation(const OperationPtr& p)
{
    const string name = fixKwd(p->name());
    const InterfaceDefPtr interface = p->interface();
    const string package = getPackage(interface);

    Output& out = output();

    const TypePtr ret = p->returnType();
    const string retS = getResultType(p, package, false, false);
    const bool returnsParams = ret || !p->outParameters().empty();
    const vector<string> params = getParamsProxy(p, package, false);
    const bool sendsOptionals = p->sendsOptionals();
    vector<string> paramsOpt;
    if (sendsOptionals)
    {
        paramsOpt = getParamsProxy(p, package, true);
    }
    const vector<string> args = getInArgs(p);

    // Arrange exceptions into most-derived to least-derived order. If we don't
    // do this, a base exception handler can appear before a derived exception
    // handler, causing compiler warnings and resulting in the base exception
    // being marshaled instead of the derived exception.
    ExceptionList throws = p->throws();
    throws.sort(Slice::DerivedToBaseCompare());

    const string contextParamName = getEscapedParamName(p, "context");
    const string contextDoc = "@param " + contextParamName + " The Context map to send with the invocation.";
    const string contextParam = "java.util.Map<String, String> " + contextParamName;
    const string noExplicitContextArg = "com.zeroc.Ice.ObjectPrx.noExplicitContext";

    DocCommentPtr dc = p->parseDocComment(javaLinkFormatter);

    //
    // Synchronous methods with required parameters.
    //
    out << sp;
    writeProxyDocComment(out, p, package, dc, false, "");
    if (dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }
    out << nl << "default " << retS << ' ' << name << spar << params << epar;
    writeThrowsClause(package, throws);
    out << sb;
    out << nl;
    if (returnsParams)
    {
        out << "return ";
    }
    out << name << spar << args << noExplicitContextArg << epar << ';';
    out << eb;

    out << sp;
    writeProxyDocComment(out, p, package, dc, false, contextDoc);
    if (dc && dc->isDeprecated())
    {
        out << nl << "@Deprecated";
    }
    out << nl << "default " << retS << ' ' << name << spar << params << contextParam << epar;
    writeThrowsClause(package, throws);
    out << sb;
    if (throws.empty())
    {
        out << nl;
        if (returnsParams)
        {
            out << "return ";
        }
        out << "_iceI_" << p->name() << "Async" << spar << args << contextParamName << "true" << epar
            << ".waitForResponse();";
    }
    else
    {
        out << nl << "try";
        out << sb;
        out << nl;
        if (returnsParams)
        {
            out << "return ";
        }
        out << "_iceI_" << p->name() << "Async" << spar << args << contextParamName << "true" << epar
            << ".waitForResponseOrUserEx();";
        out << eb;
        for (ExceptionList::const_iterator t = throws.begin(); t != throws.end(); ++t)
        {
            string exS = getUnqualified(*t, package);
            out << nl << "catch(" << exS << " ex)";
            out << sb;
            out << nl << "throw ex;";
            out << eb;
        }
        out << nl << "catch(com.zeroc.Ice.UserException ex)";
        out << sb;
        out << nl << "throw com.zeroc.Ice.UnknownUserException.fromTypeId(ex.ice_id());";
        out << eb;
    }
    out << eb;

    //
    // Synchronous methods using optional parameters (if any).
    //
    if (sendsOptionals)
    {
        out << sp;
        writeProxyDocComment(out, p, package, dc, false, "");
        if (dc && dc->isDeprecated())
        {
            out << nl << "@Deprecated";
        }
        out << nl << "default " << retS << ' ' << name << spar << paramsOpt << epar;
        writeThrowsClause(package, throws);
        out << sb;
        out << nl;
        if (returnsParams)
        {
            out << "return ";
        }
        out << name << spar << args << noExplicitContextArg << epar << ';';
        out << eb;

        out << sp;
        writeProxyDocComment(out, p, package, dc, false, contextDoc);
        if (dc && dc->isDeprecated())
        {
            out << nl << "@Deprecated";
        }
        out << nl << "default " << retS << ' ' << name << spar << paramsOpt << contextParam << epar;
        writeThrowsClause(package, throws);
        out << sb;
        if (throws.empty())
        {
            out << nl;
            if (returnsParams)
            {
                out << "return ";
            }
            out << "_iceI_" << p->name() << "Async" << spar << args << contextParamName << "true" << epar
                << ".waitForResponse();";
        }
        else
        {
            out << nl << "try";
            out << sb;
            out << nl;
            if (returnsParams)
            {
                out << "return ";
            }
            out << "_iceI_" << p->name() << "Async" << spar << args << contextParamName << "true" << epar
                << ".waitForResponseOrUserEx();";
            out << eb;
            for (ExceptionList::const_iterator t = throws.begin(); t != throws.end(); ++t)
            {
                string exS = getUnqualified(*t, package);
                out << nl << "catch(" << exS << " ex)";
                out << sb;
                out << nl << "throw ex;";
                out << eb;
            }
            out << nl << "catch(com.zeroc.Ice.UserException ex)";
            out << sb;
            out << nl << "throw com.zeroc.Ice.UnknownUserException.fromTypeId(ex.ice_id());";
            out << eb;
        }
        out << eb;
    }

    //
    // Asynchronous methods with required parameters.
    //
    out << sp;
    writeProxyDocComment(out, p, package, dc, true, "");

    const string futureType = getFutureType(p, package);

    if (dc && dc->isDeprecated())
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
    if (dc && dc->isDeprecated())
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
    out << nl << "default " << futureImpl << " _iceI_" << p->name() << "Async" << spar
        << getParamsProxy(p, package, false, true) << "java.util.Map<String, String> context"
        << "boolean sync" << epar;
    out << sb;
    out << nl << futureImpl << " f = new com.zeroc.Ice.OutgoingAsync<>(this, \"" << p->name() << "\", "
        << sliceModeToIceMode(p->mode()) << ", sync, " << (throws.empty() ? "null" : "_iceE_" + p->name()) << ");";

    out << nl << "f.invoke(";
    out.useCurrentPosAsIndent();
    out << (p->returnsData() ? "true" : "false") << ", context, " << opFormatTypeToString(p) << ", ";
    if (!p->inParameters().empty())
    {
        out << "ostr -> {";
        out.inc();
        writeMarshalProxyParams(out, package, p, false);
        out.dec();
        out << nl << '}';
    }
    else
    {
        out << "null";
    }
    out << ", ";
    if (returnsParams)
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

    if (!throws.empty())
    {
        out << sp;
        writeHiddenDocComment(out);
        out << nl << "static final Class<?>[] _iceE_" << p->name() << " =";
        out << sb;
        for (ExceptionList::const_iterator t = throws.begin(); t != throws.end(); ++t)
        {
            if (t != throws.begin())
            {
                out << ",";
            }
            out << nl << getUnqualified(*t, package) << ".class";
        }
        out << eb << ';';
    }

    if (sendsOptionals)
    {
        out << sp;
        writeProxyDocComment(out, p, package, dc, true, "");

        const string future = getFutureType(p, package);

        if (dc && dc->isDeprecated())
        {
            out << nl << "@Deprecated";
        }
        out << nl << "default " << future << ' ' << p->name() << "Async" << spar << paramsOpt << epar;
        out << sb;
        out << nl << "return _iceI_" << p->name() << "Async" << spar << args << noExplicitContextArg << "false" << epar
            << ';';
        out << eb;

        out << sp;
        writeProxyDocComment(out, p, package, dc, true, contextDoc);

        if (dc && dc->isDeprecated())
        {
            out << nl << "@Deprecated";
        }
        out << nl << "default " << future << ' ' << p->name() << "Async" << spar << paramsOpt << contextParam << epar;
        out << sb;
        out << nl << "return _iceI_" << p->name() << "Async" << spar << args << contextParamName << "false" << epar
            << ';';
        out << eb;

        out << sp;
        writeHiddenProxyDocComment(out, p);
        out << nl << "default " << futureImpl << " _iceI_" << p->name() << "Async" << spar
            << getParamsProxy(p, package, true, true) << "java.util.Map<String, String> context"
            << "boolean sync" << epar;
        out << sb;
        out << nl << futureImpl << " f = new com.zeroc.Ice.OutgoingAsync<>(this, \"" << p->name() << "\", "
            << sliceModeToIceMode(p->mode()) << ", sync, " << (throws.empty() ? "null" : "_iceE_" + p->name()) << ");";

        out << nl << "f.invoke(";
        out.useCurrentPosAsIndent();
        out << (p->returnsData() ? "true" : "false") << ", context, " << opFormatTypeToString(p) << ", ";
        if (!p->inParameters().empty())
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
        if (returnsParams)
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
    }
}
