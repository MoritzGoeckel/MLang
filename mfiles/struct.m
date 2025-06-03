# expect_result=3
let i = 1;

struct Point {
    let x: Int;
    let y: Int;
}

# TODO: Instatiate (heap allocation, garbage collection)
let p: Point;

# TODO: Access fields
p.x = 1;
# p.y = 2;

struct Line {
    let begin: Point;
    let end: Point;
}

# TODO: Access nested fields
let l: Line;
# l.begin.x = 1;

let j = 2;
ret i + j;
