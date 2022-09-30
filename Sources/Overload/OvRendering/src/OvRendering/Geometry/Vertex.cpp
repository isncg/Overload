#include "OvRendering/Geometry/Vertex.h"

OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetPositionImpl(Vertex& v)  { return reinterpret_cast<OvMaths::FVector3&>(v.position);  }
OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetNormalImpl(Vertex& v)    { return reinterpret_cast<OvMaths::FVector3&>(v.normals);   }
OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetTangentImpl(Vertex& v)   { return reinterpret_cast<OvMaths::FVector3&>(v.tangent);   }
OvMaths::FVector2& OvRendering::Geometry::VertexHelper::GetUVImpl(Vertex& v)        { return reinterpret_cast<OvMaths::FVector2&>(v.texCoords); }
OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetBitangentImpl(Vertex& v) { return reinterpret_cast<OvMaths::FVector3&>(v.bitangent); }

OvMaths::FVector3 _zero3;
OvMaths::FVector2 _zero2;

OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetPositionImpl(EmptyVertex& v)  { return _zero3; }
OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetNormalImpl(EmptyVertex& v)    { return _zero3; }
OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetTangentImpl(EmptyVertex& v)   { return _zero3; }
OvMaths::FVector2& OvRendering::Geometry::VertexHelper::GetUVImpl(EmptyVertex& v)        { return _zero2; }
OvMaths::FVector3& OvRendering::Geometry::VertexHelper::GetBitangentImpl(EmptyVertex& v) { return _zero3; }