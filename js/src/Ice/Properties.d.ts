// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * A property set used to configure Ice applications. Properties are key/value pairs, with both keys and
         * values being strings.
         */
        class Properties {
            /**
             * Initializes a new instance of the Properties class. The property set is initialized from the provided
             * argument vector.
             *
             * @param args A command-line argument vector, possibly containing options to set properties. This method
             * modifies the argument vector by removing any Ice-related options.
             * @param defaults Default values for the property set. Settings in args override these defaults. May be null.
             */
            constructor(args?: string[], defaults?: Properties);

            /**
             * Get a property by key. If the property is not set, an empty string is returned.
             *
             * @param key The property key.
             * @returns The property value.
             *
             * @see {@link setProperty}
             */
            getProperty(key: string): string;

            /**
             * Get an Ice property by key. If the property is not set, its default value is returned.
             *
             * @param key - The property key.
             * @returns The property value.
             *
             * @see {@link setProperty}
             */
            getIceProperty(key: string): string;

            /**
             * Get a property by key. If the property is not set, the given default value is returned.
             *
             * @param key - The property key.
             * @param value The default value to use if the property is not set.
             * @returns The property value or the default value.
             *
             * @see {@link setProperty}
             */
            getPropertyWithDefault(key: string, value: string): string;

            /**
             * Get a property as an integer. If the property is not set, 0 is returned.
             *
             * @param key - The property key.
             * @returns The property value interpreted as an integer.
             * @throws {@link PropertyException} - Thrown if the property value is not a valid integer.
             *
             * @see {@link setProperty}
             */
            getPropertyAsInt(key: string): number;

            /**
             * Get an Ice property as an integer. If the property is not set, its default value is returned.
             *
             * @param key - The property key.
             * @returns The property value interpreted as an integer.
             * @throws {@link PropertyException} - Thrown if the property is not a known Ice property or the value is
             * not a valid integer.
             *
             * @see {@link setProperty}
             */
            getIcePropertyAsInt(key: string): number;

            /**
             * Get a property as an integer. If the property is not set, the given default value is returned.
             *
             * @param key The property key.
             * @param defaultValue The default value to use if the property does not exist.
             * @returns The property value interpreted as an integer, or the default value.
             * @throws {@link PropertyException} - Thrown if the property value is not a valid integer.
             *
             * @see {@link setProperty}
             */
            getPropertyAsIntWithDefault(key: string, defaultValue: number): number;

            /**
             * Retrieves a property value as a list of strings. The strings must be separated by whitespace or commas.
             * If the property is not set, an empty list is returned. The strings in the list can contain whitespace
             * and commas if they are enclosed in single or double quotes. If quotes are mismatched, an empty list is
             * returned. Within single or double quotes, the respective quote character can be escaped with a backslash.
             * For example, O'Reilly can be written as O'Reilly, "O'Reilly", or 'O\'Reilly'.
             *
             * @param key - The property key.
             * @returns The property value interpreted as a list of strings.
             *
             * @see {@link setProperty}
             */
            getPropertyAsList(key: string): StringSeq;

            /**
             * Retrieves an Ice property value as a list of strings. The strings must be separated by whitespace or
             * commas. If the property is not set, an empty list is returned. The strings in the list can contain
             * whitespace and commas if they are enclosed in single or double quotes. If quotes are mismatched, an
             * empty list is returned. Within single or double quotes, the respective quote character can be escaped
             * with a backslash. For example, O'Reilly can be written as O'Reilly, "O'Reilly", or 'O\'Reilly'.
             *
             * @param key - The property key.
             * @returns The property value interpreted as a list of strings.
             *
             * @see {@link setProperty}
             */
            getIcePropertyAsList(key: string): StringSeq;

            /**
             * Retrieves a property value as a list of strings. The strings must be separated by whitespace or commas.
             * If the property is not set, an empty list is returned. The strings in the list can contain whitespace
             * and commas if they are enclosed in single or double quotes. If quotes are mismatched, an empty list is
             * returned. Within single or double quotes, the respective quote character can be escaped with a backslash.
             * For example, O'Reilly can be written as O'Reilly, "O'Reilly", or 'O\'Reilly'.
             *
             * @param key - The property key.
             * @param value - The default value to use if the property is not set.
             * @returns The property value interpreted as a list of strings.
             *
             * @see {@link setProperty}
             */
            getPropertyAsListWithDefault(key: string, value: StringSeq): StringSeq;

            /**
             * Retrieves all properties whose keys begin with the specified `prefix`. If `prefix` is an empty string,
             * all properties are returned.
             *
             * @param prefix - The prefix to search for, or an empty string to retrieve all properties.
             * @returns A dictionary containing the matching properties.
             */
            getPropertiesForPrefix(prefix: string): PropertyDict;

            /**
             * Sets a property with the specified key and value. To unset a property, set its value to an empty string.
             *
             * @param key - The property key.
             * @param value - The property value.
             *
             * @see {@link getProperty}
             */
            setProperty(key: string, value: string): void;

            /**
             * Retrieves a sequence of command-line options that is equivalent to this property set.
             * Each element of the returned sequence is a command-line option in the form `--<key>=<value>`.
             *
             * @returns A sequence of command-line options representing this property set.
             */
            getCommandLineOptions(): StringSeq;

            /**
             * Converts a sequence of command-line options into properties. All options that begin with `--<prefix>.`
             * are converted into properties. If the prefix is empty, all options that begin with `--` are converted
             * to properties.
             *
             * @param prefix - The property prefix, or an empty string to convert all options starting with `--`.
             * @param options - The command-line options to be converted.
             * @returns The command-line options that do not start with the specified prefix, in their original order.
             */
            parseCommandLineOptions(prefix: string, options: StringSeq): StringSeq;

            /**
             * Converts a sequence of command-line options into properties. All options that begin with one of the
             * following prefixes are converted into properties: `--Ice`, `--IceBox`, `--IceGrid`, `--IceSSL`,
             * `--IceStorm`, and `--Glacier2`.
             *
             * @param options - The command-line options to be converted.
             * @returns The command-line options that do not start with one of the listed prefixes, in their original
             *          order.
             */
            parseIceCommandLineOptions(options: StringSeq): StringSeq;

            /**
             * Loads properties from the contents of an Ice configuration file.
             *
             * @param data - The contents of the configuration file as a string.
             */
            parse(data: string): void;

            /**
             * Creates a deep copy of this property set.
             *
             * @returns A deep copy of this property set.
             */
            clone(): Ice.Properties;
        }
    }
}
