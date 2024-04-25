//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "SChannelTransceiverI.h"
#include "Ice/Buffer.h"
#include "Ice/Communicator.h"
#include "Ice/LocalException.h"
#include "Ice/LoggerUtil.h"
#include "Ice/SSLConnectionInfo.h"
#include "IceUtil/StringUtil.h"
#include "SChannelEngine.h"
#include "SSLInstance.h"
#include "SSLUtil.h"

#include <iostream>

using namespace std;
using namespace Ice;
using namespace IceSSL;

#ifndef CERT_CHAIN_DISABLE_AIA
#    define CERT_CHAIN_DISABLE_AIA 0x00002000
#endif

namespace
{
    string protocolName(DWORD protocol)
    {
        switch (protocol)
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
            case SP_PROT_TLS1_3_CLIENT:
            case SP_PROT_TLS1_3_SERVER:
                return "TLS 1.3";
            default:
                return "Unknown";
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
        return 0;
    }
}

IceInternal::NativeInfoPtr
SChannel::TransceiverI::getNativeInfo()
{
    return _delegate->getNativeInfo();
}

IceInternal::SocketOperation
SChannel::TransceiverI::sslHandshake()
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
        if (_localCredentialsSelectionCallback)
        {
            _credentials = _localCredentialsSelectionCallback(_host);
        }
        _ctxFlags = 0;
        _readBuffer.b.resize(2048);
        _readBuffer.i = _readBuffer.b.begin();

        if (!_incoming)
        {
            SecBuffer outBuffer = {0, SECBUFFER_TOKEN, 0};
            SecBufferDesc outBufferDesc = {SECBUFFER_VERSION, 1, &outBuffer};

            err = InitializeSecurityContext(
                &_credentials,
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
                os << "IceSSL: handshake failure:\n" << IceUtilInternal::errorToString(err);
                throw SecurityException(__FILE__, __LINE__, os.str());
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

    while (true)
    {
        if (_state == StateHandshakeReadContinue)
        {
            // If read buffer is empty, try to read some data.
            if (_readBuffer.i == _readBuffer.b.begin() && !readRaw(_readBuffer))
            {
                return IceInternal::SocketOperationRead;
            }

            SecBuffer inBuffers[2] = {
                {static_cast<DWORD>(_readBuffer.i - _readBuffer.b.begin()), SECBUFFER_TOKEN, _readBuffer.b.begin()},
                {0, SECBUFFER_EMPTY, 0}};
            SecBufferDesc inBufferDesc = {SECBUFFER_VERSION, 2, inBuffers};

            SecBuffer outBuffers[2] = {{0, SECBUFFER_TOKEN, 0}, {0, SECBUFFER_ALERT, 0}};
            SecBufferDesc outBufferDesc = {SECBUFFER_VERSION, 2, outBuffers};

            if (_incoming)
            {
                err = AcceptSecurityContext(
                    &_credentials,
                    (_sslInitialized ? &_ssl : 0),
                    &inBufferDesc,
                    _clientCertificateRequired ? flags | ASC_REQ_MUTUAL_AUTH : flags,
                    0,
                    &_ssl,
                    &outBufferDesc,
                    &_ctxFlags,
                    0);
                if (err == SEC_I_CONTINUE_NEEDED || err == SEC_E_OK)
                {
                    _sslInitialized = true;
                }
            }
            else
            {
                err = InitializeSecurityContext(
                    &_credentials,
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
                os << "SSL handshake failure:\n" << IceUtilInternal::errorToString(err);
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
                os << "\n - IceSSL: SChannel failed to setup sequence detect";
            }

            if (!(_ctxFlags & ASC_REQ_REPLAY_DETECT))
            {
                os << "\n - IceSSL: SChannel failed to setup replay detect";
            }

            if (!(_ctxFlags & ASC_REQ_CONFIDENTIALITY))
            {
                os << "\n - IceSSL: SChannel failed to setup confidentiality";
            }

            if (!(_ctxFlags & ASC_REQ_EXTENDED_ERROR))
            {
                os << "\n - IceSSL: SChannel failed to setup extended error";
            }

            if (!(_ctxFlags & ASC_REQ_ALLOCATE_MEMORY))
            {
                os << "\n - IceSSL: SChannel failed to setup memory allocation";
            }

            if (!(_ctxFlags & ASC_REQ_STREAM))
            {
                os << "\n - IceSSL: SChannel failed to setup stream";
            }
        }
        else
        {
            if (!(_ctxFlags & ISC_REQ_SEQUENCE_DETECT))
            {
                os << "\n - IceSSL: SChannel failed to setup sequence detect";
            }

            if (!(_ctxFlags & ISC_REQ_REPLAY_DETECT))
            {
                os << "\n - IceSSL: SChannel failed to setup replay detect";
            }

            if (!(_ctxFlags & ISC_REQ_CONFIDENTIALITY))
            {
                os << "\n - IceSSL: SChannel failed to setup confidentiality";
            }

            if (!(_ctxFlags & ISC_REQ_EXTENDED_ERROR))
            {
                os << "\n - IceSSL: SChannel failed to setup extended error";
            }

            if (!(_ctxFlags & ISC_REQ_ALLOCATE_MEMORY))
            {
                os << "\n - IceSSL: SChannel failed to setup memory allocation";
            }

            if (!(_ctxFlags & ISC_REQ_STREAM))
            {
                os << "\n - IceSSL: SChannel failed to setup stream";
            }
        }
        throw SecurityException(__FILE__, __LINE__, os.str());
    }

    err = QueryContextAttributes(&_ssl, SECPKG_ATTR_STREAM_SIZES, &_sizes);
    if (err != SEC_E_OK)
    {
        ostringstream os;
        os << "IceSSL: failure to query stream sizes attributes:\n" << IceUtilInternal::errorToString(err);
        throw SecurityException(__FILE__, __LINE__, os.str());
    }

    PCCERT_CONTEXT cert = 0;
    err = QueryContextAttributes(&_ssl, SECPKG_ATTR_REMOTE_CERT_CONTEXT, &cert);
    if (err != SEC_E_OK && err != SEC_E_NO_CREDENTIALS)
    {
        ostringstream os;
        os << "IceSSL: failure to query remote certificate context:\n" << IceUtilInternal::errorToString(err);
        throw SecurityException(__FILE__, __LINE__, os.str());
    }

    if (cert)
    {
        PCERT_SIGNED_CONTENT_INFO pvStructInfo = nullptr;
        DWORD pvStructInfoSize = 0;
        if (!CryptDecodeObjectEx(
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                X509_CERT,
                cert->pbCertEncoded,
                cert->cbCertEncoded,
                CRYPT_DECODE_ALLOC_FLAG,
                0,
                &pvStructInfo,
                &pvStructInfoSize))
        {
            ostringstream os;
            os << "IceSSL: error decoding peer certificate:\n" << IceUtilInternal::lastErrorToString();
            CertFreeCertificateContext(cert);
            throw SecurityException(__FILE__, __LINE__, os.str());
        }
        _certs.push_back(SChannel::Certificate::create(pvStructInfo));
        CertFreeCertificateContext(cert);
    }

    size_t pos = _readBuffer.i - _readBuffer.b.begin();
    if (pos <= (_sizes.cbHeader + _sizes.cbMaximumMessage + _sizes.cbTrailer))
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
SChannel::TransceiverI::decryptMessage(IceInternal::Buffer& buffer)
{
    assert(_readBuffer.i != _readBuffer.b.begin() || !_readUnprocessed.b.empty());

    //
    // First check if there is data in the unprocessed buffer.
    //
    size_t length = std::min(static_cast<size_t>(buffer.b.end() - buffer.i), _readUnprocessed.b.size());
    if (length > 0)
    {
        memcpy(buffer.i, _readUnprocessed.b.begin(), length);
        memmove(_readUnprocessed.b.begin(), _readUnprocessed.b.begin() + length, _readUnprocessed.b.size() - length);
        _readUnprocessed.b.resize(_readUnprocessed.b.size() - length);
    }

    while (true)
    {
        // If we have filled the buffer or if nothing left to read from the read buffer, we're done.
        std::byte* i = buffer.i + length;
        if (i == buffer.b.end() || _readBuffer.i == _readBuffer.b.begin())
        {
            break;
        }

        // Try to decrypt the buffered data.
        SecBuffer inBuffers[4] = {
            {static_cast<DWORD>(_readBuffer.i - _readBuffer.b.begin()), SECBUFFER_DATA, _readBuffer.b.begin()},
            {0, SECBUFFER_EMPTY, 0},
            {0, SECBUFFER_EMPTY, 0},
            {0, SECBUFFER_EMPTY, 0}};
        SecBufferDesc inBufferDesc = {SECBUFFER_VERSION, 4, inBuffers};

        SECURITY_STATUS err = DecryptMessage(&_ssl, &inBufferDesc, 0, 0);
        if (err == SEC_E_INCOMPLETE_MESSAGE)
        {
            // There isn't enough data to decrypt the message. The input buffer is resized to the SSL max message size
            // after the SSL handshake completes so an incomplete message can only occur if the read buffer is not full.
            assert(_readBuffer.i != _readBuffer.b.end());
            return length;
        }
        else if (err == SEC_I_CONTEXT_EXPIRED || err == SEC_I_RENEGOTIATE)
        {
            // The message sender has finished using the connection and has initiated a shutdown.
            throw ConnectionLostException(__FILE__, __LINE__, 0);
        }
        else if (err != SEC_E_OK)
        {
            throw ProtocolException(
                __FILE__,
                __LINE__,
                "IceSSL: protocol error during read:\n" + IceUtilInternal::errorToString(err));
        }

        SecBuffer* dataBuffer = getSecBufferWithType(inBufferDesc, SECBUFFER_DATA);
        assert(dataBuffer);
        DWORD remaining = min(static_cast<DWORD>(buffer.b.end() - i), dataBuffer->cbBuffer);
        length += remaining;
        if (remaining)
        {
            memcpy(i, dataBuffer->pvBuffer, remaining);

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
    return length;
}

//
// Encrypt a message and return the number of bytes that has been encrypted, if the
// number of bytes is less than the message size, the function must be called again.
//
size_t
SChannel::TransceiverI::encryptMessage(IceInternal::Buffer& buffer)
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

    SecBuffer buffers[4] = {
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
            "IceSSL: protocol error encrypting message:\n" + IceUtilInternal::errorToString(err));
    }

    // EncryptMessage resizes the buffers, so resize the write buffer as well to reflect this.
    _writeBuffer.b.resize(buffers[0].cbBuffer + buffers[1].cbBuffer + buffers[2].cbBuffer);
    _writeBuffer.i = _writeBuffer.b.begin();

    return length;
}

IceInternal::SocketOperation
SChannel::TransceiverI::initialize(IceInternal::Buffer& readBuffer, IceInternal::Buffer& writeBuffer)
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

    IceInternal::SocketOperation op = sslHandshake();
    if (op != IceInternal::SocketOperationNone)
    {
        return op;
    }

    if (_remoteCertificateValidationCallback &&
        !_remoteCertificateValidationCallback(_ssl, dynamic_pointer_cast<IceSSL::ConnectionInfo>(getInfo())))
    {
        throw SecurityException(
            __FILE__,
            __LINE__,
            "IceSSL: certificate verification failed. the certificate was explicitly rejected by the remote "
            "certificate verifier callback.");
    }

    _state = StateHandshakeComplete;

    _delegate->getNativeInfo()->ready(
        IceInternal::SocketOperationRead,
        !_readUnprocessed.b.empty() || _readBuffer.i != _readBuffer.b.begin());

    return IceInternal::SocketOperationNone;
}

IceInternal::SocketOperation
SChannel::TransceiverI::closing(bool initiator, exception_ptr)
{
    // If we are initiating the connection closure, wait for the peer
    // to close the TCP/IP connection. Otherwise, close immediately.
    return initiator ? IceInternal::SocketOperationRead : IceInternal::SocketOperationNone;
}

void
SChannel::TransceiverI::close()
{
    if (_sslInitialized)
    {
        DeleteSecurityContext(&_ssl);
        _sslInitialized = false;
    }
    _delegate->close();

    // Clear the buffers now instead of waiting for destruction.
    _writeBuffer.b.clear();
    _readBuffer.b.clear();
    _readUnprocessed.b.clear();
}

IceInternal::SocketOperation
SChannel::TransceiverI::write(IceInternal::Buffer& buf)
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
SChannel::TransceiverI::read(IceInternal::Buffer& buf)
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

#ifdef ICE_USE_IOCP

bool
SChannel::TransceiverI::startWrite(IceInternal::Buffer& buffer)
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
SChannel::TransceiverI::finishWrite(IceInternal::Buffer& buf)
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
SChannel::TransceiverI::startRead(IceInternal::Buffer& buffer)
{
    if (_state == StateNotInitialized)
    {
        _delegate->startRead(buffer);
        return;
    }
    _delegate->startRead(_readBuffer);
}

void
SChannel::TransceiverI::finishRead(IceInternal::Buffer& buf)
{
    if (_state == StateNotInitialized)
    {
        _delegate->finishRead(buf);
        return;
    }

    _delegate->finishRead(_readBuffer);
    if (_state == StateHandshakeComplete)
    {
        size_t decrypted = decryptMessage(buf);
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
#endif

bool
SChannel::TransceiverI::isWaitingToBeRead() const noexcept
{
    return _delegate->isWaitingToBeRead();
}

string
SChannel::TransceiverI::protocol() const
{
    return _instance->protocol();
}

string
SChannel::TransceiverI::toString() const
{
    return _delegate->toString();
}

string
SChannel::TransceiverI::toDetailedString() const
{
    return toString();
}

Ice::ConnectionInfoPtr
SChannel::TransceiverI::getInfo() const
{
    auto info = std::make_shared<ConnectionInfo>();
    info->underlying = _delegate->getInfo();
    info->incoming = _incoming;
    info->adapterName = _adapterName;
    info->certs = _certs;
    info->host = _host;
    info->desc = toString();
    return info;
}

void
SChannel::TransceiverI::checkSendSize(const IceInternal::Buffer&)
{
}

void
SChannel::TransceiverI::setBufferSize(int rcvSize, int sndSize)
{
    _delegate->setBufferSize(rcvSize, sndSize);
}

SChannel::TransceiverI::TransceiverI(
    const InstancePtr& instance,
    const IceInternal::TransceiverPtr& delegate,
    const string& adapterName,
    const Ice::SSL::ServerAuthenticationOptions& serverAuthenticationOptions)
    : _instance(instance),
      _engine(dynamic_pointer_cast<SChannel::SSLEngine>(instance->engine())),
      _host(""),
      _adapterName(adapterName),
      _incoming(true),
      _delegate(delegate),
      _state(StateNotInitialized),
      _bufferedW(0),
      _sslInitialized(false),
      _clientCertificateRequired(serverAuthenticationOptions.clientCertificateRequired),
      _localCredentialsSelectionCallback(serverAuthenticationOptions.serverCredentialsSelectionCallback),
      _remoteCertificateValidationCallback(serverAuthenticationOptions.clientCertificateValidationCallback)
{
}

SChannel::TransceiverI::TransceiverI(
    const InstancePtr& instance,
    const IceInternal::TransceiverPtr& delegate,
    const string& host,
    const Ice::SSL::ClientAuthenticationOptions& clientAuthenticationOptions)
    : _instance(instance),
      _engine(dynamic_pointer_cast<SChannel::SSLEngine>(instance->engine())),
      _host(host),
      _adapterName(""),
      _incoming(false),
      _delegate(delegate),
      _state(StateNotInitialized),
      _bufferedW(0),
      _sslInitialized(false),
      _clientCertificateRequired(false),
      _localCredentialsSelectionCallback(clientAuthenticationOptions.clientCredentialsSelectionCallback),
      _remoteCertificateValidationCallback(clientAuthenticationOptions.serverCertificateValidationCallback)
{
}

SChannel::TransceiverI::~TransceiverI() {}

bool
SChannel::TransceiverI::writeRaw(IceInternal::Buffer& buf)
{
    _delegate->write(buf);
    return buf.i == buf.b.end();
}

bool
SChannel::TransceiverI::readRaw(IceInternal::Buffer& buf)
{
    IceInternal::Buffer::Container::iterator p = buf.i;
    _delegate->read(buf);
    return buf.i != p;
}
