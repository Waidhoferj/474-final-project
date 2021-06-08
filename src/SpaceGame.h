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
#include "Program.h"
#include <iostream>

using namespace glm;
using namespace std;

class Planet
{
public:
    Planet(vec3 position);
    Planet();
    static Shape shape;
    vec3 pos;
    quat rot_axis;
    double scale;
    mat4 rotMat;
    void update(double time);
};

class Asteroid
{
public:
    Asteroid();
    vec3 pos;
    quat rot_axis;
    double rotScalar;
    double speedScalar;
    double scale;
    vector<vec3> path;
    static Shape shape;

    void setPath(vector<vec3> points);
    void update(double time);
};

enum ShipState
{
    Flying,
    Delivering,
    Exploding
};

class PlayerShip
{
public:
    static Shape shape;
    ShipState state = Flying;
    static vector<vec3> bounds;
    vec3 pos;
    double scale = 0.5;
    double angle;
    int points;
    mat4 rotMat;
    Planet *destination;
    double time_left = 30.0;
    void update(vec2 mousePos, double dt);
    void updatePos(vec2 mousePos);
    bool intersects(Asteroid &asteroid);
    bool intersects(Planet &planet);
    void chooseNewDestination(vector<Planet> &planets);
    void respawn();
};

#endif // LAB471_SHAPE_H_INCLUDED
