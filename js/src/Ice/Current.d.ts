//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

export namespace Ice
{
    /**
     * A request context. <code>Context</code> is used to transmit metadata about a
     * request from the server to the client, such as Quality-of-Service
     * (QoS) parameters. Each operation on the client has a <code>Context</code> as
     * its implicit final parameter.
     */
    class Context extends Map<string, string>
    {
    }

    class ContextHelper
    {
        static write(outs:OutputStream, value:Context):void;
        static read(ins:InputStream):Context;
    }

    /**
     * Determines the retry behavior an invocation in case of a (potentially) recoverable error.
     */
    class OperationMode
    {
        /**
         * Ordinary operations have <code>Normal</code> mode.  These operations
         * modify object state; invoking such an operation twice in a row
         * has different semantics than invoking it once. The Ice run time
         * guarantees that it will not violate at-most-once semantics for
         * <code>Normal</code> operations.
         */
        static readonly Normal:OperationMode;
        /**
         * Operations that use the Slice <code>nonmutating</code> keyword must not
         * modify object state. For C++, nonmutating operations generate
         * <code>const</code> member functions in the skeleton. In addition, the Ice
         * run time will attempt to transparently recover from certain
         * run-time errors by re-issuing a failed request and propagate
         * the failure to the application only if the second attempt
         * fails.
         *
         * <p class="Deprecated"><code>Nonmutating</code> is deprecated; Use the
         * <code>idempotent</code> keyword instead. For C++, to retain the mapping
         * of <code>nonmutating</code> operations to C++ <code>const</code>
         * member functions, use the <code>\["cpp:const"]</code> metadata
         * directive.
         */
        static readonly Nonmutating:OperationMode;
        /**
         * Operations that use the Slice <code>idempotent</code> keyword can modify
         * object state, but invoking an operation twice in a row must
         * result in the same object state as invoking it once.  For
         * example, <code>x = 1</code> is an idempotent statement,
         * whereas <code>x += 1</code> is not. For idempotent
         * operations, the Ice run-time uses the same retry behavior
         * as for nonmutating operations in case of a potentially
         * recoverable error.
         */
        static readonly Idempotent:OperationMode;
        
        static valueOf(value:number):OperationMode;
        equals(other:any):boolean;
        hashCode():number;
        toString():string;
        
        readonly name:string;
        readonly value:number;
    }

    /**
     * Information about the current method invocation for servers. Each
     * operation on the server has a <code>Current</code> as its implicit final
     * parameter. <code>Current</code> is mostly used for Ice services. Most
     * applications ignore this parameter.
     */
    class Current
    {
        constructor(adapter?:Ice.ObjectAdapter, con?:Ice.Connection, id?:Identity, facet?:string, operation?:string, mode?:OperationMode, ctx?:Context, requestId?:number, encoding?:EncodingVersion);
        clone():Current;
        equals(rhs:any):boolean;
        adapter:Ice.ObjectAdapter;
        con:Ice.Connection;
        id:Identity;
        facet:string;
        operation:string;
        mode:OperationMode;
        ctx:Context;
        requestId:number;
        encoding:EncodingVersion;
        static write(outs:OutputStream, value:Current):void;
        static read(ins:InputStream):Current;
    }
}
