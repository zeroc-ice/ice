//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    /// <summary>The reply status of an ice1 response frame.</summary>
    public enum ReplyStatus : byte
    {
        /// <summary>A successful reply message.</summary>
        OK = 0,
        /// <summary>A user exception reply message.</summary>
        UserException = 1,
        /// <summary>The target object does not exist.</summary>
        ObjectNotExistException = 2,
        /// <summary>The target object does not support the facet.</summary>
        FacetNotExistException = 3,
        /// <summary>The target object does not support the operation.</summary>
        OperationNotExistException = 4,
        /// <summary>The reply message carries an unknown Ice local exception.</summary>
        UnknownLocalException = 5,
        /// <summary>The reply message carries an unknown Ice user exception.</summary>
        UnknownUserException = 6,
        /// <summary>The reply message carries an unknown exception.</summary>
        UnknownException = 7
    }
}
