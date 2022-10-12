#include "OvRendering/Resources/Animation.h"

template<class T>
bool SearchTime(const std::vector<T>& keys, double time, T& result)
{
	int len = keys.size();
	if (len < 1)
		return false;
	if (len == 1)
	{
		result.value = keys[0].value;
		return true;
	}
	int i = 0;
	for (; i < keys.size(); i++)
	{
		if (keys[i].time > time)
			break;
	}

	if (i >= len)
	{
		result.value = keys[len - 1].value;
		return true;
	}
	double t1 = keys[i - 1].time;
	double t2 = keys[i].time;
	float factor = (time - t1) / (t2 - t1);
	result.value = keys[i - 1].value * t2 + keys[i].value * (1.0 - t2);
	return true;
	


	/*if (keys.size() == 1)
	{
		result.value = keys[0].value;
		return true;
	}
	int left = 0;
	int right = keys.size() - 1;
	auto vl = keys[left].time;
	auto vr = keys[right].time;
	while (left + 1 < right)
	{
		int middle = (left + right) / 2;
		auto vm = keys[middle].time;
		if (vm > time)
		{
			right = middle;
			vr = vm;
		}
		else
		{
			left = middle;
			vl = vm;
		}
	}
	if (left < right && right < keys.size())
	{
		auto& l = keys[left];
		auto& r = keys[right];
		result.value = (l.value * (r.time - time) + r.value * (time - l.time)) / (r.time - l.time);
		return true;
	}
	return false;*/
}


void OvRendering::Resources::MeshBoneAnimation::GetTransform(double time, MeshBoneTransform& result) const
{
	MeshBoneAnimationPositionKey positions_result;
	MeshBoneAnimationRotationKey rotations_result;
	MeshBoneAnimationScaleKey scales_result;
	if (SearchTime(positions, time, positions_result))
		result.position = positions_result.value;
	if (SearchTime(rotations, time, rotations_result))
		result.rotation = rotations_result.value;
	if (SearchTime(scales, time, scales_result))
		result.scale = scales_result.value;

}

void OvRendering::Resources::MeshBoneAnimation::GetTransform(double time, OvMaths::FMatrix4& result) const
{
	MeshBoneTransform cache;
	GetTransform(time, cache);
	result = OvMaths::FMatrix4::Translation(cache.position)* OvMaths::FQuaternion::ToMatrix4(OvMaths::FQuaternion::Normalize(cache.rotation))* OvMaths::FMatrix4::Scaling(cache.scale);
}
