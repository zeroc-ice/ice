//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice" {
    namespace Ice {
        /**
         * Information about the current method invocation for servers. Each operation on the server has a
         * <code>Current</code> as its implicit final parameter. <code>Current</code> is mostly used for Ice services. Most
         * applications ignore this parameter.
         */
        class Current {
            constructor(
                adapter?: Ice.ObjectAdapter,
                con?: Ice.Connection,
                id?: Identity,
                facet?: string,
                operation?: string,
                mode?: OperationMode,
                ctx?: Context,
                requestId?: number,
                encoding?: EncodingVersion,
            );
            clone(): Current;
            equals(rhs: any): boolean;
            adapter: Ice.ObjectAdapter;
            con: Ice.Connection;
            id: Identity;
            facet: string;
            operation: string;
            mode: OperationMode;
            ctx: Context;
            requestId: number;
            encoding: EncodingVersion;
            static write(outs: OutputStream, value: Current): void;
            static read(ins: InputStream): Current;
        }
    }
}
