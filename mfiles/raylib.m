
let str = "Hello, World!";

let InitWindow = extern raylib::InitWindow(w: Int, h: Int, s: String): Void;
InitWindow(800, 600, str);

let SetTargetFPS = extern raylib::SetTargetFPS(fps: Int): Void;
SetTargetFPS(60);

let WindowShouldClose = extern raylib::WindowShouldClose(): Bool;
let CloseWindow = extern raylib::CloseWindow(): Void;

let BeginDrawing = extern raylib::BeginDrawing(): Void;
let ClearBackground = extern raylib::ClearBackground(color: Int): Void;
let EndDrawing = extern raylib::EndDrawing(): Void;

let DrawText = extern raylib::DrawText(text: String, x: Int, y: Int, fontSize: Int, color: Int): Void;

while(WindowShouldClose() == false) {
    BeginDrawing();
    ClearBackground(1);
    DrawText(str, 190, 200, 20, 1);
    EndDrawing();
}

CloseWindow();

ret 0;