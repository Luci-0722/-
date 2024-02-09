#include "STrack.h"

STrack::STrack( std::vector<float> tlwha_, float score, BoxInfo box)
{
	angle = box.box.angle;

	_tlwha.resize(5);
	_tlwha.assign(tlwha_.begin(), tlwha_.end());

	is_activated = false;
	track_id = 0;
	state = TrackState::New;
	
	tlwha.resize(5);
	tlbra.resize(5);

	static_tlwha();
	static_tlbra();
	frame_id = 0;
	tracklet_len = 0;
	this->score = score;
	start_frame = 0;
}

STrack::~STrack()
{
}

void STrack::activate(byte_kalman::ByteKalmanFilter &kalman_filter, int frame_id)
{
	this->kalman_filter = kalman_filter;
	this->track_id = this->next_id();

	 std::vector<float> _tlwh_tmp(4);
	_tlwh_tmp[0] = this->_tlwha[0];
	_tlwh_tmp[1] = this->_tlwha[1];
	_tlwh_tmp[2] = this->_tlwha[2];
	_tlwh_tmp[3] = this->_tlwha[3];
	 std::vector<float> xyaha = tlwha_to_xyaha(_tlwh_tmp);
	DETECTBOX xyaha_box;
	xyaha_box[0] = xyaha[0];
	xyaha_box[1] = xyaha[1];
	xyaha_box[2] = xyaha[2];
	xyaha_box[3] = xyaha[3];
	auto mc = this->kalman_filter.initiate(xyaha_box);
	this->mean = mc.first;
	this->covariance = mc.second;

	static_tlwha();
	static_tlbra();

	this->tracklet_len = 0;
	this->state = TrackState::Tracked;
	if (frame_id == 1)
	{
		this->is_activated = true;
	}
	//this->is_activated = true;
	this->frame_id = frame_id;
	this->start_frame = frame_id;
}

void STrack::re_activate(STrack &new_track, int frame_id, bool new_id)
{
	std::vector<float> xyaha = tlwha_to_xyaha(new_track.tlwha);
	DETECTBOX xyaha_box;
	xyaha_box[0] = xyaha[0];
	xyaha_box[1] = xyaha[1];
	xyaha_box[2] = xyaha[2];
	xyaha_box[3] = xyaha[3];
	//xyaha_box[4] = xyaha[4];
	auto mc = this->kalman_filter.update(this->mean, this->covariance, xyaha_box);
	this->mean = mc.first;
	this->covariance = mc.second;

	static_tlwha();
	static_tlbra();

	this->tracklet_len = 0;
	this->state = TrackState::Tracked;
	this->is_activated = true;
	this->frame_id = frame_id;
	this->score = new_track.score;
	if (new_id)
		this->track_id = next_id();
}

void STrack::update(STrack &new_track, int frame_id)
{
	this->frame_id = frame_id;
	this->tracklet_len++;
	angle = new_track.angle;
	std::vector<float> xyaha = tlwha_to_xyaha(new_track.tlwha);
	DETECTBOX xyaha_box;
	xyaha_box[0] = xyaha[0];
	xyaha_box[1] = xyaha[1];
	xyaha_box[2] = xyaha[2];
	xyaha_box[3] = xyaha[3];
	//xyaha_box[4] = xyaha[4];
	auto mc = this->kalman_filter.update(this->mean, this->covariance, xyaha_box);
	this->mean = mc.first;
	this->covariance = mc.second;

	static_tlwha();
	static_tlbra();

	this->state = TrackState::Tracked;
	this->is_activated = true;

	this->score = new_track.score;
}



void STrack::static_tlwha()
{

	if (this->state == TrackState::New)
	{
		tlwha[0] = _tlwha[0];
		tlwha[1] = _tlwha[1];
		tlwha[2] = _tlwha[2];
		tlwha[3] = _tlwha[3];
		return;
	}

	tlwha[0] = mean[0];
	tlwha[1] = mean[1];
	tlwha[2] = mean[2];
	tlwha[3] = mean[3];

	tlwha[2] *= tlwha[3];
	tlwha[0] -= tlwha[2] / 2;
	tlwha[1] -= tlwha[3] / 2;
}


void STrack::static_tlbra()
{
	tlbra.clear();
	tlbra.assign(tlwha.begin(), tlwha.end());
	tlbra[2] += tlbra[0];
	tlbra[3] += tlbra[1];
}

 std::vector<float> STrack::tlwha_to_xyaha( std::vector<float> tlwha_tmp)
{
	 std::vector<float> tlwha_output = tlwha_tmp;
	 tlwha_output[0] += tlwha_output[2] / 2;
	 tlwha_output[1] += tlwha_output[3] / 2;
	 tlwha_output[2] /= tlwha_output[3];
	return tlwha_output;
}

 std::vector<float> STrack::xywha_to_tlbra(std::vector<float> xywha_tmp)
 {
	 std::vector<float> tlbr_result(5);
	 float x = xywha_tmp[0];
	 float y = xywha_tmp[1];
	 float width = xywha_tmp[2];
	 float height = xywha_tmp[3];

	 // 计算左上角坐标和长宽信息
	 float left = x - width / 2.0;
	 float top = y + height / 2.0;
	 float right = x + width / 2.0;
	 float down = y - height / 2.0;

	 float tlwh_width = width;
	 float tlwh_height = height;

	 // 构建tlwh结果向量
	 tlbr_result[0] = left;
	 tlbr_result[1] = down;
	 tlbr_result[2] = right;
	 tlbr_result[3] = top;
	 tlbr_result[4] = xywha_tmp[4];
	 return tlbr_result;
 }

 std::vector<float> STrack::to_xyaha()
{
	return tlwha_to_xyaha(tlwha);
}

 std::vector<float> STrack::tlbr_to_tlwh( std::vector<float> &tlbr)
{
	tlbr[2] -= tlbr[0];
	tlbr[3] -= tlbr[1];
	return tlbr;
}

void STrack::mark_lost()
{
	state = TrackState::Lost;
}

void STrack::mark_removed()
{
	state = TrackState::Removed;
}

int STrack::next_id()
{
	_count++;
	return _count;
}

int STrack::end_frame()
{
	return this->frame_id;
}

void STrack::multi_predict( std::vector<STrack*> &stracks, byte_kalman::ByteKalmanFilter &kalman_filter)
{
	for (int i = 0; i < stracks.size(); i++)
	{
		if (stracks[i]->state != TrackState::Tracked)
		{
			stracks[i]->mean[7] = 0;
		}
		kalman_filter.predict(stracks[i]->mean, stracks[i]->covariance);
	}
}

int STrack::_count = 0;