#pragma once

#include <mpv/client.h>
#include <mpv/render.h>
#include <mpv/render_gl.h>

/*
    Due to linking issues related to LuaJit symbols in mpv's libraries.
    We load the functions dynamically at runtime.
    This way we don't need to link mpv. We only need the headers.

    This is only a subset of libmpv functions.
*/
#ifdef __cplusplus
extern "C" {
#endif

typedef mpv_handle *(*mpv_create_FUNC)();
typedef int (*mpv_initialize_FUNC)(mpv_handle *ctx);
typedef mpv_event *(*mpv_wait_event_FUNC)(mpv_handle *ctx, double timeout);
typedef int (*mpv_observe_property_FUNC)(mpv_handle *mpv, uint64_t reply_userdata, const char *name, mpv_format format);
typedef uint64_t (*mpv_render_context_update_FUNC)(mpv_render_context *ctx);
typedef int (*mpv_render_context_render_FUNC)(mpv_render_context *ctx, mpv_render_param *params);
typedef int (*mpv_set_option_string_FUNC)(mpv_handle *ctx, const char *name, const char *data);
typedef int (*mpv_set_property_string_FUNC)(mpv_handle *ctx, const char *name, const char *data);
typedef int (*mpv_request_log_messages_FUNC)(mpv_handle *ctx, const char *min_level);
typedef int (*mpv_command_async_FUNC)(mpv_handle *ctx, uint64_t reply_userdata, const char **args);
typedef int (*mpv_render_context_create_FUNC)(mpv_render_context **res, mpv_handle *mpv, mpv_render_param *params);
typedef void (*mpv_set_wakeup_callback_FUNC)(mpv_handle *ctx, void (*cb)(void *d), void *d);
typedef void (*mpv_render_context_set_update_callback_FUNC)(mpv_render_context *ctx, mpv_render_update_fn callback, void *callback_ctx);
typedef int (*mpv_set_property_async_FUNC)(mpv_handle *ctx, uint64_t reply_userdata, const char *name, mpv_format format, void *data);
typedef void (*mpv_render_context_free_FUNC)(mpv_render_context *ctx);
typedef void (*mpv_detach_destroy_FUNC)(mpv_handle *ctx);
typedef void (*mpv_destroy_FUNC)(mpv_handle *ctx);
typedef void (*mpv_render_context_report_swap_FUNC)(mpv_render_context *ctx);
typedef void (*mpv_terminate_destroy_FUNC)(mpv_handle *ctx);
typedef char const* (*mpv_error_string_FUNC)(int);

struct OFS_MpvLoader {
#define DECLARE_FUNCTION(FN) static FN##_FUNC  FN##_REAL

    static mpv_create_FUNC mpv_create_REAL;
    static mpv_initialize_FUNC mpv_initialize_REAL;
    static mpv_wait_event_FUNC mpv_wait_event_REAL;
    static mpv_observe_property_FUNC mpv_observe_property_REAL;
    static mpv_render_context_update_FUNC mpv_render_context_update_REAL;
    static mpv_render_context_render_FUNC mpv_render_context_render_REAL;
    static mpv_set_option_string_FUNC mpv_set_option_string_REAL;
    static mpv_set_property_string_FUNC mpv_set_property_string_REAL;
    static mpv_request_log_messages_FUNC mpv_request_log_messages_REAL;
    static mpv_command_async_FUNC mpv_command_async_REAL;
    static mpv_render_context_create_FUNC mpv_render_context_create_REAL;
    static mpv_set_wakeup_callback_FUNC mpv_set_wakeup_callback_REAL;
    static mpv_render_context_set_update_callback_FUNC mpv_render_context_set_update_callback_REAL;
    static mpv_set_property_async_FUNC mpv_set_property_async_REAL;
    static mpv_render_context_free_FUNC mpv_render_context_free_REAL;
    static mpv_destroy_FUNC mpv_destroy_REAL;
    static mpv_render_context_report_swap_FUNC mpv_render_context_report_swap_REAL;
    static mpv_terminate_destroy_FUNC mpv_terminate_destroy_REAL;

    DECLARE_FUNCTION(mpv_error_string);

#undef DECLARE_FUNCTION
    static bool Load() noexcept;
    static void Unload() noexcept;
};

#ifdef OFS_MPV_LOADER_MACROS
#include "OFS_MpvMacros.h"
#endif // OFS_MPV_LOADER_NO_MACROS

#ifdef __cplusplus
}
#endif
