// Copyright (c) ZeroC, Inc.
    
#pragma once
    
module Test
{
    exception CallbackException
    {
        double someValue;
        string someString;
    }
<<<<<<< Updated upstream

    interface CallbackReceiver
    {
        void callback();

        void callbackEx()
        throws CallbackException;
    }

    interface Callback
    {
        void initiateCallback(CallbackReceiver* proxy);

        void initiateCallbackEx(CallbackReceiver* proxy)
        throws CallbackException;

=======
        
    interface CallbackReceiver
    {
        void callback();
            
        void callbackEx()
            throws CallbackException;
    }
        
    interface Callback
    {
        void initiateCallback(CallbackReceiver* proxy);
            
        void initiateCallbackEx(CallbackReceiver* proxy)
            throws CallbackException;
            
>>>>>>> Stashed changes
        void shutdown();
    }
}
