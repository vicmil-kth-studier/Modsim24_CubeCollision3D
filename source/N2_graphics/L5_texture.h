#include "L4_setup.h"

struct RGBPixel {
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
    RGBPixel() {};
    RGBPixel(unsigned char r_, unsigned char g_, unsigned char b_) {
        r = r_;
        g = g_;
        b = b_;
    }
};

class RawImage {
public:
    unsigned int width = 0;
    unsigned int height = 0;
    std::vector<RGBPixel> pixels = std::vector<RGBPixel>();

    RawImage() {}
    RawImage(std::vector<RGBPixel> pixels_, unsigned int width_, unsigned int height_) {
        width = width_;
        height = height_;
        pixels = pixels_;
        Assert(width*height == pixels.size());

        // Make sure width and height is a power of 2(otherwise it won't work)
        Assert(is_power_of_two(width));
        Assert(is_power_of_two(height));
    }
    unsigned char* get_pixel_data() {
        return (unsigned char*)((void*)(&pixels[0]));
    }
};

class Texture {
public:
    unsigned int renderedTexture;

    static Texture from_raw_image(RawImage& raw_image) {
        Texture new_texture;
        glGenTextures(1, &new_texture.renderedTexture);

        // "Bind" the newly created texture : all future texture functions will modify this texture
        glBindTexture(GL_TEXTURE_2D, new_texture.renderedTexture);

        unsigned char* pixel_data = raw_image.get_pixel_data();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, raw_image.width, raw_image.height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixel_data);

        set_pixel_parameters_nearest(); // Set as default(if none set the image doesn't show)
        return new_texture;
    }
    static void set_pixel_parameters_nearest() {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    static void set_pixel_parameters_linear() {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
};

