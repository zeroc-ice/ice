namespace Ice
{

public abstract class Exception : System.ApplicationException
{
    public Exception() {}
    public Exception(string msg) : base(msg) {}
    public Exception(System.Exception ex) : base(_dflt, ex) {}
    public Exception(string msg, System.Exception ex) : base(msg, ex) {}
    private static readonly string _dflt = "Ice.Exception";
}

public abstract class LocalException : Exception
{
    public LocalException() {}
    public LocalException(string msg) : base(msg) {}
    public LocalException(System.Exception ex) : base(_dflt, ex) {}
    public LocalException(string msg, System.Exception ex) : base(msg, ex) {}
    private static readonly string _dflt = "Ice.LocalException";
}

    public abstract class UserException : Exception
    {
	public UserException() {}
	public UserException(string msg) : base(msg) {}
	public UserException(System.Exception ex) : base(_dflt, ex) {}
	public UserException(string msg, System.Exception ex) : base(msg, ex) {}
	private static readonly string _dflt = "Ice.UserException";
	public abstract void __write(IceInternal.BasicStream __os);
	public abstract void __read(IceInternal.BasicStream __is, bool __rid);
	public abstract bool __usesClasses();
}

}
