# expect_result=22
let f(x) = { 
    let result = alloc x;
    return x;
};
let ptr = f(11);
return *ptr * 2;

// This is very c-like. I don't like it. But we need something to interface with c

let f(x) = { 
    let list = []
    list.add(10);
    list.add(x);
    return list;
};

let list = f(10);
let size = list.size
let elem = list[0]
let ptr = list.c_ptr

let obj = { 
    let x = 10;
    let y = 20;
    let add = (b) => { 
        return x + y + b; 
    };
}

# this could already be a string
let List = struct {
    let blob: Blob8;
    let capacity: Int;
    let size: Int;
    let add = (x) => {
        if (size >= capacity) {
            let new_blob = alloc8(capacity * 2);
            for (let i = 0; i < size; i++) {
                new_blob[i] = blob[i];
            }
            blob = new_blob;
            # Garbage collect old blob
            capacity *= 2;
        }
        blob[size] = x;
        size += 1;
    };
    let init = (x) => { 
        capacity = x;
        blob = alloc8(capacity);
    };
    let get = (index) => {
        return blob[index];
    };
};

let mine = List(10); # Structs always go to the heap

mine.add(10)