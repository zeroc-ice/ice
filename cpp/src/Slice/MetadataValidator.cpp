// Copyright (c) ZeroC, Inc.

#include "MetadataValidator.h"
#include "Ice/StringUtil.h"
#include "Util.h"

#include <algorithm>
#include <cassert>
#include <iterator>

using namespace std;
using namespace Slice;

Slice::MetadataValidator::MetadataValidator(string language, map<string, MetadataInfo> metadataInfo)
    : _metadataInfo(std::move(metadataInfo)),
      _language(language + ":")
{
    // We want to perform all the metadata validation in the same pass, to keep all the diagnostics in order.
    // So, we add all the language-agnostic metadata validation into the provided list:

    MetadataInfo amdInfo = {
        {&typeid(InterfaceDef), &typeid(Operation)},
        MetadataArgumentKind::NoArguments,
    };
    _metadataInfo.emplace("amd", std::move(amdInfo));

    MetadataInfo deprecatedInfo = {
        {&typeid(InterfaceDecl), &typeid(InterfaceDef), &typeid(ClassDecl), &typeid(ClassDef), &typeid(Operation), &typeid(Exception), &typeid(Struct), &typeid(Sequence), &typeid(Dictionary), &typeid(Enum), &typeid(Enumerator), &typeid(Const), &typeid(DataMember)},
        MetadataArgumentKind::OptionalTextArgument,
    };
    _metadataInfo.emplace("deprecated", std::move(deprecatedInfo));

    MetadataInfo formatInfo = {
        {&typeid(Operation)},
        MetadataArgumentKind::SingleArgument,
        {{"compact", "sliced", "default"}},
    };
    _metadataInfo.emplace("format", std::move(formatInfo));

    MetadataInfo marshaledResultInfo = {
        {&typeid(InterfaceDef), &typeid(Operation)},
        MetadataArgumentKind::NoArguments,
    };
    _metadataInfo.emplace("marshaled-result", std::move(marshaledResultInfo));

    MetadataInfo protectedInfo = {
        {&typeid(ClassDef), &typeid(Slice::Exception), &typeid(Struct), &typeid(DataMember)},
        MetadataArgumentKind::NoArguments,
    };
    _metadataInfo.emplace("protected", std::move(protectedInfo));

    MetadataInfo suppressWarningInfo = {
        {&typeid(ClassDef), &typeid(Slice::Exception), &typeid(Struct), &typeid(DataMember)},
        MetadataArgumentKind::AnyNumberOfArguments,
        {{"all", "deprecated", "invalid-metadata", "invalid-comment"}},
    };
    _metadataInfo.emplace("protected", std::move(suppressWarningInfo));

    // TODO: we should probably just remove this metadata. It's only checked by slice2java,
    // and there's already a 'java:UserException' metadata that we also check... better to only keep that one.
    MetadataInfo userExceptionInfo = {
        {&typeid(Operation)},
        MetadataArgumentKind::NoArguments,
    };
    _metadataInfo.emplace("userException", std::move(userExceptionInfo));
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
    p->setTypeMetadata(validate(p->typeMetadata(), p->type(), true));
}

void
Slice::MetadataValidator::visitDictionary(const DictionaryPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
    p->setKeyMetadata(validate(p->keyMetadata(), p->keyType(), true));
    p->setValueMetadata(validate(p->valueMetadata(), p->valueType(), true));
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
    p->setTypeMetadata(validate(p->typeMetadata(), p->type(), true));
}

MetadataList
Slice::MetadataValidator::validate(MetadataList metadata, const SyntaxTreeBasePtr& p, bool isTypeContext)
{
    // Iterate through the provided metadata and check each one for validity.
    // If we come across any invalid metadata, we remove it from the list (ie. we filter out invalid metadata).
    for (MetadataList::const_iterator i = metadata.begin(); i != metadata.end();)
    {
        const string& directive = (*i)->directive();

        // If the directive contains a ':' character, but is for a different language than what we're checking,
        // we mark it for removal, but perform no additional validation of it.
        if (directive.find(':') != string::npos && !directive.starts_with(_language))
        {
            i = metadata.erase(i);
        }
        else
        {
            // If the metadata is invalid, remove it. Otherwise we advance to the next metadata like normal.
            bool isValid = isMetadataValid(*i, p, isTypeContext);
            i = isValid ? std::next(i, 1) : metadata.erase(i);
        }
    }
}

bool
Slice::MetadataValidator::isMetadataValid(const MetadataPtr& metadata, const SyntaxTreeBasePtr& p, bool isTypeContext)
{
    bool isValid = true;

    // First, we check if the metadata is one we know of. If it isn't, we issue a warning and immediately return.
    const string& directive = metadata->directive();
    auto infoResult = _metadataInfo.find(directive);
    if (infoResult == _metadataInfo.end())
    {
        string message = "Ignoring unknown metadata: '" + directive + '\'';
        p->unit()->warning(metadata->file(), metadata->line(), InvalidMetadata, message);
    }
    const MetadataInfo& info = infoResult->second;

    // Second, we check to make sure that the correct number of arguments were provided.
    const string& arguments = metadata->arguments();
    switch (info.acceptedArguments)
    {
        case MetadataArgumentKind::NoArguments:
            if (!arguments.empty())
            {
                string message = "the '" + directive + "' metadata does not take any arguments";
                p->unit()->warning(metadata->file(), metadata->line(), InvalidMetadata, message);
                isValid = false;
            }
            break;

        case MetadataArgumentKind::SingleArgument:
            // Make sure there's no commas in the arguments (ie. make sure it's not a list).
            if (arguments.find(',') != string::npos)
            {
                string message = "the '" + directive + "' metadata only accepts one argument, but a list was provided";
                p->unit()->warning(metadata->file(), metadata->line(), InvalidMetadata, message);
                isValid = false;
            }
            // Then intentionally fall through to the non-empty check below, since we require an argument.

        case MetadataArgumentKind::RequiredTextArgument:
            if (arguments.empty())
            {
                string message = "missing required argument for '" + directive + "' metadata";
                p->unit()->warning(metadata->file(), metadata->line(), InvalidMetadata, message);
                isValid = false;
            }
            break;

        // We don't need to validate the number of arguments if it allows "any number of arguments".
        case MetadataArgumentKind::AnyNumberOfArguments: break;
        // Or if it's `OptionalTextArgument` since it can be anything, including the empty string.
        case MetadataArgumentKind::OptionalTextArgument: break;
    }

    // Third, we check if the argument values are valid (if the metadata restricts what values can be supplied).
    if (info.validArgumentValues.has_value())
    {
        // Split the arguments into a string delimited list and check each one.
        vector<string> argList;
        IceInternal::splitString(arguments, ",", argList);

        const StringList& validValues = *info.validArgumentValues;
        for (const auto& arg : argList)
        {
            string trimmedArg = IceInternal::trim(arg);
            if (std::find(validValues.begin(), validValues.end(), trimmedArg) == validValues.end())
            {
                string message = "invalid argument '" + trimmedArg + "'supplied to '" + directive + "' metadata.";
                p->unit()->warning(metadata->file(), metadata->line(), InvalidMetadata, message);
                isValid = false;
            }
        }
    }

    if (isTypeContext && !info.isTypeMetadata) // This metadata cannot be applied to types, but it was.
    {
        string message = '\'' + directive + "' metadata can only be applied to definitions and declarations, it cannot be applied directly to types";
        p->unit()->warning(metadata->file(), metadata->line(), InvalidMetadata, message);
        isValid = false;
    }
    else if (!isTypeContext && info.isTypeMetadata) // This metadata can only be applied to types, but it wasn't.
    {
        string message = '\'' + directive + "' metadata cannot be applied to definitions and declarations, it can only be applied directly to types";
        p->unit()->warning(metadata->file(), metadata->line(), InvalidMetadata, message);
        isValid = false;
    }
    else
    {
        const list<const type_info*>& validOn = info.validOn;
        auto result = std::find_if(validOn.begin(), validOn.end(), [&](const type_info* t) { return *t == typeid(*p); });
        if (result == validOn.end())
        {
            string message = '\'' + directive + "' metadata cannot be ";
            if (typeid(*p) == typeid(Unit))
            {
                message += "specified as file metadata";
            }
            else if (typeid(*p) == typeid(Builtin))
            {
                message += "applied to primitive types";
            }
            else
            {
                const ContainedPtr& contained = dynamic_pointer_cast<Contained>(p);
                assert(contained);
                message += "applied to " + pluralKindOf(contained);
            }
            p->unit()->warning(metadata->file(), metadata->line(), InvalidMetadata, message);
            isValid = false;
        }
    }

    // Finally, if a custom validation function is specified for this metadata, we run it.
    if (info.extraValidation.has_value())
    {
        // This function will return `nullopt` to signal everything is okay.
        // So if we get a string back, we know that the custom validation failed.
        if (auto result = (*info.extraValidation)(metadata, p))
        {
            p->unit()->warning(metadata->file(), metadata->line(), InvalidMetadata, *result);
            isValid = false;
        }
    }

    return isValid;
}
