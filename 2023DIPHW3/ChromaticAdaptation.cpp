#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>

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


void grayWorldMethod(std::vector<unsigned char>& data) {
    double sum_r = 0.0;
    double sum_g = 0.0;
    double sum_b = 0.0;
    for (size_t i = 0; i < data.size(); i+=3){
        sum_r += data[i];
        sum_g += data[i + 1];
        sum_b += data[i + 2];
    }
    double avg_r = sum_r / (data.size() / 3);
    double avg_g = sum_g / (data.size() / 3);
    double avg_b = sum_b / (data.size() / 3);

    cout << "avg_r: " << avg_r  << "avg_b: " << avg_b << "avg_g: " << avg_g << endl; // "avg_r: 0.0avg_b: 0.0avg_g: 0.0

    double gray_world_value = (avg_r + avg_g + avg_b) / 3.0;
    cout << "gray_world_value: " << gray_world_value << endl; // "gray_world_value: 0.0
    
    for (size_t i = 0; i < data.size(); i+=3){
        // cout << int(data[i]) << " " << int(data[i + 1]) << " " << int(data[i + 2]) << endl;
        if ((data[i] * gray_world_value / avg_r <= 255) && (data[i] * gray_world_value / avg_r >= 0)) {
            data[i] = static_cast<unsigned char>(data[i] * gray_world_value / avg_r);
        }
        if (data[i + 1] * gray_world_value / avg_g <= 255 && data[i + 1] * gray_world_value / avg_g >= 0) {
            data[i + 1] = static_cast<unsigned char>(data[i + 1] * gray_world_value / avg_g);
        }
        if (data[i + 2] * gray_world_value / avg_b <= 255 && data[i + 2] * gray_world_value / avg_b >= 0) {
            data[i + 2] = static_cast<unsigned char>(data[i + 2] * gray_world_value / avg_b);
        }      
    }
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " k d" << " : k is input_num, " << " d is enhance degree, which should be either 1 or 2." << endl;
        return 1;
    }
    string input_num = string(argv[1]);
    int enhance_degree = stoi(string(argv[2]));
    if ((enhance_degree < 1) || (enhance_degree > 2)) {
        cerr << "Usage: " << argv[0] << " k d" << " : k is input_num, " << " d is enhance degree, which should be either 1 or 2." << endl;
        return 1;
    }

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

    /* Chromatic Adaptation */
    grayWorldMethod(data);
    


    string output_filename = "output" + input_num + "_" + to_string(enhance_degree) + ".bmp";
    ofstream output(output_filename, ios::out | ios::binary);
    if (!output.is_open()) {
        std::cerr << "Error creating the output file" << std::endl;
        return -1;
    }
    output.write(reinterpret_cast<const char*>(&header), sizeof(BMPHeader));
    output.write(reinterpret_cast<const char*>(&infoHeader), sizeof(BMPInfoHeader));
    output.write(reinterpret_cast<const char*>(data.data()), data.size());

    output.close();

    return 0;
}