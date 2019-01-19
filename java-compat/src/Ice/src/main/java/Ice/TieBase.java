//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
