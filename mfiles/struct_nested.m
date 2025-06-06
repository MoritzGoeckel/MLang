# expect_result=0
struct Point {
    let x: Int;
    let y: Int;
}

struct Line {
    let begin: Point;
    let end: Point;
}

# TODO: Access nested fields
let l: Line;
# l.begin.x = 14;

ret 0;