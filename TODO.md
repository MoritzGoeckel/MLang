# Roadmap

- terminal IO (possible with external library, but need intrinsics)
- file IO
- Floats
- Blob type (for raw memory)
  alloc8(size), get(blob, idx), set(blob, idx)
  synatx sugar for get, set with []. Use alloc, storew and loadw
  (for arrays, strings, maps)
- Arrays
- Dynamic Strings
- Garbage collection for heap objects (structs)
- Fix recursion example. Get return type of non recursive
  return and use as override. Ignore recursive return.
- Function overloading (name, number of parameters, type of parameters)
- Member functions for structs
- Throw when encountering a function that does not return on all paths
- Throw when finding a type annotation that does not name a type (struct or primitive)
- FFI for Windows