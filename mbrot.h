#ifndef MBROT_H
#define MBROT_H

#include <vector>
#include <complex>

using std::complex;
using std::vector;

using d_complex = complex<double>;


//! Represents a point that the Mandelbrot function has been computed on.
struct MandelbrotPointInfo {
    //! The initial point being considered.
    d_complex initial_point;

    //! Whether the point escaped before the calculation completed.
    bool escaped;

    /*!
     * The number of iterations the calculation went for.  This value will be
     * no more than max_iters.
     */
    int num_iters;

    //! The maximum number of iterations specified for the calculation.
    int max_iters;

    /*!
     * All points in the path from the starting point, whether the point
     * escaped or not.
     */
    vector<d_complex> points_in_path;

    /*!
     * Initializes the MandelbrotPointInfo struct so that the primitive
     * members will have good default value, not random garbage.
     */
    MandelbrotPointInfo() : escaped{false}, num_iters{0}, max_iters{0} { };
};

using SP_MandelbrotPointInfo = std::shared_ptr<MandelbrotPointInfo>;


SP_MandelbrotPointInfo compute_mandelbrot(d_complex c, int max_iters,
                                       bool collect_points = false);

#endif // MBROT_H
