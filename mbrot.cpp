#include "mbrot.h"

SP_MandelbrotPointInfo compute_mandelbrot(d_complex c, int max_iters,
                                       bool collect_points) {
  SP_MandelbrotPointInfo mpi = std::make_shared<MandelbrotPointInfo>();
  d_complex z(0, 0); // i=0 case
  int i;

  for (i = 1; i <= max_iters; i++) {
    z = (z * z) + c;

    if (collect_points)
      mpi->points_in_path.push_back(z);

    if (abs(z) > 2) {
      mpi->escaped = true;
      break;
    }
  }

  mpi->initial_point = c;
  mpi->num_iters = std::max(i, max_iters);
  mpi->max_iters = max_iters;

  return mpi;
}
