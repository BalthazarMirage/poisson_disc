/**
 * For the moment, i will try it on 2D for a fixed gap value.
 * After, i will do 2D for a changing gap value, then i will maybe try
 * to do arbitrary dimensions with boost::geometry.
 **/

#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/arithmetic/arithmetic.hpp>
#include <boost/geometry/algorithms/comparable_distance.hpp>

#include <functional>
#include <vector>
#include <random>


const double PI = 3.14159265358979323846;

using point = boost::geometry::model::d2::point_xy<double>;
using fct_spot = std::function<void(point const &)>;

double drand (double min, double max) {
	double f = (double)rand() / RAND_MAX;
    return min + f * (max - min);
}

point prand (double dmin, double dmax) {
	point res (drand(-1.0, 1.0), drand(-1.0, 1.0));
	
	double mag = drand (dmin, dmax);
	
	mag /= sqrt (res.x()*res.x() + res.y()*res.y());
	
	res.x ( res.x() * mag );
	res.y ( res.y() * mag );
	
	return res;
}

int cx (point const& p, double side) {
	return p.x() / side;
}

int cy (point const& p, double side) {
	return p.y() / side;
}


struct cell {
	cell () : empty (true) {}
	
	bool empty;
	point content;
};



const int k = 30;	// see where to precise it...

void sample (point max, double gap, fct_spot read) {
	double side = gap / sqrt (2);
	
	int cols = max.x() / side;
	int rows = max.y() / side;
	
	std::vector<cell> grid(cols*rows);
	
	std::vector<point> actifs;
	
	// INIT
	point seed (drand(0.0, max.x()), drand(0.0, max.y()));
	read (seed);
	
	grid[ cy(seed,side) * cols + cx(seed,side) ].empty = false;
	grid[ cy(seed,side) * cols + cx(seed,side) ].content = seed;
	
	actifs.push_back(seed);
	
	// LOOP
	while (! actifs.empty() ) {
		
		int idx = rand() % actifs.size();
		
		point actif = actifs[idx];
		
		for (int c = 0; c < k; c++) {
			
			point candidate = prand(gap, 2*gap);
			
			candidate.x ( candidate.x() + actif.x() );
			candidate.y ( candidate.y() + actif.y() );
			
			if (candidate.x() < 0 || candidate.x() > max.x() ||
			    candidate.y() < 0 || candidate.y() > max.y() ) continue;
			
			int col = candidate.x() / side;
			int row = candidate.y() / side;
			
			bool conflict = false;
			
			for (int i = -1; i <= 1; i++) {
				
				for (int j = -1; j <= 1; j++) {
					
					if (row+i < 0 || row+i >= rows || col+i < 0 || col+i >= cols) continue;
					
					if ( grid[ (row+i) * cols + (col+j) ].empty ) continue;
					point smp = grid[ (row+i) * cols + (col+j) ].content;
					
					if ( boost::geometry::comparable_distance(candidate, smp) < gap*gap) {
						conflict = true;
						break;
					}
				}
				if (conflict) break;
			}
			if (conflict) continue;
			
			read (candidate);
			grid[ row * cols + col ].empty = false;
			grid[ row * cols + col ].content = candidate;
			
			actifs.push_back(candidate); 
		}
		
		std::swap (actifs.at(idx), actifs.back());
		actifs.pop_back ();
	}
}



#include <SDL2/SDL.h>

const point dims (640,480);

int main (void) {
	SDL_Window * window = nullptr;
	SDL_Renderer * renderer = nullptr;
	
	SDL_CreateWindowAndRenderer (dims.x(), dims.y(), 0, &window, &renderer);
	
	SDL_SetRenderDrawColor (renderer, 255, 255, 255, 255);
	SDL_RenderClear (renderer);
	
	SDL_SetRenderDrawColor (renderer, 0, 0, 0, 255);
	
	sample (dims, 12, [renderer](point const& p) {
		SDL_Rect r; r.w = r.h = 4; r.x = p.x()-2; r.y = p.y()-2;
		SDL_RenderFillRect (renderer, &r);
	});
	
	SDL_RenderPresent (renderer);
	
	while (1) {
		SDL_Event event;
		while (SDL_PollEvent (&event)) {
			switch (event.type) {
				case SDL_QUIT : SDL_Quit (); return 0; break;
				default : break;
			}
		}
	}
	
	return 0;
	
}
