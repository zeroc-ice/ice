// Copyright (c) ZeroC, Inc.

#include "MetadataValidator.h"

#include <cassert>

using namespace std;
using namespace Slice;

Slice::MetadataValidator::MetadataValidator(string language, map<string, ValidationFunc> validators)
    : _language(std::move(language)), _validationFunctions(std::move(validators))
{
    if (!_language.empty())
    {
        _language += ":";
    }
}

bool
Slice::MetadataValidator::visitUnitStart(const UnitPtr& p)
{
    DefinitionContextPtr dc = p->findDefinitionContext(p->topLevelFile());
    assert(dc);
    dc->setMetadata(validateMetadata(p, dc->getMetadata()));
    return true;
}

bool
Slice::MetadataValidator::visitModuleStart(const ModulePtr& p)
{
    p->setMetadata(validateMetadata(p, p->getMetadata()));
    return true;
}

void
Slice::MetadataValidator::visitClassDecl(const ClassDeclPtr& p)
{
    p->setMetadata(validateMetadata(p, p->getMetadata()));
}

bool
Slice::MetadataValidator::visitClassDefStart(const ClassDefPtr& p)
{
    p->setMetadata(validateMetadata(p, p->getMetadata()));
    return true;
}

void
Slice::MetadataValidator::visitInterfaceDecl(const InterfaceDeclPtr& p)
{
    p->setMetadata(validateMetadata(p, p->getMetadata()));
}

bool
Slice::MetadataValidator::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    p->setMetadata(validateMetadata(p, p->getMetadata()));
    return true;
}

bool
Slice::MetadataValidator::visitExceptionStart(const ExceptionPtr& p)
{
    p->setMetadata(validateMetadata(p, p->getMetadata()));
    return true;
}

bool
Slice::MetadataValidator::visitStructStart(const StructPtr& p)
{
    p->setMetadata(validateMetadata(p, p->getMetadata()));
    return true;
}

void
Slice::MetadataValidator::visitOperation(const OperationPtr& p)
{
    p->setMetadata(validateMetadata(p, p->getMetadata()));
}

void
Slice::MetadataValidator::visitParamDecl(const ParamDeclPtr& p)
{
    p->setMetadata(validateMetadata(p, p->getMetadata()));
}

void
Slice::MetadataValidator::visitDataMember(const DataMemberPtr& p)
{
    p->setMetadata(validateMetadata(p, p->getMetadata()));
}

void
Slice::MetadataValidator::visitSequence(const SequencePtr& p)
{
    p->setMetadata(validateMetadata(p, p->getMetadata()));
    p->setTypeMetadata(validateMetadata(p->type(), p->typeMetadata()));
}

void
Slice::MetadataValidator::visitDictionary(const DictionaryPtr& p)
{
    p->setMetadata(validateMetadata(p, p->getMetadata()));
    p->setKeyMetadata(validateMetadata(p->keyType(), p->keyMetadata()));
    p->setValueMetadata(validateMetadata(p->valueType(), p->valueMetadata()));
}

void
Slice::MetadataValidator::visitEnum(const EnumPtr& p)
{
    p->setMetadata(validateMetadata(p, p->getMetadata()));
}

void
Slice::MetadataValidator::visitConst(const ConstPtr& p)
{
    p->setMetadata(validateMetadata(p, p->getMetadata()));
    p->setTypeMetadata(validateMetadata(p->type(), p->typeMetadata()));
}

MetadataList
Slice::MetadataValidator::validateMetadata(const SyntaxTreeBasePtr& p, MetadataList metadata) const
{
    for (MetadataList::const_iterator i = metadata.begin(); i != metadata.end(); ++i)
    {
        const MetadataPtr& meta = *i++;

        // We only check metadata that starts with the specified language prefix.
        // Or if the language prefix is empty, then we only check metadata without a language prefix (parser metadata).
        const string& directive = meta->directive();
        if (directive.find(_language) == 0 || (_language.empty() && directive.find(':') == string::npos))
        {
            // If there is a validation function for the directive run it. Otherwise report unknown metadata.
            optional<string> warningMessage;
            auto result = _validationFunctions.find(directive);
            if (result != _validationFunctions.end())
            {
                warningMessage = (result->second)(p, meta);
            }
            else
            {
                warningMessage = "ignoring unknown metadata directive: '" + directive + "'";
            }

            // If a warning message was issued for this metadata, remove the bad metadata, and emit the warning.
            if (auto message = warningMessage)
            {
                metadata.remove(meta);
                p->unit()->warning(meta->file(), meta->line(), InvalidMetadata, *message);
            }
        }
    }

    return metadata;
}
