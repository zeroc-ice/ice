// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/Functional.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/InputUtil.h>
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
#include <IceUtil/Unicode.h>
#include <Slice/Checksum.h>
#include <Slice/DotNetNames.h>
#include <Slice/FileTracker.h>
#include <Slice/Util.h>
#include <string.h>

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

namespace
{

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

bool
isClassType(const TypePtr type)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    return (builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(type);
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

}

Slice::CsVisitor::CsVisitor(Output& out) : _out(out)
{
}

Slice::CsVisitor::~CsVisitor()
{
}

void
Slice::CsVisitor::writeMarshalUnmarshalParams(const ParamDeclList& params, const OperationPtr& op, bool marshal)
{
    ParamDeclList optionals;

    for(ParamDeclList::const_iterator pli = params.begin(); pli != params.end(); ++pli)
    {
        string param = fixId((*pli)->name());
        TypePtr type = (*pli)->type();
        if(!marshal && isClassType(type))
        {
            param = (*pli)->name() + "__PP";
            string typeS = typeToString(type);
            if((*pli)->optional())
            {
                _out << nl << "Ice.OptionalPatcher<" << typeS << "> " << param
                     << " = new Ice.OptionalPatcher<" << typeS << ">(" << getStaticId(type) << ");";
            }
            else
            {
                _out << nl << "IceInternal.ParamPatcher<" << typeS << "> " << param
                     << " = new IceInternal.ParamPatcher<" << typeS << ">(" << getStaticId(type) << ");";
            }
        }

        if((*pli)->optional())
        {
            optionals.push_back(*pli);
        }
        else
        {
            writeMarshalUnmarshalCode(_out, type, param, marshal, false);
        }
    }

    TypePtr ret;

    if(op && op->returnType())
    {
        ret = op->returnType();

        string param = "ret__";
        if(!marshal && isClassType(ret))
        {
            param += "PP";
            string typeS = typeToString(ret);
            if(op->returnIsOptional())
            {
                _out << nl << "Ice.OptionalPatcher<" << typeS << "> " << param
                     << " = new Ice.OptionalPatcher<" << typeS << ">(" << getStaticId(ret) << ");";
            }
            else
            {
                _out << nl << "IceInternal.ParamPatcher<" << typeS << "> " << param
                     << " = new IceInternal.ParamPatcher<" << typeS << ">(" << getStaticId(ret) << ");";
            }
        }

        if(!op->returnIsOptional())
        {
            writeMarshalUnmarshalCode(_out, ret, param, marshal, false);
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
            const string param = !marshal && isClassType(ret) ? "ret__PP" : "ret__";
            writeOptionalMarshalUnmarshalCode(_out, ret, param, op->returnTag(), marshal, false);
            checkReturnType = false;
        }

        string param = fixId((*pli)->name());
        TypePtr type = (*pli)->type();

        if(!marshal && isClassType(type))
        {
            param = (*pli)->name() + "__PP";
        }

        writeOptionalMarshalUnmarshalCode(_out, type, param, (*pli)->tag(), marshal, false);
    }

    if(checkReturnType)
    {
        const string param = !marshal && isClassType(ret) ? "ret__PP" : "ret__";
        writeOptionalMarshalUnmarshalCode(_out, ret, param, op->returnTag(), marshal, false);
    }
}

void
Slice::CsVisitor::writePostUnmarshalParams(const ParamDeclList& params, const OperationPtr& op)
{
    for(ParamDeclList::const_iterator pli = params.begin(); pli != params.end(); ++pli)
    {
        if(isClassType((*pli)->type()))
        {
            const string tmp = (*pli)->name() + "__PP";
            _out << nl << fixId((*pli)->name()) << " = " << tmp << ".value;";
        }
    }

    if(op && op->returnType() && isClassType(op->returnType()))
    {
        _out << nl << "ret__ = ret__PP.value;";
    }
}

void
Slice::CsVisitor::writeMarshalDataMember(const DataMemberPtr& member, const string& name)
{
    if(member->optional())
    {
        writeOptionalMarshalUnmarshalCode(_out, member->type(), name, member->tag(), true, false);
    }
    else
    {
        writeMarshalUnmarshalCode(_out, member->type(), name, true, false);
    }
}

void
Slice::CsVisitor::writeUnmarshalDataMember(const DataMemberPtr& member, const string& name, bool needPatcher,
                                           int& patchIter)
{
    const bool classType = isClassType(member->type());

    string patcher;
    if(classType)
    {
        patcher = "new Patcher__(" + getStaticId(member->type()) + ", this";
        if(needPatcher)
        {
            ostringstream ostr;
            ostr << ", " << patchIter++;
            patcher += ostr.str();
        }
        patcher += ")";
    }

    if(member->optional())
    {
        writeOptionalMarshalUnmarshalCode(_out, member->type(), classType ? patcher : name, member->tag(), false,
                                          false);
    }
    else
    {
        writeMarshalUnmarshalCode(_out, member->type(), classType ? patcher : name, false, false);
    }
}

void
Slice::CsVisitor::writeStreamMarshalDataMember(const DataMemberPtr& member, const string& name)
{
    if(!member->optional())
    {
        writeMarshalUnmarshalCode(_out, member->type(), name, true, true);
        return;
    }

    const string flag = name + ".HasValue";
    const string value = name + ".Value";

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(member->type());
    if(builtin)
    {
        _out << nl << "if(" << flag << " && outS__.writeOptional(" << member->tag() << ", "
             << getOptionalFormat(member->type()) << "))";
        _out << sb;
        if(builtin->kind() == Builtin::KindObjectProxy)
        {
            _out << nl << "int pos__ = outS__.startSize();";
        }
        writeMarshalUnmarshalCode(_out, member->type(), value, true, true);
        if(builtin->kind() == Builtin::KindObjectProxy)
        {
            _out << nl << "outS__.endSize(pos__);";
        }
        _out << eb;
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(member->type());
    if(seq)
    {
        string meta;
        const bool serializable = seq->findMetaData("clr:serializable:", meta);
        const bool isArray = !seq->findMetaData("clr:generic:", meta) && !seq->hasMetaData("clr:collection");
        const string length = isArray ? value + ".Length" : value + ".Count";

        _out << nl << "if(" << flag << " && outS__.writeOptional(" << member->tag() << ", "
             << getOptionalFormat(member->type()) << "))";
        _out << sb;

        BuiltinPtr b = BuiltinPtr::dynamicCast(seq->type());

        if(serializable)
        {
            writeMarshalUnmarshalCode(_out, member->type(), value, true, true);
        }
        else if(b)
        {
            switch(b->kind())
            {
            case Builtin::KindByte:
            case Builtin::KindBool:
                writeMarshalUnmarshalCode(_out, member->type(), value, true, true);
                break;
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindFloat:
            case Builtin::KindLong:
            case Builtin::KindDouble:
                _out << nl << "outS__.writeSize(" << value << " == null ? 1 : " << length << " * " << b->minWireSize()
                     << " + (" << length << " > 254 ? 5 : 1));";
                writeMarshalUnmarshalCode(_out, member->type(), value, true, true);
                break;
            case Builtin::KindString:
            case Builtin::KindObject:
            case Builtin::KindObjectProxy:
            {
                _out << nl << "int pos__ = outS__.startSize();";
                writeMarshalUnmarshalCode(_out, member->type(), value, true, true);
                _out << nl << "outS__.endSize(pos__);";
                break;
            }
            case Builtin::KindLocalObject:
            {
                assert(false);
                break;
            }
            }
        }
        else
        {
            StructPtr st = StructPtr::dynamicCast(seq->type());
            if(st && !st->isVariableLength())
            {
                _out << nl << "outS__.writeSize(" << value << " == null ? 1 : " << length << " * " << st->minWireSize()
                     << " + (" << length << " > 254 ? 5 : 1));";
                writeMarshalUnmarshalCode(_out, member->type(), value, true, true);
            }
            else
            {
                _out << nl << "int pos__ = outS__.startSize();";
                writeMarshalUnmarshalCode(_out, member->type(), value, true, true);
                _out << nl << "outS__.endSize(pos__);";
            }
        }

        _out << eb;
        return;
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(member->type());
    if(d)
    {
        _out << nl << "if(" << flag << " && outS__.writeOptional(" << member->tag() << ", "
             << getOptionalFormat(member->type()) << "))";
        _out << sb;

        if(d->keyType()->isVariableLength() || d->valueType()->isVariableLength())
        {
            _out << nl << "int pos__ = outS__.startSize();";
        }
        else
        {
            _out << nl << "outS__.writeSize(" << value << " == null ? 1 : " << value << ".Count * "
                 << (d->keyType()->minWireSize() + d->valueType()->minWireSize()) << " + (" << value
                 << ".Count > 254 ? 5 : 1));";
        }

        writeMarshalUnmarshalCode(_out, member->type(), value, true, true);
        if(d->keyType()->isVariableLength() || d->valueType()->isVariableLength())
        {
            _out << nl << "outS__.endSize(pos__);";
        }
        _out << eb;
        return;
    }

    StructPtr st = StructPtr::dynamicCast(member->type());
    if(st)
    {
        _out << nl << "if(" << flag << " && outS__.writeOptional(" << member->tag() << ", "
             << getOptionalFormat(member->type()) << "))";
        _out << sb;

        if(st->isVariableLength())
        {
            _out << nl << "int pos__ = outS__.startSize();";
        }
        else
        {
            _out << nl << "outS__.writeSize(" << st->minWireSize() << ");";
        }

        writeMarshalUnmarshalCode(_out, member->type(), value, true, true);
        if(st->isVariableLength())
        {
            _out << nl << "outS__.endSize(pos__);";
        }
        _out << eb;
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(member->type());
    if(en)
    {
        _out << nl << "if(" << flag << " && outS__.writeOptional(" << member->tag() << ", "
             << getOptionalFormat(member->type()) << "))";
        _out << sb;
        writeMarshalUnmarshalCode(_out, member->type(), value, true, true);
        _out << eb;
        return;
    }

    ProxyPtr pr = ProxyPtr::dynamicCast(member->type());
    if(pr)
    {
        _out << nl << "if(" << flag << " && outS__.writeOptional(" << member->tag() << ", "
             << getOptionalFormat(member->type()) << "))";
        _out << sb;
        _out << nl << "int pos__ = outS__.startSize();";
        writeMarshalUnmarshalCode(_out, member->type(), value, true, true);
        _out << nl << "outS__.endSize(pos__);";
        _out << eb;
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(member->type());
    assert(cl);
    _out << nl << "if(" << flag << " && outS__.writeOptional(" << member->tag() << ", "
         << getOptionalFormat(member->type()) << "))";
    _out << sb;
    writeMarshalUnmarshalCode(_out, member->type(), value, true, true);
    _out << eb;
}

void
Slice::CsVisitor::writeStreamUnmarshalDataMember(const DataMemberPtr& member, const string& name, bool needPatcher,
                                                 int& patchIter)
{
    const bool classType = isClassType(member->type());

    string patcher;
    if(classType)
    {
        patcher = "new Patcher__(" + getStaticId(member->type()) + ", this";
        if(needPatcher)
        {
            ostringstream ostr;
            ostr << ", " << patchIter++;
            patcher += ostr.str();
        }
        patcher += ")";
    }

    if(!member->optional())
    {
        writeMarshalUnmarshalCode(_out, member->type(), classType ? patcher : name, false, true);
        return;
    }

    _out << nl << "if(inS__.readOptional(" << member->tag() << ", "
         << getOptionalFormat(member->type()) << "))";
    _out << sb;

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(member->type());
    if(builtin && builtin->kind() == Builtin::KindObjectProxy)
    {
        _out << nl << "inS__.skip(4);";
    }

    SequencePtr seq = SequencePtr::dynamicCast(member->type());
    if(seq)
    {
        BuiltinPtr b = BuiltinPtr::dynamicCast(seq->type());
        if(!b || (b->kind() != Builtin::KindByte && b->kind() != Builtin::KindBool))
        {
            if(seq->type()->isVariableLength())
            {
                _out << nl << "inS__.skip(4);";
            }
            else
            {
                _out << nl << "inS__.skipSize();";
            }
        }
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(member->type());
    if(d)
    {
        if(d->keyType()->isVariableLength() || d->valueType()->isVariableLength())
        {
            _out << nl << "inS__.skip(4);";
        }
        else
        {
            _out << nl << "inS__.skipSize();";
        }
    }

    StructPtr st = StructPtr::dynamicCast(member->type());
    if(st)
    {
        if(st->isVariableLength())
        {
            _out << nl << "inS__.skip(4);";
        }
        else
        {
            _out << nl << "inS__.skipSize();";
        }
    }

    ProxyPtr pr = ProxyPtr::dynamicCast(member->type());
    if(pr)
    {
        _out << nl << "inS__.skip(4);";
    }

    const string typeS = typeToString(member->type());
    const string tmp = "tmpValue__";

    if(st)
    {
        if(isValueType(st))
        {
            _out << nl << typeS << ' ' << tmp << " = new " << typeS << "();";
        }
        else
        {
            _out << nl << typeS << ' ' << tmp << " = null;";
        }
    }
    else if(!classType)
    {
        _out << nl << typeS << ' ' << tmp << ';';
    }

    writeMarshalUnmarshalCode(_out, member->type(), classType ? patcher : tmp, false, true);

    if(!classType)
    {
        _out << nl << name << " = new " << typeToString(member->type(), true) << '(' << tmp << ");";
    }

    _out << eb;
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
        for(OperationList::const_iterator op = allOps.begin(); op != allOps.end(); ++op)
        {
            ClassDefPtr containingClass = ClassDefPtr::dynamicCast((*op)->container());
            bool amd = containingClass->hasMetaData("amd") || (*op)->hasMetaData("amd");
            string name = fixId((*op)->name(), DotNet::ICloneable, true);
            if(!amd)
            {
                vector<string> params = getParams(*op);
                vector<string> args = getArgs(*op);
                string retS = typeToString((*op)->returnType(), (*op)->returnIsOptional());

                _out << sp << nl << "public " << retS << ' ' << name << spar << params << epar;
                _out << sb;
                _out << nl;
                if((*op)->returnType())
                {
                    _out << "return ";
                }
                _out << name << spar << args << "Ice.ObjectImpl.defaultCurrent" << epar << ';';
                _out << eb;

                _out << sp << nl << "public abstract " << retS << ' ' << name << spar << params;
                if(!containingClass->isLocal())
                {
                    _out << "Ice.Current current__";
                }
                _out << epar << ';';
            }
            else
            {
                vector<string> params = getParamsAsync(*op, true);
                vector<string> args = getArgsAsync(*op, true);

                _out << sp << nl << "public void " << name << "_async" << spar << params << epar;
                _out << sb;
                _out << nl << name << "_async" << spar << args << epar << ';';
                _out << eb;

                _out << sp << nl << "public abstract void " << name << "_async"
                     << spar << params << "Ice.Current current__" << epar << ';';
            }
        }

        _out << sp << nl << "#endregion"; // Inherited Slice operations
    }
}

void
Slice::CsVisitor::writeDispatchAndMarshalling(const ClassDefPtr& p, bool stream)
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
    if(p->isInterface() || hasBaseClass) {
        _out << nl << "public static new readonly string[] ids__ = ";
    }
    else
    {
        _out << nl << "public static readonly string[] ids__ = ";
    }
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

    string qualifier = "override ";
    if(!p->isInterface() && !hasBaseClass) {
        qualifier = "virtual ";
    }

    _out << sp;
    if(!p->isInterface())
    {
        emitGeneratedCodeAttribute();
    }
    _out << nl << "public " << qualifier << "bool ice_isA(string s)";
    _out << sb;
    _out << nl << "return _System.Array.BinarySearch(ids__, s, IceUtilInternal.StringUtil.OrdinalStringComparer) >= 0;";
    _out << eb;

    _out << sp;
    if(!p->isInterface())
    {
        emitGeneratedCodeAttribute();
    }
    _out << nl << "public " << qualifier << "bool ice_isA(string s, Ice.Current current__)";
    _out << sb;
    _out << nl << "return _System.Array.BinarySearch(ids__, s, IceUtilInternal.StringUtil.OrdinalStringComparer) >= 0;";
    _out << eb;

    if(!p->isInterface() && !hasBaseClass) {
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public virtual void ice_ping()";
        _out << sb;
        _out << eb;

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public virtual void ice_ping(Ice.Current current__)";
        _out << sb;
        _out << eb;
    }

    _out << sp;
    if(!p->isInterface())
    {
        emitGeneratedCodeAttribute();
    }
    _out << nl << "public " << qualifier << "string[] ice_ids()";
    _out << sb;
    _out << nl << "return ids__;";
    _out << eb;

    _out << sp;
    if(!p->isInterface())
    {
        emitGeneratedCodeAttribute();
    }
    _out << nl << "public " << qualifier << "string[] ice_ids(Ice.Current current__)";
    _out << sb;
    _out << nl << "return ids__;";
    _out << eb;

    _out << sp;
    if(!p->isInterface())
    {
        emitGeneratedCodeAttribute();
    }
    _out << nl << "public " << qualifier << "string ice_id()";
    _out << sb;
    _out << nl << "return ids__[" << scopedPos << "];";
    _out << eb;

    _out << sp;
    if(!p->isInterface())
    {
        emitGeneratedCodeAttribute();
    }
    _out << nl << "public " << qualifier << "string ice_id(Ice.Current current__)";
    _out << sb;
    _out << nl << "return ids__[" << scopedPos << "];";
    _out << eb;

    _out << sp;
    if(!p->isInterface())
    {
        emitGeneratedCodeAttribute();
    }
    if(p->isInterface() || hasBaseClass) {
        _out << nl << "public static new string ice_staticId()";
    }
    else
    {
        _out << nl << "public static string ice_staticId()";
    }
    _out << sb;
    _out << nl << "return ids__[" << scopedPos << "];";
    _out << eb;

    _out << sp << nl << "#endregion"; // Slice type-related members

    OperationList ops = p->operations();
    if(!p->isInterface() || ops.size() != 0)
    {
        _out << sp << nl << "#region Operation dispatch";
    }

    if(!p->isInterface() && !hasBaseClass) {
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public virtual Ice.DispatchStatus ice_dispatch(Ice.Request request, Ice.DispatchInterceptorAsyncCallback cb)";
        _out << sb;
        _out << nl << "IceInternal.Incoming inc = (IceInternal.Incoming)request;";
        _out << nl << "if(cb != null)";
        _out << sb;
        _out << nl << "inc.push(cb);";
        _out << eb;
        _out << nl << "try";
        _out << sb;
        _out << nl << "inc.startOver();";
        _out << nl << "return dispatch__(inc, inc.getCurrent());";
        _out << eb;
        _out << nl << "finally";
        _out << sb;
        _out << nl << "if(cb != null)";
        _out << sb;
        _out << nl << "inc.pop();";
        _out << eb;
        _out << eb;
        _out << eb;

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public virtual Ice.DispatchStatus ice_dispatch(Ice.Request request)";
        _out << sb;
        _out << nl << "return ice_dispatch(request, null);";
        _out << eb;
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
        _out << nl << "public static Ice.DispatchStatus " << opName << "___(" << name
             << " obj__, IceInternal.Incoming inS__, Ice.Current current__)";
        _out << sb;

        TypePtr ret = op->returnType();

        ParamDeclList paramList = op->parameters();
        ParamDeclList inParams;
        ParamDeclList outParams;

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

        _out << nl << "Ice.ObjectImpl.checkMode__(" << sliceModeToIceMode(op->mode()) << ", current__.mode);";
        if(!inParams.empty())
        {
            //
            // Unmarshal 'in' parameters.
            //
            _out << nl << "IceInternal.BasicStream is__ = inS__.startReadParams();";
            for(ParamDeclList::const_iterator pli = inParams.begin(); pli != inParams.end(); ++pli)
            {
                string param = fixId((*pli)->name());
                string typeS = typeToString((*pli)->type(), (*pli)->optional());
                const bool isClass = isClassType((*pli)->type());

                if((*pli)->optional())
                {
                    if(!isClass)
                    {
                        _out << nl << typeS << ' ' << param << ';';
                    }
                }
                else if(!isClass)
                {
                    _out << nl << typeS << ' ' << param << ';';
                    StructPtr st = StructPtr::dynamicCast((*pli)->type());
                    if(st)
                    {
                        if(isValueType(st))
                        {
                            _out << nl << param << " = new " << typeS << "();";
                        }
                        else
                        {
                            _out << nl << param << " = null;";
                        }
                    }
                }
            }
            writeMarshalUnmarshalParams(inParams, 0, false);
            if(op->sendsClasses(false))
            {
                _out << nl << "is__.readPendingObjects();";
            }
            _out << nl << "inS__.endReadParams();";
        }
        else
        {
            _out << nl << "inS__.readEmptyParams();";
        }

        const bool amd = p->hasMetaData("amd") || op->hasMetaData("amd");
        if(!amd)
        {
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

            for(ParamDeclList::const_iterator pli = outParams.begin(); pli != outParams.end(); ++pli)
            {
                string typeS = typeToString((*pli)->type(), (*pli)->optional());
                _out << nl << typeS << ' ' << fixId((*pli)->name()) << ";";
            }

            //
            // Call on the servant.
            //
            if(!throws.empty())
            {
                _out << nl << "try";
                _out << sb;
            }
            _out << nl;
            if(ret)
            {
                string retS = typeToString(ret, op->returnIsOptional());
                _out << retS << " ret__ = ";
            }
            _out << "obj__." << fixId(opName, DotNet::ICloneable, true) << spar;
            for(ParamDeclList::const_iterator pli = inParams.begin(); pli != inParams.end(); ++pli)
            {
                string arg = fixId((*pli)->name());
                if(isClassType((*pli)->type()))
                {
                    arg = (*pli)->name() + "__PP.value";
                }
                _out << arg;
            }
            for(ParamDeclList::const_iterator pli = outParams.begin(); pli != outParams.end(); ++pli)
            {
                _out << "out " + fixId((*pli)->name());
            }
            _out << "current__" << epar << ';';

            //
            // Marshal 'out' parameters and return value.
            //
            if(!outParams.empty() || ret)
            {
                _out << nl << "IceInternal.BasicStream os__ = inS__.startWriteParams__("
                     << opFormatTypeToString(op) << ");";
                writeMarshalUnmarshalParams(outParams, op, true);
                if(op->returnsClasses(false))
                {
                    _out << nl << "os__.writePendingObjects();";
                }
                _out << nl << "inS__.endWriteParams__(true);";
            }
            else
            {
                _out << nl << "inS__.writeEmptyParams__();";
            }
            _out << nl << "return Ice.DispatchStatus.DispatchOK;";

            //
            // Handle user exceptions.
            //
            if(!throws.empty())
            {
                _out << eb;
                for(ExceptionList::const_iterator t = throws.begin(); t != throws.end(); ++t)
                {
                    string exS = fixId((*t)->scoped());
                    _out << nl << "catch(" << exS << " ex__)";
                    _out << sb;
                    _out << nl << "inS__.writeUserException__(ex__, " << opFormatTypeToString(op) << ");";
                    _out << nl << "return Ice.DispatchStatus.DispatchUserException;";
                    _out << eb;
                }
            }

            _out << eb;
        }
        else
        {
            //
            // Call on the servant.
            //
            string classNameAMD = "AMD_" + p->name();
            _out << nl << classNameAMD << '_' << opName << " cb__ = new _" << classNameAMD << '_' << opName
                 << "(inS__);";
            _out << nl << "try";
            _out << sb;
            _out << nl << "obj__.";
            if(amd)
            {
                _out << opName << "_async";
            }
            else
            {
                _out << fixId(opName, DotNet::ICloneable, true);
            }
            _out << spar;
            if(amd)
            {
                _out << "cb__";
            }
            for(ParamDeclList::const_iterator pli = inParams.begin(); pli != inParams.end(); ++pli)
            {
                string arg = fixId((*pli)->name());
                if(isClassType((*pli)->type()))
                {
                    arg = (*pli)->name() + "__PP.value";
                }
                _out << arg;
            }
            _out << "current__" << epar << ';';
            _out << eb;
            _out << nl << "catch(_System.Exception ex__)";
            _out << sb;
            _out << nl << "cb__.ice_exception(ex__);";
            _out << eb;
            _out << nl << "return Ice.DispatchStatus.DispatchAsync;";

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

        _out << sp << nl << "private static string[] all__ =";
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
        _out << nl << "public " << qualifier << "Ice.DispatchStatus "
             << "dispatch__(IceInternal.Incoming inS__, Ice.Current current__)";
        _out << sb;
        _out << nl << "int pos = _System.Array.BinarySearch(all__, current__.operation, "
             << "IceUtilInternal.StringUtil.OrdinalStringComparer);";
        _out << nl << "if(pos < 0)";
        _out << sb;
        _out << nl << "throw new Ice.OperationNotExistException(current__.id, current__.facet, current__.operation);";
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
                _out << nl << "return Ice.ObjectImpl.ice_id___(this, inS__, current__);";
            }
            else if(opName == "ice_ids")
            {
                _out << nl << "return Ice.ObjectImpl.ice_ids___(this, inS__, current__);";
            }
            else if(opName == "ice_isA")
            {
                _out << nl << "return Ice.ObjectImpl.ice_isA___(this, inS__, current__);";
            }
            else if(opName == "ice_ping")
            {
                _out << nl << "return Ice.ObjectImpl.ice_ping___(this, inS__, current__);";
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
                            _out << nl << "return " << opName << "___(this, inS__, current__);";
                        }
                        else
                        {
                            string base = cl->scoped();
                            if(cl->isInterface())
                            {
                                base += "Disp_";
                            }
                            _out << nl << "return " << fixId(base) << "." << opName << "___(this, inS__, current__);";
                        }
                        break;
                    }
                }
            }
            _out << eb;
        }
        _out << eb;
        _out << sp << nl << "_System.Diagnostics.Debug.Assert(false);";
        _out << nl << "throw new Ice.OperationNotExistException(current__.id, current__.facet, current__.operation);";
        _out << eb;
    }
    /*else if(!p->isInterface() && !hasBaseClass)
    {
        _out << sp << nl << "private static string[] all__ =";
        _out << sb;
        _out << nl << "\"ice_id\","
        _out << nl << "\"ice_ids\","
        _out << nl << "\"ice_isA\","
        _out << nl << "\"ice_ping\","
        _out << eb << ';';

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public virtual Ice.DispatchStatus "
             << "dispatch__(IceInternal.Incoming inS__, Ice.Current current__)";
        _out << sb;
        _out << nl << "int pos = System.Array.BinarySearch(all__, current__.operation);";
        _out << nl << "if(pos < 0)";
        _out << sb;
        _out << nl << "throw new Ice.OperationNotExistException(current__.id, current__.facet, current__.operation);";
        _out << eb;
        _out << sp << nl << "switch(pos)";
        _out << sb;
        _out << nl << "case 0:";
        _out << sb;
        _out << nl << "return ice_id___(this, inS__, current__);";
        _out << eb;


        int i = 0;
        for(StringList::const_iterator q = allOpNames.begin(); q != allOpNames.end(); ++q)
        {
            string opName = *q;

            _out << nl << "case " << i++ << ':';
            _out << sb;
            if(opName == "ice_id")
            {
                _out << nl << "return ice_id___(this, inS__, current__);";
            }
            else if(opName == "ice_ids")
            {
                _out << nl << "return ice_ids___(this, inS__, current__);";
            }
            else if(opName == "ice_isA")
            {
                _out << nl << "return ice_isA___(this, inS__, current__);";
            }
            else if(opName == "ice_ping")
            {
                _out << nl << "return ice_ping___(this, inS__, current__);";
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
                            _out << nl << "return " << opName << "___(this, inS__, current__);";
                        }
                        else
                        {
                            string base = cl->scoped();
                            if(cl->isInterface())
                            {
                                base += "Disp_";
                            }
                            _out << nl << "return " << fixId(base) << "." << opName << "___(this, inS__, current__);";
                        }
                        break;
                    }
                }
            }
            _out << eb;
        }
        _out << eb;
        _out << sp << nl << "_System.Diagnostics.Debug.Assert(false);";
        _out << nl << "throw new Ice.OperationNotExistException(current__.id, current__.facet, current__.operation);";
        _out << eb;
    }*/

    if(!p->isInterface() || ops.size() != 0)
    {
        _out << sp << nl << "#endregion"; // Operation dispatch
    }

    //
    // Marshalling support
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

    if(!p->isInterface() && !hasBaseClass)
    {
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public virtual void ice_preMarshal()";
        _out << sb;
        _out << eb;

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public virtual void ice_postUnmarshal()";
        _out << sb;
        _out << eb;
    }

    if(preserved && !basePreserved)
    {
        _out << sp;
        if(!p->isInterface())
        {
            emitGeneratedCodeAttribute();
        }

        _out << nl << "public " << qualifier << "void write__(IceInternal.BasicStream os__)";
        _out << sb;
        _out << nl << "os__.startWriteObject(slicedData__);";
        _out << nl << "writeImpl__(os__);";
        _out << nl << "os__.endWriteObject();";
        _out << eb;

        _out << sp;
        if(!p->isInterface())
        {
            emitGeneratedCodeAttribute();
        }
        _out << nl << "public " << qualifier << "void read__(IceInternal.BasicStream is__)";
        _out << sb;
        _out << nl << "is__.startReadObject();";
        _out << nl << "readImpl__(is__);";
        _out << nl << "slicedData__ = is__.endReadObject(true);";
        _out << eb;

        if(stream)
        {
            _out << sp;
            if(!p->isInterface())
            {
                emitGeneratedCodeAttribute();
            }
            _out << nl << "public " << qualifier << "void write__(Ice.OutputStream outS__)";
            _out << sb;
            _out << nl << "outS__.startObject(slicedData__);";
            _out << nl << "writeImpl__(outS__);";
            _out << nl << "outS__.endObject();";
            _out << eb;

             _out << sp;
             if(!p->isInterface())
             {
                 emitGeneratedCodeAttribute();
             }
             _out << nl << "public " << qualifier << "void read__(Ice.InputStream inS__)";
             _out << sb;
             _out << nl << "inS__.startObject();";
             _out << nl << "readImpl__(inS__);";
             _out << nl << "slicedData__ = inS__.endObject(true);";
             _out << eb;
        }
        else if (!p->isInterface() && !hasBaseClass)
        {
            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << "public virtual void write__(Ice.OutputStream outS__)";
            _out << sb;
            _out << nl << "outS__.startObject(null);";
            _out << nl << "writeImpl__(outS__);";
            _out << nl << "outS__.endObject();";
            _out << eb;

             _out << sp;
            emitGeneratedCodeAttribute();
             _out << nl << "public virtual void read__(Ice.InputStream inS__)";
             _out << sb;
             _out << nl << "inS__.startObject();";
             _out << nl << "readImpl__(inS__);";
             _out << nl << "inS__.endObject(false);";
             _out << eb;
        }
    }
    else if (!p->isInterface() && !hasBaseClass)
    {
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public virtual void write__(IceInternal.BasicStream os__)";
        _out << sb;
        _out << nl << "os__.startWriteObject(null);";
        _out << nl << "writeImpl__(os__);";
        _out << nl << "os__.endWriteObject();";
        _out << eb;

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public virtual void read__(IceInternal.BasicStream is__)";
        _out << sb;
        _out << nl << "is__.startReadObject();";
        _out << nl << "readImpl__(is__);";
        _out << nl << "is__.endReadObject(false);";
        _out << eb;

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public virtual void write__(Ice.OutputStream outS__)";
        _out << sb;
        _out << nl << "outS__.startObject(null);";
        _out << nl << "writeImpl__(outS__);";
        _out << nl << "outS__.endObject();";
        _out << eb;

         _out << sp;
        emitGeneratedCodeAttribute();
         _out << nl << "public virtual void read__(Ice.InputStream inS__)";
         _out << sb;
         _out << nl << "inS__.startObject();";
         _out << nl << "readImpl__(inS__);";
         _out << nl << "inS__.endObject(false);";
         _out << eb;
    }

    _out << sp;
    if(!p->isInterface())
    {
        emitGeneratedCodeAttribute();
    }
    _out << nl << "protected " << qualifier << "void writeImpl__(IceInternal.BasicStream os__)";
    _out << sb;
    _out << nl << "os__.startWriteSlice(ice_staticId(), " << p->compactId() << (!base ? ", true" : ", false") << ");";
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
    _out << nl << "os__.endWriteSlice();";
    if(base)
    {
        _out << nl << "base.writeImpl__(os__);";
    }
    _out << eb;

    if(classMembers.size() != 0)
    {
        _out << sp;
        if(!p->isInterface())
        {
            emitGeneratedCodeAttribute();
        }
        _out << nl << "public sealed ";
        if(base && !base->allClassDataMembers().empty())
        {
            _out << "new ";
        }
        _out << "class Patcher__ : IceInternal.Patcher";
        _out << sb;
        if (p->isInterface())
        {
            _out << sp << nl << "internal Patcher__(string type, Ice.ObjectImpl instance";
        }
        else
        {
            _out << sp << nl << "internal Patcher__(string type, Ice.Object instance";
        }

        if(classMembers.size() > 1)
        {
            _out << ", int member";
        }
        _out << ") : base(type)";
        _out << sb;
        _out << nl << "_instance = (" << name << ")instance;";
        if(classMembers.size() > 1)
        {
            _out << nl << "_member = member;";
        }
        _out << eb;

        _out << sp << nl << "public override void patch(Ice.Object v)";
        _out << sb;
        _out << nl << "try";
        _out << sb;
        if(classMembers.size() > 1)
        {
            _out << nl << "switch(_member)";
            _out << sb;
        }
        int memberCount = 0;
        for(DataMemberList::const_iterator d = classMembers.begin(); d != classMembers.end(); ++d)
        {
            if((*d)->optional())
            {
                continue;
            }
            if(classMembers.size() > 1)
            {
                _out.dec();
                _out << nl << "case " << memberCount << ":";
                _out.inc();
            }
            string memberName = fixId((*d)->name(), DotNet::ICloneable, true);
            string memberType = typeToString((*d)->type(), (*d)->optional());

            if(ClassDeclPtr::dynamicCast((*d)->type()))
            {
                _out << nl << "_instance." << memberName << " = (" << memberType << ")v;";
            }
            else
            {
                _out << nl << "_instance." << memberName << " = v;";
            }

            if(classMembers.size() > 1)
            {
                _out << nl << "break;";
            }
            memberCount++;
        }

        for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
        {
            TypePtr paramType = (*d)->type();
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(paramType);
            if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(paramType))
            {
                if(classMembers.size() > 1)
                {
                    _out.dec();
                    _out << nl << "case " << memberCount << ":";
                    _out.inc();
                }
                string memberName = fixId((*d)->name(), DotNet::ICloneable, true);
                string memberType = typeToString((*d)->type(), (*d)->optional());
                if((*d)->optional())
                {
                    if(ClassDeclPtr::dynamicCast((*d)->type()))
                    {
                        _out << nl << "_instance." << memberName << " = new " << memberType << "(("
                            << typeToString((*d)->type()) << ")v);";
                    }
                    else
                    {
                        _out << nl << "_instance." << memberName << " = new " << memberType << "(v);";
                    }
                }
                else
                {
                    if(ClassDeclPtr::dynamicCast((*d)->type()))
                    {
                        _out << nl << "_instance." << memberName << " = (" << memberType << ")v;";
                    }
                    else
                    {
                        _out << nl << "_instance." << memberName << " = v;";
                    }
                }
                if(classMembers.size() > 1)
                {
                    _out << nl << "break;";
                }
                memberCount++;
            }
        }

        if(classMembers.size() > 1)
        {
            _out << eb;
        }
        _out << eb;
        _out << nl << "catch(_System.InvalidCastException)";
        _out << sb;
        _out << nl << "IceInternal.Ex.throwUOE(type(), v.ice_id());";
        _out << eb;
        _out << eb;

        _out << sp << nl << "private " << name << " _instance;";
        if(classMembers.size() > 1)
        {
            _out << nl << "private int _member;";
        }
        _out << eb;
    }


    _out << sp;
    if(!p->isInterface())
    {
        emitGeneratedCodeAttribute();
    }
    _out << nl << "protected " << qualifier << "void readImpl__(IceInternal.BasicStream is__)";
    _out << sb;
    _out << nl << "is__.startReadSlice();";
    int patchIter = 0;
    const bool needCustomPatcher = classMembers.size() > 1;
    for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
    {
        if(!(*d)->optional())
        {
            writeUnmarshalDataMember(*d, fixId(*d, DotNet::ICloneable, true), needCustomPatcher, patchIter);
        }
    }
    for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
    {
        writeUnmarshalDataMember(*d, fixId(*d, DotNet::ICloneable, true), needCustomPatcher, patchIter);
    }
    _out << nl << "is__.endReadSlice();";
    if(base)
    {
        _out << nl << "base.readImpl__(is__);";
    }
    _out << eb;

    //
    // Write streaming API.
    //
    if(stream)
    {
        _out << sp;
        if(!p->isInterface())
        {
            emitGeneratedCodeAttribute();
        }
        _out << nl << "protected " << qualifier << "void writeImpl__(Ice.OutputStream outS__)";
        _out << sb;
        _out << nl << "outS__.startSlice(ice_staticId(), " << p->compactId() << (!base ? ", true" : ", false") << ");";
        for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
        {
            if(!(*d)->optional())
            {
                writeStreamMarshalDataMember(*d, fixId(*d, DotNet::ICloneable, true));
            }
        }
        for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
        {
            writeStreamMarshalDataMember(*d, fixId(*d, DotNet::ICloneable, true));
        }
        _out << nl << "outS__.endSlice();";
        if(base)
        {
            _out << nl << "base.writeImpl__(outS__);";
        }
        _out << eb;

        _out << sp;
        if(!p->isInterface())
        {
            emitGeneratedCodeAttribute();
        }
        _out << nl << "protected " << qualifier << "void readImpl__(Ice.InputStream inS__)";
        _out << sb;
        _out << nl << "inS__.startSlice();";
        patchIter = 0;
        for(DataMemberList::const_iterator d = members.begin(); d != members.end(); ++d)
        {
            if(!(*d)->optional())
            {
                writeStreamUnmarshalDataMember(*d, fixId(*d, DotNet::ICloneable, true), needCustomPatcher,
                                               patchIter);
            }
        }
        for(DataMemberList::const_iterator d = optionalMembers.begin(); d != optionalMembers.end(); ++d)
        {
            writeStreamUnmarshalDataMember(*d, fixId(*d, DotNet::ICloneable, true), needCustomPatcher,
                                           patchIter);
        }
        _out << nl << "inS__.endSlice();";
        if(base)
        {
            _out << nl << "base.readImpl__(inS__);";
        }
        _out << eb;
    }
    else if (!p->isInterface() && !hasBaseClass)
    {
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "protected virtual void writeImpl__(Ice.OutputStream os__)";
        _out << sb;
        _out << nl << "throw new Ice.MarshalException(\"class was not generated with stream support\");";
        _out << eb;

                _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "protected virtual void readImpl__(Ice.InputStream is__)";
        _out << sb;
        _out << nl << "throw new Ice.MarshalException(\"class was not generated with stream support\");";
        _out << eb;
    }

    if(preserved && !basePreserved)
    {
        _out << sp << nl << "protected Ice.SlicedData slicedData__;";
    }

    _out << sp << nl << "#endregion"; // Marshalling support

    if(!p->isInterface() && !hasBaseClass) {
        _out << sp << nl << "#region ICloneable members";

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public object Clone()";
        _out << sb;
        _out << nl << "return MemberwiseClone();";
        _out << eb;

        _out << sp << nl << "#endregion"; // ICloneable members
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
Slice::CsVisitor::getParams(const OperationPtr& op)
{
    vector<string> params;
    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        string param = getParamAttributes(*q);
        if((*q)->isOutParam())
        {
            param += "out ";
        }
        param += typeToString((*q)->type(), (*q)->optional()) + " " + fixId((*q)->name());
        params.push_back(param);
    }
    return params;
}

vector<string>
Slice::CsVisitor::getParamsAsync(const OperationPtr& op, bool amd)
{
    vector<string> params;

    string name = fixId(op->name());
    ContainerPtr container = op->container();
    ClassDefPtr cl = ClassDefPtr::dynamicCast(container); // Get the class containing the op.
    string scope = fixId(cl->scope());
    if(amd)
    {
        params.push_back(scope + "AMD_" + cl->name() + '_' + op->name() + " cb__");
    }

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if(!(*q)->isOutParam())
        {
            params.push_back(getParamAttributes(*q) + typeToString((*q)->type(), (*q)->optional()) + " " +
                             fixId((*q)->name()));
        }
    }
    return params;
}

vector<string>
Slice::CsVisitor::getParamsAsyncCB(const OperationPtr& op, bool amd, bool outKeyword)
{
    vector<string> params;

    if(amd)
    {
        TypePtr ret = op->returnType();
        if(ret)
        {
            params.push_back(typeToString(ret, op->returnIsOptional()) + " ret__");
        }
    }

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if((*q)->isOutParam())
        {
            if(amd)
            {
                params.push_back(getParamAttributes(*q) + typeToString((*q)->type(), (*q)->optional()) + ' ' +
                                 fixId((*q)->name()));
            }
            else
            {
                string s = getParamAttributes(*q);
                if(outKeyword)
                {
                    s += "out ";
                }
                s += typeToString((*q)->type(), (*q)->optional()) + ' ' + fixId((*q)->name());
                params.push_back(s);
            }
        }
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
Slice::CsVisitor::getArgsAsync(const OperationPtr& op, bool amd)
{
    vector<string> args;

    if(amd)
    {
        args.push_back("cb__");
    }

    ParamDeclList paramList = op->parameters();
    for(ParamDeclList::const_iterator q = paramList.begin(); q != paramList.end(); ++q)
    {
        if(!(*q)->isOutParam())
        {
            args.push_back(fixId((*q)->name()));
        }
    }
    return args;
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
        return fixId(en->scoped()) + "." + fixId((*en->getEnumerators().begin())->name());
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        return st->hasMetaData("clr:class") ? string("null") : "new " + fixId(st->scoped()) + "()";
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
        EnumPtr ep;
        if(bp && bp->kind() == Builtin::KindString)
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

            _out << "\"";                                    // Opening "

            vector<unsigned char> u8buffer;                  // Buffer to convert multibyte characters

            for(size_t i = 0; i < value.size();)
            {
                if(charSet.find(value[i]) == charSet.end())
                {
                    if(static_cast<unsigned char>(value[i]) < 128) // Single byte character
                    {
                        //
                        // Print as unicode if not in basic source character set
                        //
                        _out << u16CodePoint(static_cast<unsigned int>(value[i]));
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
                        writeU8Buffer(u8buffer, _out);
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
                            if(s.size() % 2 != 0 && value[j] == 'U')
                            {
                                _out << s.substr(0, s.size() - 1);
                                i = j + 1;

                                string codepoint = value.substr(j + 1, 8);
                                assert(codepoint.size() ==  8);

                                IceUtil::Int64 v = IceUtilInternal::strToInt64(codepoint.c_str(), 0, 16);


                                //
                                // Unicode character in the range U+10000 to U+10FFFF is not permitted in a character literal
                                // and is represented using a Unicode surrogate pair.
                                //
                                if(v > 0xFFFF)
                                {
                                    unsigned int high = ((static_cast<unsigned int>(v) - 0x10000) / 0x400) + 0xD800;
                                    unsigned int low = ((static_cast<unsigned int>(v) - 0x10000) % 0x400) + 0xDC00;
                                    _out << u16CodePoint(high);
                                    _out << u16CodePoint(low);
                                }
                                else
                                {
                                    _out << "\\U" << codepoint;
                                }

                                i = j + 1 + 8;
                            }
                            else
                            {
                                _out << s;
                                i = j;
                            }
                            continue;
                        }
                        case '"':
                        {
                            _out << "\\";
                            break;
                        }
                    }
                    _out << value[i];                        // Print normally if in basic source character set
                }
                i++;
            }

            //
            // Write any pedding characters in the utf8 buffer
            //
            if(!u8buffer.empty())
            {
                writeU8Buffer(u8buffer, _out);
                u8buffer.clear();
            }

            _out << "\"";                                    // Closing "
        }
        else if(bp && bp->kind() == Builtin::KindLong)
        {
            _out << value << "L";
        }
        else if(bp && bp->kind() == Builtin::KindFloat)
        {
            _out << value << "F";
        }
        else if((ep = EnumPtr::dynamicCast(type)))
        {
            string enumName = fixId(ep->scoped());
            string::size_type colon = value.rfind(':');
            string enumerator;
            if(colon != string::npos)
            {
                enumerator = fixId(value.substr(colon + 1));
            }
            else
            {
                enumerator = fixId(value);
            }
            _out << enumName << '.' << enumerator;
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
                _out << (*p)->name() << "__prop";
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
                _out << nl << fixId((*p)->name(), baseTypes) << " = \"\";";
            }

            StructPtr st = StructPtr::dynamicCast((*p)->type());
            if(st)
            {
                _out << nl << fixId((*p)->name(), baseTypes) << " = new " << typeToString(st, false) << "();";
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
        _out << nl << "/// " << *i;
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
                _out << nl << "/// " << *i;
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

    _out << nl;
}

void
Slice::CsVisitor::writeDocCommentOp(const OperationPtr& p)
{
    ContainerPtr container = p->container();
    ContainedPtr contained = ContainedPtr::dynamicCast(container);
    string deprecateReason = getDeprecateReason(p, contained, "operation");

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

    //
    // Output the leading comment block up until the first <param>, <returns>, or <exception> tag.
    //
    for(StringList::const_iterator i = summaryLines.begin(); i != summaryLines.end(); ++i)
    {
        _out << nl << "/// " << *i;
    }

    bool done = false;
    for(StringList::const_iterator i = remarksLines.begin(); i != remarksLines.end() && !done; ++i)
    {
        if(i->find("<param") != string::npos ||
           i->find("<returns") != string::npos ||
           i->find("<exception") != string::npos)
        {
            done = true;
        }
        else
        {
            _out << nl << "/// " << *i;
        }
    }

    if(!deprecateReason.empty())
    {
        _out << nl << "/// <para>" << deprecateReason << "</para>";
    }

    _out << nl << "/// </summary>";
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
        _out << nl << "/// " << *i;
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
            _out << nl << "/// " << *i;
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
                    _out << nl << "/// " << *i;
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
Slice::CsVisitor::writeDocCommentAMD(const OperationPtr& p, ParamDir paramType, const string& extraParam)
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

    if(paramType == OutParam)
    {
        _out << nl << "/// <summary>";
        _out << nl << "/// ice_response indicates that";
        _out << nl << "/// the operation completed successfully.";
        _out << nl << "/// </summary>";

        //
        // Find the comment for the return value (if any) and rewrite that as a <param> comment.
        //
        static const string returnsTag = "<returns>";
        bool doneReturn = false;
        bool foundReturn = false;
        for(StringList::const_iterator i = remarksLines.begin(); i != remarksLines.end() && !doneReturn; ++i)
        {
            if(!foundReturn)
            {
                string::size_type pos = i->find(returnsTag);
                if(pos != string::npos)
                {
                    pos += returnsTag.size();
                    foundReturn = true;
                    string::size_type endpos = i->find('<', pos);
                    if(endpos != string::npos)
                    {
                        _out << nl << "/// <param name=\"ret__\">(return value) " << i->substr(pos, endpos - pos);
                    }
                    else
                    {
                        _out << nl << "/// <param name=\"ret__\">(return value) " << i->substr(pos);
                    }
                }
            }
            else
            {
                string::size_type pos = i->find('<');
                if(pos != string::npos)
                {
                    _out << nl << "/// " << i->substr(0, pos) << "</param>";
                    doneReturn = true;
                }
                else
                {
                    _out << nl << "/// " << *i;
                }
            }
        }
        if(foundReturn && !doneReturn)
        {
            _out << "</param>";
        }
    }
    else
    {
        //
        // Output the leading comment block up until the first tag.
        //
        _out << nl << "/// <summary>";
        for(StringList::const_iterator i = summaryLines.begin(); i != summaryLines.end(); ++i)
        {
            _out << nl << "/// " << *i;
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
                _out << nl << "/// " << *i;
            }
        }
        _out << nl << "/// </summary>";
    }


    //
    // Write the comments for the parameters.
    //
    writeDocCommentParam(p, paramType, true);

    if(!extraParam.empty())
    {
        _out << nl << "/// " << extraParam;
    }

    if(paramType == InParam)
    {
        if(!deprecateReason.empty())
        {
            _out << nl << "/// <para>" << deprecateReason << "</para>";
        }
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
    // Print a comment for the callback parameter.
    //
    if(paramType == InParam && amd)
    {
        _out << nl << "/// <param name=\"cb__\">The callback object for the operation.</param>";
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
                    if (i == remarksLines.end())
                    {
                        break;
                    }
                    j = i++;
                    while(j != remarksLines.end())
                    {
                        string::size_type endpos = j->find('>');
                        if(endpos == string::npos)
                        {
                            i = j;
                            _out << nl << "/// " << *j++;
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
                bool impl, bool implTie, bool stream)
    : _includePaths(includePaths),
      _stream(stream)
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
    _out << nl << "using _Microsoft = global::Microsoft;";

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

    TypesVisitor typesVisitor(_out, _stream);
    p->visit(&typesVisitor, false);

    //
    // The async delegates are emitted before the proxy definition
    // because the proxy methods need to know the type.
    //
    AsyncDelegateVisitor asyncDelegateVisitor(_out);
    p->visit(&asyncDelegateVisitor, false);

    ProxyVisitor proxyVisitor(_out);
    p->visit(&proxyVisitor, false);

    OpsVisitor opsVisitor(_out);
    p->visit(&opsVisitor, false);

    HelperVisitor helperVisitor(_out, _stream);
    p->visit(&helperVisitor, false);

    DispatcherVisitor dispatcherVisitor(_out, _stream);
    p->visit(&dispatcherVisitor, false);

    AsyncVisitor asyncVisitor(_out);
    p->visit(&asyncVisitor, false);
}

void
Slice::Gen::generateTie(const UnitPtr& p)
{
    TieVisitor tieVisitor(_out);
    p->visit(&tieVisitor, false);
}

void
Slice::Gen::generateImpl(const UnitPtr& p)
{
    ImplVisitor implVisitor(_impl);
    p->visit(&implVisitor, false);
}

void
Slice::Gen::generateImplTie(const UnitPtr& p)
{
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
"// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.\n"
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

Slice::Gen::UnitVisitor::UnitVisitor(IceUtilInternal::Output& out)
    : CsVisitor(out)
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
        _out << nl << "public readonly static string typeId = \"" << p->scoped() << "\";";
        _out << eb;
    }
    return false;
}

Slice::Gen::TypesVisitor::TypesVisitor(IceUtilInternal::Output& out, bool stream)
    : CsVisitor(out), _stream(stream)
{
}

bool
Slice::Gen::TypesVisitor::visitModuleStart(const ModulePtr& p)
{
    DictionaryList dicts;
    if(p->hasOnlyDictionaries(dicts))
    {
        //
        // If this module contains only dictionaries and they
        // all use the new dictionary mapping, we don't need to generate
        // anything for the dictionary types. The early return prevents
        // an empty namespace from being emitted--the namespace will
        // be emitted later by the dictionary helper (which is generated
        // for both old and new dictionaries).
        //
        bool foundOld = false;
        for(DictionaryList::const_iterator i = dicts.begin(); i != dicts.end() && !foundOld; ++i)
        {
            if((*i)->hasMetaData("clr:collection"))
            {
                foundOld = true;
            }
        }
        if(!foundOld)
        {
            return false;
        }
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

    if(!p->isLocal() && _stream)
    {
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public sealed class " << name << "Helper";
        _out << sb;

        _out << sp << nl << "public " << name << "Helper(Ice.InputStream inS__)";
        _out << sb;
        _out << nl << "_in = inS__;";
        _out << nl << "_pp = new IceInternal.ParamPatcher<" << scoped << ">(\"" << p->scoped() << "\");";
        _out << eb;

        _out << sp << nl << "public static void write(Ice.OutputStream outS__, " << fixId(name) << " v__)";
        _out << sb;
        _out << nl << "outS__.writeObject(v__);";
        _out << eb;

        _out << sp << nl << "public void read()";
        _out << sb;
        _out << nl << "_in.readObject(_pp);";
        _out << eb;

        _out << sp << nl << "public " << scoped << " value";
        _out << sb;
        _out << nl << "get";
        _out << sb;
        _out << nl << "return (" << scoped << ")_pp.value;";
        _out << eb;
        _out << eb;

        _out << sp << nl << "public static Ice.OptionalFormat optionalFormat()";
        _out << sb;
        _out << nl << "return " << getOptionalFormat(p->declaration()) << ';';
        _out << eb;

        _out << sp << nl << "private Ice.InputStream _in;";
        _out << nl << "private IceInternal.ParamPatcher<" << scoped << "> _pp;";

        _out << eb;
    }

    StringList baseNames;

    _out << sp;
    emitAttributes(p);
    if(p->isInterface())
    {
        emitComVisibleAttribute();
        emitPartialTypeAttributes();
        _out << nl << "public partial interface " << fixId(name);
        if(!p->isLocal())
        {
            baseNames.push_back("Ice.Object");
            baseNames.push_back(name + "Operations_");
            baseNames.push_back(name + "OperationsNC_");
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

        _out.zeroIndent();
        _out << nl << "#if !SILVERLIGHT";
        _out.restoreIndent();
        _out << nl << "[_System.Serializable]";
        _out.zeroIndent();
        _out << nl << "#endif";
        _out.restoreIndent();

        if(p->allOperations().size() > 0) // See bug 4747
        {
            _out << nl << "[_System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Design\", \"CA1012\")]";
        }
        _out << nl << "public ";
        if(p->allOperations().size() > 0) // Don't use isAbstract() here - see bug 3739
        {
            _out << "abstract ";
        }
        _out << "partial class " << fixId(name);

        if(!hasBaseClass)
        {
            if(!p->isLocal())
            {
                baseNames.push_back("Ice.Object");
            }
        }
        else
        {
            baseNames.push_back(fixId(bases.front()->scoped()));
            bases.pop_front();
        }
        if(p->isAbstract() && !p->isLocal())
        {
            baseNames.push_back(name + "Operations_");
            baseNames.push_back(name + "OperationsNC_");
        }

        for(ClassList::const_iterator q = bases.begin(); q != bases.end(); ++q)
        {
            if((*q)->isAbstract())
            {
                baseNames.push_back(fixId((*q)->scoped()));
            }
        }
    }

    //
    // Check for clr:implements metadata.
    //
    const StringList metaData = p->getMetaData();
    static const string prefix = "clr:implements:";
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
        if(p->hasDataMembers() && !p->hasOperations())
        {
            _out << sp << nl << "#region Slice data members";
        }
        else if(p->hasDataMembers())
        {
            _out << sp << nl << "#region Slice data members and operations";
        }
        else if(p->hasOperations())
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
        if(p->hasDataMembers() && !p->hasOperations())
        {
            _out << sp << nl << "#endregion"; // Slice data members"
        }
        else if(p->hasDataMembers())
        {
            _out << sp << nl << "#endregion"; // Slice data members and operations"
        }
        else if(p->hasOperations())
        {
            _out << sp << nl << "#endregion"; // Slice operations"
        }

        if(!allDataMembers.empty())
        {
            const bool isAbstract = p->isAbstract();
            const bool propertyMapping = p->hasMetaData("clr:property");

            _out << sp << nl << "#region Constructors";

            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << (isAbstract ? "protected " : "public ") << name << spar << epar;
            if(hasBaseClass)
            {
                _out << " : base()";
            }
            _out << sb;
            writeDataMemberInitializers(dataMembers, 0, propertyMapping);
            _out << eb;

            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << (isAbstract ? "protected " : "public ") << name << spar;
            vector<string> paramDecl;
            for(DataMemberList::const_iterator d = allDataMembers.begin(); d != allDataMembers.end(); ++d)
            {
                string memberName = fixId((*d)->name());
                string memberType = typeToString((*d)->type(), (*d)->optional());
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
                    baseParamNames.push_back(fixId((*d)->name()));
                }
                _out << baseParamNames << epar;
            }
            _out << sb;
            for(DataMemberList::const_iterator d = dataMembers.begin(); d != dataMembers.end(); ++d)
            {
                _out << nl << "this.";
                const string paramName = fixId((*d)->name());
                if(propertyMapping)
                {
                    _out << (*d)->name() << "__prop";
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

        writeInheritedOperations(p);
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
        writeDispatchAndMarshalling(p, _stream);
    }

    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitOperation(const OperationPtr& p)
{
    ClassDefPtr classDef = ClassDefPtr::dynamicCast(p->container());
    bool isLocal = classDef->isLocal();
    bool isInterface = classDef->isInterface();

    //
    // Non-local classes and interfaces get the operations from their
    // Operations base interfaces.
    //
    if(isInterface && !isLocal)
    {
        return;
    }

    bool amd = !isLocal && (classDef->hasMetaData("amd") || p->hasMetaData("amd"));

    string name = p->name();
    ParamDeclList paramList = p->parameters();
    vector<string> params;
    vector<string> args;
    string retS;

    if(!amd)
    {
        params = getParams(p);
        args = getArgs(p);
        name = fixId(name, DotNet::ICloneable, true);
        retS = typeToString(p->returnType(), p->returnIsOptional());
    }
    else
    {
        params = getParamsAsync(p, true);
        args = getArgsAsync(p, true);
        retS = "void";
        name = name + "_async";
    }

    _out << sp;
    if(isInterface && isLocal)
    {
        _out << nl;
    }

    writeDocComment(p, getDeprecateReason(p, classDef, "operation"));
    emitAttributes(p);
    emitDeprecate(p, classDef, _out, "operation");
    emitGeneratedCodeAttribute();
    if(!isInterface)
    {
        _out << nl << "public ";
        if(isLocal)
        {
            _out << "abstract ";
        }
    }
    _out << retS << " " << name << spar << params << epar;
    if(isLocal)
    {
        _out << ";";
    }
    else
    {
        _out << sb;
        _out << nl;
        if(!amd && p->returnType())
        {
            _out << "return ";
        }
        _out << "this." << name << spar << args << "Ice.ObjectImpl.defaultCurrent" << epar << ';';
        _out << eb;
    }

    if(!isLocal)
    {
        emitGeneratedCodeAttribute();
        _out << nl << "public abstract " << retS << " " << name
             << spar << params << "Ice.Current current__" << epar << ';';
    }

    if(isLocal && (classDef->hasMetaData("async") || p->hasMetaData("async")))
    {
        vector<string> paramsNewAsync = getParamsAsync(p, false);

        _out << sp;
        emitAttributes(p);
        emitGeneratedCodeAttribute();
        _out << nl;
        if(!isInterface)
        {
            _out << "public abstract ";
        }
        _out << "Ice.AsyncResult";
        if(p->returnsData())
        {
            string clScope = fixId(classDef->scope());
            string cbType = clScope + "Callback_" + classDef->name() + "_" + name;
            _out << '<' << cbType << '>';
        }
        _out << " begin_" << name << spar << paramsNewAsync << epar << ';';

        _out << sp;
        emitAttributes(p);
        emitGeneratedCodeAttribute();
        _out << nl;
        if(!isInterface)
        {
            _out << "public abstract ";
        }
        _out << "Ice.AsyncResult begin_" << name << spar << paramsNewAsync << "Ice.AsyncCallback cb__"
             << "object cookie__" << epar << ';';

        _out << sp;
        emitAttributes(p);
        emitGeneratedCodeAttribute();
        _out << nl;
        if(!isInterface)
        {
            _out << "public abstract ";
        }
        _out << typeToString(p->returnType(), p->returnIsOptional()) << " end_" << name << spar
             << getParamsAsyncCB(p, false, true) << "Ice.AsyncResult r__" << epar << ';';
    }
}

void
Slice::Gen::TypesVisitor::visitSequence(const SequencePtr& p)
{
    //
    // No need to generate anything if the sequence is mapped as an array.
    //
    if(!p->hasMetaData("clr:collection"))
    {
        return;
    }

    //
    // No need to generate anything for custom sequences.
    //
    string prefix = "clr:type:";
    string meta;
    if(p->findMetaData(prefix, meta))
    {
        return;
    }

    //
    // No need to generate anything for serializable sequences.
    //
    prefix = "clr:serializable:";
    if(p->findMetaData(prefix, meta))
    {
        return;
    }

    string name = fixId(p->name());
    string s = typeToString(p->type());

    _out << sp;

    emitDeprecate(p, 0, _out, "type");

    emitAttributes(p);
    emitComVisibleAttribute();
    emitGeneratedCodeAttribute();
    if(isSerializable(p->type()))
    {
        _out.zeroIndent();
        _out << nl << "#if !SILVERLIGHT";
        _out.restoreIndent();
        _out << nl << "[_System.Serializable]";
        _out.zeroIndent();
        _out << nl << "#endif";
        _out.restoreIndent();
    }
    _out << nl << "public class " << name
         << " : IceInternal.CollectionBase<" << s << ">, _System.ICloneable";
    _out << sb;

    _out << sp << nl << "#region Constructors";

    _out << sp << nl << "public " << name << "() : base()";
    _out << sb;
    _out << eb;

    _out << sp << nl << "public " << name << "(int capacity) : base(capacity)";
    _out << sb;
    _out << eb;

    _out << sp << nl << "public " << name << "(" << s << "[] a__) : base(a__)";
    _out << sb;
    _out << eb;

    _out << sp << nl << "public " << name << "(_System.Collections.Generic.IEnumerable<" << s << "> l__) : base(l__)";
    _out << sb;
    _out << eb;

    _out << sp << nl << "#endregion"; // Constructors

    _out << sp << nl << "#region Implicit conversion to generic List";

    _out << sp << nl << "public static implicit operator _System.Collections.Generic.List<"
         << s << ">(" << name << " s__)";
    _out << sb;
    _out << nl << "return s__.list_;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Implicit conversion to generic List

    _out << sp << nl << "#region Operations returning a new sequence";

    _out << sp << nl << "public virtual " << name << " GetRange(int index, int count)";
    _out << sb;
    _out << nl << "_System.Collections.Generic.List<" << s << "> l = list_.GetRange(index, count);";
    _out << nl << name << " r = new " << name << "(l.Count);";
    _out << nl << "r.list_.AddRange(l);";
    _out << nl << "return r;";
    _out << eb;

    _out << sp << nl << "public static " << name << " Repeat(" << s << " value, int count)";
    _out << sb;
    _out << nl << name << " r = new " << name << "(count);";
    _out << nl << "for(int i = 0; i < count; ++i)";
    _out << sb;
    _out << nl << "r.Add(value);";
    _out << eb;
    _out << nl << "return r;";
    _out << eb;

    _out << sp << nl << "public object Clone()";
    _out << sb;
    _out << nl << name << " s = new " << name << "(Count);";
    _out << nl << "s.list_.AddRange(list_);";
    _out << nl << "return s;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Operations returning a new sequence

    _out << eb;
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

    _out.zeroIndent();
    _out << nl << "#if !SILVERLIGHT";
    _out.restoreIndent();
    _out << nl << "[_System.Serializable]";
    _out.zeroIndent();
    _out << nl << "#endif";
    _out.restoreIndent();

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
        _out << sp << nl << "private void initDM__()";
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
        _out << nl << "initDM__();";
    }
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public " << name << "(_System.Exception ex__) : base(ex__)";
    _out << sb;
    if(hasDataMemberInitializers)
    {
        _out << nl << "initDM__();";
    }
    _out << eb;

    _out.zeroIndent();
    _out << nl << "#if !SILVERLIGHT";
    _out.restoreIndent();

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public " << name << "(_System.Runtime.Serialization.SerializationInfo info__, "
         << "_System.Runtime.Serialization.StreamingContext context__) : base(info__, context__)";
    _out << sb;
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        string name = fixId((*q)->name(), DotNet::Exception, false);
        writeSerializeDeserializeCode(_out, (*q)->type(), name, (*q)->optional(), (*q)->tag(), false);
    }
    _out << eb;

    _out.zeroIndent();
    _out << nl << "#endif";
    _out.restoreIndent();

    if(!allDataMembers.empty())
    {
        if(!dataMembers.empty())
        {
            _out << sp << nl << "private void initDM__" << spar << paramDecl << epar;
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
            _out << nl << "initDM__" << spar << paramNames << epar << ';';
        }
        _out << eb;

        vector<string> exceptionParam;
        exceptionParam.push_back("ex__");
        vector<string> exceptionDecl;
        exceptionDecl.push_back("_System.Exception ex__");
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
            _out << nl << "initDM__" << spar << paramNames << epar << ';';
        }
        _out << eb;
    }

    _out << sp << nl << "#endregion"; // Constructors

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public override string ice_name()";
    _out << sb;
    _out << nl << "return \"" << p->scoped().substr(2) << "\";";
    _out << eb;

    _out << sp << nl << "#region Object members";

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public override int GetHashCode()";
    _out << sb;
    if(p->base())
    {
        _out << nl << "int h__ = base.GetHashCode();";
    }
    else
    {
        _out << nl << "int h__ = 5381;";
    }
    _out << nl << "IceInternal.HashUtil.hashAdd(ref h__, \"" << p->scoped() << "\");";
    writeMemberHashCode(dataMembers, DotNet::Exception);
    _out << nl << "return h__;";
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public override bool Equals(object other__)";
    _out << sb;
    _out << nl << "if(other__ == null)";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    _out << nl << "if(object.ReferenceEquals(this, other__))";
    _out << sb;
    _out << nl << "return true;";
    _out << eb;
    _out << nl << name << " o__ = other__ as " << name << ";";
    _out << nl << "if(o__ == null)";
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    if(p->base())
    {
        _out << nl << "if(!base.Equals(other__))";
        _out << sb;
        _out << nl << "return false;";
        _out << eb;
    }
    writeMemberEquals(dataMembers, DotNet::Exception);
    _out << nl << "return true;";
    _out << eb;

    if(!dataMembers.empty())
    {
        _out.zeroIndent();
        _out << nl << "#if !SILVERLIGHT";
        _out.restoreIndent();

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public override void GetObjectData(_System.Runtime.Serialization.SerializationInfo info__, "
             << "_System.Runtime.Serialization.StreamingContext context__)";
        _out << sb;
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            string name = fixId((*q)->name(), DotNet::Exception, false);
            writeSerializeDeserializeCode(_out, (*q)->type(), name, (*q)->optional(), (*q)->tag(), true);
        }
        _out << sp << nl << "base.GetObjectData(info__, context__);";
        _out << eb;

        _out.zeroIndent();
        _out << nl << "#endif";
        _out.restoreIndent();
    }

    _out << sp << nl << "#endregion"; // Object members

    _out << sp << nl << "#region Comparison members";

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public static bool operator==(" << name << " lhs__, " << name << " rhs__)";
    _out << sb;
    _out << nl << "return Equals(lhs__, rhs__);";
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public static bool operator!=(" << name << " lhs__, " << name << " rhs__)";
    _out << sb;
    _out << nl << "return !Equals(lhs__, rhs__);";
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
            _out << nl << "public override void write__(IceInternal.BasicStream os__)";
            _out << sb;
            _out << nl << "os__.startWriteException(slicedData__);";
            _out << nl << "writeImpl__(os__);";
            _out << nl << "os__.endWriteException();";
            _out << eb;

            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << "public override void read__(IceInternal.BasicStream is__)";
            _out << sb;
            _out << nl << "is__.startReadException();";
            _out << nl << "readImpl__(is__);";
            _out << nl << "slicedData__ = is__.endReadException(true);";
            _out << eb;

            if(_stream)
            {
                _out << sp;
                emitGeneratedCodeAttribute();
                _out << nl << "public override void write__(Ice.OutputStream outS__)";
                _out << sb;
                _out << nl << "outS__.startException(slicedData__);";
                _out << nl << "writeImpl__(outS__);";
                _out << nl << "outS__.endException();";
                _out << eb;

                _out << sp;
                emitGeneratedCodeAttribute();
                _out << nl << "public override void read__(Ice.InputStream inS__)";
                _out << sb;
                _out << nl << "inS__.startException();";
                _out << nl << "readImpl__(inS__);";
                _out << nl << "slicedData__ = inS__.endException(true);";
                _out << eb;
            }
        }

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "protected override void writeImpl__(IceInternal.BasicStream os__)";
        _out << sb;
        _out << nl << "os__.startWriteSlice(\"" << scoped << "\", -1, " << (!base ? "true" : "false") << ");";
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            writeMarshalDataMember(*q, fixId((*q)->name(), DotNet::Exception));
        }
        _out << nl << "os__.endWriteSlice();";
        if(base)
        {
            _out << nl << "base.writeImpl__(os__);";
        }
        _out << eb;


        if(classMembers.size() != 0)
        {
            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << "public sealed ";
            if(base && !base->allClassDataMembers().empty())
            {
                _out << "new ";
            }
            _out << "class Patcher__ : IceInternal.Patcher";
            _out << sb;
            _out << sp << nl << "internal Patcher__(string type, Ice.Exception instance";
            if(classMembers.size() > 1)
            {
                _out << ", int member";
            }
            _out << ") : base(type)";
            _out << sb;
            _out << nl << "_instance = (" << name << ")instance;";
            if(classMembers.size() > 1)
            {
                _out << nl << "_member = member;";
            }
            _out << eb;

            _out << sp << nl << "public override void patch(Ice.Object v)";
            _out << sb;
            _out << nl << "try";
            _out << sb;
            if(classMembers.size() > 1)
            {
                _out << nl << "switch(_member)";
                _out << sb;
            }
            int memberCount = 0;
            for(DataMemberList::const_iterator q = classMembers.begin(); q != classMembers.end(); ++q)
            {
                if((*q)->optional())
                {
                    continue;
                }
                if(classMembers.size() > 1)
                {
                    _out.dec();
                    _out << nl << "case " << memberCount << ":";
                    _out.inc();
                }
                string memberName = fixId((*q)->name(), DotNet::Exception);
                string memberType = typeToString((*q)->type(), (*q)->optional());
                if((*q)->optional())
                {
                    if(ClassDeclPtr::dynamicCast((*q)->type()))
                    {
                        _out << nl << "_instance." << memberName << " = new " << memberType << "(("
                             << typeToString((*q)->type()) << ")v);";
                    }
                    else
                    {
                        _out << nl << "_instance." << memberName << " = new " << memberType << "(v);";
                    }
                }
                else
                {
                    if(ClassDeclPtr::dynamicCast((*q)->type()))
                    {
                        _out << nl << "_instance." << memberName << " = (" << memberType << ")v;";
                    }
                    else
                    {
                        _out << nl << "_instance." << memberName << " = v;";
                    }
                }
                if(classMembers.size() > 1)
                {
                    _out << nl << "break;";
                }
                memberCount++;
            }

            for(DataMemberList::const_iterator q = optionalMembers.begin(); q != optionalMembers.end(); ++q)
            {

                TypePtr paramType = (*q)->type();
                BuiltinPtr builtin = BuiltinPtr::dynamicCast(paramType);
                if((builtin && builtin->kind() == Builtin::KindObject) || ClassDeclPtr::dynamicCast(paramType))
                {
                    if(classMembers.size() > 1)
                    {
                        _out.dec();
                        _out << nl << "case " << memberCount << ":";
                        _out.inc();
                    }
                    string memberName = fixId((*q)->name(), DotNet::Exception);
                    string memberType = typeToString((*q)->type(), (*q)->optional());
                    if((*q)->optional())
                    {
                        if(ClassDeclPtr::dynamicCast((*q)->type()))
                        {
                            _out << nl << "_instance." << memberName << " = new " << memberType << "(("
                                << typeToString((*q)->type()) << ")v);";
                        }
                        else
                        {
                            _out << nl << "_instance." << memberName << " = new " << memberType << "(v);";
                        }
                    }
                    else
                    {
                        if(ClassDeclPtr::dynamicCast((*q)->type()))
                        {
                            _out << nl << "_instance." << memberName << " = (" << memberType << ")v;";
                        }
                        else
                        {
                            _out << nl << "_instance." << memberName << " = v;";
                        }
                    }
                    if(classMembers.size() > 1)
                    {
                        _out << nl << "break;";
                    }
                    memberCount++;
                }
            }

            if(classMembers.size() > 1)
            {
                _out << eb;
            }
            _out << eb;
            _out << nl << "catch(_System.InvalidCastException)";
            _out << sb;
            _out << nl << "IceInternal.Ex.throwUOE(type(), v.ice_id());";
            _out << eb;
            _out << eb;

            _out << sp << nl << "private " << name << " _instance;";
            if(classMembers.size() > 1)
            {
                _out << nl << "private int _member;";
            }
            _out << eb;
        }

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "protected override void readImpl__(IceInternal.BasicStream is__)";
        _out << sb;
        _out << nl << "is__.startReadSlice();";

        int patchIter = 0;
        const bool needCustomPatcher = classMembers.size() > 1;
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            writeUnmarshalDataMember(*q, fixId((*q)->name(), DotNet::Exception), needCustomPatcher, patchIter);
        }
        _out << nl << "is__.endReadSlice();";
        if(base)
        {
            _out << nl << "base.readImpl__(is__);";
        }
        _out << eb;

        if(_stream)
        {
            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << "protected override void writeImpl__(Ice.OutputStream outS__)";
            _out << sb;
            _out << nl << "outS__.startSlice(\"" << scoped << "\", -1, " << (!base ? "true" : "false") << ");";
            for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
            {
                writeStreamMarshalDataMember(*q, fixId((*q)->name(), DotNet::Exception));
            }
            _out << nl << "outS__.endSlice();";
            if(base)
            {
                _out << nl << "base.writeImpl__(outS__);";
            }
            _out << eb;

            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << "protected override void readImpl__(Ice.InputStream inS__)";
            _out << sb;
            _out << nl << "inS__.startSlice();";
            patchIter = 0;
            for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
            {
                writeStreamUnmarshalDataMember(*q, fixId((*q)->name(), DotNet::Exception), needCustomPatcher,
                                               patchIter);
            }
            _out << nl << "inS__.endSlice();";
            if(base)
            {
                _out << nl << "base.readImpl__(inS__);";
            }
            _out << eb;
        }

        if((!base || (base && !base->usesClasses(false))) && p->usesClasses(false))
        {
            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << "public override bool usesClasses__()";
            _out << sb;
            _out << nl << "return true;";
            _out << eb;
        }

        if(preserved && !basePreserved)
        {
            _out << sp << nl << "protected Ice.SlicedData slicedData__;";
        }

        _out << sp << nl << "#endregion"; // Marshalling support
    }

    _out << eb;
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    string name = fixId(p->name());

    if(!p->isLocal() && _stream)
    {
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public sealed class " << p->name() << "Helper";
        _out << sb;

        _out << sp << nl << "public static void write(Ice.OutputStream outS__, " << name << " v__)";
        _out << sb;
        _out << nl << "v__.ice_write(outS__);";
        _out << eb;

        _out << sp << nl << "public static " << name << " read(Ice.InputStream inS__)";
        _out << sb;
        _out << nl << name << " v__ = new " << name << "();";
        _out << nl << "v__.ice_read(inS__);";
        _out << nl << "return v__;";
        _out << eb;

        _out << sp << nl << "public static Ice.OptionalFormat optionalFormat()";
        _out << sb;
        _out << nl << "return " << getOptionalFormat(p) << ';';
        _out << eb;

        _out << eb;
    }

    _out << sp;

    emitDeprecate(p, 0, _out, "type");

    emitAttributes(p);
    emitPartialTypeAttributes();

    _out.zeroIndent();
    _out << nl << "#if !SILVERLIGHT";
    _out.restoreIndent();
    _out << nl << "[_System.Serializable]";
    _out.zeroIndent();
    _out << nl << "#endif";
    _out.restoreIndent();

    _out << nl << "public partial " << (isValueType(p) ? "struct" : "class") << ' ' << name;

    StringList baseNames;
    if(!isValueType(p))
    {
        baseNames.push_back("_System.ICloneable");
    }

    //
    // Check for clr:implements metadata.
    //
    const StringList metaData = p->getMetaData();
    static const string prefix = "clr:implements:";
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

    const bool propertyMapping = p->hasMetaData("clr:property");

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
        string memberType = typeToString((*q)->type());
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
            _out << (*q)->name() << "__prop";
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
    _out << nl << "int h__ = 5381;";
    _out << nl << "IceInternal.HashUtil.hashAdd(ref h__, \"" << p->scoped() << "\");";
    writeMemberHashCode(dataMembers, isClass ? DotNet::ICloneable : 0);
    _out << nl << "return h__;";
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public override bool Equals(object other__)";
    _out << sb;
    if(isClass)
    {
        _out << nl << "if(object.ReferenceEquals(this, other__))";
        _out << sb;
        _out << nl << "return true;";
        _out << eb;
    }
    if(isClass)
    {
        _out << nl << "if(other__ == null)";
        _out << sb;
        _out << nl << "return false;";
        _out << eb;
        _out << nl << "if(GetType() != other__.GetType())";
    }
    else
    {
        _out << nl << "if(!(other__ is " << name << "))";
    }
    _out << sb;
    _out << nl << "return false;";
    _out << eb;
    if(!dataMembers.empty())
    {
        _out << nl << name << " o__ = (" << name << ")other__;";
    }
    writeMemberEquals(dataMembers, isClass ? DotNet::ICloneable : 0);
    _out << nl << "return true;";
    _out << eb;

    _out << sp << nl << "#endregion"; // Object members

    _out << sp << nl << "#region Comparison members";

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public static bool operator==(" << name << " lhs__, " << name << " rhs__)";
    _out << sb;
    _out << nl << "return Equals(lhs__, rhs__);";
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public static bool operator!=(" << name << " lhs__, " << name << " rhs__)";
    _out << sb;
    _out << nl << "return !Equals(lhs__, rhs__);";
    _out << eb;

    _out << sp << nl << "#endregion"; // Comparison members

    if(!p->isLocal())
    {
        _out << sp << nl << "#region Marshalling support";

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public void write__(IceInternal.BasicStream os__)";
        _out << sb;
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            writeMarshalDataMember(*q, fixId(*q, isClass ? DotNet::ICloneable : 0));
        }
        _out << eb;

        if(isClass && classMembers.size() != 0)
        {
            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << "public sealed class Patcher__ : IceInternal.Patcher";
            _out << sb;
            _out << sp << nl << "internal Patcher__(string type, " << name << " instance";
            if(classMembers.size() > 1)
            {
                _out << ", int member";
            }
            _out << ") : base(type)";
            _out << sb;
            _out << nl << "_instance = instance;";
            if(classMembers.size() > 1)
            {
                _out << nl << "_member = member;";
            }
            _out << eb;

            _out << sp << nl << "public override void patch(Ice.Object v)";
            _out << sb;
            _out << nl << "try";
            _out << sb;
            if(classMembers.size() > 1)
            {
                _out << nl << "switch(_member)";
                _out << sb;
            }
            int memberCount = 0;
            for(DataMemberList::const_iterator q = classMembers.begin(); q != classMembers.end(); ++q)
            {
                if(classMembers.size() > 1)
                {
                    _out.dec();
                    _out << nl << "case " << memberCount << ":";
                    _out.inc();
                }
                string memberType = typeToString((*q)->type());
                string memberName = fixId((*q)->name(), isClass ? DotNet::ICloneable : 0);
                if(ClassDeclPtr::dynamicCast((*q)->type()))
                {
                    _out << nl << "_instance." << memberName << " = (" << memberType << ")v;";
                }
                else
                {
                    _out << nl << "_instance." << memberName << " = v;";
                }
                if(classMembers.size() > 1)
                {
                    _out << nl << "break;";
                }
                memberCount++;
            }
            if(classMembers.size() > 1)
            {
                _out << eb;
            }
            _out << eb;
            _out << nl << "catch(_System.InvalidCastException)";
            _out << sb;
            _out << nl << "IceInternal.Ex.throwUOE(type(), v.ice_id());";
            _out << eb;
            _out << eb;

            _out << sp << nl << "private " << name;
            _out << " _instance;";
            if(classMembers.size() > 1)
            {
                _out << nl << "private int _member;";
            }
            _out << eb;
        }

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public void read__(IceInternal.BasicStream is__)";
        _out << sb;
        int patchIter = 0;
        const bool needCustomPatcher = classMembers.size() > 1;
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            writeUnmarshalDataMember(*q, fixId(*q, isClass ? DotNet::ICloneable : 0), needCustomPatcher,
                                     patchIter);
        }
        _out << eb;

        if(isClass)
        {
            emitGeneratedCodeAttribute();
            _out << nl << "public static void write__(IceInternal.BasicStream os__, " << name << " v__)";
            _out << sb;
            _out << nl << "if(v__ == null)";
            _out << sb;
            _out << nl << "nullMarshalValue__.write__(os__);";
            _out << eb;
            _out << nl << "else";
            _out << sb;
            _out << nl << "v__.write__(os__);";
            _out << eb;
            _out << eb;

            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << "public static " << name << " read__(IceInternal.BasicStream is__, " << name << " v__)";
            _out << sb;
            _out << nl << "if(v__ == null)";
            _out << sb;
            _out << nl << "v__ = new " << name << "();";
            _out << eb;
            _out << nl << "v__.read__(is__);";
            _out << nl << "return v__;";
            _out << eb;
        }

        if(_stream)
        {
            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << "public void ice_write(Ice.OutputStream outS__)";
            _out << sb;
            for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
            {
                writeStreamMarshalDataMember(*q, fixId(*q, isClass ? DotNet::ICloneable : 0));
            }
            _out << eb;

            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << "public void ice_read(Ice.InputStream inS__)";
            _out << sb;
            patchIter = 0;
            for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
            {
                writeStreamUnmarshalDataMember(*q, fixId(*q, isClass ? DotNet::ICloneable : 0), needCustomPatcher,
                                               patchIter);
            }
            _out << eb;

            if(isClass)
            {
                emitGeneratedCodeAttribute();
                _out << nl << "public static void ice_write(Ice.OutputStream outS__, " << name << " v__)";
                _out << sb;
                _out << nl << "if(v__ == null)";
                _out << sb;
                _out << nl << "nullMarshalValue__.ice_write(outS__);";
                _out << eb;
                _out << nl << "else";
                _out << sb;
                _out << nl << "v__.ice_write(outS__);";
                _out << eb;
                _out << eb;

                _out << sp;
                emitGeneratedCodeAttribute();
                _out << nl << "public static " << name << " ice_read(Ice.InputStream inS__, " << name << " v__)";
                _out << sb;
                _out << nl << "if(v__ == null)";
                _out << sb;
                _out << nl << "v__ = new " << name << "();";
                _out << eb;
                _out << nl << "v__.ice_read(inS__);";
                _out << nl << "return v__;";
                _out << eb;
            }
        }

        if(isClass)
        {
            _out << nl << nl << "private static readonly " << name << " nullMarshalValue__ = new " << name << "();";
        }
        _out << sp << nl << "#endregion"; // Marshalling support
    }

    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitDictionary(const DictionaryPtr& p)
{
    if(!p->hasMetaData("clr:collection"))
    {
        return;
    }

    string name = fixId(p->name());
    string ks = typeToString(p->keyType());
    string vs = typeToString(p->valueType());

    _out << sp;
    emitDeprecate(p, 0, _out, "type");
    emitAttributes(p);
    emitComVisibleAttribute();
    emitGeneratedCodeAttribute();
    if(isSerializable(p->keyType()) && isSerializable(p->valueType()))
    {
        _out.zeroIndent();
        _out << nl << "#if !SILVERLIGHT";
        _out.restoreIndent();
        _out << nl << "[_System.Serializable]";
        _out.zeroIndent();
        _out << nl << "#endif";
        _out.restoreIndent();
    }
    _out << nl << "public class " << name
         << " : IceInternal.DictionaryBase<" << ks << ", " << vs << ">, _System.ICloneable";
    _out << sb;

    _out << sp << nl << "#region " << name << " members";

    _out << sp << nl << "public void AddRange(" << name << " d__)";
    _out << sb;
    _out << nl << "foreach(_System.Collections.Generic.KeyValuePair<" << ks << ", " << vs << "> e in d__.dict_)";
    _out << sb;
    _out << nl << "try";
    _out << sb;
    _out << nl << "dict_.Add(e.Key, e.Value);";
    _out << eb;
    _out << nl << "catch(_System.ArgumentException)";
    _out << sb;
    _out << nl << "// ignore";
    _out << eb;
    _out << eb;
    _out << eb;

    _out << sp << nl << "#endregion"; // <name> members

    _out << sp << nl << "#region ICloneable members";

    _out << sp << nl << "public object Clone()";
    _out << sb;
    _out << nl << name << " d = new " << name << "();";
    _out << nl << "foreach(_System.Collections.Generic.KeyValuePair<" << ks << ", " << vs <<"> e in dict_)";
    _out << sb;
    _out << nl << "d.dict_.Add(e.Key, e.Value);";
    _out << eb;
    _out << nl << "return d;";
    _out << eb;

    _out << sp << nl << "#endregion"; // ICloneable members

    _out << eb;
}

void
Slice::Gen::TypesVisitor::visitEnum(const EnumPtr& p)
{
    string name = fixId(p->name());
    string scoped = fixId(p->scoped());
    EnumeratorList enumerators = p->getEnumerators();
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

    if(!p->isLocal() && _stream)
    {
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public sealed class " << p->name() << "Helper";
        _out << sb;

        _out << sp << nl << "public static void write(Ice.OutputStream outS__, " << scoped << " v__)";
        _out << sb;
        writeMarshalUnmarshalCode(_out, p, "v__", true, true);
        _out << eb;

        _out << sp << nl << "public static " << scoped << " read(Ice.InputStream inS__)";
        _out << sb;
        _out << nl << scoped << " v__;";
        writeMarshalUnmarshalCode(_out, p, "v__", false, true);
        _out << nl << "return v__;";
        _out << eb;

        _out << sp << nl << "public static Ice.OptionalFormat optionalFormat()";
        _out << sb;
        _out << nl << "return " << getOptionalFormat(p) << ';';
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
    if(StructPtr::dynamicCast(cont))
    {
        isValue = isValueType(StructPtr::dynamicCast(cont));
        if(!isValue)
        {
            baseTypes = DotNet::ICloneable;
        }
        if(cont->hasMetaData("clr:property"))
        {
            isProperty = true;
        }
    }
    else if(ExceptionPtr::dynamicCast(cont))
    {
        baseTypes = DotNet::Exception;
    }
    else if(ClassDefPtr::dynamicCast(cont))
    {
        baseTypes = DotNet::ICloneable;
        isClass = true;
        if(cont->hasMetaData("clr:property"))
        {
            isProperty = true;
        }
        isProtected = cont->hasMetaData("protected") || p->hasMetaData("protected");
    }

    _out << sp;

    emitDeprecate(p, cont, _out, "member");

    string type = typeToString(p->type(), isOptional);
    string propertyName = fixId(p->name(), baseTypes, isClass);
    string dataMemberName;
    if(isProperty)
    {
        dataMemberName = p->name() + "__prop";
    }
    else
    {
        dataMemberName = propertyName;
    }

    if(!isSerializable(p->type()))
    {
        _out.zeroIndent();
        _out << nl << "#if !SILVERLIGHT";
        _out.restoreIndent();
        _out << nl << "[_System.NonSerialized]";
        _out.zeroIndent();
        _out << nl << "#endif";
        _out.restoreIndent();
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
        _out << nl << "IceInternal.HashUtil.hashAdd(ref h__, " << fixId((*q)->name(), baseTypes);
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
            _out << nl << "if(" << memberName << " == null)";
            _out << sb;
            _out << nl << "if(o__." << memberName << " != null)";
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
                bool isSerializable = seq->findMetaData("clr:serializable:", meta);
                bool isGeneric = seq->findMetaData("clr:generic:", meta);
                bool isArray = !isSerializable && !isGeneric && !seq->hasMetaData("clr:collection");
                if(isArray)
                {
                    //
                    // Equals() for native arrays does not have value semantics.
                    //
                    _out << nl << "if(!IceUtilInternal.Arrays.Equals(" << memberName << ", o__." << memberName << "))";
                }
                else if(isGeneric)
                {
                    //
                    // Equals() for generic types does not have value semantics.
                    //
                    _out << nl << "if(!IceUtilInternal.Collections.SequenceEquals(" << memberName << ", o__."
                         << memberName << "))";
                }
                else
                {
                    //
                    // Equals() for CollectionBase has value semantics.
                    //
                    _out << nl << "if(!" << memberName << ".Equals(o__." << memberName << "))";
                }
            }
            else
            {
                DictionaryPtr dict = DictionaryPtr::dynamicCast(memberType);
                if(dict)
                {
                    if(dict->hasMetaData("clr:collection"))
                    {
                        //
                        // Equals() for DictionaryBase has value semantics.
                        //
                        _out << nl << "if(!" << memberName << ".Equals(o__." << memberName << "))";
                    }
                    else
                    {
                        //
                        // Equals() for generic types does not have value semantics.
                        //
                        _out << nl << "if(!IceUtilInternal.Collections.DictionaryEquals(" << memberName << ", o__."
                             << memberName << "))";
                    }
                }
                else
                {
                    _out << nl << "if(!" << memberName << ".Equals(o__." << memberName << "))";
                }
            }
            _out << sb;
            _out << nl << "return false;";
            _out << eb;
            _out << eb;
        }
        else
        {
            _out << nl << "if(!" << memberName << ".Equals(o__." << memberName << "))";
            _out << sb;
            _out << nl << "return false;";
            _out << eb;
        }
    }
}

Slice::Gen::ProxyVisitor::ProxyVisitor(IceUtilInternal::Output& out)
    : CsVisitor(out)
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
    if(p->isLocal())
    {
        return false;
    }

    string name = p->name();
    ClassList bases = p->bases();

    _out << sp;
    writeDocComment(p, getDeprecateReason(p, 0, p->isInterface() ? "interface" : "class"));
    emitGeneratedCodeAttribute();
    _out << nl << "public interface " << name << "Prx : ";
    if(bases.empty())
    {
        _out << "Ice.ObjectPrx";
    }
    else
    {
        ClassList::const_iterator q = bases.begin();
        while(q != bases.end())
        {
            _out << fixId((*q)->scoped() + "Prx");
            if(++q != bases.end())
            {
                _out << ", ";
            }
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
    vector<string> params = getParams(p);
    ParamDeclList paramList = p->parameters();
    string retS = typeToString(p->returnType(), p->returnIsOptional());

    string deprecateReason = getDeprecateReason(p, cl, "operation");

    //
    // Write two versions of the operation - with and without a context parameter.
    //
    _out << sp;
    writeDocComment(p, deprecateReason);
    if(!deprecateReason.empty())
    {
        _out << nl << "[_System.Obsolete(\"" << deprecateReason << "\")]";
    }
    _out << nl << retS << " " << name << spar << params << epar << ';';

    _out << sp;
    writeDocComment(p, deprecateReason,
        "<param name=\"ctx__\">The Context map to send with the invocation.</param>");
    if(!deprecateReason.empty())
    {
        _out << nl << "[_System.Obsolete(\"" << deprecateReason << "\")]";
    }
    _out << nl << retS << " " << name << spar << params
         << "_System.Collections.Generic.Dictionary<string, string> ctx__" << epar << ';';

    //
    // Write the operations for the new async mapping.
    //
    vector<string> paramsNewAsync = getParamsAsync(p, false);
    string clScope = fixId(cl->scope());
    string delType = clScope + "Callback_" + cl->name() + "_" + p->name();

    _out << sp;
    writeDocCommentAMI(p, InParam, deprecateReason);
    if(!deprecateReason.empty())
    {
        _out << nl << "[_System.Obsolete(\"" << deprecateReason << "\")]";
    }
    _out << nl << "Ice.AsyncResult<" << delType << "> begin_" << p->name() << spar << paramsNewAsync << epar << ';';

    _out << sp;
    writeDocCommentAMI(p, InParam, deprecateReason,
        "<param name=\"ctx__\">The Context map to send with the invocation.</param>");
    if(!deprecateReason.empty())
    {
        _out << nl << "[_System.Obsolete(\"" << deprecateReason << "\")]";
    }
    _out << nl << "Ice.AsyncResult<" << delType << "> begin_" << p->name() << spar << paramsNewAsync
         << "_System.Collections.Generic.Dictionary<string, string> ctx__" << epar << ';';

    //
    // Type-unsafe begin_ methods.
    //
    _out << sp;
    writeDocCommentAMI(p, InParam, deprecateReason,
        "<param name=\"cb__\">Asynchronous callback invoked when the operation completes.</param>",
        "<param name=\"cookie__\">Application data to store in the asynchronous result object.</param>");
    if(!deprecateReason.empty())
    {
        _out << nl << "[_System.Obsolete(\"" << deprecateReason << "\")]";
    }
    _out << nl << "Ice.AsyncResult begin_" << p->name() << spar << paramsNewAsync << "Ice.AsyncCallback cb__"
         << "object cookie__" << epar << ';';

    _out << sp;
    writeDocCommentAMI(p, InParam, deprecateReason,
        "<param name=\"ctx__\">The Context map to send with the invocation.</param>",
        "<param name=\"cb__\">Asynchronous callback invoked when the operation completes.</param>",
        "<param name=\"cookie__\">Application data to store in the asynchronous result object.</param>");
    if(!deprecateReason.empty())
    {
        _out << nl << "[_System.Obsolete(\"" << deprecateReason << "\")]";
    }
    _out << nl << "Ice.AsyncResult begin_" << p->name() << spar << paramsNewAsync
         << "_System.Collections.Generic.Dictionary<string, string> ctx__" << "Ice.AsyncCallback cb__"
         << "object cookie__" << epar << ';';

    //
    // end_ method.
    //
    _out << sp;
    writeDocCommentAMI(p, OutParam, deprecateReason,
        "<param name=\"r__\">The asynchronous result object for the invocation.</param>");
    if(!deprecateReason.empty())
    {
        _out << nl << "[_System.Obsolete(\"" << deprecateReason << "\")]";
    }
    _out << nl << retS << " end_" << p->name() << spar << getParamsAsyncCB(p, false, true) << "Ice.AsyncResult r__"
         << epar << ';';
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
    // We also generate delegates for local twoway-style operations marked with "async" metadata.
    //
    if(cl->isLocal() && (!(cl->hasMetaData("async") || p->hasMetaData("async")) || !p->returnsData()))
    {
        return;
    }

    vector<string> paramDeclAMI = getParamsAsyncCB(p, false, false);
    string retS = typeToString(p->returnType(), p->returnIsOptional());
    string delName = "Callback_" + cl->name() + "_" + p->name();

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public delegate void " << delName << spar;
    if(p->returnType())
    {
        _out << retS + " ret__";
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
    if(p->isLocal())
    {
        return; // Local interfaces and classes don't have an Operations interface.
    }

    string name = p->name();
    string scoped = fixId(p->scoped());
    ClassList bases = p->bases();
    string opIntfName = "Operations";
    if(noCurrent)
    {
        opIntfName += "NC";
    }

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
                if(!first)
                {
                    _out << ", ";
                }
                else
                {
                    first = false;
                }
                string s = (*q)->scoped();
                s += "Operations";
                if(noCurrent)
                {
                    s += "NC";
                }
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
        string opname = amd ? (op->name() + "_async") : fixId(op->name(), DotNet::ICloneable, true);

        TypePtr ret;
        vector<string> params;

        if(amd)
        {
            params = getParamsAsync(op, true);
        }
        else
        {
            params = getParams(op);
            ret = op->returnType();
        }

        _out << sp;

        string deprecateReason = getDeprecateReason(*r, p, "operation");
        string extraCurrent;
        if(!noCurrent && !p->isLocal())
        {
            extraCurrent = "<param name=\"current__\">The Current object for the invocation.</param>";
        }
        if(amd)
        {
            writeDocCommentAMD(*r, InParam, extraCurrent);
        }
        else
        {
            writeDocComment(*r, deprecateReason, extraCurrent);
        }

        emitDeprecate(op, p, _out, "operation");

        emitAttributes(op);
        string retS = typeToString(ret, op->returnIsOptional());
        _out << nl << retS << ' ' << opname << spar << params;
        if(!noCurrent && !p->isLocal())
        {
            _out << "Ice.Current current__";
        }
        _out << epar << ';';
    }

    _out << eb;
}

Slice::Gen::HelperVisitor::HelperVisitor(IceUtilInternal::Output& out, bool stream)
    : CsVisitor(out), _stream(stream)
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
    if(p->isLocal())
    {
        return false;
    }

    string name = p->name();
    ClassList bases = p->bases();

    _out << sp;
    emitComVisibleAttribute();
    emitGeneratedCodeAttribute();
    _out << nl << "public sealed class " << name << "PrxHelper : Ice.ObjectPrxHelperBase, " << name << "Prx";
    _out << sb;

    OperationList ops = p->allOperations();

    if(!ops.empty())
    {
        _out << sp << nl << "#region Synchronous operations";
    }

    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;
        string opName = fixId(op->name(), DotNet::ICloneable, true);
        TypePtr ret = op->returnType();
        string retS = typeToString(ret, op->returnIsOptional());

        vector<string> params = getParams(op);
        vector<string> args = getArgs(op);

        string deprecateReason = getDeprecateReason(op, p, "operation");

        ParamDeclList paramList = op->parameters();
        ParamDeclList inParams;
        ParamDeclList outParams;

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

        _out << sp;
        _out << nl << "public " << retS << " " << opName << spar << params << epar;
        _out << sb;
        _out << nl;
        if(ret)
        {
            _out << "return ";
        }
        _out << "this." << opName << spar << args << "null" << "false" << epar << ';';
        _out << eb;

        _out << sp;
        _out << nl << "public " << retS << " " << opName << spar << params
             << "_System.Collections.Generic.Dictionary<string, string> ctx__" << epar;
        _out << sb;
        _out << nl;
        if(ret)
        {
            _out << "return ";
        }
        _out << "this." << opName << spar << args << "ctx__" << "true" << epar << ';';
        _out << eb;

        _out << sp << nl << "private " << retS << " " << opName << spar << params
             << "_System.Collections.Generic.Dictionary<string, string> context__"
             << "bool explicitCtx__" << epar;
        _out << sb;

        if(op->returnsData())
        {
            _out << nl << "checkTwowayOnly__(__" << op->name() << "_name);";
        }

        if(ret)
        {
            _out << nl << "return ";
        }
        else
        {
            _out << nl;
        }

        _out << "end_" << op->name() << "(";
        if(!outParams.empty()) {
            for(ParamDeclList::const_iterator p = outParams.begin(); p != outParams.end(); ++p) {
                _out << "out " << fixId((*p)->name()) << ", ";
            }
        }
        _out << "begin_" << op->name() << "(";
        if(!inParams.empty())
        {
            for(ParamDeclList::const_iterator p = inParams.begin(); p != inParams.end(); ++p) {
                _out << fixId((*p)->name()) << ", ";
            }
        }
        _out << "context__, explicitCtx__, true, null, null));";
        _out << eb;
    }

    if(!ops.empty())
    {
        _out << sp << nl << "#endregion"; // Synchronous operations
    }

    //
    // New AMI mapping.
    //
    _out << sp << nl << "#region Asynchronous operations";
    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        OperationPtr op = *r;

        ClassDefPtr containingClass = ClassDefPtr::dynamicCast(op->container());
        vector<string> paramsAMI = getParamsAsync(op, false);
        vector<string> argsAMI = getArgsAsync(op, false);

        string opName = op->name();

        ParamDeclList paramList = op->parameters();
        ParamDeclList inParams;
        ParamDeclList outParams;

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
        // Write the begin_ methods.
        //
        ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());
        string clScope = fixId(cl->scope());
        string delType = clScope + "Callback_" + cl->name() + "_" + op->name();

        _out << sp;
        _out << nl << "public Ice.AsyncResult<" << delType << "> begin_" << opName << spar << paramsAMI << epar;
        _out << sb;
        _out << nl << "return begin_" << opName << spar << argsAMI << "null" << "false" << "false" << "null" << "null"
             << epar << ';';
        _out << eb;

        _out << sp;
        _out << nl << "public Ice.AsyncResult<" << delType << "> begin_" << opName << spar << paramsAMI
             << "_System.Collections.Generic.Dictionary<string, string> ctx__" << epar;
        _out << sb;
        _out << nl << "return begin_" << opName << spar << argsAMI << "ctx__" << "true" << "false" << "null" << "null"
             << epar << ';';
        _out << eb;

        _out << sp;
        _out << nl << "public Ice.AsyncResult begin_" << opName << spar << paramsAMI
             << "Ice.AsyncCallback cb__" << "object cookie__" << epar;
        _out << sb;
        _out << nl << "return begin_" << opName << spar << argsAMI << "null" << "false" << "false" << "cb__"
             << "cookie__" << epar << ';';
        _out << eb;

        _out << sp;
        _out << nl << "public Ice.AsyncResult begin_" << opName << spar << paramsAMI
             << "_System.Collections.Generic.Dictionary<string, string> ctx__" << "Ice.AsyncCallback cb__"
             << "object cookie__" << epar;
        _out << sb;
        _out << nl << "return begin_" << opName << spar << argsAMI << "ctx__" << "true" << "false" << "cb__"
             << "cookie__" << epar << ';';
        _out << eb;

        //
        // Write the end_ method.
        //
        string flatName = "__" + opName + "_name";
        _out << sp << nl << "private const string " << flatName << " = \"" << op->name() << "\";";

        TypePtr ret = op->returnType();
        string retS = typeToString(ret, op->returnIsOptional());
        _out << sp << nl << "public " << retS << " end_" << opName << spar << getParamsAsyncCB(op, false, true)
             << "Ice.AsyncResult r__" << epar;
        _out << sb;
        if(op->returnsData())
        {
            _out << nl << "IceInternal.OutgoingAsync outAsync__ = IceInternal.OutgoingAsync.check(r__, this, "
                 << flatName << ");";

            _out << nl << "try";
            _out << sb;

            _out << nl << "if(!outAsync__.wait())";
            _out << sb;

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

            _out << nl << "try";
            _out << sb;
            _out << nl << "outAsync__.throwUserException();";
            _out << eb;
            for(ExceptionList::const_iterator r = throws.begin(); r != throws.end(); ++r)
            {
                _out << nl << "catch(" << fixId((*r)->scoped()) << ")";
                _out << sb;
                _out << nl << "throw;";
                _out << eb;
            }
            _out << nl << "catch(Ice.UserException ex__)";
            _out << sb;
            _out << nl << "throw new Ice.UnknownUserException(ex__.ice_name(), ex__);";
            _out << eb;
            _out << eb;

            if(ret || !outParams.empty())
            {
                if(ret)
                {
                    _out << nl << retS << " ret__;";
                }
                _out << nl << "IceInternal.BasicStream is__ = outAsync__.startReadParams();";
                for(ParamDeclList::const_iterator pli = outParams.begin(); pli != outParams.end(); ++pli)
                {
                    string param = fixId((*pli)->name());
                    string typeS = typeToString((*pli)->type(), (*pli)->optional());
                    const bool isClass = isClassType((*pli)->type());

                    if(!(*pli)->optional() && !isClass)
                    {
                        StructPtr st = StructPtr::dynamicCast((*pli)->type());
                        if(st)
                        {
                            if(isValueType(st))
                            {
                                _out << nl << param << " = new " << typeS << "();";
                            }
                            else
                            {
                                _out << nl << param << " = null;";
                            }
                        }
                    }
                }
                if(ret)
                {
                    string typeS = typeToString(ret, op->returnIsOptional());
                    const bool isClass = isClassType(ret);

                    if(!op->returnIsOptional() && !isClass)
                    {
                        StructPtr st = StructPtr::dynamicCast(ret);
                        if(st)
                        {
                            if(isValueType(st))
                            {
                                _out << nl << "ret__ = new " << typeS << "();";
                            }
                            else
                            {
                                _out << nl << "ret__ = null;";
                            }
                        }
                    }
                }
                writeMarshalUnmarshalParams(outParams, op, false);
                if(op->returnsClasses(false))
                {
                    _out << nl << "is__.readPendingObjects();";
                }
                _out << nl << "outAsync__.endReadParams();";
                writePostUnmarshalParams(outParams, op);
                if(ret)
                {
                    _out << nl << "return ret__;";
                }
            }
            else
            {
                _out << nl << "outAsync__.readEmptyParams();";
            }

            _out << eb;
            _out << nl << "finally";
            _out << sb;
            _out << nl << "outAsync__.cacheMessageBuffers();";
            _out << eb;
        }
        else
        {
            _out << nl << "end__(r__, " << flatName << ");";
        }
        _out << eb;

        //
        // Write the common begin_ implementation.
        //
        _out << sp;
        _out << nl << "private Ice.AsyncResult<" << delType << "> begin_" << opName << spar << paramsAMI
             << "_System.Collections.Generic.Dictionary<string, string> ctx__"
             << "bool explicitContext__" << "bool synchronous__" << "Ice.AsyncCallback cb__" << "object cookie__"
             << epar;
        _out << sb;
        if(op->returnsData())
        {
            _out << nl << "checkAsyncTwowayOnly__(" << flatName << ");";
            _out << nl << "IceInternal.TwowayOutgoingAsync<" << delType << "> result__ = "
                 << " getTwowayOutgoingAsync<" << delType << ">(" << flatName << ", " << op->name()
                 << "_completed__";
        }
        else
        {
            _out << nl << "IceInternal.OnewayOutgoingAsync<" << delType << "> result__ = "
                 << "getOnewayOutgoingAsync<" << delType << ">(" << flatName << ", " << op->name()
                 << "_completed__";
        }
        _out << ", cookie__);";
        _out << nl << "if(cb__ != null)";
        _out << sb;
        _out << nl << "result__.whenCompletedWithAsyncCallback(cb__);";
        _out << eb;
        _out << nl << "try";
        _out << sb;
        _out << nl << "result__.prepare(" << flatName << ", "
             << sliceModeToIceMode(op->sendMode()) << ", ctx__, explicitContext__, synchronous__);";

        if(!inParams.empty())
        {
            _out << nl << "IceInternal.BasicStream os__ = result__.startWriteParams("
                 << opFormatTypeToString(op) << ");";
            writeMarshalUnmarshalParams(inParams, 0, true);
            if(op->sendsClasses(false))
            {
                _out << nl << "os__.writePendingObjects();";
            }
            _out << nl << "result__.endWriteParams();";
        }
        else
        {
            _out << nl << "result__.writeEmptyParams();";
        }

        _out << nl << "result__.invoke();";
        _out << eb;
        _out << nl << "catch(Ice.Exception ex__)";
        _out << sb;
        _out << nl << "result__.abort(ex__);";
        _out << eb;
        _out << nl << "return result__;";
        _out << eb;

        //
        // Write the completed callback.
        //
        if(op->returnsData())
        {
            _out << sp << nl << "private void " << op->name()
                 << "_completed__(Ice.AsyncResult r__, " << delType << " cb__, Ice.ExceptionCallback excb__)";
            _out << sb;
            for(ParamDeclList::const_iterator pli = outParams.begin(); pli != outParams.end(); ++pli)
            {
                _out << nl << typeToString((*pli)->type(), (*pli)->optional()) << ' ' << fixId((*pli)->name()) << ';';
            }
            if(ret)
            {
                _out << nl << retS << " ret__;";
            }
            _out << nl << "try";
            _out << sb;
            _out << nl;
            if(ret)
            {
                _out << "ret__ = ";
            }
            _out << "end_" << op->name() << spar;
            for(ParamDeclList::const_iterator pli = outParams.begin(); pli != outParams.end(); ++pli)
            {
                _out << "out " + fixId((*pli)->name());
            }
            _out << "r__" << epar << ';';
            _out << eb;
            _out << nl << "catch(Ice.Exception ex__)";
            _out << sb;
            _out << nl << "if(excb__ != null)";
            _out << sb;
            _out << nl << "excb__(ex__);";
            _out << eb;
            _out << nl << "return;";
            _out << eb;
            _out << nl << "if(cb__ != null)";
            _out << sb;
            _out << nl << "cb__" << spar;
            if(ret)
            {
                _out << "ret__";
            }
            for(ParamDeclList::const_iterator pli = outParams.begin(); pli != outParams.end(); ++pli)
            {
                _out << fixId((*pli)->name());
            }
            _out << epar << ';';
            _out << eb;
            _out << eb;
        }
        else
        {
            _out << sp << nl << "private void " << op->name() << "_completed__(" << delType << " cb__)";
            _out << sb;
            _out << nl << "if(cb__ != null)";
            _out << sb;
            _out << nl << "cb__();";
            _out << eb;
            _out << eb;
        }
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
    _out << nl << "h.copyFrom__(b);";
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
    _out << nl << "h.copyFrom__(b);";
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
    _out << nl << "h.copyFrom__(bb);";
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
    _out << nl << "h.copyFrom__(bb);";
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
    _out << nl << "h.copyFrom__(b);";
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
    _out << nl << "h.copyFrom__(bb);";
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

    _out << sp << nl << "public static readonly string[] ids__ =";
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
    _out << nl << "return ids__[" << scopedPos << "];";
    _out << eb;

    _out << sp << nl << "#endregion"; // Checked and unchecked cast operations

    _out << sp << nl << "#region Marshaling support";

    _out << sp << nl << "public static void write__(IceInternal.BasicStream os__, " << name << "Prx v__)";
    _out << sb;
    _out << nl << "os__.writeProxy(v__);";
    _out << eb;

    _out << sp << nl << "public static " << name << "Prx read__(IceInternal.BasicStream is__)";
    _out << sb;
    _out << nl << "Ice.ObjectPrx proxy = is__.readProxy();";
    _out << nl << "if(proxy != null)";
    _out << sb;
    _out << nl << name << "PrxHelper result = new " << name << "PrxHelper();";
    _out << nl << "result.copyFrom__(proxy);";
    _out << nl << "return result;";
    _out << eb;
    _out << nl << "return null;";
    _out << eb;

    if(_stream)
    {
        _out << sp << nl << "public static void write(Ice.OutputStream outS__, " << name << "Prx v__)";
        _out << sb;
        _out << nl << "outS__.writeProxy(v__);";
        _out << eb;

        _out << sp << nl << "public static " << name << "Prx read(Ice.InputStream inS__)";
        _out << sb;
        _out << nl << "Ice.ObjectPrx proxy = inS__.readProxy();";
        _out << nl << "if(proxy != null)";
        _out << sb;
        _out << nl << name << "PrxHelper result = new " << name << "PrxHelper();";
        _out << nl << "result.copyFrom__(proxy);";
        _out << nl << "return result;";
        _out << eb;
        _out << nl << "return null;";
        _out << eb;

        _out << sp << nl << "public static Ice.OptionalFormat optionalFormat()";
        _out << sb;
        _out << nl << "return Ice.OptionalFormat.FSize;";
        _out << eb;

    }

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

    _out << sp << nl << "public static void write(IceInternal.BasicStream os__, " << typeS << " v__)";
    _out << sb;
    writeSequenceMarshalUnmarshalCode(_out, p, "v__", true, false, false);
    _out << eb;

    _out << sp << nl << "public static " << typeS << " read(IceInternal.BasicStream is__)";
    _out << sb;
    _out << nl << typeS << " v__;";
    writeSequenceMarshalUnmarshalCode(_out, p, "v__", false, false, false);
    _out << nl << "return v__;";
    _out << eb;

    if(_stream)
    {
        _out << sp << nl << "public static void write(Ice.OutputStream outS__, " << typeS << " v__)";
        _out << sb;
        writeSequenceMarshalUnmarshalCode(_out, p, "v__", true, true, false);
        _out << eb;

        _out << sp << nl << "public static " << typeS << " read(Ice.InputStream inS__)";
        _out << sb;
        _out << nl << typeS << " v__;";
        writeSequenceMarshalUnmarshalCode(_out, p, "v__", false, true, false);
        _out << nl << "return v__;";
        _out << eb;

        _out << sp << nl << "public static Ice.OptionalFormat optionalFormat()";
        _out << sb;
        _out << nl << "return " << getOptionalFormat(p) << ';';
        _out << eb;
    }

    _out << eb;

    string prefix = "clr:generic:";
    string meta;
    if(p->findMetaData(prefix, meta))
    {
        string type = meta.substr(prefix.size());
        if(type == "List" || type == "LinkedList" || type == "Queue" || type == "Stack")
        {
            return;
        }
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(p->type());
        bool isClass = (builtin && builtin->kind() == Builtin::KindObject)
                        || ClassDeclPtr::dynamicCast(p->type());

        if(!isClass)
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
    bool isNewMapping = !p->hasMetaData("clr:collection");

    string prefix = "clr:generic:";
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
    string name = isNewMapping
                        ? "_System.Collections.Generic." + genericType + "<" + keyS + ", " + valueS + ">"
                        : fixId(p->name());

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public sealed class " << p->name() << "Helper";
    _out << sb;

    _out << sp << nl << "public static void write(";
    _out.useCurrentPosAsIndent();
    _out << "IceInternal.BasicStream os__,";
    _out << nl << name << " v__)";
    _out.restoreIndent();
    _out << sb;
    _out << nl << "if(v__ == null)";
    _out << sb;
    _out << nl << "os__.writeSize(0);";
    _out << eb;
    _out << nl << "else";
    _out << sb;
    _out << nl << "os__.writeSize(v__.Count);";
    _out << nl << "foreach(_System.Collections.";
    if(isNewMapping)
    {
        _out << "Generic.KeyValuePair<" << keyS << ", " << valueS << ">";
    }
    else
    {
        _out << "DictionaryEntry";
    }
    _out << " e__ in v__)";
    _out << sb;
    string keyArg = isNewMapping ? string("e__.Key") : "((" + keyS + ")e__.Key)";
    writeMarshalUnmarshalCode(_out, key, keyArg, true, false);
    string valueArg = isNewMapping ? string("e__.Value") : "((" + valueS + ")e__.Value)";
    writeMarshalUnmarshalCode(_out, value, valueArg, true, false);
    _out << eb;
    _out << eb;
    _out << eb;

    const bool hasClassValue = isClassType(value);

    if(hasClassValue)
    {
        _out << sp << nl << "public sealed class Patcher__ : IceInternal.Patcher";
        _out << sb;
        _out << sp << nl << "internal Patcher__(string type, " << name << " m, " << keyS << " key) : base(type)";
        _out << sb;
        _out << nl << "_m = m;";
        _out << nl << "_key = key;";
        _out << eb;

        _out << sp << nl << "public override void patch(Ice.Object v)";
        _out << sb;
        if(ClassDeclPtr::dynamicCast(value))
        {
            _out << nl << "try";
            _out << sb;
            _out << nl << "_m[_key] = (" << valueS << ")v;";
            _out << eb;
            _out << nl << "catch(_System.InvalidCastException)";
            _out << sb;
            _out << nl << "IceInternal.Ex.throwUOE(type(), v.ice_id());";
            _out << eb;
        }
        else
        {
            _out << nl << "_m[_key] = v;";
        }
        _out << eb;

        _out << sp << nl << "private " << name << " _m;";
        _out << nl << "private " << keyS << " _key;";
        _out << eb;
    }

    _out << sp << nl << "public static " << name << " read(IceInternal.BasicStream is__)";
    _out << sb;
    _out << nl << "int sz__ = is__.readSize();";
    _out << nl << name << " r__ = new " << name << "();";
    _out << nl << "for(int i__ = 0; i__ < sz__; ++i__)";
    _out << sb;
    _out << nl << keyS << " k__;";
    StructPtr st = StructPtr::dynamicCast(key);
    if(st)
    {
        if(isValueType(st))
        {
            _out << nl << "k__ = new " << typeToString(key) << "();";
        }
        else
        {
            _out << nl << "k__ = null;";
        }
    }
    writeMarshalUnmarshalCode(_out, key, "k__", false, false);

    string patcher;
    if(hasClassValue)
    {
        patcher = "new Patcher__(" + getStaticId(value) + ", r__, k__)";
    }
    else
    {
        _out << nl << valueS << " v__;";

        StructPtr st = StructPtr::dynamicCast(value);
        if(st)
        {
            if(isValueType(st))
            {
                _out << nl << "v__ = new " << typeToString(value) << "();";
            }
            else
            {
                _out << nl << "v__ = null;";
            }
        }
    }
    writeMarshalUnmarshalCode(_out, value, hasClassValue ? patcher : "v__", false, false);
    if(!hasClassValue)
    {
        _out << nl << "r__[k__] = v__;";
    }
    _out << eb;
    _out << nl << "return r__;";
    _out << eb;

    if(_stream)
    {
        _out << sp << nl << "public static void write(Ice.OutputStream outS__, " << name << " v__)";
        _out << sb;
        _out << nl << "if(v__ == null)";
        _out << sb;
        _out << nl << "outS__.writeSize(0);";
        _out << eb;
        _out << nl << "else";
        _out << sb;
        _out << nl << "outS__.writeSize(v__.Count);";
        _out << nl << "foreach(_System.Collections.";
        if(isNewMapping)
        {
            _out << nl << "Generic.KeyValuePair<" << keyS << ", " << valueS << ">";
        }
        else
        {
            _out << nl << "DictionaryEntry";
        }
        _out << " e__ in v__)";
        _out << sb;
        writeMarshalUnmarshalCode(_out, key, keyArg, true, true);
        writeMarshalUnmarshalCode(_out, value, valueArg, true, true);
        _out << eb;
        _out << eb;
        _out << eb;

        _out << sp << nl << "public static " << name << " read(Ice.InputStream inS__)";
        _out << sb;
        _out << nl << "int sz__ = inS__.readSize();";
        _out << nl << name << " r__ = new " << name << "();";
        _out << nl << "for(int i__ = 0; i__ < sz__; ++i__)";
        _out << sb;
        _out << nl << keyS << " k__;";
        StructPtr st = StructPtr::dynamicCast(key);
        if(st)
        {
            if(isValueType(st))
            {
                _out << nl << "k__ = new " << typeToString(key) << "();";
            }
            else
            {
                _out << nl << "k__ = null;";
            }
        }
        writeMarshalUnmarshalCode(_out, key, "k__", false, true);
        if(!hasClassValue)
        {
            _out << nl << valueS << " v__;";
            StructPtr st = StructPtr::dynamicCast(value);
            if(st)
            {
                if(isValueType(st))
                {
                    _out << nl << "v__ = new " << typeToString(value) << "();";
                }
                else
                {
                    _out << nl << "v__ = null;";
                }
            }
        }
        writeMarshalUnmarshalCode(_out, value, hasClassValue ? patcher : "v__", false, true);
        if(!hasClassValue)
        {
            _out << nl << "r__[k__] = v__;";
        }
        _out << eb;
        _out << nl << "return r__;";
        _out << eb;

        _out << sp << nl << "public static Ice.OptionalFormat optionalFormat()";
        _out << sb;
        _out << nl << "return " << getOptionalFormat(p) << ';';
        _out << eb;
    }

    _out << eb;
}

Slice::Gen::DispatcherVisitor::DispatcherVisitor(::IceUtilInternal::Output &out, bool stream)
    : CsVisitor(out), _stream(stream)
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
    if(p->isLocal() || !p->isInterface())
    {
        return false;
    }

    string name = p->name();

    _out << sp;
    emitComVisibleAttribute();
    emitGeneratedCodeAttribute();
    _out << nl << "public abstract class " << name << "Disp_ : Ice.ObjectImpl, " << fixId(name);
    _out << sb;

    OperationList ops = p->operations();
    if(!ops.empty())
    {
        _out << sp << nl << "#region Slice operations";
    }

    for(OperationList::const_iterator op = ops.begin(); op != ops.end(); ++op)
    {
        bool amd = p->hasMetaData("amd") || (*op)->hasMetaData("amd");

        string opname = (*op)->name();
        vector<string> params;
        vector<string> args;
        TypePtr ret;

        if(amd)
        {
            opname = opname + "_async";
            params = getParamsAsync(*op, true);
            args = getArgsAsync(*op, true);
        }
        else
        {
            opname = fixId(opname, DotNet::ICloneable, true);
            params = getParams(*op);
            ret = (*op)->returnType();
            args = getArgs(*op);
        }

        string retS = typeToString(ret, (*op)->returnIsOptional());
        _out << sp << nl << "public " << retS << " " << opname << spar << params << epar;
        _out << sb << nl;
        if(ret)
        {
            _out << "return ";
        }
        _out << opname << spar << args << "Ice.ObjectImpl.defaultCurrent" << epar << ';';
        _out << eb;

        _out << sp << nl << "public abstract " << retS << " " << opname << spar << params;
        if(!p->isLocal())
        {
            _out << "Ice.Current current__";
        }
        _out << epar << ';';
    }

    if(!ops.empty())
    {
        _out << sp << nl << "#endregion"; // Slice operations
    }

    writeInheritedOperations(p);

    writeDispatchAndMarshalling(p, _stream);

    _out << eb;

    return true;
}

Slice::Gen::AsyncVisitor::AsyncVisitor(::IceUtilInternal::Output &out)
    : CsVisitor(out)
{
}

bool
Slice::Gen::AsyncVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasAsyncOps())
    {
        return false;
    }

    _out << sp << nl << "namespace " << fixId(p->name());
    _out << sb;
    return true;
}

void
Slice::Gen::AsyncVisitor::visitModuleEnd(const ModulePtr&)
{
    _out << eb;
}

bool
Slice::Gen::AsyncVisitor::visitClassDefStart(const ClassDefPtr&)
{
    return true;
}

void
Slice::Gen::AsyncVisitor::visitClassDefEnd(const ClassDefPtr&)
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

    if(cl->hasMetaData("amd") || p->hasMetaData("amd"))
    {
        string classNameAMD = "AMD_" + cl->name();
        string classNameAMDI = "_AMD_" + cl->name();

        vector<string> paramsAMD = getParamsAsyncCB(p, true, true);

        _out << sp;
        writeDocCommentOp(p);
        emitComVisibleAttribute();
        emitGeneratedCodeAttribute();
        _out << nl << "public interface " << classNameAMD << '_' << name << " : Ice.AMDCallback";
        _out << sb;
        _out << sp;
        writeDocCommentAMD(p, OutParam, "");
        _out << nl << "void ice_response" << spar << paramsAMD << epar << ';';
        _out << eb;

        TypePtr ret = p->returnType();

        ParamDeclList paramList = p->parameters();
        ParamDeclList outParams;

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

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "class " << classNameAMDI << '_' << name
            << " : IceInternal.IncomingAsync, " << classNameAMD << '_' << name;
        _out << sb;

        _out << sp << nl << "public " << classNameAMDI << '_' << name << "(IceInternal.Incoming inc) : base(inc)";
        _out << sb;
        _out << eb;

        _out << sp << nl << "public void ice_response" << spar << paramsAMD << epar;
        _out << sb;
        _out << nl << "if(validateResponse__(true))";
        _out << sb;
        if(ret || !outParams.empty())
        {
            _out << nl << "try";
            _out << sb;
            _out << nl << "IceInternal.BasicStream os__ = startWriteParams__(" << opFormatTypeToString(p) << ");";
            writeMarshalUnmarshalParams(outParams, p, true);
            if(p->returnsClasses(false))
            {
                _out << nl << "os__.writePendingObjects();";
            }
            _out << nl << "endWriteParams__(true);";
            _out << eb;
            _out << nl << "catch(Ice.LocalException ex__)";
            _out << sb;
            _out << nl << "exception__(ex__);";
            _out << nl << "return;";
            _out << eb;
        }
        else
        {
            _out << nl << "writeEmptyParams__();";
        }
        _out << nl << "response__();";
        _out << eb;
        _out << eb;

        if(!throws.empty())
        {
            _out << sp << nl << "override public void ice_exception(_System.Exception ex)";
            _out << sb;
            _out << nl << "try";
            _out << sb;
            _out << nl << "throw ex;";
            _out << eb;
            for(ExceptionList::const_iterator r = throws.begin(); r != throws.end(); ++r)
            {
                string exS = fixId((*r)->scoped());
                _out << nl << "catch(" << exS << " ex__)";
                _out << sb;
                _out << nl << "if(validateResponse__(false))";
                _out << sb;
                _out << nl << "writeUserException__(ex__, " << opFormatTypeToString(p) << ");";
                _out << nl << "response__();";
                _out << eb;
                _out << eb;
            }
            _out << nl << "catch(_System.Exception ex__)";
            _out << sb;
            _out << nl << "base.ice_exception(ex__);";
            _out << eb;
            _out << eb;
        }
        _out << eb;
    }
}

Slice::Gen::TieVisitor::TieVisitor(IceUtilInternal::Output& out)
    : CsVisitor(out)
{
}

bool
Slice::Gen::TieVisitor::visitModuleStart(const ModulePtr& p)
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
Slice::Gen::TieVisitor::visitModuleEnd(const ModulePtr&)
{
    _out << eb;
}

bool
Slice::Gen::TieVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    if(p->isLocal() || !p->isAbstract())
    {
        return false;
    }

    string name = p->name();
    string opIntfName = "Operations";

    _out << sp;
    emitComVisibleAttribute();
    emitGeneratedCodeAttribute();
    _out << nl << "public class " << name << "Tie_ : ";
    if(p->isInterface())
    {
        _out << name << "Disp_, Ice.TieBase";
    }
    else
    {
        _out << fixId(name) << ", Ice.TieBase";
    }
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

    OperationList ops = p->operations();
    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        bool hasAMD = p->hasMetaData("amd") || (*r)->hasMetaData("amd");
        string opName = hasAMD ? (*r)->name() + "_async" : fixId((*r)->name(), DotNet::ICloneable, true);

        TypePtr ret = (*r)->returnType();
        string retS = typeToString(ret);

        vector<string> params;
        vector<string> args;
        if(hasAMD)
        {
            params = getParamsAsync((*r), true);
            args = getArgsAsync(*r, true);
        }
        else
        {
            params = getParams(*r);
            args = getArgs(*r);
        }

        _out << sp << nl << "public override ";
        _out << (hasAMD ? string("void") : retS) << ' ' << opName << spar << params << "Ice.Current current__";
        _out << epar;
        _out << sb;
        _out << nl;
        if(ret && !hasAMD)
        {
            _out << "return ";
        }
        _out << "_ice_delegate." << opName << spar << args << "current__";
        _out << epar << ';';
        _out << eb;
    }

    NameSet opNames;
    ClassList bases = p->bases();
    for(ClassList::const_iterator i = bases.begin(); i != bases.end(); ++i)
    {
        writeInheritedOperationsWithOpNames(*i, opNames);
    }

    _out << sp << nl << "private " << name << opIntfName << "_ _ice_delegate;";

    return true;
}

void
Slice::Gen::TieVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _out << eb;
}

void
Slice::Gen::TieVisitor::writeInheritedOperationsWithOpNames(const ClassDefPtr& p, NameSet& opNames)
{
    OperationList ops = p->operations();
    for(OperationList::const_iterator r = ops.begin(); r != ops.end(); ++r)
    {
        bool hasAMD = p->hasMetaData("amd") || (*r)->hasMetaData("amd");
        string opName = hasAMD ? (*r)->name() + "_async" : fixId((*r)->name(), DotNet::ICloneable, true);
        if(opNames.find(opName) != opNames.end())
        {
            continue;
        }
        opNames.insert(opName);

        TypePtr ret = (*r)->returnType();
        string retS = typeToString(ret);

        vector<string> params;
        vector<string> args;
        if(hasAMD)
        {
            params = getParamsAsync((*r), true);
            args = getArgsAsync(*r, true);
        }
        else
        {
            params = getParams(*r);
            args = getArgs(*r);
        }

        _out << sp << nl << "public override ";
        _out << (hasAMD ? string("void") : retS) << ' ' << opName << spar << params << "Ice.Current current__";
        _out << epar;
        _out << sb;
        _out << nl;
        if(ret && !hasAMD)
        {
            _out << "return ";
        }
        _out << "_ice_delegate." << opName << spar << args << "current__";
        _out << epar << ';';
        _out << eb;
    }

    ClassList bases = p->bases();
    for(ClassList::const_iterator i = bases.begin(); i != bases.end(); ++i)
    {
        writeInheritedOperationsWithOpNames(*i, opNames);
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
    string retS = typeToString(ret);
    ParamDeclList params = op->parameters();

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
        vector<string> pDecl = getParamsAsync(op, true);

        _out << "public ";
        if(!forTie)
        {
            _out << "override ";
        }
        _out << "void " << opName << "_async" << spar << pDecl << "Ice.Current current__" << epar;

        if(comment)
        {
            _out << ';';
            return;
        }

        _out << sb;
        if(ret)
        {
            _out << nl << typeToString(ret) << " ret__ = " << writeValue(ret) << ';';
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
        _out << nl << "cb__.ice_response" << spar;
        if(ret)
        {
            _out << "ret__";
        }
        for(ParamDeclList::const_iterator i = params.begin(); i != params.end(); ++i)
        {
            if((*i)->isOutParam())
            {
                _out << fixId((*i)->name());
            }
        }
        _out << epar << ';';
        _out << eb;
    }
    else
    {
        vector<string> pDecls = getParams(op);

        _out << "public ";
        if(!forTie && !cl->isLocal())
        {
            _out << "override ";
        }
        _out << retS << ' ' << fixId(opName, DotNet::ICloneable, true) << spar << pDecls;
        if(!cl->isLocal())
        {
            _out << "Ice.Current current__";
        }
        _out << epar;
        if(comment)
        {
            _out << ';';
            return;
        }
        _out << sb;
        for(ParamDeclList::const_iterator i = params.begin(); i != params.end(); ++i)
        {
            if((*i)->isOutParam())
            {
                string name = fixId((*i)->name());
                TypePtr type = (*i)->type();
                _out << nl << name << " = " << writeValue(type) << ';';
            }
        }
        if(ret)
        {
            _out << nl << "return " << writeValue(ret) << ';';
        }
        _out << eb;
    }
}

Slice::Gen::ImplVisitor::ImplVisitor(IceUtilInternal::Output& out)
    : BaseImplVisitor(out)
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
    if(!p->isAbstract())
    {
        return false;
    }

    string name = p->name();

    _out << sp << nl << "public sealed class " << name << 'I';
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
