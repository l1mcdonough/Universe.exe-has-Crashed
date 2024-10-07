#include <game/common.hpp>

namespace Game
{
    using namespace RayExtend;

    inline void draw_controls()
    {
        //DrawRect()

        //DrawText("Move Cube Placer +Z: W", )
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
