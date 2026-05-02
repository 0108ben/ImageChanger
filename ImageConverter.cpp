//
// Created by benben on 02/05/2026.
//

#include "ImageConverter.hpp"

#include <algorithm>

#include "main.hpp"

void ImageData::convertImageToHex(const string& imageLocation)
{
    ifstream imageOne(imageLocation, std::ios::binary);

    std::vector<unsigned char> imageBytes((std::istreambuf_iterator<char>(imageOne)), std::istreambuf_iterator<char>());

    imageOne.close();

    std::stringstream ss;
    for (auto byte : imageBytes)
    {
        ss << hex << std::setw(2) << std::setfill('0') << (int)byte << " ";
    }

    hexString = ss.str();
}

void ImageData::getHexChunk(const int length, string &returnHex)
{
    // If len is 1 -> return 2 chars 00, FF, etc.
    // Each hex value has a white space inbetween -> 00 11 -> return 5 chars if len is 2, 8 chars if len is 3

    const int characters = length * 2 + (length - 1);

    returnHex = hexString.substr(currentPos, characters);

    currentPos += characters + 1;
}

int ImageData::getHexValue(const string &hexChunk)
{
    auto hexVal = hexChunk;
    std::erase_if(hexVal, isspace);

    return std::stoi(hexVal, nullptr, 16);
}
