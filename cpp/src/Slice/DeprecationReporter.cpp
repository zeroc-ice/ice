// Copyright (c) ZeroC, Inc.

#include "DeprecationReporter.h"

#include <sstream>

using namespace std;
using namespace Slice;

namespace
{
    class DeprecationVisitor final : public ParserVisitor
    {
    public:
        bool visitInterfaceDefStart(const InterfaceDefPtr& interfaceDef) final;
        bool visitClassDefStart(const ClassDefPtr& classDef) final;
        bool visitExceptionStart(const ExceptionPtr& exceptionDef) final;
        bool visitStructStart(const StructPtr& structDef) final;
        void visitOperation(const OperationPtr& operation) final;
        void visitDataMember(const DataMemberPtr& member) final;
        void visitSequence(const SequencePtr& sequence) final;
        void visitDictionary(const DictionaryPtr& dictionary) final;
        void visitConst(const ConstPtr& constDef) final;
    };

    /// Checks if the provided Slice element is deprecated. If it is, this will emit a deprecation warning.
    /// @param p The Slice element to check for deprecation.
    /// @param source This element's file and line number will be used in the warning message. Can be the same as @p p.
    /// @param usePhrase A short description of how @p p is being used in context. Only used to emit better warnings.
    void reportIfDeprecated(const SyntaxTreeBasePtr& p, const ContainedPtr& source, string_view usePhrase = "uses")
    {
        if (auto contained = dynamic_pointer_cast<Contained>(p))
        {
            if (contained->isDeprecated())
            {
                ostringstream msg;
                msg << source->kindOf() << " '" << source->name() << "' " << usePhrase << " deprecated "
                    << contained->kindOf() << " '" << contained->name() + "'";
                p->unit()->warning(source->file(), source->line(), Deprecated, msg.str());
            }
        }
    }
}

void
Slice::emitDeprecationWarningsFor(const UnitPtr& unit)
{
    DeprecationVisitor visitor;
    unit->visit(&visitor);
}

bool
DeprecationVisitor::visitInterfaceDefStart(const InterfaceDefPtr& interfaceDef)
{
    // If the interface itself is deprecated, we don't need to check its bases.
    if (!interfaceDef->isDeprecated())
    {
        for (const auto& base : interfaceDef->bases())
        {
            reportIfDeprecated(base, interfaceDef, "inherits from");
        }
    }

    // Always check operations, regardless of whether or not the interface is deprecated.
    return true;
}

bool
DeprecationVisitor::visitClassDefStart(const ClassDefPtr& classDef)
{
    // If the class itself is deprecated, we don't need to check its base or fields.
    if (classDef->isDeprecated())
    {
        return false;
    }

    reportIfDeprecated(classDef->base(), classDef, "inherits from");

    return true;
}

bool
DeprecationVisitor::visitExceptionStart(const ExceptionPtr& exceptionDef)
{
    // If the exception itself is deprecated, we don't need to check its base or fields.
    if (exceptionDef->isDeprecated())
    {
        return false;
    }

    reportIfDeprecated(exceptionDef->base(), exceptionDef, "inherits from");

    return true;
}

bool
DeprecationVisitor::visitStructStart(const StructPtr& structDef)
{
    // If the struct itself is deprecated, we don't need to check its fields.
    return !structDef->isDeprecated();
}

void
DeprecationVisitor::visitOperation(const OperationPtr& operation)
{
    // If the operation itself is deprecated, we don't need to check its types.
    if (operation->isDeprecated())
    {
        return;
    }

    reportIfDeprecated(operation->returnType(), operation, "returns");
    for (const auto& parameter : operation->parameters())
    {
        reportIfDeprecated(parameter->type(), parameter);
    }
    for (const auto& exceptionRef : operation->throws())
    {
        reportIfDeprecated(exceptionRef, operation, "can throw");
    }
}

void
DeprecationVisitor::visitDataMember(const DataMemberPtr& member)
{
    // If the member itself is deprecated, we don't need to check its types.
    if (member->isDeprecated())
    {
        return;
    }

    reportIfDeprecated(member->type(), member);
    reportIfDeprecated(member->defaultValueType(), member);
}

void
DeprecationVisitor::visitSequence(const SequencePtr& sequence)
{
    // If the sequence itself is deprecated, we don't need to check its type.
    if (sequence->isDeprecated())
    {
        return;
    }

    reportIfDeprecated(sequence->type(), sequence);
}

void
DeprecationVisitor::visitDictionary(const DictionaryPtr& dictionary)
{
    // If the dictionary itself is deprecated, we don't need to check its type.
    if (dictionary->isDeprecated())
    {
        return;
    }

    reportIfDeprecated(dictionary->keyType(), dictionary);
    reportIfDeprecated(dictionary->valueType(), dictionary);
}

void
DeprecationVisitor::visitConst(const ConstPtr& constDef)
{
    // If the constant itself is deprecated, we don't need to check its types.
    if (constDef->isDeprecated())
    {
        return;
    }

    reportIfDeprecated(constDef->type(), constDef);
    reportIfDeprecated(constDef->valueType(), constDef);
}
