// Copyright (c) ZeroC, Inc.
    
module and
{
    enum assert
    {
        break
    }
        
    struct continue
    {
        int def;
    }
        
    interface del
    {
        ["amd"] void elif(int else, out int except);
    }
        
    interface exec
    {
        void finally();
    }
        
    class for
    {
        int lambda;
        exec* from;
        int global;
    }
        
    interface if extends exec, del
    {
    }
        
    sequence<assert> import;
    dictionary<string,assert> in;
        
    exception is
    {
        int lambda;
    }
        
    exception not extends is
    {
        int or;
        int pass;
    }
        
    interface print
    {
        assert raise(continue else, for return, del* while, exec* yield, if* or, int global) throws is;
    }
        
    const int lambda = 0;
        
    enum EnumNone
    {
        None
    }
}
