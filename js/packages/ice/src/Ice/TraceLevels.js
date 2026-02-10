// Copyright (c) ZeroC, Inc.

export function TraceLevels(properties) {
    const dispatch = properties.getIcePropertyAsInt("Ice.Trace.Dispatch");
    const network = properties.getIcePropertyAsInt("Ice.Trace.Network");
    const protocol = properties.getIcePropertyAsInt("Ice.Trace.Protocol");
    const retry = properties.getIcePropertyAsInt("Ice.Trace.Retry");
    const location = properties.getIcePropertyAsInt("Ice.Trace.Locator");
    const slicing = properties.getIcePropertyAsInt("Ice.Trace.Slicing");

    return class {
        static get dispatch() {
            return dispatch;
        }

        static get dispatchCat() {
            return "Dispatch";
        }

        static get network() {
            return network;
        }

        static get networkCat() {
            return "Network";
        }

        static get protocol() {
            return protocol;
        }

        static get protocolCat() {
            return "Protocol";
        }

        static get retry() {
            return retry;
        }

        static get retryCat() {
            return "Retry";
        }

        static get location() {
            return location;
        }

        static get locationCat() {
            return "Locator";
        }

        static get slicing() {
            return slicing;
        }

        static get slicingCat() {
            return "Slicing";
        }
    };
}
