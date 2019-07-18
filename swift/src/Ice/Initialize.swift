//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import IceImpl

//
// Factories are registered once when `factoriesRegistered' is lazzy initialized,
// all Swift global variables are lazzy initialized.
//
// All code paths that require the use of the factories before `initialize' is call
// should check `factoriesRegistered' to ensure lazzy initinialization occurrs before
// the factories are used.
//
internal let factoriesRegistered: Bool = {
    ICEUtil.registerFactories(exception: ExceptionFactory.self,
                              connectionInfo: ConnectionInfoFactory.self,
                              endpointInfo: EndpointInfoFactory.self,
                              adminFacet: AdminFacetFactory.self)
    return true
}()

/// Creates a communicator.
///
/// - parameter _: `[String]` - A command-line argument vector. Any Ice-related options
///   in this vector are used to initialize the communicator.
///
/// - parameter initData: `Ice.InitializationData` - Additional intialization data. Property
///   settings in args override property settings in initData.
///
/// - returns: The initialized communicator.
public func initialize(_ args: [String], initData: InitializationData? = nil) throws -> Communicator {
    return try initializeImpl(args: args, initData: initData ?? InitializationData(), withConfigFile: true).0
}

/// Creates a communicator.
///
/// - parameter _: `[String]` - A command-line argument vector. Any Ice-related options
///   in this vector are used to initialize the communicator. This method modifies the
///   argument vector by removing any Ice-related options.
///
/// - parameter initData: `Ice.InitializationData` - Additional intialization data. Property
///   settings in args override property settings in initData.
///
/// - returns: `Ice.Communicator` - The initialized communicator.
public func initialize(_ args: inout [String], initData: InitializationData? = nil) throws -> Communicator {
    let result = try initializeImpl(args: args, initData: initData ?? InitializationData(), withConfigFile: true)
    args = result.1
    return result.0
}

/// Creates a communicator.
///
/// - parameter args: `[String]` - A command-line argument array. Any Ice-related options
///   in this array are used to initialize the communicator.
///
/// - parameter configFile: `String` - Path to a config file that sets the new communicator's
///   default properties.
///
/// - returns: `Ice.Communicator` - The initialized communicator.
public func initialize(args: [String], configFile: String) throws -> Communicator {
    var initData = InitializationData()
    let properties = createProperties()
    try properties.load(configFile)
    initData.properties = properties
    return try initialize(args, initData: initData)
}

/// Creates a communicator.
///
/// - parameter args: `[String]` - A command-line argument array. Any Ice-related options
///   in this array are used to initialize the communicator. This method modifies the
///   argument array by removing any Ice-related options.
///
/// - parameter configFile: `String` - Path to a config file that sets the new communicator's
///   default properties.
///
/// - returns: `Ice.Communicator` - The initialized communicator.
public func initialize(args: inout [String], configFile: String) throws -> Communicator {
    var initData = InitializationData()
    let properties = createProperties()
    try properties.load(configFile)
    initData.properties = properties
    return try initialize(&args, initData: initData)
}

/// Creates a communicator.
///
/// - parameter _: `Ice.InitializationData` - Additional intialization data.
///
/// - returns: `Ice.Communicator` - The initialized communicator.
public func initialize(_ initData: InitializationData? = nil) throws -> Communicator {
    // This is the no-configFile flavor: we never load config from ICE_CONFIG
    return try initializeImpl(args: [], initData: initData ?? InitializationData(), withConfigFile: false).0
}

/// Creates a communicator.
///
/// - parameter _: `String` - Path to a config file that sets the new communicator's default
///   properties.
///
/// - returns: `Ice.Communicator` - The initialized communicator.
public func initialize(_ configFile: String) throws -> Communicator {
    return try initialize(args: [], configFile: configFile)
}

private func initializeImpl(args: [String],
                            initData userInitData: InitializationData,
                            withConfigFile: Bool) throws -> (Communicator, [String]) {
    // Ensure factories are initialized
    guard factoriesRegistered else {
        fatalError("Unable to initialie Ice")
    }

    var initData = userInitData
    if initData.properties == nil {
        initData.properties = createProperties()
    }

    var loggerP: ICELoggerProtocol?
    if let l = initData.logger {
        loggerP = LoggerWrapper(handle: l)
    }

    let propsHandle = (initData.properties as! PropertiesI).handle

    return try autoreleasepool {
        var remArgs: NSArray?
        let handle = try ICEUtil.initialize(args,
                                            properties: propsHandle,
                                            withConfigFile: withConfigFile,
                                            logger: loggerP,
                                            remArgs: &remArgs)

        //
        // Update initData.properties reference to point to the properties object
        // created by Ice::initialize, in case it changed
        //
        let newPropsHandle = handle.getProperties()
        initData.properties = newPropsHandle.getSwiftObject(PropertiesI.self) {
            PropertiesI(handle: newPropsHandle)
        }

        //
        // Update initData.logger referecnce in case we are using a C++ logger (defined though a property) or
        //  a C++ logger plug-in installed a new logger
        //
        if let objcLogger = handle.getLogger() as? ICELogger {
            initData.logger = objcLogger.getSwiftObject(ObjcLoggerWrapper.self) {
                ObjcLoggerWrapper(handle: objcLogger)
            }
        }

        precondition(initData.logger != nil && initData.properties != nil)

        let communicator = CommunicatorI(handle: handle, initData: initData)
        if remArgs == nil {
            return (communicator, [])
        } else {
            // swiftlint:disable force_cast
            return (communicator, remArgs as! [String])
        }
    }
}

/// Creates a new empty property set.
///
/// - returns: `Properties` - A new empty property set.
public func createProperties() -> Properties {
    guard factoriesRegistered else {
        fatalError("Unable to initialie Ice")
    }
    return PropertiesI(handle: ICEUtil.createProperties())
}

/// Creates a property set initialized from an argument array.
///
/// - parameter _: `[String]` - A command-line argument array, possibly containing options to
///   set properties. If the command-line options include a `--Ice.Config` option, the
///   corresponding configuration files are parsed. If the same property is set in a configuration
///   file and in the argument array, the argument array takes precedence.
///
/// - parameter defaults: `Ice.Properties` - Optional default values for the property set. Settings in
///   configuration files and argument array override these defaults.
///
/// - returns: `Ice.Properties` - A new property set initialized with the property settings from the arguments
///   array and defaults.
public func createProperties(_ args: [String], defaults: Properties? = nil) throws -> Properties {
    guard factoriesRegistered else {
        fatalError("Unable to initialie Ice")
    }
    return try autoreleasepool {
        let propertiesHandle = try ICEUtil.createProperties(args,
                                                            defaults: (defaults as? PropertiesI)?.handle,
                                                            remArgs: nil)
        return PropertiesI(handle: propertiesHandle)
    }
}

/// Creates a property set initialized from an argument array.
///
/// - parameter _: `[String]` - A command-line argument array, possibly containing options to
///   set properties. If the command-line options include a `--Ice.Config` option, the
///   corresponding configuration files are parsed. If the same property is set in a configuration
///   file and in the argument array, the argument array takes precedence. This method modifies the
///   argument array by removing any Ice-related options.
///
/// - parameter defaults: `Ice.Properties` - Optional default values for the property set. Settings in
///   configuration files and argument array override these defaults.
///
/// - returns: `Ice.Properties` - A new property set initialized with the property settings from args
///   and defaults.
public func createProperties(_ args: inout [String], defaults: Properties? = nil) throws -> Properties {
    guard factoriesRegistered else {
        fatalError("Unable to initialie Ice")
    }
    return try autoreleasepool {
        var remArgs: NSArray?
        let propertiesHandle = try ICEUtil.createProperties(args,
                                                            defaults: (defaults as? PropertiesI)?.handle,
                                                            remArgs: &remArgs)

        // swiftlint:disable force_cast
        args = remArgs as! [String]
        return PropertiesI(handle: propertiesHandle)
    }
}

/// Returns the Ice version as an integer in the form A.BB.CC, where A
/// indicates the major version, BB indicates the minor version, and CC
/// indicates the patch level. For example, for Ice 3.3.1, the returned
/// value is 30301.
public let intVersion: Int = 30762

/// The Ice version in the form A.B.C, where A indicates the major version,
/// B indicates the minor version, and C indicates the patch level.
public let stringVersion: String = "3.7b2"

public let Encoding_1_0 = EncodingVersion(major: 1, minor: 0)
public let Encoding_1_1 = EncodingVersion(major: 1, minor: 1)

public let currentEncoding = Encoding_1_1

/// Converts a string to an object identity.
///
/// - parameter _: `String` - The string to convert.
///
/// - returns: `Ice.Identity` - The converted object identity.
public func stringToIdentity(_ string: String) throws -> Identity {
    guard factoriesRegistered else {
        fatalError("Unable to initialie Ice")
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
/// - parameter id: `Ice.Identity` - The object identity to convert.
///
/// - parameter mode: `ToStringMode` - Specifies if and how non-printable ASCII characters are escaped
///   in the result.
///
/// - returns: `String` - The string representation of the object identity.
public func identityToString(id: Identity, mode: ToStringMode = ToStringMode.Unicode) -> String {
    return ICEUtil.identityToString(name: id.name, category: id.category, mode: mode.rawValue)
}

/// Converts an encoding version to a string.
///
/// - parameter _: `Ice.EncodingVersion` - The encoding version to convert.
///
/// - returns: `String` - The converted string.
public func encodingVersionToString(_ encoding: EncodingVersion) -> String {
    return ICEUtil.encodingVersionToString(major: encoding.major, minor: encoding.minor)
}
