#include <iostream>
#include <cstdio>
#include <chrono>
#include "construct/Construct.h"
using namespace Construct;
using namespace std;

// Output a _very_ crude PPM down the middle slice of the volume
void render_ppm(const char *path, ScalarField field, VectorField color, Domain domain) {
	FILE *f = fopen(path, "wb");
	int W = 512;//domain.res[0] * 2;
	int H = 512;//domain.res[2] * 2;

	fprintf(f,"P6\n%d %d\n255\n",W,H);
	//unsigned char *pix = new unsigned char[W*H*3];
	for(int y=H-1;y>=0;--y) {
		for(int x=0;x<W;++x) {
		
			Vec3 C(0,0,0);
#if 1
			float T=1;
			const float ds = domain.H[2] * .5f;
			for(float z=domain.bmin[2];z<=domain.bmax[2];z+=ds) {
				Vec3 X;
				X[0] = domain.bmin[0] + domain.extent[0] * (float)x / (float)(W-1);
				X[1] = domain.bmin[1] + domain.extent[1] * (float)y / (float)(H-1);
				X[2] = z;
				float rho = field.eval(X);
				if(rho <= 0) continue;
				Vec3 col = color.eval(X);

				const float dT = expf(rho * -ds);
				T *= dT;
				C += (1.f-dT) * T * col;
			}
#else
			Vec3 X;
			X[0] = domain.bmin[0] + domain.extent[0] * (float)x / (float)(W-1);
			X[1] = domain.bmin[1] + domain.extent[1] * (float)y / (float)(H-1);
			X[2] = (domain.bmin[2] + domain.bmax[2]) * .5f;
			C = field.eval(X);
#endif
			// Gamma adjust + convert to the [0,255] range
			const float gamma = 1.f / 2.f;
			for(int i=0;i<3;++i)
				C[i] = C[i] > 1.f ? 255 : powf(C[i], gamma) * 255;
			fprintf(f, "%c%c%c", (unsigned char)C[0], (unsigned char)C[1], (unsigned char)C[2]);
		}
	}

	fclose(f);
}

// Semi-lagrangian advection by warping space along 
// characteristic lines in the flow field
template<typename T>
Field<T> sla(Field<T> f, VectorField u, ScalarField dt) {
	return warp(f, identity() - u * dt);
}

// Create a field which is equal to the signed 
// disance from the surface of a sphere
// f(x) = r - |x - C|
// where r=radius, C=center of the sphere
ScalarField sphere(Vec3 center, float radius)
{ return constant(radius) - length(identity() - constant(center)); }

// Vorticity Confinement
VectorField VorticityConfinement(VectorField velocity, float epsilon, Domain domain) {
  auto C = writeToGrid(curl(velocity), constant(Vec3(0,0,0)), domain);
  auto eta = grad(length(C));
  auto N = eta / (constant(.000001f) + length(eta));
  return constant(domain.H[0] * epsilon) * cross(N,C);
}

VectorField boundary_neumann(VectorField u, ScalarField b) {
	VectorField N = constant(-1.f) * grad(b);
	return u - 2.f * mask(b) * mask(constant(-1.f) * dot(u,N)) * dot(u,N) / dot(N,N) * N;
}

//
int main(int argc, char **argv) {
	const unsigned int R = 256; // Resolution
	const int sub = 1;
  Domain domain(R, R, R/sub, Vec3(-1,-1,-1./sub), Vec3(1,1,1./sub));
  Domain domain2(R, R, R/sub, Vec3(-1,-1,-1./sub), Vec3(1,1,1./sub));

	auto boundary = constant(0.f); //sphere(Vec3(0,0,0), .35f);
	auto source = mask(sphere(Vec3(0,-1,0),.2));
	auto density = constant(0.f);
	auto velocity = constant(Vec3(0,0,0));
	auto dt = constant(.04f);

	for(int iter=0; iter<1000; ++iter) {
		//////////////////////////////////////////////////////////	
		// Advect density using semi-lagrangian advection		
		density = sla(density, velocity, dt) + dt * source;
		density = writeToGrid(density, constant(0.f), domain2);

		// Advect velocity similarly
    VectorField force = constant(Vec3(0,0,0));
		force = force + VorticityConfinement(velocity, .5f, domain);
    force = force + density * constant(Vec3(0,1,0));
		velocity = sla(velocity, velocity, dt) + force * dt;
//		velocity = boundary_neumann(velocity, boundary);
		velocity = divFree(velocity, constant(0.f), domain, 100);
		//////////////////////////////////////////////////////////	

		// Output results
#if 1
		char path[256];
		sprintf(path, "frame.%04d.ppm", iter);

		ScalarField render_density = density ;//+ mask(boundary);
		VectorField render_color = density * constant(Vec3(1,1,1)) ;//+ mask(boundary) * constant(Vec3(1,0,0));
		render_ppm(path, render_density, render_color, domain2);
#endif

		cout << "Finished time step " << iter+1 << endl;
	}
	return 0;
}
