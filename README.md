# HydrusCXX

A Hydrus database API written in C++

# Feature set

- [ ] Tags
    - [X] Read
    - [ ] Write
    - [ ] Tag Relationships
- [ ] Mappings
    - [ ] Read
    - [ ] Write
    - [ ] Parse Tags
- [ ] Metadata
    - [ ] Read Metadata!
    - [ ] Verify Metadata
    - [ ] Read Metadata!
- [ ] URLS
    - [ ] Write
    - [ ] Read

# Master Class Table operations

### Hashes operations

- [ ] Completed

| Function                 | Args                                          | Return Type                         |
|--------------------------|-----------------------------------------------|-------------------------------------|
| getHashIdFromHash        | std::bitset<256> hash                         | uint hash_id                        |
| getHash                  | uint hash_id                                  | std::bitset<256> hash               |

### local_hashes operations

- [ ] Completed

| Function               | Args                       | Return Type                  |
|------------------------|----------------------------|------------------------------|
| getIDFromMD5           | std::bitset\<MD5LENGTH>    | uint hash_id                 |
| getIDFromSHA1          | std::bitset\<SHA1LENGTH>   | uint hash_id                 |
| getIDFromSHA512        | std::bitset\<SHA512LENGTH> | uint hash_id                 |
| getMD5                 | uint hash_id               | std::bitset\<MD5LENGTH>      |
| getSHA1                | uint hash_id               | std::bitset\<SHA1LENGTH>     |
| getSHA512              | uint hash_id               | std::bitset\<SHA512LENGTH>   |

### namespaces operations

- [x] Completed

| Function                 | Args                                          | Return Type            |
|--------------------------|-----------------------------------------------|------------------------|
| getNamespaceIDFromString | std::string namespace                         | uint namespace_id      |
| getNamespace             | uint namespace_id                             | std::string namespace  |

### subtags operations

- [x] Completed

| Function              | Args               | Return Type    |
|-----------------------|--------------------|----------------|
| getSubtagIDFromString | std::string subtag | uint subtag_id |
| getSubtag             | uint subtag_id     | std::string    |

### tags operations

- [x] Completed

| Function               | Args                                          | Return Type                         |
|------------------------|-----------------------------------------------|-------------------------------------|
| getTagIDFromPair       | uint namespace_id<br/> uint subtag_id         | uint tag_id                         |
| getTagIDFromStringPair | std::string namespace<br/> std::string subtag | uint tag_id                         |
| getTagPair             | uint tag_id                                   | std::pair<uint, uint>               |
| getTagPairString       | uint tag_id                                   | std::pair<std::string, std::string> |



