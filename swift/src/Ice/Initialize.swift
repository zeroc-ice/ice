// Copyright (c) ZeroC, Inc.

import IceImpl

//
// Factories are registered once when `factoriesRegistered' is lazy initialized,
// all Swift global variables are lazy initialized.
//
// All code paths that require the use of the factories before `initialize' is call
// should check `factoriesRegistered' to ensure lazy initialization occurs before
// the factories are used.
//
let factoriesRegistered: Bool = {
    ICEUtil.registerFactories(
        exception: LocalExceptionFactory.self,
        connectionInfo: ConnectionInfoFactory.self,
        endpointInfo: EndpointInfoFactory.self,
        adminFacet: AdminFacetFactory.self)
    return true
}()

/// Creates a new communicator, using Ice properties parsed from the command-line arguments.
/// - Parameter args: The command-line arguments. This function parses arguments starting with `--` and one of the
///   reserved prefixes (Ice, IceSSL, etc.) as properties for the new communicator. If there is an argument starting
///   with `--Ice.Config`, this function loads the specified configuration file. When the same property is set in a
///   configuration file and through a command-line argument, the command-line setting takes precedence.
/// - Returns: The new communicator.
public func initialize(_ args: [String]) throws -> Communicator {
    try initialize(InitializationData(properties: createProperties(args)))
}

/// Creates a new communicator, using Ice properties parsed from the command-line arguments.
/// - Parameter args: The command-line arguments. This function parses arguments starting with `--` and one of the
///   reserved prefixes (Ice, IceSSL, etc.) as properties for the new communicator. If there is an argument starting
///   with `--Ice.Config`, this function loads the specified configuration file. When the same property is set in a
///   configuration file and through a command-line argument, the command-line setting takes precedence. This function
///   modifies args by removing any Ice-related options.
/// - Returns: The new communicator.
public func initialize(_ args: inout [String]) throws -> Communicator {
    try initialize(InitializationData(properties: createProperties(&args)))
}

/// Creates a new communicator.
/// - Parameter initData: Options for the new communicator.
/// - Returns: The new communicator.
public func initialize(_ initData: InitializationData = InitializationData()) throws -> Communicator {
    // Ensure factories are initialized.
    guard factoriesRegistered else {
        fatalError("Unable to initialize Ice")
    }

    var newInitData = initData
    newInitData.properties = initData.properties ?? createProperties()

    var loggerP: ICELoggerProtocol?
    if let logger = initData.logger {
        loggerP = LoggerWrapper(handle: logger)
    }

    let propsHandle = (newInitData.properties as! PropertiesI).handle

    let properties = newInitData.properties!

    // If the user has not set Ice.ProgramName, set it to the Swift executable name.
    if properties.getIceProperty("Ice.ProgramName").isEmpty {
        if let programName = CommandLine.arguments.first.map({ URL(fileURLWithPath: $0).lastPathComponent }) {
            properties.setProperty(key: "Ice.ProgramName", value: programName)
        } else {
            properties.setProperty(key: "Ice.ProgramName", value: "IceSwift")
        }
    }

    return try autoreleasepool {
        let handle = try ICEUtil.initialize(propsHandle, logger: loggerP)

        precondition(propsHandle === handle.getProperties(), "initialize changed the properties object")

        // Update newInitData.logger reference in case we are using a C++ logger (defined though a property) or
        // a C++ logger plug-in installed a new logger.
        if let objcLogger = handle.getLogger() as? ICELogger {
            newInitData.logger = objcLogger.getSwiftObject(ObjcLoggerWrapper.self) {
                ObjcLoggerWrapper(handle: objcLogger)
            }
        }

        precondition(newInitData.logger != nil)

        if let sliceLoader = newInitData.sliceLoader {
            newInitData.sliceLoader = CompositeSliceLoader(sliceLoader, DefaultSliceLoader())
        } else {
            newInitData.sliceLoader = DefaultSliceLoader()
        }

        let notFoundCacheSize = try newInitData.properties!.getIcePropertyAsInt("Ice.SliceLoader.NotFoundCacheSize")
        if notFoundCacheSize > 0 {
            let cacheFullLogger =
                try newInitData.properties!.getIcePropertyAsInt("Ice.Warn.SliceLoader") > 0 ? newInitData.logger : nil
            newInitData.sliceLoader = NotFoundSliceLoaderDecorator(
                newInitData.sliceLoader!,
                cacheSize: notFoundCacheSize,
                logger: cacheFullLogger)
        }

        return CommunicatorI(handle: handle, initData: newInitData)
    }
}

/// Creates a new empty property set.
///
/// - Returns: A new empty property set.
public func createProperties() -> Properties {
    guard factoriesRegistered else {
        fatalError("Unable to initialize Ice")
    }
    return PropertiesI(handle: ICEUtil.createProperties())
}

/// Creates a property set initialized from an argument array.
///
/// - Parameters:
///   - args: A command-line argument array, possibly containing options to set properties. If the
///     command-line options include `--Ice.Config`, the corresponding configuration files are parsed. If the same
///     property is set in a configuration file and in the argument array, the argument array takes precedence.
///   - defaults: Optional default values for the property set. Settings in configuration files and the
///     argument array override these defaults.
/// - Returns: A new property set initialized with the property settings from the arguments array and defaults.
public func createProperties(_ args: [String], defaults: Properties? = nil) throws -> Properties {
    guard factoriesRegistered else {
        fatalError("Unable to initialize Ice")
    }
    return try autoreleasepool {
        let propertiesHandle = try ICEUtil.createProperties(
            args,
            defaults: (defaults as? PropertiesI)?.handle,
            remArgs: nil)
        return PropertiesI(handle: propertiesHandle)
    }
}

/// Creates a property set initialized from an argument array.
///
/// - Parameters:
///   - args: A command-line argument array, possibly containing options to set properties. If the
///     command-line options include `--Ice.Config`, the corresponding configuration files are parsed. If the same
///     property is set in a configuration file and in the argument array, the argument array takes precedence. This
///     method modifies the argument array by removing any Ice-related options.
///   - defaults: Optional default values for the property set. Settings in configuration files and the
///     argument array override these defaults.
/// - Returns: A new property set initialized with the property settings from the arguments and defaults.
public func createProperties(_ args: inout [String], defaults: Properties? = nil) throws
    -> Properties
{
    guard factoriesRegistered else {
        fatalError("Unable to initialize Ice")
    }
    return try autoreleasepool {
        var remArgs: NSArray?
        let propertiesHandle = try ICEUtil.createProperties(
            args,
            defaults: (defaults as? PropertiesI)?.handle,
            remArgs: &remArgs)

        // swiftlint:disable force_cast
        args = remArgs as! [String]
        // swiftlint:enable force_cast
        return PropertiesI(handle: propertiesHandle)
    }
}

/// Returns the Ice version as an integer in the form AABBCC, where AA
/// indicates the major version, BB indicates the minor version, and CC
/// indicates the patch level. For example, for Ice 3.8.1, the returned
/// value is 30801.
public let intVersion: Int = 30850

/// The Ice version in the form A.B.C, where A indicates the major version,
/// B indicates the minor version, and C indicates the patch level.
public let stringVersion: String = "3.8.0-alpha.0"

public let Encoding_1_0 = EncodingVersion(major: 1, minor: 0)
public let Encoding_1_1 = EncodingVersion(major: 1, minor: 1)

public let currentEncoding = Encoding_1_1

/// Converts a string to an object identity.
///
/// - Parameter string: The string to convert.
/// - Returns: The converted object identity.
public func stringToIdentity(_ string: String) throws -> Identity {
    guard factoriesRegistered else {
        fatalError("Unable to initialize Ice")
    }
    return try autoreleasepool {
        var name = NSString()
        var category = NSString()
        try ICEUtil.stringToIdentity(str: string, name: &name, category: &category)
        return Identity(name: name as String, category: category as String)
    }
}

/// Converts an object identity to a string.
///
/// - Parameters:
///   - id: The object identity to convert.
///   - mode: Specifies if and how non-printable ASCII characters are escaped in the result.
/// - Returns: The string representation of the object identity.
public func identityToString(id: Identity, mode: ToStringMode = .Unicode) -> String {
    precondition(!id.name.isEmpty, "Invalid identity with an empty name")
    return ICEUtil.identityToString(name: id.name, category: id.category, mode: mode.rawValue)
}

/// Converts an encoding version to a string.
///
/// - Parameter encoding: The encoding version to convert.
/// - Returns: The converted string.
public func encodingVersionToString(_ encoding: EncodingVersion) -> String {
    return ICEUtil.encodingVersionToString(major: encoding.major, minor: encoding.minor)
}
