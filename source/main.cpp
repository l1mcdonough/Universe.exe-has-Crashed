#include <game/world.hpp>
#include <game/ray_extend.hpp>

using namespace Game::RayExtend;

void orbital_camera(Camera& camera, const float camera_orbit_speed);
void pause_display(bool sim_paused, size_t screen_height);
void draw_gizmo(
    Camera3D camera,
    Vector3 offset = Vector3{ .1f, .05f, 0.f },
    float arm_length = .01f,
    float sphere_radii_ratio = 1.f / 5.f
);
int main(int argc, char** args)
{
    const int screen_width = 1280;
    const int screen_height = 768;

    InitWindow(screen_width, screen_height, "Game");
    SetTargetFPS(60);
//
    Camera camera = { 0 };
    camera.position = Vector3{ 0.1f, 20.0f, 10.f }; // Camera position
    camera.target = Vector3{ 0.f, 0.f, 0.f }; // Camera looking at point
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f }; // Camera up vector (rotation towards target)
    camera.fovy = 90.0f; // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE; // Camera projection type
    Game::GameWorld world(Game::default_cell_colors);
    //for (size_t ii = 0; ii < 20; ++ii)
    //    world.mutable_at(world.dimensions().x / 2 + ii, world.dimensions().y / 2, 0) = 1;
    //for (size_t ii = 0; ii < 2000; ++ii)
        //world.mutable_at(GetRandomValue(0, world.dimensions().x - 1), GetRandomValue(0, world.dimensions().y - 1), 0) = 1;
    //for (size_t ii = 0; ii < 2000; ++ii)
    //{
    //    size_t x = GetRandomValue(0, world.dimensions().x - 1);
    //    size_t y = GetRandomValue(0, world.dimensions().y - 1);
    //    //uint8_t direction = GetRandomValue(0, 3) << 3;
    //    world.mutable_at(x, y, 0) = Game::is_langton_trail;
    //    //std::cout << (direction | Game::is_langton_trail) << "\n";
    //    //std::cout << (((direction | Game::is_langton_trail) & Game::is_langton_trail) == Game::is_langton_trail) << "\n";
    //    ////std::cout << (direction & Game::is_langton_ant) << "\n";
    //}
    for (size_t ii = 0; ii < 1; ++ii)
    {
        size_t x = GetRandomValue(0, world.dimensions().x - 1);
        size_t y = GetRandomValue(0, world.dimensions().y - 1);
        uint8_t direction = GetRandomValue(0, 3) << 3;
        std::cout << "Direction: " << (int) direction << "\n";
        world.mutable_at(x, y, 0) = (direction | Game::is_langton_ant);
        //std::cout << (((direction | Game::is_langton_ant) & Game::is_langton_ant ) == Game::is_langton_ant ) << "\n";
    }
    world.copy_mutable_buffer(std::array<uint8_t, 2>{0, 3});
    size_t grid_update_period = 6;
    size_t frame = 0;
    const float camera_orbit_speed = .1f;
    DisableCursor();
    bool pause_sim = true;
    bool show_gizmo = true;
    world.commit();
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

        ++frame;
        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
                    if(show_gizmo == true)
                        draw_gizmo(camera);
                    world.draw_3d(::Vector3{0.f, 0.f, 0.f});
                    DrawGrid(100, 1.0f);
            EndMode3D();
            DrawFPS(10, 10);
            pause_display(pause_sim, screen_height);
        EndDrawing();
        if (GetKeyPressed() == KEY_P)
            pause_sim = !pause_sim;
        if (IsKeyReleased(KEY_G) == true)
            show_gizmo = !show_gizmo;
        if (pause_sim == false)
        {
            if (frame % grid_update_period == 0)
            {
                frame = 0;
                //world.conway();
                world.langton(1);

                //std::cout << world.langton_position << "\n";
                world.commit();
            }
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

void pause_display(bool pause_sim, size_t screen_height)
{
    DrawText("Simulation: ", 10, screen_height - 20, 10, BLACK);
    if (pause_sim == false)
        DrawText("[RUNNING]", 70, screen_height - 20, 10, GREEN);
    else
        DrawText("[PAUSED]", 70, screen_height - 20, 10, RED);
}

void draw_gizmo(Camera3D camera, Vector3 offset, float arm_length, float sphere_radii_ratio)
{
    Vector3 forward = GetCameraForward(&camera);
    Vector3 right = GetCameraRight(&camera);
    Vector3 up = GetCameraUp(&camera);
    float sphere_radii = arm_length * sphere_radii_ratio;
    Vector3 center = camera.position + (forward * offset.x) + (right * offset.z) + (up * offset.y);
    Vector3 end_x = center + Vector3{ arm_length, 0.f, 0.f };
    Vector3 end_y = center + Vector3{ 0.f, arm_length, 0.f };
    Vector3 end_z = center + Vector3{ 0.f, 0.f, arm_length };
    DrawCylinderEx(center, end_x, sphere_radii, sphere_radii, 10, GREEN);
    DrawCylinderEx(center, end_y, sphere_radii, sphere_radii, 10, BLUE);
    DrawCylinderEx(center, end_z, sphere_radii, sphere_radii, 10, RED);
    DrawSphere(end_x, sphere_radii, GREEN);
    DrawSphere(end_y, sphere_radii, BLUE);
    DrawSphere(end_z, sphere_radii, RED);
}