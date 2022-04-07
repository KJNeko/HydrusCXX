# HydrusCXX

A Hydrus database API written in C++

# Feature set

- [ ] Tags
    - [X] Read
    - [ ] Write
    - [ ] Tag Relationships
- [ ] Mappings
    - [x] Read
    - [ ] Write
    - [ ] Parse Tags
- [ ] Metadata
    - [ ] Read Metadata!
    - [ ] Verify Metadata
    - [ ] Read Metadata!
- [ ] URLS
    - [x] Read
    - [ ] Write

Examples:

```cpp
int main()
{
	spdlog::info( "Starting HydruCXX" );

	spdlog::set_level( spdlog::level::debug );

	//JsonParser ptr;
	//ptr.parse(
	//		"/home/kj16609/Desktop/Projects/hydrusCXX/7f59a664fb4464f0d8cf63b1a0ea560743c009e20845b37894c1d72d8086451c" );

	//ptr.parse(
	//		"/home/kj16609/Desktop/Projects/hydrusCXX/7f062a8810ad3cb0a52cbaa4b864a92030e600cc413646bc255ae85a95693bea" );

	stopwatch::Stopwatch watch( "Load all data into memory" );
	watch.start();

	Mappings mappingDB { "/home/kj16609/Desktop/Projects/hydrus/db/client.mappings.db", true };

	Master masterDB { "/home/kj16609/Desktop/Projects/hydrus/db/client.master.db", true };

	Main mainDB { "/home/kj16609/Desktop/Projects/hydrus/db/client.db", true };

	watch.stop();

	std::stringstream ss;
	ss << watch;
	spdlog::debug( ss.str());

	massTagTest( mappingDB, masterDB );
	singleTest( mappingDB, masterDB );

	return 0;
}
```

# Getting tags from an image hash_id

```cpp
void singleTest( Mappings& map, Master& master )
{
stopwatch::Stopwatch watch( "singleTest: imageFetch" );
size_t hash_id { 1337 };

	watch.start();
	std::vector<size_t> tagList = map.getTags( hash_id );

	std::vector<std::string> strs = master.getTagStrings( tagList );

	watch.stop();


	std::stringstream ss;
	ss << watch;
	spdlog::debug( ss.str());
	spdlog::debug( "Number of tags returned: " + std::to_string( tagList.size()));

}`````
