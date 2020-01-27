//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.servantLocator.AMD
{
    public sealed class ServantLocator : IServantLocator
    {
        public ServantLocator(string category)
        {
            _category = category;
            _deactivated = false;
            _requestId = -1;
        }

        ~ServantLocator()
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

        public Disp Locate(Current current, out object cookie)
        {
            lock (this)
            {
                test(!_deactivated);
            }

            test(current.Id.Category.Equals(_category) || _category.Length == 0);

            if (current.Id.Name.Equals("unknown"))
            {
                cookie = null;
                return null;
            }

            if (current.Id.Name.Equals("invalidReturnValue") || current.Id.Name.Equals("invalidReturnType"))
            {
                cookie = null;
                return null;
            }

            test(current.Id.Name.Equals("locate") || current.Id.Name.Equals("finished"));
            if (current.Id.Name.Equals("locate"))
            {
                Exception(current);
            }

            //
            // Ensure locate() is only called once per request.
            //
            test(_requestId == -1);
            _requestId = current.RequestId;

            cookie = new Cookie();

            var testI = new TestIntf();
            return (current, incoming) => Test.ITestIntf.Dispatch(testI, current, incoming);
        }

        public void Finished(Current current, Disp servant, object cookie)
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

            test(current.Id.Category.Equals(_category) || _category.Length == 0);
            test(current.Id.Name.Equals("locate") || current.Id.Name.Equals("finished"));

            if (current.Id.Name.Equals("finished"))
            {
                Exception(current);
            }

            var co = (Cookie)cookie;
            test(co.message().Equals("blahblah"));
        }

        public void Deactivate(string category)
        {
            lock (this)
            {
                test(!_deactivated);

                _deactivated = true;
            }
        }

        private void Exception(Current current)
        {
            if (current.Operation.Equals("ice_ids"))
            {
                throw new Test.TestIntfUserException();
            }
            else if (current.Operation.Equals("requestFailedException"))
            {
                throw new ObjectNotExistException();
            }
            else if (current.Operation.Equals("unknownUserException"))
            {
                var ex = new UnknownUserException();
                ex.Unknown = "reason";
                throw ex;
            }
            else if (current.Operation.Equals("unknownLocalException"))
            {
                var ex = new UnknownLocalException();
                ex.Unknown = "reason";
                throw ex;
            }
            else if (current.Operation.Equals("unknownException"))
            {
                var ex = new UnknownException();
                ex.Unknown = "reason";
                throw ex;
            }
            else if (current.Operation.Equals("userException"))
            {
                throw new Test.TestIntfUserException();
            }
            else if (current.Operation.Equals("localException"))
            {
                var ex = new Ice.SocketException();
                ex.Error = 0;
                throw ex;
            }
            else if (current.Operation.Equals("csException"))
            {
                throw new System.Exception("message");
            }
            else if (current.Operation.Equals("unknownExceptionWithServantException"))
            {
                throw new UnknownException("reason");
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
