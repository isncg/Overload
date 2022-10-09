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
	public:
		static OvMaths::FVector3& GetPosition(Vertex& v);
		static OvMaths::FVector3& GetNormal(Vertex& v);
		static OvMaths::FVector3& GetTangent(Vertex& v);
		static OvMaths::FVector2& GetUV(Vertex& v);
		static OvMaths::FVector3& GetBitangent(Vertex& v);
		static const int* GetBoneIds(Vertex& v);
		static const float* GetBoneWeights(Vertex& v);

		static OvMaths::FVector3& GetPosition(EmptyVertex& v);
		static OvMaths::FVector3& GetNormal(EmptyVertex& v);
		static OvMaths::FVector3& GetTangent(EmptyVertex& v);
		static OvMaths::FVector2& GetUV(EmptyVertex& v);
		static OvMaths::FVector3& GetBitangent(EmptyVertex& v);
		static const int* GetBoneIds(EmptyVertex& v);
		static const float* GetBoneWeights(EmptyVertex& v);

		static OvMaths::FVector3& GetPosition(AssimpVertex& v);
		static OvMaths::FVector3& GetNormal(AssimpVertex& v);
		static OvMaths::FVector3& GetTangent(AssimpVertex& v);
		static OvMaths::FVector2& GetUV(AssimpVertex& v);
		static OvMaths::FVector2& GetUV(AssimpVertex& v, int index);
		static OvMaths::FVector3& GetBitangent(AssimpVertex& v);
		static const int* GetBoneIds(AssimpVertex& v);
		static const float* GetBoneWeights(AssimpVertex& v);
	};
}