// Copyright (c) ZeroC, Inc.

export class InitializationData {
    constructor() {
        this.properties = null;
        this.logger = null;
        this.sliceLoader = null;
    }

    clone() {
        const r = new InitializationData();
        r.properties = this.properties;
        r.logger = this.logger;
        r.sliceLoader = this.sliceLoader;
        return r;
    }
}
