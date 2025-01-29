// Copyright (c) ZeroC, Inc.

#include "SchannelTransceiverI.h"
#include "Ice/Buffer.h"
#include "Ice/Communicator.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"
#include "Ice/SSL/ConnectionInfo.h"
#include "Ice/StringUtil.h"
#include "SSLInstance.h"
#include "SSLUtil.h"
#include "SchannelEngine.h"

#include <sstream>

using namespace std;
using namespace Ice;
using namespace Ice::SSL;

namespace
{
    string tlsProtocolVersionToString(DWORD protocol)
    {
        switch (protocol)
        {
            // TLS 1.1 (RFC 4346) => 0x03,0x02
            case 0x302:
                return "TLSv1.1";

            // TLS 1.2 (RFC 5246) => 0x03,0x03
            case 0x303:
                return "TLSv1.2";

            // TLS 1.3 (RFC 8446) => 0x03,0x04
            case 0x304:
                return "TLSv1.3";

            default:
            {
                ostringstream os;
                os << "unknown: 0x" << setfill('0') << setw(2) << std::hex << protocol;
                return os.str();
            }
        }
    }

    SecBuffer* getSecBufferWithType(const SecBufferDesc& desc, ULONG bufferType)
    {
        for (ULONG i = 0; i < desc.cBuffers; ++i)
        {
            if (desc.pBuffers[i].BufferType == bufferType)
            {
                return &desc.pBuffers[i];
            }
        }
        return nullptr;
    }

    HCERTCHAINENGINE defaultChainEngine(HCERTSTORE trustedRootCertificates)
    {
        if (trustedRootCertificates)
        {
            // Create a chain engine that uses our Trusted Root Store
            CERT_CHAIN_ENGINE_CONFIG config{
                .cbSize = sizeof(CERT_CHAIN_ENGINE_CONFIG),
                .hExclusiveRoot = trustedRootCertificates};

            HCERTCHAINENGINE chainEngine = nullptr;
            if (!CertCreateCertificateChainEngine(&config, &chainEngine))
            {
                throw InitializationException(
                    __FILE__,
                    __LINE__,
                    "SSL transport: error creating certificate chain engine:\n" + IceInternal::lastErrorToString());
            }
            return chainEngine;
        }
        else
        {
            return HCCE_CURRENT_USER;
        }
    }

    SCH_CREDENTIALS defaultOutgoingCredentials(const string&)
    {
        return SCH_CREDENTIALS{
            .dwVersion = SCH_CREDENTIALS_VERSION,
            .dwFlags = SCH_CRED_NO_DEFAULT_CREDS | SCH_CRED_NO_SERVERNAME_CHECK | SCH_USE_STRONG_CRYPTO};
    }

    SCH_CREDENTIALS defaultIncomingCredentials(const string&)
    {
        return SCH_CREDENTIALS{
            .dwVersion = SCH_CREDENTIALS_VERSION,
            // Don't set SCH_SEND_ROOT_CERT as it seems to cause problems with Java certificate validation and
            // Schannel doesn't seems to send the root certificate either way.
            .dwFlags = SCH_CRED_NO_SYSTEM_MAPPER | SCH_USE_STRONG_CRYPTO};
    }
}

IceInternal::NativeInfoPtr
Schannel::TransceiverI::getNativeInfo()
{
    return _delegate->getNativeInfo();
}

IceInternal::SocketOperation
Schannel::TransceiverI::sslHandshake(SecBuffer* initialBuffer)
{
    DWORD flags = 0;
    if (_incoming)
    {
        flags |= ASC_REQ_SEQUENCE_DETECT | ASC_REQ_REPLAY_DETECT | ASC_REQ_CONFIDENTIALITY | ASC_REQ_ALLOCATE_MEMORY |
                 ASC_REQ_STREAM | ASC_REQ_EXTENDED_ERROR;
    }
    else
    {
        flags |= ISC_REQ_SEQUENCE_DETECT | ISC_REQ_REPLAY_DETECT | ISC_REQ_CONFIDENTIALITY | ISC_REQ_ALLOCATE_MEMORY |
                 ISC_REQ_STREAM | ISC_REQ_USE_SUPPLIED_CREDS | ISC_RET_EXTENDED_ERROR;
    }

    SECURITY_STATUS err = SEC_E_OK;
    if (_state == StateHandshakeNotStarted)
    {
        assert(!initialBuffer); // Always null for the initial handshake.
        _credentials = _localCredentialsSelectionCallback(_incoming ? _adapterName : _host);
        if (_rootStore && !_credentials.hRootStore)
        {
            _credentials.hRootStore = CertDuplicateStore(_rootStore);
        }

        if (_credentials.cCreds > 0)
        {
            for (DWORD i = 0; i < _credentials.cCreds; ++i)
            {
                if (!_credentials.paCred[i])
                {
                    throw SecurityException(
                        __FILE__,
                        __LINE__,
                        "SSL transport: invalid null certificate in the provided SCH_CREDENTIALS.");
                }
                _allCerts.push_back(CertDuplicateCertificateContext(_credentials.paCred[i]));
            }
            _credentials.paCred = &_allCerts[0];
        }

        _credentials.hRootStore = _rootStore;

        err = AcquireCredentialsHandle(
            0,
            const_cast<char*>(UNISP_NAME),
            (_incoming ? SECPKG_CRED_INBOUND : SECPKG_CRED_OUTBOUND),
            0,
            &_credentials,
            0,
            0,
            &_credentialsHandle,
            0);

        if (err != SEC_E_OK)
        {
            throw SecurityException(
                __FILE__,
                __LINE__,
                "SSL transport: failed to acquire credentials handle:\n" + IceInternal::lastErrorToString());
        }

        _ctxFlags = 0;
        _readBuffer.b.resize(2048);
        _readBuffer.i = _readBuffer.b.begin();

        if (_incoming)
        {
            _state = StateHandshakeReadContinue;
        }
        else
        {
            SecBuffer outBuffer = {0, SECBUFFER_TOKEN, 0};
            SecBufferDesc outBufferDesc = {SECBUFFER_VERSION, 1, &outBuffer};

            err = InitializeSecurityContext(
                &_credentialsHandle,
                0,
                const_cast<char*>(_host.c_str()),
                _remoteCertificateValidationCallback ? flags | ISC_REQ_MANUAL_CRED_VALIDATION : flags,
                0,
                0,
                0,
                0,
                &_ssl,
                &outBufferDesc,
                &_ctxFlags,
                0);
            if (err != SEC_E_OK && err != SEC_I_CONTINUE_NEEDED)
            {
                ostringstream os;
                os << "SSL transport: handshake failure:\n" << IceInternal::errorToString(err);
                throw SecurityException(__FILE__, __LINE__, os.str());
            }

            if (_sslNewSessionCallback)
            {
                _sslNewSessionCallback(_ssl, _host);
            }

            //
            // Copy the data to the write buffer
            //
            _writeBuffer.b.resize(outBuffer.cbBuffer);
            _writeBuffer.i = _writeBuffer.b.begin();
            memcpy(_writeBuffer.i, outBuffer.pvBuffer, outBuffer.cbBuffer);
            FreeContextBuffer(outBuffer.pvBuffer);

            _state = StateHandshakeWriteContinue;
        }
    }

    while (true)
    {
        if (_state == StateHandshakeReadContinue)
        {
            SecBuffer inBuffers[2]{{0, SECBUFFER_EMPTY, 0}, {0, SECBUFFER_EMPTY, 0}};
            SecBufferDesc inBufferDesc{SECBUFFER_VERSION, 2, inBuffers};

            if (initialBuffer)
            {
                inBuffers[0] = {initialBuffer->cbBuffer, SECBUFFER_TOKEN, initialBuffer->pvBuffer};
                initialBuffer = nullptr;
            }
            else
            {
                // If read buffer is empty, try to read some data.
                if (_readBuffer.i == _readBuffer.b.begin() && !readRaw(_readBuffer))
                {
                    return IceInternal::SocketOperationRead;
                }

                inBuffers[0] = {
                    static_cast<DWORD>(_readBuffer.i - _readBuffer.b.begin()),
                    SECBUFFER_TOKEN,
                    _readBuffer.b.begin()};
            }

            SecBuffer outBuffers[2]{{0, SECBUFFER_TOKEN, 0}, {0, SECBUFFER_ALERT, 0}};
            SecBufferDesc outBufferDesc{SECBUFFER_VERSION, 2, outBuffers};

            if (_incoming)
            {
                bool newSession = _ssl.dwLower == 0 && _ssl.dwUpper == 0;
                err = AcceptSecurityContext(
                    &_credentialsHandle,
                    newSession ? 0 : &_ssl,
                    &inBufferDesc,
                    _clientCertificateRequired ? flags | ASC_REQ_MUTUAL_AUTH : flags,
                    0,
                    &_ssl,
                    &outBufferDesc,
                    &_ctxFlags,
                    0);

                if (newSession && _sslNewSessionCallback)
                {
                    _sslNewSessionCallback(_ssl, _host);
                }
            }
            else
            {
                err = InitializeSecurityContext(
                    &_credentialsHandle,
                    &_ssl,
                    const_cast<char*>(_host.c_str()),
                    flags,
                    0,
                    0,
                    &inBufferDesc,
                    0,
                    0,
                    &outBufferDesc,
                    &_ctxFlags,
                    0);
            }

            // If the message is incomplete we need to read more data.
            if (err == SEC_E_INCOMPLETE_MESSAGE)
            {
                SecBuffer* missing = getSecBufferWithType(inBufferDesc, SECBUFFER_MISSING);
                size_t pos = _readBuffer.i - _readBuffer.b.begin();
                _readBuffer.b.resize((missing && missing->cbBuffer > 0) ? (pos + missing->cbBuffer) : (pos * 2));
                _readBuffer.i = _readBuffer.b.begin() + pos;
                return IceInternal::SocketOperationRead;
            }
            else if (err != SEC_I_CONTINUE_NEEDED && err != SEC_E_OK)
            {
                ostringstream os;
                os << "SSL handshake failure:\n" << IceInternal::errorToString(err);
                throw SecurityException(__FILE__, __LINE__, os.str());
            }

            // Copy out security tokens to the write buffer if any.
            SecBuffer* token = getSecBufferWithType(outBufferDesc, SECBUFFER_TOKEN);
            assert(token);
            if (token->cbBuffer)
            {
                _writeBuffer.b.resize(static_cast<size_t>(token->cbBuffer));
                _writeBuffer.i = _writeBuffer.b.begin();
                memcpy(_writeBuffer.i, token->pvBuffer, token->cbBuffer);
                FreeContextBuffer(token->pvBuffer);
            }

            // Check for remaining data in the input buffer.
            SecBuffer* extra = getSecBufferWithType(inBufferDesc, SECBUFFER_EXTRA);
            if (extra)
            {
                // Shift the extra data to the start of the input buffer
                memmove(_readBuffer.b.begin(), _readBuffer.i - extra->cbBuffer, extra->cbBuffer);
                _readBuffer.i = _readBuffer.b.begin() + extra->cbBuffer;
            }
            else
            {
                _readBuffer.i = _readBuffer.b.begin();
            }

            if (token->cbBuffer)
            {
                if (err == SEC_E_OK)
                {
                    _state = StateHandshakeWriteNoContinue; // Write and don't continue.
                }
                else
                {
                    _state = StateHandshakeWriteContinue; // Continue writing if we have a token.
                }
            }
            else if (err == SEC_E_OK)
            {
                break; // We're done.
            }

            // Otherwise continue either reading credentials
        }

        if (_state == StateHandshakeWriteContinue || _state == StateHandshakeWriteNoContinue)
        {
            //
            // Write any pending data.
            //
            if (!writeRaw(_writeBuffer))
            {
                return IceInternal::SocketOperationWrite;
            }
            if (_state == StateHandshakeWriteNoContinue)
            {
                break; // Token is written and we weren't told to continue, so we're done!
            }
            _state = StateHandshakeReadContinue;
        }
    }

    //
    // Check if the requested capabilities are met.
    //
    // NOTE: it's important for _ctxFlags to be a data member. The context flags might not be checked immediately
    // if the last write can't complete without blocking above. In such a case, the context flags are checked here
    // only once the sslHandshake is called again after the write completes.
    //
    if (flags != _ctxFlags)
    {
        ostringstream os;
        if (_incoming)
        {
            if (!(_ctxFlags & ASC_REQ_SEQUENCE_DETECT))
            {
                os << "\n - SSL transport: Schannel failed to setup sequence detect";
            }

            if (!(_ctxFlags & ASC_REQ_REPLAY_DETECT))
            {
                os << "\n - SSL transport: Schannel failed to setup replay detect";
            }

            if (!(_ctxFlags & ASC_REQ_CONFIDENTIALITY))
            {
                os << "\n - SSL transport: Schannel failed to setup confidentiality";
            }

            if (!(_ctxFlags & ASC_REQ_EXTENDED_ERROR))
            {
                os << "\n - SSL transport: Schannel failed to setup extended error";
            }

            if (!(_ctxFlags & ASC_REQ_ALLOCATE_MEMORY))
            {
                os << "\n - SSL transport: Schannel failed to setup memory allocation";
            }

            if (!(_ctxFlags & ASC_REQ_STREAM))
            {
                os << "\n - SSL transport: Schannel failed to setup stream";
            }
        }
        else
        {
            if (!(_ctxFlags & ISC_REQ_SEQUENCE_DETECT))
            {
                os << "\n - SSL transport: Schannel failed to setup sequence detect";
            }

            if (!(_ctxFlags & ISC_REQ_REPLAY_DETECT))
            {
                os << "\n - SSL transport: Schannel failed to setup replay detect";
            }

            if (!(_ctxFlags & ISC_REQ_CONFIDENTIALITY))
            {
                os << "\n - SSL transport: Schannel failed to setup confidentiality";
            }

            if (!(_ctxFlags & ISC_REQ_EXTENDED_ERROR))
            {
                os << "\n - SSL transport: Schannel failed to setup extended error";
            }

            if (!(_ctxFlags & ISC_REQ_ALLOCATE_MEMORY))
            {
                os << "\n - SSL transport: Schannel failed to setup memory allocation";
            }

            if (!(_ctxFlags & ISC_REQ_STREAM))
            {
                os << "\n - SSL transport: Schannel failed to setup stream";
            }
        }
        throw SecurityException(__FILE__, __LINE__, os.str());
    }

    err = QueryContextAttributes(&_ssl, SECPKG_ATTR_STREAM_SIZES, &_sizes);
    if (err != SEC_E_OK)
    {
        ostringstream os;
        os << "SSL transport: failure to query stream sizes attributes:\n" << IceInternal::errorToString(err);
        throw SecurityException(__FILE__, __LINE__, os.str());
    }

    assert(!_peerCertificate || _sslConnectionRenegotiating);
    if (_peerCertificate)
    {
        CertFreeCertificateContext(_peerCertificate);
        _peerCertificate = nullptr;
    }

    err = QueryContextAttributes(&_ssl, SECPKG_ATTR_REMOTE_CERT_CONTEXT, &_peerCertificate);
    if (err != SEC_E_OK && err != SEC_E_NO_CREDENTIALS)
    {
        ostringstream os;
        os << "SSL transport: failure to query remote certificate context:\n" << IceInternal::errorToString(err);
        throw SecurityException(__FILE__, __LINE__, os.str());
    }

    size_t pos = _readBuffer.i - _readBuffer.b.begin();
    if (pos <= (_sizes.cbHeader + _sizes.cbMaximumMessage + _sizes.cbTrailer))
    {
        _readBuffer.b.resize(_sizes.cbHeader + _sizes.cbMaximumMessage + _sizes.cbTrailer);
        _readBuffer.i = _readBuffer.b.begin() + pos;
    }

    _writeBuffer.b.reset();
    _writeBuffer.i = _writeBuffer.b.begin();

    if (_remoteCertificateValidationCallback &&
        !_remoteCertificateValidationCallback(
            _ssl,
            dynamic_pointer_cast<Ice::SSL::ConnectionInfo>(getInfo(_incoming, _adapterName, ""))))
    {
        throw SecurityException(
            __FILE__,
            __LINE__,
            "IceSSL: certificate verification failed. The certificate was rejected by the remote validation callback.");
    }

    if (_engine->securityTraceLevel() >= 1)
    {
        Trace out(_instance->logger(), _engine->securityTraceCategory());
        out << "SSL summary for " << (_incoming ? "incoming" : "outgoing") << " connection\n";

        SecPkgContext_CipherInfo cipherInfo{};
        err = QueryContextAttributes(&_ssl, SECPKG_ATTR_CIPHER_INFO, &cipherInfo);
        if (err != SEC_E_OK)
        {
            out << "unknown cipher\n";
        }
        else
        {
            out << "cipher = " << wstringToString(wstring{cipherInfo.szCipherSuite}, getProcessStringConverter())
                << "\n";
            out << "bits = " << cipherInfo.dwCipherLen << "\n";
            out << "protocol = " << tlsProtocolVersionToString(cipherInfo.dwProtocol) << "\n";
        }
        out << toString();
    }

    _state = StateHandshakeComplete;

    _delegate->getNativeInfo()->ready(
        IceInternal::SocketOperationRead,
        !_readUnprocessed.b.empty() || _readBuffer.i != _readBuffer.b.begin());

    return IceInternal::SocketOperationNone;
}

// Try to decrypt a message and return the number of bytes decrypted, if the number of bytes decrypted is less than the
// size requested it means that the application needs to read more data before it can decrypt the complete message.
size_t
Schannel::TransceiverI::decryptMessage(IceInternal::Buffer& buffer)
{
    assert(_readBuffer.i != _readBuffer.b.begin() || !_readUnprocessed.b.empty());

    std::byte* i = buffer.i;

    // The number of bytes of the current message that are already decrypted.
    size_t length = std::min(static_cast<size_t>(buffer.b.end() - buffer.i), _readUnprocessed.b.size());
    if (length > 0)
    {
        memcpy(buffer.i, _readUnprocessed.b.begin(), length);
        i += length;
        // Move any remaining data to the beginning of the unprocessed buffer.
        size_t remaining = _readUnprocessed.b.size() - length;
        memmove(_readUnprocessed.b.begin(), _readUnprocessed.b.begin() + length, remaining);
        _readUnprocessed.b.resize(remaining);
    }

    while (i != buffer.b.end() && _readBuffer.i != _readBuffer.b.begin())
    {
        // Try to decrypt the buffered data.
        SecBuffer inBuffers[4]{
            {static_cast<DWORD>(_readBuffer.i - _readBuffer.b.begin()), SECBUFFER_DATA, _readBuffer.b.begin()},
            {0, SECBUFFER_EMPTY, 0},
            {0, SECBUFFER_EMPTY, 0},
            {0, SECBUFFER_EMPTY, 0}};
        SecBufferDesc inBufferDesc{SECBUFFER_VERSION, 4, inBuffers};

        SECURITY_STATUS err = DecryptMessage(&_ssl, &inBufferDesc, 0, nullptr);
        if (err == SEC_E_INCOMPLETE_MESSAGE)
        {
            // There isn't enough data to decrypt the message. The input buffer is resized to the SSL max message size
            // after the SSL handshake completes so an incomplete message can only occur if the read buffer is not full.
            assert(_readBuffer.i != _readBuffer.b.end());
            return i - buffer.i;
        }
        else if (err == SEC_I_RENEGOTIATE)
        {
            if (_sslConnectionRenegotiating)
            {
                throw ProtocolException(
                    __FILE__,
                    __LINE__,
                    "SSL transport: peer requested renegotiation while we are already renegotiating.");
            }
            // The peer has requested a renegotiation.
            SecBuffer* extraBuffer = getSecBufferWithType(inBufferDesc, SECBUFFER_EXTRA);
            _sslConnectionRenegotiating = true;
            _state = StateHandshakeReadContinue;
            if (extraBuffer)
            {
                _extraBuffer.b.resize(extraBuffer->cbBuffer);
                _extraBuffer.i = _extraBuffer.b.begin();
                memcpy(_extraBuffer.i, extraBuffer->pvBuffer, extraBuffer->cbBuffer);
                _extraBuffer.i += extraBuffer->cbBuffer;
            }
            return 0;
        }
        else if (err == SEC_I_CONTEXT_EXPIRED)
        {
            // The message sender has finished using the connection and has initiated a shutdown.
            throw ConnectionLostException(__FILE__, __LINE__, 0);
        }
        else if (err != SEC_E_OK)
        {
            throw ProtocolException(
                __FILE__,
                __LINE__,
                "SSL transport: protocol error during read:\n" + IceInternal::errorToString(err));
        }

        SecBuffer* dataBuffer = getSecBufferWithType(inBufferDesc, SECBUFFER_DATA);
        assert(dataBuffer);
        DWORD remaining = min(static_cast<DWORD>(buffer.b.end() - i), dataBuffer->cbBuffer);
        if (remaining > 0)
        {
            memcpy(i, dataBuffer->pvBuffer, remaining);
            i += remaining;

            // Copy remaining decrypted data to unprocessed buffer.
            if (dataBuffer->cbBuffer > remaining)
            {
                _readUnprocessed.b.resize(dataBuffer->cbBuffer - remaining);
                memcpy(
                    _readUnprocessed.b.begin(),
                    reinterpret_cast<uint8_t*>(dataBuffer->pvBuffer) + remaining,
                    dataBuffer->cbBuffer - remaining);
            }
        }

        // Move any remaining encrypted data to the beginning of the input buffer.
        SecBuffer* extraBuffer = getSecBufferWithType(inBufferDesc, SECBUFFER_EXTRA);
        if (extraBuffer && extraBuffer->cbBuffer > 0)
        {
            memmove(_readBuffer.b.begin(), _readBuffer.i - extraBuffer->cbBuffer, extraBuffer->cbBuffer);
            _readBuffer.i = _readBuffer.b.begin() + extraBuffer->cbBuffer;
        }
        else
        {
            _readBuffer.i = _readBuffer.b.begin();
        }
    }
    return i - buffer.i;
}

//
// Encrypt a message and return the number of bytes that has been encrypted, if the
// number of bytes is less than the message size, the function must be called again.
//
size_t
Schannel::TransceiverI::encryptMessage(IceInternal::Buffer& buffer)
{
    //
    // Limit the message size to cbMaximumMessage which is the maximum size data that can be
    // embedded in a SSL record.
    //
    DWORD length = std::min(static_cast<DWORD>(buffer.b.end() - buffer.i), _sizes.cbMaximumMessage);

    //
    // Resize the buffer to hold the encrypted data
    //
    _writeBuffer.b.resize(_sizes.cbHeader + length + _sizes.cbTrailer);
    _writeBuffer.i = _writeBuffer.b.begin();

    SecBuffer buffers[4]{
        {_sizes.cbHeader, SECBUFFER_STREAM_HEADER, _writeBuffer.i},
        {length, SECBUFFER_DATA, _writeBuffer.i + _sizes.cbHeader},
        {_sizes.cbTrailer, SECBUFFER_STREAM_TRAILER, _writeBuffer.i + _sizes.cbHeader + length},
        {0, SECBUFFER_EMPTY, 0}};
    SecBufferDesc buffersDesc = {SECBUFFER_VERSION, 4, buffers};

    // Data is encrypted in place, copy the data to be encrypted to the data buffer.
    memcpy(buffers[1].pvBuffer, buffer.i, length);

    SECURITY_STATUS err = EncryptMessage(&_ssl, 0, &buffersDesc, 0);
    if (err != SEC_E_OK)
    {
        throw ProtocolException(
            __FILE__,
            __LINE__,
            "SSL transport: protocol error encrypting message:\n" + IceInternal::errorToString(err));
    }

    // EncryptMessage resizes the buffers, so resize the write buffer as well to reflect this.
    _writeBuffer.b.resize(buffers[0].cbBuffer + buffers[1].cbBuffer + buffers[2].cbBuffer);
    _writeBuffer.i = _writeBuffer.b.begin();

    return length;
}

IceInternal::SocketOperation
Schannel::TransceiverI::initialize(IceInternal::Buffer& readBuffer, IceInternal::Buffer& writeBuffer)
{
    if (_state == StateNotInitialized)
    {
        IceInternal::SocketOperation op = _delegate->initialize(readBuffer, writeBuffer);
        if (op != IceInternal::SocketOperationNone)
        {
            return op;
        }
        _state = StateHandshakeNotStarted;
    }
    return sslHandshake();
}

IceInternal::SocketOperation
Schannel::TransceiverI::closing(bool initiator, exception_ptr)
{
    // If we are initiating the connection closure, wait for the peer
    // to close the TCP/IP connection. Otherwise, close immediately.
    return initiator ? IceInternal::SocketOperationRead : IceInternal::SocketOperationNone;
}

void
Schannel::TransceiverI::close()
{
    if (_ssl.dwLower || _ssl.dwUpper)
    {
        DeleteSecurityContext(&_ssl);
        _ssl = {};
    }

    if (_credentialsHandle.dwLower || _credentialsHandle.dwUpper)
    {
        FreeCredentialsHandle(&_credentialsHandle);
        _credentialsHandle = {};
    }

    if (_credentials.hRootStore)
    {
        CertCloseStore(_credentials.hRootStore, 0);
        _credentials.hRootStore = nullptr;
    }

    for (const auto& certificate : _allCerts)
    {
        CertFreeCertificateContext(certificate);
    }
    _credentials.cCreds = 0;
    _credentials.paCred = nullptr;
    _allCerts.clear();

    if (_peerCertificate)
    {
        CertFreeCertificateContext(_peerCertificate);
        _peerCertificate = nullptr;
    }

    if (_chainEngine && _chainEngine != HCCE_CURRENT_USER)
    {
        CertFreeCertificateChainEngine(_chainEngine);
        _chainEngine = nullptr;
    }

    _delegate->close();

    // Clear the buffers now instead of waiting for destruction.
    _writeBuffer.b.clear();
    _readBuffer.b.clear();
    _readUnprocessed.b.clear();
}

IceInternal::SocketOperation
Schannel::TransceiverI::write(IceInternal::Buffer& buf)
{
    if (_state == StateNotInitialized)
    {
        return _delegate->write(buf);
    }

    if (buf.i == buf.b.end())
    {
        return IceInternal::SocketOperationNone;
    }
    assert(_state == StateHandshakeComplete);

    while (buf.i != buf.b.end())
    {
        if (_bufferedW == 0)
        {
            assert(_writeBuffer.i == _writeBuffer.b.end());
            _bufferedW = encryptMessage(buf);
        }

        if (!writeRaw(_writeBuffer))
        {
            return IceInternal::SocketOperationWrite;
        }

        assert(_writeBuffer.i == _writeBuffer.b.end()); // Finished writing the encrypted data

        buf.i += _bufferedW;
        _bufferedW = 0;
    }
    return IceInternal::SocketOperationNone;
}

IceInternal::SocketOperation
Schannel::TransceiverI::read(IceInternal::Buffer& buf)
{
    if (_state == StateNotInitialized)
    {
        return _delegate->read(buf);
    }

    if (buf.i == buf.b.end())
    {
        return IceInternal::SocketOperationNone;
    }
    assert(_state == StateHandshakeComplete);

    _delegate->getNativeInfo()->ready(IceInternal::SocketOperationRead, false);
    while (buf.i != buf.b.end())
    {
        if (_readUnprocessed.b.empty() && _readBuffer.i == _readBuffer.b.begin() && !readRaw(_readBuffer))
        {
            return IceInternal::SocketOperationRead;
        }

        size_t decrypted = decryptMessage(buf);
        if (_sslConnectionRenegotiating)
        {
            // The peer has requested a renegotiation.
            SecBuffer extraBuffer{
                static_cast<DWORD>(_extraBuffer.i - _extraBuffer.b.begin()),
                SECBUFFER_EXTRA,
                _extraBuffer.b.begin()};
            IceInternal::SocketOperation op = sslHandshake(&extraBuffer);
            _extraBuffer.b.clear();
            if (op == IceInternal::SocketOperationNone)
            {
                _sslConnectionRenegotiating = false;
                continue;
            }
            throw ConnectionLostException(__FILE__, __LINE__, 0);
        }

        if (decrypted == 0)
        {
            if (!readRaw(_readBuffer))
            {
                return IceInternal::SocketOperationRead;
            }
            continue;
        }

        buf.i += decrypted;
    }
    _delegate->getNativeInfo()->ready(
        IceInternal::SocketOperationRead,
        !_readUnprocessed.b.empty() || _readBuffer.i != _readBuffer.b.begin());
    return IceInternal::SocketOperationNone;
}

bool
Schannel::TransceiverI::startWrite(IceInternal::Buffer& buffer)
{
    if (_state == StateNotInitialized)
    {
        return _delegate->startWrite(buffer);
    }

    if (_state == StateHandshakeComplete && _bufferedW == 0)
    {
        assert(_writeBuffer.i == _writeBuffer.b.end());
        _bufferedW = encryptMessage(buffer);
    }

    return _delegate->startWrite(_writeBuffer) && _bufferedW == static_cast<size_t>((buffer.b.end() - buffer.i));
}

void
Schannel::TransceiverI::finishWrite(IceInternal::Buffer& buf)
{
    if (_state == StateNotInitialized)
    {
        _delegate->finishWrite(buf);
        return;
    }

    _delegate->finishWrite(_writeBuffer);
    if (_writeBuffer.i != _writeBuffer.b.end())
    {
        return; // We're not finished yet with writing the write buffer.
    }

    if (_state == StateHandshakeComplete)
    {
        buf.i += _bufferedW;
        _bufferedW = 0;
    }
}

void
Schannel::TransceiverI::startRead(IceInternal::Buffer& buffer)
{
    if (_state == StateNotInitialized)
    {
        _delegate->startRead(buffer);
        return;
    }
    _delegate->startRead(_readBuffer);
}

void
Schannel::TransceiverI::finishRead(IceInternal::Buffer& buf)
{
    if (_state == StateNotInitialized)
    {
        _delegate->finishRead(buf);
        return;
    }

    _delegate->finishRead(_readBuffer);
    if (_state == StateHandshakeComplete)
    {
        size_t decrypted;
        while (true)
        {
            decrypted = decryptMessage(buf);
            if (_sslConnectionRenegotiating)
            {
                // The peer has requested a renegotiation.
                SecBuffer extraBuffer{
                    static_cast<DWORD>(_extraBuffer.i - _extraBuffer.b.begin()),
                    SECBUFFER_EXTRA,
                    _extraBuffer.b.begin()};
                IceInternal::SocketOperation op = sslHandshake(&extraBuffer);
                _extraBuffer.b.clear();
                if (op == IceInternal::SocketOperationNone)
                {
                    _sslConnectionRenegotiating = false;
                    if (buf.i != buf.b.begin())
                    {
                        continue;
                    }
                    break;
                }
                throw ConnectionLostException(__FILE__, __LINE__, 0);
            }
            break;
        }

        if (decrypted > 0)
        {
            buf.i += decrypted;
            _delegate->getNativeInfo()->ready(
                IceInternal::SocketOperationRead,
                !_readUnprocessed.b.empty() || _readBuffer.i != _readBuffer.b.begin());
        }
        else
        {
            _delegate->getNativeInfo()->ready(IceInternal::SocketOperationRead, false);
        }
    }
}

string
Schannel::TransceiverI::protocol() const
{
    return _instance->protocol();
}

string
Schannel::TransceiverI::toString() const
{
    return _delegate->toString();
}

string
Schannel::TransceiverI::toDetailedString() const
{
    return toString();
}

Ice::ConnectionInfoPtr
Schannel::TransceiverI::getInfo(bool incoming, string adapterName, string connectionId) const
{
    assert(incoming == _incoming);
    // adapterName is the name of the object adapter currently associated with this connection, while _adapterName
    // represents the name of the object adapter that created this connection (incoming only).

    return make_shared<ConnectionInfo>(
        _delegate->getInfo(incoming, std::move(adapterName), std::move(connectionId)),
        CertDuplicateCertificateContext(_peerCertificate));
}

void
Schannel::TransceiverI::checkSendSize(const IceInternal::Buffer&)
{
}

void
Schannel::TransceiverI::setBufferSize(int rcvSize, int sndSize)
{
    _delegate->setBufferSize(rcvSize, sndSize);
}

Schannel::TransceiverI::TransceiverI(
    const InstancePtr& instance,
    const IceInternal::TransceiverPtr& delegate,
    const string& adapterName,
    const Ice::SSL::ServerAuthenticationOptions& serverAuthenticationOptions)
    : _instance(instance),
      _engine(dynamic_pointer_cast<Schannel::SSLEngine>(instance->engine())),
      _host(""),
      _adapterName(adapterName),
      _incoming(true),
      _delegate(delegate),
      _state(StateNotInitialized),
      _bufferedW(0),
      _clientCertificateRequired(serverAuthenticationOptions.clientCertificateRequired),
      _credentials({}),
      _credentialsHandle({}),
      _localCredentialsSelectionCallback(
          serverAuthenticationOptions.serverCredentialsSelectionCallback
              ? serverAuthenticationOptions.serverCredentialsSelectionCallback
              : defaultIncomingCredentials),
      _sslNewSessionCallback(serverAuthenticationOptions.sslNewSessionCallback),
      _peerCertificate(nullptr),
      _remoteCertificateValidationCallback(serverAuthenticationOptions.clientCertificateValidationCallback),
      _rootStore(serverAuthenticationOptions.trustedRootCertificates),
      _ssl({}),
      _chainEngine(nullptr),
      _sslConnectionRenegotiating(false)
{
    if (!_remoteCertificateValidationCallback)
    {
        // If the user didn't provide a validation callback. We setup a  default validation callback that exclusively
        // trust the provided trusted root certificates if any, or uses the current user store if no trusted root
        // certificates were provided. Schannel doesn't provide a default validation mechanism for client credentials.
        _chainEngine = defaultChainEngine(_rootStore);
        _remoteCertificateValidationCallback = [this](CtxtHandle ssl, const ConnectionInfoPtr&)
        {
            return SSLEngine::validationCallback(
                _chainEngine, // The chain engine configured to trust the provided trusted root certificates.
                ssl,          // The SSL context handle.
                true,         // This is an incoming connection.
                "",           // The target host, empty for incoming connections.
                _clientCertificateRequired, // Whether or not the peer must provide a certificate.
                0,                          // Disable revocation checking.
                false);                     // Whether or not revocation checks only uses cached information.
        };
    }
}

Schannel::TransceiverI::TransceiverI(
    const InstancePtr& instance,
    const IceInternal::TransceiverPtr& delegate,
    const string& host,
    const Ice::SSL::ClientAuthenticationOptions& clientAuthenticationOptions)
    : _instance(instance),
      _engine(dynamic_pointer_cast<Schannel::SSLEngine>(instance->engine())),
      _host(host),
      _adapterName(""),
      _incoming(false),
      _delegate(delegate),
      _state(StateNotInitialized),
      _bufferedW(0),
      _clientCertificateRequired(false),
      _credentials({}),
      _credentialsHandle({}),
      _localCredentialsSelectionCallback(
          clientAuthenticationOptions.clientCredentialsSelectionCallback
              ? clientAuthenticationOptions.clientCredentialsSelectionCallback
              : defaultOutgoingCredentials),
      _sslNewSessionCallback(clientAuthenticationOptions.sslNewSessionCallback),
      _peerCertificate(nullptr),
      _remoteCertificateValidationCallback(clientAuthenticationOptions.serverCertificateValidationCallback),
      _rootStore(clientAuthenticationOptions.trustedRootCertificates),
      _ssl({}),
      _chainEngine(nullptr),
      _sslConnectionRenegotiating(false)
{
    if (_rootStore && !_remoteCertificateValidationCallback)
    {
        // If the user configured the trusted root certificates, but didn't provide a validation callback. We setup
        // a default validation callback that exclusively trust the provided trusted root certificates.
        _chainEngine = defaultChainEngine(_rootStore);
        _remoteCertificateValidationCallback = [this](CtxtHandle ssl, const ConnectionInfoPtr&)
        {
            return SSLEngine::validationCallback(
                _chainEngine, // The chain engine configured to trust the provided trusted root certificates.
                ssl,          // The SSL context handle.
                false,        // This is an outgoing connection.
                _host,        // The target host.
                true,         // Whether or not the peer must provide a certificate.
                0,            // Disable revocation checking.
                false);       // Whether or not revocation checks only uses cached information.
        };
    }
    // If the user provides a validation callback is up to the callback to validate the certificate chain.
    // If the user doesn't set the trusted root certificates, we use Schannel default behavior, with uses the system
    // trusted root certificates.
}

Schannel::TransceiverI::~TransceiverI() = default;

bool
Schannel::TransceiverI::writeRaw(IceInternal::Buffer& buf)
{
    _delegate->write(buf);
    return buf.i == buf.b.end();
}

bool
Schannel::TransceiverI::readRaw(IceInternal::Buffer& buf)
{
    IceInternal::Buffer::Container::iterator p = buf.i;
    _delegate->read(buf);
    return buf.i != p;
}
