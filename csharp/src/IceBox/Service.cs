//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceBox
{
    public partial class FailureException : global::Ice.LocalException
    {
        public string reason;

        private void _initDM()
        {
            this.reason = "";
        }

        public FailureException()
        {
            _initDM();
        }

        public FailureException(global::System.Exception ex) : base(ex)
        {
            _initDM();
        }

        public FailureException(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context) : base(info, context)
        {
            this.reason = info.GetString("reason");
        }

        private void _initDM(string reason)
        {
            this.reason = reason;
        }

        public FailureException(string reason)
        {
            _initDM(reason);
        }

        public FailureException(string reason, global::System.Exception ex) : base(ex)
        {
            _initDM(reason);
        }

        public override string ice_id()
        {
            return "::IceBox::FailureException";
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::IceBox::FailureException");
            global::IceInternal.HashUtil.hashAdd(ref h_, reason);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (other == null)
            {
                return false;
            }
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            FailureException o = other as FailureException;
            if (o == null)
            {
                return false;
            }
            if (this.reason == null)
            {
                if (o.reason != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.reason.Equals(o.reason))
                {
                    return false;
                }
            }
            return true;
        }

        public override void GetObjectData(global::System.Runtime.Serialization.SerializationInfo info, global::System.Runtime.Serialization.StreamingContext context)
        {
            info.AddValue("reason", this.reason == null ? "" : this.reason);

            base.GetObjectData(info, context);
        }

        public static bool operator ==(FailureException lhs, FailureException rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(FailureException lhs, FailureException rhs)
        {
            return !Equals(lhs, rhs);
        }
    }

    public partial interface Service
    {
        /// <summary>
        /// Start the service.
        /// The given communicator is created by the
        /// ServiceManager for use by the service. This communicator may
        /// also be used by other services, depending on the service
        /// configuration.
        ///
        /// The ServiceManager owns this communicator, and is
        /// responsible for destroying it.
        ///
        /// </summary>
        /// <param name="name">The service's name, as determined by the
        /// configuration.
        ///
        /// </param>
        /// <param name="communicator">A communicator for use by the service.
        ///
        /// </param>
        /// <param name="args">The service arguments that were not converted into
        /// properties.
        ///
        /// </param>
        /// <exception name="FailureException">Raised if start failed.</exception>
        void start(string name, global::Ice.Communicator communicator, string[] args);

        /// <summary>
        /// Stop the service.
        /// </summary>
        void stop();
    }
}
