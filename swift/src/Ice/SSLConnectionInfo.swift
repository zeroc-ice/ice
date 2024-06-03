//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation

/// Provides access to the connection details of an SSL connection
public protocol SSLConnectionInfo: ConnectionInfo {
    /// The certificate chain.
    var peerCertificate: SecCertificate? { get set }
}
