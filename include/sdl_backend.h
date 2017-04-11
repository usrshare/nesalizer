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
int sdldbg_mvputs(const char x, const char y,const char* s);
int sdldbg_printf(const char* format, ...);
int sdldbg_mvprintf(int x, int y, const char* format, ...);
int sdldbg_move(int x, int y);
int sdldbg_clear(int width, int height);
int sdldbg_mvclear(int x, int y, int width, int height);
#define KM_SHIFT 128
#define KM_CTRL 256
#define KM_ALT 512

int sdldbg_getkey(void);
int sdldbg_getkey_nonblock(void);
int sdl_text_prompt(const char* prompt, char* value, size_t value_sz);

#define DBG_SCRWIDTH 624
#define DBG_SCRHEIGHT 480
#define DBG_CHARWIDTH 6
#define DBG_CHARHEIGHT 8
#define DBG_COLUMNS (DBG_SCRWIDTH / DBG_CHARWIDTH)
#define DBG_ROWS (DBG_SCRHEIGHT / DBG_CHARHEIGHT)

extern Uint8 debug_contents[DBG_COLUMNS * DBG_ROWS];
extern Uint8 debug_colors[DBG_COLUMNS * DBG_ROWS];
