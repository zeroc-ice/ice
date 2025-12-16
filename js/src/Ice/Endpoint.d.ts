// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * An endpoint specifies the address of the server-end of an Ice connection: an object adapter listens on one or
         * more endpoints and a client establishes a connection to an endpoint.
         */
        interface Endpoint {
            /**
             * Return a string representation of the endpoint.
             * @returns The string representation of the endpoint.
             */
            toString(): string;

            /**
             * Returns this endpoint's information.
             * @returns This endpoint's information class
             */
            getInfo(): Ice.EndpointInfo;

            /**
             * Checks if this endpoint is equal to the specified object.
             *
             * @param other The object to compare against.
             * @returns `true` if the objects are equal, `false` otherwise.
             */
            equals(other: any): boolean;
        }

        /**
         * Base class for the endpoint info classes.
         */
        class EndpointInfo {
            /**
             * The information of the underlying endpoint or null if there's no underlying endpoint.
             */
            get underlying(): Ice.EndpointInfo | null;

            /**
             * The timeout for the endpoint in milliseconds. -1 means no timeout.
             */
            get timeout(): number;

            /**
             * Specifies whether or not compression should be used if available when using this endpoint.
             */
            get compress(): boolean;

            /**
             * Returns the type of the endpoint.
             * @returns The endpoint type.
             */
            type(): number;

            /**
             * Returns `true` if this endpoint's transport uses WSS, `false` otherwise.
             * @returns `true` for WSS transport, `false` otherwise.
             */
            secure(): boolean;
        }

        /**
         * Provides access to the address details of a IP endpoint.
         * @see Endpoint
         */
        class IPEndpointInfo extends EndpointInfo {
            /**
             * The host or address configured with the endpoint.
             */
            get host(): string;

            /**
             * The port number.
             */
            get port(): number;

            /**
             * The source IP address.
             */
            get sourceAddress(): string;
        }

        /**
         * Provides access to a TCP endpoint information.
         * @see Endpoint
         */
        class TCPEndpointInfo extends IPEndpointInfo {}

        /**
         * Provides access to a WebSocket endpoint information.
         */
        class WSEndpointInfo extends EndpointInfo {
            /**
             * The URI configured with the endpoint.
             */
            get resource(): string;
        }

        /**
         * Provides access to the details of an opaque endpoint.
         * @see Endpoint
         */
        class OpaqueEndpointInfo extends EndpointInfo {
            /**
             * The encoding version of the opaque endpoint (to decode or encode the rawBytes).
             */
            get rawEncoding(): EncodingVersion;

            /**
             * The raw encoding of the opaque endpoint.
             */
            get rawBytes(): ByteSeq;
        }
    }
}
