#include "SpaceGame.h"

void PlayerShip::update(vec2 mousePos)
{
    vec2 mouseDif = mousePos - vec2(0.5);
    double spdFact = abs(glm::min(glm::length(mouseDif * vec2(2)), 1.0f));
    static double smoothSpdFact = 0.0;
    smoothSpdFact += (spdFact - smoothSpdFact) * 0.5;
    double shipDir = atan2(-mouseDif.y, mouseDif.x);
    static double smoothDir = 0.0;
    smoothDir += (shipDir - smoothDir) * 0.07;
    angle = smoothDir;
    vec3 spd = vec3(0.2);
    static vec3 shipPos = vec3(0.0);
    pos += vec3(cos(smoothDir), 0, -sin(smoothDir)) * spd * vec3(smoothSpdFact);
}

void Planet::update(double time)
{
}
