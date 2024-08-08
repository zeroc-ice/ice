//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice" {
    namespace Ice {
        namespace SSL {
            /**
             * Provides access to the connection details of an SSL connection
             */
            class ConnectionInfo extends Ice.ConnectionInfo {
                /**
                 * One-shot constructor to initialize all data members.
                 * @param underlying The information of the underlying transport or null if there's no underlying transport.
                 * @param incoming Whether or not the connection is an incoming or outgoing connection.
                 * @param adapterName The name of the adapter associated with the connection.
                 * @param connectionId The connection id.
                 */
                constructor(
                    underlying?: Ice.ConnectionInfo,
                    incoming?: boolean,
                    adapterName?: string,
                    connectionId?: string,
                    cipher?: string,
                    certs?: Ice.StringSeq,
                    verified?: boolean,
                );
            }
        }
    }
}
