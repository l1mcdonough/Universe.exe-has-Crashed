#include <engine/common.hpp>
#ifndef  UNIVERSE_EXE__ENGINE__CL_CONTEXT_HPP_HEADER_INCLUDE_GUARD
#define  UNIVERSE_EXE__ENGINE__CL_CONTEXT_HPP_HEADER_INCLUDE_GUARD
namespace Engine
{
    namespace compute = boost::compute;

    struct OpenCLContext
    {
        compute::device device;
        compute::context context;
        compute::command_queue queue;
        OpenCLContext(
            compute::device device_ = compute::system::default_device()
        ) : device(device_), context(device), queue(context, device) {
            std::cout << "CL Device Name: " << device.name() << "\n";
        }
    };

    template<size_t... Is>
    inline void set_kernel_args_impl(
        compute::kernel& kernel,
        std::index_sequence<Is...>,
        auto&... args
    )
    {
        (kernel.set_arg(Is, args), ...);
    }

    inline void set_kernel_args(
        compute::kernel& kernel, 
        auto&... args
    )
    {
        set_kernel_args_impl(
            kernel, 
            std::make_index_sequence<sizeof...(args)>(),
            args...
        );
    }
}

#endif // UNIVERSE_EXE__ENGINE__CL_CONTEXT_HPP_HEADER_INCLUDE_GUARD

