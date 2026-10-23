- Fixed a replication bug where a replica rejoining the group could reuse the in-memory remnant of a topic it
  had destroyed locally. The rejoined replica then ignored a later replicated destroy of that topic, keeping the
  topic's records in its database and resurrecting the destroyed topic on restart.
