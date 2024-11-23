// Copyright (c) ZeroC, Inc.

/**
 *  Base class providing access to the endpoint details.
 **/
export class EndpointInfo {}

/**
 *  Provides access to the address details of a IP endpoint.
 *  @see {@link Endpoint}
 **/
export class IPEndpointInfo extends EndpointInfo {}

/**
 *  Provides access to a TCP endpoint information.
 *  @see {@link Endpoint}
 **/
export class TCPEndpointInfo extends IPEndpointInfo {}

/**
 *  Provides access to a WebSocket endpoint information.
 **/
export class WSEndpointInfo extends EndpointInfo {}

/**
 *  Provides access to the details of an opaque endpoint.
 *  @see {@link Endpoint}
 **/
export class OpaqueEndpointInfo extends EndpointInfo {}
