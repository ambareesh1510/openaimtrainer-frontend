#ifndef TARGET_H
#define TARGET_H

#include <raylib.h>

struct Sphere {
    float radius;
};
typedef struct Sphere Sphere;

enum TargetType {
    SPHERE,
};
typedef enum TargetType TargetType;

struct Target {
    TargetType type;
    Vector3 position;
    union {
        Sphere sphere;
    } data;
};
typedef struct Target Target;

#endif /* TARGET_H */
