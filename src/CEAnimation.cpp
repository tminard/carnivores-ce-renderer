#include "CEAnimation.h"

CEAnimation::CEAnimation(const std::string& ani_name, int kps, int total_frames, int total_time_ms)
    : m_name(ani_name), m_kps(kps), m_number_of_frames(total_frames), m_total_time(total_time_ms) {
      m_animation_start_at = 0.0;
      m_animation_last_update_at = 0.0;
}

CEAnimation::~CEAnimation() {
}

void CEAnimation::setAnimationData(std::vector<short int> raw_animation_data, int vcount, std::vector<TFace> faces, std::vector<TPoint3d> original_vertices) {
    m_animation_data.assign(raw_animation_data.begin(), raw_animation_data.end());
  // Sadly we need to keep this around since we need it to rebuild the faces after updating mesh with animation data.
  m_faces = faces;
  m_original_vertices = original_vertices;
}

std::shared_ptr<const std::vector<short int>> CEAnimation::GetAnimationData() const {
    return std::make_shared<const std::vector<short int>>(m_animation_data);
}

std::shared_ptr<const std::vector<TFace>> CEAnimation::GetFaces() const {
    return std::make_shared<const std::vector<TFace>>(m_faces);
}

std::shared_ptr<const std::vector<TPoint3d>> CEAnimation::GetOriginalVertices() const {
    return std::make_shared<const std::vector<TPoint3d>>(m_original_vertices);
}
