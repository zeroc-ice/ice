declare module "ice" {
    namespace Ice {
        namespace SSL {
            /**
             * Provides access to an SSL endpoint information.
             */
            class EndpointInfo extends Ice.EndpointInfo {
                /**
                 * One-shot constructor to initialize all data members.
                 * @param underlying The information of the underlying endpoint or null if there's no underlying endpoint.
                 * @param timeout The timeout for the endpoint in milliseconds.
                 * @param compress Specifies whether or not compression should be used if available when using this endpoint.
                 */
                constructor(underlying?: Ice.EndpointInfo, timeout?: number, compress?: boolean);
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
}
