#pragma once

#include "ImageConverter.hpp"

using std::ifstream;
using std::fstream;
using std::hex;
using std::string;
using std::cout;
using std::endl;

int main();

inline const string imageLocationOne = "Images/image1.png";

inline const string pngSignature = "89 50 4e 47 0d 0a 1a 0a";

inline const string hexIHDR = "49 48 44 52";