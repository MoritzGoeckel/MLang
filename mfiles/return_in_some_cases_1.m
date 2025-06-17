# expect_result=void
let f(x) = {
    if (x == 1) {
        ret 2;
    }
};
ret f(2);
