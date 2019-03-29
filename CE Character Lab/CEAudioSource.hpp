/*
 * Represents an audio object in space
 */
#pragma once

#include "deps/libAF/af2-sound.h"
#include <glm/glm.hpp>
#include <OpenAL/al.h>

#include <memory>

using libAF2::Sound;

class CEAudioSource
{
public:
  CEAudioSource(Sound* sfx);
  CEAudioSource(const CEAudioSource& source);

  ~CEAudioSource();

  CEAudioSource& operator= (const CEAudioSource& source);

  void play();

  void setLooped(bool looped);

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

  // void setDropoff(int dropoff) - or an enum of QUIET, LOUD, VERY_LOUD? Maybe we abstract this a bit and instead make this API declaritive (ie "this is a loud sound that will carry far" etc)

private:
  ALuint m_audio_buffer, m_audio_source;
  Sound* m_original_audio;

  void upload();
};