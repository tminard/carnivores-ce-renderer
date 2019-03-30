#pragma once

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#include <memory>
#include <thread>
#include <mutex>

#include <stdexcept>

#include "CEAudioSource.hpp"
#include "camera.h"

#define TEST_OPENAL_ERROR(_msg)    \
ALCenum error = alGetError();    \
if (error != AL_NO_ERROR) {  \
fprintf(stderr, _msg "\n");  \
throw std::runtime_error(_msg);   \
}

class LocalAudioManager
{
private:

  /*
   * Collection of active audio sources for "one off" tracks
   */
  std::vector<std::shared_ptr<CEAudioSource>> m_current_audio_sources;
  std::mutex m_mutate_audio_sources;

  /*
   * NOTE: we do not need to manage these objects; openAL internally handles
   * resource allocation and release.
   */
  ALCcontext* m_alc_context;
  ALCdevice* m_alc_device;

  std::mutex m_audio_device_mutex;

  void setupDevice();
  void destroyDevice();
  void listDevices(const ALCchar *devices);

public:
  LocalAudioManager();
  ~LocalAudioManager();

  /*
   * Loop through current audio and perform crossovers as needed; prune current pool.
   */
  void update(const Camera& camera);

  /*
   * Play the given audio source as a one off.
   *
   * Will select a random location some distance from the given point.
   *
   * Initiation of audio will occur in a separate thread.
   */
  void play(std::shared_ptr<CEAudioSource> source);
};
