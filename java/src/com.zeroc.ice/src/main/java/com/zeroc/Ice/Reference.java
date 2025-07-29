// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.time.Duration;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

/**
 * @hidden Public because it's used by the generated code.
 */
public abstract class Reference implements Cloneable {
    public static final int ModeTwoway = 0;
    public static final int ModeOneway = 1;
    public static final int ModeBatchOneway = 2;
    public static final int ModeDatagram = 3;
    public static final int ModeBatchDatagram = 4;
    public static final int ModeLast = ModeBatchDatagram;

    /**
     * Callback interface for asynchronous connection establishment.
     */
    public interface GetConnectionCallback {
        /**
         * Sets the established connection.
         *
         * @param connection the established connection
         * @param compress whether compression is enabled for the connection
         */
        void setConnection(ConnectionI connection, boolean compress);

        /**
         * Sets an exception that occurred during connection establishment.
         *
         * @param ex the exception that occurred
         */
        void setException(LocalException ex);
    }

    /**
     * Returns true if this reference uses a batch mode.
     *
     * @return true if the reference is for batch oneway or batch datagram invocations
     */
    public final boolean isBatch() {
        return _mode == ModeBatchOneway || _mode == ModeBatchDatagram;
    }

    /**
     * Returns true if this reference uses twoway mode.
     *
     * @return true if the reference is for twoway invocations
     */
    public final boolean isTwoway() {
        return _mode == ModeTwoway;
    }

    /**
     * Gets the invocation mode for this reference.
     *
     * @return the invocation mode (twoway, oneway, etc.)
     */
    public final int getMode() {
        return _mode;
    }

    /**
     * Returns true if this reference requires a secure connection.
     *
     * @return true if secure connections are required
     */
    public final boolean getSecure() {
        return _secure;
    }

    /**
     * Gets the Ice protocol version for this reference.
     *
     * @return the protocol version
     */
    public final ProtocolVersion getProtocol() {
        return _protocol;
    }

    /**
     * Gets the Ice encoding version for this reference.
     *
     * @return the encoding version
     */
    public final EncodingVersion getEncoding() {
        return _encoding;
    }

    /**
     * Gets the object identity for this reference.
     *
     * @return the object identity
     */
    public final Identity getIdentity() {
        return _identity;
    }

    /**
     * Gets the facet name for this reference.
     *
     * @return the facet name
     */
    public final String getFacet() {
        return _facet;
    }

    /**
     * Gets the Ice instance associated with this reference.
     *
     * @return the Ice instance
     */
    public final Instance getInstance() {
        return _instance;
    }

    /**
     * Gets the request context for this reference.
     *
     * @return the request context map
     */
    public final Map<String, String> getContext() {
        return _context;
    }

    /**
     * Gets the invocation timeout for this reference.
     *
     * @return the invocation timeout
     */
    public Duration getInvocationTimeout() {
        return _invocationTimeout;
    }

    /**
     * Gets the compression setting for this reference.
     *
     * @return the compression setting, or empty if not specified
     */
    public Optional<Boolean> getCompress() {
        return _compress;
    }

    /**
     * Gets the communicator associated with this reference.
     *
     * @return the communicator
     */
    public final Communicator getCommunicator() {
        return _communicator;
    }

    /**
     * Gets the endpoints for this reference.
     *
     * @return the array of endpoints
     */
    public abstract EndpointI[] getEndpoints();

    /**
     * Gets the adapter ID for this reference.
     *
     * @return the adapter ID
     */
    public abstract String getAdapterId();

    /**
     * Gets the router information for this reference.
     *
     * @return the router information
     */
    public abstract RouterInfo getRouterInfo();

    /**
     * Gets the locator information for this reference.
     *
     * @return the locator information
     */
    public abstract LocatorInfo getLocatorInfo();

    /**
     * Returns true if collocation optimization is enabled.
     *
     * @return true if collocation optimization is enabled
     */
    public abstract boolean getCollocationOptimized();

    /**
     * Returns true if connection caching is enabled.
     *
     * @return true if connection caching is enabled
     */
    public abstract boolean getCacheConnection();

    /**
     * Returns true if secure endpoints are preferred.
     *
     * @return true if secure endpoints are preferred
     */
    public abstract boolean getPreferSecure();

    /**
     * Gets the endpoint selection type for this reference.
     *
     * @return the endpoint selection type
     */
    public abstract EndpointSelectionType getEndpointSelection();

    /**
     * Gets the locator cache timeout for this reference.
     *
     * @return the locator cache timeout
     */
    public abstract Duration getLocatorCacheTimeout();

    /**
     * Gets the connection ID for this reference.
     *
     * @return the connection ID
     */
    public abstract String getConnectionId();

    /**
     * Gets the thread pool for this reference.
     *
     * @return the thread pool
     */
    public abstract ThreadPool getThreadPool();

    /**
     * Gets the connection for this reference.
     *
     * @return the connection
     */
    public abstract ConnectionI getConnection();

    //
    // The change* methods (here and in derived classes) create
    // a new reference based on the existing one, with the
    // corresponding value changed.
    //
    /**
     * Creates a new reference with the specified context.
     *
     * @param newContext the new context map
     * @return a new reference with the updated context
     */
    public final Reference changeContext(Map<String, String> newContext) {
        if (newContext == null) {
            newContext = _emptyContext;
        }
        Reference r = _instance.referenceFactory().copy(this);
        if (newContext.isEmpty()) {
            r._context = _emptyContext;
        } else {
            r._context = new HashMap<>(newContext);
        }
        return r;
    }

    /**
     * Creates a new reference with the specified invocation mode.
     *
     * @param newMode the new invocation mode
     * @return a new reference with the updated mode
     */
    public Reference changeMode(int newMode) {
        Reference r = _instance.referenceFactory().copy(this);
        r._mode = newMode;
        return r;
    }

    /**
     * Creates a new reference with the specified secure setting.
     *
     * @param newSecure the new secure setting
     * @return a new reference with the updated secure setting
     */
    public Reference changeSecure(boolean newSecure) {
        Reference r = _instance.referenceFactory().copy(this);
        r._secure = newSecure;
        return r;
    }

    /**
     * Creates a new reference with the specified identity.
     *
     * @param newIdentity the new object identity
     * @return a new reference with the updated identity
     */
    public final Reference changeIdentity(Identity newIdentity) {
        Reference r = _instance.referenceFactory().copy(this);
        r._identity = newIdentity.clone();
        return r;
    }

    /**
     * Creates a new reference with the specified facet.
     *
     * @param newFacet the new facet name
     * @return a new reference with the updated facet
     */
    public final Reference changeFacet(String newFacet) {
        Reference r = _instance.referenceFactory().copy(this);
        r._facet = newFacet;
        return r;
    }

    /**
     * Creates a new reference with the specified invocation timeout.
     *
     * @param newTimeout the new invocation timeout
     * @return a new reference with the updated timeout
     */
    public final Reference changeInvocationTimeout(Duration newTimeout) {
        Reference r = _instance.referenceFactory().copy(this);
        r._invocationTimeout = newTimeout;
        return r;
    }

    /**
     * Creates a new reference with the specified encoding version.
     *
     * @param newEncoding the new encoding version
     * @return a new reference with the updated encoding
     */
    public Reference changeEncoding(EncodingVersion newEncoding) {
        Reference r = _instance.referenceFactory().copy(this);
        r._encoding = newEncoding;
        return r;
    }

    /**
     * Creates a new reference with the specified compression setting.
     *
     * @param newCompress the new compression setting
     * @return a new reference with the updated compression setting
     */
    public Reference changeCompress(boolean newCompress) {
        Reference r = _instance.referenceFactory().copy(this);
        r._compress = Optional.of(newCompress);
        return r;
    }

    /**
     * Creates a new reference with the specified adapter ID.
     *
     * @param newAdapterId the new adapter ID
     * @return a new reference with the updated adapter ID
     */
    public abstract Reference changeAdapterId(String newAdapterId);

    /**
     * Creates a new reference with the specified endpoints.
     *
     * @param newEndpoints the new endpoints
     * @return a new reference with the updated endpoints
     */
    public abstract Reference changeEndpoints(EndpointI[] newEndpoints);

    /**
     * Creates a new reference with the specified locator.
     *
     * @param newLocator the new locator proxy
     * @return a new reference with the updated locator
     */
    public abstract Reference changeLocator(LocatorPrx newLocator);

    /**
     * Creates a new reference with the specified router.
     *
     * @param newRouter the new router proxy
     * @return a new reference with the updated router
     */
    public abstract Reference changeRouter(RouterPrx newRouter);

    /**
     * Creates a new reference with the specified collocation optimization setting.
     *
     * @param newCollocationOptimized the new collocation optimization setting
     * @return a new reference with the updated setting
     */
    public abstract Reference changeCollocationOptimized(boolean newCollocationOptimized);

    /**
     * Creates a new reference with the specified connection caching setting.
     *
     * @param newCache the new connection caching setting
     * @return a new reference with the updated setting
     */
    public abstract Reference changeCacheConnection(boolean newCache);

    /**
     * Creates a new reference with the specified prefer secure setting.
     *
     * @param newPreferSecure the new prefer secure setting
     * @return a new reference with the updated setting
     */
    public abstract Reference changePreferSecure(boolean newPreferSecure);

    /**
     * Creates a new reference with the specified endpoint selection type.
     *
     * @param newType the new endpoint selection type
     * @return a new reference with the updated selection type
     */
    public abstract Reference changeEndpointSelection(EndpointSelectionType newType);

    /**
     * Creates a new reference with the specified locator cache timeout.
     *
     * @param newTimeout the new locator cache timeout
     * @return a new reference with the updated timeout
     */
    public abstract Reference changeLocatorCacheTimeout(Duration newTimeout);

    /**
     * Creates a new reference with the specified connection ID.
     *
     * @param connectionId the new connection ID
     * @return a new reference with the updated connection ID
     */
    public abstract Reference changeConnectionId(String connectionId);

    /**
     * Creates a new reference with the specified connection.
     *
     * @param connection the new connection
     * @return a new reference with the updated connection
     */
    public abstract Reference changeConnection(ConnectionI connection);

    @Override
    public int hashCode() {
        int h = 5381;
        h = HashUtil.hashAdd(h, _mode);
        h = HashUtil.hashAdd(h, _secure);
        h = HashUtil.hashAdd(h, _identity);
        h = HashUtil.hashAdd(h, _context);
        h = HashUtil.hashAdd(h, _facet);
        if (_compress.isPresent()) {
            h = HashUtil.hashAdd(h, _compress.get());
        }
        // We don't include protocol and encoding in the hash; they are using 1.0 and 1.1,
        // respectively.
        h = HashUtil.hashAdd(h, _invocationTimeout.toMillis());

        return h;
    }

    /**
     * Gets the effective compression setting, taking into account the override.
     *
     * @return the effective compression setting
     */
    public Optional<Boolean> getCompressOverride() {
        DefaultsAndOverrides defaultsAndOverrides = getInstance().defaultsAndOverrides();

        return defaultsAndOverrides.overrideCompress.isPresent()
            ? defaultsAndOverrides.overrideCompress
            : _compress;
    }

    //
    // Utility methods
    //
    /**
     * Returns true if this is an indirect reference.
     *
     * @return true if this is an indirect reference
     */
    public abstract boolean isIndirect();

    /**
     * Returns true if this is a well-known reference.
     *
     * @return true if this is a well-known reference
     */
    public abstract boolean isWellKnown();

    //
    // Marshal the reference.
    //
    /**
     * Writes this reference to an output stream.
     *
     * @param s the output stream to write to
     */
    public void streamWrite(OutputStream s) {
        //
        // Don't write the identity here. Operations calling streamWrite
        // write the identity.
        //

        //
        // For compatibility with the old FacetPath.
        //
        if (_facet.isEmpty()) {
            s.writeStringSeq(null);
        } else {
            String[] facetPath = {_facet};
            s.writeStringSeq(facetPath);
        }

        s.writeByte((byte) _mode);

        s.writeBool(_secure);

        if (!s.getEncoding().equals(Util.Encoding_1_0)) {
            _protocol.ice_writeMembers(s);
            _encoding.ice_writeMembers(s);
        }

        // Derived class writes the remainder of the reference.
    }

    //
    // Convert the reference to its string form.
    //
    @Override
    public String toString() {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        StringBuilder s = new StringBuilder(128);

        ToStringMode toStringMode = _instance.toStringMode();
        final String separators = " :@";

        String id = Util.identityToString(_identity, toStringMode);

        //
        // If the encoded identity string contains characters which
        // the reference parser uses as separators, then we enclose
        // the identity string in quotes.
        //

        if (StringUtil.findFirstOf(id, separators) != -1) {
            s.append('"');
            s.append(id);
            s.append('"');
        } else {
            s.append(id);
        }

        if (!_facet.isEmpty()) {
            //
            // If the encoded facet string contains characters which
            // the reference parser uses as separators, then we enclose
            // the facet string in quotes.
            //
            s.append(" -f ");
            String fs = StringUtil.escapeString(_facet, "", toStringMode);
            if (StringUtil.findFirstOf(fs, separators) != -1) {
                s.append('"');
                s.append(fs);
                s.append('"');
            } else {
                s.append(fs);
            }
        }

        switch (_mode) {
            case ModeTwoway -> {} // Don't print the default mode.
            case ModeOneway -> s.append(" -o");
            case ModeBatchOneway -> s.append(" -O");
            case ModeDatagram -> s.append(" -d");
            case ModeBatchDatagram -> s.append(" -D");
        }

        if (_secure) {
            s.append(" -s");
        }

        if (!_protocol.equals(Util.Protocol_1_0)) {
            // We print the protocol unless it's 1.0.
            s.append(" -p ");
            s.append(Util.protocolVersionToString(_protocol));
        }

        // We print the encoding if it's not 1.1 or if Ice.Default.EncodingVersion is set to
        // something other than 1.1.
        if (!_encoding.equals(Util.Encoding_1_1)
            || !_instance.defaultsAndOverrides().defaultEncoding.equals(Util.Encoding_1_1)) {
            s.append(" -e ");
            s.append(Util.encodingVersionToString(_encoding));
        }

        return s.toString();

        // Derived class writes the remainder of the string.
    }

    //
    // Convert the reference to its property form.
    //
    /**
     * Converts this reference to its property form.
     *
     * @param prefix the property prefix to use
     * @return a map of properties representing this reference
     */
    public abstract Map<String, String> toProperty(String prefix);

    /**
     * Gets the request handler for this reference.
     *
     * @return the request handler
     */
    abstract RequestHandler getRequestHandler();

    /**
     * Gets the batch request queue for this reference.
     *
     * @return the batch request queue
     */
    abstract BatchRequestQueue getBatchRequestQueue();

    @Override
    public boolean equals(java.lang.Object obj) {
        //
        // Note: if(this == obj) and type test are performed by each non-abstract derived class.
        //

        Reference r = (Reference) obj; // Guaranteed to succeed.

        if (_mode != r._mode) {
            return false;
        }

        if (_secure != r._secure) {
            return false;
        }

        if (!_identity.equals(r._identity)) {
            return false;
        }

        if (!_context.equals(r._context)) {
            return false;
        }

        if (!_facet.equals(r._facet)) {
            return false;
        }

        if (!_compress.equals(r._compress)) {
            return false;
        }

        if (!_protocol.equals(r._protocol)) {
            return false;
        }

        if (!_encoding.equals(r._encoding)) {
            return false;
        }

        if (!_invocationTimeout.equals(r._invocationTimeout)) {
            return false;
        }

        return true;
    }

    @Override
    public Reference clone() {
        Reference c = null;
        try {
            c = (Reference) super.clone();
        } catch (CloneNotSupportedException ex) {
            assert false;
        }
        return c;
    }

    private static final Map<String, String> _emptyContext = new HashMap<>();

    private final Instance _instance;
    private final Communicator _communicator;

    private int _mode;
    private boolean _secure;
    private Optional<Boolean> _compress;
    private Identity _identity;
    private Map<String, String> _context;
    private String _facet;
    private final ProtocolVersion _protocol;
    private EncodingVersion _encoding;
    private Duration _invocationTimeout;

    /**
     * Constructs a new Reference with the specified parameters.
     *
     * @param instance the Ice instance
     * @param communicator the communicator
     * @param identity the object identity
     * @param facet the facet name
     * @param mode the invocation mode
     * @param secure whether secure connections are required
     * @param compress the compression setting
     * @param protocol the protocol version
     * @param encoding the encoding version
     * @param invocationTimeout the invocation timeout
     * @param context the request context
     */
    protected Reference(
            Instance instance,
            Communicator communicator,
            Identity identity,
            String facet,
            int mode,
            boolean secure,
            Optional<Boolean> compress,
            ProtocolVersion protocol,
            EncodingVersion encoding,
            Duration invocationTimeout,
            Map<String, String> context) {
        //
        // Validate string arguments.
        //
        assert (identity.name != null);
        assert (identity.category != null);
        assert (facet != null);

        _instance = instance;
        _communicator = communicator;
        _mode = mode;
        _secure = secure;
        _compress = compress;
        _identity = identity;
        _context = context != null ? new HashMap<>(context) : _emptyContext;
        _facet = facet;
        _protocol = protocol;
        _encoding = encoding;
        _invocationTimeout = invocationTimeout;
    }
}
