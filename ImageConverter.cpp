#include "ImageConverter.hpp"

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

    if (chunkType == hextEXt)
        getChunkData(chunk["data"], chunk["type"], chunk, getHexValue(chunkLength));

    else
        getChunkData(chunk["data"], chunk["type"], chunk);
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

void ImageData::getSmallHexChunk(string &returnHex, const string &chunk, const int length, int startingPoint, bool keyword)
{
    if (!keyword)
    {
        // If len is 1 -> return 2 chars 00, FF, etc.
        // Each hex value has a white space inbetween -> 00 11 -> return 5 chars if len is 2, 8 chars if len is 3
        const int characters = length * 2 + (length - 1);

        startingPoint *= 3;

        returnHex = chunk.substr(startingPoint, characters);
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

        keywordLength = characters;
        returnHex.erase(returnHex.end()-1);
    }
}

void ImageData::getChunkData(const string &chunk, const string &type, std::map<string, string> &chunkData, const int dataLength)
{
    if (type == hexIHDR)
    {
        string width, height, bitDepth, colourTypeData, compression, filter, interlace;

        getSmallHexChunk(width, chunk, 4);
        chunkData["width"] = width;

        getSmallHexChunk(height, chunk, 4, 4);
        chunkData["height"] = height;

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
        string compressionData, FLG;

        getSmallHexChunk(compressionData, chunk, 1);
        chunkData["compressionData"] = compressionData;

        compressionData = getHexBinary(compressionData);
        chunkData["compressionDataBinary"] = compressionData;
        chunkData["compressionInfo"] = compressionData.substr(0, 4);
        chunkData["compressionMethod"] = compressionData.substr(4, 4);

        getSmallHexChunk(FLG, chunk, 1, 1);
        chunkData["FLG"] = FLG;

        FLG = getHexBinary(FLG);
        chunkData["FLGBinary"] = FLG;
        chunkData["FCHECK"] = FLG.substr(0, 5);
        chunkData["FDICT"] = FLG.substr(5, 1);
        chunkData["FLEVEL"] = FLG.substr(6, 2);
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

        getSmallHexChunk(keyword, chunk, 0, 0, true);
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
        cout << "Compression info: " << chunk["compressionInfo"] << endl;
        cout << "Compression method: " << chunk["compressionMethod"] << endl;

        cout << "FLG: " << chunk["FLG"] << " (Binary: " << chunk["FLGBinary"] << ")" << endl;
        cout << "Check bits: " << chunk["FCHECK"] << " -> (" << getHexValue(chunk["compressionData"]) << "*256 + " << getHexValue(chunk["FLG"]) << ") % 31 = " << (getHexValue(chunk["compressionData"])*256 + getHexValue(chunk["FLG"])) % 31 << endl;
        cout << "Preset dictionary: " << chunk["FDICT"] << endl;
        cout << "Compression level: " << chunk["FLEVEL"] << endl;

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

    else if (hexChar >= 'a' && hexChar <= 'f')
        return  hexChar - 'a' + 10;

    else if (hexChar >= 'A' && hexChar <= 'F')
        return hexChar - 'A' + 10;

    else
        return -1;
}

string ImageData::getHexBinary(const string& hexChunk)
{
    string binaryChunk;

    for (char c : hexChunk)
    {
        switch (c)
        {
            case '0':
            {
                binaryChunk += "0000";
                break;
            }
            case '1':
            {
                binaryChunk += "0001";
                break;
            }
            case '2':
            {
                binaryChunk += "0010";
                break;
            }
            case '3':
            {
                binaryChunk += "0011";
                break;
            }
            case '4':
            {
                binaryChunk += "0100";
                break;
            }
            case '5':
            {
                binaryChunk += "0101";
                break;
            }
            case '6':
            {
                binaryChunk += "0110";
                break;
            }
            case '7':
            {
                binaryChunk += "0111";
                break;
            }
            case '8':
            {
                binaryChunk += "1000";
                break;
            }
            case '9':
            {
                binaryChunk += "1001";
                break;
            }
            case 'a':
            {
                binaryChunk += "1010";
                break;
            }
            case 'b':
            {
                binaryChunk += "1011";
                break;
            }
            case 'c':
            {
                binaryChunk += "1100";
                break;
            }
            case 'd':
            {
                binaryChunk += "1101";
                break;
            }
            case 'e':
            {
                binaryChunk += "1110";
                break;
            }
            case 'f':
            {
                binaryChunk += "1111";
                break;
            }
        }
    }

    return binaryChunk;
}
