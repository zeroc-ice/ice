// Copyright (c) ZeroC, Inc.

#include "MetadataValidator.h"

#include <cassert>

using namespace std;
using namespace Slice;

namespace
{
    optional<string> validateAmd(const MetadataPtr& metadata, const SyntaxTreeBasePtr& p)
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

    optional<string> validateDeprecated(const SyntaxTreeBasePtr& p)
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

    optional<string> validateFormat(const MetadataPtr& metadata, const SyntaxTreeBasePtr& p)
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

    optional<string> validateMarshaledResult(const MetadataPtr& metadata, const SyntaxTreeBasePtr& p)
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

    optional<string> validateProtected(const MetadataPtr& metadata, const SyntaxTreeBasePtr& p)
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

    optional<string> validateSuppressWarning(const MetadataPtr& metadata, const SyntaxTreeBasePtr& p)
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
    optional<string> validateUserException(const MetadataPtr& metadata, const SyntaxTreeBasePtr& p)
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

bool
Slice::MetadataValidator::visitUnitStart(const UnitPtr& p)
{
    DefinitionContextPtr dc = p->findDefinitionContext(p->topLevelFile());
    assert(dc);
    dc->setMetadata(validate(dc->getMetadata(), p));
    return true;
}

bool
Slice::MetadataValidator::visitModuleStart(const ModulePtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
    return true;
}

void
Slice::MetadataValidator::visitClassDecl(const ClassDeclPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
}

bool
Slice::MetadataValidator::visitClassDefStart(const ClassDefPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
    return true;
}

void
Slice::MetadataValidator::visitInterfaceDecl(const InterfaceDeclPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
}

bool
Slice::MetadataValidator::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
    return true;
}

bool
Slice::MetadataValidator::visitExceptionStart(const ExceptionPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
    return true;
}

bool
Slice::MetadataValidator::visitStructStart(const StructPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
    return true;
}

void
Slice::MetadataValidator::visitOperation(const OperationPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
}

void
Slice::MetadataValidator::visitParamDecl(const ParamDeclPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
}

void
Slice::MetadataValidator::visitDataMember(const DataMemberPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
}

void
Slice::MetadataValidator::visitSequence(const SequencePtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
    p->setTypeMetadata(validate(p->typeMetadata(), p->type()));
}

void
Slice::MetadataValidator::visitDictionary(const DictionaryPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
    p->setKeyMetadata(validate(p->keyMetadata(), p->keyType()));
    p->setValueMetadata(validate(p->valueMetadata(), p->valueType()));
}

void
Slice::MetadataValidator::visitEnum(const EnumPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
}

void
Slice::MetadataValidator::visitConst(const ConstPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
    p->setTypeMetadata(validate(p->typeMetadata(), p->type()));
}

MetadataList
Slice::MetadataValidator::validate(MetadataList metadata, const SyntaxTreeBasePtr& p)
{
    // Iterate through the provided metadata and check each one for validity.
    for (MetadataList::const_iterator i = metadata.begin(); i != metadata.end();)
    {
        const MetadataPtr& meta = *i++;
        if (auto message = validateMetadata(meta, p))
        {
            // If a warning message was returned from the validation function, it means the metadata was invalid.
            // We remove it from the list and emit a warning to the user about it.
            metadata.remove(meta);
            p->unit()->warning(meta->file(), meta->line(), InvalidMetadata, *message);
        }
    }

    return metadata;
}

optional<string>
Slice::MetadataValidator::validateMetadata(const MetadataPtr& metadata, const SyntaxTreeBasePtr& p)
{
    // We only want to check metadata that that doesn't have a language prefix (ie. parser metadata).
    const string& directive = metadata->directive();
    if (directive.find(':') != string::npos)
    {
        return nullopt;
    }

    // Check each of the language-agnostic metadata directives that the parser is aware of.
    if (directive == "amd")
    {
        return validateAmd(metadata, p);
    }
    else if (directive == "deprecate")
    {
        return validateDeprecated(p);
    }
    else if (directive == "deprecated")
    {
        return validateDeprecated(p);
    }
    else if (directive == "format")
    {
        return validateFormat(metadata, p);
    }
    else if (directive == "marshaled-result")
    {
        return validateMarshaledResult(metadata, p);
    }
    else if (directive == "protected")
    {
        return validateProtected(metadata, p);
    }
    else if (directive == "suppress-warning")
    {
        return validateSuppressWarning(metadata, p);
    }
    else if (directive == "UserException")
    {
        return validateUserException(metadata, p);
    }
    else
    {
        return "ignoring unknown metadata directive: '" + directive + "'";
    }
}
