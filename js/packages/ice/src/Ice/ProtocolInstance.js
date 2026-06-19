// Copyright (c) ZeroC, Inc.

export class ProtocolInstance {
    constructor(instance, type, protocol, secure) {
        this._instance = instance;
        this._logger = instance.initializationData().logger;
        this._properties = instance.initializationData().properties;
        this._type = type;
        this._protocol = protocol;
        this._secure = secure;
    }

    logger() {
        return this._logger;
    }

    protocol() {
        return this._protocol;
    }

    type() {
        return this._type;
    }

    secure() {
        return this._secure;
    }

    properties() {
        return this._properties;
    }

    defaultHost() {
        return this._instance.defaultsAndOverrides().defaultHost;
    }

    defaultSourceAddress() {
        return this._instance.defaultsAndOverrides().defaultSourceAddress;
    }

    messageSizeMax() {
        return this._instance.messageSizeMax();
    }
}
