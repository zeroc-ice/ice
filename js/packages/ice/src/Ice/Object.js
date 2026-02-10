// Copyright (c) ZeroC, Inc.

export class Object {
    ice_isA(s) {
        return this._iceMostDerivedType()._iceIds.indexOf(s) >= 0;
    }

    ice_ping() {}

    ice_ids() {
        return this._iceMostDerivedType()._iceIds;
    }

    ice_id() {
        return this._iceMostDerivedType()._iceId;
    }

    toString() {
        return "[object " + this.ice_id() + "]";
    }

    //
    // _iceMostDerivedType returns the most derived Ice generated class. This is
    // necessary because the user might extend Slice generated classes. The user
    // class extensions don't have _iceId, _iceIds, etc static members so the implementation
    // of ice_id and ice_ids would fail trying to access those members of the user
    // defined class. Instead, ice_id, ice_ids and ice_instanceof call _iceMostDerivedType
    // to get the most derived Ice class.
    //
    _iceMostDerivedType() {
        return Object;
    }

    //
    // The default implementation of equals compare references.
    //
    equals(other) {
        return this === other;
    }

    static get _iceImplements() {
        return [];
    }
}
