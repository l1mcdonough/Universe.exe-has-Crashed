#include <game/common.hpp>


#ifndef GAME_WORLD_HPP_HEADER_INCLUDE_GUARD
#define GAME_WORLD_HPP_HEADER_INCLUDE_GUARD
namespace Game
{
	using DefaultCellType = uint8_t;
	enum class Direction {
		Left, Right, Up, Down, Forward, Backward
	};

	constexpr const inline uint8_t is_langton_trail       = 0b00000011;
	constexpr const inline uint8_t is_langton_ant         = 0b00100000;
	constexpr const inline uint8_t langton_direction_mask = 0b00011000;
	constexpr const inline uint8_t t = (2 << 3);
	
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

	inline DefaultCellType mod_cell(DefaultCellType from, DefaultCellType to)
	{
		if (to == 3 || (to & is_langton_trail) == is_langton_trail)
		{
			if ((from & is_langton_trail) == is_langton_trail)
				return from & (~is_langton_trail);
			else
				return from | is_langton_trail;
		}
		else if (to == 4 || (to & is_langton_ant) == is_langton_ant)
		{
			if ((from & is_langton_ant) == is_langton_ant)
				return from & (~is_langton_ant) & (~langton_direction_mask);
			else
				return from | is_langton_ant | static_cast<uint8_t>(GetRandomValue(0, 3) << 3);
		}
		else
			return to;
	}


	inline std::string_view cell_type_name(DefaultCellType type)
	{
		if (type == 1)
			return "Conway";
		else if (type == 2)
			return "Conway Blocker";
		else if (type == 3 || (type & is_langton_trail) == is_langton_trail)
			return "Ant Trail";
		else if (type == 4 || (type & is_langton_ant) == is_langton_ant)
			return "Ant";
		else
			return "Unknown";
	}


	template<
		typename Cell_T, 
		size_t Nx, 
		size_t Ny, 
		size_t Nz, 
		bool WrapAround = false, 
		float CubeSideLength = 1.f
	>
	struct Grid
	{
		constexpr static const size_t XSize = Nx;
		constexpr static const size_t YSize = Ny;
		constexpr static const size_t ZSize = Nz;
		constexpr static const Index3 grid_dimensions{ Nx, Ny, Nz };
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
		Grid(ColorsType colors_) : colors(colors_), grid_read(new Cube), grid_write(new Cube), grid_alpha(255)
		{
			loop3d([](auto, auto, auto cell_out, size_t, size_t, size_t) {
					cell_out = 0;
				});
			commit();
			loop3d([](auto, auto, auto cell_out, size_t, size_t, size_t) {
					cell_out = 0;
				});
		}
		Grid(const Grid& other) = delete;
		Grid(Grid&& other) = default;
		~Grid() { delete grid_read; delete grid_write;  }
		Grid& operator=(const Grid& other) = delete;
		Grid& operator=(Grid&& other) = default;
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


		#define GAME_WORLD_HPP_HEADER_OFFSET_DIM(DIM) \
			auto offset_##DIM ( int DIM ) const \
			{ \
				if constexpr (WrapAround == true) \
					return DIM <= 0 ? N##DIM + DIM : DIM % N##DIM; \
				else \
					return DIM <= 0 ? 0 : (( DIM >= N##DIM ) ? DIM - 1 : DIM ); \
			}

		GAME_WORLD_HPP_HEADER_MINUS_DIM(x)
		GAME_WORLD_HPP_HEADER_MINUS_DIM(y)
		GAME_WORLD_HPP_HEADER_MINUS_DIM(z)
		GAME_WORLD_HPP_HEADER_ADD_DIM(x)
		GAME_WORLD_HPP_HEADER_ADD_DIM(y)
		GAME_WORLD_HPP_HEADER_ADD_DIM(z)
		GAME_WORLD_HPP_HEADER_OFFSET_DIM(x)
		GAME_WORLD_HPP_HEADER_OFFSET_DIM(y)
		GAME_WORLD_HPP_HEADER_OFFSET_DIM(z)

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


		Cell_T neighbor_sum(size_t x, size_t y, size_t z, Cell_T count_value) const
		{
			Cell_T total = Cell_T{ 0 };
			for (size_t ix = minus_x(x); ix <= add_x(x); ++ix)
			{
				for (size_t iy = minus_y(y); iy <= add_y(y); ++iy)
				{
					for (size_t iz = minus_z(z); iz <= add_z(z); ++iz)
					{
						total += static_cast<Cell_T>(read_at(ix, iy, iz) == count_value)* count_value;
					}
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

		void set_grid_alpha(float grid_alpha_) {
			grid_alpha = grid_alpha_;
		}

		float get_grid_alpha() const {
			return grid_alpha;
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
					color.a = grid_alpha;
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
					if (cell_in < 2)
					{
						auto results = std::array<Cell_T, 10>{ 0, 0, static_cast<Cell_T>(cell_in), 1, 0, 0, 0, 0, 0, 0 };
						const size_t sum = neighbor_sum(x, y, z, 1) - static_cast<uint8_t>(cell_in == 1);
						if (sum >= results.size()) cell_out = 0;
						else cell_out = results[sum];
					}
				}
			);
		}

		void reset()
		{
			loop3d([this](auto, auto& cell_in, auto cell_out, size_t x, size_t y, size_t z)
				{
					cell_out = Cell_T{ 0 };
				}
			);
			commit();
			loop3d([this](auto, auto& cell_in, auto cell_out, size_t x, size_t y, size_t z)
				{
					cell_out = Cell_T{ 0 };
				}
			);
			commit();
		}

		void fractal()
		{
			loop3d([this](auto, auto& cell_in, auto cell_out, size_t x, size_t y, size_t z)
				{
					if (cell_in > 1)
					{
						int8_t x_offset = (1 - static_cast<int8_t>((x + cell_in) % 3));
						int8_t y_offset = (1 - static_cast<int8_t>((y + cell_in) % 3));
						int8_t z_offset = (1 - static_cast<int8_t>((z + cell_in) % 3));
						size_t out_x = offset_x(static_cast<int>(x) + x_offset);
						size_t out_y = offset_y(static_cast<int>(y) + y_offset);
						size_t out_z = offset_z(static_cast<int>(z) + z_offset);
						mutable_at(out_x, out_y, out_z) = read_at(out_x, out_y, out_z) + 1;
						cell_out -= 1;
					}
				}
			);
		}

		void langton()
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
						Cell_T value = ((mutable_at(position) & is_langton_trail) | direction | is_langton_ant);
						mutable_at(position) = value;
						commit();
						mutable_at(position) = value;
						commit();
					};
					if ((cell_in & is_langton_ant) == is_langton_ant)
					{
						const auto position = std::array<Index3, 4>{
								Index3{ minus_x(x), y, z },
								Index3{ add_x(x), y, z },
								Index3{ x, add_y(y), z },
								Index3{ x, minus_y(y), z }
						};
						const auto lateral_position = std::array<Index3, 4>{
								Index3{ x, y, add_z(z) },
								Index3{ x, y, minus_z(z) },
								Index3{ x, y, add_z(z) },
								Index3{ x, y, minus_z(z) }
						};
						Cell_T direction = (cell_in & langton_direction_mask) >> 3;
						if ((cell_in & is_langton_trail) == 1)
						{
							cell_out = 1;
							uint8_t next_direction = clockwise[direction];
							//uint8_t next_direction = ((x % 2 + y % 3 + z % 4) % 2) == 1 ? clockwise[direction] : counter_clockwise[direction]; // pseudo-random
							ant_at(lateral_position[next_direction >> 3], next_direction);
						}
						else if ((cell_in & is_langton_trail) == is_langton_trail)
						{
							cell_out = 0;
							uint8_t next_direction = clockwise[direction];
							ant_at(position[next_direction >> 3], next_direction);
						}
						else
						{
							cell_out = is_langton_trail;
							uint8_t next_direction = counter_clockwise[direction];
							ant_at(position[next_direction >> 3], next_direction);
						}
					}
					else if((cell_in & is_langton_trail) == is_langton_trail)
						cell_out = cell_in;
				}
			);
		}

		void draw_box_3d(Vector3 center) const
		{
			Vector3 minimum{
				center.x - Nx / 2,
				center.z - Nz / 2,
				center.y - Ny / 2
			};
			Vector3 maximum{
				center.x + Nx / 2,
				center.z + Nz / 2,
				center.y + Ny / 2
			};
			DrawBoundingBox(BoundingBox{ .min = minimum, .max = maximum }, VIOLET);
		}

	protected:
		Cube* grid_read;
		Cube* grid_write;
		float grid_alpha;
	};
	


	using GameGrid = Grid<DefaultCellType, 48, 48, 16>;
}
#endif // GAME_WORLD_HPP_HEADER_INCLUDE_GUARD
