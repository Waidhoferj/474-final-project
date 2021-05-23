#pragma once

#ifndef SPACE_GAME
#define SPACE_GAME

#include <string>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include "GLSL.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Shape.h"
#include "line.h"

using namespace glm;
using namespace std;

class Planet
{
public:
    vec3 pos;
    quat rot_axis;
    double rotation;
    double scale;
    shared_ptr<Shape> shape;
    void update(double time);
};

class Asteroid
{
public:
    vec3 pos;
    quat rot_axis;
    double rotation;
    double scale;
    vector<vec3> path;
    shared_ptr<Shape> shape;
};

class PlayerShip
{
public:
    vec3 pos;
    double angle;
    void update(vec2 mousePos);
};

#endif // LAB471_SHAPE_H_INCLUDED
