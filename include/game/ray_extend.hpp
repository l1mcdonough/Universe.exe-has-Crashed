#include <game/common.hpp>

namespace Game::RayExtend
{
	inline ::Vector3 operator+(::Vector3 left, float right) {
		return Vector3AddValue(left, right);
	}

	inline ::Vector3 operator+(::Vector3 left, ::Vector3 right) {
		return Vector3Add(left, right);
	}

	inline ::Vector3 operator*(::Vector3 left, ::Vector3 right) {
		return Vector3Multiply(left, right);
	}

	inline ::Vector3 operator*(::Vector3 left, float right) {
		return ::Vector3{ left.x * right, left.y * right, left.z * right };
	}

	inline ::Vector3 operator/(::Vector3 left, ::Vector3 right) {
		return Vector3Divide(left, right);
	}

	inline ::Vector3 operator/(::Vector3 left, float right) {
		return ::Vector3{ left.x / right, left.y / right, left.z / right };
	}
}
