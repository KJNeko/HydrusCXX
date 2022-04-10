# HydrusCXX Version 0.1, Supporting Hydrus V480

A Hydrus database API written in C++

## Important notes

- Hydrus must be told to leave the database or must be closed
- HydrusCXX wil self terminate if Hydrus is not closed
- HydrusCXX will lock the database to prevent Hydrus from starting.
- HydrusCXX prioritises runtime speed over startup speed meaning that it can take awhile to parse the database. Do not
  use this API for small tasks.

# Feature set

- [ ] Tags
    - [X] Read
    - [ ] Write
- [ ] Mappings
    - [x] Read
    - [ ] Write
- [ ] Metadata
    - [ ] Read Metadata
- [ ] URLS
    - [x] Read
    - [ ] Write

Examples:

```cpp
int main()
{
	spdlog::info( "Starting HydruCXX" );

	//Sets debug level
	spdlog::set_level( spdlog::level::debug );

        HydrusCXX::HydruCXX hydrusDB("/path/to/hydrus/db/folder");
			
	return 0;
}
```

