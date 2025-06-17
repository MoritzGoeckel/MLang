# expect_result=2
let f(x) = {
    if (x == 1) {
        ret 2;
    }
};
ret f(1);
