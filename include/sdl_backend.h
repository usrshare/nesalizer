// Video, audio, and input backend. Uses SDL2.

#include <SDL.h>

void init_sdl();
void deinit_sdl();

// SDL rendering thread. Runs separately from the emulation thread.
void sdl_thread();

// Called from the emulation thread to cause the SDL thread to exit
void exit_sdl_thread();

// Video

void put_pixel(unsigned x, unsigned y, uint32_t color);
void draw_frame();

// Audio

int const sample_rate = 44100;

// Protect the audio buffer from concurrent access by the emulation thread and
// SDL
void lock_audio();
void unlock_audio();

// Stop and start audio playback in SDL
void start_audio_playback();
void stop_audio_playback();

// Input and events

void handle_ui_keys();

extern SDL_mutex *event_lock;
extern Uint8 const *keys;

int sdldbg_puts(const char* s);
int mvsdldbg_puts(const char* s,const char x, const char y);
int sdldbg_printf(const char* format, ...);
int mvsdldbg_printf(int x, int y, const char* format, ...);

extern Uint8 debug_contents[128*60];
extern Uint8 debug_colors[128*60];
