//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

export function TraceLevels(properties) {
    const network = properties.getPropertyAsInt("Ice.Trace.Network");
    const protocol = properties.getPropertyAsInt("Ice.Trace.Protocol");
    const retry = properties.getPropertyAsInt("Ice.Trace.Retry");
    const location = properties.getPropertyAsInt("Ice.Trace.Locator");
    const slicing = properties.getPropertyAsInt("Ice.Trace.Slicing");

    return class {
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
