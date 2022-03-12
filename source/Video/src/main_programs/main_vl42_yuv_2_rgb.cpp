//
// This is the C++ main fronting for the C main (called here v4l2_raw_capture_main()).
//

/////////////////////////////////////////////////////////////////////////////////
// MIT License
//
// Copyright (c) 2022 Andrew Kelly
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
/////////////////////////////////////////////////////////////////////////////////

class RGB
{
public:
    unsigned char R;
    unsigned char G;
    unsigned char B;

    RGB(unsigned char r, unsigned char g, unsigned char b)
    {
        R = r;
        G = g;
        B = b;
    }

    bool Equals(RGB rgb)
    {
        return (R == rgb.R) && (G == rgb.G) && (B == rgb.B);
    }
};

class YUV
{
public:
    double Y;
    double U;
    double V;

    YUV(double y, double u, double v)
    {
        Y = y;
        U = u;
        V = v;
    }

    bool Equals(YUV yuv)
    {
        return (Y == yuv.Y) && (U == yuv.U) && (V == yuv.V);
    }
};

static RGB YUVToRGB(YUV yuv) {
    unsigned char r = (unsigned char)(yuv.Y + 1.4075 * (yuv.V - 128));
    unsigned char g = (unsigned char)(yuv.Y - 0.3455 * (yuv.U - 128) - (0.7169 * (yuv.V - 128)));
    unsigned char b = (unsigned char)(yuv.Y + 1.7790 * (yuv.U - 128));

    return RGB(r, g, b);
}

#include <v4l2_raw_capture.h>

int main(int argc, char *argv[])
{
    return v4l2_raw_capture_main(argc, argv);
}

