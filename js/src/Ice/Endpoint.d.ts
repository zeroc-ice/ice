//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice" {
    namespace Ice {
        /**
         * Base class providing access to the endpoint details.
         */
        class EndpointInfo {
            /**
             * One-shot constructor to initialize all data members.
             * @param underlying The information of the underlying endpoint or null if there's no underlying endpoint.
             * @param timeout The timeout for the endpoint in milliseconds.
             * @param compress Specifies whether or not compression should be used if available when using this endpoint.
             */
            constructor(underlying?: Ice.EndpointInfo, timeout?: number, compress?: boolean);
            /**
             * The information of the underlying endpoint or null if there's no underlying endpoint.
             */
            underlying: Ice.EndpointInfo;
            /**
             * The timeout for the endpoint in milliseconds. 0 means non-blocking, -1 means no timeout.
             */
            timeout: number;
            /**
             * Specifies whether or not compression should be used if available when using this endpoint.
             */
            compress: boolean;
            /**
             * Returns the type of the endpoint.
             * @return The endpoint type.
             */
            type(): number;
            /**
             * Returns true if this endpoint is a datagram endpoint.
             * @return True for a datagram endpoint.
             */
            datagram(): boolean;
            /**
             * @return True for a secure endpoint.
             */
            secure(): boolean;
        }

        /**
         * The user-level interface to an endpoint.
         */
        interface Endpoint {
            /**
             * Return a string representation of the endpoint.
             * @return The string representation of the endpoint.
             */
            toString(): string;
            /**
             * Returns the endpoint information.
             * @return The endpoint information class.
             */
            getInfo(): Ice.EndpointInfo;
            equals(rhs: any): boolean;
        }

        /**
         * Provides access to the address details of a IP endpoint.
         * @see Endpoint
         */
        class IPEndpointInfo extends EndpointInfo {
            /**
             * One-shot constructor to initialize all data members.
             * @param underlying The information of the underlying endpoint or null if there's no underlying endpoint.
             * @param timeout The timeout for the endpoint in milliseconds.
             * @param compress Specifies whether or not compression should be used if available when using this endpoint.
             * @param host The host or address configured with the endpoint.
             * @param port The port number.
             * @param sourceAddress The source IP address.
             */
            constructor(
                underlying?: Ice.EndpointInfo,
                timeout?: number,
                compress?: boolean,
                host?: string,
                port?: number,
                sourceAddress?: string,
            );
            /**
             * The host or address configured with the endpoint.
             */
            host: string;
            /**
             * The port number.
             */
            port: number;
            /**
             * The source IP address.
             */
            sourceAddress: string;
            /**
             * Returns the type of the endpoint.
             * @return The endpoint type.
             */
            type(): number;
            /**
             * Returns true if this endpoint is a datagram endpoint.
             * @return True for a datagram endpoint.
             */
            datagram(): boolean;
            /**
             * @return True for a secure endpoint.
             */
            secure(): boolean;
        }

        /**
         * Provides access to a TCP endpoint information.
         * @see Endpoint
         */
        class TCPEndpointInfo extends IPEndpointInfo {
            /**
             * One-shot constructor to initialize all data members.
             * @param underlying The information of the underlying endpoint or null if there's no underlying endpoint.
             * @param timeout The timeout for the endpoint in milliseconds.
             * @param compress Specifies whether or not compression should be used if available when using this endpoint.
             * @param host The host or address configured with the endpoint.
             * @param port The port number.
             * @param sourceAddress The source IP address.
             */
            constructor(
                underlying?: Ice.EndpointInfo,
                timeout?: number,
                compress?: boolean,
                host?: string,
                port?: number,
                sourceAddress?: string,
            );
            /**
             * Returns the type of the endpoint.
             * @return The endpoint type.
             */
            type(): number;
            /**
             * Returns true if this endpoint is a datagram endpoint.
             * @return True for a datagram endpoint.
             */
            datagram(): boolean;
            /**
             * @return True for a secure endpoint.
             */
            secure(): boolean;
        }

        /**
         * Provides access to a WebSocket endpoint information.
         */
        class WSEndpointInfo extends EndpointInfo {
            /**
             * One-shot constructor to initialize all data members.
             * @param underlying The information of the underlying endpoint or null if there's no underlying endpoint.
             * @param timeout The timeout for the endpoint in milliseconds.
             * @param compress Specifies whether or not compression should be used if available when using this endpoint.
             * @param resource The URI configured with the endpoint.
             */
            constructor(underlying?: Ice.EndpointInfo, timeout?: number, compress?: boolean, resource?: string);
            /**
             * The URI configured with the endpoint.
             */
            resource: string;
            /**
             * Returns the type of the endpoint.
             * @return The endpoint type.
             */
            type(): number;
            /**
             * Returns true if this endpoint is a datagram endpoint.
             * @return True for a datagram endpoint.
             */
            datagram(): boolean;
            /**
             * @return True for a secure endpoint.
             */
            secure(): boolean;
        }

        /**
         * Provides access to the details of an opaque endpoint.
         * @see Endpoint
         */
        class OpaqueEndpointInfo extends EndpointInfo {
            /**
             * One-shot constructor to initialize all data members.
             * @param underlying The information of the underlying endpoint or null if there's no underlying endpoint.
             * @param timeout The timeout for the endpoint in milliseconds.
             * @param compress Specifies whether or not compression should be used if available when using this endpoint.
             * @param rawEncoding The encoding version of the opaque endpoint (to decode or encode the rawBytes).
             * @param rawBytes The raw encoding of the opaque endpoint.
             */
            constructor(
                underlying?: Ice.EndpointInfo,
                timeout?: number,
                compress?: boolean,
                rawEncoding?: EncodingVersion,
                rawBytes?: ByteSeq,
            );
            /**
             * The encoding version of the opaque endpoint (to decode or encode the rawBytes).
             */
            rawEncoding: EncodingVersion;
            /**
             * The raw encoding of the opaque endpoint.
             */
            rawBytes: ByteSeq;
            /**
             * Returns the type of the endpoint.
             * @return The endpoint type.
             */
            type(): number;
            /**
             * Returns true if this endpoint is a datagram endpoint.
             * @return True for a datagram endpoint.
             */
            datagram(): boolean;
            /**
             * @return True for a secure endpoint.
             */
            secure(): boolean;
        }
    }
}
