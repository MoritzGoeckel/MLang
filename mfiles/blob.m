# expect_result=16

let b = alloc8(10); # Allocate 10 bytes of memory
set(b, 3, 16); # Set the 4th byte to 16 (index starts at 0)

# TODO: What should be the type of result here? Maybe byte?
let result = get(b, 3); # Get the value at the 4th byte

ret result;

# blob should be collected by the garbage collector