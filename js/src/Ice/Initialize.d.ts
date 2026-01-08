// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Creates a communicator.
         *
         * This method is provided for backwards compatibility. New code should call the {@link Communicator}
         * constructor directly.
         *
         * @returns The initialized communicator.
         * @throws {@link InitializationException} If an error occurs during initialization.
         */
        function initialize(): Communicator;

        /**
         * Creates a communicator.
         *
         * This method is provided for backwards compatibility. New code should call the {@link Communicator}
         * constructor directly.
         *
         * @param initData Options for the new communicator.
         * @returns The initialized communicator.
         * @throws {@link InitializationException} If an error occurs during initialization.
         */
        function initialize(initData?: InitializationData): Communicator;

        /**
         * Creates a communicator.
         *
         * This method is provided for backwards compatibility. New code should call the {@link Communicator}
         * constructor directly.
         *
         * @param args A command-line argument vector. Any Ice-related options in this vector are used to initialize
         * the communicator. This method modifies the argument vector by removing any Ice-related options.
         * @returns The initialized communicator.
         * @throws {@link InitializationException} If an error occurs during initialization.
         */
        function initialize(args: string[]): Communicator;

        /**
         * Creates a property set
         *
         * This method is provided for backwards compatibility. New code should call the {@link Properties}
         * constructor directly.
         *
         * @param args An optional command-line argument vector, possibly containing options to set properties.
         * This method modifies the argument vector by removing any Ice-related options.
         * @param defaults Default values for the property set. Settings in args override these defaults.
         * @returns A property set.
         */
        function createProperties(args?: string[], defaults?: Properties): Properties;

        /**
         * Returns the Ice version as a string in the format "A.B.C", where:
         * - A: major version
         * - B: minor version
         * - C: patch version
         *
         * For example, "3.9.0".
         *
         * @returns The Ice version as a string.
         */
        function stringVersion(): string;

        /**
         * Returns the Ice version as an integer in the format AABBCC, where:
         * - AA: major version
         * - BB: minor version
         * - CC: patch version
         *
         * For example, for Ice 3.9.1, the returned value is 30901.
         *
         * @returns The Ice version as an integer.
         */
        function intVersion(): number;
    }
}
