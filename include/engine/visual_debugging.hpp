#include <engine/common.hpp>

#ifndef UNIVERSE_EXE__ENGINE__VISUAL_DEBUGGING_HPP_HEADER_INCLUDE_GUARD 
#define UNIVERSE_EXE__ENGINE__VISUAL_DEBUGGING_HPP_HEADER_INCLUDE_GUARD 
namespace Engine
{ 
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

    void camera_debug_display(Camera camera)
    {
        const std::string camera_position_string = cat(
            "camera_potition(x: ", 
            camera.position.x, 
            ", y: ", 
            camera.position.y, 
            ", z: ", 
            camera.position.z,
            ")"
        );
        const Vector3 camera_forward = GetCameraForward(&camera);
        const std::string camera_direction_string = cat(
            "camera_direction(.x:", 
            camera_forward.x, 
            ", y: ", 
            camera_forward.y, 
            ", z: ", 
            camera_forward.z,
            ")"
        );
        static const auto status = std::array{
            camera_position_string,
            camera_direction_string, 
            cat("fov-y: ", camera.fovy)
        };
        static const size_t longest_string_index = find_longest_string(status);
        const int font_size = 12;
        const size_t text_width = MeasureText(status[longest_string_index].c_str(), font_size);
        const size_t x_offset = text_width + 48;
        const size_t text_x = x_offset + 8;
        const size_t y_start = 48;
        const size_t line_count = status .size() + 8;
        const size_t line_offset = 5;
        for(size_t ii = 0; ii < status.size(); ++ii)
            DrawText(status[ii].c_str(), text_x, y_start + font_size * (ii + line_offset), font_size, GREEN);
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
#endif // UNIVERSE_EXE__ENGINE__VISUAL_DEBUGGING_HPP_HEADER_INCLUDE_GUARD 

