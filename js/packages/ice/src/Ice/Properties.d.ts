// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Represents a set of properties used to configure Ice and Ice-based applications. A property is a key/value
         * pair, where both the key and the value are strings. By convention, property keys should have the form
         * `application-name[.category[.sub-category]].name`.
         */
        class Properties {
            /**
             * Constructs a property set.
             *
             * @param args  The command-line arguments. This constructor parses arguments starting with `--` and one
             * of the reserved prefixes (Ice, IceSSL, etc.) as properties and removes these elements from the vector.
             * @param defaults Default values for the new Properties object. Settings in args override these defaults.
             * May be null.
             */
            constructor(args?: string[], defaults?: Properties);

            /**
             * Gets a property by key.
             *
             * @param key The property key.
             * @returns The property value, or the empty string if the property is not set.
             * @see {@link setProperty}
             */
            getProperty(key: string): string;

            /**
             * Gets an Ice property by key.
             *
             * @param key - The property key.
             * @returns The property value, or the default value for this property if the property is not set.
             * @throws {@link PropertyException} - Thrown when the property is not a known Ice property.
             * @see {@link setProperty}
             */
            getIceProperty(key: string): string;

            /**
             * Gets a property by key.
             *
             * @param key - The property key.
             * @param value The default value to return if the property is not set.
             * @returns The property value or the default value if the property is not set.
             * @see {@link setProperty}
             */
            getPropertyWithDefault(key: string, value: string): string;

            /**
             * Gets a property as an integer.
             *
             * @param key - The property key.
             * @returns The property value interpreted as an integer, or 0 if the property is not set.
             * @throws {@link PropertyException} - Thrown when the property value is not a valid integer.
             *
             * @see {@link setProperty}
             */
            getPropertyAsInt(key: string): number;

            /**
             * Gets an Ice property as an integer.
             *
             * @param key - The property key.
             * @returns The property value interpreted as an integer, or the default value if the property is not set.
             * @throws {@link PropertyException} - Thrown when the property is not a known Ice property or the value is
             * not a valid integer.
             *
             * @see {@link setProperty}
             */
            getIcePropertyAsInt(key: string): number;

            /**
             * Gets a property as an integer.
             *
             * @param key The property key.
             * @param value The default value to return if the property does not exist.
             * @returns The property value interpreted as an integer, or the default value if the property is not set.
             * @throws {@link PropertyException} - Thrown when the property value is not a valid integer.
             * @see {@link setProperty}
             */
            getPropertyAsIntWithDefault(key: string, value: number): number;

            /**
             * Gets a property as a list of strings. The strings must be separated by whitespace or comma. The strings
             * in the list can contain whitespace and commas if they are enclosed in single or double quotes. If quotes
             * are mismatched, an empty list is returned. Within single quotes or double quotes, you can escape the
             * quote in question with a backslash, e.g. O'Reilly can be written as O'Reilly, "O'Reilly" or 'O\'Reilly'.
             *
             * @param key - The property key.
             * @returns The property value interpreted as a list of strings, or an empty list if the property is not
             * set.
             * @see {@link setProperty}
             */
            getPropertyAsList(key: string): StringSeq;

            /**
             * Gets an Ice property as a list of strings. The strings must be separated by whitespace or comma. The
             * strings in the list can contain whitespace and commas if they are enclosed in single or double quotes.
             * If quotes are mismatched, the default list is returned.  Within single quotes or double quotes, you can
             * escape the quote in question with a backslash, e.g. O'Reilly can be written as O'Reilly, "O'Reilly" or
             * 'O\'Reilly'.
             *
             * @param key - The property key.
             * @returns The property value interpreted as a list of strings.
             * @throws {@link PropertyException} - Thrown if the property is not a known Ice property.
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
             * @see {@link setProperty}
             */
            getPropertyAsListWithDefault(key: string, value: StringSeq): StringSeq;

            /**
             * Gets all properties whose keys begin with `prefix`. If `prefix` is the empty string, then all properties
             * are returned.
             *
             * @param prefix - The prefix to search for.
             * @returns The matching property set.
             */
            getPropertiesForPrefix(prefix: string): PropertyDict;

            /**
             * Sets a property. To unset a property, set it to the empty string.
             *
             * @param key - The property key.
             * @param value - The property value.
             * @see {@link getProperty}
             */
            setProperty(key: string, value: string): void;

            /**
             * Gets a sequence of command-line options that is equivalent to this property set. Each element of the
             * returned sequence is a command-line option of the form `--key=value`.
             *
             * @returns A sequence of command-line options representing this property set.
             */
            getCommandLineOptions(): StringSeq;

            /**
             * Converts a sequence of command-line options into properties. All options that start with `--prefix.` are
             * converted into properties. If the prefix is empty, all options that begin with `--` are converted to
             * properties.
             *
             * @param prefix - The property prefix, or the empty string to convert all options starting with `--`.
             * @param options - The command-line options.
             * @returns The command-line options that do not start with the specified prefix, in their original order.
             */
            parseCommandLineOptions(prefix: string, options: StringSeq): StringSeq;

            /**
             * Converts a sequence of command-line options into properties. All options that start with one of the
             * reserved Ice prefixes (`--Ice`, `--IceSSL`, etc.) are converted into properties.
             *
             * @param options - The command-line options.
             * @returns The command-line options that do not start with one of the reserved prefixes, in their original
             * order.
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
