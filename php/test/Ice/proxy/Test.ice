// Copyright (c) ZeroC, Inc.
    
#pragma once
    
module Test
{
    dictionary<string, string> Context;
        
    interface MyClass
    {
        void shutdown();
            
        Context getContext();
    }
        
    interface MyDerivedClass extends MyClass
    {
        Object* echo(Object* obj);
    }
}
