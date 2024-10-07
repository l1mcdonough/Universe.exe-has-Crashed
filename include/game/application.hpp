#include <game/game_0.hpp>

#ifndef GAME_APPLICATION_HPP_HEADER_INCLUDE_GUARD
#define GAME_APPLICATION_HPP_HEADER_INCLUDE_GUARD
namespace Game
{
    using GridTypes = std::tuple<
        Grid<DefaultCellType, 48, 48, 16>,
        Grid<DefaultCellType, 256, 256, 1>,
        Grid<DefaultCellType, 64, 64, 1>,
        Grid<DefaultCellType, 48, 48, 32>
    >;
    constexpr inline const size_t grid_type_count = 4;

    using Game0Variant = std::variant<
        Game0<Grid<DefaultCellType, 48, 48, 16>>,
        Game0<Grid<DefaultCellType, 256, 256, 1>>,
        Game0<Grid<DefaultCellType, 64, 64, 1>>,
        Game0<Grid<DefaultCellType, 48, 48, 32>>
    >;

    template<typename Tuple, size_t I>
    using TupleTypeAt = typename std::decay_t<decltype(std::get<I>(std::declval<Tuple>()))>;

    inline Index3 game_0_grid_dimensions(size_t grid_type)
    {
        switch (grid_type)
        {
        case 0:
            return TupleTypeAt<GridTypes, 0>::grid_dimensions;
        case 1:
            return TupleTypeAt<GridTypes, 1>::grid_dimensions;
        case 2:
            return TupleTypeAt<GridTypes, 2>::grid_dimensions;
        case 3:
            return TupleTypeAt<GridTypes, 3>::grid_dimensions;
        default:
            return game_0_grid_dimensions(grid_type % 4);
        }
    }

    inline Game0Variant* make_game0(size_t grid_type, ApplicationBase& application, ColorsType colors)
    {
        switch (grid_type)
        {
        case 0:
            return new Game0Variant(std::in_place_index<0>, colors, application);
        case 1:
            return new Game0Variant(std::in_place_index<1>, colors, application);
        case 2:
            return new Game0Variant(std::in_place_index<2>, colors, application);
        case 3:
            return new Game0Variant(std::in_place_index<3>, colors, application);
        default:
            return make_game0(grid_type % 4, application, colors);
        }
    }

    static const auto screen_widths = std::array{ 1280, 768 };
    static const auto screen_heights = std::array{ 768, 480 };
    constexpr const char* default_title = "Game";
    struct Application : public ApplicationBase
    {
        size_t current_resolution = 0;
        size_t current_grid_type = 0;
        size_t select_resolution = 0;
        size_t select_grid_type = 0;
        int8_t current_option = 0;
        bool settings_open = false;
        bool game_run = false;
        std::string_view title;
        Game0Variant* game = nullptr;
        bool exit = false;
        Application(
            size_t screen_width = 1280,
            size_t screen_height = 768,
            std::string_view title_ = default_title
        ) : ApplicationBase(screen_width, screen_height, title_), title(title_), game(nullptr) {
            SetTargetFPS(60);
        }
        ~Application() { if (game == nullptr) delete game; }

        virtual void settings_menu(int key) override
        {
            if (key == KEY_DOWN)
                current_option = (current_option + 1) % 5;
            if (key == KEY_UP)
                current_option = (current_option - 1) % 5;
            if (current_option == 0)
            {
                if (key == KEY_LEFT)
                    select_resolution = (select_resolution - 1) % screen_widths.size();
                else if (key == KEY_RIGHT)
                    select_resolution = (select_resolution + 1) % screen_widths.size();
            }
            else if (current_option == 1)
            {
                if (key == KEY_LEFT)
                    select_grid_type = (select_grid_type - 1) % grid_type_count;
                else if (key == KEY_RIGHT)
                    select_grid_type = (select_grid_type + 1) % grid_type_count;
            }
            const Color selected_color = RED;
            const char* resolution_text = TextFormat(
                "Resolution: <%ix%i>",
                screen_widths[select_resolution],
                screen_heights[select_resolution]
            );
            auto grid_dimensions = game_0_grid_dimensions(select_grid_type);
            const char* grid_text = TextFormat(
                "Grid Dimensions: <%ix%ix%i>",
                grid_dimensions.x,
                grid_dimensions.y,
                grid_dimensions.z
            );

            size_t menu_width = window.screen_width / 2;
            size_t menu_height = window.screen_height / 3;
            size_t menu_inset_x = 4 * menu_width / 100;
            size_t menu_inset_y = 4 * menu_height / 100;
            DrawRectangle(
                window.screen_width / 2 - menu_width / 2,
                window.screen_height / 2 - menu_height / 2,
                menu_width,
                menu_height,
                BLACK
            );
            size_t start_x = window.screen_width / 2 - menu_width / 2 + menu_inset_x;
            size_t start_y = window.screen_height / 2 - menu_height / 2 + menu_inset_y;
            DrawRectangle(
                start_x,
                start_y,
                menu_width - menu_inset_x * 2,
                menu_height - menu_inset_y * 2,
                RAYWHITE
            );
            const size_t font_size = 32;
            DrawText(
                resolution_text,
                start_x + 8,
                start_y + 8,
                font_size,
                current_option == 0 ? selected_color : BLACK
            );
            DrawText(
                grid_text,
                start_x + 8,
                start_y + 8 + font_size + 16,
                font_size,
                current_option == 1 ? selected_color : BLACK
            );
            const char* accept_text = "[Accept]";
            const char* close_text = "[Close]";
            const char* return_to_title_text = "[Return to Title]";
            size_t text_width = MeasureText(accept_text, font_size);
            DrawText(
                accept_text,
                start_x + 8 + menu_width / 2 - menu_inset_x * 2 - text_width / 2,
                start_y + 8 + font_size * 3 + 16,
                font_size,
                current_option == 2 ? selected_color : BLACK
            );
            text_width = MeasureText(close_text, font_size);
            DrawText(
                close_text,
                start_x + 8 + menu_width / 2 - menu_inset_x * 2 - text_width / 2,
                start_y + 8 + font_size * 4 + 16,
                font_size,
                current_option == 3 ? selected_color : BLACK
            );
            text_width = MeasureText(return_to_title_text, font_size);
            DrawText(
                return_to_title_text,
                start_x + 8 + menu_width / 2 - menu_inset_x * 2 - text_width / 2,
                start_y + 8 + font_size * 5 + 16,
                font_size,
                current_option == 4 ? selected_color : BLACK
            );
            if (key == KEY_SPACE)
            {
                if (current_option == 2)
                {
                    if (select_resolution != current_resolution)
                    {
                        window = Window(
                            screen_widths[select_resolution],
                            screen_heights[select_resolution],
                            title
                        );
                        current_resolution = select_resolution;
                    }
                    if (select_grid_type != current_grid_type) {
                        current_grid_type = select_grid_type;
                        make_game();
                    }
                }
                else if (current_option == 3) {
                    settings_open = false;
                }
                else if (current_option == 4)
                {
                    game_run = false;
                    settings_open = false;
                }
            }
        }
        virtual void run() override
        {
            while (!WindowShouldClose() && exit == false)
            {
                int key = GetKeyPressed();
                if (game != nullptr && game_run == true)
                    std::visit([key](auto& game) { game.play(key); }, *game);
                else {
                    SetExitKey(KEY_ESCAPE);
                    BeginDrawing();
                    ClearBackground(RAYWHITE);
                    if (settings_open == true)
                        settings_menu(key);
                    else
                        title_screen(key);
                    EndDrawing();
                }
            }
        }
        virtual void title_screen(int key)
        {
            const char* title = "Cellular Automota!";
            const size_t title_font_size = 72;
            const size_t title_width = MeasureText(title, title_font_size);
            DrawText(
                title,
                window.screen_width / 2 - title_width / 2,
                2 * window.screen_height / 8,
                title_font_size,
                BLUE
            );
            if (key == KEY_DOWN)
                current_option = (current_option + 1) % 3;
            if (key == KEY_UP)
                current_option = (current_option - 1) % 3;
            if (key == KEY_SPACE)
            {
                if (current_option == 0) {
                    make_game();
                    game_run = true;
                }
                else if (current_option == 1)
                    settings_open = true;
                else if (current_option == 2)
                    exit = true;
            }
            const char* start_text = "[Start]";
            const char* settings_text = "[Settings]";
            const char* exit_text = "[Exit]";
            const size_t option_font_size = 32;
            const size_t start_width = MeasureText(start_text, option_font_size);
            const size_t settings_width = MeasureText(settings_text, option_font_size);
            const size_t exit_width = MeasureText(exit_text, option_font_size);
            DrawText(
                start_text,
                window.screen_width / 2 - start_width / 2,
                3 * window.screen_height / 8,
                title_font_size,
                current_option == 0 ? RED : BLACK
            );
            DrawText(
                settings_text,
                window.screen_width / 2 - settings_width / 2,
                5 * window.screen_height / 8,
                title_font_size,
                current_option == 1 ? RED : BLACK
            );
            DrawText(
                exit_text,
                window.screen_width / 2 - exit_width / 2,
                7 * window.screen_height / 8,
                title_font_size,
                current_option == 2 ? RED : BLACK
            );
        }
        void make_game()
        {
            if (game != nullptr) {
                game = nullptr;
                delete game;
            }
            game = make_game0(current_grid_type, *this, Game::default_cell_colors);
        }
        virtual void open_settings() override {
            settings_open = true;
        }
        virtual bool settings_are_open() override {
            return settings_open;
        }
    };
}
#endif // GAME_APPLICATION_HPP_HEADER_INCLUDE_GUARD
