#include "OvRendering/Geometry/Vertex.h"

OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetPosition(Vertex& v)  { return reinterpret_cast<OvMaths::FVector3&>(v.position);  }
OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetNormal(Vertex& v)    { return reinterpret_cast<OvMaths::FVector3&>(v.normals);   }
OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetTangent(Vertex& v)   { return reinterpret_cast<OvMaths::FVector3&>(v.tangent);   }
OvMaths::FVector2& OvRendering::Geometry::VertexHelper::GetUV(Vertex& v)        { return reinterpret_cast<OvMaths::FVector2&>(v.texCoords); }
OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetBitangent(Vertex& v) { return reinterpret_cast<OvMaths::FVector3&>(v.bitangent); }


OvMaths::FVector3 _zero3;
OvMaths::FVector2 _zero2;
float boneWeights[4]{ 0,0,0,0 };
int boneIds[4]{ 0,0,0,0 };

OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetPosition(EmptyVertex& v)  { return _zero3; }
OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetNormal(EmptyVertex& v)    { return _zero3; }
OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetTangent(EmptyVertex& v)   { return _zero3; }
OvMaths::FVector2& OvRendering::Geometry::VertexHelper::GetUV(EmptyVertex& v)        { return _zero2; }
OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetBitangent(EmptyVertex& v) { return _zero3; }
const int* OvRendering::Geometry::VertexHelper::GetBoneIds(EmptyVertex& v) { return boneIds; }
const float* OvRendering::Geometry::VertexHelper::GetBoneWeights(EmptyVertex& v) { return boneWeights; }
const int* OvRendering::Geometry::VertexHelper::GetBoneIds(Vertex& v) { return boneIds; }
const float* OvRendering::Geometry::VertexHelper::GetBoneWeights(Vertex& v) { return boneWeights; }
