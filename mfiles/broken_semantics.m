# expect_failing_type_inference
let f(x) = {
    let y = z + 2;
    ret y + 3;
};
ret f(10);
