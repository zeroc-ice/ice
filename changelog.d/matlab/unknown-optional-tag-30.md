- Fixed the unmarshaling of unknown optional values with tags greater than or equal to 30. These no
  longer desynchronize the input stream causing spurious `MarshalException`.
