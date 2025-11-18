#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <cassert>


using std::vector;


/*!
 * A simple class for representing 2D grayscale images.  A std::vector is used
 * for storing pixels, and 2D (x, y) accesses are mapped to a 1D row-major index
 * for each pixel's value.  The use of a std::vector means we don't have to do
 * any manual memory management, and we don't have to implement copy
 * constructors, destructors, assignment operators, move operators, etc. - we
 * get all those features automatically.
 */
class Image {
    //! The actual data for each pixel in the image.
    vector<int> data;

    //! The width of the image.
    int width;

    //! The height of the image.
    int height;

    /*!
     * This helper function maps 2D (x, y) coordinates to a 1D index.  It also
     * verifies that coordinate values are within proper ranges with assertions,
     * so that all the functions that use this helper don't have to do these
     * checks themselves.
     */
    int indexOf(int x, int y) const {
        assert(x >= 0);
        assert(x < width);
        assert(y >= 0);
        assert(y < height);

        return y * width + x;
    }

public:
    //! Initialize an image of the specified dimensions.
    Image(int width, int height) {
        assert(width > 0);
        assert(height > 0);

        this->width = width;
        this->height = height;

        data.resize(width * height);
    }

    //! Returns the width of the image.
    int getWidth() const {
        return width;
    }

    //! Returns the height of the image.
    int getHeight() const {
        return height;
    }

    //! Returns the value of the pixel at (x, y).
    int getValue(int x, int y) const {
        return data[indexOf(x, y)];
    }

    //! Sets the value of the pixel at (x, y).
    void setValue(int x, int y, int value) {
        data[indexOf(x, y)] = value;
    }


    //! Increments the value of the pixel at (x, y).
    void incValue(int x, int y) {
        data[indexOf(x, y)]++;
    }


    //! Decrements the value of the pixel at (x, y).
    void decValue(int x, int y) {
        data[indexOf(x, y)]--;
    }
};

#endif // IMAGE_H
