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

// Stop and start audio playback in SDL, returns old state.
int audio_pause(bool value);

// Input and events

void handle_ui_keys();

extern SDL_mutex *event_lock;
extern Uint8 const *keys;

extern bool show_debugger;

int sdldbg_puts(const char* s);
int mvsdldbg_puts(const char x, const char y,const char* s);
int sdldbg_printf(const char* format, ...);
int mvsdldbg_printf(int x, int y, const char* format, ...);
int sdldbg_move(int x, int y);
int sdldbg_clear(int width, int height);
int mvsdldbg_clear(int x, int y, int width, int height);
#define KM_SHIFT 128
#define KM_CTRL 256
#define KM_ALT 512

int sdldbg_getkey(void);
int sdldbg_getkey_nonblock(void);
int sdl_text_prompt(const char* prompt, char* value, size_t value_sz);

extern Uint8 debug_contents[128*60];
extern Uint8 debug_colors[128*60];
