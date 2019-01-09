// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package Ice;

/**
 * Applications implement this interface to provide a plug-in factory
 * to the Ice run time.
 **/
public interface PluginFactory
{
    /**
     * Called by the Ice run time to create a new plug-in.
     *
     * @param communicator The communicator that is in the process of being initialized.
     * @param name The name of the plug-in.
     * @param args The arguments that are specified in the plug-ins configuration.
     * @return The plug-in that was created by this method.
     **/
    Plugin create(Communicator communicator, String name, String[] args);
}
