#pragma once

#include <vector>

class ColorConversion {
private:
    std::vector<std::vector<unsigned char>> yuvRed;
    std::vector<std::vector<std::vector<unsigned char>>> yuvGreen;
    std::vector<std::vector<unsigned char>> yuvBlue;

public:
    ColorConversion();
    void setup();
    void cby0cry1_to_y(unsigned char* cby0cry1, unsigned char* y, unsigned int n);
    void cby0cry1_to_rgb(unsigned char* cby0cry1, unsigned char* rgb, unsigned int n);
};
