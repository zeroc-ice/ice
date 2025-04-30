// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * A class that encapsulates data to initialize a communicator.
         */
        class InitializationData {
            /**
             * Construct a new InitializationData instance.
             */
            constructor();

            /**
             * Creates a deep copy of the object.
             *
             * @returns A deep copy of the object
             */
            clone(): InitializationData;

            /**
             * The properties for the communicator.
             */
            properties: Properties;

            /**
             * The logger for the communicator.
             */
            logger: Logger;

            /**
             * The Slice loader, used to unmarshal Slice classes and exceptions.
             */
            sliceLoader: SliceLoader;
        }

        /**
         * Creates a communicator.
         *
         * @param initData Additional initialization data.
         * @returns The initialized communicator.
         * @throws {@link InitializationException} If an error occurs during initialization.
         */
        function initialize(initData?: InitializationData): Communicator;

        /**
         * Creates a communicator.
         *
         * @param args A command-line argument vector. Any Ice-related options in this vector are used to initialize
         * the communicator. This method modifies the argument vector by removing any Ice-related options.
         * @param initData Additional initialization data. Property settings in args override property settings in
         * initData.
         * @returns The initialized communicator.
         * @throws {@link InitializationException} If an error occurs during initialization.
         */
        function initialize(args: string[], initData?: InitializationData): Communicator;

        /**
         * Creates a property set
         * @param args An optional command-line argument vector, possibly containing options to set properties.
         * This method modifies the argument vector by removing any Ice-related options.
         * @param defaults Default values for the property set. Settings in args override these defaults.
         * @returns A property set.
         */
        function createProperties(args?: string[], defaults?: Properties): Properties;

        /**
         * Gets the latest protocol encoding version, which is currently 1.0.
         *
         * @returns The latest protocol encoding version.
         */
        function currentProtocol(): ProtocolVersion;

        /**
         * Gets the latest encoding version, which is currently 1.1.
         *
         * @returns The latest encoding version.
         */
        function currentEncoding(): EncodingVersion;

        /**
         * Returns the Ice version as a string in the format "A.B.C", where:
         * - A: major version
         * - B: minor version
         * - C: patch version
         *
         * For example, "3.8.0".
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
         * For example, 030800.
         *
         * @returns The Ice version as an integer.
         */
        function intVersion(): number;
    }
}
