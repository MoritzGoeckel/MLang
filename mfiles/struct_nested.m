# expect_result=12
struct Point {
    let x: Int;
    let y: Int;
}

struct Line {
    let begin: Point;
    let someInt: Int;
    let end: Point;
}

let l: Line;
l.begin.x = 3;
l.begin.y = 4;

l.end.x = 8;
l.end.y = 9;

ret l.begin.x + l.end.y;