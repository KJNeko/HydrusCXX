# HydrusCXX Version 0.1, Supporting Hydrus V480

A Hydrus database API written in C++

## Important notes

- WIP
- TODO: this

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

