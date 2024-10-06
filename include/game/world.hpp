#include <game/common.hpp>


#ifndef GAME_WORLD_HPP_HEADER_INCLUDE_GUARD
#define GAME_WORLD_HPP_HEADER_INCLUDE_GUARD
namespace Game
{
	using ColorType = decltype(RAYWHITE);
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
		constexpr static const bool has_render_mode = true;
		ColorsType cell_colors;
		Mesh plane_mesh;
		Model plane;
		static const auto cell_null = Cell_T{ 0 };
		const ::Color cell_null_color;
		
		CellImageBuffer2D(ColorsType cell_colors_ = default_cell_colors)
			: cell_colors(cell_colors_), cell_null_color(cell_colors[cell_null])
		{
			buffer = LoadRenderTexture(Nx, Ny);
			plane_mesh = GenMeshCube(Nx, 1.f, Ny);
			plane = LoadModelFromMesh(plane_mesh);
			plane.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = buffer.texture;
		}
		
		~CellImageBuffer2D() {
			UnloadRenderTexture(buffer);
			UnloadModel(plane);
		}

		struct RenderMode
		{
			constexpr static const bool has_render_mode = false;
			RenderTexture& buffer;
			ColorsType& cell_colors;
			inline RenderMode(RenderTexture& buffer_, ColorsType& cell_colors_) : buffer(buffer_), cell_colors(cell_colors_) {
				BeginTextureMode(buffer);
			}
			inline ~RenderMode() { EndTextureMode(); }
			inline RenderMode& write(const size_t x, const size_t y, const Cell_T cell_value) {
				DrawPixel(x, y, cell_colors[cell_value]);
				return *this;
			}
		};

		RenderMode render_mode() {
			return RenderMode( buffer, cell_colors );
		}

		inline void write_one(const size_t x, const size_t y, const Cell_T cell_value) {
			render_mode().write(x, y, cell_value);
		}

		void draw(::Vector3 center, float scale_factor) const {
			plane.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = buffer.texture;
			DrawModel(plane, center, 1.f, RAYWHITE);
		}

		const RenderTexture2D& get_buffer() const {
			return buffer;
		}
	protected: 
		RenderTexture buffer;
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
		using CellImageBuffer2DType = CellImageBuffer2D<Cell_T, Nx, Ny>;
		using CellImageBuffers2DType = std::array<CellImageBuffer2DType, Nz>;
		template<typename R>
		struct Mutable
		{
			const size_t x;
			const size_t y;
			Cell_T& cell;
			R& render;
			inline Mutable& operator=(Cell_T value)
			{
				cell = value;
				if constexpr (R::has_render_mode == true)
					render.write_one(x, y, value);
				else
					render.write(x, y, value);
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
			//xz_texture = LoadTextureFromImage(cell_image_buffers_2d[0].get_buffer());
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

		inline Mutable<CellImageBuffer2DType> mutable_at(const Index3 index3) {
			return grid_write->at(from_index3(index3));
		}

		inline Mutable<CellImageBuffer2DType> mutable_at(size_t x, size_t y, size_t z)
		{
			return Mutable< CellImageBuffer2DType>{
				x, 
				y, 
				grid_write->at(from_index3(x, y, z)), 
				cell_image_buffers_2d[z]
			};
		}

		inline auto mutable_at(size_t x, size_t y, size_t z, typename CellImageBuffer2DType::RenderMode render_mode)
		{
			return Mutable{
				x,
				y,
				grid_write->at(from_index3(x, y, z)),
				render_mode
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
			for (size_t iz = 0; iz < Nz; ++iz)
			{
				auto render_mode = cell_image_buffers_2d[iz].render_mode();
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
						visitor(grid_read, read_at(ix, iy, iz), mutable_at(ix, iy, iz, render_mode), ix, iy, iz);
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

		void draw_2d_in_3d(::Vector3 center, float scale_factor = 1.f) const
		{
			/*	center.x = -scale_factor * Nx;
				center.y = -scale_factor * Nz;
				center.z = -scale_factor * Ny;*/
			loop3d_read([this, center, scale_factor](const auto, const auto& cell, size_t x, size_t y, size_t z)
				{
					//plane.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = cell_image_buffers_2d[z].get_texture();
					//DrawModel(plane, center, scale_factor, RAYWHITE);
					cell_image_buffers_2d[z].draw(center, scale_factor);
				}
			);
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
			loop3d([this](auto, auto& cell_in, auto cell_out, size_t x, size_t y, size_t z)
				{
					auto results = std::array<Cell_T, 10>{ 0, 0, static_cast<Cell_T>(cell_in), 1, 0, 0, 0, 0, 0, 0 };
					const size_t sum = neighbor_sum(x, y, z) - cell_in;
					if (sum >= results.size()) cell_out = 0;
					else cell_out = results[sum];
				}
			);
		}
	protected:
		Cube* grid_read;
		Cube* grid_write;
		CellImageBuffers2DType cell_image_buffers_2d;
		Texture xz_texture;
	};

	using DefaultCellType = uint8_t;
	using GameWorld = World<DefaultCellType, 64, 64, 1>;
}
#endif GAME_WORLD_HPP_HEADER_INCLUDE_GUARD
