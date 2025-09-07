#include <engine/cl_context.hpp>
#ifndef  UNIVERSE_EXE__ENGINE__CONWAY_HPP_HEADER_INCLUDE_GUARD
#define  UNIVERSE_EXE__ENGINE__CONWAY_HPP_HEADER_INCLUDE_GUARD
namespace Engine
{
    namespace compute = boost::compute;

    inline const char* conway3d_kernel_src = BOOST_COMPUTE_STRINGIZE_SOURCE(
        __kernel void conway3d_step(__global const char* current,
            __global char* next,
            const uint width,
            const uint height,
            const uint depth) {
        int x = get_global_id(0);
        int y = get_global_id(1);
        int z = get_global_id(2);
        int index = z * width * height + y * width + x;

        int count = 0;
        for (int dz = -1; dz <= 1; dz++) {
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dx == 0 && dy == 0 && dz == 0) continue;
                    int nx = x + dx;
                    int ny = y + dy;
                    int nz = z + dz;
                    if (nx >= 0 && ny >= 0 && nz >= 0 && nx < width && ny < height && nz < depth) {
                        int n_index = nz * width * height + ny * width + nx;
                        count += current[n_index];
                    }
                }
            }
        }

        char state = current[index];
        if (state == 1 && (count == 2 || count == 3)) {
            next[index] = 1;
        }
        else if (state == 0 && count == 3) {
            next[index] = 1;
        }
        else {
            next[index] = 0;
        }
    }
    );

    struct ConwayLayer
    {
        const size_t width;
        const size_t height;
        const size_t depth;
        const size_t grid_size = width * height * depth;
        std::vector<char> host_grid;
        std::vector<Matrix> transforms;
        compute::vector<char> d_current;
        compute::vector<char> d_next;
        ConwayLayer(
            OpenCLContext& cl, 
            const size_t width_ = 100,
            const size_t height_ = 100,
            const size_t depth_ = 100
        ) : width(width_), height(height_), depth(depth_), 
            grid_size(width * height * depth), 
            host_grid(grid_size, 0),
            d_current(host_grid.begin(), host_grid.end(), cl.queue),
            d_next(grid_size, cl.context) {}
    };
}
#endif // UNIVERSE_EXE__ENGINE__CONWAY_HPP_HEADER_INCLUDE_GUARD

