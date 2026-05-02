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

    cout << "Image type: " << signature << (signature == pngSignature ? " (PNG)" : "") << " Chunk size: " << chunk["length"] << " (" << imageData.getHexValue(chunk["length"]) << ")" << " Chunk type: " << chunk["type"]  << (chunk["type"] == hexIHDR ? " (IHDR)" : "") << endl;
    cout << "Width: " << chunkData["width"] << " (" << imageData.getHexValue(chunkData["width"]) << ")" << " Height: " << chunkData["height"] << " (" << imageData.getHexValue(chunkData["height"]) << ")" << " Bit depth: " << chunkData["bitDepth"] << " Colour type: " << chunkData["colourType"] << " (" << imageData.colourType.find(imageData.getHexValue(chunkData["colourType"]))->second << ")" << " Compression: " << chunkData["compression"] << " Filter: " << chunkData["filter"] << " Interlace: " << chunkData["interlace"] << endl;

    return 0;
}
