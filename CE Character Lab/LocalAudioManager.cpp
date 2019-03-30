#include "LocalAudioManager.hpp"

#include "CEPlayer.hpp"

LocalAudioManager::LocalAudioManager()
{
  this->setupDevice();
}

LocalAudioManager::~LocalAudioManager()
{
  printf("Destroying audio manager\n");

  this->shutdown();
  this->destroyDevice();
}

void LocalAudioManager::startUpdateLoop()
{
    std::unique_lock<std::mutex> lock_device(this->m_audio_device_mutex);
  if (!m_ready) {
    throw std::runtime_error("Device is not ready; cannot start loop. Check your init order. FATAL.");
  }

  bool ready = m_ready;
  lock_device.unlock();

  while (ready) {

    this->update();
    std::this_thread::sleep_for (std::chrono::seconds(1));

    lock_device.lock();
    ready = m_ready;
    printf("Audio Look Update\n");
    lock_device.unlock();
  }
}

void LocalAudioManager::play(std::shared_ptr<CEAudioSource> source)
{
  std::unique_lock<std::mutex> lock_device(this->m_audio_device_mutex);
  if (!m_ready) return;

  lock_device.unlock();

  std::lock_guard<std::mutex> guard(m_mutate_audio_sources);
  m_current_audio_sources.push_back(source);

  source->play();
}

// See https://ffainelli.github.io/openal-example/
void LocalAudioManager::setupDevice()
{
  std::lock_guard<std::mutex> guard(this->m_audio_device_mutex);

  this->m_ready = false;
  this->m_alc_device = alcOpenDevice(NULL);

  if (!m_alc_device) {
    printf("Failed to create audio device. FATAL.\n");
    throw std::runtime_error("Could not create main audio device. FATAL.");
  }

  ALboolean enumeration;
  enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
  if (enumeration == AL_FALSE) {
    printf("Audio: enumeration not supported. FATAL.\n");
    throw std::runtime_error("Could not query audio parameters. FATAL.");
  } else {
    printf("Audio: enumeration supported. OK.\n");
    listDevices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
  }

  alGetError(); // Prime
  this->m_alc_context = alcCreateContext(m_alc_device, NULL);
  if (!alcMakeContextCurrent(m_alc_context)) {
    throw std::runtime_error("Could not make default audio context. FATAL.");
  }
  TEST_OPENAL_ERROR("make default context");
}

void LocalAudioManager::bind(std::shared_ptr<CEPlayer> player)
{
  if (this->m_player || m_ready) {
    throw std::runtime_error("Cannot bind audio manager; manager already bound to player. Unbind first.");
  }

  this->m_player = player;
  this->update();

  m_ready = true;
  m_update_thread = std::thread([this] { startUpdateLoop(); });
}

/*
 * System is shutting down; don't free resources yet (save that for deallocation) but mark this system as done and disable threaded processes
 */
void LocalAudioManager::shutdown()
{
  // Prevent potential CPU reordering
  {
    std::lock_guard<std::mutex> guard(m_audio_device_mutex);
    m_ready = false;
  }

  if (m_update_thread.joinable()) m_update_thread.join(); // wait for the tread to terminate
}

void LocalAudioManager::update()
{
  Camera* camera = m_player->getCamera();

  glm::vec3 pos = camera->GetCurrentPos();
  glm::vec3 forward = camera->GetForward();
  glm::vec3 up = camera->GetUp();

  ALfloat listenerOri[] = { forward.x, forward.y, forward.z, up.x, up.y, up.z }; // forward and up

  alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
  alListener3f(AL_VELOCITY, 0, 0, 0);
  alListenerfv(AL_ORIENTATION, listenerOri);

  // Loop through currently playing sources and check status
  std::lock_guard<std::mutex> guard(m_mutate_audio_sources);
  std::vector<std::shared_ptr<CEAudioSource>> playing;

  for(auto const& source: this->m_current_audio_sources) {
    if (source->isPlaying()) {
      // print current duration
      playing.push_back(source);
    }
  }

  m_current_audio_sources = playing;
}

void LocalAudioManager::destroyDevice()
{
  std::lock_guard<std::mutex> guard(m_audio_device_mutex);

  alcMakeContextCurrent(NULL);
  alcDestroyContext(m_alc_context);
  alcCloseDevice(m_alc_device);
}

void LocalAudioManager::listDevices(const ALCchar *devices)
{
  const ALCchar *device = devices, *next = devices + 1;
  size_t len = 0;

  fprintf(stdout, "Devices list:\n");
  fprintf(stdout, "----------\n");
  while (device && *device != '\0' && next && *next != '\0') {
    fprintf(stdout, "%s\n", device);
    len = strlen(device);
    device += (len + 1);
    next += (len + 2);
  }
  fprintf(stdout, "----------\n");
}
