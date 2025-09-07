#include <engine/common.hpp>
#ifndef  UNIVERSE_EXE__ENGINE__CL_CONTEXT_HPP_HEADER_INCLUDE_GUARD
#define  UNIVERSE_EXE__ENGINE__CL_CONTEXT_HPP_HEADER_INCLUDE_GUARD
namespace Engine
{
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
    /*
    template<typename T>
    concept Uint64 = std::unsigned_integral<T> && sizeof(T) == 64;
    template<typename T>
    concept Uint32 = std::unsigned_integral<T> && sizeof(T) == 32;
    template<typename T>
    concept Uint16 = std::unsigned_integral<T> && sizeof(T) == 16;
    template<typename T>
    concept Uint8 = std::unsigned_integral<T> && sizeof(T) == 8;
    
    template<typename T>
    concept Int64 = std::unsigned_integral<T> && sizeof(T) == 64;
    template<typename T>
    concept Int32 = std::unsigned_integral<T> && sizeof(T) == 32;
    template<typename T>
    concept Int16 = std::unsigned_integral<T> && sizeof(T) == 16;
    template<typename T>
    concept Int8 = std::unsigned_integral<T> && sizeof(T) == 8;
    
    template<typename T>
    concept Float32 = std::floating_point<T> && sizeof(T) == 32;
    template<typename T>
    concept Float64 = std::floating_point<T> && sizeof(T) == 64;
    
    template<typename Host>
    struct CLScalarConvert { using Type = void; };
    
    #define CL_SCALAR_CONVERSION(KIND, SIZE, CLTYPE) \
        template< KIND#SIZE > \
        struct CLScalarConvert< { using Type = CLTYPE }
    
    CL_SCALAR_CONVERSION(Int, 64, cl_long);
    CL_SCALAR_CONVERSION(Int, 32, cl_int);
    CL_SCALAR_CONVERSION(Int, 16, cl_short);
    CL_SCALAR_CONVERSION(Int, 8, cl_char);
    
    CL_SCALAR_CONVERSION(Uint, 64, cl_ulong);
    CL_SCALAR_CONVERSION(Uint, 32, cl_uint);
    CL_SCALAR_CONVERSION(Uint, 16, cl_ushort);
    CL_SCALAR_CONVERSION(Uint, 8, cl_uchar);
    
    CL_SCALAR_CONVERSION(Float64, 64, cl_float);
    CL_SCALAR_CONVERSION(Float32, 32, cl_double);
    

    void cl_forward(auto to, auto arg, auto args...)
    {
    }
    
    void run_kernel(compute::kernel kernel, auto... args)
    {
    }
 */   
}

#endif // UNIVERSE_EXE__ENGINE__CL_CONTEXT_HPP_HEADER_INCLUDE_GUARD

