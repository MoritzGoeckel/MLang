# expect_result=void

let s = "Hello, World!";
let str = s;

let print = extern test::print(s: String): Void;
print(str);
