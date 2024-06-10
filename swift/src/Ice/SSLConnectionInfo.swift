// Copyright (c) ZeroC, Inc.

import Foundation

/// Provides access to the connection details of an SSL connection
public protocol SSLConnectionInfo: ConnectionInfo {
    /// The certificate chain.
    var peerCertificate: SecCertificate? { get set }
}
