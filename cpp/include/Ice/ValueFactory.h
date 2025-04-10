// Copyright (c) ZeroC, Inc.

#ifndef ICE_VALUE_FACTORY_H
#define ICE_VALUE_FACTORY_H

#include "Config.h"
#include "ValueF.h"

#include <functional>
#include <memory>
#include <string_view>

namespace Ice
{
    /// Creates a new instance of a class with the given Slice type ID.
    /// @param typeId The Slice type ID of the class. For example, the Slice type ID for Slice class `Bar` in the
    /// module `Foo` is "::Foo::Bar". Note that the leading `::` is required.
    /// @return A new instance of the class identified by @p typeId, or nullptr if the factory is unable to create this
    /// instance.
    /// @headerfile Ice/Ice.h
    using ValueFactory = std::function<ValuePtr(std::string_view typeId)>;

    /// Maintains a collection of value factories. An application can supply a custom implementation of this class
    /// during communicator initialization; otherwise Ice provides a default implementation.
    /// @see ValueFactory
    /// @headerfile Ice/Ice.h
    class ICE_API ValueFactoryManager
    {
    public:
        virtual ~ValueFactoryManager();

        /// Adds a value factory.
        /// @param factory The factory to add.
        /// @param id The type ID for which the factory can create instances, or an empty string for the default
        /// factory.
        /// @throws AlreadyRegisteredException Thrown when a factory with the same type ID is already registered.
        virtual void add(ValueFactory factory, std::string_view id) = 0;

        /// Finds a value factory.
        /// @param id The type ID for which the factory can create instances, or an empty string for the default
        /// factory.
        /// @return The value factory, or nullptr if no value factory was found for @p id.
        /// @remark When unmarshaling an Ice value, the Ice runtime reads the most-derived type ID off the wire and
        /// attempts to create an instance of the type using a factory. If no instance is created, either because no
        /// factory was found, or because all factories returned nullptr, the behavior of the Ice runtime depends on the
        /// format with which the value was marshaled: If the value uses the "sliced" format, Ice ascends the class
        /// hierarchy until it finds a type that is recognized by a factory, or it reaches the least-derived type. If no
        /// factory is found that can create an instance, the runtime throws MarshalException. If the value uses
        /// the "compact" format, Ice immediately throws MarshalException. The following order is used to locate a
        /// factory for a type:
        /// - The Ice runtime looks for a factory registered specifically for the type ID.
        /// - If no instance has been created, the Ice runtime looks for the default factory, which is registered
        /// with an empty type ID.
        /// - If no instance has been created by any of the preceding steps, the Ice runtime looks for a factory
        /// generated by the Slice compiler.
        [[nodiscard]] virtual ValueFactory find(std::string_view id) const noexcept = 0;
    };

    /// A shared pointer to a ValueFactoryManager.
    using ValueFactoryManagerPtr = std::shared_ptr<ValueFactoryManager>;
}

#endif
