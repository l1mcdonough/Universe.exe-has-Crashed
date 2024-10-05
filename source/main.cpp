#include <game/world.hpp>

void orbital_camera(Camera& camera, const float camera_orbit_speed);

int main(int argc, char** args)
{
    const int screen_width = 1280;
    const int screen_height = 768;

    InitWindow(screen_width, screen_height, "Game");
    SetTargetFPS(60);
//
    Camera camera = { 0 };
    camera.position = Vector3{ .1f, 200.0f, 0.f }; // Camera position
    camera.target = Vector3{ 0.f, 0.f, 0.f }; // Camera looking at point
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f }; // Camera up vector (rotation towards target)
    camera.fovy = 180.0f; // Camera field-of-view Y
    camera.projection = CAMERA_ORTHOGRAPHIC; // Camera projection type
    Game::GameWorld world;
    for(size_t ii = 0; ii < 2000; ++ii)
        world.mutable_at(GetRandomValue(0, 255), GetRandomValue(0, 255), 0) = 1.f;
        for (size_t foodX = 50; foodX > 53; foodX++) {
            world.mutable_at(foodX, 50, 0) = 1.f;
        }
    //world.mutable_at(49, 50, 0) = 2;
    world.commit();
    size_t grid_update_period = 6;
    size_t frame = 0;
    const float camera_orbit_speed = .1f;
    DisableCursor();
    while (!WindowShouldClose())
    {
        orbital_camera(camera, camera_orbit_speed);
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

        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
                    world.draw(::Vector3{0,0,0}, Game::default_cell_colors);
                DrawGrid(100, 1.0f);
            EndMode3D();
            DrawFPS(10, 10);
        EndDrawing();
        ++frame;
        if (frame % grid_update_period == 0)
        {
            frame = 0;
            world.conway();
            world.commit();
        }
    }
    // Cleanup //
    CloseWindow();
    return 0;
}
void orbital_camera(Camera& camera, const float camera_orbit_speed)
{
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) == true)
    {
        ::Vector2 orbit_vector = Vector2Normalize(GetMouseDelta());
        orbit_vector.x *= camera_orbit_speed;
        orbit_vector.y *= camera_orbit_speed;
        Matrix yaw = MatrixRotate(GetCameraUp(&camera), orbit_vector.x);
        Matrix pitch = MatrixRotate(GetCameraRight(&camera), orbit_vector.y);
        Vector3 view = Vector3Subtract(camera.position, camera.target);
        view = Vector3Transform(view, yaw);
        camera.position = Vector3Add(camera.target, view);
        view = Vector3Transform(view, pitch);
        camera.position = Vector3Add(camera.target, view);
    }
    float zoom = GetMouseWheelMove();
    ::Vector3 camera_forward = GetCameraForward(&camera);
    camera_forward.x *= zoom;
    camera_forward.y *= zoom;
    camera_forward.z *= zoom;
    camera.position = Vector3Add(camera.position, camera_forward);
}