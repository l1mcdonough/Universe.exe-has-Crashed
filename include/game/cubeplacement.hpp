#include <game/common.hpp>
#include <game/grid.hpp>
#include <game/ray_extend.hpp>

#ifndef GAME_CUBEPLACEMENT_HPP_HEADER_INCLUDE_GUARD
#define GAME_CUBEPLACEMENT_HPP_HEADER_INCLUDE_GUARD
namespace Game
{
    using namespace Game::RayExtend;
    struct CubePlacement
    {
        unsigned int x = 24;
        unsigned int y = 24;
        unsigned int z = 0;
        int cubeType = 1;
        float CubeSideLength = 1.f;
        CubePlacement(Game::Index3 grid_dimensions) :
            x(grid_dimensions.x / 2),
            y(grid_dimensions.y / 2),
            z(grid_dimensions.z / 2) {}
        void processCubePlacement(auto* world, int key) {
            handleKey(world, key);
            drawGhostCube(world);
        }
        void handleKey(auto* world, int key) {
            switch (key) {
            case KEY_LEFT_BRACKET:
                cubeType = (cubeType - 1) % Game::default_cell_colors.size();
                break;
            case KEY_RIGHT_BRACKET:
                cubeType = (cubeType + 1) % Game::default_cell_colors.size();
                break;
            case KEY_W:
                y++;
                break;
            case KEY_S:
                y--;
                break;
            case KEY_A:
                x--;
                break;
            case KEY_D:
                x++;
                break;
            case KEY_Q:
                z--;
                break;
            case KEY_ZERO:
                world->reset();
                break;
            case KEY_E:
                z++;
                break;
            case KEY_V:
                cubeType++;
                if (cubeType >= Game::default_cell_colors.size())
                    cubeType = 0;
                break;
            case KEY_F:
                cubeType--;
                if (cubeType < 0)
                    cubeType = Game::default_cell_colors.size() - 1;
                std::cout << cubeType;
                break;
            case KEY_R:
                world->commit();
                if (cubeType == 1)
                    randomConway(world);
                else if (cubeType == 3)
                    randomAntPaths(world);
                else if (cubeType == 4)
                    randomAnt(world);
                world->commit();
                break;
            case KEY_SPACE:
                world->commit();
                world->mutable_at(x, y, z) = Game::mod_cell(world->read_at(x, y, z), cubeType);
                world->commit();
                break;
            default:
                break;
            }
        }
        void drawGhostCube(auto* world) {
            Color color = Game::default_cell_colors[cubeType];
            //Color color = RED;
            const Vector3 position{
                static_cast<float>(x) - world->dimensions().x / 2,
                static_cast<float>(z) - world->dimensions().z / 2,
                static_cast<float>(y) - world->dimensions().y / 2
            };
            const Vector3 cube_size = Vector3One() * CubeSideLength;
            BoundingBox outline_box{ .min = position - cube_size / 2, .max = position + cube_size / 2};
            DrawBoundingBox(outline_box, LIME);

            DrawCube(
                position,
                CubeSideLength,
                CubeSideLength,
                CubeSideLength,
                color
            );
        }

        void drawCellTypeName(size_t screen_width, size_t screen_height)
        {
            const char* text = Game::cell_type_name(cubeType).data();
            constexpr static const size_t font_size = 32;
            size_t text_width = MeasureText(text, font_size);
            DrawText(
                text,
                screen_width / 2 - text_width / 2,
                screen_height - (font_size + 16),
                font_size,
                SKYBLUE
            );
        }

        void randomConway(auto* grid)
        {
            for (size_t ii = 0; ii < 200; ++ii)
            {
                const size_t x = GetRandomValue(0, grid->dimensions().x - 1);
                const size_t y = GetRandomValue(0, grid->dimensions().y - 1);
                const size_t z = GetRandomValue(0, grid->dimensions().z - 1);
                grid->mutable_at(x, y, z) = 1;
            }
        }
        void randomAntPaths(auto* grid)
        {
            for (size_t ii = 0; ii < 10; ++ii)
            {
                const size_t x = GetRandomValue(0, grid->dimensions().x - 1);
                const size_t y = GetRandomValue(0, grid->dimensions().y - 1);
                const size_t z = GetRandomValue(0, grid->dimensions().z - 1);
                grid->mutable_at(x, y, z) = grid->mutable_at(x, y, z) | Game::is_langton_trail;
            }
        }
        void randomAnt(auto* grid)
        {
            const size_t x = GetRandomValue(0, grid->dimensions().x - 1);
            const size_t y = GetRandomValue(0, grid->dimensions().y - 1);
            const size_t z = GetRandomValue(0, grid->dimensions().z - 1);
            const uint8_t direction = GetRandomValue(0, 3) << 3;
            grid->mutable_at(x, y, z) = (direction | Game::is_langton_ant);
        }

    };
}
#endif // GAME_CUBEPLACEMENT_HPP_HEADER_INCLUDE_GUARD