#include <game/common.hpp>


#ifndef GAME_WORLD_HPP_HEADER_INCLUDE_GUARD
#define GAME_WORLD_HPP_HEADER_INCLUDE_GUARD
namespace Game
{

	enum class Direction {
		Left, Right, Up, Down, Forward, Backward
	};

	constexpr const inline uint8_t is_langton_trail = 0b11000000;
	constexpr const inline uint8_t is_langton_ant = 0b000010000;
	enum LangtonDirection
	{
		LEFT = 0b00000000,
		RIGHT = 0b00100000,
		FORWARD = 0b0001000,
		BACKWARD = 0b0011000,
	};

	using ColorType = ::Color;
	struct Index3 {
		size_t x, y, z;
		operator Vector3() {
			return Vector3{ static_cast<float>(x), static_cast<float>(y), static_cast<float>(z) };
		}
	};
	std::ostream& operator<<(std::ostream& out, const Index3& index) {
		out << "Index3:{.x=" << index.x << ",.y=" << index.y << ",.z=" << index.z << "\n";
		return out;
	}
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
		size_t Ny, 
		size_t Nz, 
		bool WrapAround = false, 
		float CubeSideLength = 1.f
	>
	struct World
	{
		struct Mutable
		{
			const size_t x;
			const size_t y;
			Cell_T& cell;
			inline Mutable& operator=(Cell_T value) {
				cell = value;
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
		}
		World(const World& other) = delete;
		World(World&& other) = default;
		~World() { delete grid_read; delete grid_write;  }
		World& operator=(const World& other) = delete;
		World& operator=(World&& other) = default;
		constexpr inline const Index3 dimensions() const {
			return Index3{ Nx, Ny, Nz };
		}
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
			return mutable_at(index3.x, index3.y, index3.z);
		}

		inline Mutable mutable_at(size_t x, size_t y, size_t z)
		{
			return Mutable{
				x, 
				y, 
				grid_write->at(from_index3(x, y, z))
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

		auto jump_3d(auto visitor, size_t steps, Index3 next)
		{
			for (size_t ii = 0; ii < steps; ++ii) {
				next = visitor(read_at(next), mutable_at(next), next);
			}
		}

		auto loop3d(auto visitor)
		{
			for (size_t iz = 0; iz < Nz; ++iz)
			{
				for (size_t ix = 0; ix < Nx; ++ix)
				{
					for (size_t iy = 0; iy < Ny; ++iy)
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
			DrawCube(
				::Vector3{
					static_cast<float>(langton_position.x) - Nx / 2 + center.x,
					static_cast<float>(langton_position.z) - Nz / 2 + center.z,
					static_cast<float>(langton_position.y) - Ny / 2 + center.y
				},
				CubeSideLength,
				CubeSideLength,
				CubeSideLength,
				PURPLE
			);
		}

		template<size_t ValueCount>
		void copy_mutable_buffer(std::array<Cell_T, ValueCount> values)
		{
			commit();
			for (auto value : values)
			{
				for (size_t iz = 0; iz < Nz; ++iz)
				{
					for (size_t ix = 0; ix < Nx; ++ix)
					{
						for (size_t iy = 0; iy < Ny; ++iy)
						{
							if (read_at(ix, iy, iz) == value)
							{
								mutable_at(ix, iy, iz) = value;
							}
						}
					}
				}
			}
			commit();
		}

		void conway()
		{
			loop3d([this](auto, auto& cell_in, auto cell_out, size_t x, size_t y, size_t z)
				{
					auto results = std::array<Cell_T, 10>{ 0, 0, static_cast<Cell_T>(cell_in), 1, 0, 0, 0, 0, 0, 0 };
					const size_t sum = neighbor_sum(x, y, z) - cell_in;
					if (sum >= results.size()) cell_out = 0;
					else cell_out = results[sum];
				}
			);
		}

		void langton(size_t steps)
		{
			jump_3d([this](auto& cell_in, Mutable cell_out, Index3 current)
				{
					Direction direction = langton_direction;
					Index3 next = current;
					if (cell_in == 3)
					{
						cell_out = 0;
						if (direction == Direction::Forward) {
							next.x = add_x(current.x);
							direction = Direction::Right;
						}
						else if (direction == Direction::Backward) {
							next.x = minus_x(current.x);
							direction = Direction::Left;
						}
						else if (direction == Direction::Left) {
							next.y = minus_y(current.y);
							direction = Direction::Backward;
						}
						else if (direction == Direction::Right) {
							next.y = add_y(current.y);
							direction = Direction::Forward;
						}
						else if (direction == Direction::Up) next.z = add_z(current.z);
						else next.z = minus_z(current.z);
					}
					else if (cell_in == 1)
						cell_out = 0;
					else if (cell_in == 2) {
						cell_out = 0;
					}
					else if (cell_in == 0)
					{
						cell_out = 3;
						if (direction == Direction::Forward) {
							next.x = minus_x(current.x);
							direction = Direction::Left;
						}
						else if (direction == Direction::Backward) {
							next.x = add_x(current.x);
							direction = Direction::Right;
						}
						else if (direction == Direction::Left) {
							next.y = add_y(current.y);
							direction = Direction::Forward;
						}
						else if (direction == Direction::Right) {
							next.y = minus_y(current.y);
							direction = Direction::Backward;
						}
						else if (direction == Direction::Up) {
							next.z = minus_z(current.z);
							direction = Direction::Down;
						}
						else {
							next.z = add_z(current.z);
							direction = Direction::Up;
						}
					}
					else cell_out = 3;
					langton_direction = direction;
					langton_position = next;
					//std::cout << "Cell in " << cell_in << " Cell out " << cell_out.cell << "\n";
					return next;
				}, steps, langton_position
			);
		}

		Index3 langton_position;
		Direction langton_direction;
	protected:
		Cube* grid_read;
		Cube* grid_write;
	};
	

	using DefaultCellType = uint8_t;
	using GameWorld = World<DefaultCellType, 48, 48, 1>;
}
#endif GAME_WORLD_HPP_HEADER_INCLUDE_GUARD
