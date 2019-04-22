#pragma once

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#include <memory>
#include <thread>
#include <mutex>
#include <chrono>

#include <stdexcept>

#include "CEAudioSource.hpp"
#include "camera.h"

#define TEST_OPENAL_ERROR(_msg)    \
ALCenum error = alGetError();    \
if (error != AL_NO_ERROR) {  \
fprintf(stderr, _msg "\n");  \
throw std::runtime_error(_msg);   \
}

class CELocalPlayerController;

class LocalAudioManager
{
private:
  const float MAX_AMBIENT_GAIN = 0.5f;
  const float AMBIENT_FADE_IN_TIME_MS = 2000;

  std::shared_ptr<CELocalPlayerController> m_player_controller;

  /*
   * Collection of active audio sources for "one off" tracks
   */
  std::vector<std::shared_ptr<CEAudioSource>> m_current_audio_sources;
  std::mutex m_mutate_audio_sources;

  std::vector<std::shared_ptr<CEAudioSource>> m_ambient_queue;
  std::shared_ptr<CEAudioSource> m_next_ambient;
  double m_next_ambient_started_at;
  std::mutex m_mutate_next_ambient;
  std::mutex m_mutate_ambient_queue;

  /*
   * NOTE: we do not need to manage these objects; openAL internally handles
   * resource allocation and release.
   */
  ALCcontext* m_alc_context;
  ALCdevice* m_alc_device;
  bool m_ready;

  std::mutex m_audio_device_mutex;
  std::thread m_update_thread;

  void setupDevice();
  void destroyDevice();
  void listDevices(const ALCchar *devices);

  void startUpdateLoop();
  void update();
  void ingestNextAmbient();

public:
  LocalAudioManager();
  ~LocalAudioManager();

  void bind(std::shared_ptr<CELocalPlayerController> player_controller);
  
  void shutdown();

  /*
   * Play the given audio source as a one off.
   *
   * Will select a random location some distance from the given point.
   *
   * Initiation of audio will occur in a separate thread.
   */
  void play(std::shared_ptr<CEAudioSource> source);

  /*
   * Manage audio as main ambient track.
   *
   * If another ambient track is active, will fade transition immediately.
   */
  void playAmbient(std::shared_ptr<CEAudioSource> source);
};
