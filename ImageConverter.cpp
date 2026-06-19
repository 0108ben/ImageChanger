#include "ImageConverter.hpp"

#include <algorithm>
#include <bits/local_lim.h>

const std::unordered_map<int, string> ImageData::colourType =
{
    {0, "Greyscale"},
    {2, "Truecolour"},
    {3, "Indexed-colour"},
    {4, "Greyscale with alpha"},
    {6, "Truecolour with alpha"}
};

const std::unordered_map<int, string> ImageData::renderingIntent =
{
    {0, "Perceptual"},
    {1, "Relative colorimetric"},
    {2, "Saturation"},
    {3, "Absolute colorimetric"}
};

const std::unordered_map<string, string> ImageData::compressionType =
{
    {"00", "Stored"},
    {"01", "Fixed Huffman"},
    {"10", "Dynamic Huffman"},
    {"11", "Reserved/Error"}
};

const std::array<HuffmanCode, 4> ImageData::huffmanCodes =
    {{
        {7, 0b0000000, 0b0010111, 256, 279},
        {8, 0b00110000, 0b10111111, 0, 143},
        {8, 0b11000000, 0b11000111, 280, 287},
        {9, 0b110010000, 0b111111111, 144, 255}
    }};

ImageData::ImageData(const string& imageLocation)
{
    convertImageToHex(imageLocation);

    // Signature
    getSmallHexChunk(signature, 8);

    while (!endOfBytes)
    {
        std::map<string, string> chunk{};
        getHexChunk(chunk);

        chunks.push_back({chunk});
    }
}


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

void ImageData::getHexChunk(std::map<string, string>& chunk)
{
    string chunkLength, chunkType, chunkData, CRC;

    getSmallHexChunk(chunkLength, 4);
    chunk["length"] = chunkLength;

    getSmallHexChunk(chunkType, 4);
    chunk["type"] = chunkType;

    // IEND has no data, need to test this
    getSmallHexChunk(chunkData, getHexValue(chunk["length"]));
    chunk["data"] = chunkData;

    getSmallHexChunk(CRC, 4);
    chunk["CRC"] = CRC;

    getChunkData(chunk["data"], chunk["type"], chunk, getHexValue(chunkLength));
}

void ImageData::getSmallHexChunk(string& returnHex, const int length, int startingPoint)
{
    // If len is 1 -> return 2 chars 00, FF, etc.
    // Each hex value has a white space inbetween -> 00 11 -> return 5 chars if len is 2, 8 chars if len is 3

    // If < 0 likely IEND chunk
    if (length < 1)
        return;

    const int characters = length * 2 + (length - 1);

    bool defaultValue = false;

     startingPoint == -1 ? startingPoint = currentPos, defaultValue = true : defaultValue = false;

    if (!defaultValue)
        startingPoint *= 3;

    returnHex = hexString.substr(startingPoint, characters);

    if (defaultValue)
    {
        currentPos += characters + 1;

        if (currentPos == hexString.length())
        {
            endOfBytes = true;
        }
    }
}

int ImageData::getSmallHexChunk(string &returnHex, const string &chunk, const int length, int startingPoint, bool keyword)
{
    startingPoint *= 3;

    if (!keyword)
    {
        // If len is 1 -> return 2 chars 00, FF, etc.
        // Each hex value has a white space inbetween -> 00 11 -> return 5 chars if len is 2, 8 chars if len is 3
        const int characters = length * 2 + (length - 1);

        returnHex = chunk.substr(startingPoint, characters);

        return 0;
    }

    else
    {
        // Size of keyword is not specified, any size between 1-79 bytes, read each byte until a null seperator (00)
        string currentHex;

        int characters = 0;

        while (currentHex = chunk.substr(startingPoint, 2), currentHex != "00")
        {
            characters += 1;
            startingPoint += 3;

            returnHex.append(currentHex + " ");
        }

        returnHex.append("00");

        return characters;
    }
}

void ImageData::getChunkData(const string &chunk, const string &type, std::map<string, string> &chunkData, const int dataLength)
{
    if (type == hexIHDR)
    {
        string width, height, bitDepth, colourTypeData, compression, filter, interlace;

        getSmallHexChunk(width, chunk, 4);
        chunkData["width"] = width;
        imageWidth = getHexValue(width);

        getSmallHexChunk(height, chunk, 4, 4);
        chunkData["height"] = height;
        imageHeight = getHexValue(height);

        getSmallHexChunk(bitDepth, chunk, 1, 8);
        chunkData["bitDepth"] = bitDepth;

        getSmallHexChunk(colourTypeData, chunk, 1, 9);
        chunkData["colourType"] = colourTypeData;
        colourTypeVal = getHexValue(colourTypeData);

        getSmallHexChunk(compression, chunk, 1, 10);
        chunkData["compression"] = compression;

        getSmallHexChunk(filter, chunk, 1, 11);
        chunkData["filter"] = filter;

        getSmallHexChunk(interlace, chunk, 1, 12);
        chunkData["interlace"] = interlace;
    }

    else if (type == hexIDAT)
    {
        string compressionData, FLG, imageData, adlerCheckSum;

        getSmallHexChunk(compressionData, chunk, 1);
        chunkData["compressionData"] = compressionData;

        compressionData = getHexBinary(compressionData);
        chunkData["compressionDataBinary"] = compressionData;
        chunkData["windowSize"] = compressionData.substr(0, 4);
        chunkData["compressionInfo"] = compressionData.substr(4, 4);

        getSmallHexChunk(FLG, chunk, 1, 1);
        chunkData["FLG"] = FLG;

        FLG = getHexBinary(FLG);
        chunkData["FLGBinary"] = FLG;
        chunkData["FCHECK"] = FLG.substr(0, 5);
        chunkData["FDICT"] = FLG.substr(5, 1);
        chunkData["FLEVEL"] = FLG.substr(6, 2);

        // Get actual image data:
        getSmallHexChunk(imageData, chunk, dataLength-6, 2);
        chunkData["imageData"] = imageData;

        string binaryData = getHexBinary(imageData, true);
        chunkData["binaryImageData"] = binaryData;

        chunkData["finalBlock"] = binaryData[0]; // 1 = final, 0 = continue
        chunkData["BTYPE"] = std::string() + binaryData[2] + binaryData[1];

        chunkData["huffmanSequence"] = binaryData.substr(3);
        readImageDataBinary(chunkData["huffmanSequence"], chunkData["BTYPE"]);

        getSmallHexChunk(adlerCheckSum, chunk, 4, dataLength-4); // data length + flg + null
        chunkData["adlerCheckSum"] = adlerCheckSum;
    }

    else if (type == hexIEND)
    {
        return;
    }

    else if (type == hexsRGB)
    {
        string renderingIntentVal;
        getSmallHexChunk(renderingIntentVal, chunk, 1);
        chunkData["renderingIntent"] = renderingIntentVal;
    }

    else if (type == hexsBIT)
    {
        switch (colourTypeVal)
        {
            case 0:
            {
                string greyScale;
                getSmallHexChunk(greyScale, chunk, 1);
                chunkData["greyScale"] = greyScale;
                break;
            }

            case 2: case 3:
            {
                string red, green, blue;

                getSmallHexChunk(red, chunk, 1);
                chunkData["red"] = red;
                getSmallHexChunk(green, chunk, 1, 1);
                chunkData["green"] = green;
                getSmallHexChunk(blue, chunk, 1, 2);
                chunkData["blue"] = blue;
                break;
            }

            case 4:
            {
                string greyScale, alpha;
                getSmallHexChunk(greyScale, chunk, 1);
                chunkData["greyScale"] = greyScale;

                getSmallHexChunk(alpha, chunk, 1, 1);
                chunkData["alpha"] = alpha;
                break;
            }

            case 6:
            {
                string red, green, blue, alpha;

                getSmallHexChunk(red, chunk, 1);
                chunkData["red"] = red;
                getSmallHexChunk(green, chunk, 1, 1);
                chunkData["green"] = green;
                getSmallHexChunk(blue, chunk, 1, 2);
                chunkData["blue"] = blue;

                getSmallHexChunk(alpha, chunk, 1, 3);
                chunkData["alpha"] = alpha;
                break;
            }
        }
    }

    else if (type == hextEXt)
    {
        string keyword, nullSeperator, text;

        int keywordLength = getSmallHexChunk(keyword, chunk, 0, 0, true);
        chunkData["keyword"] = keyword;
        getSmallHexChunk(nullSeperator, chunk, 1, keywordLength);
        chunkData["nullSeperator"] = nullSeperator;
        getSmallHexChunk(text, chunk, dataLength - keywordLength+1, keywordLength+1);
        chunkData["text"] = text;
    }

    else
    {
        cout << "Invalid chunk type: " << type << endl;
        exit(-1);
    }
}

void ImageData::readImageDataBinary(string& binaryData, const string& compressionVal)
{
    string type = compressionType.find(compressionVal)->second;

    if (type == "Stored")
    {
        cout << "INCOMPLETED COMPRESSION TYPE: " << type << endl;
        exit(-1);
    }

    else if (type == "Fixed Huffman")
    {
        int bitPos = 0;
        bool firstBit = true;
        int rgbLoc = 0;
        int pixelLoc = 0;

        while (bitPos < binaryData.length())
        {
            bool matchedCode = false;

            for (auto& code : huffmanCodes)
            {
                string checkBits = binaryData.substr(bitPos, code.bits);

                int bitValue = std::stoi(checkBits, nullptr, 2);

                if (code.minBinary <= bitValue && bitValue <= code.maxBinary)
                {
                    matchedCode = true;
                    bitPos += code.bits;
                    int symbol = code.minSymbol + (bitValue - code.minBinary);

                    if (symbol == 256)
                    {
                        int loc = 0;
                        int buffer = 0;
                        int row = 0;

                        for (auto& val : pixelData)
                        {
                            if (loc == 0)
                            {
                                buffer = val;
                                cout << "Buffer: " << buffer << endl;
                                loc++;
                                continue;
                            }

                            if (buffer == 1)
                            {
                                int leftVal;

                                if (loc < 5)
                                    leftVal = 0;
                                else
                                    leftVal = pixelData[((imageWidth*4+1)*row) + loc - 4];

                                val = (val + leftVal) % 256;
                            }

                            else if (buffer == 2)
                            {
                                if (row == 0)
                                    val = val % 256;
                                else
                                    val = (val + pixelData[((row-1) * (imageWidth*4+1)) + loc]) % 256;
                            }

                            else if (buffer == 3 || buffer == 4)
                            {
                                int leftVal;
                                int upperVal;

                                if (loc < 5)
                                    leftVal = 0;
                                else
                                    leftVal = pixelData[((imageWidth*4+1)*row) + loc-4];
                                if (row == 0)
                                    upperVal = 0;
                                else
                                    upperVal = pixelData[((row-1) * (imageWidth*4+1)) + loc];

                                if (buffer == 3)
                                {
                                    val = (val + (int)floor((leftVal + upperVal) / 2.0)) % 256;
                                }
                                else
                                {
                                    int upperLeftVal;

                                    if (row == 0 || loc < 5)
                                        upperLeftVal = 0;
                                    else
                                        upperLeftVal = pixelData[((row-1) * (imageWidth*4+1)) + loc-4];

                                    // Paeth implementation

                                    int paeth = leftVal + upperVal - upperLeftVal;

                                    int paethLeft = abs(paeth - leftVal);
                                    int paethUp = abs(paeth - upperVal);
                                    int paethUpLeft = abs(paeth - upperLeftVal);

                                    if (paethLeft <= paethUp && paethLeft <= paethUpLeft)
                                        val = (val + leftVal) % 256;
                                    else if (paethUp <= paethUpLeft)
                                        val = (val + upperVal) % 256;
                                    else
                                        val = (val + upperLeftVal) % 256;
                                }

                            }

                            loc++;

                            if (loc == imageWidth*4+1)
                            {
                                loc = 0;
                                row++;
                            }
                        }

                        loc = 0;
                        Pixel pixel{};
                        rgbLoc = 0;
                        for (auto& val : pixelData)
                        {
                            if (loc % (imageWidth*4+1) == 0)
                            {
                                loc++;
                                continue;
                            }

                            switch (rgbLoc)
                            {
                                case 0:
                                {
                                    pixel.red = val;
                                    break;
                                }
                                case 1:
                                {
                                    pixel.green = val;
                                    break;
                                }
                                case 2:
                                {
                                    pixel.blue = val;
                                    break;
                                }
                                case 3:
                                {
                                    pixel.alpha = val;
                                    image.push_back(pixel);
                                    pixel = Pixel();
                                    rgbLoc = -1;
                                    break;
                                }
                            }
                            rgbLoc++;
                            loc++;
                        }
                        return;
                    }

                    cout << checkBits << " -> " << symbol << endl;

                    if (firstBit && !(symbol >= 257 && symbol <= 285))
                    {
                        pixelData.push_back(symbol);
                        firstBit = false;

                        cout << symbol << endl;
                    }

                    else if (symbol >= 257 && symbol <= 285)
                    {
                        // Calculate length

                        int length = 0;

                        if (symbol < 285)
                        {
                            int offset = symbol - 257;

                            int extraBits = (offset < 8 || symbol == 285) ? 0 : ((offset - 8)/4) + 1;

                            int extraBitsVal = 0;

                            if (extraBits > 0)
                            {
                                string extraBitsStr = binaryData.substr(bitPos, extraBits);
                                std::reverse(extraBitsStr.begin(), extraBitsStr.end());
                                cout << extraBitsStr << " -> ";
                                extraBitsVal = std::stoi(extraBitsStr, nullptr, 2);
                                cout << extraBitsVal << endl;

                                bitPos += extraBits;
                            }

                            int bitGroup = (offset - 8) % 4;

                            // << is so useful!!! (Works like: (1 << n) = (1 * 2^n)
                            length = extraBits == 0 ? 3 + offset : 3 + (1 << (extraBits + 2)) + bitGroup * (1 << extraBits) + extraBitsVal;
                        }
                        else
                        {
                            length = 258;
                        }

                        checkBits = binaryData.substr(bitPos, 5);
                        bitValue = std::stoi(checkBits, nullptr, 2);

                        bitPos += 5;

                        cout << checkBits << " -> " << bitValue << endl;

                        // Calculate distance

                        int distance = bitValue + 1;
                        if (bitValue >= 4)
                        {
                            int extraBits = (bitValue - 4) / 2 + 1;

                            int extraBitsVal = 0;
                            if (extraBits > 0)
                            {
                                string extraBitsStr = binaryData.substr(bitPos, extraBits);
                                std::reverse(extraBitsStr.begin(), extraBitsStr.end());
                                cout << extraBitsStr << " -> ";
                                extraBitsVal = std::stoi(extraBitsStr, nullptr, 2);
                                cout << extraBitsVal << endl;

                                bitPos += extraBits;
                            }

                            int bitGroup = (bitValue - 4) % 2;

                            distance = 1 + (1 << (extraBits + 1)) + bitGroup * (1 << extraBits) + extraBitsVal;
                        }

                        // Add length bytes and move back distance bytes

                        int repeat;

                        for (int i = 0; i < length; i++)
                        {
                            repeat = pixelData[pixelData.size()-distance];
                            pixelData.push_back(repeat);
                            cout << repeat << " ";

                            if (firstBit && i == 0)
                            {
                                firstBit = false;
                                continue;
                            }

                            rgbLoc++;
                            if (rgbLoc >= 4)
                            {
                                rgbLoc = 0;
                                pixelLoc++;
                                if (pixelLoc == imageWidth)
                                {
                                    pixelLoc = 0;
                                    firstBit = true;
                                }
                            }
                        }
                        cout << endl;
                    }

                    else
                    {
                        pixelData.push_back(symbol);
                        rgbLoc++;

                        if (rgbLoc >= 4)
                        {
                            rgbLoc = 0;
                            pixelLoc++;
                            if (pixelLoc == imageWidth)
                            {
                                pixelLoc = 0;
                                firstBit = true;
                            }
                        }

                        cout << symbol << endl;
                    }
                    break;
                }
            }

            if (!matchedCode)
            {
                cout << "Invalid bit string!" << endl;
                exit(-1);
            }
        }
        cout << "N0 EXIT SYMBOL FOUND, EXITING" << endl;
        exit(-1);
    }

    else if (type == "Dynamic Huffman")
    {
        cout << "INCOMPLETED COMPRESSION TYPE: " << type << endl;
        exit(-1);
    }

    else if (type == "Reserved/Error")
    {
        cout << "INCOMPLETED COMPRESSION TYPE: " << type << endl;
        exit(-1);
    }

    else
    {
        cout << "Unknown compression type: " << compressionVal << endl;
        exit(-1);
    }
}

void ImageData::displayChunk(std::map<string, string> &chunk)
{
    cout << "\n\n";

    if (chunk["type"] == hexIHDR)
    {
        cout << "Chunk size: " << chunk["length"] << " (" << getHexValue(chunk["length"]) << ")" << endl;
        cout << "Chunk type: " << chunk["type"]  << " (IHDR)" << endl;

        cout << "Width: " << chunk["width"] << " (" << getHexValue(chunk["width"]) << ")" << endl;
        cout << "Height: " << chunk["height"] << " (" << getHexValue(chunk["height"]) << ")" << endl;
        cout << "Bit depth: " << chunk["bitDepth"] << endl;
        cout << "Colour type: " << chunk["colourType"] << " (" << colourType.find(colourTypeVal)->second << ")" << endl;
        cout << "Compression: " << chunk["compression"] << endl;
        cout << "Filter: " << chunk["filter"] << endl;
        cout << "Interlace: " << chunk["interlace"] << endl;

        cout << "CRC: " << chunk["CRC"] << endl;
    }

    else if (chunk["type"] == hexIDAT)
    {
        cout << "Chunk size: " << chunk["length"] << " (" << getHexValue(chunk["length"]) << ")" << endl;
        cout << "Chunk type: " << chunk["type"]  << " (IDAT)" << endl;

        cout << "Image Info: " << chunk["data"] << endl;

        cout << "Compression data: " << chunk["compressionData"] << " (Binary: " << chunk["compressionDataBinary"] << ")" << endl;
        cout << "Compression info: " << chunk["compressionInfo"] << (chunk["compressionInfo"] == "1000" ? " (Deflate)" : " (Unknown)") << endl;
        cout << "Window size: " << chunk["windowSize"] << " (" << powf(2, getBinaryValue(chunk["windowSize"])+8) << " bytes (2^(windowSize+8))" << endl;

        cout << "FLG: " << chunk["FLG"] << " (Binary: " << chunk["FLGBinary"] << ")" << endl;
        cout << "Check bits: " << chunk["FCHECK"] << " -> (" << getHexValue(chunk["compressionData"]) << "*256 + " << getHexValue(chunk["FLG"]) << ") % 31 = " << (getHexValue(chunk["compressionData"])*256 + getHexValue(chunk["FLG"])) % 31 << endl;
        cout << "Preset dictionary: " << chunk["FDICT"] << endl;
        cout << "Compression level: " << chunk["FLEVEL"] << endl;

        cout << "Image Data: " << chunk["imageData"] << endl;
        cout << "Binary Image Data: " << chunk["binaryImageData"] << endl;

        cout << "Final Image Data Chunk? : " << chunk["finalBlock"] << (chunk["finalBlock"] == "1" ? " (True)" : " (False)") << endl;// 1 = final, 0 = continue
        cout << "BTYPE: " << chunk["BTYPE"] << " (" << compressionType.find(chunk["BTYPE"])->second << ")" << endl;

        cout << "Huffman Sequence: " << chunk["huffmanSequence"] << endl;
        cout << "Pixels: " << endl;
        int counter = 0;
        for (auto pixel : image)
        {
            if (counter == imageWidth)
            {
                cout << "New Row: " << endl;
                counter = 0;
            }

            cout << "Red: " << pixel.red << " Green: " << pixel.green << " Blue: " << pixel.blue << " Alpha: " << pixel.alpha << endl;
            counter++;
        }

        cout << "Adler Checksum: " << chunk["adlerCheckSum"] << endl;

        cout << "CRC: " << chunk["CRC"] << endl;
    }

    else if (chunk["type"] == hexIEND)
    {
        cout << "Chunk size: " << chunk["length"] << " (" << getHexValue(chunk["length"]) << ")" << endl;
        cout << "Chunk type: " << chunk["type"]  << " (IEND)" << endl;

        cout << "Data: " << "NO DATA (IEND)" << endl;

        cout << "CRC: " << chunk["CRC"] << endl;
    }

    else if (chunk["type"] == hexsRGB)
    {
        cout << "Chunk size: " << chunk["length"] << " (" << getHexValue(chunk["length"]) << ")" << endl;
        cout << "Chunk type: " << chunk["type"]  << " (sRGB)" << endl;

        cout << "Rendering Intent: " << chunk["renderingIntent"] << " (" << renderingIntent.find(getHexValue(chunk["renderingIntent"]))->second << ") " << endl;

        cout << "CRC: " << chunk["CRC"] << endl;
    }

    else if (chunk["type"] == hexsBIT)
    {
        cout << "Chunk size: " << chunk["length"] << " (" << getHexValue(chunk["length"]) << ")" << endl;
        cout << "Chunk type: " << chunk["type"]  << " (sBIT)" << endl;

        switch (colourTypeVal)
        {
            case 0:
            {
                cout << "Grey scale: " << chunk["greyScale"] << endl;
                break;
            }

            case 2: case 3:
            {
                cout << "Red: " << chunk["red"] << endl;
                cout << "Green: " << chunk["green"] << endl;
                cout << "Blue: " << chunk["blue"] << endl;
                break;
            }

            case 4:
            {
                cout << "Grey scale: " << chunk["greyScale"] << endl;
                cout << "Alpha: " << chunk["alpha"] << endl;
                break;
            }

            case 6:
            {
                cout << "Red: " << chunk["red"] << endl;
                cout << "Green: " << chunk["green"] << endl;
                cout << "Blue: " << chunk["blue"] << endl;
                cout << "Alpha: " << chunk["alpha"] << endl;
                break;
            }
        }

        cout << "CRC: " << chunk["CRC"] << endl;
    }

    else if (chunk["type"] == hextEXt)
    {
        cout << "Keyword: " << chunk["keyword"] << " (" << getHexASCII(chunk["keyword"]) << ") " << endl;
        cout << "Null seperator: " << chunk["nullSeperator"] << endl;
        cout << "Text: " << chunk["text"] << " (" << getHexASCII(chunk["text"]) << ") " << endl;

        cout << "CRC: " << chunk["CRC"] << endl;
    }

    else
    {
        cout << "Invalid chunk type: " << chunk["type"] << endl;
        exit(-1);
    }
}

int ImageData::getHexValue(const string &hexChunk)
{
    auto hexVal = hexChunk;
    std::erase_if(hexVal, isspace);

    return std::stoi(hexVal, nullptr, 16);
}

int ImageData::getBinaryValue(const string &binaryChunk)
{
    return std::stoi(binaryChunk, nullptr, 2);
}

string ImageData::getHexASCII(const string &hexChunk)
{
    string asciiChunk;

    for (int i = 0; i < hexChunk.length(); i += 3)
    {
        asciiChunk += getHexASCIIValue(hexChunk[i])*16 + getHexASCIIValue(hexChunk[i + 1]);
    }

    return asciiChunk;
}

int ImageData::getHexASCIIValue(char hexChar)
{
    if (hexChar >= '0' && hexChar <= '9')
        return hexChar - '0';

    if (hexChar >= 'a' && hexChar <= 'f')
        return  hexChar - 'a' + 10;

    if (hexChar >= 'A' && hexChar <= 'F')
        return hexChar - 'A' + 10;

    return -1;
}

string ImageData::getHexBinary(const string& hexChunk, bool LSB)
{
    string binaryChunk;
    string LSBHolder;

    int count = 0;

    for (char c : hexChunk)
    {
        switch (c)
        {
            // If more than 1 byte is being calculated
            case ' ':
            {
                count--;
                break;
            }

            case '0':
            {
                if (LSB)
                {
                    if (count % 2 == 0)
                        LSBHolder = "0000";
                    else
                    {
                        binaryChunk += "0000";
                        binaryChunk += LSBHolder;
                    }
                }
                else
                    binaryChunk += "0000";

                break;
            }
            case '1':
            {
                if (LSB)
                {
                    if (count % 2 == 0)
                        LSBHolder = "1000";
                    else
                    {
                        binaryChunk += "1000";
                        binaryChunk += LSBHolder;
                    }
                }
                else
                    binaryChunk += "0001";
                break;
            }
            case '2':
            {
                if (LSB)
                {
                    if (count % 2 == 0)
                        LSBHolder = "0100";
                    else
                    {
                        binaryChunk += "0100";
                        binaryChunk += LSBHolder;
                    }
                }
                else
                    binaryChunk += "0010";
                break;
            }
            case '3':
            {
                if (LSB)
                {
                    if (count % 2 == 0)
                        LSBHolder = "1100";
                    else
                    {
                        binaryChunk += "1100";
                        binaryChunk += LSBHolder;
                    }
                }
                else
                    binaryChunk += "0011";
                break;
            }
            case '4':
            {
                if (LSB)
                {
                    if (count % 2 == 0)
                        LSBHolder = "0010";
                    else
                    {
                        binaryChunk += "0010";
                        binaryChunk += LSBHolder;
                    }
                }
                else
                    binaryChunk += "0100";
                break;
            }
            case '5':
            {
                if (LSB)
                {
                    if (count % 2 == 0)
                        LSBHolder = "1010";
                    else
                    {
                        binaryChunk += "1010";
                        binaryChunk += LSBHolder;
                    }
                }
                else
                    binaryChunk += "0101";
                break;
            }
            case '6':
            {
                if (LSB)
                {
                    if (count % 2 == 0)
                        LSBHolder = "0110";
                    else
                    {
                        binaryChunk += "0110";
                        binaryChunk += LSBHolder;
                    }
                }
                else
                    binaryChunk += "0110";
                break;
            }
            case '7':
            {
                if (LSB)
                {
                    if (count % 2 == 0)
                        LSBHolder = "1110";
                    else
                    {
                        binaryChunk += "1110";
                        binaryChunk += LSBHolder;
                    }
                }
                else
                    binaryChunk += "0111";
                break;
            }
            case '8':
            {
                if (LSB)
                {
                    if (count % 2 == 0)
                        LSBHolder = "0001";
                    else
                    {
                        binaryChunk += "0001";
                        binaryChunk += LSBHolder;
                    }
                }
                else
                    binaryChunk += "1000";
                break;
            }
            case '9':
            {
                if (LSB)
                {
                    if (count % 2 == 0)
                        LSBHolder = "1001";
                    else
                    {
                        binaryChunk += "1001";
                        binaryChunk += LSBHolder;
                    }
                }
                else
                    binaryChunk += "1001";
                break;
            }
            case 'a':
            {
                if (LSB)
                {
                    if (count % 2 == 0)
                        LSBHolder = "0101";
                    else
                    {
                        binaryChunk += "0101";
                        binaryChunk += LSBHolder;
                    }
                }
                else
                    binaryChunk += "1010";
                break;
            }
            case 'b':
            {
                if (LSB)
                {
                    if (count % 2 == 0)
                        LSBHolder = "1101";
                    else
                    {
                        binaryChunk += "1101";
                        binaryChunk += LSBHolder;
                    }
                }
                else
                    binaryChunk += "1011";
                break;
            }
            case 'c':
            {
                if (LSB)
                {
                    if (count % 2 == 0)
                        LSBHolder = "0011";
                    else
                    {
                        binaryChunk += "0011";
                        binaryChunk += LSBHolder;
                    }
                }
                else
                    binaryChunk += "1100";
                break;
            }
            case 'd':
            {
                if (LSB)
                {
                    if (count % 2 == 0)
                        LSBHolder = "1011";
                    else
                    {
                        binaryChunk += "1011";
                        binaryChunk += LSBHolder;
                    }
                }
                else
                    binaryChunk += "1101";
                break;
            }
            case 'e':
            {
                if (LSB)
                {
                    if (count % 2 == 0)
                        LSBHolder = "0111";
                    else
                    {
                        binaryChunk += "0111";
                        binaryChunk += LSBHolder;
                    }
                }
                else
                    binaryChunk += "1110";
                break;
            }
            case 'f':
            {
                if (LSB)
                {
                    if (count % 2 == 0)
                        LSBHolder = "1111";
                    else
                    {
                        binaryChunk += "1111";
                        binaryChunk += LSBHolder;
                    }
                }
                else
                    binaryChunk += "1111";
                break;
            }
        }
        count++;
    }

    return binaryChunk;
}
