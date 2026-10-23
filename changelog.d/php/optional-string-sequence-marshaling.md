- Fixed Ice for PHP mis-marshaling a non-empty optional `sequence<string>`: the size of the optional value was left as
  zero, so a peer that skipped this optional value, for example because it did not know its tag, failed to unmarshal
  the remainder of the message.
