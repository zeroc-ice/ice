// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 3.3.1
// Generated from file `Employees.ice'

#if __MonoCS__

using _System = System;
using _Microsoft = Microsoft;
#else

using _System = global::System;
using _Microsoft = global::Microsoft;
#endif

namespace Employees
{
    public class EmployeeDetails : _System.ICloneable
    {
        #region Slice data members

        public string name;

        public string address;

        #endregion

        #region Constructors

        public EmployeeDetails()
        {
        }

        public EmployeeDetails(string name, string address)
        {
            this.name = name;
            this.address = address;
        }

        #endregion

        #region ICloneable members

        public object Clone()
        {
            return MemberwiseClone();
        }

        #endregion

        #region Object members

        public override int GetHashCode()
        {
            int h__ = 0;
            if(name != null)
            {
                h__ = 5 * h__ + name.GetHashCode();
            }
            if(address != null)
            {
                h__ = 5 * h__ + address.GetHashCode();
            }
            return h__;
        }

        public override bool Equals(object other__)
        {
            if(object.ReferenceEquals(this, other__))
            {
                return true;
            }
            if(other__ == null)
            {
                return false;
            }
            if(GetType() != other__.GetType())
            {
                return false;
            }
            EmployeeDetails o__ = (EmployeeDetails)other__;
            if(name == null)
            {
                if(o__.name != null)
                {
                    return false;
                }
            }
            else
            {
                if(!name.Equals(o__.name))
                {
                    return false;
                }
            }
            if(address == null)
            {
                if(o__.address != null)
                {
                    return false;
                }
            }
            else
            {
                if(!address.Equals(o__.address))
                {
                    return false;
                }
            }
            return true;
        }

        #endregion

        #region Comparison members

        public static bool operator==(EmployeeDetails lhs__, EmployeeDetails rhs__)
        {
            return Equals(lhs__, rhs__);
        }

        public static bool operator!=(EmployeeDetails lhs__, EmployeeDetails rhs__)
        {
            return !Equals(lhs__, rhs__);
        }

        #endregion

        #region Marshalling support

        public void write__(IceInternal.BasicStream os__)
        {
            os__.writeString(name);
            os__.writeString(address);
        }

        public void read__(IceInternal.BasicStream is__)
        {
            name = is__.readString();
            address = is__.readString();
        }

        #endregion
    }

    public interface Employee : Ice.Object, EmployeeOperations_, EmployeeOperationsNC_
    {
    }

    public interface EmployeeFinder : Ice.Object, EmployeeFinderOperations_, EmployeeFinderOperationsNC_
    {
    }
}

namespace Employees
{
    public interface EmployeePrx : Ice.ObjectPrx
    {
        Employees.EmployeeDetails getDetails();
        Employees.EmployeeDetails getDetails(_System.Collections.Generic.Dictionary<string, string> context__);

        void setAnnualSalary(double salary);
        void setAnnualSalary(double salary, _System.Collections.Generic.Dictionary<string, string> context__);

        void setTaxPayable(double amount);
        void setTaxPayable(double amount, _System.Collections.Generic.Dictionary<string, string> context__);
    }

    public interface EmployeeFinderPrx : Ice.ObjectPrx
    {
        _System.Collections.Generic.List<Employees.EmployeePrx> list();
        _System.Collections.Generic.List<Employees.EmployeePrx> list(_System.Collections.Generic.Dictionary<string, string> context__);
    }
}

namespace Employees
{
    public interface EmployeeOperations_
    {
        Employees.EmployeeDetails getDetails(Ice.Current current__);

        void setAnnualSalary(double salary, Ice.Current current__);

        void setTaxPayable(double amount, Ice.Current current__);
    }

    public interface EmployeeOperationsNC_
    {
        Employees.EmployeeDetails getDetails();

        void setAnnualSalary(double salary);

        void setTaxPayable(double amount);
    }

    public interface EmployeeFinderOperations_
    {
        _System.Collections.Generic.List<Employees.EmployeePrx> list(Ice.Current current__);
    }

    public interface EmployeeFinderOperationsNC_
    {
        _System.Collections.Generic.List<Employees.EmployeePrx> list();
    }
}

namespace Employees
{
    public sealed class EmployeePrxHelper : Ice.ObjectPrxHelperBase, EmployeePrx
    {
        #region Synchronous operations

        public Employees.EmployeeDetails getDetails()
        {
            return getDetails(null, false);
        }

        public Employees.EmployeeDetails getDetails(_System.Collections.Generic.Dictionary<string, string> context__)
        {
            return getDetails(context__, true);
        }

        private Employees.EmployeeDetails getDetails(_System.Collections.Generic.Dictionary<string, string> context__, bool explicitContext__)
        {
            if(explicitContext__ && context__ == null)
            {
                context__ = emptyContext_;
            }
            int cnt__ = 0;
            while(true)
            {
                Ice.ObjectDel_ delBase__ = null;
                try
                {
                    checkTwowayOnly__("getDetails");
                    delBase__ = getDelegate__(false);
                    EmployeeDel_ del__ = (EmployeeDel_)delBase__;
                    return del__.getDetails(context__);
                }
                catch(IceInternal.LocalExceptionWrapper ex__)
                {
                    handleExceptionWrapper__(delBase__, ex__, null);
                }
                catch(Ice.LocalException ex__)
                {
                    handleException__(delBase__, ex__, null, ref cnt__);
                }
            }
        }

        public void setAnnualSalary(double salary)
        {
            setAnnualSalary(salary, null, false);
        }

        public void setAnnualSalary(double salary, _System.Collections.Generic.Dictionary<string, string> context__)
        {
            setAnnualSalary(salary, context__, true);
        }

        private void setAnnualSalary(double salary, _System.Collections.Generic.Dictionary<string, string> context__, bool explicitContext__)
        {
            if(explicitContext__ && context__ == null)
            {
                context__ = emptyContext_;
            }
            int cnt__ = 0;
            while(true)
            {
                Ice.ObjectDel_ delBase__ = null;
                try
                {
                    delBase__ = getDelegate__(false);
                    EmployeeDel_ del__ = (EmployeeDel_)delBase__;
                    del__.setAnnualSalary(salary, context__);
                    return;
                }
                catch(IceInternal.LocalExceptionWrapper ex__)
                {
                    handleExceptionWrapper__(delBase__, ex__, null);
                }
                catch(Ice.LocalException ex__)
                {
                    handleException__(delBase__, ex__, null, ref cnt__);
                }
            }
        }

        public void setTaxPayable(double amount)
        {
            setTaxPayable(amount, null, false);
        }

        public void setTaxPayable(double amount, _System.Collections.Generic.Dictionary<string, string> context__)
        {
            setTaxPayable(amount, context__, true);
        }

        private void setTaxPayable(double amount, _System.Collections.Generic.Dictionary<string, string> context__, bool explicitContext__)
        {
            if(explicitContext__ && context__ == null)
            {
                context__ = emptyContext_;
            }
            int cnt__ = 0;
            while(true)
            {
                Ice.ObjectDel_ delBase__ = null;
                try
                {
                    delBase__ = getDelegate__(false);
                    EmployeeDel_ del__ = (EmployeeDel_)delBase__;
                    del__.setTaxPayable(amount, context__);
                    return;
                }
                catch(IceInternal.LocalExceptionWrapper ex__)
                {
                    handleExceptionWrapper__(delBase__, ex__, null);
                }
                catch(Ice.LocalException ex__)
                {
                    handleException__(delBase__, ex__, null, ref cnt__);
                }
            }
        }

        #endregion

        #region Checked and unchecked cast operations

        public static EmployeePrx checkedCast(Ice.ObjectPrx b)
        {
            if(b == null)
            {
                return null;
            }
            EmployeePrx r = b as EmployeePrx;
            if((r == null) && b.ice_isA("::Employees::Employee"))
            {
                EmployeePrxHelper h = new EmployeePrxHelper();
                h.copyFrom__(b);
                r = h;
            }
            return r;
        }

        public static EmployeePrx checkedCast(Ice.ObjectPrx b, _System.Collections.Generic.Dictionary<string, string> ctx)
        {
            if(b == null)
            {
                return null;
            }
            EmployeePrx r = b as EmployeePrx;
            if((r == null) && b.ice_isA("::Employees::Employee", ctx))
            {
                EmployeePrxHelper h = new EmployeePrxHelper();
                h.copyFrom__(b);
                r = h;
            }
            return r;
        }

        public static EmployeePrx checkedCast(Ice.ObjectPrx b, string f)
        {
            if(b == null)
            {
                return null;
            }
            Ice.ObjectPrx bb = b.ice_facet(f);
            try
            {
                if(bb.ice_isA("::Employees::Employee"))
                {
                    EmployeePrxHelper h = new EmployeePrxHelper();
                    h.copyFrom__(bb);
                    return h;
                }
            }
            catch(Ice.FacetNotExistException)
            {
            }
            return null;
        }

        public static EmployeePrx checkedCast(Ice.ObjectPrx b, string f, _System.Collections.Generic.Dictionary<string, string> ctx)
        {
            if(b == null)
            {
                return null;
            }
            Ice.ObjectPrx bb = b.ice_facet(f);
            try
            {
                if(bb.ice_isA("::Employees::Employee", ctx))
                {
                    EmployeePrxHelper h = new EmployeePrxHelper();
                    h.copyFrom__(bb);
                    return h;
                }
            }
            catch(Ice.FacetNotExistException)
            {
            }
            return null;
        }

        public static EmployeePrx uncheckedCast(Ice.ObjectPrx b)
        {
            if(b == null)
            {
                return null;
            }
            EmployeePrx r = b as EmployeePrx;
            if(r == null)
            {
                EmployeePrxHelper h = new EmployeePrxHelper();
                h.copyFrom__(b);
                r = h;
            }
            return r;
        }

        public static EmployeePrx uncheckedCast(Ice.ObjectPrx b, string f)
        {
            if(b == null)
            {
                return null;
            }
            Ice.ObjectPrx bb = b.ice_facet(f);
            EmployeePrxHelper h = new EmployeePrxHelper();
            h.copyFrom__(bb);
            return h;
        }

        #endregion

        #region Marshaling support

        protected override Ice.ObjectDelM_ createDelegateM__()
        {
            return new EmployeeDelM_();
        }

        protected override Ice.ObjectDelD_ createDelegateD__()
        {
            return new EmployeeDelD_();
        }

        public static void write__(IceInternal.BasicStream os__, EmployeePrx v__)
        {
            os__.writeProxy(v__);
        }

        public static EmployeePrx read__(IceInternal.BasicStream is__)
        {
            Ice.ObjectPrx proxy = is__.readProxy();
            if(proxy != null)
            {
                EmployeePrxHelper result = new EmployeePrxHelper();
                result.copyFrom__(proxy);
                return result;
            }
            return null;
        }

        #endregion
    }

    public sealed class EmployeeSeqHelper
    {
        public static void write(IceInternal.BasicStream os__, _System.Collections.Generic.List<Employees.EmployeePrx> v__)
        {
            if(v__ == null)
            {
                os__.writeSize(0);
            }
            else
            {
                os__.writeSize(v__.Count);
                for(int ix__ = 0; ix__ < v__.Count; ++ix__)
                {
                    Employees.EmployeePrxHelper.write__(os__, v__[ix__]);
                }
            }
        }

        public static _System.Collections.Generic.List<Employees.EmployeePrx> read(IceInternal.BasicStream is__)
        {
            _System.Collections.Generic.List<Employees.EmployeePrx> v__;
            {
                int szx__ = is__.readSize();
                is__.startSeq(szx__, 2);
                v__ = new _System.Collections.Generic.List<Employees.EmployeePrx>();
                for(int ix__ = 0; ix__ < szx__; ++ix__)
                {
                    v__.Add(Employees.EmployeePrxHelper.read__(is__));
                    is__.checkSeq();
                    is__.endElement();
                }
                is__.endSeq(szx__);
            }
            return v__;
        }
    }

    public sealed class EmployeeFinderPrxHelper : Ice.ObjectPrxHelperBase, EmployeeFinderPrx
    {
        #region Synchronous operations

        public _System.Collections.Generic.List<Employees.EmployeePrx> list()
        {
            return list(null, false);
        }

        public _System.Collections.Generic.List<Employees.EmployeePrx> list(_System.Collections.Generic.Dictionary<string, string> context__)
        {
            return list(context__, true);
        }

        private _System.Collections.Generic.List<Employees.EmployeePrx> list(_System.Collections.Generic.Dictionary<string, string> context__, bool explicitContext__)
        {
            if(explicitContext__ && context__ == null)
            {
                context__ = emptyContext_;
            }
            int cnt__ = 0;
            while(true)
            {
                Ice.ObjectDel_ delBase__ = null;
                try
                {
                    checkTwowayOnly__("list");
                    delBase__ = getDelegate__(false);
                    EmployeeFinderDel_ del__ = (EmployeeFinderDel_)delBase__;
                    return del__.list(context__);
                }
                catch(IceInternal.LocalExceptionWrapper ex__)
                {
                    handleExceptionWrapper__(delBase__, ex__, null);
                }
                catch(Ice.LocalException ex__)
                {
                    handleException__(delBase__, ex__, null, ref cnt__);
                }
            }
        }

        #endregion

        #region Checked and unchecked cast operations

        public static EmployeeFinderPrx checkedCast(Ice.ObjectPrx b)
        {
            if(b == null)
            {
                return null;
            }
            EmployeeFinderPrx r = b as EmployeeFinderPrx;
            if((r == null) && b.ice_isA("::Employees::EmployeeFinder"))
            {
                EmployeeFinderPrxHelper h = new EmployeeFinderPrxHelper();
                h.copyFrom__(b);
                r = h;
            }
            return r;
        }

        public static EmployeeFinderPrx checkedCast(Ice.ObjectPrx b, _System.Collections.Generic.Dictionary<string, string> ctx)
        {
            if(b == null)
            {
                return null;
            }
            EmployeeFinderPrx r = b as EmployeeFinderPrx;
            if((r == null) && b.ice_isA("::Employees::EmployeeFinder", ctx))
            {
                EmployeeFinderPrxHelper h = new EmployeeFinderPrxHelper();
                h.copyFrom__(b);
                r = h;
            }
            return r;
        }

        public static EmployeeFinderPrx checkedCast(Ice.ObjectPrx b, string f)
        {
            if(b == null)
            {
                return null;
            }
            Ice.ObjectPrx bb = b.ice_facet(f);
            try
            {
                if(bb.ice_isA("::Employees::EmployeeFinder"))
                {
                    EmployeeFinderPrxHelper h = new EmployeeFinderPrxHelper();
                    h.copyFrom__(bb);
                    return h;
                }
            }
            catch(Ice.FacetNotExistException)
            {
            }
            return null;
        }

        public static EmployeeFinderPrx checkedCast(Ice.ObjectPrx b, string f, _System.Collections.Generic.Dictionary<string, string> ctx)
        {
            if(b == null)
            {
                return null;
            }
            Ice.ObjectPrx bb = b.ice_facet(f);
            try
            {
                if(bb.ice_isA("::Employees::EmployeeFinder", ctx))
                {
                    EmployeeFinderPrxHelper h = new EmployeeFinderPrxHelper();
                    h.copyFrom__(bb);
                    return h;
                }
            }
            catch(Ice.FacetNotExistException)
            {
            }
            return null;
        }

        public static EmployeeFinderPrx uncheckedCast(Ice.ObjectPrx b)
        {
            if(b == null)
            {
                return null;
            }
            EmployeeFinderPrx r = b as EmployeeFinderPrx;
            if(r == null)
            {
                EmployeeFinderPrxHelper h = new EmployeeFinderPrxHelper();
                h.copyFrom__(b);
                r = h;
            }
            return r;
        }

        public static EmployeeFinderPrx uncheckedCast(Ice.ObjectPrx b, string f)
        {
            if(b == null)
            {
                return null;
            }
            Ice.ObjectPrx bb = b.ice_facet(f);
            EmployeeFinderPrxHelper h = new EmployeeFinderPrxHelper();
            h.copyFrom__(bb);
            return h;
        }

        #endregion

        #region Marshaling support

        protected override Ice.ObjectDelM_ createDelegateM__()
        {
            return new EmployeeFinderDelM_();
        }

        protected override Ice.ObjectDelD_ createDelegateD__()
        {
            return new EmployeeFinderDelD_();
        }

        public static void write__(IceInternal.BasicStream os__, EmployeeFinderPrx v__)
        {
            os__.writeProxy(v__);
        }

        public static EmployeeFinderPrx read__(IceInternal.BasicStream is__)
        {
            Ice.ObjectPrx proxy = is__.readProxy();
            if(proxy != null)
            {
                EmployeeFinderPrxHelper result = new EmployeeFinderPrxHelper();
                result.copyFrom__(proxy);
                return result;
            }
            return null;
        }

        #endregion
    }
}

namespace Employees
{
    public interface EmployeeDel_ : Ice.ObjectDel_
    {
        Employees.EmployeeDetails getDetails(_System.Collections.Generic.Dictionary<string, string> context__);

        void setAnnualSalary(double salary, _System.Collections.Generic.Dictionary<string, string> context__);

        void setTaxPayable(double amount, _System.Collections.Generic.Dictionary<string, string> context__);
    }

    public interface EmployeeFinderDel_ : Ice.ObjectDel_
    {
        _System.Collections.Generic.List<Employees.EmployeePrx> list(_System.Collections.Generic.Dictionary<string, string> context__);
    }
}

namespace Employees
{
    public sealed class EmployeeDelM_ : Ice.ObjectDelM_, EmployeeDel_
    {
        public Employees.EmployeeDetails getDetails(_System.Collections.Generic.Dictionary<string, string> context__)
        {
            IceInternal.Outgoing og__ = handler__.getOutgoing("getDetails", Ice.OperationMode.Normal, context__);
            try
            {
                bool ok__ = og__.invoke();
                try
                {
                    if(!ok__)
                    {
                        try
                        {
                            og__.throwUserException();
                        }
                        catch(Ice.UserException ex__)
                        {
                            throw new Ice.UnknownUserException(ex__.ice_name(), ex__);
                        }
                    }
                    IceInternal.BasicStream is__ = og__.istr();
                    is__.startReadEncaps();
                    Employees.EmployeeDetails ret__;
                    ret__ = null;
                    if(ret__ == null)
                    {
                        ret__ = new Employees.EmployeeDetails();
                    }
                    ret__.read__(is__);
                    is__.endReadEncaps();
                    return ret__;
                }
                catch(Ice.LocalException ex__)
                {
                    throw new IceInternal.LocalExceptionWrapper(ex__, false);
                }
            }
            finally
            {
                handler__.reclaimOutgoing(og__);
            }
        }

        public void setAnnualSalary(double salary, _System.Collections.Generic.Dictionary<string, string> context__)
        {
            IceInternal.Outgoing og__ = handler__.getOutgoing("setAnnualSalary", Ice.OperationMode.Normal, context__);
            try
            {
                try
                {
                    IceInternal.BasicStream os__ = og__.ostr();
                    os__.writeDouble(salary);
                }
                catch(Ice.LocalException ex__)
                {
                    og__.abort(ex__);
                }
                bool ok__ = og__.invoke();
                if(!og__.istr().isEmpty())
                {
                    try
                    {
                        if(!ok__)
                        {
                            try
                            {
                                og__.throwUserException();
                            }
                            catch(Ice.UserException ex__)
                            {
                                throw new Ice.UnknownUserException(ex__.ice_name(), ex__);
                            }
                        }
                        og__.istr().skipEmptyEncaps();
                    }
                    catch(Ice.LocalException ex__)
                    {
                        throw new IceInternal.LocalExceptionWrapper(ex__, false);
                    }
                }
            }
            finally
            {
                handler__.reclaimOutgoing(og__);
            }
        }

        public void setTaxPayable(double amount, _System.Collections.Generic.Dictionary<string, string> context__)
        {
            IceInternal.Outgoing og__ = handler__.getOutgoing("setTaxPayable", Ice.OperationMode.Normal, context__);
            try
            {
                try
                {
                    IceInternal.BasicStream os__ = og__.ostr();
                    os__.writeDouble(amount);
                }
                catch(Ice.LocalException ex__)
                {
                    og__.abort(ex__);
                }
                bool ok__ = og__.invoke();
                if(!og__.istr().isEmpty())
                {
                    try
                    {
                        if(!ok__)
                        {
                            try
                            {
                                og__.throwUserException();
                            }
                            catch(Ice.UserException ex__)
                            {
                                throw new Ice.UnknownUserException(ex__.ice_name(), ex__);
                            }
                        }
                        og__.istr().skipEmptyEncaps();
                    }
                    catch(Ice.LocalException ex__)
                    {
                        throw new IceInternal.LocalExceptionWrapper(ex__, false);
                    }
                }
            }
            finally
            {
                handler__.reclaimOutgoing(og__);
            }
        }
    }

    public sealed class EmployeeFinderDelM_ : Ice.ObjectDelM_, EmployeeFinderDel_
    {
        public _System.Collections.Generic.List<Employees.EmployeePrx> list(_System.Collections.Generic.Dictionary<string, string> context__)
        {
            IceInternal.Outgoing og__ = handler__.getOutgoing("list", Ice.OperationMode.Normal, context__);
            try
            {
                bool ok__ = og__.invoke();
                try
                {
                    if(!ok__)
                    {
                        try
                        {
                            og__.throwUserException();
                        }
                        catch(Ice.UserException ex__)
                        {
                            throw new Ice.UnknownUserException(ex__.ice_name(), ex__);
                        }
                    }
                    IceInternal.BasicStream is__ = og__.istr();
                    is__.startReadEncaps();
                    _System.Collections.Generic.List<Employees.EmployeePrx> ret__;
                    {
                        int szx__ = is__.readSize();
                        is__.startSeq(szx__, 2);
                        ret__ = new _System.Collections.Generic.List<Employees.EmployeePrx>();
                        for(int ix__ = 0; ix__ < szx__; ++ix__)
                        {
                            ret__.Add(Employees.EmployeePrxHelper.read__(is__));
                            is__.checkSeq();
                            is__.endElement();
                        }
                        is__.endSeq(szx__);
                    }
                    is__.endReadEncaps();
                    return ret__;
                }
                catch(Ice.LocalException ex__)
                {
                    throw new IceInternal.LocalExceptionWrapper(ex__, false);
                }
            }
            finally
            {
                handler__.reclaimOutgoing(og__);
            }
        }
    }
}

namespace Employees
{
    public sealed class EmployeeDelD_ : Ice.ObjectDelD_, EmployeeDel_
    {
        public Employees.EmployeeDetails getDetails(_System.Collections.Generic.Dictionary<string, string> context__)
        {
            Ice.Current current__ = new Ice.Current();
            initCurrent__(ref current__, "getDetails", Ice.OperationMode.Normal, context__);
            Employees.EmployeeDetails result__ = new Employees.EmployeeDetails();
            IceInternal.Direct.RunDelegate run__ = delegate(Ice.Object obj__)
            {
                Employee servant__ = null;
                try
                {
                    servant__ = (Employee)obj__;
                }
                catch(_System.InvalidCastException)
                {
                    throw new Ice.OperationNotExistException(current__.id, current__.facet, current__.operation);
                }
                result__ = servant__.getDetails(current__);
                return Ice.DispatchStatus.DispatchOK;
            };
            IceInternal.Direct direct__ = null;
            try
            {
                direct__ = new IceInternal.Direct(current__, run__);
                try
                {
                    Ice.DispatchStatus status__ = direct__.servant().collocDispatch__(direct__);
                    _System.Diagnostics.Debug.Assert(status__ == Ice.DispatchStatus.DispatchOK);
                }
                finally
                {
                    direct__.destroy();
                }
            }
            catch(Ice.SystemException)
            {
                throw;
            }
            catch(System.Exception ex__)
            {
                IceInternal.LocalExceptionWrapper.throwWrapper(ex__);
            }
            return result__;
        }

        public void setAnnualSalary(double salary, _System.Collections.Generic.Dictionary<string, string> context__)
        {
            Ice.Current current__ = new Ice.Current();
            initCurrent__(ref current__, "setAnnualSalary", Ice.OperationMode.Normal, context__);
            IceInternal.Direct.RunDelegate run__ = delegate(Ice.Object obj__)
            {
                Employee servant__ = null;
                try
                {
                    servant__ = (Employee)obj__;
                }
                catch(_System.InvalidCastException)
                {
                    throw new Ice.OperationNotExistException(current__.id, current__.facet, current__.operation);
                }
                servant__.setAnnualSalary(salary, current__);
                return Ice.DispatchStatus.DispatchOK;
            };
            IceInternal.Direct direct__ = null;
            try
            {
                direct__ = new IceInternal.Direct(current__, run__);
                try
                {
                    Ice.DispatchStatus status__ = direct__.servant().collocDispatch__(direct__);
                    _System.Diagnostics.Debug.Assert(status__ == Ice.DispatchStatus.DispatchOK);
                }
                finally
                {
                    direct__.destroy();
                }
            }
            catch(Ice.SystemException)
            {
                throw;
            }
            catch(System.Exception ex__)
            {
                IceInternal.LocalExceptionWrapper.throwWrapper(ex__);
            }
        }

        public void setTaxPayable(double amount, _System.Collections.Generic.Dictionary<string, string> context__)
        {
            Ice.Current current__ = new Ice.Current();
            initCurrent__(ref current__, "setTaxPayable", Ice.OperationMode.Normal, context__);
            IceInternal.Direct.RunDelegate run__ = delegate(Ice.Object obj__)
            {
                Employee servant__ = null;
                try
                {
                    servant__ = (Employee)obj__;
                }
                catch(_System.InvalidCastException)
                {
                    throw new Ice.OperationNotExistException(current__.id, current__.facet, current__.operation);
                }
                servant__.setTaxPayable(amount, current__);
                return Ice.DispatchStatus.DispatchOK;
            };
            IceInternal.Direct direct__ = null;
            try
            {
                direct__ = new IceInternal.Direct(current__, run__);
                try
                {
                    Ice.DispatchStatus status__ = direct__.servant().collocDispatch__(direct__);
                    _System.Diagnostics.Debug.Assert(status__ == Ice.DispatchStatus.DispatchOK);
                }
                finally
                {
                    direct__.destroy();
                }
            }
            catch(Ice.SystemException)
            {
                throw;
            }
            catch(System.Exception ex__)
            {
                IceInternal.LocalExceptionWrapper.throwWrapper(ex__);
            }
        }
    }

    public sealed class EmployeeFinderDelD_ : Ice.ObjectDelD_, EmployeeFinderDel_
    {
        public _System.Collections.Generic.List<Employees.EmployeePrx> list(_System.Collections.Generic.Dictionary<string, string> context__)
        {
            Ice.Current current__ = new Ice.Current();
            initCurrent__(ref current__, "list", Ice.OperationMode.Normal, context__);
            _System.Collections.Generic.List<Employees.EmployeePrx> result__ = null;
            IceInternal.Direct.RunDelegate run__ = delegate(Ice.Object obj__)
            {
                EmployeeFinder servant__ = null;
                try
                {
                    servant__ = (EmployeeFinder)obj__;
                }
                catch(_System.InvalidCastException)
                {
                    throw new Ice.OperationNotExistException(current__.id, current__.facet, current__.operation);
                }
                result__ = servant__.list(current__);
                return Ice.DispatchStatus.DispatchOK;
            };
            IceInternal.Direct direct__ = null;
            try
            {
                direct__ = new IceInternal.Direct(current__, run__);
                try
                {
                    Ice.DispatchStatus status__ = direct__.servant().collocDispatch__(direct__);
                    _System.Diagnostics.Debug.Assert(status__ == Ice.DispatchStatus.DispatchOK);
                }
                finally
                {
                    direct__.destroy();
                }
            }
            catch(Ice.SystemException)
            {
                throw;
            }
            catch(System.Exception ex__)
            {
                IceInternal.LocalExceptionWrapper.throwWrapper(ex__);
            }
            return result__;
        }
    }
}

namespace Employees
{
    public abstract class EmployeeDisp_ : Ice.ObjectImpl, Employee
    {
        #region Slice operations

        public Employees.EmployeeDetails getDetails()
        {
            return getDetails(Ice.ObjectImpl.defaultCurrent);
        }

        public abstract Employees.EmployeeDetails getDetails(Ice.Current current__);

        public void setAnnualSalary(double salary)
        {
            setAnnualSalary(salary, Ice.ObjectImpl.defaultCurrent);
        }

        public abstract void setAnnualSalary(double salary, Ice.Current current__);

        public void setTaxPayable(double amount)
        {
            setTaxPayable(amount, Ice.ObjectImpl.defaultCurrent);
        }

        public abstract void setTaxPayable(double amount, Ice.Current current__);

        #endregion

        #region Slice type-related members

        public static new string[] ids__ = 
        {
            "::Employees::Employee",
            "::Ice::Object"
        };

        public override bool ice_isA(string s)
        {
            return _System.Array.BinarySearch(ids__, s, IceUtilInternal.StringUtil.OrdinalStringComparer) >= 0;
        }

        public override bool ice_isA(string s, Ice.Current current__)
        {
            return _System.Array.BinarySearch(ids__, s, IceUtilInternal.StringUtil.OrdinalStringComparer) >= 0;
        }

        public override string[] ice_ids()
        {
            return ids__;
        }

        public override string[] ice_ids(Ice.Current current__)
        {
            return ids__;
        }

        public override string ice_id()
        {
            return ids__[0];
        }

        public override string ice_id(Ice.Current current__)
        {
            return ids__[0];
        }

        public static new string ice_staticId()
        {
            return ids__[0];
        }

        #endregion

        #region Operation dispatch

        public static Ice.DispatchStatus getDetails___(Employee obj__, IceInternal.Incoming inS__, Ice.Current current__)
        {
            checkMode__(Ice.OperationMode.Normal, current__.mode);
            inS__.istr().skipEmptyEncaps();
            IceInternal.BasicStream os__ = inS__.ostr();
            Employees.EmployeeDetails ret__ = obj__.getDetails(current__);
            if(ret__ == null)
            {
                Employees.EmployeeDetails tmp__ = new Employees.EmployeeDetails();
                tmp__.write__(os__);
            }
            else
            {
                ret__.write__(os__);
            }
            return Ice.DispatchStatus.DispatchOK;
        }

        public static Ice.DispatchStatus setAnnualSalary___(Employee obj__, IceInternal.Incoming inS__, Ice.Current current__)
        {
            checkMode__(Ice.OperationMode.Normal, current__.mode);
            IceInternal.BasicStream is__ = inS__.istr();
            is__.startReadEncaps();
            double salary;
            salary = is__.readDouble();
            is__.endReadEncaps();
            obj__.setAnnualSalary(salary, current__);
            return Ice.DispatchStatus.DispatchOK;
        }

        public static Ice.DispatchStatus setTaxPayable___(Employee obj__, IceInternal.Incoming inS__, Ice.Current current__)
        {
            checkMode__(Ice.OperationMode.Normal, current__.mode);
            IceInternal.BasicStream is__ = inS__.istr();
            is__.startReadEncaps();
            double amount;
            amount = is__.readDouble();
            is__.endReadEncaps();
            obj__.setTaxPayable(amount, current__);
            return Ice.DispatchStatus.DispatchOK;
        }

        private static string[] all__ =
        {
            "getDetails",
            "ice_id",
            "ice_ids",
            "ice_isA",
            "ice_ping",
            "setAnnualSalary",
            "setTaxPayable"
        };

        public override Ice.DispatchStatus dispatch__(IceInternal.Incoming inS__, Ice.Current current__)
        {
            int pos = _System.Array.BinarySearch(all__, current__.operation, IceUtilInternal.StringUtil.OrdinalStringComparer);
            if(pos < 0)
            {
                throw new Ice.OperationNotExistException(current__.id, current__.facet, current__.operation);
            }

            switch(pos)
            {
                case 0:
                {
                    return getDetails___(this, inS__, current__);
                }
                case 1:
                {
                    return ice_id___(this, inS__, current__);
                }
                case 2:
                {
                    return ice_ids___(this, inS__, current__);
                }
                case 3:
                {
                    return ice_isA___(this, inS__, current__);
                }
                case 4:
                {
                    return ice_ping___(this, inS__, current__);
                }
                case 5:
                {
                    return setAnnualSalary___(this, inS__, current__);
                }
                case 6:
                {
                    return setTaxPayable___(this, inS__, current__);
                }
            }

            _System.Diagnostics.Debug.Assert(false);
            throw new Ice.OperationNotExistException(current__.id, current__.facet, current__.operation);
        }

        #endregion

        #region Marshaling support

        public override void write__(IceInternal.BasicStream os__)
        {
            os__.writeTypeId(ice_staticId());
            os__.startWriteSlice();
            os__.endWriteSlice();
            base.write__(os__);
        }

        public override void read__(IceInternal.BasicStream is__, bool rid__)
        {
            if(rid__)
            {
                /* string myId = */ is__.readTypeId();
            }
            is__.startReadSlice();
            is__.endReadSlice();
            base.read__(is__, true);
        }

        public override void write__(Ice.OutputStream outS__)
        {
            Ice.MarshalException ex = new Ice.MarshalException();
            ex.reason = "type Employees::Employee was not generated with stream support";
            throw ex;
        }

        public override void read__(Ice.InputStream inS__, bool rid__)
        {
            Ice.MarshalException ex = new Ice.MarshalException();
            ex.reason = "type Employees::Employee was not generated with stream support";
            throw ex;
        }

        #endregion
    }

    public abstract class EmployeeFinderDisp_ : Ice.ObjectImpl, EmployeeFinder
    {
        #region Slice operations

        public _System.Collections.Generic.List<Employees.EmployeePrx> list()
        {
            return list(Ice.ObjectImpl.defaultCurrent);
        }

        public abstract _System.Collections.Generic.List<Employees.EmployeePrx> list(Ice.Current current__);

        #endregion

        #region Slice type-related members

        public static new string[] ids__ = 
        {
            "::Employees::EmployeeFinder",
            "::Ice::Object"
        };

        public override bool ice_isA(string s)
        {
            return _System.Array.BinarySearch(ids__, s, IceUtilInternal.StringUtil.OrdinalStringComparer) >= 0;
        }

        public override bool ice_isA(string s, Ice.Current current__)
        {
            return _System.Array.BinarySearch(ids__, s, IceUtilInternal.StringUtil.OrdinalStringComparer) >= 0;
        }

        public override string[] ice_ids()
        {
            return ids__;
        }

        public override string[] ice_ids(Ice.Current current__)
        {
            return ids__;
        }

        public override string ice_id()
        {
            return ids__[0];
        }

        public override string ice_id(Ice.Current current__)
        {
            return ids__[0];
        }

        public static new string ice_staticId()
        {
            return ids__[0];
        }

        #endregion

        #region Operation dispatch

        public static Ice.DispatchStatus list___(EmployeeFinder obj__, IceInternal.Incoming inS__, Ice.Current current__)
        {
            checkMode__(Ice.OperationMode.Normal, current__.mode);
            inS__.istr().skipEmptyEncaps();
            IceInternal.BasicStream os__ = inS__.ostr();
            _System.Collections.Generic.List<Employees.EmployeePrx> ret__ = obj__.list(current__);
            if(ret__ == null)
            {
                os__.writeSize(0);
            }
            else
            {
                os__.writeSize(ret__.Count);
                for(int ix__ = 0; ix__ < ret__.Count; ++ix__)
                {
                    Employees.EmployeePrxHelper.write__(os__, ret__[ix__]);
                }
            }
            return Ice.DispatchStatus.DispatchOK;
        }

        private static string[] all__ =
        {
            "ice_id",
            "ice_ids",
            "ice_isA",
            "ice_ping",
            "list"
        };

        public override Ice.DispatchStatus dispatch__(IceInternal.Incoming inS__, Ice.Current current__)
        {
            int pos = _System.Array.BinarySearch(all__, current__.operation, IceUtilInternal.StringUtil.OrdinalStringComparer);
            if(pos < 0)
            {
                throw new Ice.OperationNotExistException(current__.id, current__.facet, current__.operation);
            }

            switch(pos)
            {
                case 0:
                {
                    return ice_id___(this, inS__, current__);
                }
                case 1:
                {
                    return ice_ids___(this, inS__, current__);
                }
                case 2:
                {
                    return ice_isA___(this, inS__, current__);
                }
                case 3:
                {
                    return ice_ping___(this, inS__, current__);
                }
                case 4:
                {
                    return list___(this, inS__, current__);
                }
            }

            _System.Diagnostics.Debug.Assert(false);
            throw new Ice.OperationNotExistException(current__.id, current__.facet, current__.operation);
        }

        #endregion

        #region Marshaling support

        public override void write__(IceInternal.BasicStream os__)
        {
            os__.writeTypeId(ice_staticId());
            os__.startWriteSlice();
            os__.endWriteSlice();
            base.write__(os__);
        }

        public override void read__(IceInternal.BasicStream is__, bool rid__)
        {
            if(rid__)
            {
                /* string myId = */ is__.readTypeId();
            }
            is__.startReadSlice();
            is__.endReadSlice();
            base.read__(is__, true);
        }

        public override void write__(Ice.OutputStream outS__)
        {
            Ice.MarshalException ex = new Ice.MarshalException();
            ex.reason = "type Employees::EmployeeFinder was not generated with stream support";
            throw ex;
        }

        public override void read__(Ice.InputStream inS__, bool rid__)
        {
            Ice.MarshalException ex = new Ice.MarshalException();
            ex.reason = "type Employees::EmployeeFinder was not generated with stream support";
            throw ex;
        }

        #endregion
    }
}
