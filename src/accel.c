#include <pebble.h>
#include "screen.h"
#include "configs.h"
#include "keys.h"

#if !defined PBL_PLATFORM_APLITE
static bool initialized;
static int lastPassX;
static int lastPassY;
static int lastPassZ;
static int lastX;
static int lastY;
static int lastZ;
static bool begin_tap;
static bool mid_tap;
static bool end_tap;
static bool neg;
static int mid_tap_count;
static int end_tap_count;
static bool show_tap_mode;
static bool show_wrist_mode;
static int timeout_sec = 0;

static Window *watchface_ref;

static void reset_tap() {
    begin_tap = false;
    mid_tap = false;
    end_tap = false;
    end_tap_count = 0;
    mid_tap_count = 0;
}

void reset_tap_handler() {
    show_tap_mode = false;
    redraw_screen(watchface_ref);
}

void accel_data_handler(AccelData *data, uint32_t num_samples) {
    if (show_tap_mode) {
        return;
    }

    int passX[2];
    int passY[2];
    int passZ[2];
    int Z[2];
    int X[2];
    int Y[2];

    if (!initialized) {
        passX[0] = passX[1] = data[0].x;
        passY[0] = passY[1] = data[0].y;
        passZ[0] = passZ[1] = data[0].z;
        Z[0] = Z[1] = data[0].z;
        X[0] = X[1] = data[0].x;
        Y[0] = Y[1] = data[0].y;
        initialized = true;
        show_tap_mode = false;
        reset_tap();
    } else {
        passX[0] = passX[1] = lastPassX;
        passY[0] = passY[1] = lastPassY;
        passZ[0] = passZ[1] = lastPassZ;
        X[0] = X[1] = lastX;
        Y[0] = Y[1] = lastY;
        Z[0] = Z[1] = lastZ;
    }

    int factor = 4;
    int high_threshold = 40;
    int low_threshold = 10;
    int threshold_other_axis = 15;
    int range = 2;
    int x_threshold = 350;
    int y_threshold = 150;
    for (int i = 0; i < (int)num_samples; ++i) {
        if (data[i].did_vibrate) {
            reset_tap();
            continue;
        }
        Z[1] = data[i].z;
        X[1] = data[i].x;
        Y[1] = data[i].y;

        passX[1] = (int) ((passX[0] + X[1] - X[0])/factor);
        passY[1] = (int) ((passY[0] + Y[1] - Y[0])/factor);
        passZ[1] = (int) ((passZ[0] + Z[1] - Z[0])/factor);

        if (!end_tap && mid_tap) {
            if (end_tap_count < 3*range) {
                end_tap_count++;
                if (abs(passZ[1]) <= abs(low_threshold)) {
                    end_tap = true;
                }
            } else {
                reset_tap();
            }
        }

        if (!mid_tap && begin_tap) {
            if (mid_tap_count < range) {
                mid_tap_count++;
                if ((neg && passZ[1] >= high_threshold) || (!neg && passZ[1] <= -1*high_threshold)) {
                    mid_tap = true;
                }
            } else {
                reset_tap();
            }
        }

        if (
            (abs(passZ[0]) <= low_threshold) &&
            ((abs(X[0]) <= x_threshold &&
                Y[0] <= y_threshold)) &&
            ((abs(passX[0]) <= threshold_other_axis &&
                abs(passY[0]) <= threshold_other_axis)) &&
            abs(passZ[1]) >= high_threshold &&
            !begin_tap
        ) {
            begin_tap = true;
            neg = passZ[1] < 0;
        }

        if (begin_tap && mid_tap && end_tap) {
            show_tap_mode = true;
            reset_tap();
            redraw_screen(watchface_ref);
        }
        passX[0] = passX[1];
        passY[0] = passY[1];
        passZ[0] = passZ[1];
        Z[0] = Z[1];
        X[0] = X[1];
        Y[0] = Y[1];
    }

    lastPassX = passX[1];
    lastPassY = passY[1];
    lastPassZ = passZ[1];
    lastX = X[1];
    lastY = Y[1];
    lastZ = Z[1];
}


bool tap_mode_visible() {
    return show_tap_mode && !show_wrist_mode;
}

bool wrist_mode_visible() {
    return show_wrist_mode && !show_tap_mode;
}

void reset_wrist_handler() {
    show_wrist_mode = false;
    redraw_screen(watchface_ref);
}

void shake_data_handler(AccelAxisType axis, int32_t direction) {
    if (show_wrist_mode) {
        return;
    }

    if (axis == ACCEL_AXIS_Y) {
        show_wrist_mode = true;
        redraw_screen(watchface_ref);
    }
}

void init_accel_service(Window * watchface) {
    timeout_sec = persist_exists(KEY_TAPTIME) ? persist_read_int(KEY_TAPTIME) : 7;
    watchface_ref = watchface;

    #if !defined PBL_PLATFORM_APLITE
    accel_data_service_unsubscribe();
    if (is_tap_enabled()) {
        accel_data_service_subscribe(25, accel_data_handler);
    }
    #endif

    accel_tap_service_unsubscribe();
    if (is_wrist_enabled()) {
        accel_tap_service_subscribe(shake_data_handler);
    }
}
#else

bool tap_mode_visible() {
    return false;
}

bool wrist_mode_visible() {
    return false;
}

#endif
