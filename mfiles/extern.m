# expect_result=15

let mul = extern test::mul(a: Int, b: Int): Int;

let i = 5;
let result = mul(i, 3);
ret result;
