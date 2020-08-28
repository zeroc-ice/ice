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
//
// Returns the namespace of a Contained entity.
//
std::string getNamespace(const ContainedPtr&);
//
// Returns the namespace prefix of a Contained entity.
//
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
std::string interfaceName(const InterfaceDeclPtr&);
std::string interfaceName(const InterfaceDefPtr&);

std::string helperName(const TypePtr&, const std::string&);

std::string builtinSuffix(const BuiltinPtr&);

std::string returnTypeStr(const OperationPtr&, const std::string& ns, bool dispatch);
std::string returnTaskStr(const OperationPtr&, const std::string& ns, bool dispatch);

bool isCollectionType(const TypePtr&);
bool isValueType(const TypePtr&); // value with C# "struct" meaning
bool isReferenceType(const TypePtr&); // opposite of value
bool isMappedToReadOnlyMemory(const SequencePtr& seq);

std::vector<std::string> getNames(const MemberList& params, const std::string& prefix = "");
std::vector<std::string> getNames(const MemberList&, std::function<std::string (const MemberPtr&)>);

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

    //
    // Validate all metadata in the unit with a "cs:" prefix.
    //
    static void validateMetaData(const UnitPtr&);

    static std::string typeToString(const TypePtr& type, const std::string& package, bool readOnly = false);

protected:

    //
    // Generate code to marshal or unmarshal a type
    //
    std::string outputStreamWriter(const TypePtr&, const std::string&, bool);
    void writeMarshalCode(::IceUtilInternal::Output&, const TypePtr&, int&, bool, const std::string&,
                          const std::string&, const std::string& = "ostr");

    std::string inputStreamReader(const TypePtr&, const std::string&);
    void writeUnmarshalCode(::IceUtilInternal::Output&, const TypePtr&, int&, const std::string&, const std::string&,
                            const std::string& = "istr");

    void writeTaggedMarshalCode(::IceUtilInternal::Output&, const OptionalPtr&, bool, const std::string&,
                                const std::string&, int, const std::string& = "ostr");
    void writeTaggedUnmarshalCode(::IceUtilInternal::Output&, const OptionalPtr&, const std::string&,
                                  const std::string&, int, const MemberPtr&, const std::string& = "istr");

    std::string sequenceMarshalCode(const SequencePtr&, const std::string&, const std::string&, const std::string&);
    std::string sequenceUnmarshalCode(const SequencePtr&, const std::string&, const std::string&);

    void writeConstantValue(::IceUtilInternal::Output&, const TypePtr&, const SyntaxTreeBasePtr&, const std::string&,
        const std::string& ns);

private:

    class MetaDataVisitor : public ParserVisitor
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
