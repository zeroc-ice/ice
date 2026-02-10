// Copyright (c) ZeroC, Inc.

import { Ice as Ice_Identity } from "./Identity.js";
const { Identity } = Ice_Identity;
import { MarshalException } from "./LocalExceptions.js";

import { Ice as Ice_OperationMode } from "./OperationMode.js";
const { OperationMode } = Ice_OperationMode;
import { Ice as Ice_Context } from "./Context.js";
const { Context } = Ice_Context;
import { Ice as Ice_Version } from "./Version.js";
const { EncodingVersion } = Ice_Version;
import { Current } from "./Current.js";
import { Ice as Ice_BuiltinSequences } from "./BuiltinSequences.js";
const { StringSeqHelper } = Ice_BuiltinSequences;

export class IncomingRequest {
    get current() {
        return this._current;
    }
    get inputStream() {
        return this._inputStream;
    }
    get size() {
        return this._size;
    }

    constructor(requestId, connection, adapter, inputStream) {
        this._inputStream = inputStream;

        // Read everything else from the input stream
        const start = inputStream.pos;
        const identity = new Identity();
        identity._read(inputStream);

        let facet = "";
        const facetPath = StringSeqHelper.read(inputStream);
        if (facetPath.length > 0) {
            if (facetPath.length > 1) {
                throw new MarshalException(`Received invalid facet path with ${facetPath.length} elements.`);
            }
            facet = facetPath[0];
        }
        const operation = inputStream.readString();
        const mode = OperationMode.valueOf(inputStream.readByte());
        const ctx = new Context();
        let sz = inputStream.readSize();
        while (sz-- > 0) {
            ctx.set(inputStream.readString(), inputStream.readString());
        }

        const encapsulationSize = inputStream.readInt();
        const encoding = new EncodingVersion();
        encoding._read(inputStream);

        this._current = new Current(adapter, connection, identity, facet, operation, mode, ctx, requestId, encoding);

        // Rewind to the start of the encapsulation
        inputStream.pos = inputStream.pos - 6;

        this._size = inputStream.pos - start + encapsulationSize;
    }
}
