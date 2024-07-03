// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import com.zeroc.IceInternal.Protocol;
import java.util.Objects;
import java.util.concurrent.CompletionException;
import java.util.function.BiConsumer;

/** Provides information about an incoming request being dispatched. */
public final class Current implements Cloneable {
  /** TThe object adapter that received the request. */
  public final ObjectAdapter adapter;

  /**
   * The connection that received the request. It's null when the invocation and dispatch are
   * collocated.
   */
  public final Connection con;

  /** The identity of the target Ice object. */
  public final Identity id;

  /** The facet of the target Ice object. */
  public String facet;

  /** The name of the operation. */
  public String operation;

  /** The operation mode (idempotent or not). */
  public OperationMode mode;

  /** The request context. */
  public final java.util.Map<String, String> ctx;

  /** The request ID. 0 means the request is a one-way request. */
  public final int requestId;

  /** The encoding of the request payload. */
  public EncodingVersion encoding;

  /**
   * Creates a new Current object.
   *
   * @param adapter The adapter.
   * @param con The connection. Can be null.
   * @param id The identity of the target object.
   * @param facet The facet of the target object.
   * @param operation The name of the operation.
   * @param mode The operation mode.
   * @param ctx The request context.
   * @param requestId The request ID.
   * @param encoding The encoding of the payload.
   */
  public Current(
      ObjectAdapter adapter,
      Connection con,
      Identity id,
      String facet,
      String operation,
      OperationMode mode,
      java.util.Map<String, String> ctx,
      int requestId,
      EncodingVersion encoding) {
    // We may occasionally construct a Current with a null adapter, however we never return such a
    // current to the
    // application code.
    Objects.requireNonNull(id);
    Objects.requireNonNull(facet);
    Objects.requireNonNull(operation);
    Objects.requireNonNull(mode);
    Objects.requireNonNull(ctx);
    // Objects.requireNonNull(encoding);
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

  public Current clone() {
    Current clone = null;
    try {
      clone = (Current) super.clone();
    } catch (CloneNotSupportedException ex) {
      assert false; // impossible
    }
    return clone;
  }

  /**
   * Creates an outgoing response with reply status {@link ReplyStatus#Ok}.
   *
   * @param <TResult> The type of result.
   * @param result The result to marshal into the response payload.
   * @param marshal The action that marshals result into an output stream.
   * @param formatType The class format.
   * @return A new outgoing response.
   */
  public <TResult> OutgoingResponse createOutgoingResponse(
      TResult result, BiConsumer<OutputStream, TResult> marshal, FormatType formatType) {
    OutputStream ostr = startReplyStream();
    if (requestId != 0) {
      try {
        ostr.startEncapsulation(encoding, formatType);
        marshal.accept(ostr, result);
        ostr.endEncapsulation();
        return new OutgoingResponse(ostr);
      } catch (java.lang.Exception exception) {
        return createOutgoingResponse(exception);
      }
    } else {
      assert false : "A one-way request cannot return a response";
      return new OutgoingResponse(ostr);
    }
  }

  /**
   * Creates an empty outgoing response with reply status {@link ReplyStatus#Ok}.
   *
   * @return An outgoing response with an empty payload.
   */
  public OutgoingResponse createEmptyOutgoingResponse() {
    OutputStream ostr = startReplyStream();
    if (requestId != 0) {
      try {
        ostr.writeEmptyEncapsulation(encoding);
      } catch (java.lang.Exception ex) {
        return createOutgoingResponse(ex);
      }
    }
    return new OutgoingResponse(ostr);
  }

  /**
   * Creates an outgoing response with the specified payload.
   *
   * @param ok When true, the reply status of the response is {@link ReplyStatus#Ok}; otherwise,
   *     it's {@link ReplyStatus#UserException}.
   * @param encapsulation The payload of the response.
   * @return A new outgoing response.
   */
  public OutgoingResponse createOutgoingResponse(boolean ok, byte[] encapsulation) {
    // For compatibility with the Ice 3.7 and earlier.
    encapsulation = encapsulation != null ? encapsulation : new byte[0];

    OutputStream ostr = startReplyStream(ok ? ReplyStatus.Ok : ReplyStatus.UserException);

    if (requestId != 0) {
      try {
        if (encapsulation.length > 0) {
          ostr.writeEncapsulation(encapsulation);
        } else {
          ostr.writeEmptyEncapsulation(encoding);
        }
      } catch (Throwable ex) {
        return createOutgoingResponse(ex);
      }
    }
    return new OutgoingResponse(ostr);
  }

  /**
   * Creates an outgoing response that marshals an exception.
   *
   * @param exception The exception to marshal into the response payload.
   * @return A new outgoing response.
   */
  public OutgoingResponse createOutgoingResponse(Throwable exception) {
    assert exception != null;
    try {
      if (exception instanceof CompletionException completionException) {
        // Unwrap the completion exception.
        exception = completionException.getCause();
        assert exception != null;
      }

      return createOutgoingResponseCore(exception);
    } catch (Throwable ex) {
      // Try a second time with the marshal exception. This should not fail.
      return createOutgoingResponseCore(ex);
    }
  }

  private OutgoingResponse createOutgoingResponseCore(Throwable exc) {
    OutputStream ostr;

    if (requestId != 0) {
      ostr = new OutputStream(adapter.getCommunicator(), Protocol.currentProtocolEncoding);
      ostr.writeBlob(Protocol.replyHdr);
      ostr.writeInt(requestId);
    } else {
      ostr = new OutputStream();
    }

    ReplyStatus replyStatus;
    String exceptionId;
    String exceptionMessage;

    // TODO: replace by switch statement with Java 21
    if (exc instanceof RequestFailedException rfe) {
      exceptionId = rfe.ice_id();

      if (rfe instanceof ObjectNotExistException) {
        replyStatus = ReplyStatus.ObjectNotExist;
      } else if (rfe instanceof FacetNotExistException) {
        replyStatus = ReplyStatus.FacetNotExist;
      } else if (rfe instanceof OperationNotExistException) {
        replyStatus = ReplyStatus.OperationNotExist;
      } else {
        throw new MarshalException("Unexpected exception type");
      }

      if (rfe.id.name.length() == 0) {
        rfe.id = id;
      }

      if (rfe.facet.length() == 0 && facet.length() > 0) {
        rfe.facet = facet;
      }

      if (rfe.operation.length() == 0 && operation.length() > 0) {
        rfe.operation = operation;
      }

      // Called after fixing id, facet and operation.
      exceptionMessage = rfe.toString();

      if (requestId != 0) {
        ostr.writeByte(replyStatus.value());
        Identity.ice_write(ostr, rfe.id);

        if (rfe.facet.length() == 0) {
          ostr.writeStringSeq(new String[] {});
        } else {
          ostr.writeStringSeq(new String[] {rfe.facet});
        }

        ostr.writeString(rfe.operation);
      }
    } else if (exc instanceof UserException ex) {
      exceptionId = ex.ice_id();
      exceptionMessage = ex.toString();

      replyStatus = ReplyStatus.UserException;

      if (requestId != 0) {
        ostr.writeByte(replyStatus.value());
        ostr.startEncapsulation(encoding, FormatType.SlicedFormat);
        ostr.writeException(ex);
        ostr.endEncapsulation();
      }
    } else if (exc instanceof UnknownLocalException ex) {
      exceptionId = ex.ice_id();
      replyStatus = ReplyStatus.UnknownLocalException;
      exceptionMessage = ex.unknown;
    } else if (exc instanceof UnknownUserException ex) {
      exceptionId = ex.ice_id();
      replyStatus = ReplyStatus.UnknownUserException;
      exceptionMessage = ex.unknown;
    } else if (exc instanceof UnknownException ex) {
      exceptionId = ex.ice_id();
      replyStatus = ReplyStatus.UnknownException;
      exceptionMessage = ex.unknown;
    } else if (exc instanceof LocalException ex) {
      exceptionId = ex.ice_id();
      replyStatus = ReplyStatus.UnknownLocalException;
      exceptionMessage = ex.toString();
    } else if (exc instanceof com.zeroc.Ice.Exception ex) {
      exceptionId = ex.ice_id();
      replyStatus = ReplyStatus.UnknownException;
      exceptionMessage = ex.toString();
    } else {
      replyStatus = ReplyStatus.UnknownException;
      exceptionId =
          exc.getClass().getName() != null ? exc.getClass().getName() : "java.lang.Exception";
      exceptionMessage = exc.toString();
    }

    if (requestId != 0
        && (replyStatus == ReplyStatus.UnknownUserException
            || replyStatus == ReplyStatus.UnknownLocalException
            || replyStatus == ReplyStatus.UnknownException)) {
      ostr.writeByte(replyStatus.value());
      ostr.writeString(exceptionMessage);
    }

    return new OutgoingResponse(replyStatus, exceptionId, exceptionMessage, ostr);
  }

  /**
   * Starts the output stream for a successful reply, with everything up to and including the reply
   * status. When the request ID is 0 (one-way request), the returned output stream is empty.
   *
   * @return The new output stream with status ReplyStatus.Ok.
   */
  public OutputStream startReplyStream() {
    return startReplyStream(ReplyStatus.Ok);
  }

  /**
   * Starts the output stream for a reply, with everything up to and including the reply status.
   * When the request ID is 0 (one-way request), the returned output stream is empty.
   *
   * @param replyStatus The reply status.
   * @return The new output stream.
   */
  private OutputStream startReplyStream(ReplyStatus replyStatus) {
    if (requestId == 0) {
      return new OutputStream();
    } else {
      var ostr = new OutputStream(adapter.getCommunicator(), Protocol.currentProtocolEncoding);
      ostr.writeBlob(Protocol.replyHdr);
      ostr.writeInt(requestId);
      ostr.writeByte(replyStatus.value());
      return ostr;
    }
  }
}
