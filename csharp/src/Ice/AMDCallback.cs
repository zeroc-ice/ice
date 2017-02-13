// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    /// <summary>
    /// AMDCallback is the interface from which all AMD callbacks are derived.
    /// </summary>
    public interface AMDCallback
    {
        /// <summary>
        /// Indicates to the Ice run time that an operation completed
        /// with a run-time exception.
        /// </summary>
        /// <param name="ex">The encoded Ice run-time exception. Note that, if ex
        /// is a user exception, the caller receives UnknownUserException.
        /// Use ice_response to raise user exceptions.</param>
        void ice_exception(System.Exception ex);
    }
}
