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

    #define CL_SCALAR_CONCEPT(TYPE, SIZE, CONCEPT) \
        template<typename T> \
        concept TYPE##SIZE = CONCEPT <std::decay_t<T>> && sizeof(std::decay_t<T>) == SIZE / 8

    CL_SCALAR_CONCEPT(Uint,  8, std::unsigned_integral);
    CL_SCALAR_CONCEPT(Uint, 16, std::unsigned_integral);
    CL_SCALAR_CONCEPT(Uint, 32, std::unsigned_integral);
    CL_SCALAR_CONCEPT(Uint, 64, std::unsigned_integral);

    CL_SCALAR_CONCEPT(Int,  8, std::integral);
    CL_SCALAR_CONCEPT(Int, 16, std::integral);
    CL_SCALAR_CONCEPT(Int, 32, std::integral);
    CL_SCALAR_CONCEPT(Int, 64, std::integral);

    CL_SCALAR_CONCEPT(Float, 32, std::floating_point);
    CL_SCALAR_CONCEPT(Float, 64, std::floating_point);

    template<typename T>
    concept Scalar = Float64<T> || Float32<T>
        || Int8<T> || Int16<T> || Int32<T> || Int64<T>
        || Uint8<T> || Uint16<T> || Uint32<T> || Uint64<T>;
    
    template<typename Host> requires(!Scalar<Host>)
    struct CLScalarConvert { using Type = Host; };
    
    #define CL_SCALAR_CONVERSION(KIND, SIZE, CLTYPE) \
        template< KIND##SIZE T > requires(Scalar<T>) \
        struct CLScalarConvert<T> { using Type = CLTYPE; }
    
    CL_SCALAR_CONVERSION(Int, 64, cl_ulong);
    CL_SCALAR_CONVERSION(Int, 32, cl_int);
    CL_SCALAR_CONVERSION(Int, 16, cl_short);
    CL_SCALAR_CONVERSION(Int, 8,  cl_char);
    
    CL_SCALAR_CONVERSION(Uint, 64, cl_ulong);
    CL_SCALAR_CONVERSION(Uint, 32, cl_uint);
    CL_SCALAR_CONVERSION(Uint, 16, cl_ushort);
    CL_SCALAR_CONVERSION(Uint, 8,  cl_uchar);
    
    CL_SCALAR_CONVERSION(Float, 64, cl_float);
    CL_SCALAR_CONVERSION(Float, 32, cl_double);
    

    inline auto cl_forward(auto arg) {
        using CLType = typename CLScalarConvert<decltype(arg)>::Type;
        return static_cast<CLType>(arg);
    }
    
    template<size_t... Is>
    inline void set_kernel_args_impl(
        compute::kernel& kernel,
        std::index_sequence<Is...>,
        auto... args
    )
    {
        (kernel.set_arg(Is, cl_forward(args)), ...);
    }

    inline void set_kernel_args(
        compute::kernel& kernel, 
        auto... args
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

