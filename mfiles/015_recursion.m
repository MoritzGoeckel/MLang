let f(x) = {
    if(x > 100){
        ret x;
    }
    else{
        ret f(x * x);
    }
};
let y = f(3);
ret y;
