#include <game/common.hpp>

namespace Game
{
    using namespace RayExtend;

    inline size_t find_longest_string(auto& strings)
    {
        size_t longest = 0;
        size_t longest_index = 0;
        for (size_t ii = 0; ii < strings.size(); ++ii)
        {
            const size_t current = std::string_view{ strings[ii] }.size();
            if (current > longest) {
                longest = current;
                longest_index = ii;
            }
        }
        return longest_index;
    }

    inline void draw_controls(const size_t screen_width, const size_t screen_height)
    {
        static const auto controls = std::array{
            "Toggle This Menu:         H",
            "Move Cell Placer +X:      D",
            "Move Cell Placer -X:      A",
            "Move Cell Placer +Z:      W",
            "Move Cell Placer -Z:      S",
            "Move Cell Placer +Y:      E",
            "Move Cell Placer -Y:      Q",
            "Place Cell:           SPACE",
            "Random Cells (Selected):  R",
            "Pause/Unpause Simulation: P",
            "Toggle Gizmo:             G",
            "Zoom In/Out:    Mouse Wheel",
            "Settings Menu:       ESCAPE",
            "Quit to Desktop:          ;", 
            "Rotate Camera: Hold Right\n    Click and Move Mouse"
        };
        static const size_t longest_string_index = find_longest_string(controls);
        const int font_size = 12;
        const size_t text_width = MeasureText(controls[longest_string_index], font_size);
        const size_t y_start = 48;
        const size_t x_offset = text_width + 48;
        const size_t line_count = 22;
        DrawRectangle(screen_width - x_offset - 16, y_start - 8, x_offset + 16, font_size * line_count+ 16, BLACK);
        DrawRectangle(screen_width - x_offset - 8, y_start, x_offset, font_size * line_count, RAYWHITE);
        const size_t text_x = screen_width - x_offset + 8;
        const char* title = "[Controls]";
        const size_t title_width = MeasureText(title, font_size);
        DrawText(title, text_x + text_width / 2 - title_width /2, y_start + font_size * 1, font_size, BLACK);
        const size_t line_offset = 5;
        for(size_t ii = 0; ii < controls.size(); ++ii)
            DrawText(controls[ii], text_x, y_start + font_size * (ii + line_offset), font_size, BLACK);
    }

    inline void orbital_camera(Camera& camera, const float camera_orbit_speed)
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

    inline void draw_gizmo(
        Camera3D camera,
        Vector3 offset = Vector3{ .1f, .05f, 0.f },
        float arm_length = .01f,
        float sphere_radii_ratio = 1.f / 5.f
    )
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

    inline void pause_display(bool pause_sim, size_t screen_height)
    {
        DrawText("Simulation: ", 10, screen_height - 20, 10, BLACK);
        if (pause_sim == false)
            DrawText("[RUNNING]", 70, screen_height - 20, 10, GREEN);
        else
            DrawText("[PAUSED]", 70, screen_height - 20, 10, RED);
    }


}
