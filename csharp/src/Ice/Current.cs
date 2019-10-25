namespace Ice
{
    public partial class Current : global::System.ICloneable
    {
        public ObjectAdapter adapter;
        public Connection con;
        public Identity id;
        public string facet;
        public string operation;
        public OperationMode mode;
        public System.Collections.Generic.Dictionary<string, string> ctx;
        public int requestId;
        public EncodingVersion encoding;

        public Current()
        {
            id = new Identity();
            facet = "";
            operation = "";
            encoding = new EncodingVersion();
        }

        public Current(ObjectAdapter adapter, Connection con, Identity id, string facet, string operation, OperationMode mode, global::System.Collections.Generic.Dictionary<string, string> ctx, int requestId, EncodingVersion encoding)
        {
            this.adapter = adapter;
            this.con = con;
            this.id = id;
            this.facet = facet;
            this.operation = operation;
            this.mode = mode;
            this.ctx = ctx;
            this.requestId = requestId;
            this.encoding = encoding;
        }

        public object Clone()
        {
            return MemberwiseClone();
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            global::IceInternal.HashUtil.hashAdd(ref h_, "::Ice::Current");
            global::IceInternal.HashUtil.hashAdd(ref h_, adapter);
            global::IceInternal.HashUtil.hashAdd(ref h_, con);
            global::IceInternal.HashUtil.hashAdd(ref h_, id);
            global::IceInternal.HashUtil.hashAdd(ref h_, facet);
            global::IceInternal.HashUtil.hashAdd(ref h_, operation);
            global::IceInternal.HashUtil.hashAdd(ref h_, mode);
            global::IceInternal.HashUtil.hashAdd(ref h_, ctx);
            global::IceInternal.HashUtil.hashAdd(ref h_, requestId);
            global::IceInternal.HashUtil.hashAdd(ref h_, encoding);
            return h_;
        }

        public override bool Equals(object other)
        {
            if (object.ReferenceEquals(this, other))
            {
                return true;
            }
            if (other == null)
            {
                return false;
            }
            if (GetType() != other.GetType())
            {
                return false;
            }
            Current o = (Current)other;
            if (this.adapter == null)
            {
                if (o.adapter != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.adapter.Equals(o.adapter))
                {
                    return false;
                }
            }
            if (this.con == null)
            {
                if (o.con != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.con.Equals(o.con))
                {
                    return false;
                }
            }
            if (this.id == null)
            {
                if (o.id != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.id.Equals(o.id))
                {
                    return false;
                }
            }
            if (this.facet == null)
            {
                if (o.facet != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.facet.Equals(o.facet))
                {
                    return false;
                }
            }
            if (this.operation == null)
            {
                if (o.operation != null)
                {
                    return false;
                }
            }
            else
            {
                if (!this.operation.Equals(o.operation))
                {
                    return false;
                }
            }
            if (!this.mode.Equals(o.mode))
            {
                return false;
            }
            if (this.ctx == null)
            {
                if (o.ctx != null)
                {
                    return false;
                }
            }
            else
            {
                if (!global::IceUtilInternal.Collections.DictionaryEquals(this.ctx, o.ctx))
                {
                    return false;
                }
            }
            if (!this.requestId.Equals(o.requestId))
            {
                return false;
            }
            if (!this.encoding.Equals(o.encoding))
            {
                return false;
            }
            return true;
        }

        public static bool operator ==(Current lhs, Current rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(Current lhs, Current rhs)
        {
            return !Equals(lhs, rhs);
        }
    }
}
