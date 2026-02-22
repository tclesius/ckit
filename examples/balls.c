#include "window.h"
#include "vec2.h"

#define GRAVITY 0.2f // m/s^2
#define TICK_INTERVAL 5000

typedef enum
{
    BALL = 0,
    WALL = 1,
} ObjectType;

typedef struct
{
    ObjectType type;
    uint32_t color;
    int width;  // border box
    int height; // border box
    Vec2 pos;
    Vec2 vel;
    Vec2 acc;
    float mass;

    float squeeze;
    int squeeze_timer;
} Object;

int object_collide(Object *self, Object *other)
{
    return (self->pos.x - self->width < other->pos.x + other->width &&
            self->pos.x + self->width > other->pos.x - other->width &&
            self->pos.y - self->height < other->pos.y + other->height &&
            self->pos.y + self->height > other->pos.y - other->height);
}

void object_reflect(Object *self, Object *other)
{
    if (!object_collide(self, other))
        return;

    Vec2 diff = vec2_sub(self->pos, &other->pos);
    Vec2 overlap = {
        (self->width + other->width) - fabsf(diff.x),
        (self->height + other->height) - fabsf(diff.y),
    };
    Vec2 dir = vec2_sign(diff);
    float factor = (other->mass == 0) ? 1.0f : 0.5f;

    if (overlap.x < overlap.y)
    {
        self->pos.x += dir.x * overlap.x * factor;
        if (dir.x * self->vel.x < 0)
        {
            self->vel.x = -self->vel.x;
        }
    }
    else
    {
        self->pos.y += dir.y * overlap.y * factor;
        if (dir.y * self->vel.y < 0)
        {
            self->vel.y = -self->vel.y;
        }
    }
    self->squeeze = 0.8f;
    self->squeeze_timer = 10;
}

typedef struct
{
    Object *objs;
    int objs_len;
} Env;

void env_tick(Env *env)
{
    // apply forces
    for (size_t i = 0; i < env->objs_len; i++)
    {
        Object *obj = &env->objs[i];
        if (obj->mass == 0)
        {
            // objects with 0 mass are immovable
            continue;
        }

        vec2_addi(&obj->vel, &obj->acc);
        vec2_addi(&obj->pos, &obj->vel);

        if (obj->squeeze_timer > 0)
        {
            obj->squeeze_timer--;
            obj->squeeze += (1.0f - obj->squeeze) * 0.2f;
        }
        else
        {
            obj->squeeze = 1.0f;
        }
    }

    // apply collisions
    for (size_t i = 0; i < env->objs_len; i++)
    {
        Object *obj = &env->objs[i];
        if (obj->mass == 0)
        {
            continue;
        }

        for (size_t j = 0; j < env->objs_len; j++)
        {
            if (i == j)
            {
                continue;
            }
            object_reflect(&env->objs[i], &env->objs[j]);
        }
    }
}

int main()
{
    Window *w = window_create(800, 600, 100, 100, "Balls");

    Object objs[] = {
        {.pos = {25, 25}, .vel = {2, 1}, .acc = {0, GRAVITY}, .width = 10, .height = 10, .mass = 1.0f, .type = BALL, .color = WINDOW_RED},
        {.pos = {50, 50}, .vel = {2, 1}, .acc = {0, GRAVITY}, .width = 20, .height = 20, .mass = 2.0f, .type = BALL, .color = WINDOW_GREEN},
        {.pos = {75, 75}, .vel = {2, 1}, .acc = {0, GRAVITY}, .width = 15, .height = 15, .mass = 1.5f, .type = BALL, .color = WINDOW_BLUE},
        {.pos = {100, 100}, .vel = {2, 1}, .acc = {0, GRAVITY}, .width = 30, .height = 30, .mass = 3.0f, .type = BALL, .color = WINDOW_RED},

        {.pos = {w->width / 2, 0}, .width = w->width / 2, .height = 20, .mass = 0, .type = WALL, .color = WINDOW_BLACK},
        {.pos = {w->width / 2, w->height}, .width = w->width / 2, .height = 20, .mass = 0, .type = WALL, .color = WINDOW_BLACK},
        {.pos = {0, w->height / 2}, .width = 20, .height = w->height / 2, .mass = 0, .type = WALL, .color = WINDOW_BLACK},
        {.pos = {w->width, w->height / 2}, .width = 20, .height = w->height / 2, .mass = 0, .type = WALL, .color = WINDOW_BLACK},
    };

    Env env = {
        .objs = objs,
        .objs_len = 8,
    };

    while (window_poll(w))
    {
        if (w->tick % TICK_INTERVAL)
        {
            continue;
        }

        env_tick(&env);

        window_rect(w, 0, 0, w->width, w->height, WINDOW_WHITE); // clear

        for (size_t i = 0; i < env.objs_len; i++)
        {
            Object obj = env.objs[i];
            switch (obj.type)
            {
            case WALL:
                window_rect(w, obj.pos.x - obj.width, obj.pos.y - obj.height,
                            obj.pos.x + obj.width, obj.pos.y + obj.height, obj.color);
                break;
            case BALL:
                window_ellipse(w,
                               obj.pos.x, obj.pos.y,
                               (int)(obj.width / obj.squeeze), // wider when squished
                               (int)(obj.width * obj.squeeze), // shorter when squished
                               obj.color);
                break;
            default:
                break;
            }
        }

        window_flush(w);
    }
    window_destroy(w);
    return 0;
}