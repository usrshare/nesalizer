// Save state and rewinding implementation

void init_save_states_for_rom();
void deinit_save_states_for_rom();

// Plain old save state. Not related to rewinding.
void save_state();
void load_state();

#ifdef INCLUDE_REWIND
// Called once per frame to implementing rewinding. If 'do_rewind' is true, we
// should rewind.
void handle_rewind(bool do_rewind);
#endif

// Returns the length of the current frame in CPU ticks. Assumes we are
// currently rewinding. (There'd be no way to know the length if we hadn't
// already run the frame.)
unsigned get_frame_len();

#ifdef INCLUDE_REWIND
// True if the current frame should appear to run in reverse (e.g., w.r.t.
// audio)
extern bool is_backwards_frame;
#else
#define is_backwards_frame 0
#endif

