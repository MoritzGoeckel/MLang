# expect_result=15

# TODO: Implement parsing for extern, register extern functions to use in call later
let mul = extern libprint::mul(a: Int, b: Int): Int;

# TODO: Implement calls to extern functions
let i = 5;
let result = mul(i, 3);
ret result;
