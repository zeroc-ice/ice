//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/** Unknown sliced value holds an instance of an unknown Slice class type. */
public final class UnknownSlicedValue extends Value {
  /**
   * Represents an instance of a Slice class type having the given Slice type.
   *
   * @param unknownTypeId The Slice type ID of the unknown object.
   */
  public UnknownSlicedValue(String unknownTypeId) {
    _unknownTypeId = unknownTypeId;
  }

  /**
   * Determine the Slice type ID associated with this object.
   *
   * @return The type ID.
   */
  @Override
  public String ice_id() {
    return _unknownTypeId;
  }

  private final String _unknownTypeId;

  /**
   * @hidden
   */
  public static final long serialVersionUID = 0L;
}
