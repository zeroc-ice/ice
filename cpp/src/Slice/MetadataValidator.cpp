// Copyright (c) ZeroC, Inc.

#include "MetadataValidator.h"

#include <cassert>

using namespace std;
using namespace Slice;

namespace
{
    optional<string> validateAmd(const SyntaxTreeBasePtr& p, const MetadataPtr& metadata)
    {
        if (!dynamic_pointer_cast<InterfaceDef>(p) && !dynamic_pointer_cast<Operation>(p))
        {
            return "the 'amd' metadata can only be applied to interfaces and operations";
        }

        if (!metadata->arguments().empty())
        {
            return "the 'amd' metadata does not take any arguments";
        }

        return nullopt;
    }

    optional<string> validateDeprecated(const SyntaxTreeBasePtr& p, const MetadataPtr&)
    {
        if (dynamic_pointer_cast<Unit>(p))
        {
            return "the 'deprecated' metadata cannot be specified as file metadata";
        }

        if (dynamic_pointer_cast<Builtin>(p))
        {
            return "the 'deprecated' metadata cannot be applied to builtin types";
        }

        if (dynamic_pointer_cast<Module>(p) || dynamic_pointer_cast<ParamDecl>(p))
        {
            const string kind = dynamic_pointer_cast<Contained>(p)->kindOf();
            return "the 'deprecated' metadata cannot be applied to " + kind + "s";
        }

        return nullopt;
    }

    optional<string> validateFormat(const SyntaxTreeBasePtr& p, const MetadataPtr& metadata)
    {
        if (dynamic_pointer_cast<Operation>(p))
        {
            return "the 'format' metadata can only be applied to operations";
        }

        const string& arguments = metadata->arguments();
        if (arguments != "compact" && arguments != "sliced" && arguments != "default")
        {
            return "invalid argument '" + arguments + "' supplied to 'format' metadata" +
                   "\nonly the following formats are valid: 'compact', 'sliced', 'default'";
        }

        return nullopt;
    }

    optional<string> validateMarshaledResult(const SyntaxTreeBasePtr& p, const MetadataPtr& metadata)
    {
        if (!dynamic_pointer_cast<InterfaceDef>(p) && !dynamic_pointer_cast<Operation>(p))
        {
            return "the 'marshaled-result' metadata can only be applied to interfaces and operations";
        }

        if (!metadata->arguments().empty())
        {
            return "the 'marshaled-result' metadata does not take any arguments";
        }

        return nullopt;
    }

    optional<string> validateProtected(const SyntaxTreeBasePtr& p, const MetadataPtr& metadata)
    {
        if (!dynamic_pointer_cast<DataMember>(p) && !dynamic_pointer_cast<ClassDef>(p) &&
            !dynamic_pointer_cast<Struct>(p) && !dynamic_pointer_cast<Slice::Exception>(p))
        {
            return "the 'protected' metadata can only be applied to data members, classes, structs, and exceptions";
        }

        if (!metadata->arguments().empty())
        {
            return "the 'protected' metadata does not take any arguments";
        }

        return nullopt;
    }

    optional<string> validateSuppressWarning(const SyntaxTreeBasePtr& p, const MetadataPtr& metadata)
    {
        if (!dynamic_pointer_cast<Unit>(p))
        {
            return "the 'suppress-warning' metadata can only be specified as file metadata. Ex: [[suppress-warning]]";
        }

        const string& arguments = metadata->arguments();
        if (arguments != "" && arguments != "all" && arguments != "deprecated" && arguments != "invalid-metadata")
        {
            return "invalid category '" + arguments + "' supplied to 'suppress-warning' metadata" +
                   "\nonly the following categories are valid: 'all', 'deprecated', 'invalid-metadata'";
        }

        return nullopt;
    }

    // TODO: we should probably just remove this metadata. It's only checked by slice2java,
    // and there's already a 'java:UserException' metadata that we also check... better to only keep that one.
    optional<string> validateUserException(const SyntaxTreeBasePtr& p, const MetadataPtr& metadata)
    {
        if (!dynamic_pointer_cast<Operation>(p))
        {
            return "the 'UserException' metadata can only be applied to operations";
        }

        if (!metadata->arguments().empty())
        {
            return "the 'UserException' metadata does not take any arguments";
        }

        return nullopt;
    }
}

Slice::MetadataValidator::MetadataValidator(string language, map<string, ValidationFunc> validators)
    : _language(std::move(language)),
      _validationFunctions(std::move(validators))
{
    // Ensures that we fully match language prefixes instead of hitting false positives.
    _language += ":";

    // Add validation functions for parser metadata.
    _validationFunctions.emplace("amd", validateAmd);
    _validationFunctions.emplace("deprecate", validateDeprecated);
    _validationFunctions.emplace("deprecated", validateDeprecated);
    _validationFunctions.emplace("format", validateFormat);
    _validationFunctions.emplace("marshaled-result", validateMarshaledResult);
    _validationFunctions.emplace("protected", validateProtected);
    _validationFunctions.emplace("suppress-warning", validateSuppressWarning);
    _validationFunctions.emplace("UserException", validateUserException);
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
        // Or metadata that doesn't have a language prefix (parser metadata).
        const string& directive = meta->directive();
        if (directive.find(_language) == 0 || directive.find(':') == string::npos)
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
