#include "LocalAudioManager.hpp"

LocalAudioManager::LocalAudioManager()
{
  this->setupDevice();
}

LocalAudioManager::~LocalAudioManager()
{
  this->destroyDevice();
}

// See https://ffainelli.github.io/openal-example/
void LocalAudioManager::setupDevice()
{
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

void LocalAudioManager::update(const Camera& camera)
{
  glm::vec3 pos = camera.GetCurrentPos();
  glm::vec3 forward = camera.GetForward();
  glm::vec3 up = camera.GetUp();
  ALfloat listenerOri[] = { forward.x, forward.y, forward.z, up.x, up.y, up.z }; // forward and up

  alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
  alListener3f(AL_VELOCITY, 0, 0, 0);
  alListenerfv(AL_ORIENTATION, listenerOri);
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
