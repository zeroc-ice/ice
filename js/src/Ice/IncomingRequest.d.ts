//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice" {
    namespace Ice {
        class IncomingRequest {
            current: Current;
            inputStream: InputStream;
            size: number;
            constructor(requestId: number, connection: Connection, adapter: ObjectAdapter, inputStream: InputStream);
        }
    }
}
