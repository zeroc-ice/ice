//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using Ice.servantLocator.AMD;

namespace Ice
{
    namespace servantLocator
    {
        namespace AMD
        {
            public sealed class ServantLocatorI : Ice.ServantLocator
            {
                public ServantLocatorI(String category)
                {
                    _category = category;
                    _deactivated = false;
                    _requestId = -1;
                }

                ~ServantLocatorI()
                {
                    lock (this)
                    {
                        test(_deactivated);
                    }
                }

                private static void test(bool b)
                {
                    if (!b)
                    {
                        throw new System.Exception();
                    }
                }

                public Disp locate(Current current, out object cookie)
                {
                    lock (this)
                    {
                        test(!_deactivated);
                    }

                    test(current.Id.category.Equals(_category) || _category.Length == 0);

                    if (current.Id.name.Equals("unknown"))
                    {
                        cookie = null;
                        return null;
                    }

                    if (current.Id.name.Equals("invalidReturnValue") || current.Id.name.Equals("invalidReturnType"))
                    {
                        cookie = null;
                        return null;
                    }

                    test(current.Id.name.Equals("locate") || current.Id.name.Equals("finished"));
                    if (current.Id.name.Equals("locate"))
                    {
                        exception(current);
                    }

                    //
                    // Ensure locate() is only called once per request.
                    //
                    test(_requestId == -1);
                    _requestId = current.RequestId;

                    cookie = new Cookie();

                    TestI testI = new TestI();
                    Test.TestIntfTraits testT = default;
                    return (current, incoming) => testT.Dispatch(testI, current, incoming);
                }

                public void finished(Current current, Disp servant, object cookie)
                {
                    lock (this)
                    {
                        test(!_deactivated);
                    }

                    //
                    // Ensure finished() is only called once per request.
                    //
                    test(_requestId == current.RequestId);
                    _requestId = -1;

                    test(current.Id.category.Equals(_category) || _category.Length == 0);
                    test(current.Id.name.Equals("locate") || current.Id.name.Equals("finished"));

                    if (current.Id.name.Equals("finished"))
                    {
                        exception(current);
                    }

                    var co = (Cookie)cookie;
                    test(co.message().Equals("blahblah"));
                }

                public void deactivate(string category)
                {
                    lock (this)
                    {
                        test(!_deactivated);

                        _deactivated = true;
                    }
                }

                private void exception(Ice.Current current)
                {
                    if (current.Operation.Equals("ice_ids"))
                    {
                        throw new Test.TestIntfUserException();
                    }
                    else if (current.Operation.Equals("requestFailedException"))
                    {
                        throw new Ice.ObjectNotExistException();
                    }
                    else if (current.Operation.Equals("unknownUserException"))
                    {
                        var ex = new Ice.UnknownUserException();
                        ex.unknown = "reason";
                        throw ex;
                    }
                    else if (current.Operation.Equals("unknownLocalException"))
                    {
                        var ex = new Ice.UnknownLocalException();
                        ex.unknown = "reason";
                        throw ex;
                    }
                    else if (current.Operation.Equals("unknownException"))
                    {
                        var ex = new Ice.UnknownException();
                        ex.unknown = "reason";
                        throw ex;
                    }
                    else if (current.Operation.Equals("userException"))
                    {
                        throw new Test.TestIntfUserException();
                    }
                    else if (current.Operation.Equals("localException"))
                    {
                        var ex = new Ice.SocketException();
                        ex.error = 0;
                        throw ex;
                    }
                    else if (current.Operation.Equals("csException"))
                    {
                        throw new System.Exception("message");
                    }
                    else if (current.Operation.Equals("unknownExceptionWithServantException"))
                    {
                        throw new Ice.UnknownException("reason");
                    }
                    else if (current.Operation.Equals("impossibleException"))
                    {
                        throw new Test.TestIntfUserException(); // Yes, it really is meant to be TestIntfException.
                    }
                    else if (current.Operation.Equals("intfUserException"))
                    {
                        throw new Test.TestImpossibleException(); // Yes, it really is meant to be TestImpossibleException.
                    }
                    else if (current.Operation.Equals("asyncResponse"))
                    {
                        throw new Test.TestImpossibleException();
                    }
                    else if (current.Operation.Equals("asyncException"))
                    {
                        throw new Test.TestImpossibleException();
                    }
                }

                private bool _deactivated;
                private string _category;
                private int _requestId;
            }
        }
    }
}
