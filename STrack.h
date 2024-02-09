#pragma once

#include <opencv2/opencv.hpp>
#include "BytekalmanFilter.h"
#include "Yolo.h"

enum TrackState { New = 0, Tracked, Lost, Removed };

class STrack
{
public:
	STrack( std::vector<float> tlwh_, float score, BoxInfo box);
	~STrack();

	 std::vector<float> static tlbr_to_tlwh( std::vector<float> &tlbr);
	void static multi_predict( std::vector<STrack*> &stracks, byte_kalman::ByteKalmanFilter &kalman_filter);
	void static_tlwha();
	void static_tlbra();
	static std::vector<float> tlwha_to_xyaha( std::vector<float> tlwha_tmp);
	static std::vector<float> xywha_to_tlbra(std::vector<float> xywha_tmp);
	 std::vector<float> to_xyaha();
	void mark_lost();
	void mark_removed();
	int next_id();
	int end_frame();
	
	void activate(byte_kalman::ByteKalmanFilter &kalman_filter, int frame_id);
	void re_activate(STrack &new_track, int frame_id, bool new_id = false);
	void update(STrack &new_track, int frame_id);

public:
	static int _count;
	bool is_activated;
	int track_id;
	int state;
	float angle;
	std::vector<float> _tlwha;
	std::vector<float> tlwha;
	std::vector<float> tlbra;
	BoxInfo boxinfo;
	int frame_id;
	int tracklet_len;
	int start_frame;

	KAL_MEAN mean;
	KAL_COVA covariance;
	float score;

private:
	byte_kalman::ByteKalmanFilter kalman_filter;
};
