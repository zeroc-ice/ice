//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;
using System;
using System.ComponentModel;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace Ice
{
    ///<summary>The base interface for all servants.</summary>
    public interface IObject
    {
        /// <summary>Dispatches a request on this servant.</summary>
        /// <param name="istr">The <see cref="InputStream"/> that holds the request frame. It is positionned at
        /// the start of the request's payload, just after starting to read the encapsulation.</param>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// current request.</param>
        /// <returns>A value task that provides the response frame for the request. Null means the request is a oneway
        /// request. See <see cref="IceInternal.Protocol.StartResponseFrame"/>.</returns>
        /// <exception cref="System.Exception">Any exception thrown by Dispatch is marshaled into the response frame.
        /// </exception>
        public ValueTask<OutputStream>? Dispatch(InputStream istr, Current current)
        {
            // TODO: switch to abstract method
            Debug.Assert(false);
            return null;
        }

        // The following are helper methods for generated servants.

        /// <summary>
        /// Tests whether this object can be reached.
        /// </summary>
        /// <param name="current">The Current object for the dispatch.</param>
        public void IcePing(Current current)
        {
            // Does nothing
        }

        /// <summary>Tests whether this object supports the specified Slice interface.</summary>
        /// <param name="typeId">The type ID of the Slice interface to test against.</param>
        /// <param name="current">The Current object for the dispatch.</param>
        /// <returns>True if this object implements the interface specified by typeId.</returns>
        public bool IceIsA(string typeId, Current current)
            => Array.BinarySearch(IceIds(current), typeId, StringComparer.Ordinal) >= 0;

        /// <summary>Returns the Slice type IDs of the interfaces supported by this object.</summary>
        /// <param name="current">The Current object for the dispatch.</param>
        /// <returns>The Slice type IDs of the interfaces supported by this object, in alphabetical order.</returns>
        public string[] IceIds(Current current) => new string[]{ "::Ice::Object" };

        /// <summary>Returns the Slice type ID of the most-derived interface supported by this object.</summary>
        /// <param name="current">The Current object for the dispatch.</param>
        /// <returns>The Slice type ID of the most-derived interface.</returns>
        public string IceId(Current current) => "::Ice::Object";

        // The following protected static methods with Ice-prefixes are Ice-internal helper methods used by
        // generated servants.

        protected static void IceCheckMode(OperationMode expected, OperationMode received)
        {
            if (expected != received)
            {
                if (expected == OperationMode.Idempotent && received == OperationMode.Nonmutating)
                {
                    // Fine: typically an old client still using the deprecated nonmutating keyword or metadata.
                }
                else
                {
                    throw new MarshalException(
                        $"unexpected operation mode. expected = {expected} received = {received}");
                }
            }
        }
        protected static ValueTask<Ice.OutputStream> IceFromValueTask<R>(ValueTask<R> valueTask,
            Current current, FormatType? format, Action<Ice.OutputStream, R> write)
        {
            // NOTE: it's important that the continuation doesn't mutate the request state to
            // guarantee thread-safety. Multiple continuations can execute concurrently if the
            // user installed a dispatch interceptor and the dispatch is retried.

            if (valueTask.IsCompleted)
            {
                Ice.OutputStream ostr = Protocol.StartResponseFrame(current, format);
                write(ostr, valueTask.Result);
                ostr.EndEncapsulation();
                return IceFromResult(ostr);
            }
            else
            {
                return FromTask(valueTask.AsTask().ContinueWith((Task<R> t) =>
                    {
                        R result = t.GetAwaiter().GetResult();
                        Ice.OutputStream ostr = Protocol.StartResponseFrame(current, format);
                        write(ostr, result);
                        ostr.EndEncapsulation();
                        return Task.FromResult<Ice.OutputStream>(ostr);
                    },
                    CancellationToken.None,
                    TaskContinuationOptions.ExecuteSynchronously,
                    TaskScheduler.Current).Unwrap());
            }
        }

        protected static ValueTask<Ice.OutputStream> IceFromValueTask<T>(ValueTask<T> valueTask)
            where T : struct, IMarshaledReturnValue
        {
            if (valueTask.IsCompleted)
            {
                return IceFromResult(valueTask.Result.OutputStream);
            }
            else
            {
                return FromTask(valueTask.AsTask().ContinueWith((Task<T> t) =>
                    Task.FromResult<Ice.OutputStream>(t.GetAwaiter().GetResult().OutputStream),
                    CancellationToken.None,
                    TaskContinuationOptions.ExecuteSynchronously,
                    TaskScheduler.Current).Unwrap());
            }
        }

        protected static ValueTask<Ice.OutputStream>? IceFromTask(Task? task, Current current)
        {
            if (current.IsOneway)
            {
                return null;
            }
            else if (task == null)
            {
                // A null task as response to a two-way request means success:
                return IceFromVoidResult(current);
            }
            else
            {
                // NOTE: it's important that the continuation doesn't mutate the request state to
                // guarantee thread-safety. Multiple continuations can execute concurrently if the
                // user installed a dispatch interceptor and the dispatch is retried.
                return FromTask(task.ContinueWith((Task t) =>
                    {
                        t.GetAwaiter().GetResult();
                        var ostr = Protocol.CreateEmptyResponseFrame(current);
                        return Task.FromResult(ostr);
                    },
                    CancellationToken.None,
                    TaskContinuationOptions.ExecuteSynchronously,
                    TaskScheduler.Current).Unwrap());
            }
        }

        protected static ValueTask<Ice.OutputStream> IceFromResult(Ice.OutputStream ostr)
            => new ValueTask<Ice.OutputStream>(ostr);

        protected static ValueTask<Ice.OutputStream>? IceFromVoidResult(Ice.Current current)
        {
            if (current.IsOneway)
            {
                return null;
            }
            else
            {
                return IceFromResult(Protocol.CreateEmptyResponseFrame(current));
            }
        }

        protected ValueTask<OutputStream>? IceD_ice_ping(InputStream istr, Current current)
        {
            istr.CheckIsReadable();
            istr.EndEncapsulation();
            IcePing(current);
            return IceFromVoidResult(current);
        }

        protected ValueTask<OutputStream> IceD_ice_isA(InputStream istr, Current current)
        {
            istr.CheckIsReadable();
            string id = istr.ReadString();
            istr.EndEncapsulation();
            bool ret = IceIsA(id, current);
            OutputStream ostr = Protocol.StartResponseFrame(current);
            ostr.WriteBool(ret);
            ostr.EndEncapsulation();
            return IceFromResult(ostr);
        }

        protected ValueTask<OutputStream> IceD_ice_id(InputStream istr, Current current)
        {
            istr.CheckIsReadable();
            istr.EndEncapsulation();
            string ret = IceId(current);
            OutputStream ostr = Protocol.StartResponseFrame(current);
            ostr.WriteString(ret);
            ostr.EndEncapsulation();
            return IceFromResult(ostr);
        }

        protected ValueTask<OutputStream> IceD_ice_ids(InputStream istr, Current current)
        {
            istr.CheckIsReadable();
            istr.EndEncapsulation();
            string[] ret = IceIds(current);
            OutputStream ostr = Protocol.StartResponseFrame(current);
            ostr.WriteStringSeq(ret);
            ostr.EndEncapsulation();
            return IceFromResult(ostr);
        }
        private static ValueTask<Ice.OutputStream> FromTask(Task<Ice.OutputStream> task)
            => new ValueTask<Ice.OutputStream>(task);
    }

    /// <summary>All marshaled return value structs generated by the Slice compiler implement this
    /// IMarshaledResultValue interface.</summary>
    public interface IMarshaledReturnValue
    {
        public OutputStream OutputStream { get; }
    }

    // TODO: remove once the proxy InvokeAsync signature is updated to match the Dispatch signature.
    public abstract class BlobjectAsync : IObject
    {
        public abstract Task<Ice.Object_Ice_invokeResult> IceInvokeAsync(byte[] inEncaps, Current current);
        public ValueTask<OutputStream>? Dispatch(InputStream istr, Current current)
        {
            byte[] inEncaps = ReadParamEncaps(istr);
            Task<Object_Ice_invokeResult> task = IceInvokeAsync(inEncaps, current);
            if (current.IsOneway)
            {
                return null;
            }
            else
            {
                return new ValueTask<OutputStream>(task.ContinueWith((Task<Object_Ice_invokeResult> t) =>
                    {
                        Object_Ice_invokeResult ret = t.GetAwaiter().GetResult();
                        return Task.FromResult(WriteParamEncaps(ret.OutEncaps, ret.ReturnValue, current));
                    },
                    TaskScheduler.Current).Unwrap());
            }
        }

        private static byte[] ReadParamEncaps(InputStream istr)
        {
            istr.Pos -= 6;
            byte[] result = istr.ReadEncapsulation(out Ice.EncodingVersion encoding);
            return result;
        }

        private static Ice.OutputStream WriteParamEncaps(byte[]? v, bool ok, Current current)
        {
            Debug.Assert(!current.IsOneway);

            if (!ok)
            {
                current.DispatchObserver?.UserException();
            }

            var ostr = new Ice.OutputStream(current.Adapter.Communicator, Ice.Util.CurrentProtocolEncoding);
            ostr.WriteBlob(Protocol.replyHdr);
            ostr.WriteInt(current.RequestId);
            ostr.WriteByte(ok ? ReplyStatus.replyOK : ReplyStatus.replyUserException);

            if (v == null || v.Length == 0)
            {
                ostr.WriteEmptyEncapsulation(current.Encoding);
            }
            else
            {
                ostr.WriteEncapsulation(v);
            }
            return ostr;
        }
    }
}
