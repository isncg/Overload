/**
* @project: Overload
* @author: Overload Tech.
* @licence: MIT
*/

#pragma once
#include "OvMaths/FVector2.h"
#include "OvMaths/FVector3.h"

namespace OvRendering::Geometry
{
	/**
	* Data structure that defines the geometry of a vertex
	*/
	struct Vertex
	{
		float position[3];
		float texCoords[2];
		float normals[3];
		float tangent[3];
		float bitangent[3];
	};

	struct EmptyVertex {};

	struct AssimpVertex;

	class VertexHelper
	{
	private:
		static OvMaths::FVector3& GetPositionImpl(Vertex& v);
		static OvMaths::FVector3& GetNormalImpl(Vertex& v);
		static OvMaths::FVector3& GetTangentImpl(Vertex& v);
		static OvMaths::FVector2& GetUVImpl(Vertex& v);
		static OvMaths::FVector3& GetBitangentImpl(Vertex& v);

		static OvMaths::FVector3& GetPositionImpl(EmptyVertex& v);
		static OvMaths::FVector3& GetNormalImpl(EmptyVertex& v);
		static OvMaths::FVector3& GetTangentImpl(EmptyVertex& v);
		static OvMaths::FVector2& GetUVImpl(EmptyVertex& v);
		static OvMaths::FVector3& GetBitangentImpl(EmptyVertex& v);

		static OvMaths::FVector3& GetPositionImpl(AssimpVertex& v);
		static OvMaths::FVector3& GetNormalImpl(AssimpVertex& v);
		static OvMaths::FVector3& GetTangentImpl(AssimpVertex& v);
		static OvMaths::FVector2& GetUVImpl(AssimpVertex& v);
		static OvMaths::FVector2& GetUVImpl(AssimpVertex& v, int index);
		static OvMaths::FVector3& GetBitangentImpl(AssimpVertex& v);
	public:
		template <class T> static OvMaths::FVector3& GetPosition(T& v) { return GetPositionImpl(v); };
		template <class T> static OvMaths::FVector3& GetNormal(T& v) { return GetNormalImpl(v); };
		template <class T> static OvMaths::FVector3& GetTangent(T& v) { return GetTangentImpl(v); };
		template <class T> static OvMaths::FVector2& GetUV(T& v) { return GetUVImpl(v); };
		template <class T> static OvMaths::FVector2& GetUV(T& v, int index) { return GetUVImpl(v, index); };
		template <class T> static OvMaths::FVector3& GetBitangent(T& v) { return GetBitangentImpl(v); };
	};
}