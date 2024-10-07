#include <game/grid.hpp>
#include <game/cubeplacement.hpp>
#include <game/ray_extend.hpp>
#include <game/game_functions.hpp>

#ifndef GAME_GAME_0_HPP_HEADER_INCLUDE_GUARD
#define GAME_GAME_0_HPP_HEADER_INCLUDE_GUARD
namespace Game
{
    using namespace RayExtend;


    template<typename Grid_T>
    struct Game0
    {
        Grid_T grid;
        size_t screen_width;
        size_t screen_height;

        size_t grid_update_period = 6;
        size_t frame = 0;
        const float camera_orbit_speed = .1f;
        bool pause_sim;
        bool show_gizmo = true;
        bool camera_orthographic = false;


        Camera camera;
        CubePlacement cubePlacement;
        Game0(
                ColorsType colors_, 
                size_t screen_width_ = 1280, 
                size_t screen_height_ = 768, 
                bool pause_sim_ = true, 
                size_t grid_update_period_ = 6, 
                size_t frame_ = 0, 
                const float camera_orbit_speed_ = .1f, 
                bool show_gizmo_ = true, 
                bool camera_orthographic_ = false, 
                std::optional<Camera> camera_option = std::nullopt
        ) : grid(colors_), 
            screen_width(screen_width_), 
            screen_height(screen_height_), 
            pause_sim(pause_sim_), 
            grid_update_period(grid_update_period_), 
            frame(frame_), 
            camera_orbit_speed(camera_orbit_speed_), 
            show_gizmo(show_gizmo_), 
            camera_orthographic(camera_orthographic_), 
            cubePlacement(Index3{48, 48, 16})
        {
            InitWindow(screen_width, screen_height, "Game");
            SetTargetFPS(60);
            if (camera_option == std::nullopt)
            {
                camera = { 0 };
                //camera.position = Vector3{ 10.1f, 200.0f, 0.f }; // Camera position
                camera.target = Vector3{ 0.f, 0.f, 0.f }; // Camera looking at point
                camera.up = Vector3{ 0.0f, 1.0f, 0.0f }; // Camera up vector (rotation towards target)
                camera.position = Vector3{ 0.1f, 20.0f, 10.f };
                camera.fovy = 90.0f; // Default
                camera.projection = CAMERA_PERSPECTIVE; // Default
            }
            else
                camera = camera_option.value();
            //SetExitKey(KEY_SEMICOLON);
            //grid.copy_mutable_buffer(std::array<uint8_t, 2>{0, 3});
            DisableCursor();
            //grid.commit();
        }
        ~Game0() {
            CloseWindow();
        }

        void draw(int key)
        {
            ++frame;
            const auto grid3d_center = ::Vector3{ 0.f, 0.f, 0.f };
            const auto grid_dimension_max = std::max(
                std::max(grid.dimensions().x, grid.dimensions().y), 
                grid.dimensions().z
            );
            BeginDrawing();
                ClearBackground(RAYWHITE);
                    BeginMode3D(camera);
                BeginBlendMode(BLEND_ALPHA);
                        if (show_gizmo == true)
                            draw_gizmo(camera);
                        cubePlacement.processCubePlacement(&grid, key);
                        DrawGrid(grid_dimension_max, 1.0f);
                        grid.draw_3d(grid3d_center);
                        //fractal_grid.draw_3d(::Vector3{0.f, 0.f, 0.f});
                        grid.draw_box_3d(grid3d_center);
                EndBlendMode();
                    EndMode3D();
                    DrawFPS(10, 10);
                    pause_display(pause_sim, screen_height);
                    cubePlacement.drawCellTypeName(screen_width, screen_height);
            EndDrawing();
        }

        int input()
        {
            int key = GetKeyPressed();
            if (IsKeyReleased(KEY_O) == true)
            {
                camera_orthographic = !camera_orthographic;
                if (camera_orthographic == true) {
                    camera.fovy = 180.0f;
                    camera.projection = CAMERA_ORTHOGRAPHIC;
                }
                else {
                    camera.fovy = 90.0f;
                    camera.projection = CAMERA_PERSPECTIVE;
                }
            }
            if (key == KEY_P)
                pause_sim = !pause_sim;
            orbital_camera(camera, camera_orbit_speed);
            return key;
        }

        void simulate()
        {
            if (pause_sim == false)
            {
                grid.set_grid_alpha(255);
                if (frame % grid_update_period == 0)
                {
                    frame = 0;
                    //fractal_grid.fractal();
                    //fractal_grid.commit();
                    grid.conway();
                    grid.langton();
                    grid.commit();
                }
            }
            else
                grid.set_grid_alpha(128);
        }

        void play()
        {
            while (!WindowShouldClose()) {
                draw(input());
                simulate();
            }
        }

    };

    //using DefaultGame0 = Game0<GameGrid>;
    //    Grid<DefaultCellType, 256, 256, 1>,
    //    Grid<DefaultCellType, 48, 48, 16>,
    //    Grid<DefaultCellType, 64, 64, 1>,
    //    Grid<DefaultCellType, 48, 48, 32>
    //>;
}
#endif // GAME_GAME_0_HPP_HEADER_INCLUDE_GUARD
