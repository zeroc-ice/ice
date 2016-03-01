// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/SChannelTransceiverI.h>

#include <IceUtil/StringUtil.h>

#include <IceSSL/ConnectionInfo.h>
#include <IceSSL/Instance.h>
#include <IceSSL/SSLEngine.h>
#include <IceSSL/Util.h>
#include <Ice/Communicator.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Buffer.h>
#include <Ice/LocalException.h>

#ifdef ICE_USE_SCHANNEL

using namespace std;
using namespace Ice;
using namespace IceSSL;

namespace
{

string
protocolName(DWORD protocol)
{
    switch(protocol)
    {
        case SP_PROT_SSL2_CLIENT:
        case SP_PROT_SSL2_SERVER:
            return "SSL 2.0";
        case SP_PROT_SSL3_CLIENT:
        case SP_PROT_SSL3_SERVER:
            return "SSL 3.0";
        case SP_PROT_TLS1_CLIENT:
        case SP_PROT_TLS1_SERVER:
            return "TLS 1.0";
        case SP_PROT_TLS1_1_CLIENT:
        case SP_PROT_TLS1_1_SERVER:
            return "TLS 1.1";
        case SP_PROT_TLS1_2_CLIENT:
        case SP_PROT_TLS1_2_SERVER:
            return "TLS 1.2";
        default:
            return "Unknown";
    }
}

string
trustStatusToString(DWORD status)
{
    if(status & CERT_TRUST_NO_ERROR)
    {
        return "CERT_TRUST_NO_ERROR";
    }

    if(status & CERT_TRUST_IS_NOT_TIME_VALID)
    {
        return "CERT_TRUST_IS_NOT_TIME_VALID";
    }

    if(status & CERT_TRUST_IS_REVOKED)
    {
        return "CERT_TRUST_IS_REVOKED";
    }

    if(status & CERT_TRUST_IS_NOT_SIGNATURE_VALID)
    {
        return "CERT_TRUST_IS_NOT_SIGNATURE_VALID";
    }

    if(status & CERT_TRUST_IS_NOT_VALID_FOR_USAGE)
    {
        return "CERT_TRUST_IS_NOT_VALID_FOR_USAGE";
    }

    if(status & CERT_TRUST_IS_UNTRUSTED_ROOT)
    {
        return "CERT_TRUST_IS_UNTRUSTED_ROOT";
    }

    if(status & CERT_TRUST_REVOCATION_STATUS_UNKNOWN)
    {
        return "CERT_TRUST_REVOCATION_STATUS_UNKNOWN";
    }

    if(status & CERT_TRUST_IS_CYCLIC)
    {
        return "CERT_TRUST_IS_CYCLIC";
    }

    if(status & CERT_TRUST_INVALID_EXTENSION)
    {
        return "CERT_TRUST_INVALID_EXTENSION";
    }

    if(status & CERT_TRUST_INVALID_POLICY_CONSTRAINTS)
    {
        return "CERT_TRUST_INVALID_POLICY_CONSTRAINTS";
    }

    if(status & CERT_TRUST_INVALID_BASIC_CONSTRAINTS)
    {
        return "CERT_TRUST_INVALID_BASIC_CONSTRAINTS";
    }

    if(status & CERT_TRUST_INVALID_NAME_CONSTRAINTS)
    {
        return "CERT_TRUST_INVALID_NAME_CONSTRAINTS";
    }

    if(status & CERT_TRUST_HAS_NOT_SUPPORTED_NAME_CONSTRAINT)
    {
        return "CERT_TRUST_HAS_NOT_SUPPORTED_NAME_CONSTRAINT";
    }

    if(status & CERT_TRUST_HAS_NOT_DEFINED_NAME_CONSTRAINT)
    {
        return "CERT_TRUST_HAS_NOT_DEFINED_NAME_CONSTRAINT";
    }

    if(status & CERT_TRUST_HAS_NOT_PERMITTED_NAME_CONSTRAINT)
    {
        return "CERT_TRUST_HAS_NOT_PERMITTED_NAME_CONSTRAINT";
    }

    if(status & CERT_TRUST_HAS_EXCLUDED_NAME_CONSTRAINT)
    {
        return "CERT_TRUST_HAS_EXCLUDED_NAME_CONSTRAINT";
    }

    if(status & CERT_TRUST_IS_OFFLINE_REVOCATION)
    {
        return "CERT_TRUST_IS_OFFLINE_REVOCATION";
    }

    if(status & CERT_TRUST_NO_ISSUANCE_CHAIN_POLICY)
    {
        return "CERT_TRUST_NO_ISSUANCE_CHAIN_POLICY";
    }

    if(status & CERT_TRUST_IS_EXPLICIT_DISTRUST)
    {
        return "CERT_TRUST_IS_EXPLICIT_DISTRUST";
    }

    if(status & CERT_TRUST_HAS_NOT_SUPPORTED_CRITICAL_EXT)
    {
        return "CERT_TRUST_HAS_NOT_SUPPORTED_CRITICAL_EXT";
    }

    //
    // New in Windows 8
    //
    //if(status & CERT_TRUST_HAS_WEAK_SIGNATURE)
    //{
    //    return "CERT_TRUST_HAS_WEAK_SIGNATURE";
    //}

    if(status & CERT_TRUST_IS_PARTIAL_CHAIN)
    {
        return "CERT_TRUST_IS_PARTIAL_CHAIN";
    }

    if(status & CERT_TRUST_CTL_IS_NOT_TIME_VALID)
    {
        return "CERT_TRUST_CTL_IS_NOT_TIME_VALID";
    }

    if(status & CERT_TRUST_CTL_IS_NOT_SIGNATURE_VALID)
    {
        return "CERT_TRUST_CTL_IS_NOT_SIGNATURE_VALID";
    }

    if(status & CERT_TRUST_CTL_IS_NOT_VALID_FOR_USAGE)
    {
        return "CERT_TRUST_CTL_IS_NOT_VALID_FOR_USAGE";
    }

    ostringstream os;
    os << "UNKNOWN TRUST FAILURE: " << status;
    return os.str();
}

SecBuffer*
getSecBufferWithType(const SecBufferDesc& desc, ULONG bufferType)
{
    for(ULONG i = 0; i < desc.cBuffers; ++i)
    {
        if(desc.pBuffers[i].BufferType == bufferType)
        {
            return &desc.pBuffers[i];
        }
    }
    return 0;
}

}

IceInternal::NativeInfoPtr
IceSSL::TransceiverI::getNativeInfo()
{
    return _stream;
}

IceInternal::SocketOperation
IceSSL::TransceiverI::sslHandshake()
{
    DWORD flags = ASC_REQ_SEQUENCE_DETECT | ASC_REQ_REPLAY_DETECT | ASC_REQ_CONFIDENTIALITY | ASC_REQ_ALLOCATE_MEMORY |
        ASC_REQ_STREAM;
    if(_incoming)
    {
        flags |= ASC_REQ_EXTENDED_ERROR;
        if(_engine->getVerifyPeer() > 0)
        {
            flags |= ASC_REQ_MUTUAL_AUTH;
        }
    }
    else
    {
        flags |= ISC_REQ_USE_SUPPLIED_CREDS | ISC_REQ_MANUAL_CRED_VALIDATION | ISC_RET_EXTENDED_ERROR;
    }

    SECURITY_STATUS err = SEC_E_OK;
    DWORD ctxFlags = 0;
    if(_state == StateHandshakeNotStarted)
    {
        _readBuffer.b.resize(2048);
        _readBuffer.i = _readBuffer.b.begin();
        _credentials = _engine->newCredentialsHandle(_incoming);
        _credentialsInitialized = true;

        if(!_incoming)
        {
            SecBuffer outBuffer = { 0, SECBUFFER_TOKEN, 0 };
            SecBufferDesc outBufferDesc = { SECBUFFER_VERSION, 1, &outBuffer };

            err = InitializeSecurityContext(&_credentials, 0, const_cast<char *>(_host.c_str()), flags, 0, 0, 0, 0,
                                            &_ssl, &outBufferDesc, &ctxFlags, 0);
            if(err != SEC_E_OK && err != SEC_I_CONTINUE_NEEDED)
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: handshake failure:\n" +
                                        IceUtilInternal::lastErrorToString());
            }
            _sslInitialized = true;

            //
            // Copy the data to the write buffer
            //
            _writeBuffer.b.resize(outBuffer.cbBuffer);
            _writeBuffer.i = _writeBuffer.b.begin();
            memcpy(_writeBuffer.i, outBuffer.pvBuffer, outBuffer.cbBuffer);
            FreeContextBuffer(outBuffer.pvBuffer);

            _state = StateHandshakeWriteContinue;
        }
        else
        {
            _state = StateHandshakeReadContinue;
        }
    }

    while(true)
    {
        if(_state == StateHandshakeReadContinue)
        {
            // If read buffer is empty, try to read some data.
            if(_readBuffer.i == _readBuffer.b.begin() && !readRaw(_readBuffer))
            {
                return IceInternal::SocketOperationRead;
            }

            SecBuffer inBuffers[2] = {
                { static_cast<DWORD>(_readBuffer.i - _readBuffer.b.begin()), SECBUFFER_TOKEN, _readBuffer.b.begin() },
                { 0, SECBUFFER_EMPTY, 0 }
            };
            SecBufferDesc inBufferDesc = { SECBUFFER_VERSION, 2, inBuffers };

            SecBuffer outBuffers[2] = {
                { 0, SECBUFFER_TOKEN, 0 },
                { 0, SECBUFFER_ALERT, 0 }
            };
            SecBufferDesc outBufferDesc = { SECBUFFER_VERSION, 2, outBuffers };

            if(_incoming)
            {
                err = AcceptSecurityContext(&_credentials, (_sslInitialized ? &_ssl : 0), &inBufferDesc, flags, 0,
                                            &_ssl, &outBufferDesc, &ctxFlags, 0);
                if(err == SEC_I_CONTINUE_NEEDED || err == SEC_E_OK)
                {
                    _sslInitialized = true;
                }
            }
            else
            {
                err = InitializeSecurityContext(&_credentials, &_ssl, const_cast<char*>(_host.c_str()), flags, 0, 0,
                                                &inBufferDesc, 0, 0, &outBufferDesc, &ctxFlags, 0);
            }

            //
            // If the message is incomplete we need to read more data.
            //
            if(err == SEC_E_INCOMPLETE_MESSAGE)
            {
                SecBuffer* missing = getSecBufferWithType(inBufferDesc, SECBUFFER_MISSING);
                size_t pos = _readBuffer.i - _readBuffer.b.begin();
                _readBuffer.b.resize(missing ? (pos + missing->cbBuffer) : (pos * 2));
                _readBuffer.i = _readBuffer.b.begin() + pos;
                return IceInternal::SocketOperationRead;
            }
            else if(err != SEC_I_CONTINUE_NEEDED && err != SEC_E_OK)
            {
                throw SecurityException(__FILE__, __LINE__, "SSL handshake failure:\n" +
                                        IceUtilInternal::lastErrorToString());
            }

            //
            // Copy out security tokens to the write buffer if any.
            //
            SecBuffer* token = getSecBufferWithType(outBufferDesc, SECBUFFER_TOKEN);
            assert(token);
            if(token->cbBuffer)
            {
                _writeBuffer.b.resize(static_cast<size_t>(token->cbBuffer));
                _writeBuffer.i = _writeBuffer.b.begin();
                memcpy(_writeBuffer.i, token->pvBuffer, token->cbBuffer);
                FreeContextBuffer(token->pvBuffer);
            }

            //
            // Check for remaining data in the input buffer.
            //
            SecBuffer* extra = getSecBufferWithType(inBufferDesc, SECBUFFER_EXTRA);
            if(extra)
            {
                // Shift the extra data to the start of the input buffer
                memmove(_readBuffer.b.begin(), _readBuffer.i - extra->cbBuffer, extra->cbBuffer);
                _readBuffer.i = _readBuffer.b.begin() + extra->cbBuffer;
            }
            else
            {
                _readBuffer.i = _readBuffer.b.begin();
            }

            if(token->cbBuffer)
            {
                _state = StateHandshakeWriteContinue; // Continue writing if we have a token.
            }
            else if(err == SEC_E_OK)
            {
                break; // We're done.
            }

	    // Otherwise continue either reading credentials
        }

        if(_state == StateHandshakeWriteContinue)
        {
            //
            // Write any pending data.
            //
            if(!writeRaw(_writeBuffer))
            {
                return IceInternal::SocketOperationWrite;
            }
            if(err == SEC_E_OK)
            {
                break; // Token is written and we weren't told to continue, so we're done!
            }
            assert(err == SEC_I_CONTINUE_NEEDED);
            _state = StateHandshakeReadContinue;
        }
    }

    //
    // Check if the requested capabilities are met
    //
    if(flags != ctxFlags)
    {
        if(_incoming)
        {
            if(!(ctxFlags & ASC_REQ_SEQUENCE_DETECT))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup sequence detect");
            }

            if(!(ctxFlags & ASC_REQ_REPLAY_DETECT))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup replay detect");
            }

            if(!(ctxFlags & ASC_REQ_CONFIDENTIALITY))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup confidentiality");
            }

            if(!(ctxFlags & ASC_REQ_EXTENDED_ERROR))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup extended error");
            }

            if(!(ctxFlags & ASC_REQ_ALLOCATE_MEMORY))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup memory allocation");
            }

            if(!(ctxFlags & ASC_REQ_STREAM))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup stream");
            }
        }
        else
        {
            if(!(ctxFlags & ISC_REQ_SEQUENCE_DETECT))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup sequence detect");
            }

            if(!(ctxFlags & ISC_REQ_REPLAY_DETECT))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup replay detect");
            }

            if(!(ctxFlags & ISC_REQ_CONFIDENTIALITY))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup confidentiality");
            }

            if(!(ctxFlags & ISC_REQ_EXTENDED_ERROR))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup extended error");
            }

            if(!(ctxFlags & ISC_REQ_ALLOCATE_MEMORY))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup memory allocation");
            }

            if(!(ctxFlags & ISC_REQ_STREAM))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup stream");
            }
        }
    }

    err = QueryContextAttributes(&_ssl, SECPKG_ATTR_STREAM_SIZES, &_sizes);
    if(err != SEC_E_OK)
    {
        throw SecurityException(__FILE__, __LINE__, "IceSSL: failure to query stream sizes attributes:\n" +
                                IceUtilInternal::lastErrorToString());
    }

    size_t pos = _readBuffer.i - _readBuffer.b.begin();
    if(pos <= (_sizes.cbHeader + _sizes.cbMaximumMessage + _sizes.cbTrailer))
    {
        _readBuffer.b.resize(_sizes.cbHeader + _sizes.cbMaximumMessage + _sizes.cbTrailer);
        _readBuffer.i = _readBuffer.b.begin() + pos;
    }

    _writeBuffer.b.reset();
    _writeBuffer.i = _writeBuffer.b.begin();

    return IceInternal::SocketOperationNone;
}

//
// Try to decrypt a message and return the number of bytes decrypted, if the number of bytes
// decrypted is less than the size requested it means that the application needs to read more
// data before it can decrypt the complete message.
//
size_t
IceSSL::TransceiverI::decryptMessage(IceInternal::Buffer& buffer)
{
    assert(_readBuffer.i != _readBuffer.b.begin() || !_readUnprocessed.b.empty());

    //
    // First check if there is data in the unprocessed buffer.
    //
    size_t length = std::min(static_cast<size_t>(buffer.b.end() - buffer.i), _readUnprocessed.b.size());
    if(length > 0)
    {
        memcpy(buffer.i, _readUnprocessed.b.begin(), length);
        memmove(_readUnprocessed.b.begin(), _readUnprocessed.b.begin() + length, _readUnprocessed.b.size() - length);
        _readUnprocessed.b.resize(_readUnprocessed.b.size() - length);
    }

    while(true)
    {
        //
        // If we have filled the buffer or if nothing left to read from
        // the read buffer, we're done.
        //
        Byte* i = buffer.i + length;
        if(i == buffer.b.end() || _readBuffer.i == _readBuffer.b.begin())
        {
            break;
        }

        //
        // Try to decrypt the buffered data.
        //
        SecBuffer inBuffers[4] = {
            { static_cast<DWORD>(_readBuffer.i - _readBuffer.b.begin()), SECBUFFER_DATA, _readBuffer.b.begin() },
            { 0, SECBUFFER_EMPTY, 0 },
            { 0, SECBUFFER_EMPTY, 0 },
            { 0, SECBUFFER_EMPTY, 0 }
        };
        SecBufferDesc inBufferDesc = { SECBUFFER_VERSION, 4, inBuffers };

        SECURITY_STATUS err = DecryptMessage(&_ssl, &inBufferDesc, 0, 0);
        if(err == SEC_E_INCOMPLETE_MESSAGE)
        {
            //
            // There isn't enough data to decrypt the message. The input
            // buffer is resized to the SSL max message size after the SSL
            // handshake completes so an incomplete message can only occur
            // if the read buffer is not full.
            //
            assert(_readBuffer.i != _readBuffer.b.end());
            return length;
        }
        else if(err == SEC_I_CONTEXT_EXPIRED || err == SEC_I_RENEGOTIATE)
        {
            //
            // The message sender has finished using the connection and
            // has initiated a shutdown.
            //
            throw ConnectionLostException(__FILE__, __LINE__, 0);
        }
        else if(err != SEC_E_OK)
        {
            throw ProtocolException(__FILE__, __LINE__, "IceSSL: protocol error during read:\n" +
                                    IceUtilInternal::lastErrorToString());
        }

        SecBuffer* dataBuffer = getSecBufferWithType(inBufferDesc, SECBUFFER_DATA);
        assert(dataBuffer);
        DWORD remaining = min(static_cast<DWORD>(buffer.b.end() - i), dataBuffer->cbBuffer);
        length += remaining;
        if(remaining)
        {
            memcpy(i, dataBuffer->pvBuffer, remaining);

            //
            // Copy remaining decrypted data to unprocessed buffer
            //
            if(dataBuffer->cbBuffer > remaining)
            {
                _readUnprocessed.b.resize(dataBuffer->cbBuffer - remaining);
                memcpy(_readUnprocessed.b.begin(), reinterpret_cast<Byte*>(dataBuffer->pvBuffer) + remaining,
                    dataBuffer->cbBuffer - remaining);
            }
        }

        //
        // Move any remaining encrypted data to the begining of the input buffer
        //
        SecBuffer* extraBuffer = getSecBufferWithType(inBufferDesc, SECBUFFER_EXTRA);
        if(extraBuffer && extraBuffer->cbBuffer > 0)
        {
            memmove(_readBuffer.b.begin(), _readBuffer.i - extraBuffer->cbBuffer, extraBuffer->cbBuffer);
            _readBuffer.i = _readBuffer.b.begin() + extraBuffer->cbBuffer;
        }
        else
        {
            _readBuffer.i = _readBuffer.b.begin();
        }
    }
    return length;
}

//
// Encrypt a message and return the number of bytes that has been encrypted, if the
// number of bytes is less than the message size, the function must be called again.
//
size_t
IceSSL::TransceiverI::encryptMessage(IceInternal::Buffer& buffer)
{
    //
    // Limit the message size to cbMaximumMessage which is the maximun size data that can be
    // embeded in a SSL record.
    //
    DWORD length = std::min(static_cast<DWORD>(buffer.b.end() - buffer.i), _sizes.cbMaximumMessage);

    //
    // Resize the buffer to hold the encrypted data
    //
    _writeBuffer.b.resize(_sizes.cbHeader + length + _sizes.cbTrailer);
    _writeBuffer.i = _writeBuffer.b.begin();

    SecBuffer buffers[4] = {
        { _sizes.cbHeader, SECBUFFER_STREAM_HEADER, _writeBuffer.i },
        { length, SECBUFFER_DATA, _writeBuffer.i + _sizes.cbHeader },
        { _sizes.cbTrailer, SECBUFFER_STREAM_TRAILER, _writeBuffer.i + _sizes.cbHeader + length },
        { 0, SECBUFFER_EMPTY, 0 }
    };
    SecBufferDesc buffersDesc = { SECBUFFER_VERSION, 4, buffers };

    // Data is encrypted in place, copy the data to be encrypted to the data buffer.
    memcpy(buffers[1].pvBuffer, buffer.i, length);

    SECURITY_STATUS err = EncryptMessage(&_ssl, 0, &buffersDesc, 0);
    if(err != SEC_E_OK)
    {
        throw ProtocolException(__FILE__, __LINE__, "IceSSL: protocol error encrypting message:\n" +
                                IceUtilInternal::lastErrorToString());
    }

    // EncryptMessage resizes the buffers, so resize the write buffer as well to reflect this.
    _writeBuffer.b.resize(buffers[0].cbBuffer + buffers[1].cbBuffer + buffers[2].cbBuffer);
    _writeBuffer.i = _writeBuffer.b.begin();

    return length;
}

IceInternal::SocketOperation
IceSSL::TransceiverI::initialize(IceInternal::Buffer& readBuffer, IceInternal::Buffer& writeBuffer, bool& hasMoreData)
{
    IceInternal::SocketOperation op = _stream->connect(readBuffer, writeBuffer);
    if(op != IceInternal::SocketOperationNone)
    {
        return op;
    }

    op = sslHandshake();
    if(op != IceInternal::SocketOperationNone)
    {
        return op;
    }

    //
    // Build the peer certificate chain and verify it.
    //
    PCCERT_CONTEXT cert = 0;
    SECURITY_STATUS err = QueryContextAttributes(&_ssl, SECPKG_ATTR_REMOTE_CERT_CONTEXT, &cert);
    if(err && err != SEC_E_NO_CREDENTIALS)
    {
        throw SecurityException(__FILE__, __LINE__, "IceSSL: certificate verification failure:" +
                                IceUtilInternal::lastErrorToString());
    }

    if(!cert && ((!_incoming && _engine->getVerifyPeer() > 0) || (_incoming && _engine->getVerifyPeer() == 2)))
    {
        //
        // Clients require server certificate if VerifyPeer > 0 and servers require client
        // certificate if VerifyPeer == 2
        //
        throw SecurityException(__FILE__, __LINE__, "IceSSL: certificate required:" +
                                IceUtilInternal::lastErrorToString());
    }
    else if(cert) // Verify the remote certificate
    {
        try
        {
            CERT_CHAIN_PARA chainP;
            memset(&chainP, 0, sizeof(chainP));
            chainP.cbSize = sizeof(chainP);

            PCCERT_CHAIN_CONTEXT certChain;
            if(!CertGetCertificateChain(_engine->chainEngine(), cert, 0, 0, &chainP,
                                        CERT_CHAIN_REVOCATION_CHECK_CACHE_ONLY, 0, &certChain))
            {
                CertFreeCertificateContext(cert);
                throw IceUtilInternal::lastErrorToString();
            }

            CERT_SIMPLE_CHAIN* simpleChain = certChain->rgpChain[0];

            string trustError;
            if(simpleChain->TrustStatus.dwErrorStatus != CERT_TRUST_NO_ERROR)
            {
                trustError = trustStatusToString(certChain->TrustStatus.dwErrorStatus);
            }
            else
            {
                _verified = true;
            }

            CertFreeCertificateChain(certChain);
            CertFreeCertificateContext(cert);
            if(!trustError.empty())
            {
                throw trustError;
            }
        }
        catch(const string& reason)
        {
            if(_engine->getVerifyPeer() == 0)
            {
                if(_instance->traceLevel() >= 1)
                {
                    _instance->logger()->trace(_instance->traceCategory(),
                                               "IceSSL: ignoring certificate verification failure\n" + reason);
                }
            }
            else
            {
                ostringstream os;
                os << "IceSSL: certificate verification failure\n" << reason;
                string msg = os.str();
                if(_instance->traceLevel() >= 1)
                {
                    _instance->logger()->trace(_instance->traceCategory(), msg);
                }
                throw SecurityException(__FILE__, __LINE__, msg);
            }
        }
    }

    _engine->verifyPeer(_stream->fd(), _host, NativeConnectionInfoPtr::dynamicCast(getInfo()));
    _state = StateHandshakeComplete;

    if(_instance->engine()->securityTraceLevel() >= 1)
    {
        string sslCipherName;
        string sslKeyExchangeAlgorithm;
        string sslProtocolName;
        SecPkgContext_ConnectionInfo info;
        if(QueryContextAttributes(&_ssl, SECPKG_ATTR_CONNECTION_INFO, &info) == SEC_E_OK)
        {
            sslCipherName = _engine->getCipherName(info.aiCipher);
            sslKeyExchangeAlgorithm = _engine->getCipherName(info.aiExch);
            sslProtocolName = protocolName(info.dwProtocol);
        }

        Trace out(_instance->logger(), _instance->traceCategory());
        out << "SSL summary for " << (_incoming ? "incoming" : "outgoing") << " connection\n";

        if(sslCipherName.empty())
        {
            out << "unknown cipher\n";
        }
        else
        {
            out << "cipher = " << sslCipherName
                << "\nkey exchange = " << sslKeyExchangeAlgorithm
                << "\nprotocol = " << sslProtocolName << "\n";
        }
        out << toString();
    }
    hasMoreData = !_readUnprocessed.b.empty() || _readBuffer.i != _readBuffer.b.begin();
    return IceInternal::SocketOperationNone;
}

IceInternal::SocketOperation
IceSSL::TransceiverI::closing(bool initiator, const Ice::LocalException&)
{
    // If we are initiating the connection closure, wait for the peer
    // to close the TCP/IP connection. Otherwise, close immediately.
    return initiator ? IceInternal::SocketOperationRead : IceInternal::SocketOperationNone;
}

void
IceSSL::TransceiverI::close()
{
    if(_sslInitialized)
    {
        DeleteSecurityContext(&_ssl);
        _sslInitialized = false;
    }

    if(_credentialsInitialized)
    {
        FreeCredentialsHandle(&_credentials);
        _credentialsInitialized = false;
    }

    _stream->close();

    //
    // Clear the buffers now instead of waiting for destruction.
    //
    _writeBuffer.b.clear();
    _readBuffer.b.clear();
    _readUnprocessed.b.clear();
}

IceInternal::SocketOperation
IceSSL::TransceiverI::write(IceInternal::Buffer& buf)
{
    if(!_stream->isConnected())
    {
        return _stream->write(buf);
    }

    if(buf.i == buf.b.end())
    {
        return IceInternal::SocketOperationNone;
    }
    assert(_state == StateHandshakeComplete);

    while(buf.i != buf.b.end())
    {
        if(_bufferedW == 0)
        {
            assert(_writeBuffer.i == _writeBuffer.b.end());
            _bufferedW = encryptMessage(buf);
        }

        if(!writeRaw(_writeBuffer))
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
IceSSL::TransceiverI::read(IceInternal::Buffer& buf, bool& hasMoreData)
{
    if(!_stream->isConnected())
    {
        return _stream->read(buf);
    }

    if(buf.i == buf.b.end())
    {
        return IceInternal::SocketOperationNone;
    }
    assert(_state == StateHandshakeComplete);

    hasMoreData = false;
    while(buf.i != buf.b.end())
    {
        if(_readUnprocessed.b.empty() && _readBuffer.i == _readBuffer.b.begin() && !readRaw(_readBuffer))
        {
            return IceInternal::SocketOperationRead;
        }

        size_t decrypted = decryptMessage(buf);
        if(decrypted == 0)
        {
            if(!readRaw(_readBuffer))
            {
                return IceInternal::SocketOperationRead;
            }
            continue;
        }

        buf.i += decrypted;
    }
    hasMoreData = !_readUnprocessed.b.empty() || _readBuffer.i != _readBuffer.b.begin();
    return IceInternal::SocketOperationNone;
}

#ifdef ICE_USE_IOCP

bool
IceSSL::TransceiverI::startWrite(IceInternal::Buffer& buffer)
{
    if(!_stream->isConnected())
    {
        return _stream->startWrite(buffer);
    }

    if(_state == StateHandshakeComplete && _bufferedW == 0)
    {
        assert(_writeBuffer.i == _writeBuffer.b.end());
        _bufferedW = encryptMessage(buffer);
    }

    return _stream->startWrite(_writeBuffer);
}

void
IceSSL::TransceiverI::finishWrite(IceInternal::Buffer& buf)
{
    if(!_stream->isConnected())
    {
        _stream->finishWrite(buf);
        return;
    }

    _stream->finishWrite(_writeBuffer);
    if(_writeBuffer.i != _writeBuffer.b.end())
    {
        return; // We're not finished yet with writing the write buffer.
    }

    if(_state == StateHandshakeComplete)
    {
        buf.i += _bufferedW;
        _bufferedW = 0;
    }
}

void
IceSSL::TransceiverI::startRead(IceInternal::Buffer& buffer)
{
    if(!_stream->isConnected())
    {
        _stream->startRead(buffer);
        return;
    }
    _stream->startRead(_readBuffer);
}

void
IceSSL::TransceiverI::finishRead(IceInternal::Buffer& buf, bool& hasMoreData)
{
    if(!_stream->isConnected())
    {
        _stream->finishRead(buf);
        return;
    }

    _stream->finishRead(_readBuffer);
    if(_state == StateHandshakeComplete)
    {
        size_t decrypted = decryptMessage(buf);
        if(decrypted > 0)
        {
            buf.i += decrypted;
            hasMoreData = !_readUnprocessed.b.empty() || _readBuffer.i != _readBuffer.b.begin();
        }
        else
        {
            hasMoreData = false;
        }
    }
}
#endif

string
IceSSL::TransceiverI::protocol() const
{
    return _instance->protocol();
}

string
IceSSL::TransceiverI::toString() const
{
    return _stream->toString();
}

string
IceSSL::TransceiverI::toDetailedString() const
{
    return toString();
}

Ice::ConnectionInfoPtr
IceSSL::TransceiverI::getInfo() const
{
    NativeConnectionInfoPtr info = new NativeConnectionInfo();
    fillConnectionInfo(info, info->nativeCerts);
    return info;
}

Ice::ConnectionInfoPtr
IceSSL::TransceiverI::getWSInfo(const Ice::HeaderDict& headers) const
{
    WSSNativeConnectionInfoPtr info = new WSSNativeConnectionInfo();
    fillConnectionInfo(info, info->nativeCerts);
    info->headers = headers;
    return info;
}

void
IceSSL::TransceiverI::checkSendSize(const IceInternal::Buffer&)
{
}

void
IceSSL::TransceiverI::setBufferSize(int rcvSize, int sndSize)
{
    _stream->setBufferSize(rcvSize, sndSize);
}

IceSSL::TransceiverI::TransceiverI(const InstancePtr& instance,
                                   const IceInternal::StreamSocketPtr& stream,
                                   const string& hostOrAdapterName,
                                   bool incoming) :
    _instance(instance),
    _engine(SChannelEnginePtr::dynamicCast(instance->engine())),
    _host(incoming ? "" : hostOrAdapterName),
    _adapterName(incoming ? hostOrAdapterName : ""),
    _incoming(incoming),
    _stream(stream),
    _state(StateHandshakeNotStarted),
    _bufferedW(0),
    _sslInitialized(false),
    _credentialsInitialized(false),
    _verified(false)
{
}

IceSSL::TransceiverI::~TransceiverI()
{
}

void
IceSSL::TransceiverI::fillConnectionInfo(const ConnectionInfoPtr& info, vector<CertificatePtr>& nativeCerts) const
{
    IceInternal::fdToAddressAndPort(_stream->fd(), info->localAddress, info->localPort, info->remoteAddress,
                                    info->remotePort);
    if(_stream->fd() != INVALID_SOCKET)
    {
        info->rcvSize = IceInternal::getRecvBufferSize(_stream->fd());
        info->sndSize = IceInternal::getSendBufferSize(_stream->fd());
    }

    info->verified = _verified;

    if(_sslInitialized)
    {
        CtxtHandle* ssl = const_cast<CtxtHandle*>(&_ssl);
        PCCERT_CONTEXT cert = 0;
        PCCERT_CHAIN_CONTEXT certChain = 0;
        SECURITY_STATUS err = QueryContextAttributes(ssl, SECPKG_ATTR_REMOTE_CERT_CONTEXT, &cert);
        if(err == SEC_E_OK)
        {
            assert(cert);
            CERT_CHAIN_PARA chainP;
            memset(&chainP, 0, sizeof(chainP));
            chainP.cbSize = sizeof(chainP);

            if(CertGetCertificateChain(_engine->chainEngine(), cert, 0, 0, &chainP,
                                       CERT_CHAIN_REVOCATION_CHECK_CACHE_ONLY, 0, &certChain))
            {
                CERT_SIMPLE_CHAIN* simpleChain = certChain->rgpChain[0];
                for(DWORD i = 0; i < simpleChain->cElement; ++i)
                {
                    PCCERT_CONTEXT c = simpleChain->rgpElement[i]->pCertContext;
                    PCERT_SIGNED_CONTENT_INFO cc;

                    DWORD length = 0;
                    if(!CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, X509_CERT, c->pbCertEncoded,
                                            c->cbCertEncoded, CRYPT_DECODE_ALLOC_FLAG, 0, &cc, &length))
                    {
                        CertFreeCertificateChain(certChain);
                        CertFreeCertificateContext(cert);
                        throw SecurityException(__FILE__, __LINE__,
                                                "IceSSL: error decoding peer certificate chain:\n" +
                                                IceUtilInternal::lastErrorToString());
                    }

                    CertificatePtr certificate = new Certificate(cc);
                    nativeCerts.push_back(certificate);
                    info->certs.push_back(certificate->encode());
                }
                CertFreeCertificateChain(certChain);
            }
            CertFreeCertificateContext(cert);
        }
        else if(err != SEC_E_NO_CREDENTIALS)
        {
            throw SecurityException(__FILE__, __LINE__, "IceSSL: error reading peer certificate:" +
                                    IceUtilInternal::lastErrorToString());
        }

        SecPkgContext_ConnectionInfo connInfo;
        if(QueryContextAttributes(ssl, SECPKG_ATTR_CONNECTION_INFO, &connInfo) == SEC_E_OK)
        {
            info->cipher = _engine->getCipherName(connInfo.aiCipher);
        }
        else
        {
            throw SecurityException(__FILE__, __LINE__, "IceSSL: error reading cipher info:" +
                                    IceUtilInternal::lastErrorToString());
        }
    }

    info->adapterName = _adapterName;
    info->incoming = _incoming;
}

bool
IceSSL::TransceiverI::writeRaw(IceInternal::Buffer& buf)
{
    ssize_t ret = _stream->write(reinterpret_cast<const char*>(&*buf.i), buf.b.end() - buf.i);
    buf.i += ret;
    return buf.i == buf.b.end();
}

bool
IceSSL::TransceiverI::readRaw(IceInternal::Buffer& buf)
{
    ssize_t ret = _stream->read(reinterpret_cast<char*>(&*buf.i), buf.b.end() - buf.i);
    buf.i += ret;
    return ret > 0;
}
#endif
