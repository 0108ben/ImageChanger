#include "main.hpp"

int main()
{
    ImageData imageData;

    imageData.convertImageToHex(imageLocationOne);

    string signature, chunkSize, chunkType, chunkData, CRC;

    // Signature
    imageData.getHexChunk(8, signature);
    // ChunkSize
    imageData.getHexChunk(4, chunkSize);
    // ChunkType
    imageData.getHexChunk(4, chunkType);
    // ChunkData
    imageData.getHexChunk(imageData.getHexValue(chunkSize), chunkData);
    // CRC
    imageData.getHexChunk(4, CRC);

    cout << "Image type: " << signature << (signature == pngSignature ? " (PNG)" : "") << " Chunk size: " << chunkSize << " (" << imageData.getHexValue(chunkSize) << ")" << " Chunk type: " << chunkType  << (chunkType == hexIHDR ? " (IHDR)" : "") << endl;
    cout << "Chunk data: " << chunkData << endl;

    return 0;
}
