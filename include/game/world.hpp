#include <game/common.hpp>


#ifndef GAME_WORLD_HPP_HEADER_INCLUDE_GUARD
#define GAME_WORLD_HPP_HEADER_INCLUDE_GUARD
namespace Game
{
	using ColorType = decltype(RAYWHITE);
	struct Index3 {
		size_t x, y, z;
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
		using Cube = std::array<Cell_T, Nx * Ny * Nz>;
		World() : grid_read(new Cube), grid_write(new Cube)
		{
			loop3d([](auto, auto, auto& cell_out, size_t, size_t, size_t) {
					cell_out = 0;
				});
			commit();
			loop3d([](auto, auto, auto& cell_out, size_t, size_t, size_t) {
				cell_out = 0;
			});
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

		inline Cell_T& mutable_at(const Index3 index3) const {
			return grid_write->at(from_index3(index3));
		}

		inline Cell_T& mutable_at(const size_t x, const size_t y, const size_t z) {
			return grid_write->at(from_index3(x, y, z));
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

		void draw(::Vector3 camera_position, auto& colors) const
		{
			loop3d_read([&colors](const auto, const auto& cell, size_t x, size_t y, size_t z)
			{
				if (cell > 0)
				{
					DrawCube(
						::Vector3{ 
							static_cast<float>(x), 
							static_cast<float>(z), 
							static_cast<float>(y)
						},
						CubeSideLength, 
						CubeSideLength, 
						CubeSideLength,
						colors.at(cell)
					);
				}
			});
		}
	protected:
		Cube* grid_read;
		Cube* grid_write;
	};

	using DefaultCellType = uint8_t;
	using GameWorld = World<DefaultCellType, 256, 256, 16>;

	inline const auto default_cell_colors = std::map<int, ColorType>{
		{1, BLUE},
		{2, RED},
		{3, GREEN},
		{4, PURPLE},
		{5, ORANGE}
	};


}
#endif GAME_WORLD_HPP_HEADER_INCLUDE_GUARD
