// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

namespace Ice
{
    namespace servantLocator
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
                lock(this)
                {
                    test(_deactivated);
                }
            }

            private static void test(bool b)
            {
                if(!b)
                {
                    throw new System.Exception();
                }
            }

            public Ice.Object locate(Ice.Current current, out object cookie)
            {
                lock(this)
                {
                    test(!_deactivated);
                }

                test(current.id.category.Equals(_category) || _category.Length == 0);

                if(current.id.name.Equals("unknown"))
                {
                    cookie = null;
                    return null;
                }

                test(current.id.name.Equals("locate") || current.id.name.Equals("finished"));
                if(current.id.name.Equals("locate"))
                {
                    exception(current);
                }

                //
                // Ensure locate() is only called once per request.
                //
                test(_requestId == -1);
                _requestId = current.requestId;

                cookie = new CookieI();

                return new TestI();
            }

            public void finished(Ice.Current current, Ice.Object servant, System.Object cookie)
            {
                lock(this)
                {
                    test(!_deactivated);
                }

                //
                // Ensure finished() is only called once per request.
                //
                test(_requestId == current.requestId);
                _requestId = -1;

                test(current.id.category.Equals(_category) || _category.Length == 0);
                test(current.id.name.Equals("locate") || current.id.name.Equals("finished"));

                if(current.id.name.Equals("finished"))
                {
                    exception(current);
                }

                var co =(Test.Cookie)cookie;
                test(co.message().Equals("blahblah"));
            }

            public void deactivate(string category)
            {
                lock(this)
                {
                    test(!_deactivated);

                    _deactivated = true;
                }
            }

            private void exception(Ice.Current current)
            {
                if(current.operation.Equals("ice_ids"))
                {
                    throw new Test.TestIntfUserException();
                }
                else if(current.operation.Equals("requestFailedException"))
                {
                    throw new Ice.ObjectNotExistException();
                }
                else if(current.operation.Equals("unknownUserException"))
                {
                    var ex = new Ice.UnknownUserException();
                    ex.unknown = "reason";
                    throw ex;
                }
                else if(current.operation.Equals("unknownLocalException"))
                {
                    var ex = new Ice.UnknownLocalException();
                    ex.unknown = "reason";
                    throw ex;
                }
                else if(current.operation.Equals("unknownException"))
                {
                    var ex = new Ice.UnknownException();
                    ex.unknown = "reason";
                    throw ex;
                }
                else if(current.operation.Equals("userException"))
                {
                    throw new Test.TestIntfUserException();
                }
                else if(current.operation.Equals("localException"))
                {
                    var ex = new Ice.SocketException();
                    ex.error = 0;
                    throw ex;
                }
                else if(current.operation.Equals("csException"))
                {
                    throw new System.Exception("message");
                }
                else if(current.operation.Equals("unknownExceptionWithServantException"))
                {
                    throw new Ice.UnknownException("reason");
                }
                else if(current.operation.Equals("impossibleException"))
                {
                    throw new Test.TestIntfUserException(); // Yes, it really is meant to be TestIntfException.
                }
                else if(current.operation.Equals("intfUserException"))
                {
                    throw new Test.TestImpossibleException(); // Yes, it really is meant to be TestImpossibleException.
                }
                else if(current.operation.Equals("asyncResponse"))
                {
                    throw new Test.TestImpossibleException();
                }
                else if(current.operation.Equals("asyncException"))
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
