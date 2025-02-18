// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Creates a new value for a given value type. The type should be the absolute Slice type ID. For example,
         * the absolute Slice type ID for an interface `Bar` in the module `Foo` is `"::Foo::Bar"`. The leading `"::""`
         * is required.
         *
         * @param type - The value type as a string.
         * @returns The value created for the given type, or `null` if the factory is unable to create the value.
         */
        type ValueFactory = (type: string) => Ice.Value | null;

        /**
         * A value factory manager maintains a collection of value factories. An application can supply a custom
         * implementation during communicator initialization, otherwise Ice provides a default implementation.
         *
         * @see {@link ValueFactory}
         */
        interface ValueFactoryManager {
            /**
             * Adds a value factory. Attempting to add a factory with an ID for which a factory is already registered throws
             * an `AlreadyRegisteredException`.
             *
             * When unmarshaling an Ice value, the Ice runtime reads the most-derived type ID from the wire and attempts to
             * create an instance of the type using a factory. If no instance is created, either because no factory was found
             * or because all factories returned `null`, the behavior of the Ice runtime depends on the format with which the
             * value was marshaled:
             *
             * - If the value uses the "sliced" format, Ice ascends the class hierarchy until it finds a type recognized
             *   by a factory, or until it reaches the least-derived type. If no factory is found that can create an instance,
             *   the runtime throws a `MarshalException`.
             * - If the value uses the "compact" format, Ice immediately raises a `MarshalException`.
             *
             * The following order is used to locate a factory for a type:
             * 1. The Ice runtime looks for a factory registered specifically for the type.
             * 2. If no instance has been created, the Ice runtime looks for the default factory, which is registered with
             *    an empty type ID.
             * 3. If no instance has been created by any of the preceding steps, the Ice runtime looks for a generated class
             *    with the given type ID.
             *
             * @param factory - The factory to add.
             * @param id - The type ID for which the factory can create instances, or an empty string for the default factory.
             * @throws AlreadyRegisteredException - Thrown if a factory with the specified ID is already registered.
             */
            add(factory: Ice.ValueFactory, id: string): void;

            /**
             * Finds a value factory registered with this value factory manager.
             *
             * @param id - The type ID for which the factory can create instances, or an empty string for the default factory.
             * @returns The value factory, or `null` if no value factory was found for the given ID.
             */
            find(id: string): Ice.ValueFactory;
        }
    }
}
