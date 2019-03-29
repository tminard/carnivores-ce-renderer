#pragma once

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#include <memory>
#include <thread>
#include <mutex>

#include "CEAudioSource.hpp"

using std::shared_ptr;

class LocalAudioManager
{
private:
  std::vector<shared_ptr<CEAudioSource>> m_current_sources;

public:
  LocalAudioManager();
  ~LocalAudioManager();

  /*
   * Play the given audio source.
   * This will retain a shared pointer to the source audio until the play completes
   */
  void play(shared_ptr<CEAudioSource> source);
};
