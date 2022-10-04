#include "OvRendering/Resources/Animation.h"

template<class T>
void SearchTime(std::vector<T> keys, double time, int& left, int& right)
{
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
}


void OvRendering::Resources::MeshBoneAnimation::GetTransform(double time, MeshBoneTransform& result)
{
	int pos_index_l = 0;
	int pos_index_r = positions.size();
	int rot_index_l = 0;
	int rot_index_r = rotations.size();
	int scl_index_l = 0;
	int scl_index_r = scales.size();
	SearchTime(positions, time, pos_index_l, pos_index_r);
	SearchTime(rotations, time, rot_index_l, rot_index_r);
	SearchTime(scales, time, scl_index_l, scl_index_r);

	if (pos_index_l < pos_index_r)
	{
		auto& l = positions[pos_index_l];
		auto& r = positions[pos_index_r];
		result.position = (l.value * (r.time - time) + r.value * (time - l.time)) / (r.time - l.time);
	}

	if (rot_index_l < rot_index_r)
	{
		auto& l = rotations[rot_index_l];
		auto& r = rotations[rot_index_r];
		result.rotation = (l.value * (r.time - time) + r.value * (time - l.time)) / (r.time - l.time);
	}

	if (scl_index_l < scl_index_r)
	{
		auto& l = scales[scl_index_l];
		auto& r = scales[scl_index_r];
		result.scale = (l.value * (r.time - time) + r.value * (time - l.time)) / (r.time - l.time);
	}
}
