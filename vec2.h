#pragma once
#include <math.h>

typedef struct
{
    float x;
    float y;
} Vec2;

static inline float vec2_len(Vec2 *self)
{
    return sqrtf(self->x * self->x + self->y * self->y);
}
static inline void vec2_addi(Vec2 *self, Vec2 *other)
{
    self->x += other->x;
    self->y += other->y;
}
static inline void vec2_subi(Vec2 *self, Vec2 *other)
{
    self->x -= other->x;
    self->y -= other->y;
}
static inline void vec2_roti(Vec2 *self, float angle)
{
    float x = self->x * cosf(angle) - self->y * sinf(angle);
    float y = self->x * sinf(angle) + self->y * cosf(angle);
    self->x = x;
    self->y = y;
}
static inline void vec2_normi(Vec2 *self)
{
    float l = vec2_len(self);
    self->x /= l;
    self->y /= l;
}
static inline void vec2_scalei(Vec2 *self, float s)
{
    self->x *= s;
    self->y *= s;
}
static inline void vec2_rotoi(Vec2 *self, Vec2 *o, float angle)
{
    vec2_subi(self, o);
    vec2_roti(self, angle);
    vec2_addi(self, o);
}
static inline Vec2 vec2_copy(Vec2 v)
{
    return v;
}
static inline Vec2 vec2_rot(Vec2 v, float angle)
{
    return (Vec2){
        .x = v.x * cosf(angle) - v.y * sinf(angle),
        .y = v.x * sinf(angle) + v.y * cosf(angle),
    };
}
static inline Vec2 vec2_roto(Vec2 v, Vec2 *o, float angle)
{
    Vec2 result = vec2_copy(v);
    vec2_subi(&result, o);
    vec2_roti(&result, angle);
    vec2_addi(&result, o);
    return result;
}
static inline Vec2 vec2_add(Vec2 v, Vec2 *other)
{
    return (Vec2){v.x + other->x, v.y + other->y};
}
static inline Vec2 vec2_sub(Vec2 v, Vec2 *other)
{
    return (Vec2){v.x - other->x, v.y - other->y};
}
static inline Vec2 vec2_norm(Vec2 v)
{
    float l = vec2_len(&v);
    return (Vec2){v.x / l, v.y / l};
}
static inline Vec2 vec2_scale(Vec2 v, float s)
{
    return (Vec2){v.x * s, v.y * s};
}

static inline Vec2 vec2_sign(Vec2 v)
{
    return (Vec2){
        copysignf(1.0f, v.x),
        copysignf(1.0f, v.y),
    };
}
