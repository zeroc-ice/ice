// Copyright (c) ZeroC, Inc.

import Foundation

/// Represents a set of properties used to configure Ice and Ice-based applications. A property is a key/value pair,
/// where both the key and the value are strings. By convention, property keys should have the form
/// `application-name[.category[.sub-category]].name`.
public protocol Properties: AnyObject {
    /// Gets a property by key.
    ///
    /// - Parameter key: The property key.
    /// - Returns: The property value, or the empty string if the property is not set.
    func getProperty(_ key: String) -> String

    /// Gets an Ice property by key.
    ///
    /// - Parameter key: The property key.
    /// - Returns: The property value, or the default value for this property if the property is not set.
    /// - Throws: ``PropertyException`` when the property is not a known Ice property.
    func getIceProperty(_ key: String) -> String

    /// Gets a property by key.
    ///
    /// - Parameters:
    ///   - key: The property key.
    ///   - value: The default value to return if the property is not set.
    /// - Returns: The property value or the default value if the property is not set.
    func getPropertyWithDefault(key: String, value: String) -> String

    /// Gets a property as an integer.
    ///
    /// - Parameter key: The property key.
    /// - Returns: The property value interpreted as an integer, or 0 if the property is not set.
    /// - Throws: ``PropertyException`` when the property value is not a valid integer.
    func getPropertyAsInt(_ key: String) throws -> Int32

    /// Gets an Ice property as an integer.
    ///
    /// - Parameter key: The property key.
    /// - Returns: The property value interpreted as an integer, or the default value if the property is not set.
    /// - Throws: ``PropertyException`` when the property is not a known Ice property or the value is not a valid
    ///   integer.
    func getIcePropertyAsInt(_ key: String) throws -> Int32

    /// Gets a property as an integer.
    ///
    /// - Parameters:
    ///   - key: The property key.
    ///   - value: The default value to return if the property does not exist.
    /// - Returns: The property value interpreted as an integer, or the default value if the property is not set.
    /// - Throws: ``PropertyException`` when the property value is not a valid integer.
    func getPropertyAsIntWithDefault(key: String, value: Int32) throws -> Int32

    /// Gets a property as a list of strings. The strings must be separated by whitespace or comma. The strings in
    /// the list can contain whitespace and commas if they are enclosed in single or double quotes. If quotes are
    /// mismatched, an empty list is returned. Within single quotes or double quotes, you can escape the quote in
    /// question with a backslash, e.g. O'Reilly can be written as O'Reilly, "O'Reilly" or 'O\'Reilly'.
    ///
    /// - Parameter key: The property key.
    /// - Returns: The property value interpreted as a list of strings, or an empty list if the property is not set.
    func getPropertyAsList(_ key: String) -> StringSeq

    /// Gets an Ice property as a list of strings. The strings must be separated by whitespace or comma. The strings in
    /// the list can contain whitespace and commas if they are enclosed in single or double quotes. If quotes are
    /// mismatched, the default list is returned. Within single quotes or double quotes, you can escape the quote in
    /// question with a backslash, e.g. O'Reilly can be written as O'Reilly, "O'Reilly" or 'O\'Reilly'.
    ///
    /// - Parameter key: The property key.
    /// - Returns: The property value interpreted as a list of strings, or the default value if the property is not
    ///   set.
    /// - Throws: `PropertyException` when the property is not a known Ice property.
    func getIcePropertyAsList(_ key: String) -> StringSeq

    /// Gets a property as a list of strings. The strings must be separated by whitespace or comma. The strings in
    /// the list can contain whitespace and commas if they are enclosed in single or double quotes. If quotes are
    /// mismatched, the default list is returned. Within single quotes or double quotes, you can escape the quote in
    /// question with a backslash, e.g. O'Reilly can be written as O'Reilly, "O'Reilly" or 'O\'Reilly'.
    ///
    /// - Parameters:
    ///   - key: The property key.
    ///   - value: The default value to use if the property is not set.
    /// - Returns: The property value interpreted as a list of strings, or the default value if the property is not set.
    func getPropertyAsListWithDefault(key: String, value: StringSeq) -> StringSeq

    /// Gets all properties whose keys begin with the prefix. If the prefix is the empty string, then all properties
    /// are returned.
    ///
    /// - Parameter prefix: The prefix to search for.
    /// - Returns: The matching property set.
    func getPropertiesForPrefix(_ prefix: String) -> PropertyDict

    /// Sets a property. To unset a property, set it to the empty string.
    ///
    /// - Parameters:
    ///   - key: The property key.
    ///   - value: The property value.
    func setProperty(key: String, value: String)

    /// Gets a sequence of command-line options that is equivalent to this property set. Each element of the returned
    /// sequence is a command-line option of the form `--key=value`.
    ///
    /// - Returns: The command line options for this property set.
    func getCommandLineOptions() -> StringSeq

    /// Converts a sequence of command-line options into properties. All options that start with `--prefix.` are
    /// converted into properties. If the prefix is empty, all options that begin with `--` are converted to
    /// properties.
    ///
    /// - Parameters:
    ///   - prefix: The property prefix, or the empty string to convert all options starting with `--`.
    ///   - options: The command-line options.
    /// - Returns: The command-line options that do not start with the specified prefix, in their original order.
    func parseCommandLineOptions(prefix: String, options: StringSeq) throws -> StringSeq

    /// Converts a sequence of command-line options into properties. All options that start with one of the reserved
    /// Ice prefixes (`--Ice`, `--IceSSL`, etc.) are converted into properties.
    ///
    /// - Parameter options: The command-line options.
    /// - Returns: The command-line options that do not start with one of the reserved prefixes, in their original
    ///   order.
    func parseIceCommandLineOptions(_ options: StringSeq) throws -> StringSeq

    /// Loads properties from a file.
    ///
    /// - Parameter file: The property file.
    func load(_ file: String) throws

    /// Creates a copy of this property set.
    ///
    /// - Returns: A copy of this property set.
    func clone() -> Properties
}
