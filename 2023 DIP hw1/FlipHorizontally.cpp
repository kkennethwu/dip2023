#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

#pragma pack(push, 1) // Disable structure padding
struct BMPHeader {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
};

struct BMPInfoHeader {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    int32_t xPixelsPerMeter;
    int32_t yPixelsPerMeter;
    uint32_t colorsUsed;
    uint32_t colorsImportant;
};
#pragma pack(pop)

void FlipHorizontally(vector<unsigned char>& data, int height, int width, BMPHeader &header, BMPInfoHeader &infoHeader, string input_num, int num_channel);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " k" << " : k is input_num" << endl;
        return 1;
    }
    string input_num = string(argv[1]);


    /* Read BMP */
    string filename = "input" + input_num + ".bmp";
    std::ifstream file(filename, std::ios::in | std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error opening the file" << std::endl;
        return 1;
    }

    BMPHeader header;
    BMPInfoHeader infoHeader;

    // Read the headers
    file.read(reinterpret_cast<char*>(&header), sizeof(BMPHeader));
    file.read(reinterpret_cast<char*>(&infoHeader), sizeof(BMPInfoHeader));

    // Check if it's a BMP file
    if (header.type != 0x4D42) {
        std::cerr << "Not a BMP file" << std::endl;
        return 1;
    }
    
    int bitsPerPixel = infoHeader.bitsPerPixel;
    int width = infoHeader.width;
    int height = infoHeader.height;
    int num_channel = bitsPerPixel / 8;
    int imageSize = width * height * num_channel; // Each pixel has RGB or RGBA

    // Allocate memory to store pixel data
    std::vector<unsigned char> data(imageSize);
    // Read pixel data
    file.read(reinterpret_cast<char*>(data.data()), imageSize);
    // Close the file
    file.close();
    


    /*Task1:  Flip Horizontally*/
    FlipHorizontally(data, height, width, header, infoHeader, input_num, num_channel);

    
    return 0;
}

void FlipHorizontally(vector<unsigned char>& data, int height, int width, BMPHeader &header, BMPInfoHeader &infoHeader, string input_num, int num_channel){
    std::vector<unsigned char> data_copy(data);
    for(int y = 0; y < height; y++){
        for(int x = 0; x < width; x++){
            int index = num_channel * (x + y * width);
            int target_index = num_channel * ((width - 1 - x) + y * width);
            for(int c = 0; c < num_channel; c++){
                data_copy[index+c] = data[target_index+c];
            }
        }
    }
    string filename = "output" + input_num + "_flip.bmp";
    ofstream output(filename, ios::out | ios::binary);
    if (!output.is_open()) {
        std::cerr << "Error creating the output file" << std::endl;
        return;
    }

    // Write the headers
    output.write(reinterpret_cast<const char*>(&header), sizeof(BMPHeader));
    output.write(reinterpret_cast<const char*>(&infoHeader), sizeof(BMPInfoHeader));

    // Write the quantized pixel data
    output.write(reinterpret_cast<const char*>(data_copy.data()), data_copy.size());

    // Close the output file
    output.close();
}
