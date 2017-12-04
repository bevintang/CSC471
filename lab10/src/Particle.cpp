//
// sueda - geometry edits Z. Wood
// 3/16
//

#include <iostream>
#include "Particle.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Texture.h"


float randFloat(float l, float h)
{
	float r = rand() / (float) RAND_MAX;
	return (1.0f - r) * l + r * h;
}

void Particle::load()
{
	// Random initialization
	rebirth(0.0f);
}

// all particles born at the origin
void Particle::rebirth(float t)
{
	time = 0;

	charge = randFloat(0.0f, 1.0f) < 0.5f ? -1.0f : 1.0f;
	m = 1.0f;
	d = randFloat(0.0f, 0.02f);
	x.x = .2;
	x.y = -.20;
	x.z = randFloat(-3.f, -2.f);
	v.x = randFloat(0.2f, 1.8f);
	v.y = randFloat(0.2f, .5f);
	v.z = randFloat(-0.5f, 0.5f);
	lifespan = randFloat(0.5, 20.0);
	tEnd = t + lifespan;

	scale = randFloat(0.2f, 1.0f);
	color.r = randFloat(0.15f, 0.3f);
	color.g = randFloat(0.1f, 0.2f);
	color.b = randFloat(0.1f, 0.2f);
	color.a = 1.0f;
}

void Particle::update(float t, float h, const vec3 &g, const bool *keyToggles)
{
	time += 0.01;
	if (t > tEnd)
	{
		rebirth(t);
	}

	// update gravity vector
	float dy = v.y * time + 0.5 * -9.81 * (time * time);	// kinematic equation
	vec3 newSpeed = v;
	newSpeed.y += dy;

	// very simple update
	x += h * newSpeed;
	color.a = (tEnd - t) / lifespan;
}
