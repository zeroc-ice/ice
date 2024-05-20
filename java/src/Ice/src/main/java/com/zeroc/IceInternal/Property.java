//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public class Property {
  public Property(String pattern, boolean usesRegex, String defaultValue, boolean deprecated) {
    _pattern = pattern;
    _usesRegex = usesRegex;
    _defaultValue = defaultValue;
    _deprecated = deprecated;
  }

  public String pattern() {
    return _pattern;
  }

  public boolean usesRegex() {
    return _usesRegex;
  }

  public String defaultValue() {
    return _defaultValue;
  }

  public boolean deprecated() {
    return _deprecated;
  }

  private String _pattern;
  private boolean _usesRegex;
  private String _defaultValue;
  private boolean _deprecated;
}
