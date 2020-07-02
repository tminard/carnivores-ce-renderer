#include "CEAudioSource.hpp"

CEAudioSource::CEAudioSource(std::shared_ptr<Sound> sfx) : m_original_audio(sfx)
{
    this->upload();
}

CEAudioSource::CEAudioSource(const CEAudioSource& source)
{
    this->m_original_audio = source.m_original_audio;
    this->upload();
}

CEAudioSource& CEAudioSource::operator=(const CEAudioSource& source)
{
    this->m_original_audio = source.m_original_audio;
    this->upload();

    return (*this);
}

CEAudioSource::~CEAudioSource()
{
    alDeleteSources(1, &m_audio_source);
    alDeleteBuffers(1, &m_audio_buffer);
}

// see http://openal.996291.n3.nabble.com/AL-REFERENCE-DISTANCE-Question-td3782.html and https://www.openal.org/documentation/OpenAL_Programmers_Guide.pdf
void CEAudioSource::upload()
{
    alGenSources((ALuint)1, &m_audio_source);

    alSourcef(m_audio_source, AL_PITCH, 1);
    alSourcef(m_audio_source, AL_GAIN, 1);
    alSource3f(m_audio_source, AL_POSITION, 0, 0, 0);
    alSource3f(m_audio_source, AL_VELOCITY, 0, 0, 0);

    alSourcei(m_audio_source, AL_ROLLOFF_FACTOR, 1); // decline in gain starting at ref distance, through max distance
    alSourcei(m_audio_source, AL_MAX_DISTANCE, (256*100)); // distance until gain is 0
    alSourcei(m_audio_source, AL_REFERENCE_DISTANCE, (256*1)); // clamped distance; up to distance where gain is always 1

    alGenBuffers(1, &m_audio_buffer);
    alBufferData(m_audio_buffer, AL_FORMAT_MONO16, (ALvoid*)this->m_original_audio->getWaveData(), (ALsizei)this->m_original_audio->getLength(), (ALsizei)this->m_original_audio->getFrequency());

    alSourcei(m_audio_source, AL_BUFFER, m_audio_buffer);
}

const bool CEAudioSource::isPlaying() const
{
  ALint source_state;
  alGetSourcei(m_audio_source, AL_SOURCE_STATE, &source_state);

  if (source_state == AL_PLAYING) {
    return true;
  } else {
    return false;
  }
}

void CEAudioSource::setLooped(bool looped)
{
    if (looped) {
        alSourcei(m_audio_source, AL_LOOPING, AL_TRUE);
    } else {
        alSourcei(m_audio_source, AL_LOOPING, AL_FALSE);
    }
}

void CEAudioSource::setNoDistance(float gain)
{
    alSourcef(m_audio_source, AL_GAIN, gain);
    alSourcei(m_audio_source, AL_SOURCE_RELATIVE, AL_TRUE); // true means the below pos is an offset against the current listener position
    alSource3f(m_audio_source, AL_POSITION, 0.0, -1.0, 0.0); // TODO: make this an enum of "at, behind, right" etc
}

void CEAudioSource::setMaxDistance(int distance)
{
    alSourcei(m_audio_source, AL_MAX_DISTANCE, distance);
}

void CEAudioSource::setClampDistance(int distance)
{
    alSourcei(m_audio_source, AL_REFERENCE_DISTANCE, distance);
}

void CEAudioSource::play()
{
    alSourcePlay(m_audio_source);
}

void CEAudioSource::setPosition(glm::vec3 position)
{
    alSource3f(m_audio_source, AL_POSITION, position.x, position.y, position.z);
}



