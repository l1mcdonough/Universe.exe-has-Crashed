#include <game/world.hpp>

int main(int argc, char** args)
{
    const int screen_width = 1280;
    const int screen_height = 768;

    InitWindow(screen_width, screen_height, "Game");
    SetTargetFPS(60);
//
    Camera camera = { 0 };
    camera.position = Vector3{ 10.f, 10.0f, 0.f }; // Camera position
    camera.target = Vector3{ 0.f, 0.f, 0.f }; // Camera looking at point
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f }; // Camera up vector (rotation towards target)
    camera.fovy = 45.0f; // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;// CAMERA_ORTHOGRAPHIC; // Camera projection type
    Game::GameWorld world;
    for(size_t ii = 0; ii < 2000; ++ii)
        world.mutable_at(GetRandomValue(0, 255), GetRandomValue(0, 255), 0) = 1.f;
    world.commit();
    std::cout << " CELL " << world.read_at(16 / 2, 16 / 2, 0) << "\n";
    std::cout << " CELL " << world.read_at(16 / 2+1, 16 / 2+1, 0) << "\n";
    while (!WindowShouldClose())
    {
        Vector2 mouse_position = GetMousePosition();
//        std::cout << camera.up.x << ", "
//            << camera.up.y << ", "
//            << camera.up.z << "\n";
//            //// Check if the left mouse button is clicked
//        //if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
//        //    // Calculate the column and row of the rectangle clicked
//        //    int clickedColumn = mousePosition.x / rectSize;
//        //    int clickedRow = mousePosition.y / rectSize;
//        //    grid[clickedRow][clickedColumn] = 255;
//        //}
        UpdateCamera(&camera, CAMERA_FREE);
        DisableCursor();
        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
                    world.draw(camera.position, Game::default_cell_colors);
                DrawGrid(100, 1.0f);
            EndMode3D();
            DrawFPS(10, 10);
        EndDrawing();
    }
    // Cleanup //
    CloseWindow();
    return 0;
}
