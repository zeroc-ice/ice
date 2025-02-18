// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        namespace SSL {
            /**
             * Provides access to the connection details of an SSL connection
             */
            class ConnectionInfo extends Ice.ConnectionInfo {
                // TODO: add missing fields (cipher, certs, verified)
            }
        }
    }
}
