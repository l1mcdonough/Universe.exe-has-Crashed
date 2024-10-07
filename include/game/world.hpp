#include <game/common.hpp>


#ifndef GAME_WORLD_HPP_HEADER_INCLUDE_GUARD
#define GAME_WORLD_HPP_HEADER_INCLUDE_GUARD
namespace Game
{

	enum class Direction {
		Left, Right, Up, Down, Forward, Backward
	};

	constexpr const inline uint8_t is_langton_trail       = 0b00000011;
	constexpr const inline uint8_t is_langton_ant         = 0b00100000;
	constexpr const inline uint8_t langton_direction_mask = 0b00011000;
	enum LangtonDirection
	{
		LANGTON_LEFT     = 0b00000000,
		LANGTON_RIGHT    = 0b00001000,
		LANGTON_FORWARD  = 0b00010000,
		LANGTON_BACKWARD = 0b00011000,
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

		inline const Cell_T& read_at(Index3 index3) const {
			return grid_read->at(from_index3(index3));
		}

		inline const Cell_T& read_at(const size_t x, const size_t y, const size_t z) const {
			return grid_read->at(from_index3(x, y, z));
		}

		inline Mutable mutable_at(Index3 index3) {
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
					Color color = RAYWHITE;
					if ((cell & is_langton_ant) == is_langton_ant)
						color = PURPLE;
					else if ((cell & is_langton_trail) == is_langton_trail)
						color = GREEN;
					else if ((cell & langton_direction_mask) != 0)
						color = BROWN;
					else
						color = colors.at(cell);
					DrawCube(
						::Vector3{ 
							static_cast<float>(x) - Nx / 2 + center.x, 
							static_cast<float>(z) - Nz / 2 + center.z,
							static_cast<float>(y) - Ny / 2 + center.y
						},
						CubeSideLength, 
						CubeSideLength, 
						CubeSideLength,
						color
					);
				}
			});
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
			loop3d([this](auto, auto& cell_in, auto cell_out, size_t x, size_t y, size_t z)
				{
					static const auto clockwise = std::array<uint8_t, 4>{
						LANGTON_FORWARD,
						LANGTON_BACKWARD,
						LANGTON_RIGHT,
						LANGTON_LEFT
					};
					static const auto counter_clockwise = std::array<uint8_t, 4>{
						LANGTON_BACKWARD,
						LANGTON_FORWARD,
						LANGTON_LEFT, 
						LANGTON_RIGHT
					};
					auto ant_at = [&](Index3 position, uint8_t direction)
					{
						mutable_at(position) = (mutable_at(position) | direction | is_langton_ant);
						commit();
						mutable_at(position) = (mutable_at(position) | direction | is_langton_ant);
						commit();
					};
					if ((cell_in & is_langton_ant) == is_langton_ant)
					{
						Cell_T direction = (cell_in & langton_direction_mask) >> 3;
						if ((cell_in & is_langton_trail) == is_langton_trail)
						{
							const auto clockwise_position = std::array<Index3, 4>{
								Index3{ x, add_y(y), z },
								Index3{ x, minus_y(y), z },
								Index3{ add_x(x), y, z },
								Index3{ minus_x(x), y, z }
							};
							cell_out = clockwise[direction];
							ant_at(clockwise_position[direction], clockwise[direction]);
						}
						else
						{
							const auto counter_clockwise_position = std::array<Index3, 4>{
								Index3{ x, minus_y(y), z },
								Index3{ x, add_y(y), z },
								Index3{ minus_x(x), y, z },
								Index3{ add_x(x), y, z }
							};
							cell_out = counter_clockwise[direction] | is_langton_trail;
							ant_at(counter_clockwise_position[direction], counter_clockwise[direction]);
						}
					}
					else
						cell_out = cell_in;
				}
			);
		}

	protected:
		Cube* grid_read;
		Cube* grid_write;
	};
	

	using DefaultCellType = uint8_t;
	using GameWorld = World<DefaultCellType, 48, 48, 1>;
}
#endif // GAME_WORLD_HPP_HEADER_INCLUDE_GUARD
