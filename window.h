#include <stdint.h>
#include <objc/runtime.h>
#include <objc/message.h>
#include <CoreGraphics/CoreGraphics.h>
#include <stdlib.h>

typedef struct
{
    const char *title;
    int width;
    int height;
    int x;
    int y;
    uint64_t tick;
    uint32_t *buffer;
    id window;
    id app;
    id pool;
} Window;

#define WINDOW_BLACK 0x00000000
#define WINDOW_WHITE 0x00FFFFFF
#define WINDOW_RED 0x00FF0000
#define WINDOW_GREEN 0x0000FF00
#define WINDOW_BLUE 0x000000FF

#define msg(ret, obj, sel) \
    ((ret (*)(id, SEL))objc_msgSend)((id)(obj), sel_registerName(sel))

#define msg1(ret, obj, sel, T1, a1) \
    ((ret (*)(id, SEL, T1))objc_msgSend)((id)(obj), sel_registerName(sel), a1)

#define msg4(ret, obj, sel, T1, a1, T2, a2, T3, a3, T4, a4) \
    ((ret (*)(id, SEL, T1, T2, T3, T4))objc_msgSend)((id)(obj), sel_registerName(sel), a1, a2, a3, a4)

#define cls(name) ((id)objc_getClass(name))

static inline Window *window_create(int width, int height, int x, int y, const char *title)
{
    Window *w = (Window *)malloc(sizeof(Window));
    w->buffer = (uint32_t *)malloc(width * height * sizeof(uint32_t));
    w->title = title;
    w->width = width;
    w->height = height;
    w->x = x;
    w->y = y;

    w->pool = msg(id, msg(id, cls("NSAutoreleasePool"), "alloc"), "init");

    w->app = msg(id, cls("NSApplication"), "sharedApplication");
    msg1(void, w->app, "setActivationPolicy:", int, 0);

    CGRect frame = {{w->x, w->y}, {w->width, w->height}};

    w->window = msg(id, cls("NSWindow"), "alloc");
    w->window = msg4(id, w->window, "initWithContentRect:styleMask:backing:defer:",
                     CGRect, frame, int, 3, int, 2, int, 0);
    msg1(void, w->window, "setTitle:", id, (id)CFStringCreateWithCString(NULL, title, kCFStringEncodingUTF8));
    msg1(void, w->window, "makeKeyAndOrderFront:", id, NULL);
    msg1(void, w->app, "activateIgnoringOtherApps:", int, 1);
    msg(void, w->app, "finishLaunching");

    id contentView = msg(id, w->window, "contentView");
    id layer = msg(id, msg(id, cls("CALayer"), "alloc"), "init");
    msg1(void, contentView, "setLayer:", id, layer);
    msg1(void, contentView, "setWantsLayer:", BOOL, YES);

    return w;
}

static inline int window_poll(Window *w)
{
    while (1)
    {
        w->tick += 1;
        id event = msg4(id, w->app, "nextEventMatchingMask:untilDate:inMode:dequeue:",
                        unsigned long, 0xFFFFFFFFFFFFFFFF,
                        id, NULL,
                        id, (id)CFSTR("kCFRunLoopDefaultMode"),
                        int, 1);
        if (!event)
        {
            break;
        }
        msg1(void, w->app, "sendEvent:", id, event);
    }
    return msg(BOOL, w->window, "isVisible");
}

static inline void window_destroy(Window *w)
{
    msg(void, w->window, "close");
    msg(void, w->pool, "drain");
    free(w->buffer);
    free(w);
}

static inline void window_flush(Window *w)
{
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate(
        w->buffer, w->width, w->height,
        8, w->width * 4, space,
        kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little);
    CGImageRef img = CGBitmapContextCreateImage(ctx);

    id contentView = msg(id, w->window, "contentView");
    id layer = msg(id, contentView, "layer");
    msg1(void, layer, "setContents:", id, (id)img);

    msg(void, cls("CATransaction"), "flush");

    CGImageRelease(img);
    CGContextRelease(ctx);
    CGColorSpaceRelease(space);
}

static inline void window_setpixel(Window *w, int x, int y, uint32_t color)
{
    if (x < 0 || y < 0 || x >= w->width || y >= w->height)
        return;
    w->buffer[y * w->width + x] = color;
}

static inline uint32_t *window_getpixel(Window *w, int x, int y)
{
    if (x < 0 || y < 0 || x >= w->width || y >= w->height)
        return NULL;
    return &w->buffer[y * w->width + x];
}

void window_line(Window *w, int x0, int y0, int x1, int y1, uint32_t color)
{
    int sy = 1, sx = 1;
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    if (x0 > x1)
        sx = -1;
    if (y0 > y1)
        sy = -1;

    if (dx == 0)
    {
        for (int i = 0; i < dy; i++)
        {
            window_setpixel(w, x0, y0 + i * sy, color);
        }
        return;
    }

    float m = (float)dy / (float)dx;

    if (m <= 1)
    {
        for (int i = 0; i <= dx; i++)
        {
            window_setpixel(w, (int)ceilf(x0 + i * sx), (int)ceilf(y0 + i * m * sy), color);
        }
    }
    else
    {
        for (int i = 0; i <= dy; i++)
        {
            window_setpixel(w, (int)ceilf(x0 + i / m * sx), (int)ceilf(y0 + i * sy), color);
        }
    }
}

void window_rect(Window *w, int x0, int y0, int x1, int y1, uint32_t color)
{
    for (int x = x0; x <= x1; x++)
    {
        for (int y = y0; y <= y1; y++)
        {
            window_setpixel(w, x, y, color);
        }
    }
}

void window_grid(Window *w, int width, int height, uint32_t color)
{
    for (size_t i = 0; i < w->width; i += width)
    {
        window_line(w, i, 0, i, w->height, color);
    }

    for (size_t i = 0; i < w->height; i += height)
    {
        window_line(w, 0, i, w->width, i, color);
    }
}

void window_circle(Window *w, int x, int y, int r, uint32_t color)
{
    for (int dy = -r; dy <= r; dy++)
    {
        for (int dx = -r; dx <= r; dx++)
        {
            if (dx * dx + dy * dy <= r * r)
            {
                window_setpixel(w, x + dx, y + dy, color);
            }
        }
    }
}

void window_ellipse(Window *w, int x, int y, int rx, int ry, uint32_t color)
{
    for (int dy = -ry; dy <= ry; dy++)
    {
        for (int dx = -rx; dx <= rx; dx++)
        {
            float nx = (float)dx / rx;
            float ny = (float)dy / ry;
            if (nx * nx + ny * ny <= 1.0f)
            {
                window_setpixel(w, x + dx, y + dy, color);
            }
        }
    }
}