//#include <game/grid.hpp>
//#include <game/cubeplacement.hpp>
//#include <game/ray_extend.hpp>
//
//namespace Game
//{
//
//
//    void pause_display(bool pause_sim, size_t screen_height)
//    {
//        DrawText("Simulation: ", 10, screen_height - 20, 10, BLACK);
//        if (pause_sim == false)
//            DrawText("[RUNNING]", 70, screen_height - 20, 10, GREEN);
//        else
//            DrawText("[PAUSED]", 70, screen_height - 20, 10, RED);
//    }
//
//    void draw_gizmo(Camera3D camera, Vector3 offset, float arm_length, float sphere_radii_ratio)
//    {
//        Vector3 forward = GetCameraForward(&camera);
//        Vector3 right = GetCameraRight(&camera);
//        Vector3 up = GetCameraUp(&camera);
//        float sphere_radii = arm_length * sphere_radii_ratio;
//        Vector3 center = camera.position + (forward * offset.x) + (right * offset.z) + (up * offset.y);
//        Vector3 end_x = center + Vector3{ arm_length, 0.f, 0.f };
//        Vector3 end_y = center + Vector3{ 0.f, arm_length, 0.f };
//        Vector3 end_z = center + Vector3{ 0.f, 0.f, arm_length };
//        DrawCylinderEx(center, end_x, sphere_radii, sphere_radii, 10, GREEN);
//        DrawCylinderEx(center, end_y, sphere_radii, sphere_radii, 10, BLUE);
//        DrawCylinderEx(center, end_z, sphere_radii, sphere_radii, 10, RED);
//        DrawSphere(end_x, sphere_radii, GREEN);
//        DrawSphere(end_y, sphere_radii, BLUE);
//        DrawSphere(end_z, sphere_radii, RED);
//    }
//
//}