#include "bbrot.h"
#include "cbqueue.h"
#include <iostream>
#include <stdlib.h>
#include <random>
#include <thread>
#include <vector>
#include <getopt.h>

using namespace std;

const double real_start_range = -2;
const double real_end_range = 1;
const double imag_start_range = -1.5;
const double imag_end_range = 1.5;

int main(int argc, char **argv) {
  int image_size = 800;
  int num_points = 1'000'000;
  int max_iters = 1'000;
  int num_threads = std::thread::hardware_concurrency() ?
    std::thread::hardware_concurrency() : 1;
  
  vector<std::thread> vec_producer_threads;
  int ch;

  // options descriptor
  struct option longopts[] = {
    { "size",    required_argument, NULL, 's' },
    { "points",  required_argument, NULL, 'p' },
    { "iters",   required_argument, NULL, 'i' },
    { "threads", required_argument, NULL, 't' },
    { NULL,      0,                 NULL, 0 }
  };

  /* parse options */
  while ((ch = getopt_long(argc, argv, "s:p:i:t:", longopts, NULL)) != -1) {
    switch (ch) {
    case 's':
      if (is_valid_integer(optarg)) {
        image_size = atoi(optarg);
      } else {
        usage(argv[0], "Error: <image_size> must be a valid integer.");
        return 1;
      }
      break;

    case 'p':
      if (is_valid_integer(optarg)) {
        num_points = atoi(optarg);
      } else {
        usage(argv[0], "Error: <num_points> must be a valid integer.");
        return 1;
      }
      break;
      
    case 'i':
      if (is_valid_integer(optarg)) {
        max_iters = atoi(optarg);
      } else {
        usage(argv[0], "Error: <max_iters> must be a valid integer.");
        return 1;
      }
      break;

    case 't':
      if (is_valid_integer(optarg)) {
        num_threads = atoi(optarg);
      } else {
        usage(argv[0], "Error: <num_threads> must be a valid integer.");
        return 1;
      }
      break;

    default:
      usage(argv[0]);
      return 1;
    }
  }

  /* Print configuration */
  cerr << "Image Size: " << image_size << endl;
  cerr << "# of Points: " << num_points << endl;
  cerr << "Max Iterations: " << max_iters << endl;
  cerr << "# of Producer Threads: " << num_threads << endl;

  /* Init concurrent bounded queue with some upper bound (e.g., 100 items) */
  ConcurrentBoundedQueue<SP_MandelbrotPointInfo> q(100);

  /* Init image */
  Image img(image_size, image_size);
  
  /* Start up the producer threads */
  int points_per_producer =
    static_cast<int>(num_points / num_threads);
  bool points_evenly_distributed = num_points % num_threads == 0;
  
  for (int t = 0; t < num_threads; t++) {
    /* If points cannot be evenly distributed, then the last thread will have to
       work on the extra points */
    if (!points_evenly_distributed && t == num_threads - 1) {
      points_per_producer += num_points % num_threads;
    }

    vec_producer_threads.push_back(std::thread {generate_bbrot_trajectories,
                                                points_per_producer, max_iters,
                                                std::ref(q)});
  }

  /* Consume results from the concurrent bounded queue, until all producers are
     finished. The results are used to update the image contents */
  int items_left = num_threads;
  
  while (items_left > 0) {
    SP_MandelbrotPointInfo mpi = q.get();
    
    if (mpi == nullptr) {
      items_left--;
    } else {
      update_image(img, mpi);
    }
  }

  /* When all producers are finished, join all threads */
  for (std::thread &pt : vec_producer_threads) {
    pt.join();
  }

  cerr << endl; // newline after progress bar

  /* Output the image to standard output */
  output_image_to_pgm(img, cout);

  return 0;
}

void generate_bbrot_trajectories(int num_points, int max_iters,
                                 ConcurrentBoundedQueue<SP_MandelbrotPointInfo>
                                 &queue) {
  std::random_device random_device;
  std::default_random_engine random_engine(random_device());
  std::uniform_real_distribution<double>
    real_rand(real_start_range, real_end_range);
  std::uniform_real_distribution<double>
    imag_rand(imag_start_range, imag_end_range);

  int progress_bar_period = 100'000;
  int progress_bar_count = 0;

  for (int p = 0; p < num_points; p++) {
    d_complex c(real_rand(random_engine), imag_rand(random_engine));
    SP_MandelbrotPointInfo mpi = compute_mandelbrot(c, max_iters, true);
    
    if (mpi->escaped) {
      queue.put(mpi);
    }

    // display progress bar
    ++progress_bar_count;
    if (progress_bar_count % progress_bar_period == 0)
      cerr << ".";
  }

  /* Add nullptr value to tell consumer that this producer is done
     i.e., shared_ptr's default constructor creats a nullptr value */
  queue.put(SP_MandelbrotPointInfo());
}

double normalize(double min, double max, double value) {
  assert(max > min);

  if (value == min) return 0.0;
  if (value == max) return 1.0;
  
  return (value - min) / (max - min);
}

void update_image(Image &image, const SP_MandelbrotPointInfo &info) {
  for (d_complex p : info->points_in_path) {
    if (p.real() < min(real_start_range, real_end_range)
        || p.real() > max(real_start_range, real_end_range)
        || p.imag() < min (imag_start_range, imag_end_range)
        || p.imag() > max(imag_start_range, imag_end_range)) {
      continue;
    }

    int x = normalize(real_start_range, real_end_range, p.real())
      * (image.getWidth() - 1);
    int y = normalize(imag_start_range, imag_end_range, p.imag())
      * (image.getHeight() - 1);

    image.incValue(x, y);
    image.incValue(x, image.getWidth() - y - 1);
  }
}

void output_image_to_pgm(const Image &image, ostream &os) {
  const int max_value = 255;
  int max_pixel = getMaxPixel(image);
  
  os << "P2 " << image.getWidth() << " " << image.getHeight()
     << " " << max_value << "\n";

  for (int y = 0; y < image.getWidth(); y++) {
    for (int x = 0; x < image.getHeight(); x++) {
      double normalized_value = normalize(0, max_pixel, image.getValue(x, y));
      normalized_value *= max_value;
      
      os << (int)normalized_value << " ";
    }
    os << "\n";
  }
}

int getMaxPixel(const Image &image) {
  int max_pixel = 0;
  
  for (int x = 0; x < image.getWidth(); x++) {
    for (int y = 0; y < image.getHeight(); y++) {
      max_pixel = max(image.getValue(x, y), max_pixel);
    }
  }

  return max_pixel;
}

void usage(string program_name, string optional_msg) {
  if (optional_msg != "")
    cerr << optional_msg << endl;
  
  cerr << endl;
  cerr << "usage: " << program_name << " [options]" << endl;
  cerr << "Generates a Buddhabrot image. Takes four optional arguments: " << endl;
  cerr << "size, points, iters, and threads with some reasonable defaults.";
  cerr << endl << endl;

  cerr << "-s|--size <image_size> sets the image size in pixels." << endl << endl;
  
  cerr << "-p|--points <num_points> sets the total number of random starting "
       << "points to generate." << endl << endl;

  cerr << "-i|--iters <max_iters> sets the maximum iteration limit for testing "
       << "a point for membership in the Mandelbrot set." << endl << endl;

  cerr << "-t|--threads <num_threads> sets the number of producer threads "
       << "to run." << endl;
}

bool is_valid_integer(const char *str) {
  char *str_end;

  /* str_end will be updated to point to the location in the string where
     the parsing function stops parsing.  If str_end ends up pointing to
     the NULL character, the entire string was parsed. */
  int val = strtol(str, &str_end, /* base */ 10);
  return val && *str_end == 0;
}
