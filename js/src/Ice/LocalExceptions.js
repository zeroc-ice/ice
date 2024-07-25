//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { LocalException } from "./LocalException.js";
import { Ice as Ice_Identity } from "./Identity.js";
import { identityToString } from "./IdentityToString.js";
import { Ice as Ice_Version } from "./Version.js";
import { ObjectPrx } from "./ObjectPrx.js";

const Ice = {
    ...Ice_Identity,
    ...Ice_Version,
};

// This file contains all the exception classes derived from LocalException defined in the Ice assembly.

//
// The 6 (7 with the RequestFailedException base class) special local exceptions that can be marshaled in an Ice reply
// message. Other local exceptions can't be marshaled.
//

export class RequestFailedException extends LocalException {
    static get _id() {
        return "::Ice::RequestFailedException";
    }

    constructor(typeName = "", id = new Ice.Identity(), facet = "", operation = "") {
        super(RequestFailedException.createMessage(typeName, id, facet, operation));
        this.id = id;
        this.facet = facet;
        this.operation = operation;
    }

    static createMessage(typeName, id, facet, operation) {
        return `Dispatch failed with ${typeName} { id = '${identityToString(id)}', facet = '${facet}', operation = '${operation}' }`;
    }
}
export class ObjectNotExistException extends RequestFailedException {
    static get _id() {
        return "::Ice::ObjectNotExistException";
    }

    constructor(id = new Ice.Identity(), facet = "", operation = "") {
        super("ObjectNotExistException", id, facet, operation);
    }
}

export class FacetNotExistException extends RequestFailedException {
    static get _id() {
        return "::Ice::FacetNotExistException";
    }

    constructor(id = new Ice.Identity(), facet = "", operation = "") {
        super("FacetNotExistException", id, facet, operation);
    }
}

export class OperationNotExistException extends RequestFailedException {
    static get _id() {
        return "::Ice::OperationNotExistException";
    }

    constructor(id = new Ice.Identity(), facet = "", operation = "") {
        super("OperationNotExistException", id, facet, operation);
    }
}

export class UnknownException extends LocalException {
    static get _id() {
        return "::Ice::UnknownException";
    }

    constructor(message) {
        super(message);
    }

    get unknown() {
        return this.message;
    }
}

export class UnknownLocalException extends UnknownException {
    static get _id() {
        return "::Ice::UnknownLocalException";
    }

    constructor(message) {
        super(message);
    }
}

export class UnknownUserException extends UnknownException {
    static get _id() {
        return "::Ice::UnknownUserException";
    }

    constructor(message) {
        super(message);
    }
}

//
// Protocol exceptions
//

export class ProtocolException extends LocalException {
    static get _id() {
        return "::Ice::ProtocolException";
    }
}

export class CloseConnectionException extends ProtocolException {
    static get _id() {
        return "::Ice::CloseConnectionException";
    }

    constructor() {
        super("Connection closed by the peer.");
    }
}

export class MarshalException extends ProtocolException {
    static get _id() {
        return "::Ice::MarshalException";
    }
}

//
// Timeout exceptions
//

export class TimeoutException extends LocalException {
    static get _id() {
        return "::Ice::TimeoutException";
    }

    constructor(message = undefined, cause = undefined) {
        super(message || "operation timed out", cause);
    }
}

export class CloseTimeoutException extends TimeoutException {
    static get _id() {
        return "::Ice::CloseTimeoutException";
    }

    constructor() {
        super("Close timed out.");
    }
}

export class ConnectTimeoutException extends TimeoutException {
    static get _id() {
        return "::Ice::ConnectTimeoutException";
    }

    constructor() {
        super("Connect timed out.");
    }
}

export class InvocationTimeoutException extends TimeoutException {
    static get _id() {
        return "::Ice::InvocationTimeoutException";
    }

    constructor() {
        super("Invocation timed out.");
    }
}

//
// Syscall exceptions
//

export class SyscallException extends LocalException {
    static get _id() {
        return "::Ice::SyscallException";
    }
}

//
// Socket exceptions
//

export class SocketException extends SyscallException {
    static get _id() {
        return "::Ice::SocketException";
    }
}

export class ConnectFailedException extends SocketException {
    static get _id() {
        return "::Ice::ConnectFailedException";
    }
}

export class ConnectionLostException extends SocketException {
    static get _id() {
        return "::Ice::ConnectionLostException";
    }
}

export class ConnectionRefusedException extends ConnectFailedException {
    static get _id() {
        return "::Ice::ConnectionRefusedException";
    }
}

//
// Other leaf local exceptions in alphabetical order.
//

export class AlreadyRegisteredException extends LocalException {
    static get _id() {
        return "::Ice::AlreadyRegisteredException";
    }

    constructor(kindOfObject, id) {
        super(`Another ${kindOfObject} is already registered with ID '${id}'.`);
        this.kindOfObject = kindOfObject;
        this.id = id;
    }
}

export class CommunicatorDestroyedException extends LocalException {
    static get _id() {
        return "::Ice::CommunicatorDestroyedException";
    }

    constructor() {
        super("Communicator destroyed.");
    }
}

export class ConnectionAbortedException extends LocalException {
    static get _id() {
        return "::Ice::ConnectionAbortedException";
    }

    constructor(message, closedByApplication) {
        super(message);
        this.closedByApplication = closedByApplication;
    }
}

export class ConnectionClosedException extends LocalException {
    static get _id() {
        return "::Ice::ConnectionClosedException";
    }

    constructor(message, closedByApplication) {
        super(message);
        this.closedByApplication = closedByApplication;
    }
}

export class FeatureNotSupportedException extends LocalException {
    static get _id() {
        return "::Ice::FeatureNotSupportedException";
    }

    constructor(message) {
        super(message);
    }
}

export class FixedProxyException extends LocalException {
    static get _id() {
        return "::Ice::FixedProxyException";
    }

    constructor() {
        super("Cannot change the connection properties of a fixed proxy.");
    }
}

export class InitializationException extends LocalException {
    static get _id() {
        return "::Ice::InitializationException";
    }
}

export class InvocationCanceledException extends LocalException {
    static get _id() {
        return "::Ice::InvocationCanceledException";
    }

    constructor() {
        super("Invocation canceled.");
    }
}

export class NoEndpointException extends LocalException {
    static get _id() {
        return "::Ice::NoEndpointException";
    }

    constructor(strOrProxy) {
        super(NoEndpointException.createMessage(strOrProxy));
    }

    static createMessage(arg) {
        return arg instanceof ObjectPrx ? `$"No suitable endpoint available for proxy '${arg}'.` : arg;
    }
}

export class NotRegisteredException extends LocalException {
    static get _id() {
        return "::Ice::NotRegisteredException";
    }

    constructor(kindOfObject, id) {
        super(`No ${kindOfObject} is registered with ID '${id}'.`);
        this.kindOfObject = kindOfObject;
        this.id = id;
    }
}

export class ObjectAdapterDeactivatedException extends LocalException {
    static get _id() {
        return "::Ice::ObjectAdapterDeactivatedException";
    }

    constructor(name) {
        super(`Object adapter '${name}' is deactivated.`);
    }
}

export class ObjectAdapterIdInUseException extends LocalException {
    static get _id() {
        return "::Ice::ObjectAdapterIdInUseException";
    }

    constructor(adapterId) {
        super(`An object adapter with adapter ID '${adapterId}' is already active.`);
    }
}

export class ParseException extends LocalException {
    static get _id() {
        return "::Ice::ParseException";
    }
}

export class TwowayOnlyException extends LocalException {
    static get _id() {
        return "::Ice::TwowayOnlyException";
    }

    constructor(operation) {
        super(`Cannot invoke operation '${operation}' with a oneway, batchOneway, datagram, or batchDatagram proxy.`);
        this.operation = operation;
    }
}
