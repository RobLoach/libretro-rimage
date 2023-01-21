#include <stdlib.h> // free(), malloc()
#include <stdbool.h> // bool
#include <stdarg.h> // va_list
#include <stdio.h> // stderr
#include <string.h>

#include "libretro.h"

#define RIMAGE_IMPLEMENTATION
#include "rimage.h"

typedef struct Core {
   Image backBuffer;
   Image frontBuffer;

   retro_video_refresh_t video_cb;
   retro_audio_sample_t audio_cb;
   retro_audio_sample_batch_t audio_batch_cb;
   retro_input_poll_t input_poll_cb;
   retro_input_state_t input_state_cb;

} Core;

retro_environment_t environ_cb;
Core* core;
retro_log_printf_t log_cb;
struct retro_log_callback logging;

void TraceLogFallback(enum retro_log_level level, const char *fmt, ...) {
   (void)level;
   va_list va;
   va_start(va, fmt);
   vfprintf(stderr, fmt, va);
   va_end(va);
}

Core* GetCoreHandle() {
   return core;
}

bool IsCoreReady() {
   return core != NULL;
}

void CloseCore() {
   if (!IsCoreReady()) {
      return;
   }

   UnloadImage(core->backBuffer);
   UnloadImage(core->frontBuffer);

   free(core);
   core = NULL;
}

int GetScreenWidth() {
   if (IsCoreReady()) {
      return core->backBuffer.width;
   }

   return 0;
}

int GetScreenHeight() {
   if (IsCoreReady()) {
      return core->backBuffer.height;
   }

   return 0;
}

bool InitCore() {
   // Make sure the core is available to be loaded.
   CloseCore();

   core = malloc(sizeof(Core));

   // Initialize the screen buffers. Back as RGBA8888, front as RGB565.
   int width = 400;
   int height = 225;
   core->backBuffer = GenImageColor(width, height, RED); // RGBA8888
   core->frontBuffer = GenImageColor(width, height, BLUE);
   ImageFormat(&core->frontBuffer, PIXELFORMAT_UNCOMPRESSED_R5G6B5);

   return true;
}

void retro_init(void) {
   InitCore();
}

void retro_deinit(void) {
   CloseCore();
}

unsigned retro_api_version(void) {
   return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned port, unsigned device) {}

void retro_get_system_info(struct retro_system_info *info) {
   memset(info, 0, sizeof(*info));
   info->library_name     = "rimage";
   info->library_version  = "0.0.1";
   info->need_fullpath    = false;
   info->valid_extensions = "png";
   info->block_extract    = false;
}

void retro_get_system_av_info(struct retro_system_av_info *info) {
   if (!IsCoreReady()) {
      return;
   }
   
   info->geometry.base_width   = GetScreenWidth();
   info->geometry.base_height  = GetScreenHeight();
   info->geometry.max_width    = GetScreenWidth();
   info->geometry.max_height   = GetScreenHeight();
   info->geometry.aspect_ratio = (float)GetScreenWidth() / (float)GetScreenHeight();
   info->timing.fps = 60.0;
   info->timing.sample_rate = 44100.0f;
}

void TraceLog(int logLevel, const char *text, ...) {
   enum retro_log_level level;
   switch (logLevel) {
      case LOG_ALL:
         level = RETRO_LOG_INFO;
         break;
      case LOG_TRACE:
         level = RETRO_LOG_DEBUG;
         break;
      case LOG_DEBUG:
         level = RETRO_LOG_DEBUG;
         break;
      case LOG_INFO:
         level = RETRO_LOG_INFO;
         break;
      case LOG_WARNING:
         level = RETRO_LOG_WARN;
         break;
      case LOG_ERROR:
         level = RETRO_LOG_ERROR;
         break;
      case LOG_FATAL:
         level = RETRO_LOG_ERROR;
         break;
      case LOG_NONE:
         return;
   }

   va_list va;
   va_start(va, text);
   if (log_cb != NULL) {
      log_cb(level, text, va);
   }
   else {
      TraceLogFallback(level, text, va);
   }
   va_end(va);
}

void retro_set_environment(retro_environment_t cb) {

   environ_cb = cb;

   if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging)) {
      log_cb = logging.log;
   }
   else {
      log_cb = TraceLogFallback;
   }

   bool supports_no_game = true;
   cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &supports_no_game);
}

void retro_set_audio_sample(retro_audio_sample_t cb) {
   if (!IsCoreReady()) {
      return;
   }

   core->audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) {
   if (!IsCoreReady()) {
      return;
   }

   core->audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb) {
   if (!IsCoreReady()) {
      return;
   }

   core->input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb) {
   if (!IsCoreReady()) {
      return;
   }

   core->input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb) {
   if (!IsCoreReady()) {
      return;
   }

   core->video_cb = cb;
}

void retro_reset(void) {
   TraceLog(LOG_INFO, "retro_reset");
}

void PollInputEvents(void) {
   // TODO: Update the input state
}

static void check_variables(void) {}

static void audio_callback(void) {
}

static void audio_set_state(bool enable) {
   (void)enable;
}

void ClearBackground(Color color) {
   ImageClearBackground(&core->backBuffer, color);
}

void UpdateGame() {
   if (!IsCoreReady()) {
      return;
   }

   ClearBackground(RAYWHITE);
   ImageDrawCircle(&core->backBuffer, 300, 100, 30, SKYBLUE);
   ImageDrawRectangle(&core->backBuffer, 100, 100, 100, 140, GREEN);
   ImageDrawLine(&core->backBuffer, 50, 50, 200, 80, ORANGE);
   ImageDrawRectangle(&core->backBuffer, 20, 150, 50, 80, RED);
}

void retro_run(void) {
   if (!IsCoreReady()) {
      return;
   }
   PollInputEvents();

   bool updated = false;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated) {
      check_variables();
   }

   UpdateGame();
   // Render the backbuffer to the front buffer.
   Rectangle screenRect = {0, 0, core->frontBuffer.width, core->frontBuffer.height};
   ImageDraw(&core->frontBuffer, core->backBuffer, screenRect, screenRect, WHITE);

   size_t pitch = (size_t)GetPixelDataSize(core->frontBuffer.width, 1, core->frontBuffer.format);
   core->video_cb((const void*)core->frontBuffer.data, core->frontBuffer.width, core->frontBuffer.height, pitch);
}

bool LoadGame(const void* data, size_t size, const char* path) {
   if (!IsCoreReady()) {
      return false;
   }
   TraceLog(LOG_INFO, "LoadGame\n");

   // Load the game.

   return true;
}

void UnloadGame() {
   if (!IsCoreReady()) {
      return;
   }
   TraceLog(LOG_INFO, "UnloadGame\n");

   // Unload the game
}

bool retro_load_game(const struct retro_game_info *info) {
   if (!IsCoreReady()) {
      return false;
   }
   // The pixel format is the same as the front buffer.
   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt)) {
      TraceLog(LOG_ERROR, "RGB565 isn't supported\n");
      return false;
   }

   check_variables();

   if (info) {
      return LoadGame(info->data, info->size, info->path);
   }
   else {
      return LoadGame(NULL, 0, "");
   }
}

void retro_unload_game(void) {
   UnloadGame();
}

unsigned retro_get_region(void) {
   return RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num) {
   return false;
}

size_t retro_serialize_size(void) {
   return 0;
}

bool retro_serialize(void *data_, size_t size) {
   return false;
}

bool retro_unserialize(const void *data_, size_t size) {
   return false;
}

void *retro_get_memory_data(unsigned id) {
   (void)id;
   return NULL;
}

size_t retro_get_memory_size(unsigned id) {
   (void)id;
   return 0;
}

void retro_cheat_reset(void) {}

void retro_cheat_set(unsigned index, bool enabled, const char *code) {
   (void)index;
   (void)enabled;
   (void)code;
}

