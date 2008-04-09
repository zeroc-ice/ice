// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace IceInternal
{
    public abstract class Reference : ICloneable
    {
        public enum Mode {
                            ModeTwoway,
                            ModeOneway,
                            ModeBatchOneway,
                            ModeDatagram,
                            ModeBatchDatagram,
                            ModeLast=ModeBatchDatagram
                         };

        public Mode getMode()
        {
            return mode_;
        }

        public Ice.Identity getIdentity()
        {
            return identity_;
        }

        public string getFacet()
        {
            return facet_;
        }

        public Instance getInstance()
        {
            return instance_;
        }

        public Dictionary<string, string> getContext()
        {
            return context_;
        }

        public Reference defaultContext()
        {
            Reference r = instance_.referenceFactory().copy(this);
            r.context_ = instance_.getDefaultContext();
            return r;
        }

        public Ice.Communicator getCommunicator()
        {
            return communicator_;
        }

        public bool getSecure()
        {
            return _secure;
        }

        public bool getCompress()
        {
            return compress_;
        }

        public int getTimeout()
        {
            return timeout_;
        }

        public abstract string getAdapterId();
        public abstract EndpointI[] getEndpoints();

        //
        // The change* methods (here and in derived classes) create
        // a new reference based on the existing one, with the
        // corresponding value changed.
        //
        public Reference changeContext(Dictionary<string, string> newContext)
        {
            if(newContext == null)
            {
                newContext = _emptyContext;
            }
            Reference r = instance_.referenceFactory().copy(this);
            if(newContext.Count == 0)
            {
                r.context_ = _emptyContext;
            }
            else
            {
                r.context_ = new Dictionary<string, string>(newContext);
            }
            return r;
        }

        public Reference changeMode(Mode newMode)
        {
            if(newMode == mode_)
            {
                return this;
            }
            Reference r = instance_.referenceFactory().copy(this);
            r.mode_ = newMode;
            return r;
        }

        public Reference changeIdentity(Ice.Identity newIdentity)
        {
            if(newIdentity.Equals(identity_))
            {
                return this;
            }
            Reference r = instance_.referenceFactory().copy(this);
            r.identity_ = newIdentity; // Identity is a value type, therefore a copy of newIdentity is made.
            return r;
        }

        public Reference changeFacet(string newFacet)
        {
            if(newFacet.Equals(facet_))
            {
                return this;
            }
            Reference r = instance_.referenceFactory().copy(this);
            r.facet_ = newFacet;
            return r;
        }

        public Reference changeSecure(bool newSecure)
        {
            if(newSecure == _secure)
            {
                return this;
            }
            Reference r = instance_.referenceFactory().copy(this);
            r._secure = newSecure;
            return r;
        }

        public virtual Reference changeCompress(bool newCompress)
        {
            if(compress_ == newCompress)
            {
                return this;
            }
            Reference r = instance_.referenceFactory().copy(this);
            r.compress_ = newCompress;
            return r;
        }

        public virtual Reference changeTimeout(int newTimeout)
        {
            if(timeout_ == newTimeout)
            {
                return this;
            }
            Reference r = instance_.referenceFactory().copy(this);
            r.timeout_ = newTimeout;
            return r;
        }

        public abstract Reference changeAdapterId(string newAdapterId);
        public abstract Reference changeEndpoints(EndpointI[] newEndpoints);

        public override int GetHashCode()
        {
            lock(this)
            {
                if(hashInitialized_)
                {
                    return hashValue_;
                }

                int h = (int)mode_;

                int sz = identity_.name.Length;
                for(int i = 0; i < sz; i++)
                {   
                    h = 5 * h + (int)identity_.name[i];
                }

                sz = identity_.category.Length;
                for(int i = 0; i < sz; i++)
                {   
                    h = 5 * h + (int)identity_.category[i];
                }

                h = 5 * h + context_.GetHashCode();

                sz = facet_.Length;
                for(int i = 0; i < sz; i++)
                {   
                    h = 5 * h + (int)facet_[i];
                }

                h = 5 * h + (getSecure() ? 1 : 0);

                hashValue_ = h;
                hashInitialized_ = true;

                return h;
            }
        }

        //
        // Marshal the reference.
        //
        public virtual void streamWrite(BasicStream s)
        {
            //
            // Don't write the identity here. Operations calling streamWrite
            // write the identity.
            //

            //
            // For compatibility with the old FacetPath.
            //
            if(facet_.Length == 0)
            {
                s.writeStringSeq(null);
            }
            else
            {
                string[] facetPath = { facet_ };
                s.writeStringSeq(facetPath);
            }

            s.writeByte((byte)mode_);

            s.writeBool(getSecure());

            // Derived class writes the remainder of the reference.
        }

        //
        // Convert the reference to its string form.
        //
        public override string ToString()
        {
            //
            // WARNING: Certain features, such as proxy validation in Glacier2,
            // depend on the format of proxy strings. Changes to toString() and
            // methods called to generate parts of the reference string could break
            // these features. Please review for all features that depend on the
            // format of proxyToString() before changing this and related code.
            //
            StringBuilder s = new StringBuilder();

            //
            // If the encoded identity string contains characters which
            // the reference parser uses as separators, then we enclose
            // the identity string in quotes.
            //
            string id = instance_.identityToString(identity_);
            if(IceUtil.StringUtil.findFirstOf(id, " \t\n\r:@") != -1)
            {
                s.Append('"');
                s.Append(id);
                s.Append('"');
            }
            else
            {
                s.Append(id);
            }

            if(facet_.Length > 0)
            {
                //
                // If the encoded facet string contains characters which
                // the reference parser uses as separators, then we enclose
                // the facet string in quotes.
                //
                s.Append(" -f ");
                string fs = IceUtil.StringUtil.escapeString(facet_, "");
                if(IceUtil.StringUtil.findFirstOf(fs, " \t\n\r:@") != -1)
                {
                    s.Append('"');
                    s.Append(fs);
                    s.Append('"');
                }
                else
                {
                    s.Append(fs);
                }
            }

            switch(mode_)
            {
            case Mode.ModeTwoway:
                {
                    s.Append(" -t");
                    break;
                }

            case Mode.ModeOneway:
                {
                    s.Append(" -o");
                    break;
                }

            case Mode.ModeBatchOneway:
                {
                    s.Append(" -O");
                    break;
                }

            case Mode.ModeDatagram:
                {
                    s.Append(" -d");
                    break;
                }

            case Mode.ModeBatchDatagram:
                {
                    s.Append(" -D");
                    break;
                }
            }

            if(getSecure())
            {
                s.Append(" -s");
            }

            return s.ToString();

            // Derived class writes the remainder of the string.
        }

        public override bool Equals(object obj)
        {
            //
            // Note: if(this == obj) and type test are performed by each non-abstract derived class.
            //

            Reference r = (Reference)obj; // Guaranteed to succeed.

            if(mode_ != r.mode_)
            {
                return false;
            }

            if(!identity_.Equals(r.identity_))
            {
                return false;
            }

            if(!Ice.CollectionComparer.Equals(context_, r.context_))
            {
                return false;
            }

            if(!facet_.Equals(r.facet_))
            {
                return false;
            }

            if(_secure != r._secure)
            {
                return false;
            }

            if(compress_ != r.compress_)
            {
                return false;
            }

            if(timeout_ != r.timeout_)
            {
                return false;
            }

            return true;
        }

        public Object Clone()
        {
            //
            // A member-wise copy is safe because the members are immutable.
            //
            return MemberwiseClone();
        }

        private Instance instance_;
        private Ice.Communicator communicator_;
        private Mode mode_;
        private Ice.Identity identity_;
        private Dictionary<string, string> context_;
        private static Dictionary<string, string> _emptyContext = new Dictionary<string, string>();
        private string facet_;
        private bool _secure;
        private bool compress_;
        private int timeout_;

        protected int hashValue_;
        protected bool hashInitialized_;

        protected Reference(Instance inst,
                            Ice.Communicator com,
                            Ice.Identity ident,
                            Dictionary<string, string> ctx,
                            string fac,
                            Mode md,
                            bool secure)
        {
            //
            // Validate string arguments.
            //
            Debug.Assert(ident.name != null);
            Debug.Assert(ident.category != null);
            Debug.Assert(fac != null);

            instance_ = inst;
            communicator_ = com;
            mode_ = md;
            identity_ = ident;
            context_ = ctx == null ? _emptyContext : ctx;
            facet_ = fac;
            hashInitialized_ = false;
            _secure = secure;
            compress_ = false;
            timeout_ = -1;
        }

        protected static System.Random rand_ = new System.Random(unchecked((int)System.DateTime.Now.Ticks));
    }

    public class DirectReference : Reference
    {
        public DirectReference(Instance inst,
                               Ice.Communicator com,
                               Ice.Identity ident,
                               Dictionary<string, string> ctx,
                               string fs,
                               Reference.Mode md,
                               bool sec,
                               EndpointI[] endpts)
            : base(inst, com, ident, ctx, fs, md, sec)
        {
            _endpoints = endpts;
        }

        public override string getAdapterId()
        {
            return "";
        }

        public override EndpointI[] getEndpoints()
        {
            return _endpoints;
        }

        public override Reference changeCompress(bool newCompress)
        {
            DirectReference r = (DirectReference)base.changeCompress(newCompress);
            if(r != this) // Also override the compress flag on the endpoints if it was updated.
            {
                EndpointI[] newEndpoints = new EndpointI[_endpoints.Length];
                for(int i = 0; i < _endpoints.Length; i++)
                {
                    newEndpoints[i] = _endpoints[i].compress(newCompress);
                }
                r._endpoints = newEndpoints;
            }
            return r;
        }

        public override Reference changeTimeout(int newTimeout)
        {
            DirectReference r = (DirectReference)base.changeTimeout(newTimeout);
            if(r != this) // Also override the timeout on the endpoints if it was updated.
            {
                EndpointI[] newEndpoints = new EndpointI[_endpoints.Length];
                for(int i = 0; i < _endpoints.Length; i++)
                {
                    newEndpoints[i] = _endpoints[i].timeout(newTimeout);
                }
                r._endpoints = newEndpoints;
            }
            return r;
        }

        public override Reference changeAdapterId(string newAdapterId)
        {
            if(newAdapterId == null || newAdapterId.Length == 0)
            {
                return this;
            }
            return getInstance().referenceFactory().create(
                getIdentity(), getContext(), getFacet(), getMode(), getSecure(), newAdapterId);
        }

        public override Reference changeEndpoints(EndpointI[] newEndpoints)
        {
            if(Array.Equals(newEndpoints, _endpoints))
            {
                return this;
            }
            DirectReference r = (DirectReference)getInstance().referenceFactory().copy(this);
            r._endpoints = newEndpoints;
            return r;
        }

        public override void streamWrite(BasicStream s)
        {
            base.streamWrite(s);

            s.writeSize(_endpoints.Length);
            if(_endpoints.Length > 0)
            {
                for(int i = 0; i < _endpoints.Length; i++)
                {
                    _endpoints[i].streamWrite(s);
                }
            }
            else
            {
                s.writeString(""); // Adapter id.
            }
        }

        public override string ToString()
        {
            //
            // WARNING: Certain features, such as proxy validation in Glacier2,
            // depend on the format of proxy strings. Changes to toString() and
            // methods called to generate parts of the reference string could break
            // these features. Please review for all features that depend on the
            // format of proxyToString() before changing this and related code.
            //
            StringBuilder s = new StringBuilder();
            s.Append(base.ToString());

            for(int i = 0; i < _endpoints.Length; i++)
            {
                string endp = _endpoints[i].ToString();
                if(endp != null && endp.Length > 0)
                {
                    s.Append(':');
                    s.Append(endp);
                }
            }
            return s.ToString();
        }

        public override bool Equals(object obj)
        {
            if(Object.ReferenceEquals(this, obj))
            {
                return true;
            }
            if(!(obj is DirectReference))
            {
                return false;
            }
            DirectReference rhs = (DirectReference)obj;
            if(!base.Equals(rhs))
            {
                return false;
            }
            return IceUtil.Arrays.Equals(_endpoints, rhs._endpoints);
        }

        //
        // If we override Equals, we must also override GetHashCode.
        //
        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        private EndpointI[] _endpoints;
    }

    public class IndirectReference : Reference
    {
        public IndirectReference(Instance inst,
                                 Ice.Communicator com,
                                 Ice.Identity ident,
                                 Dictionary<string, string> ctx,
                                 string fs,
                                 Reference.Mode md,
                                 bool sec,
                                 string adptid)
            : base(inst, com, ident, ctx, fs, md, sec)
        {
            adapterId_ = adptid;
        }

        public override string getAdapterId()
        {
            return adapterId_;
        }

        public override EndpointI[] getEndpoints()
        {
            return new EndpointI[0];
        }

        public override Reference changeAdapterId(string newAdapterId)
        {
            if(adapterId_.Equals(newAdapterId))
            {
                return this;
            }
            IndirectReference r = (IndirectReference)getInstance().referenceFactory().copy(this);
            r.adapterId_ = newAdapterId;
            return r;
        }

        public override Reference changeEndpoints(EndpointI[] newEndpoints)
        {
            if(newEndpoints == null || newEndpoints.Length == 0)
            {
                return this;
            }
            return getInstance().referenceFactory().create(
                getIdentity(), getContext(), getFacet(), getMode(), getSecure(), newEndpoints);
        }

        public override void streamWrite(BasicStream s)
        {
            base.streamWrite(s);

            s.writeSize(0);
            s.writeString(adapterId_);
        }

        public override string ToString()
        {
            //
            // WARNING: Certain features, such as proxy validation in Glacier2,
            // depend on the format of proxy strings. Changes to toString() and
            // methods called to generate parts of the reference string could break
            // these features. Please review for all features that depend on the
            // format of proxyToString() before changing this and related code.
            //
            string result = base.ToString();

            if(adapterId_.Length == 0)
            {
                return result;
            }

            StringBuilder s = new StringBuilder();
            s.Append(result);
            s.Append(" @ ");

            //
            // If the encoded adapter id string contains characters which
            // the reference parser uses as separators, then we enclose
            // the adapter id string in quotes.
            //
            string a = IceUtil.StringUtil.escapeString(adapterId_, null);
            if(IceUtil.StringUtil.findFirstOf(a, " \t\n\r") != -1)
            {
                s.Append('"');
                s.Append(a);
                s.Append('"');
            }
            else
            {
                s.Append(a);
            }
            return s.ToString();
        }

        public override bool Equals(object obj)
        {
            if(object.ReferenceEquals(this, obj))
            {
                return true;
            }
            if(!(obj is IndirectReference))
            {
                return false;
            }
            IndirectReference rhs = (IndirectReference)obj;
            if(!base.Equals(rhs))
            {
                return false;
            }
            if(!adapterId_.Equals(rhs.adapterId_))
            {
                return false;
            }
            return true;
        }

        //
        // If we override Equals, we must also override GetHashCode.
        //
        public override int GetHashCode()
        {
            lock(this)
            {
                if(base.hashInitialized_)
                {
                    return hashValue_;
                }
                base.GetHashCode();         // Initializes hashValue_.
                int sz = adapterId_.Length; // Add hash of adapter ID to base hash.
                for(int i = 0; i < sz; i++)
                {   
                    hashValue_ = 5 * hashValue_ + (int)adapterId_[i];
                }
                return hashValue_;
            }
        }

        private string adapterId_;
    }
}
