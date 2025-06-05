// Copyright (c) ZeroC, Inc.

#include "MetadataValidation.h"
#include "Ice/StringUtil.h"
#include "Util.h"

#include <algorithm>
#include <cassert>
#include <iterator>

using namespace std;
using namespace Slice;

// Where we define the internal `MetadataVisitor`, which performs the 'actual' metadata validation.
namespace
{
    class MetadataVisitor final : public ParserVisitor
    {
    public:
        MetadataVisitor(string_view language, map<string, MetadataInfo> knownMetadata);

        // We don't visit `ClassDef` and `InterfaceDef` because their metadata is always stored on their corresponding
        // `ClassDecl` and `InterfaceDecl` types. So just visiting the `Decl` types is sufficient to check everything.

        bool visitUnitStart(const UnitPtr&) final;
        bool visitModuleStart(const ModulePtr&) final;
        void visitClassDecl(const ClassDeclPtr&) final;
        void visitInterfaceDecl(const InterfaceDeclPtr&) final;
        bool visitExceptionStart(const ExceptionPtr&) final;
        bool visitStructStart(const StructPtr&) final;
        void visitOperation(const OperationPtr&) final;
        void visitParameter(const ParameterPtr&) final;
        void visitDataMember(const DataMemberPtr&) final;
        void visitSequence(const SequencePtr&) final;
        void visitDictionary(const DictionaryPtr&) final;
        void visitEnum(const EnumPtr&) final;
        void visitConst(const ConstPtr&) final;

        MetadataList validate(MetadataList metadata, const SyntaxTreeBasePtr& p, bool isTypeContext = false);
        bool isMetadataValid(const MetadataPtr& metadata, const SyntaxTreeBasePtr& p, bool isTypeContext);

        string_view _language;
        map<string, MetadataInfo> _knownMetadata;
        map<string, MetadataPtr> _seenDirectives;
    };
}

string
Slice::misappliedMetadataMessage(const MetadataPtr& metadata, const SyntaxTreeBasePtr& p)
{
    string message = '\'' + metadata->directive() + "' metadata cannot be ";
    if (dynamic_pointer_cast<Unit>(p))
    {
        message += "specified as file metadata";
    }
    else if (dynamic_pointer_cast<Builtin>(p))
    {
        message += "applied to builtin types";
    }
    else
    {
        const ContainedPtr contained = dynamic_pointer_cast<Contained>(p);
        assert(contained);
        message += "applied to " + pluralKindOf(contained);
    }
    return message;
}

void
Slice::validateMetadata(const UnitPtr& p, string_view prefix, map<string, MetadataInfo> knownMetadata)
{
    // We want to perform all the metadata validation in the same pass, to keep all the diagnostics in order.
    // So, we add all the language-agnostic metadata validation into the provided list.

    // "amd"
    MetadataInfo amdInfo = {
        .validOn = {typeid(InterfaceDecl), typeid(Operation)},
        .acceptedArgumentKind = MetadataArgumentKind::NoArguments,
    };
    knownMetadata.emplace("amd", std::move(amdInfo));

    // "deprecated"
    MetadataInfo deprecatedInfo = {
        .validOn =
            {typeid(InterfaceDecl),
             typeid(Operation),
             typeid(ClassDecl),
             typeid(Slice::Exception),
             typeid(Struct),
             typeid(Sequence),
             typeid(Dictionary),
             typeid(Enum),
             typeid(Enumerator),
             typeid(Const),
             typeid(DataMember)},
        .acceptedArgumentKind = MetadataArgumentKind::OptionalTextArgument,
    };
    knownMetadata.emplace("deprecate", deprecatedInfo); // Kept as an alias for 'deprecated'.
    knownMetadata.emplace("deprecated", std::move(deprecatedInfo));

    // "format"
    MetadataInfo formatInfo = {
        .validOn = {typeid(InterfaceDecl), typeid(Operation)},
        .acceptedArgumentKind = MetadataArgumentKind::SingleArgument,
        .validArgumentValues = {{"compact", "sliced", "default"}},
    };
    knownMetadata.emplace("format", std::move(formatInfo));

    // "marshaled-result"
    MetadataInfo marshaledResultInfo = {
        .validOn = {typeid(InterfaceDecl), typeid(Operation)},
        .acceptedArgumentKind = MetadataArgumentKind::NoArguments,
    };
    knownMetadata.emplace("marshaled-result", std::move(marshaledResultInfo));

    // "suppress-warning"
    MetadataInfo suppressWarningInfo = {
        .validOn = {typeid(Unit)},
        .acceptedArgumentKind = MetadataArgumentKind::AnyNumberOfArguments,
        .validArgumentValues = {{"all", "deprecated", "invalid-comment"}},
        .mustBeUnique = false,
    };
    knownMetadata.emplace("suppress-warning", std::move(suppressWarningInfo));

    // Then we pass this list off the internal visitor, which performs the heavy lifting.
    auto visitor = MetadataVisitor(prefix, std::move(knownMetadata));
    p->visit(&visitor);
}

MetadataVisitor::MetadataVisitor(string_view language, map<string, MetadataInfo> knownMetadata)
    : _language(language),
      _knownMetadata(std::move(knownMetadata)),
      _seenDirectives()
{
}

bool
MetadataVisitor::visitUnitStart(const UnitPtr& p)
{
    DefinitionContextPtr dc = p->findDefinitionContext(p->topLevelFile());
    assert(dc);
    dc->setMetadata(validate(dc->getMetadata(), p));
    return true;
}

bool
MetadataVisitor::visitModuleStart(const ModulePtr& p)
{
    const MetadataList& moduleMetadata = p->getMetadata();
    if (p->usesNestedSyntax && !moduleMetadata.empty())
    {
        // Metadata cannot be applied to modules that used nested-module-syntax, since it's ambiguous in meaning.
        // We issue an error, and clear the metadata, so that we're in a valid state for further validation.
        p->unit()->error(
            p->file(),
            p->line(),
            "metadata cannot be applied to modules defined using nested module syntax");
        p->setMetadata({});
    }
    else
    {
        p->setMetadata(validate(moduleMetadata, p));
    }
    return true;
}

void
MetadataVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
}

void
MetadataVisitor::visitInterfaceDecl(const InterfaceDeclPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
}

bool
MetadataVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
    return true;
}

bool
MetadataVisitor::visitStructStart(const StructPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
    return true;
}

void
MetadataVisitor::visitOperation(const OperationPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
    for (const auto& param : p->parameters())
    {
        param->visit(this);
    }
}

void
MetadataVisitor::visitParameter(const ParameterPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
}

void
MetadataVisitor::visitDataMember(const DataMemberPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
}

void
MetadataVisitor::visitSequence(const SequencePtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
    p->setTypeMetadata(validate(p->typeMetadata(), p->type(), true));
}

void
MetadataVisitor::visitDictionary(const DictionaryPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
    p->setKeyMetadata(validate(p->keyMetadata(), p->keyType(), true));
    p->setValueMetadata(validate(p->valueMetadata(), p->valueType(), true));
}

void
MetadataVisitor::visitEnum(const EnumPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
}

void
MetadataVisitor::visitConst(const ConstPtr& p)
{
    p->setMetadata(validate(p->getMetadata(), p));
    p->setTypeMetadata(validate(p->typeMetadata(), p->type(), true));
}

MetadataList
MetadataVisitor::validate(MetadataList metadata, const SyntaxTreeBasePtr& p, bool isTypeContext)
{
    // Reset the set of 'seenDirectives' now that we're visiting a new Slice element.
    _seenDirectives.clear();

    // Iterate through the provided metadata and check each one for validity.
    // If we come across any invalid metadata, we remove it from the list (i.e. we filter out invalid metadata).
    for (auto i = metadata.begin(); i != metadata.end();)
    {
        const string& directive = (*i)->directive();

        // If the directive contains a ':' character, but is for a different language than what we're checking,
        // we mark it for removal, but perform no additional validation of it.
        if (directive.find(':') != string::npos && directive.find(_language) != 0)
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

    return metadata;
}

bool
MetadataVisitor::isMetadataValid(const MetadataPtr& metadata, const SyntaxTreeBasePtr& p, bool isTypeContext)
{
    // First, we check if the metadata is one we know of. If it isn't, we issue a warning and immediately return.
    const string& directive = metadata->directive();
    auto lookupResult = _knownMetadata.find(directive);
    if (lookupResult == _knownMetadata.end())
    {
        ostringstream msg;
        msg << "ignoring unknown metadata: '" << *metadata << '\'';
        p->unit()->warning(metadata->file(), metadata->line(), InvalidMetadata, msg.str());
        return false;
    }
    const MetadataInfo& info = lookupResult->second;

    // If we get here, then it's metadata that we know about!
    bool isValid = true;

    // Second, we check to make sure that the correct number of arguments were provided.
    const string& arguments = metadata->arguments();
    switch (info.acceptedArgumentKind)
    {
        case MetadataArgumentKind::NoArguments:
            if (!arguments.empty())
            {
                string msg = "the '" + directive + "' metadata does not take any arguments";
                p->unit()->warning(metadata->file(), metadata->line(), InvalidMetadata, msg);
                isValid = false;
            }
            break;

        case MetadataArgumentKind::SingleArgument:
            // Make sure there's no commas in the arguments (i.e. make sure it's not a list).
            if (arguments.find(',') != string::npos)
            {
                string msg = "the '" + directive + "' metadata only accepts one argument but a list was provided";
                p->unit()->warning(metadata->file(), metadata->line(), InvalidMetadata, msg);
                isValid = false;
            }
            // Then intentionally fall through to the non-empty check below, since we require an argument.
            [[fallthrough]];

        case MetadataArgumentKind::RequiredTextArgument:
            if (arguments.empty())
            {
                string msg = "missing required argument for '" + directive + "' metadata";
                p->unit()->warning(metadata->file(), metadata->line(), InvalidMetadata, msg);
                isValid = false;
            }
            break;

        // We don't need to validate the number of arguments if it allows "any number of arguments".
        case MetadataArgumentKind::AnyNumberOfArguments:
        // Or if it's `OptionalTextArgument` since it can be anything, including the empty string.
        case MetadataArgumentKind::OptionalTextArgument:
            break;
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
                string msg = "invalid argument '" + trimmedArg + "' supplied to '" + directive + "' metadata";
                p->unit()->warning(metadata->file(), metadata->line(), InvalidMetadata, msg);
                isValid = false;
            }
        }
    }

    // Fourth, we check what the metadata was applied to - does that Slice definition support this metadata?
    SyntaxTreeBasePtr appliedTo;
    if (info.acceptedContext == MetadataApplicationContext::Definitions)
    {
        if (isTypeContext)
        {
            string msg = '\'' + directive + "' metadata can only be applied to definitions and declarations" +
                         ", it cannot be applied to type references";
            p->unit()->warning(metadata->file(), metadata->line(), InvalidMetadata, msg);
            isValid = false;
        }
        else
        {
            appliedTo = p;
        }
    }
    else
    {
        // Some metadata can only be applied to parameters (and return types). This bool stores that information.
        bool isAppliedToParameter = false;

        // If the metadata we're validating can be applied to type references, but it was applied to an operation,
        // we treat the metadata as if it was applied to that operation's return type.
        // Same thing if this metadata has been applied to a parameter or data member as well.
        if (auto op = dynamic_pointer_cast<Operation>(p))
        {
            isAppliedToParameter = true;
            if (const auto returnType = op->returnType())
            {
                appliedTo = returnType;
            }
            else
            {
                string msg = '\'' + directive + "' metadata cannot be applied to operations with void return type";
                p->unit()->warning(metadata->file(), metadata->line(), InvalidMetadata, msg);
                isValid = false;
            }
        }
        else if (auto param = dynamic_pointer_cast<Parameter>(p))
        {
            isAppliedToParameter = true;
            appliedTo = param->type();
        }
        else if (auto dm = dynamic_pointer_cast<DataMember>(p))
        {
            appliedTo = dm->type();
        }
        else
        {
            // Otherwise there's nothing special going on and we know that the metadata was applied directly to `p`.
            appliedTo = p;
        }

        // If this metadata is only valid in the context of parameters issue a warning if that condition wasn't met.
        if (info.acceptedContext == MetadataApplicationContext::ParameterTypeReferences && !isAppliedToParameter)
        {
            auto msg = '\'' + directive + "' metadata can only be applied to operation parameters and return types";
            p->unit()->warning(metadata->file(), metadata->line(), InvalidMetadata, msg);
            isValid = false;
        }
    }

    // After we've deduced exactly what Slice element the metadata should be applied to, check that it's supported.
    const list<reference_wrapper<const type_info>>& validOn = info.validOn;
    if (!validOn.empty() && appliedTo) // 'appliedTo' will be null if we already found a problem and should stop.
    {
        auto appliedToPtr = appliedTo.get();
        auto comparator = [&](reference_wrapper<const type_info> t) { return t.get() == typeid(*appliedToPtr); };
        if (std::none_of(validOn.begin(), validOn.end(), comparator))
        {
            string message = misappliedMetadataMessage(metadata, appliedTo);
            p->unit()->warning(metadata->file(), metadata->line(), InvalidMetadata, message);
            isValid = false;
        }
    }

    // Fifth we check if this metadata is a duplicate, i.e. have we already seen this metadata in this context?
    if (info.mustBeUnique)
    {
        // 'emplace' only returns `true` if the value wasn't already present in the set.
        bool wasInserted = _seenDirectives.emplace(directive, metadata).second;
        if (!wasInserted)
        {
            // We make a special exception to uniqueness for metadata on forward declarations, since there can be
            // multiple forward declarations for the same entity, but they all share a single backing `MetadataList`.
            // So for these types, even 'unique' metadata to appear multiple times, but we require them to be identical.
            if (dynamic_pointer_cast<ClassDecl>(p) || dynamic_pointer_cast<InterfaceDecl>(p))
            {
                const MetadataPtr& previousMetadata = _seenDirectives[directive];
                if (arguments != previousMetadata->arguments())
                {
                    ostringstream msg;
                    msg << "ignoring duplicate metadata: '" << *metadata
                        << "' does not match previously applied metadata of '" << *previousMetadata << "'";
                    p->unit()->warning(metadata->file(), metadata->line(), InvalidMetadata, msg.str());
                }

                // Even in this special case, we want to remove the metadata. At worst it was invalid, and at best,
                // it's an exact duplicate of some other metadata. Either way, there's no reason to keep it around.
                isValid = false;
            }
            else
            {
                auto msg = "ignoring duplicate metadata: '" + directive + "' has already been applied in this context";
                p->unit()->warning(metadata->file(), metadata->line(), InvalidMetadata, msg);
                isValid = false;
            }
        }
    }

    // Finally, if a custom validation function is specified for this metadata, we run it.
    if (info.extraValidation && appliedTo) // 'appliedTo' is null if we already found a problem and should stop.
    {
        // This function will return `nullopt` to signal everything is okay.
        // So if we get a string back, we know that the custom validation failed.
        if (auto result = info.extraValidation(metadata, appliedTo))
        {
            p->unit()->warning(metadata->file(), metadata->line(), InvalidMetadata, *result);
            isValid = false;
        }
    }

    return isValid;
}
