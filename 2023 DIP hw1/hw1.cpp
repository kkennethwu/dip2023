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

void Scaling(vector<unsigned char>& data, string up_down, int height, int width, BMPHeader &header, BMPInfoHeader &infoHeader, float rate, string input_num, int num_channel);

void Resolution(vector<unsigned char>& data, int reso, string input_num, BMPHeader &header, BMPInfoHeader &infoHeader, int num_channel);

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
    
    /*Task 2: Resolution*/
    Resolution(data, 6, input_num, header, infoHeader, num_channel);
    Resolution(data, 4, input_num, header, infoHeader, num_channel);
    Resolution(data, 2, input_num, header, infoHeader, num_channel);    

    /*Task 3: Down/Up Scaling*/
    // downscale 1.5
    Scaling(data, "down", height, width, header, infoHeader, 1.5, input_num, num_channel);
    // upscale 1.5
    Scaling(data, "up", height, width, header, infoHeader, 1 / 1.5, input_num, num_channel);
    
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

void Resolution(vector<unsigned char>& data, int reso, string input_num, BMPHeader &header, BMPInfoHeader &infoHeader, int num_channel) {
    std::vector<unsigned char> data_copy(data);
    // for(int i = 0; i < data.size(); i++){
    //     data_copy.push_back(data[i]);
    // }

    int k = 8 - reso; // k is the number of discarded bits
    for(int i = 0; i < data_copy.size(); i+=num_channel){ 
        // discard k least significant bits, and shift back to padding them with 0
        for(int c = 0; c < num_channel; c++){
            data_copy[i+c] = (data_copy[i+c] >> k) << k;
        }
    }

    string filename = "output" + input_num + "_" + to_string(k/2) + ".bmp";
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

void Scaling(vector<unsigned char>& data, string up_down, int height, int width, BMPHeader &header, BMPInfoHeader &infoHeader, float rate, string input_num, int num_channel) {
    // round to the nearest neighbor
    int new_height = int(height / rate); 
    int new_width = int(round((width / rate) / 4.0) * 4.0); //approximate to the nearest multiple of 4
    int new_ImageSize = new_height * new_width * num_channel;
    // cout << "w, h, image_size: " << new_width << " " << new_height << " " << new_ImageSize << endl;
    
    vector<unsigned char> scaledData(new_ImageSize, 255);
    float sourceX, sourceY, x_weight, y_weight;
    int sourceX_floor, sourceY_floor;
    for(int y = 0; y < new_height; y++){
        for(int x = 0; x < new_width; x++){
            sourceX = x * (width - 1) / (new_width - 1);
            sourceY = y * (height - 1) / (new_height - 1);
            sourceX_floor = int(sourceX);
            sourceY_floor = int(sourceY);
            x_weight = sourceX - sourceX_floor;
            y_weight = sourceY - sourceY_floor;
    
            for(int c = 0; c < num_channel; c++){
                int b1 = data[num_channel * (sourceX_floor + sourceY_floor * width) + c];
                int b2 = data[num_channel * (sourceX_floor + (sourceY_floor+1) * width) + c];
                int b3 = data[num_channel * ((sourceX_floor+1) + sourceY_floor * width) + c];
                int b4 = data[num_channel * ((sourceX_floor+1) + (sourceY_floor+1) * width) + c];
                int tmp = static_cast<int>((1 - x_weight) * (1 - y_weight) * b1 + (1 - x_weight) * y_weight * b2 +
                            x_weight * (1 - y_weight) * b3 + x_weight * y_weight * b4);
                
                scaledData[num_channel * (x + y * new_width) + c] = static_cast<unsigned char>(tmp);
            }
        }
    }
    
    string filename = "output" + input_num + "_" + up_down + ".bmp";
    ofstream output(filename, ios::out | ios::binary);
    if (!output.is_open()) {
        std::cerr << "Error creating the output file" << std::endl;
        return;
    }

    infoHeader.width = new_width;
    infoHeader.height = new_height;
    infoHeader.imageSize = new_ImageSize;

    // Write the headers
    output.write(reinterpret_cast<const char*>(&header), sizeof(BMPHeader));
    output.write(reinterpret_cast<const char*>(&infoHeader), sizeof(BMPInfoHeader));

    // Write the scaled data
    output.write(reinterpret_cast<const char*>(scaledData.data()), new_ImageSize);

    // Close the output file
    output.close();
}
