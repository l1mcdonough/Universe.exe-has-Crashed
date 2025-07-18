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
    // <compute> Demonstrate that boost compute is working
    compute::device gpu = compute::system::default_device();
    compute::context cl_context(gpu);
    compute::command_queue queue(cl_context, gpu);
    const size_t sample_count = 10000000;
    std::vector<float> host_vector(10000000);
    for(size_t ii = 0; ii < sample_count; ++ii)
        host_vector[ii] = GetRandomValue(0, 100000);
    compute::vector<float> device_vector(10000000, cl_context);
    compute::copy(host_vector.begin(), host_vector.end(), device_vector.begin(), queue);
    //compute::sort(device_vector.begin(), device_vector.end(), queue);
    compute::copy(device_vector.begin(), device_vector.end(), host_vector.begin(), queue);
    // </compute>

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
