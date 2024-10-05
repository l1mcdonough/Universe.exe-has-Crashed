#include <game/common.hpp>


#ifndef GAME_WORLD_HPP_HEADER_INCLUDE_GUARD
#define GAME_WORLD_HPP_HEADER_INCLUDE_GUARD
namespace Game
{
	struct Index3 {
		size_t x, y, z;
	};

	using ColorsType = std::vector<::Color>;

	inline const auto default_cell_colors = std::vector{
		RAYWHITE,
		BLUE,
		RED,
		GREEN,
		PURPLE,
		ORANGE
	};

	template<
		typename Cell_T, 
		size_t Nx,
		size_t Ny
	>
	struct CellImageBuffer2D
	{
		ColorsType cell_colors;
		static const auto cell_null = Cell_T{ 0 };
		const ::Color cell_null_color;
		
		CellImageBuffer2D(ColorsType cell_colors_ = default_cell_colors)
			: cell_colors(cell_colors_), cell_null_color(cell_colors[cell_null]) {
			buffer = GenImageColor(Nx, Ny, cell_null_color);
		}
		
		~CellImageBuffer2D() {
			UnloadImage(buffer);
		}

		inline void write(const size_t x, const size_t y, const Cell_T cell_value) {
			SetPixelColor(buffer.data, cell_colors[cell_value], buffer.format);
		}
		inline ::Color read(const size_t x, const size_t y) {
			return GetImageColor(buffer, x, y);
		}

	protected: 
		Image buffer;
	};

	template<
		typename Cell_T, 
		size_t Nx, 
		size_t Ny, 
		size_t Nz, 
		bool WrapAround = false, 
		float CubeSideLength = 1.f
	>
	struct World
	{
		using CellImageBuffer2DType = CellImageBuffer2D<Cell_T, Nz, Ny>;
		using CellImageBuffers2DType = std::array<CellImageBuffer2DType, Nz>;
		struct Mutable
		{
			const size_t x;
			const size_t y;
			Cell_T& cell;
			CellImageBuffer2DType& buffer_2d;
			inline Mutable& operator=(Cell_T value)
			{
				cell = value;
				buffer_2d.write(x, y, value);
				return *this;
			}
			inline operator Cell_T&() {
				return cell;
			}
		};
		using Cube = std::array<Cell_T, Nx * Ny * Nz>;
		static const auto cell_null = Cell_T{ 0 };
		ColorsType colors;
		World(ColorsType colors_) : colors(colors_), grid_read(new Cube), grid_write(new Cube)
		{
			loop3d([](auto, auto, auto cell_out, size_t, size_t, size_t) {
					cell_out = 0;
				});
			commit();
			loop3d([](auto, auto, auto cell_out, size_t, size_t, size_t) {
				cell_out = 0;
			});
			for (auto& buffer : cell_image_buffers_2d) buffer.cell_colors = colors;
		}
		World(const World& other) = delete;
		World(World&& other) = default;
		~World() { delete grid_read; delete grid_write; }
		World& operator=(const World& other) = delete;
		World& operator=(World&& other) = default;

		#define GAME_WORLD_HPP_HEADER_MINUS_DIM(DIM) \
			auto minus_##DIM (size_t DIM ) const \
			{ \
				if constexpr (WrapAround == true) \
					return DIM == 0 ? N##DIM - 1 : DIM - 1; \
				else \
					return DIM == 0 ? 0 : DIM - 1; \
			}

		#define GAME_WORLD_HPP_HEADER_ADD_DIM(DIM) \
			auto add_##DIM (size_t DIM ) const \
			{ \
				if constexpr (WrapAround == true) \
					return (DIM + 1) % N##DIM; \
				else \
					return DIM >= (N##DIM - 1) ? (N##DIM - 1) : DIM + 1; \
			}

		GAME_WORLD_HPP_HEADER_MINUS_DIM(x)
		GAME_WORLD_HPP_HEADER_MINUS_DIM(y)
		GAME_WORLD_HPP_HEADER_MINUS_DIM(z)
		GAME_WORLD_HPP_HEADER_ADD_DIM(x)
		GAME_WORLD_HPP_HEADER_ADD_DIM(y)
		GAME_WORLD_HPP_HEADER_ADD_DIM(z)

		inline size_t from_index3(const Index3 index3) const {
			return from_index3(index3.x, index3.y, index3.z);
		}

		inline size_t from_index3(const size_t x, const size_t y, const size_t z) const {
			return (z * Nx * Ny) + (y * Nx) + x;
		}

		inline const Cell_T& read_at(const Index3 index3) const {
			return grid_read->at(from_index3(index3));
		}

		inline const Cell_T& read_at(const size_t x, const size_t y, const size_t z) const {
			return grid_read->at(from_index3(x, y, z));
		}

		inline Mutable mutable_at(const Index3 index3) {
			return grid_write->at(from_index3(index3));
		}

		inline Mutable mutable_at(size_t x, size_t y, size_t z)
		{
			return Mutable{ 
				x, 
				y, 
				grid_write->at(from_index3(x, y, z)), 
				cell_image_buffers_2d[z]
			};
		}

		Cell_T neighbor_sum(size_t x, size_t y, size_t z) const
		{
			Cell_T total = Cell_T{ 0 };
			for (size_t ix = minus_x(x); ix <= add_x(x); ++ix)
			{
				for (size_t iy = minus_y(y); iy <= add_y(y); ++iy)
				{
					for (size_t iz = minus_z(z); iz <= add_z(z); ++iz)
						total += read_at(ix, iy, iz);
				}
			}
			return total;
		}

		Cell_T neighbor_sum(Index3 index3) const {
			return neighbor_sum(index3.x, index3.y, index3.z);
		}

		auto loop3d_read(auto visitor) const
		{
			for (size_t ix = 0; ix < Nx; ++ix)
			{
				for (size_t iy = 0; iy < Ny; ++iy) {
					for (size_t iz = 0; iz < Nz; ++iz)
						visitor(grid_read, read_at(ix, iy, iz), ix, iy, iz);
				}
			}

		}
		auto loop3d(auto visitor)
		{
			for (size_t ix = 0; ix < Nx; ++ix)
			{
				for (size_t iy = 0; iy < Ny; ++iy)
				{
					for (size_t iz = 0; iz < Nz; ++iz)
					{
						/*
						@grid_read, so you can examine things around them
						@read_at, for the value of the cell
						@mutable_at, to be written to
						@ix, iy, iz, incase the inidicies are nessisary (can be used with from_index if grid is captured)
						*/
						visitor(grid_read, read_at(ix, iy, iz), mutable_at(ix, iy, iz), ix, iy, iz);
					}
				}
			}
		}

		void commit()
		{
			Cube* swap = grid_read;
			grid_read = grid_write;
			grid_write = swap;
		}

		void draw_2d_in_3d()
		{
		}

		void draw_3d(::Vector3 center) const
		{
			loop3d_read([this, center](const auto, const auto& cell, size_t x, size_t y, size_t z)
			{
				if (cell > 0)
				{
					DrawCube(
						::Vector3{ 
							static_cast<float>(x) - Nx / 2 + center.x, 
							static_cast<float>(z) - Nz / 2 + center.z,
							static_cast<float>(y) - Ny / 2 + center.y
						},
						CubeSideLength, 
						CubeSideLength, 
						CubeSideLength,
						colors.at(cell)
					);
				}
			});
		}

		void conway()
		{
			loop3d([this](auto, auto& cell_in, Mutable cell_out, size_t x, size_t y, size_t z) {
				const size_t sum = neighbor_sum(x, y, z) - cell_in;
				static uint8_t results[9] = { 0, 0, cell_in, 1, 0, 0, 0, 0, 0 };
				cell_out = results[sum % 9]; // TODO: There is proably a bug here, without modulo it crashes when accessing colors...
			});
		}
	protected:
		Cube* grid_read;
		Cube* grid_write;
		CellImageBuffers2DType cell_image_buffers_2d;
	};

	using DefaultCellType = uint8_t;
	using GameWorld = World<DefaultCellType, 256, 256, 10>;


}
#endif GAME_WORLD_HPP_HEADER_INCLUDE_GUARD
