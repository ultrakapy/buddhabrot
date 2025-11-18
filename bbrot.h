#ifndef BBROT_H
#define BBROT_H

#include "image.h"
#include "mbrot.h"
#include "cbqueue.h"
#include <ostream>

using std::ostream;

double normalize(double min, double max, double value);
void update_image(Image &image, const SP_MandelbrotPointInfo &info);
void output_image_to_pgm(const Image &image, ostream &os);

int getMaxPixel(const Image &image);

void generate_bbrot_trajectories(int num_points, int max_iters,
                                 ConcurrentBoundedQueue<SP_MandelbrotPointInfo>
                                 &queue);

void usage(std::string program_name, std::string optional_msg = "");

bool is_valid_integer(const char *str);

#endif // BBROT_H
