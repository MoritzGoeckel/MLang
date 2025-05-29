# expect_result=null
let f() = {
    ret 10;
};
f();
ret; # We don't want to have the return of f() still on the stack
