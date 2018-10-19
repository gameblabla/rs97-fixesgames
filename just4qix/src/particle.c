#include <SDL/SDL.h>
#include <math.h>
#include <stdlib.h>


#define PI 3.1415926536
#define MAX_PARTICLES 10000
#define Uint8 char unsigned
#define time_scale 10
#define Uint16 short unsigned
#define screen_width 	320
#define screen_height 	240

typedef struct {
	float x;
        float  y;
        int angle;
	double energy; 
        unsigned char r;
        unsigned char g;
        unsigned char b;
} particle_t, *particle_p;


particle_t particles[MAX_PARTICLES];
int active_particles = 0;

static void AddParticle(particle_p particle);

static void DeleteParticle(int index);




static void AddParticle(particle_p particle)
{
    /* If there are already too many particles, forget it. */
    if (active_particles >= MAX_PARTICLES)
        return;
    particles[active_particles] = *particle;
    active_particles++;
}
/* Removes a particle from the system (by index). */
static void DeleteParticle(int index)
{
    /* Replace the particle with the one at the end
       of the list, and shorten the list. */
    particles[index] = particles[active_particles - 1];
    active_particles--;
}
/* Draws all active particles on the screen. */
void DrawParticles(SDL_Surface* dest, int camera_x, int camera_y)
{
    int i;
    Uint16 *pixels;
    /* Lock the target surface for direct access. */
    if (SDL_LockSurface(dest) != 0)
        return;
    pixels = (Uint16 *) dest->pixels;
    for (i = 0; i < active_particles; i++) {
        int x, y;
        Uint16 color;
        /* Convert world coords to screen coords. */
        x = particles[i].x - camera_x;
        y = particles[i].y - camera_y;
        if ((x < 0) || (x >= screen_width))
            continue;
        if ((y < 0) || (y >= screen_height))
            continue;
        /* Find the color of this particle. */
	color = SDL_MapRGB(dest->format, particles[i].r,  particles[i].g, particles[i].b);
        
        /* Draw the particle. */
        pixels[(dest->pitch / 2) * y + x] = color;
    }
    /* Release the screen. */
    SDL_UnlockSurface(dest);
}
/* Updates the position of each particle. Kills particles with
   zero energy. */
void UpdateParticles(void)
{
    int i;
    for (i = 0; i < active_particles; i++) {
        particles[i].x += particles[i].energy *
            cos(particles[i].angle * (PI / 180.0)) * time_scale;
        particles[i].y += particles[i].energy *
            -sin(particles[i].angle * (PI / 180.0)) * time_scale;
        /* Fade the particle's color. */
        if (particles[i].r>0) { particles[i].r--; }
        if (particles[i].g>0) {particles[i].g--;}
        if (particles[i].b>0) {particles[i].b--;}
        
        /* If the particle has faded to black, delete it. */
        if ((particles[i].r + particles[i].g +
             particles[i].b) == 0 | particles[i].x<0 | particles[i].y<0 | particles[i].x>=screen_width | particles[i].y>=screen_height) {
            DeleteParticle(i);
            /* DeleteParticle replaces the current particle with
               the one at the end of the list, so we'll need to
               take a step back. */
            i--;
        }
    }
    //printf("%d\n",active_particles);
}
/* Creates a particle explosion of the given relative
   size and position. */
void CreateParticleExplosion(int x, int y, int r, int g, int b,
                             int energy, int density)
{
    int i;
    particle_t particle;
    /* Create a number of particles proportional to the size
       of the explosion. */
    for (i = 0; i < density; i++) {
        particle.x = x;
        particle.y = y;
        particle.angle = rand() % 360;
        particle.energy = (double)(rand() % (energy * 1000)) /
                                  1000.0;
        /* Set the particle's color. */
        particle.r = r;
        particle.g = g;
        particle.b = b;
        /* Add the particle to the particle system. */
        AddParticle(&particle);
    }
}
/* This is directly from another code listing. It creates a
   16-bit pixel. */
