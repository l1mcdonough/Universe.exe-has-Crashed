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
		float CubeSideLength = 1.f, 
		size_t ThreadCount = 16
	>
	struct World
	{
		using Cube = std::array<Cell_T, Nx * Ny * Nz>;
		using Transforms = std::array<::Matrix, Nx * Ny * Nz>;
	protected:
		Cube* grid_read;
		Cube* grid_write;
		Transforms* transforms_memory;
	public:
		World() :
				grid_read(new Cube), 
				grid_write(new Cube),
				transforms_memory(new Transforms),
				transforms(*transforms_memory), 
				cube(GenMeshCube(CubeSideLength, CubeSideLength, CubeSideLength)), 
				shader(LoadShader(
						TextFormat("resources/shaders/glsl%i/lighting_instancing.vs", GLSL_VERSION),
						TextFormat("resources/shaders/glsl%i/lighting.fs", GLSL_VERSION)
				)), 
				ambient_location(GetShaderLocation(shader, "ambient")), 
				grid_default_material(LoadMaterialDefault())
		{
			CreateLight(LIGHT_DIRECTIONAL, ::Vector3 { 50.0f, 50.0f, 0.0f }, ::Vector3{ 0.f, 0.f, 0.0f }, WHITE, shader);
			shader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(shader, "mvp");
			shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
			shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(shader, "instanceTransform");
			set_ambient_light(.2f, .2f, .2f);
			TraceLog(LOG_INFO, "Setting Grid Transforms...");
			loop3d_transforms(
				[this](Matrix& transform, size_t x, size_t y, size_t z)
				{
					const Matrix translation = MatrixTranslate(
						static_cast<float>(x),
						static_cast<float>(z), 
						static_cast<float>(y)
					);
					const Vector3 axis = Vector3Normalize(::Vector3{ 0.f, 0.f, 0.f });
					const float angle = 0.f * DEG2RAD;
					const Matrix rotation = MatrixRotate(axis, angle);
					transform = MatrixMultiply(rotation, translation);
					grid_read->at(from_index3(x, y, z)) = 0.f;
					grid_write->at(from_index3(x, y, z)) = 0.f;
				}
			);
			TraceLog(LOG_INFO, "... Done Setting Grid Transforms");
			grid_default_material.shader = shader;
			grid_default_material.maps[MATERIAL_MAP_DIFFUSE].color = BLUE;
		}
		World(const World& other) = delete;
		World(World&& other) = default;
		~World() { delete grid_read; delete grid_write; delete transforms_memory; UnloadMesh(cube); }
		World& operator=(const World& other) = delete;
		World& operator=(World&& other) = default;

		void set_ambient_light(float r, float g, float b) {
			float color[4] = { r, g, b, 1.0f };
			SetShaderValue(shader, ambient_location, color, SHADER_UNIFORM_VEC4);
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

		auto loop3d_transforms(auto visitor)
		{
			for (size_t ix = 0; ix < Nx; ++ix)
			{
				for (size_t iy = 0; iy < Ny; ++iy)
				{
					for (size_t iz = 0; iz < Nz; ++iz)
						visitor(transforms[from_index3(ix, iy, iz)], ix, iy, iz);
				}
			}
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
			loop3d_read([&colors](const auto, const auto& cell, size_t x, size_t y, size_t z) {
				if (cell != 0) {
					DrawCube(::Vector3{ (float)x , (float)z, (float)y}, 1.f, 1.f, 1.f, colors.at(cell));
				}
				});
			//auto camera_position_array = std::array{ camera_position.x, camera_position.y, camera_position.z };
			//rlEnableShader(shader.id);
			//int cell_value_location = rlGetLocationAttrib(shader.id, "vertexCellValue");
			//rlEnableVertexAttribute(cell_value_location);
			//rlSetVertexAttribute(cell_value_location, 1, RL_FLOAT, false, 0, 0);
			//std::cout << "cell " << grid_read->data()[from_index3(16/ 2 + 1, 16/ 2 + 1, 0)] << "\n";
			//std::cout << "cell " << grid_read->data()[from_index3(16/ 2, 16/ 2 + 1, 0)] << "\n";
			//int vboCellId = rlLoadVertexBuffer(grid_read->data(), Nx * Ny * Nz, true);
			//rlEnableVertexBuffer(vboCellId);
			//SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], camera_position_array.data(), SHADER_UNIFORM_VEC3);
			//DrawMeshInstanced(cube, grid_default_material, transforms.data(), transforms.size());
			//rlUnloadVertexBuffer(vboCellId);
		}
	protected:
		Transforms& transforms; // WAY too much memory to allocate on the stack //
		const ::Mesh cube;
		::Shader shader;
		int ambient_location;
		::Material grid_default_material; // TODO: Modify shader to accept cell values and color uniform //

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
