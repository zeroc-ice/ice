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
#include <string.h>

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

namespace
{

bool
isIdempotent(const OperationPtr& operation)
{
    // TODO: eliminate Nonmutating enumerator in the parser together with the nonmutating metadata.
    return operation->mode() != Operation::Normal;
}

string
opFormatTypeToString(const OperationPtr& op, string ns)
{
    switch (op->format())
    {
        case DefaultFormat:
        {
            return "null";
        }
        case CompactFormat:
        {
            return getUnqualified("Ice.FormatType.Compact", ns);
        }
        case SlicedFormat:
        {
            return getUnqualified("Ice.FormatType.Sliced", ns);
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
getEscapedParamName(const ExceptionPtr& p, const string& name)
{
    DataMemberList params = p->allDataMembers();

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
hasDataMemberWithName(const DataMemberList& dataMembers, const string& name)
{
    return find_if(dataMembers.begin(), dataMembers.end(), [name](const auto& m)
                                                           {
                                                               return m->name() == name;
                                                           }) != dataMembers.end();
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
Slice::CsVisitor::writeMarshalParams(const OperationPtr& op,
                                     const list<ParamInfo>& requiredParams,
                                     const list<ParamInfo>& taggedParams,
                                     const string& customStream,
                                     const string& obj)
{
    const string stream = customStream.empty() ? "ostr" : customStream;
    string ns = getNamespace(ClassDefPtr::dynamicCast(op->container()));
    for(const auto& param : requiredParams)
    {
        writeMarshalCode(_out, param.type, ns, obj + param.name, stream);
    }

    for(const auto& param : taggedParams)
    {
        writeTaggedMarshalCode(_out, param.type, ns, obj + param.name, param.tag, stream);
    }
}

void
Slice::CsVisitor::writeUnmarshalParams(const OperationPtr& op,
                                       const list<ParamInfo>& requiredParams,
                                       const list<ParamInfo>& taggedParams,
                                       const string& customStream)
{
    const string stream = customStream.empty() ? "istr" : customStream;
    string ns = getNamespace(ClassDefPtr::dynamicCast(op->container()));
    for(const auto& param : requiredParams)
    {
        _out << nl << param.typeStr;
        if(isNullable(param.type))
        {
            _out << "?";
        }
        _out << " " << param.name;
        if(isClassType(param.type) || StructPtr::dynamicCast(param.type))
        {
            _out << " = default";
        }
        _out << ";";
        writeUnmarshalCode(_out, param.type, ns, param.name, stream);
    }

    for(const auto& param : taggedParams)
    {
        _out << nl << param.typeStr << " " << param.name << " = null;";
        writeTaggedUnmarshalCode(_out, param.type, ns, param.name, param.tag,
                                   stream);
    }
}

void
Slice::CsVisitor::writeMarshalDataMember(const DataMemberPtr& member, const string& name, const string& ns,
                                         const string& customStream)
{
    const string stream = customStream.empty() ? "ostr" : customStream;
    if(member->tagged())
    {
        writeTaggedMarshalCode(_out, member->type(), ns, "this." + name, member->tag(), stream);
    }
    else
    {
        writeMarshalCode(_out, member->type(), ns, "this." + name, stream);
    }
}

void
Slice::CsVisitor::writeUnmarshalDataMember(const DataMemberPtr& member, const string& name, const string& ns,
                                           const string& customStream)
{
    const string stream = customStream.empty() ? "ostr" : customStream;
    if(member->tagged())
    {
        writeTaggedUnmarshalCode(_out, member->type(), ns, name, member->tag(), stream);
    }
    else
    {
        writeUnmarshalCode(_out, member->type(), ns, name, stream);
    }
}

void
Slice::CsVisitor::writeMarshaling(const ClassDefPtr& p)
{
    string name = fixId(p->name());
    string scoped = p->scoped();
    string ns = getNamespace(p);
    ClassList allBases = p->allBases();
    ClassList bases = p->bases();

    //
    // Marshaling support
    //
    DataMemberList members = p->dataMembers();
    DataMemberList taggedMembers = p->sortedTaggedDataMembers();
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
        _out << nl << "protected override " << getUnqualified("Ice.SlicedData", ns) << "? IceSlicedData { get; set; }";
    }

    _out << sp;
    if(!p->isInterface())
    {
        emitGeneratedCodeAttribute();
    }

    _out << nl << "protected override void IceWrite(" << getUnqualified("Ice.OutputStream", ns)
         << " iceP_ostr, bool iceP_firstSlice)";
    _out << sb;
    _out << nl << "iceP_ostr.IceStartSlice" << spar << "_iceTypeId" << "iceP_firstSlice";
    if (preserved || basePreserved)
    {
        _out << "iceP_firstSlice ? IceSlicedData : null";
    }
    else
    {
        _out << "null";
    }
    if (p->compactId() >= 0)
    {
        _out << p->compactId();
    }
    _out << epar << ";";
    for(auto m : members)
    {
        if(!m->tagged())
        {
            writeMarshalDataMember(m, fixId(dataMemberName(m)), ns, "iceP_ostr");
        }
    }

    for(auto m : taggedMembers)
    {
        writeMarshalDataMember(m, fixId(dataMemberName(m)), ns, "iceP_ostr");
    }
    if(base)
    {
        _out << nl << "iceP_ostr.IceEndSlice(false);";
        _out << nl << "base.IceWrite(iceP_ostr, false);";
    }
    else
    {
         _out << nl << "iceP_ostr.IceEndSlice(true);"; // last slice
    }
    _out << eb;

    _out << sp;
    if(!p->isInterface())
    {
        emitGeneratedCodeAttribute();
    }

    _out << nl << "protected override void IceRead("
         << getUnqualified("Ice.InputStream", ns) << " iceP_istr, bool iceP_firstSlice)";
    _out << sb;
    if (preserved || basePreserved)
    {
        _out << nl << "if (iceP_firstSlice)";
        _out << sb;
        _out << nl << "IceSlicedData = iceP_istr.IceStartSliceAndGetSlicedData(_iceTypeId);";
        _out << eb;
        _out << "else";
        _out << sb;
        _out << nl << "iceP_istr.IceStartSlice" << spar << "_iceTypeId" << "false" << epar << ";";
        _out << eb;
    }
    else
    {
        _out << nl << "iceP_istr.IceStartSlice" << spar << "_iceTypeId" << "iceP_firstSlice" << epar << ";";
    }

    for(auto m : members)
    {
        if(!m->tagged())
        {
            writeUnmarshalDataMember(m, fixId(dataMemberName(m)), ns, "iceP_istr");
        }
    }

    for(auto m : taggedMembers)
    {
        writeUnmarshalDataMember(m, fixId(dataMemberName(m)), ns, "iceP_istr");
    }
    _out << nl << "iceP_istr.IceEndSlice();";
    if(base)
    {
        _out << nl << "base.IceRead(iceP_istr, false);";
    }
    _out << eb;
}

string
getParamAttributes(const ParamDeclPtr& p)
{
    string result;
    for(const auto& s : p->getMetaData())
    {
        static const string prefix = "cs:attribute:";
        if(s.find(prefix) == 0)
        {
            result += "[" + s.substr(prefix.size()) + "] ";
        }
    }
    return result;
}

vector<string>
getInvocationParams(const OperationPtr& op, const string& ns)
{
    vector<string> params;
    ParamDeclList paramList = op->parameters();
    for(const auto& p : op->parameters())
    {
        if(p->isOutParam())
        {
            continue;
        }
        ostringstream param;
        param << getParamAttributes(p);
        if(StructPtr::dynamicCast(p->type()))
        {
            param << "in ";
        }
        param << CsGenerator::typeToString(p->type(), ns, p->tagged() || isNullable(p->type()))
              << " "
              << fixId(p->name());
        params.push_back(param.str());
    }
    params.push_back("global::System.Collections.Generic.IReadOnlyDictionary<string, string>? " +
                     getEscapedParamName(op, "context") + " = null");
    return params;
}

vector<string>
getInvocationParamsAMI(const OperationPtr& op, const string& ns, bool defaultValues, const string& prefix = "")
{
    vector<string> params;
    for(const auto& p : op->parameters())
    {
        if(p->isOutParam())
        {
            continue;
        }

        ostringstream param;
        param << getParamAttributes(p);
        if(StructPtr::dynamicCast(p->type()))
        {
            param << "in ";
        }
        param << CsGenerator::typeToString(p->type(), ns, p->tagged() || isNullable(p->type()))
              << " "
              << fixId(prefix + p->name());
        params.push_back(param.str());
    }

    string context = prefix.empty() ? getEscapedParamName(op, "context") : "context";
    string progress = prefix.empty() ? getEscapedParamName(op, "progress") : "progress";
    string cancel = prefix.empty() ? getEscapedParamName(op, "cancel") : "cancel";

    if(defaultValues)
    {
        params.push_back("global::System.Collections.Generic.IReadOnlyDictionary<string, string>? " + context + " = null");
        params.push_back("global::System.IProgress<bool>? " + progress + " = null");
        params.push_back("global::System.Threading.CancellationToken " + cancel +
                         " = default");
    }
    else
    {
        params.push_back("global::System.Collections.Generic.IReadOnlyDictionary<string, string>? " + context);
        params.push_back("global::System.IProgress<bool>? " + progress);
        params.push_back("global::System.Threading.CancellationToken " + cancel);
    }
    return params;
}

vector<string>
getInvocationArgsAMI(const OperationPtr& op,
                     const string& context = "",
                     const string& progress = "null",
                     const string cancelationToken = "global::System.Threading.CancellationToken.None",
                     const string& async = "true")
{
    vector<string> args = getNames(getAllInParams(op));

    if(context.empty())
    {
        args.push_back(getEscapedParamName(op, "context"));
    }
    else
    {
        args.push_back(context);
    }

    args.push_back(progress);
    args.push_back(cancelationToken);
    args.push_back(async);

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
    _out << nl << "[global::System.Runtime.InteropServices.ComVisible(false)]";
}

void
Slice::CsVisitor::emitGeneratedCodeAttribute()
{
    _out << nl << "[global::System.CodeDom.Compiler.GeneratedCodeAttribute(\"slice2cs\", \"" << ICE_STRING_VERSION
         << "\")]";
}

void
Slice::CsVisitor::emitTypeIdAttribute(const string& typeId)
{
    _out << nl << "[global::Ice.TypeId(\"" << typeId << "\")]";
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
            }
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindUShort:
            case Builtin::KindInt:
            case Builtin::KindUInt:
            case Builtin::KindVarInt:
            case Builtin::KindVarUInt:
            case Builtin::KindLong:
            case Builtin::KindULong:
            case Builtin::KindVarLong:
            case Builtin::KindVarULong:
            {
                return "0";
            }
            case Builtin::KindFloat:
            {
                return "0.0f";
            }
            case Builtin::KindDouble:
            {
                return "0.0";
            }
            default:
            {
                return "null";
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
        return "new " + typeToString(type, ns) + "()";
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
        if(bp)
        {
            if(bp->kind() == Builtin::KindString)
            {
                _out << "\"" << toStringLiteral(value, "\a\b\f\n\r\t\v\0", "", UCN, 0) << "\"";
            }
            else if(bp->kind() == Builtin::KindUShort || bp->kind() == Builtin::KindUInt ||
                    bp->kind() == Builtin::KindVarUInt)
            {
                _out << value << "U";
            }
            else if(bp->kind() == Builtin::KindLong || bp->kind() == Builtin::KindVarLong)
            {
                _out << value << "L";
            }
            else if(bp->kind() == Builtin::KindULong || bp->kind() == Builtin::KindVarULong)
            {
                _out << value << "UL";
            }
            else if(bp->kind() == Builtin::KindFloat)
            {
                _out << value << "F";
            }
            else
            {
                _out << value;
            }

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
        else if((*p)->tagged())
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
Slice::CsVisitor::writeDataMemberInitializers(const DataMemberList& members, const string& ns, unsigned int baseTypes)
{
    for(const auto& p : members)
    {
        TypePtr type = p->type();

        if(p->defaultValueType())
        {
            _out << nl << "this." << dataMemberName(p) << " = ";
            writeConstantValue(type, p->defaultValueType(), p->defaultValue());
            _out << ';';
        }
        else if(!p->tagged())
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(p->type());
            StructPtr st = StructPtr::dynamicCast(type);
            SequencePtr seq = SequencePtr::dynamicCast(type);
            DictionaryPtr dict = DictionaryPtr::dynamicCast(type);

            if(builtin && builtin->kind() == Builtin::KindString)
            {
                _out << nl << fixId(dataMemberName(p), baseTypes) << " = \"\";";
            }
            else if(st)
            {
                _out << nl << fixId(dataMemberName(p), baseTypes) << " = new " << typeToString(st, ns) << "();";
            }
            else if(seq && !seq->hasMetaDataWithPrefix("cs:serializable:"))
            {
                if(seq->hasMetaDataWithPrefix("cs:generic:"))
                {
                    _out << nl << fixId(dataMemberName(p), baseTypes) << " = new " << typeToString(type, ns) << "();";
                }
                else
                {
                    _out << nl << fixId(dataMemberName(p), baseTypes) << " = global::System.Array.Empty<"
                         << typeToString(seq->type(), ns, false) << ">();";
                }
            }
            else if(dict)
            {
                _out << nl << fixId(dataMemberName(p), baseTypes) << " = new " << typeToString(type, ns) << "();";
            }
        }
    }
}

namespace
{

//
// Convert the identifier part of a Java doc Link tag to a CSharp identiefier. If the identifier
// is an interface the link should point to the corresponding generated proxy, we apply the
// case conversions required to match the CSharp generatd code.
//
string
csharpIdentifier(ContainedPtr contained, const string& identifier)
{
    string ns = getNamespace(contained);
    string typeName;
    string memberName;
    string::size_type pos = identifier.find('#');
    if(pos == 0)
    {
        memberName = identifier.substr(1);
    }
    else if(pos == string::npos)
    {
        typeName = identifier;
    }
    else
    {
        typeName = identifier.substr(0, pos);
        memberName = identifier.substr(pos + 1);
    }

    // lookup the Slice definition for the identifier
    ContainedPtr definition;
    if(typeName.empty())
    {
        definition = contained;
    }
    else
    {
        TypeList types = contained->unit()->lookupTypeNoBuiltin(typeName, false, true);
        definition = types.empty() ? nullptr : ContainedPtr::dynamicCast(types.front());
    }

    ostringstream os;
    if(!definition || !normalizeCase(definition))
    {
        if(typeName == "::Ice::Object")
        {
            os << "Ice.IObjectPrx";
        }
        else
        {
            os << fixId(typeName);
        }
    }
    else
    {
        ClassDefPtr def = ClassDefPtr::dynamicCast(definition);
        if(!def)
        {
            ClassDeclPtr decl = ClassDeclPtr::dynamicCast(definition);
            if(decl)
            {
                def = decl->definition();
            }
        }

        if(def && def->isInterface())
        {
            os << getUnqualified(fixId(definition->scope()) + interfaceName(def), ns) << "Prx";
        }
        else
        {
            typeName = fixId(typeName);
            pos = typeName.rfind(".");
            if(pos == string::npos)
            {
                os << pascalCase(fixId(typeName));
            }
            else
            {
                os << typeName.substr(0, pos) << "." << pascalCase(typeName.substr(pos + 1));
            }
        }
    }

    if(!memberName.empty())
    {
        os << "." << (definition && normalizeCase(definition) ? pascalCase(fixId(memberName)) : fixId(memberName));
    }
    string result = os.str();
    //
    // strip global:: prefix if present, it is not supported in doc comment cref attributes
    //
    const string global = "global::";
    if(result.find(global) == 0)
    {
        result = result.substr(global.size());
    }
    return result;
}

vector<string>
splitLines(const string& s)
{
    vector<string> lines;
    istringstream is(s);
    for(string line; getline(is, line, '\n');)
    {
        lines.push_back(trim(line));
    }
    return lines;
}

//
// Transform a Java doc style tag to a C# doc style tag, returns a map idenxed by the C#
// tag name atrribute and the value contains all the lines in the comment.
//
// @param foo is the Foo argument -> {"foo": ["foo is the Foo argument"]}
//
map<string, vector<string>>
processTag(const string& sourceTag, const string& s)
{
    map<string, vector<string>> result;
    for(string::size_type pos = s.find(sourceTag); pos != string::npos; pos = s.find(sourceTag, pos + 1))
    {
        string::size_type startIdent = s.find_first_not_of(" \t", pos + sourceTag.size());
        string::size_type endIdent = s.find_first_of(" \t", startIdent);
        string::size_type endComment = s.find_first_of("@", endIdent);
        if(endIdent != string::npos)
        {
            string ident = s.substr(startIdent, endIdent - startIdent);
            string comment = s.substr(endIdent + 1,
                                      endComment == string::npos ? endComment : endComment - endIdent - 1);
            result[ident] = splitLines(trim(comment));
        }
    }
    return result;
}

CommentInfo
processComment(const ContainedPtr& contained, const string& deprecateReason)
{
    //
    // Strip HTML markup and javadoc links that are not displayed by Visual Studio.
    //
    string data = contained->comment();
    for(string::size_type pos = data.find('<'); pos != string::npos; pos = data.find('<', pos))
    {
        string::size_type endpos = data.find('>', pos);
        if(endpos == string::npos)
        {
            break;
        }
        data.erase(pos, endpos - pos + 1);
    }

    const string link = "{@link ";
    for(string::size_type pos = data.find(link); pos != string::npos; pos = data.find(link, pos))
    {
        data.erase(pos, link.size());
        string::size_type endpos = data.find('}', pos);
        if(endpos != string::npos)
        {
            string ident = data.substr(pos, endpos - pos);
            data.erase(pos, endpos - pos + 1);
            data.insert(pos, csharpIdentifier(contained, ident));
        }
    }

    const string see = "{@see ";
    for(string::size_type pos = data.find(see); pos != string::npos; pos = data.find(see, pos))
    {
        string::size_type endpos = data.find('}', pos);
        if(endpos != string::npos)
        {
            string ident = data.substr(pos + see.size(), endpos - pos - see.size());
            data.erase(pos, endpos - pos + 1);
            data.insert(pos, "<see cref=\"" + csharpIdentifier(contained, ident) + "\"/>");
        }
    }

    CommentInfo comment;

    const string paramTag = "@param";
    const string throwsTag = "@throws";
    const string exceptionTag = "@exception";
    const string returnTag = "@return";

    string::size_type pos;
    for(pos = data.find('@'); pos != string::npos; pos = data.find('@', pos + 1))
    {
        if((data.substr(pos, paramTag.size()) == paramTag ||
            data.substr(pos, throwsTag.size()) == throwsTag ||
            data.substr(pos, exceptionTag.size()) == exceptionTag ||
            data.substr(pos, returnTag.size()) == returnTag))
        {
            break;
        }
    }

    if(pos > 0)
    {
        ostringstream os;
        os << trim(data.substr(0, pos));
        if(!deprecateReason.empty())
        {
            os << "<para>" << deprecateReason << "</para>";
        }
        comment.summaryLines = splitLines(os.str());
    }

    if(comment.summaryLines.empty() && !deprecateReason.empty())
    {
        comment.summaryLines.push_back(deprecateReason);
    }

    comment.params = processTag("@param", data);
    comment.exceptions = processTag("@throws", data);

    pos = data.find(returnTag);
    if(pos != string::npos)
    {
        pos += returnTag.size();
        string::size_type endComment = data.find("@", pos);
        comment.returnLines = splitLines(
            trim(data.substr(pos , endComment == string::npos ? endComment : endComment - pos)));
    }

    return comment;
}

}

void writeDocCommentLines(IceUtilInternal::Output& out, const vector<string>& lines)
{
    for(vector<string>::const_iterator i = lines.begin(); i != lines.end(); ++i)
    {
        if(i == lines.begin())
        {
            out << *i;
        }
        else
        {
            out << nl << "///";
            if(!i->empty())
            {
                out << " " << (*i);
            }
        }
    }
}

void writeDocCommentLines(IceUtilInternal::Output& out,
                          const vector<string>& lines,
                          const string& tag,
                          const string& name = "",
                          const string& value = "")
{
    out << nl << "/// <" << tag;
    if(!name.empty())
    {
        out << " " << name << "=\"" << value << "\"";
    }
    out << ">";
    writeDocCommentLines(out, lines);
    out << "</" << tag << ">";
}

void
writeSeeAlsoDocComment(IceUtilInternal::Output& out, const string& identifier)
{
    out << nl << "/// <seealso cref=\"" << identifier << "\"/>";
}

void
Slice::CsVisitor::writeTypeDocComment(const ContainedPtr& p,
                                      const string& deprecateReason)
{
    CommentInfo comment = processComment(p, deprecateReason);
    writeDocCommentLines(_out, comment.summaryLines, "summary");
}

void
Slice::CsVisitor::writeOperationDocComment(const OperationPtr& p, const string& deprecateReason,
                                           bool dispatch, bool async)
{
    CommentInfo comment = processComment(p, deprecateReason);
    writeDocCommentLines(_out, comment.summaryLines, "summary");
    writeParamDocComment(p, comment, InParam);

    list<ParamInfo> outParams = getAllOutParams(p,"", true);

    if(dispatch)
    {
        _out << nl << "/// <param name=\"" << getEscapedParamName(p, "current")
             << "\">The Current object for the dispatch.</param>";
    }
    else
    {
        _out << nl << "/// <param name=\"" << getEscapedParamName(p, "context")
             << "\">Context map to send with the invocation.</param>";

        if(async)
        {
            _out << nl << "/// <param name=\"" << getEscapedParamName(p, "cancel")
                 << "\">Sent progress provider.</param>";
            _out << nl << "/// <param name=\"" << getEscapedParamName(p, "progress")
                 << "\">A cancellation token that receives the cancellation requests.</param>";
        }
    }

    if(dispatch && p->hasMarshaledResult())
    {
        _out << nl << "/// <returns>The operation marshaled result.</returns>";
    }
    else if(async)
    {
        _out << nl << "/// <returns>The task object representing the asynchronous operation.</returns>";
    }
    else if(outParams.size() == 1)
    {
        writeDocCommentLines(_out, comment.returnLines, "returns");
    }
    else if(outParams.size() > 1)
    {
        _out << nl << "/// <returns>Named tuple with the following fields:";

        for(const auto& paramInfo : outParams)
        {
            if(paramInfo.param)
            {
                auto i = comment.params.find(paramInfo.name);
                if(i != comment.params.end())
                {
                    _out << nl << "/// <para> " << paramName(paramInfo) << ": ";
                    writeDocCommentLines(_out, i->second);
                    _out << "</para>";
                }
            }
            else if(!comment.returnLines.empty()) // Return type
            {
                _out << nl << "/// <para> " << paramInfo.name << ": ";
                writeDocCommentLines(_out, comment.returnLines);
                _out << "</para>";
            }
        }
        _out << nl << "/// </returns>";
    }

    for(const auto& e : comment.exceptions)
    {
        writeDocCommentLines(_out, e.second, "exceptions", "cref", e.first);
    }
}

void
Slice::CsVisitor::writeParamDocComment(const OperationPtr& p, const CommentInfo& comment, ParamDir paramType)
{
    //
    // Collect the names of the in- or -out parameters to be documented.
    //
    for(const auto param : p->parameters())
    {
        if((param->isOutParam() && paramType == OutParam) || (!param->isOutParam() && paramType == InParam))
        {
            auto i = comment.params.find(param->name());
            if(i != comment.params.end())
            {
                writeDocCommentLines(_out, i->second, "param", "name", paramName(param));
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

    _out << nl << "#nullable enable";

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

    ProxyVisitor proxyVisitor(_out);
    p->visit(&proxyVisitor, false);

    HelperVisitor helperVisitor(_out);
    p->visit(&helperVisitor, false);

    DispatcherVisitor dispatcherVisitor(_out);
    p->visit(&dispatcherVisitor, false);
}

void
Slice::Gen::generateImpl(const UnitPtr& p)
{
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

    if(!ns.empty() && (p->hasValueDefs() || p->hasExceptions()))
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
    if(p->isInterface())
    {
        return false;
    }

    string name = p->name();
    string scoped = fixId(p->scoped(), Slice::ObjectType);
    string ns = getNamespace(p);
    ClassList bases = p->bases();

    _out << sp;
    writeTypeDocComment(p, getDeprecateReason(p, 0, "type"));
    emitAttributes(p);
    emitComVisibleAttribute();
    emitPartialTypeAttributes();
    _out << nl << "[global::System.Serializable]";
    emitTypeIdAttribute(p->scoped());
    _out << nl << "public partial class " << fixId(name) << " : "
         << (bases.empty() ? getUnqualified("Ice.AnyClass", ns) : getUnqualified(bases.front(), ns))
         << sb;
    return true;
}

void
Slice::Gen::TypesVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
    DataMemberList dataMembers = p->dataMembers();
    DataMemberList allDataMembers = p->allDataMembers();
    ClassList bases = p->bases();
    bool hasBaseClass = !bases.empty();

    _out << sp;
    _out << nl << "public static readonly new Ice.InputStreamReader<" << name << "?> IceReader =";
    _out.inc();
    _out << nl << "(istr) => istr.ReadClass<" << name << ">();";
    _out.dec();

    _out << sp;
    _out << nl << "public static readonly new Ice.OutputStreamWriter<" << name << "?> IceWriter =";
    _out.inc();
    _out << nl << "(ostr, value) => ostr.WriteClass(value);";
    _out.dec();

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "private readonly string _iceTypeId = global::Ice.TypeExtensions.GetIceTypeId(typeof("
         << fixId(p->name()) << "))!;";

    bool partialInitialize = !hasDataMemberWithName(allDataMembers, "Initialize");
    if(partialInitialize)
    {
        _out << sp << nl << "partial void Initialize();";
    }

    if(allDataMembers.empty())
    {
        if(partialInitialize)
        {
            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << "public " << name << spar << epar;
            _out << sb;
            _out << nl << "Initialize();";
            _out << eb;
        }
    }
    else
    {
        const bool propertyMapping = p->hasMetaData("cs:property");

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public " << name << spar << epar;
        _out << sb;
        writeDataMemberInitializers(dataMembers, ns, propertyMapping);
        if(partialInitialize)
        {
            _out << nl << "Initialize();";
        }
        _out << eb;

        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public " << name
             << spar
             << mapfn<DataMemberPtr>(allDataMembers,
                                     [&ns](const auto& i)
                                     {
                                         return typeToString(i->type(), ns, i->tagged()) + " " + fixId(i->name());
                                     })
             << epar;
        if(hasBaseClass && allDataMembers.size() != dataMembers.size())
        {
            _out << " : base" << spar;
            vector<string> baseParamNames;
            for(const auto& d : bases.front()->allDataMembers())
            {
                baseParamNames.push_back(fixId(d->name()));
            }
            _out << baseParamNames << epar;
        }
        _out << sb;
        for(const auto& d : dataMembers)
        {
            _out << nl << "this." << fixId(dataMemberName(d), Slice::ObjectType) << " = "
                 << fixId(d->name(), Slice::ObjectType) << ";";
        }
        if(partialInitialize)
        {
            _out << nl << "Initialize();";
        }
        _out << eb;
    }

    writeMarshaling(p);
    _out << eb;
}

bool
Slice::Gen::TypesVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
    ExceptionPtr base = p->base();

    _out << sp;
    writeTypeDocComment(p, getDeprecateReason(p, 0, "type"));
    emitDeprecate(p, 0, _out, "type");
    emitAttributes(p);
    emitComVisibleAttribute();
    //
    // Suppress FxCop diagnostic about a missing constructor MyException(String).
    //
    _out << nl << "[global::System.Diagnostics.CodeAnalysis.SuppressMessage(\"Microsoft.Design\", \"CA1032\")]";
    _out << nl << "[global::System.Serializable]";
    emitPartialTypeAttributes();
    emitTypeIdAttribute(p->scoped());
    _out << nl << "public partial class " << name << " : ";
    if(base)
    {
        _out << getUnqualified(base, ns);
    }
    else
    {
        _out << getUnqualified("Ice.RemoteException", ns);
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

    string messageParamName = getEscapedParamName(p, "message");
    string innerExceptionParamName = getEscapedParamName(p, "innerException");

    vector<string> allParamDecl;
    for(DataMemberList::const_iterator q = allDataMembers.begin(); q != allDataMembers.end(); ++q)
    {
        string memberName = fixId((*q)->name());
        string memberType = typeToString((*q)->type(), ns, (*q)->tagged());
        allParamDecl.push_back(memberType + " " + memberName);
    }

    vector<string> baseParamNames;
    if(p->base())
    {
        DataMemberList baseDataMembers = p->base()->allDataMembers();
        for(DataMemberList::const_iterator q = baseDataMembers.begin(); q != baseDataMembers.end(); ++q)
        {
            baseParamNames.push_back(fixId((*q)->name()));
        }
    }

    const bool hasDataMemberInitializers = requiresDataMemberInitializers(dataMembers);

    emitGeneratedCodeAttribute();
    _out << nl << "private readonly string _iceTypeId = global::Ice.TypeExtensions.GetIceTypeId(typeof("
         << name << "))!;";

    _out << sp;

    // Parameterless constructor - all exceptions derived from System.Exception should provide such a constructor.
    emitGeneratedCodeAttribute();
    _out << nl << "public " << name << "()";
    _out << sb;
    if(hasDataMemberInitializers)
    {
        writeDataMemberInitializers(dataMembers, ns, Slice::ExceptionType);
    }
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public " << name << "(global::System.Runtime.Serialization.SerializationInfo info, "
         << "global::System.Runtime.Serialization.StreamingContext context)";
    _out.inc();
    _out << nl << ": base(info, context)";
    _out.dec();
    _out << sb;
    if(!dataMembers.empty())
    {
        bool optionals = false;
        static const std::array<std::string, 18> builtinGetter =
        {
            "GetBoolean",
            "GetByte",
            "GetInt16",
            "GetUInt16",
            "GetInt32",
            "GetUInt32",
            "GetInt32",
            "GetUInt32",
            "GetInt64",
            "GetUInt64",
            "GetInt64",
            "GetUInt64",
            "GetSingle",
            "GetDouble",
            "GetString",
            "",
            "",
            "",
        };

        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            DataMemberPtr m = *q;
            if(m->tagged() && isValueType(m->type()))
            {
                optionals = true;
                continue;
            }
            string getter;
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(m->type());
            if(builtin)
            {
                getter = builtinGetter[builtin->kind()];
            }
            if(getter.empty())
            {
                getter = "GetValue";
            }
            string mName = fixId(dataMemberName(m), Slice::ExceptionType);
            _out << nl << "this." << mName << " = ";
            if(getter == "GetValue")
            {
                _out << "(" << typeToString(m->type(), ns, false) << ")";
            }
            _out << "info." << getter << "(\"" << mName << "\"";
            if(getter == "GetValue")
            {
                _out << ", typeof(" << typeToString(m->type(), ns, false) << ")";
            }
            _out << ")!;";
        }

        if(optionals)
        {
            _out << nl << "foreach (var entry in info)";
            _out << sb;
            _out << nl << "switch (entry.Name)";
            _out << sb;
            for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
            {
                DataMemberPtr m = *q;
                if(!m->tagged() || !isValueType(m->type()))
                {
                    continue;
                }
                string mName = fixId(m->name(), Slice::ExceptionType);
                _out << nl << "case \"" << mName << "\":";
                _out << sb;
                _out << nl << "this." << mName << " = (" << typeToString(m->type(), ns, false) << ") entry.Value!;";
                _out << nl << "break;";
                _out << eb;
            }
            _out << eb;
            _out << eb;
        }
    }
    _out << eb;

    // Up to 3 "one-shot" constructors
    for (int i = 0; i < 3; i++)
    {
        if (allParamDecl.size() > 0)
        {
            _out << sp;
            emitGeneratedCodeAttribute();
            _out << nl << "public " << name << spar << allParamDecl << epar;
            _out.inc();
            if (baseParamNames.size() > 0)
            {
                _out << nl << ": base" << spar << baseParamNames << epar;
            }
            // else we use the base's parameterless ctor.
            _out.dec();
            _out << sb;
            if (!dataMembers.empty())
            {
                for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
                {
                    string memberName = fixId(dataMemberName(*q), Slice::ExceptionType);
                    _out << nl << "this." << memberName << " = " << fixId((*q)->name()) << ';';
                }
            }
            _out << eb;
        }

        if (i == 0)
        {
            // Insert message first
            allParamDecl.insert(allParamDecl.cbegin(), "string " + messageParamName);
            baseParamNames.insert(baseParamNames.cbegin(), messageParamName);
        }
        else if (i == 1)
        {
            // Also add innerException
            allParamDecl.push_back("global::System.Exception " + innerExceptionParamName);
            baseParamNames.push_back(innerExceptionParamName);
        }
    }

    if(!dataMembers.empty())
    {
        _out << sp;
        emitGeneratedCodeAttribute();
        _out << nl << "public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, "
             << "global::System.Runtime.Serialization.StreamingContext context)";
        _out << sb;
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            DataMemberPtr m = *q;
            string mName = fixId(dataMemberName(m), Slice::ExceptionType);
            if(m->tagged() && isValueType(m->type()))
            {
                _out << nl << "if (" << mName << " != null)";
                _out << sb;
            }
            _out << nl << "info.AddValue(\"" << mName << "\", " << mName;

            if(m->tagged() && isValueType(m->type()))
            {
                _out << ".Value";
            }

            if(ContainedPtr::dynamicCast(m->type()))
            {
                _out << ", typeof(" << typeToString(m->type(), ns, false) << ")";
            }

            _out << ");";

            if(m->tagged() && isValueType(m->type()))
            {
                _out << eb;
            }
        }
        _out << sp << nl << "base.GetObjectData(info, context);";
        _out << eb;
    }

    string scoped = p->scoped();
    ExceptionPtr base = p->base();

    // Remote exceptions are always "preserved".

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "protected override void IceWrite(" << getUnqualified("Ice.OutputStream", ns)
         << " iceP_ostr, bool iceP_firstSlice)";
    _out << sb;
    _out << nl << "iceP_ostr.IceStartSlice" << spar << "_iceTypeId" << "iceP_firstSlice";
    _out << "iceP_firstSlice ? IceSlicedData : null";
    _out << epar << ";";

    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        writeMarshalDataMember(*q, fixId(dataMemberName(*q), Slice::ExceptionType), ns, "iceP_ostr");
    }
    if(base)
    {
        _out << nl << "iceP_ostr.IceEndSlice(false);"; // the current slice is not last slice
        _out << nl << "base.IceWrite(iceP_ostr, false);"; // the next one is not the first slice
    }
    else
    {
        _out << nl << "iceP_ostr.IceEndSlice(true);"; // this is the last slice.
    }
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();

    _out << nl << "protected override void IceRead("
         << getUnqualified("Ice.InputStream", ns) << " iceP_istr, bool iceP_firstSlice)";
    _out << sb;

    _out << nl << "if (iceP_firstSlice)";
    _out << sb;
    _out << nl << "IceSlicedData = iceP_istr.IceStartSliceAndGetSlicedData(_iceTypeId);";
    _out << eb;
    _out << nl << "else";
    _out << sb;
    _out << nl << "iceP_istr.IceStartSlice" << spar << "_iceTypeId" << "false" << epar << ";";
    _out << eb;

    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        writeUnmarshalDataMember(*q, fixId(dataMemberName(*q), Slice::ExceptionType), ns, "iceP_istr");
    }
    _out << nl << "iceP_istr.IceEndSlice();";
    if(base)
    {
        _out << nl << "base.IceRead(iceP_istr, false);";
    }
    _out << eb;
    _out << eb;
}

bool
Slice::Gen::TypesVisitor::visitStructStart(const StructPtr& p)
{
    string name = fixId(p->name());
    string ns = getNamespace(p);
    _out << sp;

    writeTypeDocComment(p, getDeprecateReason(p, 0, "type"));
    emitDeprecate(p, 0, _out, "type");
    emitAttributes(p);
    emitPartialTypeAttributes();
    _out << nl << "[global::System.Serializable]";
    _out << nl << "public ";
    if(p->hasMetaData("cs:readonly"))
    {
        _out << "readonly ";
    }
    _out << "partial struct " << name <<  " : global::System.IEquatable<" << name << ">, Ice.IStreamableStruct";
    _out << sb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public static Ice.InputStreamReader<" << name << "> IceReader => ";
    _out.inc();
    _out << nl << "(istr) => new " << name << "(istr);";
    _out.dec();

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public static Ice.OutputStreamStructWriter<" << name << "> IceWriter => ";
    _out.inc();
    _out << nl << "(Ice.OutputStream ostr, in " << name << " value) => value.IceWrite(ostr);";
    _out.dec();

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public const int IceMinWireSize = " << p->minWireSize() << ";";
    return true;
}

void
Slice::Gen::TypesVisitor::visitStructEnd(const StructPtr& p)
{
    string name = fixId(p->name());
    string scope = fixId(p->scope());
    string ns = getNamespace(p);
    DataMemberList dataMembers = p->dataMembers();

    bool partialInitialize = !hasDataMemberWithName(dataMembers, "Initialize");

    if(partialInitialize)
    {
        _out << sp << nl << "partial void Initialize();";
    }

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public ";
    _out << name
         << spar
         << mapfn<DataMemberPtr>(dataMembers, [&ns](const auto& i)
                                              {
                                                  return typeToString(i->type(), ns) + " " + fixId(i->name());
                                              })
         << epar;
    _out << sb;
    for(const auto& i : dataMembers)
    {
        string paramName = fixId(i->name());
        string memberName = fixId(dataMemberName(i), Slice::ObjectType);
        _out << nl << (paramName == memberName ? "this." : "") << memberName  << " = " << paramName  << ";";
    }
    if(partialInitialize)
    {
        _out << nl << "Initialize();";
    }
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public " << name << "(" << getUnqualified("Ice.InputStream", ns) << " iceP_istr)";
    _out << sb;
    for(auto m : dataMembers)
    {
        writeUnmarshalDataMember(m, fixId(dataMemberName(m)) , ns, "iceP_istr");
    }

    if(partialInitialize)
    {
        _out << nl << "Initialize();";
    }

    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public override int GetHashCode()";
    _out << sb;
    _out << nl << "var hash = new global::System.HashCode();";
    for(const auto& i : dataMembers)
    {
        _out << nl << "hash.Add(this." << fixId(dataMemberName(i), Slice::ObjectType) << ");";
    }
    _out << nl << "return hash.ToHashCode();";
    _out << eb;

    //
    // Equals implementation
    //
    _out << sp;
    emitGeneratedCodeAttribute();

    _out << nl << "public bool Equals(" << fixId(p->name()) << " other)";

    _out.inc();
    _out << nl << "=> ";
    for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end();)
    {
        string mName = fixId(dataMemberName(*q));
        TypePtr mType = (*q)->type();

        if(isProxyType(mType))
        {
            _out << getUnqualified("Ice.IObjectPrx", ns) << ".Equals(this." << mName << ", other." << mName << ")";
        }
        else
        {
            _out << "this." << mName << " == other." << mName;
        }

        if(++q != dataMembers.end())
        {
            _out << " &&" << nl << "   ";
        }
        else
        {
            _out << ";";
        }
    }
    _out.dec();

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public override bool Equals(object? other)";
    _out << sb;
    _out << nl << "if (object.ReferenceEquals(this, other))";
    _out << sb;
    _out << nl << "return true;";
    _out << eb;
    _out << nl << "return other is " << name << " value && this.Equals(value);";
    _out << eb;

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public static bool operator ==(" << name << " lhs, " << name << " rhs)";
    _out << " => lhs.Equals(rhs);";

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public static bool operator !=(" << name << " lhs, " << name << " rhs)";
    _out << " => !lhs.Equals(rhs);";

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public readonly void IceWrite(Ice.OutputStream iceP_ostr)";
    _out << sb;
    for(auto m : dataMembers)
    {
        writeMarshalDataMember(m, fixId(dataMemberName(m)), ns, "iceP_ostr");
    }
    _out << eb;

    _out << eb;

    _out << sp;
    _out << nl << "public static class " << p->name() << "Helper";
    _out << sb;

    _out << sp;
    _out << nl << "public static Ice.OutputStreamWriter<" << name
         << "> IceWriter = (ostr, value) => value.IceWrite(ostr);";

    _out << sp;
    _out << nl << "public static Ice.InputStreamReader<" << name << "> IceReader = (istr) => new " << name << "(istr);";

    _out << eb;
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
    _out << nl << "public static class " << p->name() << "Helper";
    _out << sb;
    if(explicitValue)
    {
        _out << sp;
        _out << nl << "public static readonly global::System.Collections.Generic.HashSet<int> EnumeratorValues = ";
        _out.inc();
        _out << nl << "new global::System.Collections.Generic.HashSet<int>()";
        _out.spar('{');
        for(EnumeratorList::const_iterator en = enumerators.begin(); en != enumerators.end(); ++en)
        {
            _out << (*en)->value();
        }
        _out.epar('}');
        _out << ";";
        _out.dec();
    }

    _out << sp;
    _out << nl << "public static void Write(this Ice.OutputStream ostr, " << name
         << " value) => ostr.WriteSize((int)value);";

    _out << sp;
    _out << nl << "public static readonly Ice.OutputStreamWriter<" << name << "> IceWriter = Write;";

    _out << sp;
    _out << nl << "public static " << name << " Read" << p->name() << "(this Ice.InputStream istr)";
    _out << sb;
    _out << nl << "int value = istr.ReadSize();";
    if(explicitValue)
    {
        _out << nl << "if(!EnumeratorValues.Contains(value))";
    }
    else
    {
        _out << nl << "if(value < 0 || value > " << p->maxValue() << ")";
    }
    _out << sb;
    _out << nl << "throw new Ice.InvalidDataException($\"invalid enumerator value `{value}' for "
        << fixId(p->scoped()) << "\");";
    _out << eb;
    _out << nl << "return (" << name << ") value;";
    _out << eb;

    _out << sp;
    _out << nl << "public static readonly Ice.InputStreamReader<" << name << "> IceReader = Read" << p->name() << ";";

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
    ContainedPtr cont = ContainedPtr::dynamicCast(p->container());
    assert(cont);

    _out << sp;

    bool readonly = StructPtr::dynamicCast(cont) && cont->hasMetaData("cs:readonly");

    writeTypeDocComment(p, getDeprecateReason(p, cont, "member"));
    emitDeprecate(p, cont, _out, "member");
    emitAttributes(p);
    emitGeneratedCodeAttribute();
    _out << nl << "public ";
    if(readonly)
    {
        _out << "readonly ";
    }
    _out << typeToString(p->type(), getNamespace(cont), p->tagged());
    if(isNullable(p->type()) && !p->tagged())
    {
        _out << "?";
    }
    _out << " " << fixId(dataMemberName(p), ExceptionPtr::dynamicCast(cont) ? Slice::ExceptionType : Slice::ObjectType);
    if(cont->hasMetaData("cs:property"))
    {
        _out << "{ get; set; }";
    }
    else
    {
        _out << ";";
    }
}

Slice::Gen::ProxyVisitor::ProxyVisitor(IceUtilInternal::Output& out) :
    CsVisitor(out)
{
}

bool
Slice::Gen::ProxyVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasInterfaceDefs())
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
    if(!p->isInterface())
    {
        return false;
    }

    string name = p->name();
    string ns = getNamespace(p);

    _out << sp;
    writeTypeDocComment(p, getDeprecateReason(p, 0, "interface"));
    emitGeneratedCodeAttribute();
    emitTypeIdAttribute(p->scoped());
    _out << nl << "public interface " << interfaceName(p) << "Prx : ";

    vector<string> baseInterfaces =
        mapfn<ClassDefPtr>(p->bases(), [&ns](const auto& c)
                           {
                               return getUnqualified(getNamespace(c) + "." +
                                                     interfaceName(c) + "Prx", ns);
                           });

    if(baseInterfaces.empty())
    {
        baseInterfaces.push_back(getUnqualified("Ice.IObjectPrx", ns));
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
Slice::Gen::ProxyVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    string ns = getNamespace(p);
    ClassList bases = p->bases();

    string name = interfaceName(p) + "Prx";
    //
    // Proxy static methods
    //
    _out << sp;
    _out << nl << "public static readonly new Ice.ProxyFactory<" << name << "> Factory =";
    _out.inc();
    _out << nl << "(reference) => new _" << p->name() << "Prx(reference);";
    _out.dec();

    _out << sp;
    _out << nl << "public static readonly new Ice.InputStreamReader<" << name << "?> IceReader =";
    _out.inc();
    _out << nl << "(istr) => istr.ReadProxy(Factory);";
    _out.dec();

    _out << sp;
    _out << nl << "public static readonly new Ice.OutputStreamWriter<" << name << "?> IceWriter =";
    _out.inc();
    _out << nl << "(ostr, value) => ostr.WriteProxy(value);";
    _out.dec();

    _out << sp;
    _out << nl << "public static new " << name << " Parse("
         << "string s, "
         << getUnqualified("Ice.Communicator", ns) << " communicator) => "
         << "new _" << p->name() << "Prx(communicator.CreateReference(s));";

    _out << sp;
    _out << nl << "public static bool TryParse("
         << "string s, "
         << getUnqualified("Ice.Communicator", ns) << " communicator, "
         << "out " <<name << "? prx)";
    _out << sb;
    _out << nl << "try";
    _out << sb;
    _out << nl << "prx = new _" << p->name() << "Prx(communicator.CreateReference(s));";
    _out << eb;
    _out << nl << "catch (global::System.Exception)";
    _out << sb;
    _out << nl << "prx = null;";
    _out << nl << "return false;";
    _out << eb;
    _out << nl << "return true;";
    _out << eb;

    _out << sp;
    _out << nl << "public static ";
    if(!bases.empty())
    {
        _out << "new ";
    }
    _out << name << " UncheckedCast(" << getUnqualified("Ice.IObjectPrx", ns) << " prx) => new _" << p->name()
        << "Prx(prx.IceReference);";

    _out << sp;
    _out << nl << "public static ";
    if(!bases.empty())
    {
        _out << "new ";
    }
    _out << name << "? CheckedCast("
         << getUnqualified("Ice.IObjectPrx", ns) << " prx, "
         << "global::System.Collections.Generic.IReadOnlyDictionary<string, string>? context = null)";
    _out << sb;

    _out << nl << "if(prx.IceIsA(global::Ice.TypeExtensions.GetIceTypeId(typeof(" << interfaceName(p)
         << "Prx))!, context))";
    _out << sb;
    _out << nl << "return new _" << p->name() << "Prx(prx.IceReference);";
    _out << eb;
    _out << nl << "else";
    _out << sb;
    _out << nl << "return null;";
    _out << eb;

    _out << eb;

    _out << eb;

    //
    // Proxy instance
    //
    _out << sp;
    _out << nl << "[global::System.Serializable]";
    _out << nl << "internal sealed class _" << p->name() << "Prx : " << getUnqualified("Ice.ObjectPrx", ns) << ", "
         << name;
    _out << sb;

    _out << nl << "private _" << p->name() << "Prx("
         << "global::System.Runtime.Serialization.SerializationInfo info, "
         << "global::System.Runtime.Serialization.StreamingContext context)";
    _out.inc();
    _out << nl << ": base(info, context)";
    _out.dec();
    _out << sb;
    _out << eb;

    _out << sp;
    _out << nl << "internal _" << p->name() << "Prx(global::Ice.Reference reference)";
    _out.inc();
    _out << nl << ": base(reference)";
    _out.dec();
    _out << sb;
    _out << eb;

    _out << sp;
    _out << nl << getUnqualified("Ice.IObjectPrx", ns) << " " << getUnqualified("Ice.IObjectPrx", ns)
        << ".IceClone(global::Ice.Reference reference) => new _" << p->name() << "Prx(reference);";

    _out << eb;
}

namespace
{

string
requestType(const list<ParamInfo> inParams, const list<ParamInfo>& outParams)
{
    ostringstream os;
    if(inParams.size() == 0)
    {
        os << "Ice.OutgoingRequestWithEmptyParamList";
        if(outParams.size() > 0)
        {
            os << "<" << toTupleType(outParams) << ">";
        }
    }
    else if(inParams.size() == 1 && (!StructPtr::dynamicCast(inParams.front().type) || inParams.front().tagged))
    {
        os << "Ice.OutgoingRequestWithParam<" << toTupleType(inParams);
        if(outParams.size() > 0)
        {
            os << ", " << toTupleType(outParams);
        }
        os << ">";
    }
    else
    {
        os << "Ice.OutgoingRequestWithStructParam<" << toTupleType(inParams);
        if(outParams.size() > 0)
        {
            os << ", " << toTupleType(outParams);
        }
        os << ">";
    }
    return os.str();
}

}

void
Slice::Gen::ProxyVisitor::visitOperation(const OperationPtr& operation)
{
    list<ParamInfo> outParams = getAllOutParams(operation);
    list<ParamInfo> requiredOutParams;
    list<ParamInfo> taggedOutParams;
    getOutParams(operation, requiredOutParams, taggedOutParams, "iceP_");

    list<ParamInfo> inParams = getAllInParams(operation);
    list<ParamInfo> requiredInParams;
    list<ParamInfo> taggedInParams;
    getInParams(operation, requiredInParams, taggedInParams);

    ClassDefPtr cl = ClassDefPtr::dynamicCast(operation->container());
    string deprecateReason = getDeprecateReason(operation, cl, "operation");

    string ns = getNamespace(cl);
    string opName = operationName(operation);
    string name = fixId(opName);
    string asyncName = opName + "Async";
    string internalName = "_iceI_" + opName + "Async";

    TypePtr ret = operation->returnType();
    string retS = typeToString(operation->returnType(), ns, operation->returnIsTagged());

    string context = getEscapedParamName(operation, "context");
    string cancel = getEscapedParamName(operation, "cancel");
    string progress = getEscapedParamName(operation, "progress");

    string requestProperty = "IceI_" + opName + "Request";
    string requestObject = "_iceI_" + opName + "Request";

    {
        //
        // Write the synchronous version of the operation.
        //
        _out << sp;
        writeOperationDocComment(operation, deprecateReason, false, false);
        if(!deprecateReason.empty())
        {
            _out << nl << "[global::System.Obsolete(\"" << deprecateReason << "\")]";
        }
        _out << nl << resultType(operation, ns, false)  << " " << name << spar
             << getInvocationParams(operation, ns)
             << epar << " =>";
        _out.inc();
        _out << nl << requestProperty << ".Invoke(this, ";
        if(inParams.size() > 0)
        {
            _out << toTuple(inParams) << ", ";
        }
        _out << context << ");";
        _out.dec();
    }

    {
        //
        // Write the async version of the operation
        //
        _out << sp;
        writeOperationDocComment(operation, deprecateReason, false, true);
        if(!deprecateReason.empty())
        {
            _out << nl << "[global::System.Obsolete(\"" << deprecateReason << "\")]";
        }

        _out << nl << resultTask(operation, ns, false) << " "
             << asyncName << spar << getInvocationParamsAMI(operation, ns, true) << epar << " => ";
        _out.inc();
        _out << nl << requestProperty << ".InvokeAsync(this, ";
        if(inParams.size() > 0)
        {
            _out << toTuple(inParams) << ", ";
        }
        _out << context << ", " << progress << ", " << cancel << ");";
        _out.dec();
    }

    outParams = getAllOutParams(operation, "iceP_", true);
    string requestT = requestType(inParams, outParams);
    // Write the static outgoing request instance
    _out << sp;

    _out << nl << "private static " << requestT << "? " << requestObject << ";";
    _out << nl << "private static " << requestT << " " << requestProperty << " =>";
    _out.inc();
    _out << nl << requestObject << " ?\?= new " << requestT << "(";
    _out.inc();
    _out << nl << "operationName: \"" << operation->name() << "\",";
    _out << nl << "idempotent: " << (isIdempotent(operation) ? "true" : "false");
    if(inParams.size() > 0)
    {
        _out << ",";
        _out << nl << "format: " << opFormatTypeToString(operation, ns) << ",";
        _out << nl << "writer: ";
        writeOutgoingRequestWriter(operation);
    }

    if(outParams.size() > 0)
    {
        _out << ",";
        _out << nl << "reader: ";
        writeOutgoingRequestReader(operation);
    }
    _out << ");";
    _out.dec();
    _out.dec();
}

void
Slice::Gen::ProxyVisitor::writeOutgoingRequestWriter(const OperationPtr& operation)
{
    ClassDefPtr cl = ClassDefPtr::dynamicCast(operation->container());
    string ns = getNamespace(cl);

    list<ParamInfo> params = getAllInParams(operation);
    list<ParamInfo> requiredParams;
    list<ParamInfo> taggedParams;
    getInParams(operation, requiredParams, taggedParams, "iceP_");

    bool defaultWriter = params.size() == 1 && !params.front().tagged;
    if(defaultWriter)
    {
        _out << outputStreamWriter(params.front().type, ns);
    }
    else if(params.size() > 1)
    {
        _out << "(" << getUnqualified("Ice.OutputStream", ns) << " ostr, in " << toTupleType(params, "iceP_")
             << " value) =>";
        _out << sb;
        writeMarshalParams(operation, requiredParams, taggedParams, "ostr", "value.");
        _out << eb;
    }
    else
    {
        ParamInfo p = params.front();
        _out << "(" << getUnqualified("Ice.OutputStream", ns) << " ostr, "
             << p.typeStr << " " << (p.param ? fixId("iceP_" + p.param->name()) : fixId("iceP_" + p.name))
             << ") =>";
        _out << sb;
        writeMarshalParams(operation, requiredParams, taggedParams, "ostr");
        _out << eb;
    }
}

void
Slice::Gen::ProxyVisitor::writeOutgoingRequestReader(const OperationPtr& operation)
{
    ClassDefPtr cl = ClassDefPtr::dynamicCast(operation->container());
    string ns = getNamespace(cl);

    list<ParamInfo> params = getAllOutParams(operation);
    list<ParamInfo> requiredParams;
    list<ParamInfo> taggedParams;
    getOutParams(operation, requiredParams, taggedParams, "iceP_");

    bool defaultReader = params.size() == 1 && !params.front().tagged;

    if(defaultReader)
    {
        _out << inputStreamReader(params.front().type, ns);
    }
    else if(params.size() > 0)
    {
        _out << "istr =>";
        _out << sb;
        writeUnmarshalParams(operation, requiredParams, taggedParams);

        if(params.size() == 1)
        {
            ParamInfo p = params.front();
            _out << nl << "return " << (p.param ? fixId("iceP_" +  p.param->name()) : fixId("iceP_" + p.name)) << ";";
        }
        else
        {
            params = getAllOutParams(operation, "iceP_", true);
            _out << nl << "return " << spar << getNames(params) << epar << ";";
        }
        _out << eb;
    }
}

Slice::Gen::HelperVisitor::HelperVisitor(IceUtilInternal::Output& out) :
    CsVisitor(out)
{
}

bool
Slice::Gen::HelperVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasSequences() && !p->hasDictionaries())
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

void
Slice::Gen::HelperVisitor::visitSequence(const SequencePtr& p)
{
    string name = p->name();
    string scope = getNamespace(p);
    string seqS = typeToString(p, scope);
    TypePtr type = p->type();

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public static class " << name << "Helper";
    _out << sb;

    _out << sp;
    _out << nl << "public static void Write(this Ice.OutputStream ostr, " << seqS << " sequence) => ";
    _out.inc();
    _out << nl << sequenceMarshalCode(p, scope, "sequence", "ostr") << ";";
    _out.dec();

    _out << sp;
    _out << nl << "public static readonly Ice.OutputStreamWriter<" << seqS << "> IceWriter = Write;";

    _out << sp;
    _out << nl << "public static " << seqS << " Read" << name << "(this Ice.InputStream istr) => ";
    _out.inc();
    _out << nl << sequenceUnmarshalCode(p, scope, "istr") << ";";
    _out.dec();

    _out << sp;
    _out << nl << "public static readonly Ice.InputStreamReader<" << seqS << "> IceReader = Read" << name << ";";

    _out << eb;
}

void
Slice::Gen::HelperVisitor::visitDictionary(const DictionaryPtr& p)
{
    string ns = getNamespace(p);
    string name = p->name();
    TypePtr key = p->keyType();
    TypePtr value = p->valueType();
    string dictS = typeToString(p, ns);
    string readOnlyDictS = typeToString(p, ns, false, true);
    string generic = p->findMetaDataWithPrefix("cs:generic:");

    _out << sp;
    emitGeneratedCodeAttribute();
    _out << nl << "public static class " << name << "Helper";
    _out << sb;
    _out << nl << "public static void Write(this Ice.OutputStream ostr, "<< readOnlyDictS << " dictionary) => ";
    _out.inc();
    _out << nl << "ostr.WriteDict(dictionary, "
         << outputStreamWriter(key, ns) << ", "
         << outputStreamWriter(value, ns) << ");";
    _out.dec();

    _out << sp;
    _out << nl << "public static readonly Ice.OutputStreamWriter<" << readOnlyDictS << "> IceWriter = Write;";

    _out << sp;
    _out << nl << "public static " << dictS << " Read" << name << "(this Ice.InputStream istr) => ";
    _out.inc();
    if(generic == "SortedDictionary")
    {
        _out << nl << "istr.ReadSortedDict("
             << inputStreamReader(key, ns) << ", "
             << inputStreamReader(value, ns) << ");";
    }
    else
    {
        _out << nl << "istr.ReadDict("
             << inputStreamReader(key, ns) << ", "
             << inputStreamReader(value, ns) << ", "
             << (key->minWireSize() + value->minWireSize()) << ");";
    }
    _out.dec();

    _out << sp;
    _out << nl << "public static readonly Ice.InputStreamReader<" << dictS << "> IceReader = Read" << name << ";";

    _out << eb;
}

Slice::Gen::DispatcherVisitor::DispatcherVisitor(::IceUtilInternal::Output& out) :
    CsVisitor(out)
{
}

bool
Slice::Gen::DispatcherVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasInterfaceDefs())
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
    if(!p->isInterface())
    {
        return false;
    }

    ClassList bases = p->bases();
    string name = interfaceName(p);
    string ns = getNamespace(p);

    _out << sp;
    writeTypeDocComment(p, getDeprecateReason(p, 0, "interface"));
    writeSeeAlsoDocComment(_out, interfaceName(p) + "Prx");
    emitComVisibleAttribute();
    emitGeneratedCodeAttribute();
    emitTypeIdAttribute(p->scoped());
    _out << nl << "public interface " << fixId(name) << " : ";
    if (bases.empty())
    {
        _out << getUnqualified("Ice.IObject", ns);
    }
    else
    {
        for(ClassList::const_iterator q = bases.begin(); q != bases.end();)
        {
            _out << getUnqualified(getNamespace(*q) + "." + interfaceName(*q), ns);
            if(++q != bases.end())
            {
                _out << ", ";
            }
        }
    }

    _out << sb;

    // The _ice prefix is in case the user "extends" the partial generated interface.
    _out << nl << "private static readonly string _iceTypeId = global::Ice.TypeExtensions.GetIceTypeId(typeof("
        << name << "))!;";
    _out << nl
        << "private static readonly string[] _iceAllTypeIds = global::Ice.TypeExtensions.GetAllIceTypeIds(typeof("
        << name << "));";

    for(const auto& op : p->operations())
    {
        writeReturnValueStruct(op);
        writeMethodDeclaration(op);
    }

    _out << sp;
    _out << nl << "string global::Ice.IObject.IceId(global::Ice.Current current) => _iceTypeId;";
    _out << nl << "string[] global::Ice.IObject.IceIds(global::Ice.Current current) => _iceAllTypeIds;";

    _out << sp;
    _out << nl << "global::System.Threading.Tasks.ValueTask<global::Ice.OutgoingResponseFrame> "
        << getUnqualified("Ice.IObject", ns)
        << ".DispatchAsync(global::Ice.IncomingRequestFrame request, global::Ice.Current current)";
    _out.inc();
    _out << nl << " => DispatchAsync(this, request, current);";
    _out.dec();

    _out << sp;
    _out << nl << "// This protected static DispatchAsync allows a derived class to override the instance DispatchAsync";
    _out << nl << "// and reuse the generated implementation.";
    _out << nl << "protected static global::System.Threading.Tasks.ValueTask<global::Ice.OutgoingResponseFrame> "
        << "DispatchAsync(" << fixId(name) << " servant, "
        << "global::Ice.IncomingRequestFrame request, global::Ice.Current current)";
    _out << sb;
    _out << nl << "switch(current.Operation)";
    _out << sb;
    StringList allOpNames;
    for(const auto& op : p->allOperations())
    {
        allOpNames.push_back(op->name());
    }
    allOpNames.push_back("ice_id");
    allOpNames.push_back("ice_ids");
    allOpNames.push_back("ice_isA");
    allOpNames.push_back("ice_ping");

    for(const auto& opName : allOpNames)
    {
        _out << nl << "case \"" << opName << "\":";
        _out << sb;
        _out << nl << "return servant.IceD_" << opName << "Async(request, current);";
        _out << eb;
    }

    _out << nl << "default:";
    _out << sb;
    _out << nl << "throw new " << getUnqualified("Ice.OperationNotExistException", ns)
            << "(current.Identity, current.Facet, current.Operation);";
    _out << eb;

    _out << eb; // switch
    _out << eb; // method

    return true;
}

void
Slice::Gen::DispatcherVisitor::writeReturnValueStruct(const OperationPtr& operation)
{
    ClassDefPtr cl = ClassDefPtr::dynamicCast(operation->container());
    string ns = getNamespace(cl);
    const string opName = pascalCase(operation->name());

    list<ParamInfo> outParams = getAllOutParams(operation, "", true);
    list<ParamInfo> requiredOutParams;
    list<ParamInfo> taggedOutParams;
    getOutParams(operation, requiredOutParams, taggedOutParams, "iceP_");

    if(operation->hasMarshaledResult())
    {
        _out << sp;
        _out << nl << "public struct " << opName << "MarshaledReturnValue";
        _out << sb;
        _out << nl << "public " << getUnqualified("Ice.OutgoingResponseFrame", ns) << " Response { get; }";

        _out << nl << "public " << opName << "MarshaledReturnValue" << spar
             << getNames(outParams, [](const auto& p)
                                    {
                                        return p.typeStr + " iceP_" + p.name;
                                    })
             << (getUnqualified("Ice.Current", ns) + " current")
             << epar;
        _out << sb;
        _out << nl << "Response = " << getUnqualified("Ice.OutgoingResponseFrame", ns) << ".WithReturnValue(";
        _out.inc();
        _out << nl << "current, " << opFormatTypeToString(operation, ns) << ", ";
        _out << nl << toTuple(outParams, "iceP_") << ", ";
        if(outParams.size() > 1)
        {
            _out << nl << "(Ice.OutputStream ostr, " << toTupleType(outParams, "iceP_") << " value) => ";
            _out << sb;
            writeMarshalParams(operation, requiredOutParams, taggedOutParams, "ostr", "value.");
            _out << eb;
        }
        else
        {
            _out << nl << "(ostr, " << getNames(outParams, "iceP_") << ") =>";
            _out << sb;
            writeMarshalParams(operation, requiredOutParams, taggedOutParams, "ostr");
            _out << eb;
        }
        _out << ");";
        _out.dec();
        _out << eb;
        _out << eb;
    }
}

void
Slice::Gen::DispatcherVisitor::writeMethodDeclaration(const OperationPtr& operation)
{
    ClassDefPtr cl = ClassDefPtr::dynamicCast(operation->container());
    string ns = getNamespace(cl);
    string deprecateReason = getDeprecateReason(operation, cl, "operation");
    bool amd = cl->hasMetaData("amd") || operation->hasMetaData("amd");
    const string name = fixId(operationName(operation) + (amd ? "Async" : ""));
    list<ParamInfo> inParams = getAllInParams(operation);

    _out << sp;
    writeOperationDocComment(operation, deprecateReason, true, amd);
    _out << nl << "public ";

    if(amd)
    {
        _out << resultTask(operation, ns, true);
    }
    else
    {
        _out << resultType(operation, ns, true);
    }

    _out << " " << name << spar;
    _out << getNames(inParams, [](const auto& param)
                               {
                                   return param.typeStr + (!param.tagged && isNullable(param.type) ? "?" : "") + " " + param.name;
                               });
    _out << (getUnqualified("Ice.Current", ns) + " " + getEscapedParamName(operation, "current"));
    _out << epar << ';';
}

void
Slice::Gen::DispatcherVisitor::visitOperation(const OperationPtr& operation)
{
    ClassDefPtr cl = ClassDefPtr::dynamicCast(operation->container());
    bool amd = cl->hasMetaData("amd") || operation->hasMetaData("amd");
    string ns = getNamespace(cl);
    string opName = operationName(operation);
    string name = fixId(opName + (amd ? "Async" : ""));
    string internalName = "IceD_" + operation->name() + "Async";

    list<ParamInfo> inParams = getAllInParams(operation, "iceP_");
    list<ParamInfo> requiredInParams;
    list<ParamInfo> taggedInParams;
    getInParams(operation, requiredInParams, taggedInParams, "iceP_");

    list<ParamInfo> outParams = getAllOutParams(operation, "", true);
    list<ParamInfo> requiredOutParams;
    list<ParamInfo> taggedOutParams;
    getOutParams(operation, requiredOutParams, taggedOutParams);

    bool defaultWriter = outParams.size() == 1 && !outParams.front().tagged;
    string writer = defaultWriter ? outputStreamWriter(outParams.front().type, ns) : "_iceD_" + opName + "Writer";

    bool defaultReader = inParams.size() == 1 && !inParams.front().tagged;
    string reader = defaultReader ? inputStreamReader(inParams.front().type, ns) : "_iceD_" + opName + "Reader";

    _out << sp;
    _out << nl << "protected ";
    if (amd)
    {
        _out << "async ";
    }
    _out << "global::System.Threading.Tasks.ValueTask<" + getUnqualified("Ice.OutgoingResponseFrame", ns) + ">";
    _out << " " << internalName << "(global::Ice.IncomingRequestFrame request, " << getUnqualified("Ice.Current", ns)
         << " current)";
    _out << sb;

    if (!isIdempotent(operation))
    {
         _out << nl << "IceCheckNonIdempotent(current);";
    }

    // Even when the parameters are empty, we verify we could read the data. Note that EndEncapsulation
    // skips tagged members, and needs to understand them.
    if(inParams.empty())
    {
        _out << nl << "request.ReadEmptyParamList();";
    }
    else if(inParams.size() == 1)
    {
        _out << nl << "var " << inParams.front().name << " = request.ReadParamList(" << reader << ");";
    }
    else
    {
        _out << nl << "var paramList = request.ReadParamList(" << reader << ");";
    }

    // The 'this.' is necessary only when the operation name matches one of our local variable (current, istr etc.)

    if(operation->hasMarshaledResult())
    {
        if (amd)
        {
            _out << nl << "var result = await this." << name << spar << getNames(inParams) << "current" << epar
                << ".ConfigureAwait(false);";
            _out << nl << "return result.Response;";
        }
        else
        {
            _out << nl << "return new global::System.Threading.Tasks.ValueTask<global::Ice.OutgoingResponseFrame>(this."
                 << name << spar << getNames(inParams) << "current" << epar << ".Response);";
        }
        _out << eb;
    }
    else
    {
        _out << nl;
        if(outParams.size() >= 1)
        {
            _out << "var result = ";
        }

        if (amd)
        {
            _out << "await ";
        }
        _out << "this." << name << spar << getNames(inParams, inParams.size() > 1 ? "paramList." : "") << "current"
             << epar;
        if (amd)
        {
            _out << ".ConfigureAwait(false)";
        }
        _out << ";";

        if(outParams.size() == 0)
        {
            if(amd)
            {
                _out << nl << "return global::Ice.OutgoingResponseFrame.WithVoidReturnValue(current);";
            }
            else
            {
                _out << nl << "return new global::System.Threading.Tasks.ValueTask<global::Ice.OutgoingResponseFrame>(";
                _out.inc();
                _out << nl << "global::Ice.OutgoingResponseFrame.WithVoidReturnValue(current));";
                _out.dec();
            }
        }
        else
        {
            _out << nl << "var response = " << getUnqualified("Ice.OutgoingResponseFrame", ns)
                << ".WithReturnValue(current, " << opFormatTypeToString(operation, ns)
                << ", result, " << writer << ");";

            if(amd)
            {
                _out << nl << "return response;";
            }
            else
            {
                _out << nl << "return new global::System.Threading.Tasks.ValueTask<global::Ice.OutgoingResponseFrame>("
                     << "response);";
            }
        }
        _out << eb;
    }

    // Write the output stream writer used to fill the request frame
    if(outParams.size() > 1)
    {
        _out << sp;
        _out << nl << "private static void " << writer << "(" << getUnqualified("Ice.OutputStream", ns) << " ostr, "
             << toTupleType(outParams) << " value)";
        _out << sb;
        writeMarshalParams(operation, requiredOutParams, taggedOutParams, "ostr", "value.");
        _out << eb;
    }
    else if(outParams.size() == 1 && !defaultWriter)
    {
        auto param = outParams.front();
        _out << sp;
        _out << nl << "private static void " << writer << "(" << getUnqualified("Ice.OutputStream", ns) << " ostr, "
             << param.typeStr << " " << param.name << ")";
        _out << sb;
        writeMarshalParams(operation, requiredOutParams, taggedOutParams, "ostr");
        _out << eb;
    }

    if(inParams.size() > 1)
    {
        _out << sp;
        _out << nl << "private static readonly global::Ice.InputStreamReader<" << toTupleType(inParams, "iceP_") << "> "
             << reader << " = istr =>";
        _out << sb;
        writeUnmarshalParams(operation, requiredInParams, taggedInParams);
        _out << nl << "return " << toTuple(inParams, "iceP_") << ";";
        _out << eb << ";";
    }
    else if(inParams.size() == 1 && inParams.front().tagged)
    {
        _out << sp;
        _out << nl << "private static readonly global::Ice.InputStreamReader<" << inParams.front().typeStr << "> "
             << reader << " = istr =>";
        _out << sb;
        writeUnmarshalParams(operation, requiredInParams, taggedInParams);
        _out << nl << "return " << inParams.front().name << ";";
        _out << eb << ";";
    }
}

void
Slice::Gen::DispatcherVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _out << eb; // interface
}

Slice::Gen::ImplVisitor::ImplVisitor(IceUtilInternal::Output& out) :
    CsVisitor(out)
{
}

bool
Slice::Gen::ImplVisitor::visitModuleStart(const ModulePtr& p)
{
    if(!p->hasInterfaceDefs())
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
    if(p->isInterface())
    {
        _out << sp << nl << "public class " << p->name() << "I : " << fixId(p->name());
        _out << sb;
    }
    return p->isInterface();
}

void
Slice::Gen::ImplVisitor::visitOperation(const OperationPtr& op)
{
    ClassDefPtr cl = ClassDefPtr::dynamicCast(op->container());
    string ns = getNamespace(cl);
    string opName = operationName(op);

    list<ParamInfo> outParams = getAllOutParams(op);

    _out << sp << nl;

    if(cl->hasMetaData("amd") || op->hasMetaData("amd"))
    {
        _out << "public override " << resultTask(op, ns, true) << " " << opName << "Async" << spar
             << getNames(getAllInParams(op))
             << (getUnqualified("Ice.Current", ns) + " " + getEscapedParamName(op, "current"))
             << epar;
        _out << sb;

        for(const auto& p : outParams)
        {
            _out << nl << p.typeStr << " " << p.name << " = " << writeValue(p.type, ns) << ';';
        }

        if(outParams.size() == 0)
        {
            _out << nl << "global::System.Threading.Tasks.Task.CompletedTask;";
        }
        else if(op->hasMarshaledResult() || outParams.size() > 1)
        {
            _out << nl << "return new " << opName << (op->hasMarshaledResult() ? "MarshaledResult" : "Result")
                 << spar
                 << getNames(getAllOutParams(op, "", true));
            if(op->hasMarshaledResult())
            {
                _out << (getUnqualified("Ice.Current", ns) + " " + getEscapedParamName(op, "current"));
            }
            _out << epar << ";";
        }
        else
        {
            _out << nl << "return " << outParams.front().name << ";";
        }
        _out << eb;
    }
    else
    {
        _out << "public override " << resultType(op, ns, true) << " " << opName << spar
             << getNames(getAllInParams(op))
             << (getUnqualified("Ice.Current", ns) + " " + getEscapedParamName(op, "current"))
             << epar;
        _out << sb;

        if(op->hasMarshaledResult())
        {
            _out << nl << "return new " << opName << "MarshaledResult"
                 << spar
                 << getNames(getAllOutParams(op, "", true))
                 << (getUnqualified("Ice.Current", ns) + " " + getEscapedParamName(op, "current"))
                 << epar << ";";
        }
        else
        {
            for(const auto& p : outParams)
            {
                if(p.param)
                {
                    _out << nl << p.name << " = " << writeValue(p.type, ns) << ';';
                }
                else
                {
                    _out << nl << "return " << writeValue(p.type, ns) << ';';
                }
            }
        }
        _out << eb;
    }
}

void
Slice::Gen::ImplVisitor::visitClassDefEnd(const ClassDefPtr&)
{
    _out << eb;
}
