#include <game/application.hpp>
#include <vector>
#include <algorithm>
#ifndef BOOST_COMPUTE_USE_CPP11
    #define BOOST_COMPUTE_USE_CPP11
#endif
#ifndef CL_TARGET_OPENCL_VERSION
    #define CL_TARGET_OPENCL_VERSION 300
#endif
#include <boost/compute.hpp>

using namespace Game::RayExtend;
namespace compute = boost::compute;
int main(int argc, char** args)
{
    Game::Application application;

    application.run();
    //auto game = Game::Game0<Game::GameGrid>(Game::default_cell_colors);
    //game.play();
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
