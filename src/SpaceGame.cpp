#include "SpaceGame.h"

void PlayerShip::update(vec2 mousePos, double dt)
{
    if (state == Delivering)
        updatePos(mousePos);
    time_left -= dt;
    // if (time_left < 0.0)
    // {
    //     points = 0;
    //     pos = vec3(0);
    // }
}

void PlayerShip::updatePos(vec2 mousePos)
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

bool PlayerShip::intersects(Asteroid &asteroid)
{
    return glm::length(glm::distance(asteroid.pos, pos)) <= asteroid.scale + scale;
}
bool PlayerShip::intersects(Planet &planet)
{
    return glm::length(glm::distance(planet.pos, pos)) <= planet.scale + scale;
}

void PlayerShip::respawn()
{
    points = 0;
    pos = vec3(0);
    state = Delivering;
}

Planet::Planet()
{
    float rand_axis = (float)rand() / RAND_MAX * (float)acos(0) / 4.0f;
    rot_axis = quat(glm::rotate(mat4(1.0), rand_axis, vec3(0, 0, 1)));
    scale = (double)rand() / RAND_MAX * 1.5 + 0.5;
}

Planet::Planet(vec3 position)
{
    float rand_axis = (float)rand() / RAND_MAX * (float)acos(0) / 4.0f;
    rot_axis = quat(glm::rotate(mat4(1.0), rand_axis, vec3(0, 0, 1)));
    scale = (double)rand() / RAND_MAX * 1.5 + 0.5;
    pos = position;
}
void Planet::update(double time)
{
    quat spin = quat(glm::rotate(mat4(1.0), (float)time, vec3(0, 1, 0)));
    rotMat = mat4(spin * rot_axis);
}

void Asteroid::setPath(vector<vec3> points)
{
    spline(path, points, 200, 2);
}

Asteroid::Asteroid()
{
    float rand_axis = (float)rand() / RAND_MAX * (float)acos(0) / 4.0f;
    rotScalar = (float)rand() / RAND_MAX * 2.0;
    speedScalar = (float)rand() / RAND_MAX + 0.1;
    rot_axis = quat(glm::rotate(mat4(1.0), rand_axis, vec3(0, 0, 1)));
    scale = (double)rand() / RAND_MAX * 0.1 + 0.2;
}

void Asteroid::update(double time)
{
    time = time * speedScalar;
    double anim_time = time * path.size() / 10.0;
    int anim_step = (int)anim_time;
    double interp_t = anim_time - (double)anim_step;

    // Interpolate along path
    int point_index = anim_step % (path.size() - 1);

    vec3 cur_point = path[point_index];
    vec3 next_point = path[point_index + 1];
    pos = cur_point * glm::vec3(1.0 - interp_t) + next_point * glm::vec3(interp_t);
}
