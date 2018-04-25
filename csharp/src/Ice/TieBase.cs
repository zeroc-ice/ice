// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    /// <summary>
    /// Interface for servants using the tie mapping.
    /// </summary>
    public interface TieBase
    {
        /// <summary>
        /// Returns the delegate for this tie.
        /// </summary>
        /// <returns>The delegate.</returns>
        object ice_delegate();

        /// <summary>
        /// Returns the delegate for this tie.
        /// </summary>
        /// <param name="o">The delegate.</param>
        void ice_delegate(object o);
    }

}
