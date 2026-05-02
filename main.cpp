#include "main.hpp"

int main()
{
    ImageData imageData;

    imageData.convertImageToHex(imageLocationOne);

    string signature;

    // Signature
    imageData.getSmallHexChunk(signature, 8);

    std::map<string, string> chunk{};
    imageData.getHexChunk(chunk);

    std::map<string, string> chunkData{};
    imageData.getChunkData(chunk["data"], chunk["type"], chunkData);

    cout << imageData.hexString << endl;

    cout << "Image type: " << signature << (signature == pngSignature ? " (PNG)" : "") << endl;

    imageData.displayChunk(chunk, chunkData);

    return 0;
}
