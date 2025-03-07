// Copyright (c) ZeroC, Inc.
    
#pragma once
    
["cs:namespace:Ice.invoke"]
module Test
{
    exception MyException
    {
    }
<<<<<<< Updated upstream

    interface MyClass
    {
        void opOneway();

        string opString(string s1, out string s2);

        void opException() throws MyException;

=======
        
    interface MyClass
    {
        void opOneway();
            
        string opString(string s1, out string s2);
            
        void opException() throws MyException;
            
>>>>>>> Stashed changes
        void shutdown();
    }
}
