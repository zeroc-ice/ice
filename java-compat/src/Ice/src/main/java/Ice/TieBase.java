// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Interface for servants using the tie mapping.
 **/
public interface TieBase
{
    /**
     * Returns the delegate for this tie.
     *
     * @return The delegate.
     **/
    java.lang.Object ice_delegate();

    /**
     * Sets the delegate for this tie.
     *
     * @param delegate The delegate.
     **/
    void ice_delegate(java.lang.Object delegate);
}
