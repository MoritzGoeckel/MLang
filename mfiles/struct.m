# expect_result=95
struct Point {
    let x: Int;
    let y: Int;
    let z: Int;
}

let p: Point;

# TODO: Access fields
p.x = 18;
p.y = 95;
p.z = 42;

ret p.y;