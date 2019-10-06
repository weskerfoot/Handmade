/* This header file implements my audio APIs */
/* It is all built on SDL2, since ALSA and pulse are a bit too low level */

int
initializeSound(void) {
  uint32_t sdl_flag = SDL_INIT_AUDIO;
  if (SDL_Init(sdl_flag) != 0) {
    SDL_Log("Could not initialize audio subsystem!");
    return 1;
  }
  return 0;
}

void
playFile(char *filename) {
  SDL_AudioSpec wavspec;
  uint32_t wavlength;
  uint8_t *wavbuf;

  /* Open the file and get the wavspec, wavbuf, and wavlength filled */
  SDL_LoadWAV(filename, &wavspec, &wavbuf, &wavlength);

  SDL_AudioDeviceID device_id = SDL_OpenAudioDevice(NULL, 0, &wavspec, NULL, 0);

  int success = SDL_QueueAudio(device_id, wavbuf, wavlength);
  SDL_PauseAudioDevice(device_id, 0);
}
