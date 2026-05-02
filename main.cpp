#include "main.hpp"

int main()
{
    ImageData imageData;

    string hexString;
    imageData.convertImageToHex(imageLocationOne, hexString);

    string signature, chunkSize, chunkType, chunkData, CRC;

    // Signature
    imageData.getHexChunk(hexString, 8, signature);
    // ChunkSize
    imageData.getHexChunk(hexString, 4, chunkSize);
    // ChunkType
    imageData.getHexChunk(hexString, 4, chunkType);
    // ChunkData
    imageData.getHexChunk(hexString, imageData.getHexValue(chunkSize), chunkData);
    // CRC
    imageData.getHexChunk(hexString, 4, CRC);

    cout << "Image type: " << signature << (signature == pngSignature ? " (PNG)" : "") << " Chunk size: " << chunkSize << " (" << imageData.getHexValue(chunkSize) << ")" << " Chunk type: " << chunkType  << (chunkType == hexIHDR ? " (IHDR)" : "") << endl;
    cout << "Chunk data: " << chunkData << endl;

    return 0;
}
