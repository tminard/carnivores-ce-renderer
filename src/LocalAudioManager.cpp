#include "LocalAudioManager.hpp"

#include "CELocalPlayerController.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

LocalAudioManager::LocalAudioManager()
{
  m_next_ambient = nullptr;
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
    std::this_thread::sleep_for (std::chrono::milliseconds(100));

    lock_device.lock();
    ready = m_ready;
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

  source->setLooped(false);
  source->play();
}

void LocalAudioManager::playAmbient(std::shared_ptr<CEAudioSource> source)
{
  std::unique_lock<std::mutex> lock_device(this->m_audio_device_mutex);
  if (!m_ready) return;

  lock_device.unlock();

  std::lock_guard<std::mutex> guard(m_mutate_next_ambient);
  m_next_ambient.swap(source);
}

// See https://ffainelli.github.io/openal-example/
void LocalAudioManager::setupDevice()
{
  std::lock_guard<std::mutex> guard(this->m_audio_device_mutex);

  this->m_ready = false;
  this->m_alc_device = alcOpenDevice(NULL);
  checkError();

  if (!m_alc_device) {
    printf("Failed to create audio device. FATAL.\n");
    throw std::runtime_error("Could not create main audio device. FATAL.");
  }

  ALboolean enumeration, eax;
  enumeration = alcIsExtensionPresent(m_alc_device, "ALC_ENUMERATION_EXT");
  checkError();
  if (enumeration == AL_FALSE) {
    printf("\tAudio: enumeration not supported. FATAL.\n");
    throw std::runtime_error("Could not query audio parameters. FATAL.");
  }
  eax = alIsExtensionPresent("EAX2.0");
  checkError();
  if (eax == AL_TRUE) {
      printf("\tAudio: EAX 2.0 supported. OK.\n");
  }
  else {
      printf("\tAudio: EAX 2.0 NOT supported. OK.\n");
  }

  printf("\tAudio: enumeration supported. OK.\n");
  const ALCchar* devices = alcGetString(0, ALC_DEVICE_SPECIFIER);
  checkError();

  if (!devices) {
      throw std::runtime_error("Tried to get device list. Found nothing. FATAL.");
  }

  listDevices(devices);

  alGetError(); // Clear
  ALint attribs[] = { ALC_FREQUENCY, 22050,
               ALC_INVALID };
  this->m_alc_context = alcCreateContext(m_alc_device, attribs);
  checkError();

  if (!this->m_alc_context) {
      throw std::runtime_error("Could not create audio context. FATAL.");
  }

  if (!alcMakeContextCurrent(m_alc_context)) {
      checkError();
      throw std::runtime_error("Could not make default audio context. FATAL.");
  }
  TEST_OPENAL_ERROR("make default context");
}

void LocalAudioManager::bind(std::shared_ptr<CELocalPlayerController> player)
{
  if (this->m_player_controller || m_ready) {
    throw std::runtime_error("Cannot bind audio manager; manager already bound to player. Unbind first.");
  }

  this->m_player_controller = player;
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

void LocalAudioManager::checkError() {
    if (alGetError() != AL_NO_ERROR) {
        printf("Failed to set audio property!\n");
    }
}

void LocalAudioManager::update()
{
  const Camera* camera = m_player_controller->getCamera();

  glm::vec3 pos = camera->GetCurrentPos();
  glm::vec3 forward = camera->GetForward();
  glm::vec3 up = camera->GetUp();

  ALfloat listenerOri[] = { forward.x, forward.y, forward.z, up.x, up.y, up.z }; // forward and up

  alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
  checkError();
  alListener3f(AL_VELOCITY, 0, 0, 0);
  checkError();
  alListenerfv(AL_ORIENTATION, listenerOri);
  checkError();

  // Loop through currently playing sources and check status
  {
    std::lock_guard<std::mutex> guard(m_mutate_audio_sources);
    std::vector<std::shared_ptr<CEAudioSource>> playing;

    for(auto const& source: this->m_current_audio_sources) {
      if (source->isPlaying()) {
        playing.push_back(source);
      }
    }

    m_current_audio_sources = playing;
  }

  /* == Handle ambient == */

  // prune ambient vector to remove non-playing sources
  // TODO: generalize this?
  {
    std::lock_guard<std::mutex> guard(m_mutate_ambient_queue);
    std::vector<std::shared_ptr<CEAudioSource>> playing;

    for(auto const& source: this->m_ambient_queue) {
      if (source->isPlaying()) {
        playing.push_back(source);
      }
    }

    m_ambient_queue = playing;
  }

  // ingest staged ambient track
  this->ingestNextAmbient();

  {
    std::lock_guard<std::mutex> guard(m_mutate_ambient_queue);

    if (m_ambient_queue.size() >= 2) {
      double second_duration_sec = glfwGetTime() - m_next_ambient_started_at;
      float k1, k2;

      k2 = fminf((float(second_duration_sec*1000.f) / AMBIENT_FADE_IN_TIME_MS), 1.f);
      k1 = 1.f - k2;

      if (k1 <= 0.25) m_ambient_queue[0]->setLooped(false);

      m_ambient_queue[0]->setNoDistance(k1 * MAX_AMBIENT_GAIN);
      m_ambient_queue[1]->setNoDistance(k2 * MAX_AMBIENT_GAIN);
    }
  }
}

void LocalAudioManager::ingestNextAmbient()
{
  std::lock_guard<std::mutex> guard_queue(this->m_mutate_ambient_queue);
  if (m_ambient_queue.size() >= 2) return;

  std::lock_guard<std::mutex> guard(this->m_mutate_next_ambient);
  if (m_next_ambient == nullptr) return;

  std::shared_ptr<CEAudioSource> current = nullptr;

  if (!m_ambient_queue.empty()) {
    current = m_ambient_queue.back();

    if (current == m_next_ambient) {
      m_next_ambient = nullptr;

      return;
    }
  }

  m_next_ambient->setLooped(true);

  if (current == nullptr) {
    m_next_ambient->setNoDistance(MAX_AMBIENT_GAIN);
  } else {
    m_next_ambient->setNoDistance(0);
  }

  m_next_ambient_started_at = glfwGetTime();
  m_next_ambient->play();

  m_ambient_queue.push_back(m_next_ambient);
  m_next_ambient = nullptr;
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
    fprintf(stdout, "Device: %s\n", device);
    len = strlen(device);
    device += (len + 1);
    next += (len + 2);
  }
  fprintf(stdout, "----------\n");
}
