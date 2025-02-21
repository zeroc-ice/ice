// Copyright (c) ZeroC, Inc.

import { LocalException } from "./LocalException.js";
import { identityToString } from "./IdentityToString.js";
import { ObjectPrx } from "./ObjectPrx.js";
import { Ice as Ice_Identity } from "./Identity.js";
const { Identity } = Ice_Identity;
import { Ice as Ice_ReplyStatus } from "./ReplyStatus.js";
const { ReplyStatus } = Ice_ReplyStatus;

// This file contains all the exception classes derived from LocalException defined in the Ice assembly.

//
// The 7 (8 with the RequestFailedException base class) special local exceptions that can be marshaled in an Ice reply
// message. Other local exceptions can't be marshaled.
//

export class DispatchException extends LocalException {
    static get _ice_id() {
        return "::Ice::DispatchException";
    }

    constructor(replyStatus, message) {
        super(DispatchException.createDispatchFailedMessage(replyStatus, message));
        this._replyStatus = replyStatus;

        DEV: console.assert(
            replyStatus.value > ReplyStatus.UserException.value && replyStatus.value <= 255,
            "The reply status must fit in a byte and have a value greater than UserException",
        );
    }

    get replyStatus() {
        return this._replyStatus;
    }

    static createDispatchFailedMessage(replyStatus, message) {
        if (message !== undefined) {
            return message;
        } else {
            return `The dispatch failed with reply status ${replyStatus}.`;
        }
    }
}

export class RequestFailedException extends DispatchException {
    static get _ice_id() {
        return "::Ice::RequestFailedException";
    }

    constructor(replyStatus, id = new Identity(), facet = "", operation = "") {
        super(replyStatus, RequestFailedException.createRequestFailedMessage(replyStatus, id, facet, operation));
        this._id = id;
        this._facet = facet;
        this._operation = operation;
    }

    get id() {
        return this._id;
    }

    get facet() {
        return this._facet;
    }

    get operation() {
        return this._operation;
    }

    static createRequestFailedMessage(replyStatus, id, facet, operation) {
        if (id.name.length > 0) {
            return `Dispatch failed with ${replyStatus} { id = '${identityToString(id)}', facet = '${facet}', operation = '${operation}' }`;
        } else {
            return undefined;
        }
    }
}

export class ObjectNotExistException extends RequestFailedException {
    static get _ice_id() {
        return "::Ice::ObjectNotExistException";
    }

    constructor(id, facet, operation) {
        super(ReplyStatus.ObjectNotExist, id, facet, operation);
    }
}

export class FacetNotExistException extends RequestFailedException {
    static get _ice_id() {
        return "::Ice::FacetNotExistException";
    }

    constructor(id, facet, operation) {
        super(ReplyStatus.FacetNotExist, id, facet, operation);
    }
}

export class OperationNotExistException extends RequestFailedException {
    static get _ice_id() {
        return "::Ice::OperationNotExistException";
    }

    constructor(id, facet, operation) {
        super(ReplyStatus.OperationNotExist, id, facet, operation);
    }
}

export class UnknownException extends DispatchException {
    static get _ice_id() {
        return "::Ice::UnknownException";
    }

    constructor(message, replyStatus = ReplyStatus.UnknownException) {
        super(replyStatus, message);
    }

    get unknown() {
        return this.message;
    }
}

export class UnknownLocalException extends UnknownException {
    static get _ice_id() {
        return "::Ice::UnknownLocalException";
    }

    constructor(message) {
        super(message, ReplyStatus.UnknownLocalException);
    }
}

export class UnknownUserException extends UnknownException {
    static get _ice_id() {
        return "::Ice::UnknownUserException";
    }

    constructor(message) {
        super(message, ReplyStatus.UnknownUserException);
    }
}

//
// Protocol exceptions
//

export class ProtocolException extends LocalException {
    static get _ice_id() {
        return "::Ice::ProtocolException";
    }
}

export class CloseConnectionException extends ProtocolException {
    static get _ice_id() {
        return "::Ice::CloseConnectionException";
    }

    constructor() {
        super("Connection closed by the peer.");
    }
}

export class MarshalException extends ProtocolException {
    static get _ice_id() {
        return "::Ice::MarshalException";
    }
}

//
// Timeout exceptions
//

export class TimeoutException extends LocalException {
    static get _ice_id() {
        return "::Ice::TimeoutException";
    }

    constructor(message = undefined, ...params) {
        super(message || "operation timed out", ...params);
    }
}

export class CloseTimeoutException extends TimeoutException {
    static get _ice_id() {
        return "::Ice::CloseTimeoutException";
    }

    constructor() {
        super("Close timed out.");
    }
}

export class ConnectTimeoutException extends TimeoutException {
    static get _ice_id() {
        return "::Ice::ConnectTimeoutException";
    }

    constructor() {
        super("Connect timed out.");
    }
}

export class InvocationTimeoutException extends TimeoutException {
    static get _ice_id() {
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
    static get _ice_id() {
        return "::Ice::SyscallException";
    }
}

//
// Socket exceptions
//

export class SocketException extends SyscallException {
    static get _ice_id() {
        return "::Ice::SocketException";
    }
}

export class ConnectFailedException extends SocketException {
    static get _ice_id() {
        return "::Ice::ConnectFailedException";
    }
}

export class ConnectionLostException extends SocketException {
    static get _ice_id() {
        return "::Ice::ConnectionLostException";
    }
}

export class ConnectionRefusedException extends ConnectFailedException {
    static get _ice_id() {
        return "::Ice::ConnectionRefusedException";
    }
}

//
// Other leaf local exceptions in alphabetical order.
//

export class AlreadyRegisteredException extends LocalException {
    static get _ice_id() {
        return "::Ice::AlreadyRegisteredException";
    }

    constructor(kindOfObject, id) {
        super(`Another ${kindOfObject} is already registered with ID '${id}'.`);
        this._kindOfObject = kindOfObject;
        this._id = id;
    }

    get kindOfObject() {
        return this._kindOfObject;
    }

    get id() {
        return this._id;
    }
}

export class CommunicatorDestroyedException extends LocalException {
    static get _ice_id() {
        return "::Ice::CommunicatorDestroyedException";
    }

    constructor() {
        super("Communicator destroyed.");
    }
}

export class ConnectionAbortedException extends LocalException {
    static get _ice_id() {
        return "::Ice::ConnectionAbortedException";
    }

    constructor(message, closedByApplication) {
        super(message);
        this.closedByApplication = closedByApplication;
    }
}

export class ConnectionClosedException extends LocalException {
    static get _ice_id() {
        return "::Ice::ConnectionClosedException";
    }

    constructor(message, closedByApplication) {
        super(message);
        this.closedByApplication = closedByApplication;
    }
}

export class FeatureNotSupportedException extends LocalException {
    static get _ice_id() {
        return "::Ice::FeatureNotSupportedException";
    }

    constructor(message) {
        super(message);
    }
}

export class FixedProxyException extends LocalException {
    static get _ice_id() {
        return "::Ice::FixedProxyException";
    }

    constructor() {
        super("Cannot change the connection properties of a fixed proxy.");
    }
}

export class InitializationException extends LocalException {
    static get _ice_id() {
        return "::Ice::InitializationException";
    }
}

export class InvocationCanceledException extends LocalException {
    static get _ice_id() {
        return "::Ice::InvocationCanceledException";
    }

    constructor() {
        super("Invocation canceled.");
    }
}

export class NoEndpointException extends LocalException {
    static get _ice_id() {
        return "::Ice::NoEndpointException";
    }

    constructor(messageOrProxy) {
        super(NoEndpointException.createMessage(messageOrProxy));
    }

    static createMessage(arg) {
        return arg instanceof ObjectPrx ? `$"No suitable endpoint available for proxy '${arg}'.` : arg;
    }
}

export class NotRegisteredException extends LocalException {
    static get _ice_id() {
        return "::Ice::NotRegisteredException";
    }

    constructor(kindOfObject, id) {
        super(`No ${kindOfObject} is registered with ID '${id}'.`);
        this._kindOfObject = kindOfObject;
        this._id = id;
    }

    get kindOfObject() {
        return this._kindOfObject;
    }

    get id() {
        return this._id;
    }
}

export class ObjectAdapterDestroyedException extends LocalException {
    static get _ice_id() {
        return "::Ice::ObjectAdapterDestroyedException";
    }

    constructor(name) {
        super(`Object adapter '${name}' is destroyed.`);
    }
}

export class ParseException extends LocalException {
    static get _ice_id() {
        return "::Ice::ParseException";
    }
}

export class TwowayOnlyException extends LocalException {
    static get _ice_id() {
        return "::Ice::TwowayOnlyException";
    }

    constructor(operation) {
        super(`Cannot invoke operation '${operation}' with a oneway, batchOneway, datagram, or batchDatagram proxy.`);
        this.operation = operation;
    }
}

export class PropertyException extends LocalException {
    static get _ice_id() {
        return "::Ice::PropertyException";
    }

    constructor(message) {
        super(message);
    }
}
