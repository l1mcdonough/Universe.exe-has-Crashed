#include <game/grid.hpp>
#include <game/cubeplacement.hpp>
#include <game/ray_extend.hpp>
#include <game/game_functions.hpp>

#ifndef GAME_GAME_0_HPP_HEADER_INCLUDE_GUARD
#define GAME_GAME_0_HPP_HEADER_INCLUDE_GUARD
namespace Game
{
    using namespace RayExtend;


    struct Window
    {
        size_t screen_width;
        size_t screen_height;
        Window(
            size_t screen_width_,
            size_t screen_height_,
            std::string_view title
        ) : screen_width(screen_width_), screen_height(screen_height_) {
            InitWindow(screen_width, screen_height, "Game");
        }
        void close() {
            CloseWindow();
            closed = true;
        }
        ~Window() {
            if (closed == false) CloseWindow();
        }
    protected:
        bool closed = false;
    };

    struct ApplicationBase
    {
        Window window;
        ApplicationBase(
            size_t screen_width,
            size_t screen_height,
            std::string_view title
        ) : window(screen_width, screen_height, title) {}


        virtual bool settings_are_open() = 0;
        virtual void title_screen(int key) = 0;
        virtual void settings_menu(int key) = 0;
        virtual void run() = 0;
        virtual void open_settings() = 0;
    };

    template<typename Grid_T>
    struct Game0
    {
        Grid_T grid;
        //size_t screen_width;
        //size_t screen_height;
        ApplicationBase& application;
        size_t grid_update_period;
        size_t frame;
        const float camera_orbit_speed;
        bool pause_sim;
        bool show_gizmo;
        bool camera_orthographic;
        bool display_controls;
        bool display_grid_box;
        bool display_grid_lines;

        Camera camera;
        CubePlacement cubePlacement;
        Game0(
            ColorsType colors_,
            ApplicationBase& application_,
            //size_t screen_width_ = 1280, 
            //size_t screen_height_ = 768, 
            bool pause_sim_ = true,
            size_t grid_update_period_ = 6,
            size_t frame_ = 0,
            const float camera_orbit_speed_ = .1f,
            bool show_gizmo_ = false,
            bool camera_orthographic_ = false,
            bool display_controls_ = true,
            bool display_grid_box_ = true, 
            bool display_grid_lines_ = true, 
            std::optional<Camera> camera_option = std::nullopt
        ) : grid(colors_),
            application(application_),
            //screen_width(screen_width_), 
            //screen_height(screen_height_), 
            pause_sim(pause_sim_),
            grid_update_period(grid_update_period_),
            frame(frame_),
            camera_orbit_speed(camera_orbit_speed_),
            show_gizmo(show_gizmo_),
            camera_orthographic(camera_orthographic_),
            display_controls(display_controls_),
            display_grid_box(display_grid_box_), 
            display_grid_lines(display_grid_lines_), 
            cubePlacement(grid.dimensions())
        {
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
            SetExitKey(KEY_SEMICOLON);
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
            SetExitKey(KEY_SEMICOLON);
            BeginDrawing();
            ClearBackground(RAYWHITE);
            if (application.settings_are_open() == true)
                application.settings_menu(key);
            else
            {
                BeginMode3D(camera);
                    BeginBlendMode(BLEND_ALPHA);
                        if (show_gizmo == true)
                            draw_gizmo(camera);
                        cubePlacement.processCubePlacement(&grid, key);
                        if(display_grid_lines == true)
                            DrawGrid(grid_dimension_max, 1.0f);
                        grid.draw_3d(grid3d_center);
                        //fractal_grid.draw_3d(::Vector3{0.f, 0.f, 0.f});
                        if(display_grid_box == true)
                           grid.draw_box_3d(grid3d_center);
                    EndBlendMode();
                EndMode3D();
                DrawFPS(10, 10);
                pause_display(pause_sim, application.window.screen_height);
                cubePlacement.drawCellTypeName(application.window.screen_width, application.window.screen_height);
                if (display_controls == true)
                    draw_controls(application.window.screen_width, application.window.screen_height);
            }
            EndDrawing();
        }

        int input(int key)
        {
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
            if (key == KEY_H)
                display_controls = !display_controls;
            if (key == KEY_ESCAPE)
                application.open_settings();
            if (key == KEY_L)
                display_grid_lines = !display_grid_lines;
            if (key == KEY_B)
                display_grid_box = !display_grid_box;
            if (key == KEY_G)
                show_gizmo = !show_gizmo;
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
                    grid.anti_conway();
                    grid.conway_crystalizer();
                    grid.grow_mold();
                    grid.commit();
                }
            }
            else
                grid.set_grid_alpha(128);
        }

        void play(int key) {
            draw(input(key));
            simulate();
        }

    };
}
#endif // GAME_GAME_0_HPP_HEADER_INCLUDE_GUARD
