#include <game/game_0.hpp>

using namespace Game::RayExtend;

int main(int argc, char** args)
{
    auto game = Game::Game0<Game::GameGrid>(Game::default_cell_colors);
    game.play();
    //SetExitKey(KEY_SEMICOLON);
    //Game::GameGrid fractal_grid(Game::default_cell_colors);
    //{
    //    size_t x = grid.dimensions().x / 2;
    //    size_t y = grid.dimensions().y / 2;
    //    size_t z = grid.dimensions().z / 2;
    //    fractal_grid.mutable_at(x, y, z) = 8;
    //}
    return 0;
}
