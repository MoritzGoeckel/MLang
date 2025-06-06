# expect_result=3
let i = 1;

struct Point {
    let x: Int;
    let y: Int;
}

# TODO: Instatiate (heap allocation, garbage collection)
let p: Point;

# TODO: Access fields
p.x = 18;
p.y = 99;

struct Line {
    let begin: Point;
    let end: Point;
}

# TODO: Access nested fields
let l: Line;
l.begin.x = 14;

let j = 2;
ret i + j;
