# expect_result=2
let f(x) = {
    if(x > 20) {
        let z = 1;
        ret z;
    }
    let y = 2;
    ret y;
};
ret f(10);
