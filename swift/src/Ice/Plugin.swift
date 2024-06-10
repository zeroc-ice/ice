// Copyright (c) ZeroC, Inc.

import Foundation

/// A communicator plug-in. A plug-in generally adds a feature to a communicator, such as support for a protocol.
/// The communicator loads its plug-ins in two stages: the first stage creates the plug-ins, and the second stage
/// invokes Plugin.initialize on each one.
public protocol Plugin: AnyObject {
    /// Perform any necessary initialization steps.
    func initialize() throws

    /// Called when the communicator is being destroyed.
    func destroy() throws
}

/// Each communicator has a plug-in manager to administer the set of plug-ins.
public protocol PluginManager: AnyObject {
    /// Initialize the configured plug-ins. The communicator automatically initializes the plug-ins by default, but an
    /// application may need to interact directly with a plug-in prior to initialization. In this case, the application
    /// must set Ice.InitPlugins=0 and then invoke initializePlugins manually. The plug-ins are
    /// initialized in the order in which they are loaded. If a plug-in raises an exception during initialization, the
    /// communicator invokes destroy on the plug-ins that have already been initialized.
    ///
    /// - throws:
    ///
    ///   - InitializationException - Raised if the plug-ins have already been initialized.
    func initializePlugins() throws

    /// Get a list of plugins installed.
    ///
    /// - returns: `StringSeq` - The names of the plugins installed.
    func getPlugins() -> StringSeq

    /// Obtain a plug-in by name.
    ///
    /// - parameter _: `String` The plug-in's name.
    ///
    /// - returns: `Plugin?` - The plug-in.
    ///
    /// - throws:
    ///
    ///   - NotRegisteredException - Raised if no plug-in is found with the given name.
    func getPlugin(_ name: String) throws -> Plugin?

    /// Install a new plug-in.
    ///
    /// - parameter name: `String` The plug-in's name.
    ///
    /// - parameter pi: `Plugin?` The plug-in.
    ///
    /// - throws:
    ///
    ///   - AlreadyRegisteredException - Raised if a plug-in already exists with the given name.
    func addPlugin(name: String, pi: Plugin?) throws

    /// Called when the communicator is being destroyed.
    func destroy()
}
