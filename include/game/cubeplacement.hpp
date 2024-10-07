#include <game/common.hpp>
#include <game/world.hpp>

struct CubePlacement {
	unsigned int x = 24;
	unsigned int y = 24;
	unsigned int z = 0;
    int cubeType = 1;
    float CubeSideLength = 1.f;

    void processCubePlacement(Game::GameWorld* world, int key) {
        handleKey(world, key);
        drawGhostCube(world);
    }
	void handleKey(Game::GameWorld* world, int key) {
        switch (key) {
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
        case KEY_E:
            z++;
            break;
        case KEY_R:
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
        case KEY_SPACE:
            world->commit();
            world->mutable_at(x, y, z) = cubeType;
            world->commit();
            break;
        default:
            break;
        }
	}
    void drawGhostCube(Game::GameWorld* world) {
        Color color = Game::default_cell_colors[cubeType];
        //Color color = RED;
        color.a = 128;
        BeginBlendMode(BLEND_ALPHA);
        DrawCube(
            ::Vector3{
                static_cast<float>(x) - world->dimensions().x/2,
                static_cast<float>(z) - world->dimensions().z / 2,
                static_cast<float>(y) - world->dimensions().y / 2
            },
            CubeSideLength,
            CubeSideLength,
            CubeSideLength,
            color
        );
        EndBlendMode();
    }
};