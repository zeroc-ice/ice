// Copyright (c) ZeroC, Inc.

#include "CsMetadataValidator.h"
#include "../Slice/MetadataValidation.h"

using namespace std;
using namespace Slice;

void
Slice::validateCsMetadata(const UnitPtr& unit)
{
    map<string, MetadataInfo> knownMetadata;

    // "cs:attribute"
    MetadataInfo attributeInfo = {
        .validOn = {typeid(Enum), typeid(Enumerator), typeid(Const), typeid(DataMember)},
        .acceptedArgumentKind = MetadataArgumentKind::RequiredTextArgument,
        .mustBeUnique = false,
    };
    knownMetadata.emplace("cs:attribute", attributeInfo);

    // "cs:class"
    MetadataInfo classInfo = {
        .validOn = {typeid(Struct)},
        .acceptedArgumentKind = MetadataArgumentKind::NoArguments,
    };
    knownMetadata.emplace("cs:class", std::move(classInfo));

    // "cs:generic"
    MetadataInfo genericInfo = {
        .validOn = {typeid(Sequence), typeid(Dictionary)},
        .acceptedArgumentKind = MetadataArgumentKind::RequiredTextArgument,
        .extraValidation = [](const MetadataPtr& meta, const SyntaxTreeBasePtr& p) -> optional<string>
        {
            const string& argument = meta->arguments();
            if (auto seq = dynamic_pointer_cast<Sequence>(p); seq && seq->type()->isClassType())
            {
                if (argument == "LinkedList" || argument == "Queue" || argument == "Stack")
                {
                    return "'cs:generic:" + argument +
                           "' is not supported on sequences of objects; only 'List' is supported for object sequences";
                }
            }
            else if (dynamic_pointer_cast<Dictionary>(p))
            {
                if (argument != "SortedDictionary" && argument != "SortedList")
                {
                    return "the 'cs:generic' metadata only supports 'SortedDictionary' and 'SortedList' as arguments "
                           "when applied to a dictionary";
                }
            }
            return nullopt;
        },
    };
    knownMetadata.emplace("cs:generic", genericInfo);

    // "cs:identifier"
    MetadataInfo identifierInfo = {
        .validOn =
            {typeid(Module),
             typeid(InterfaceDecl),
             typeid(Operation),
             typeid(ClassDecl),
             typeid(Slice::Exception),
             typeid(Struct),
             typeid(Sequence),
             typeid(Dictionary),
             typeid(Enum),
             typeid(Enumerator),
             typeid(Const),
             typeid(Parameter),
             typeid(DataMember)},
        .acceptedArgumentKind = MetadataArgumentKind::SingleArgument,
    };
    knownMetadata.emplace("cs:identifier", std::move(identifierInfo));

    // "cs:namespace"
    MetadataInfo namespaceInfo = {
        .validOn = {typeid(Module)},
        .acceptedArgumentKind = MetadataArgumentKind::SingleArgument,
        .extraValidation = [](const MetadataPtr& metadata, const SyntaxTreeBasePtr& p) -> optional<string>
        {
            const string msg = "'cs:namespace' is deprecated; use 'cs:identifier' to remap modules instead";
            p->unit()->warning(metadata->file(), metadata->line(), Deprecated, msg);

            if (auto cont = dynamic_pointer_cast<Contained>(p); cont && cont->hasMetadata("cs:identifier"))
            {
                return "A Slice element can only have one of 'cs:namespace' and 'cs:identifier' applied to it";
            }

            // 'cs:namespace' can only be applied to top-level modules
            // Top-level modules are contained by the 'Unit'. Non-top-level modules are contained in 'Module's.
            if (auto mod = dynamic_pointer_cast<Module>(p); mod && !mod->isTopLevel())
            {
                return "the 'cs:namespace' metadata can only be applied to top-level modules";
            }
            return nullopt;
        },
    };
    knownMetadata.emplace("cs:namespace", std::move(namespaceInfo));

    // "cs:property"
    MetadataInfo propertyInfo = {
        .validOn = {typeid(ClassDecl), typeid(Slice::Exception), typeid(Struct)},
        .acceptedArgumentKind = MetadataArgumentKind::NoArguments,
    };
    knownMetadata.emplace("cs:property", std::move(propertyInfo));

    // Pass this information off to the parser's metadata validation logic.
    Slice::validateMetadata(unit, "cs", std::move(knownMetadata));
}
