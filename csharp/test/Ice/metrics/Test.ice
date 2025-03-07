// Copyright (c) ZeroC, Inc.
    
#pragma once
    
module Test
{
    exception UserEx
    {
    }
<<<<<<< Updated upstream

    sequence<byte> ByteSeq;

    interface Metrics
    {
        void op();

        idempotent void fail();

        void opWithUserException()
        throws UserEx;

        void opWithRequestFailedException();

        void opWithLocalException();

        void opWithUnknownException();

        void opByteS(ByteSeq bs);

        Object* getAdmin();

        void shutdown();
    }

    interface Controller
    {
        void hold();

=======
        
    sequence<byte> ByteSeq;
        
    interface Metrics
    {
        void op();
            
        idempotent void fail();
            
        void opWithUserException()
            throws UserEx;
            
        void opWithRequestFailedException();
            
        void opWithLocalException();
            
        void opWithUnknownException();
            
        void opByteS(ByteSeq bs);
            
        Object* getAdmin();
            
        void shutdown();
    }
        
    interface Controller
    {
        void hold();
            
>>>>>>> Stashed changes
        void resume();
    }
}
