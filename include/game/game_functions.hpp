#include <game/common.hpp>

namespace Game
{
    using namespace RayExtend;



    inline void draw_controls(const size_t screen_width, const size_t screen_height)
    {
        static const auto controls = std::array{
            "Toggle This Menu:           H",
            "Move Cell Placer +X:        D",
            "Move Cell Placer -X:        A",
            "Move Cell Placer +Z:        W",
            "Move Cell Placer -Z:        S",
            "Move Cell Placer +Y:        E",
            "Move Cell Placer -Y:        Q",
            "Place Cell:             SPACE",
            "Random Cells (Selected):    R",
            "Reset Grid:                 0",
            "Pause/Unpause Simulation:   P",
            "Toggle Gizmo:               G",
            "Toggle Orthographic Camera: O",
            "Toggle Grid Lines:          L",
            "Toggle Grid Box:            B",
            "Zoom In/Out:      Mouse Wheel",
            "Settings Menu:         ESCAPE",
            "Quit to Desktop:            ;",
            "Rotate Camera: Hold Right\n    Click and Move Mouse"
        };
        static const size_t longest_string_index = find_longest_string(controls);
        const int font_size = 12;
        const size_t text_width = MeasureText(controls[longest_string_index], font_size);
        const size_t y_start = 48;
        const size_t x_offset = text_width + 48;
        const size_t line_count = controls.size() + 8;
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




}
