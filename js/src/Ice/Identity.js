// Copyright (c) ZeroC, Inc.

export const Ice = {};

/**
 *  The identity of an Ice object. In a proxy, an empty {@link Identity#name} denotes a nil proxy. An identity with
 *  an empty {@link Identity#name} and a non-empty {@link Identity#category} is illegal. You cannot add a servant
 *  with an empty name to the Active Servant Map.
 *  @see ServantLocator
 *  @see ObjectAdapter#addServantLocator
 **/
Ice.Identity = class {
    constructor(name = "", category = "") {
        this.name = name;
        this.category = category;
    }

    _write(ostr) {
        ostr.writeString(this.name);
        ostr.writeString(this.category);
    }

    _read(istr) {
        this.name = istr.readString();
        this.category = istr.readString();
    }

    static get minWireSize() {
        return 2;
    }
};

// Moved to IdentityExtensions.js to avoid circular dependencies.
// Ice.defineStruct(Ice.Identity, true, true);
// Ice.IdentitySeqHelper = Ice.StreamHelpers.generateSeqHelper(Ice.Identity, false);
