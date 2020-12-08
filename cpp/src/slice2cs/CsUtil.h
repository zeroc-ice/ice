//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef CS_UTIL_H
#define CS_UTIL_H

#include <Slice/Parser.h>
#include <Slice/Util.h>
#include <IceUtil/OutputUtil.h>
#include <functional>

namespace Slice
{

enum CSharpBaseType { ObjectType = 1, ExceptionType = 2 };

std::string fixId(const std::string&, unsigned int = 0);

// Returns the namespace of a Contained entity.
std::string getNamespace(const ContainedPtr&);

// Returns the namespace prefix of a Contained entity.
std::string getNamespacePrefix(const ContainedPtr&);

std::string getUnqualified(const std::string&, const std::string&, bool builtin = false);
std::string getUnqualified(const ContainedPtr&,
                           const std::string& package = "",
                           const std::string& prefix = "",
                           const std::string& suffix = "");

bool normalizeCase(const ContainedPtr&);
std::string operationName(const OperationPtr&);
std::string paramName(const MemberPtr& param, const std::string& prefix = "");
std::string paramTypeStr(const MemberPtr& param, bool readOnly = false);

std::string fieldName(const MemberPtr&);
std::string interfaceName(const InterfaceDeclPtr&, bool isAsync = false);
std::string interfaceName(const InterfaceDefPtr&, bool isAsync = false);

std::string helperName(const TypePtr&, const std::string&);

std::string builtinSuffix(const BuiltinPtr&);

std::string returnTypeStr(const OperationPtr& operation, const std::string& ns, bool dispatch);
std::string returnTaskStr(const OperationPtr& operation, const std::string& ns, bool dispatch);

bool isCollectionType(const TypePtr&);
bool isValueType(const TypePtr&); // value with C# "struct" meaning
bool isReferenceType(const TypePtr&); // opposite of value
bool isFixedSizeNumericSequence(const SequencePtr& seq);

std::vector<std::string> getNames(const MemberList& params, const std::string& prefix = "");
std::vector<std::string> getNames(const MemberList& params, std::function<std::string (const MemberPtr&)> fn);

std::string toTuple(const MemberList& params, const std::string& prefix = "");
std::string toTupleType(const MemberList& params, bool readOnly);

template<typename T> inline std::vector<std::string>
mapfn(const std::list<T>& items, std::function<std::string (const T&)> fn)
{
    std::vector<std::string> result;
    for(const auto& item : items)
    {
        result.push_back(fn(item));
    }
    return result;
}

class CsGenerator : private ::IceUtil::noncopyable
{
public:

    virtual ~CsGenerator() {};

    // Validate all metadata in the unit with a "cs:" prefix.
    static void validateMetadata(const UnitPtr&);

    static std::string typeToString(
        const TypePtr& type,
        const std::string& package,
        bool readOnly = false,
        bool readOnlyParam = false,
        bool streamParam = false);

protected:

    std::string outputStreamWriter(
        const TypePtr& type,
        const std::string& scope,
        bool readOnly = false,
        bool readOnlyParam = false);

    void writeMarshalCode(
        ::IceUtilInternal::Output& out,
        const TypePtr& type,
        int& bitSequenceIndex,
        bool forNestedType,
        const std::string& scope,
        const std::string& param);

    std::string inputStreamReader(const TypePtr& type, const std::string& scope);

    std::string streamDataReader(const TypePtr& type);

    std::string streamDataWriter(const TypePtr& type);

    void writeUnmarshalCode(
        ::IceUtilInternal::Output& out,
        const TypePtr& type,
        int& bitSequenceIndex,
        const std::string& scope,
        const std::string& param);

    void writeTaggedMarshalCode(
        ::IceUtilInternal::Output& out,
        const OptionalPtr& optionalType,
        bool isDataMember,
        const std::string& scope,
        const std::string& param,
        int tag);

    void writeTaggedUnmarshalCode(
        ::IceUtilInternal::Output& out,
        const OptionalPtr& optionaType,
        const std::string& scope,
        const std::string& param,
        int tag,
        const MemberPtr& dataMember);

    void writeConstantValue(
        ::IceUtilInternal::Output& out,
        const TypePtr& type,
        const SyntaxTreeBasePtr& valueType,
        const std::string& value,
        const std::string& scope);

private:

    std::string sequenceMarshalCode(
        const SequencePtr& seq,
        const std::string& scope,
        const std::string& value,
        bool readOnly = false,
        bool readOnlyParam = false);

    std::string sequenceUnmarshalCode(const SequencePtr& seq, const std::string& scope);

    std::string dictionaryMarshalCode(
        const DictionaryPtr& dict,
        const std::string& scope,
        const std::string& param);

    std::string dictionaryUnmarshalCode(const DictionaryPtr& dict, const std::string& scope);
    class MetadataVisitor : public ParserVisitor
    {
    public:

        bool visitUnitStart(const UnitPtr&) override;
        bool visitModuleStart(const ModulePtr&) override;
        void visitModuleEnd(const ModulePtr&) override;
        void visitClassDecl(const ClassDeclPtr&) override;
        bool visitClassDefStart(const ClassDefPtr&) override;
        void visitClassDefEnd(const ClassDefPtr&) override;
        bool visitExceptionStart(const ExceptionPtr&) override;
        void visitExceptionEnd(const ExceptionPtr&) override;
        bool visitStructStart(const StructPtr&) override;
        void visitStructEnd(const StructPtr&) override;
        void visitOperation(const OperationPtr&) override;
        void visitParameter(const MemberPtr&) override;
        void visitDataMember(const MemberPtr&) override;
        void visitSequence(const SequencePtr&) override;
        void visitDictionary(const DictionaryPtr&) override;
        void visitEnum(const EnumPtr&) override;
        void visitConst(const ConstPtr&) override;

    private:

        void validate(const ContainedPtr&);

        std::string _fileName;
    };
};

}

#endif
