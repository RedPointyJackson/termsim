#include <stdio.h>
#include <stdlib.h>
#include <math.h>


double *x, *y, *z, *vx, *vy, *vz;

const int N = 1000;
const double G = 0.0001;
const double dt = 0.01;
const double epsilon = 1e-5; // Soften r=0 singularity

void evolve_bruteforce(double dt) {
    // Compute forces and integrate the system with an Euler
    // integrator.
    int i, j;
    for (i = 0; i < N; i++) {
        double totalforce[3] = {0, 0, 0};
        double dist[3];
        for (j = 0; j < N; j++) {
            if (i != j) {
                dist[0] = (x[j] - x[i]);
                dist[1] = (y[j] - y[i]);
                dist[2] = (z[j] - z[i]);
                double d3 = sqrt(dist[0]*dist[0] + dist[1]*dist[1] + dist[2]*dist[2]);
                d3 = d3*d3*d3 + epsilon;
                totalforce[0] += G*dist[0]/d3;
                totalforce[1] += G*dist[1]/d3;
                totalforce[2] += G*dist[2]/d3;
            }
        }
        vx[i] += totalforce[0] * dt;
        vy[i] += totalforce[1] * dt;
        vz[i] += totalforce[2] * dt;
    }
    for (i = 0; i < N; i++) {
        x[i] += vx[i] * dt;
        y[i] += vy[i] * dt;
        z[i] += vz[i] * dt;
    }
}

void print_stars() {
    printf("DATA\n");
    for (int i = 0; i < N; i++) {
        printf("%lf %lf %lf\n", x[i], y[i], z[i]);
    }
    printf("END\n");
}

int main(int argc, char *argv[]) {
    srand(42);

    printf("Info: Allocating stars...\n");
    // Malloc all the arrays needed
    x = malloc(sizeof(double) * N);
    y = malloc(sizeof(double) * N);
    z = malloc(sizeof(double) * N);
    vx = malloc(sizeof(double) * N);
    vy = malloc(sizeof(double) * N);
    vz = malloc(sizeof(double) * N);

    for(int i=0;i<N;i++){
        x[i] = rand()/(double)RAND_MAX - 0.5;
        y[i] = rand()/(double)RAND_MAX - 0.5;
        z[i] = rand()/(double)RAND_MAX - 0.5;
        vx[i] = 0;
        vy[i] = 0;
        vz[i] = 0;
    }

    printf("Warning: This visualization software may be too cool.\n");

    const int iters = 200;
    for(int i=0;i<iters;i++){
        print_stars();
        evolve_bruteforce(dt);
	printf("%d/%d (%.2lf%%) iterations done\r", i+1, iters, i*100.0/(iters-1));
    }

    printf("Info: All iterations done.\n");
}
