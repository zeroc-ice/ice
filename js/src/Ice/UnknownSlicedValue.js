// Copyright (c) ZeroC, Inc.

import { Value } from "./Value.js";

export class SliceInfo {
    constructor() {
        //
        // The Slice type ID for this slice.
        //
        this.typeId = "";

        //
        // The Slice compact type ID for this slice.
        //
        this.compactId = -1;

        //
        // The encoded bytes for this slice, including the leading size integer.
        //
        this.bytes = [];

        //
        // The class instances referenced by this slice.
        //
        this.instances = [];

        //
        // Whether or not the slice contains optional members.
        //
        this.hasOptionalMembers = false;

        //
        // Whether or not this is the last slice.
        //
        this.isLastSlice = false;
    }
}

export class SlicedData {
    constructor(slices) {
        this.slices = slices;
    }
}

export class UnknownSlicedValue extends Value {
    constructor(unknownTypeId) {
        super();
        this._unknownTypeId = unknownTypeId;
    }

    ice_id() {
        return this._unknownTypeId;
    }

    _iceWrite(os) {
        os.startValue(this._iceSlicedData);
        os.endValue();
    }

    _iceRead(is) {
        is.startValue();
        this._iceSlicedData = is.endValue();
    }
}
