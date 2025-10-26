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

    /*template<typename T>
    struct ReadPtr
    {
        const T& t;
        inline explicit ReadPtr(T& t_, std::atomic<size_t>& read_count_) 
            : t(t_), read_count(read_count_) { read_count += 1; }
        inline ~ReadPtr() { read_count -= 1; }
        inline T* const operator->() { return &t; }
        inline const T& operator*() { return t; }
        inline operator const T&() const { return t; }
        protected: 
            std::atomic<size_t> read_count;
    };

    template<
        auto LayerType_PC, 
        typename Cell_PT, 
        size_t DimensionCount_PC = 3, 
        size_t BufferCount_PC = 2
    >
    struct LayerBuffer
    {
        using Cell = Cell_PT;
        using HostGrid = std::vector<Cell>;
        using ComputeBuffer = compute::vector<Cell>;
        using ComputeBuffers = std::array<ComputeBuffer, BufferCount_PC>;
        using ModifyGridCall = std::function<void(HostGrid&)>;
        constexpr static size_t buffer_count = BufferCount_PC;
        const std::array<size_t, DimensionCount> dimensions;
        const size_t grid_size;

        constexpr inline size_t calculate_grid_size() const {
            const size_t size = 1;
            for (size_t current_size : dimensions) {
                size *= current_size;
            }
            return size;
        }

        inline void modify_host_grid(ModifyGridCall modify, bool lock_grid_ = true) {
            while (
                std::atomic_compare_exchange_strong(lock_grid, false, lock_grid_)
                    && reader_count > 0
            );
            modify(host_grid);
        }

        inline void update_compute_grid()
        {
            compute::copy(
                host_grid.begin(),
                host_grid.end(),
                compute_buffers[current_buffer].begin(),
                context.queue
            );
            lock_grid = false;
        }

        inline void modify_grid(ModifyGridCall modify) {
            modify_host_grid(modify);
            update_compute_grid();
        }

        ReadPtr<HostGrid> ref_host_grid() const
        {
            auto read_ptr = ReadPtr<HostGrid>(host_grid, reader_count); /* This needs to 
                    go before lock_grid is acquired, otherwise if 
                    reader_count is zero inbetween acquisiation and increment 
                    then the lock could be re-acquired by another thread.*/
     /*       while (lock_grid == true);
            return read_ptr;
        }


        protected:
            OpenCLContext& context;
            HostGrid host_grid;
            ComputeBuffers compute_buffers;
            size_t current_buffer = 0;
            std::atomic<bool> lock_grid = false;
            std::atomic<size_t> reader_count = false;
    };*/

    struct ConwayLayer
    {
        const int width;
        const int height;
        const int depth;
        const int grid_size = width * height * depth;
        const char* kernel_source;
        ConwayLayer(
            OpenCLContext& cl, 
            const int width_ = 100,
            const int height_ = 100,
            const int depth_ = 100, 
            const char* kernel_source_ = conway3d_kernel_src
        ) : width(width_), height(height_), depth(depth_), 
            grid_size(width * height * depth), 
            kernel_source(kernel_source_), 
            host_grid(grid_size, 0),
            d_current(host_grid.begin(), host_grid.end(), cl.queue),
            d_next(grid_size, cl.context),
            program(compute::program::build_with_source(
                kernel_source, cl.context
            )) {}
        //protected:
            std::vector<char> host_grid;
            compute::vector<char> d_current;
            compute::vector<char> d_next;
            std::vector<Matrix> transforms;
            compute::program program;
            std::vector<compute::kernel> kernel_steps;
    };
} //
#endif // UNIVERSE_EXE__ENGINE__CONWAY_HPP_HEADER_INCLUDE_GUARD

