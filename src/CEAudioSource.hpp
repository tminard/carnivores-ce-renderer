/*
 * Represents an audio object in space
 */
#pragma once

#include "dependency/libAF/af2-sound.h"
#include <glm/glm.hpp>

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#include <memory>

using libAF2::Sound;

class CEAudioSource
{
public:
  CEAudioSource(std::shared_ptr<Sound> sfx);
  CEAudioSource(const CEAudioSource& source);

  ~CEAudioSource();

  CEAudioSource& operator= (const CEAudioSource& source);

  void play();
  const bool isPlaying() const;

  void setLooped(bool looped);
  
  void setGain(float gain);

  /*
   * The position of the source in world space
   */
  void setPosition(glm::vec3 position);

  /*
   * The max distance that the audio will be heard
   */
  void setMaxDistance(int distance);

  /*
   * The minimum distance that the audio will be full gain before
   * dropping off.
   */
  void setClampDistance(int distance);

  void setNoDistance(float gain);

private:
  ALuint m_audio_buffer, m_audio_source;
  std::shared_ptr<Sound> m_original_audio;

  void upload();
};
