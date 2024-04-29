#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>

#include "image_encode.h"


int main(int argc, char** argv) {

    if (argc < 4 || argc > 6) {
        std::cout << "Usage: icrypt <encrypt|decrypt> <TEXT_FILE> <INPUT_IMAGE> [OUTPUT_IMAGE] [BIT_WIDTH]" << std::endl;
        return -1;
    }
    
    std::string mode = argv[1];
    if (mode == "encrypt" && argc < 5) {
        std::cout << "Usage: icrypt encrypt <TEXT_FILE> <INPUT_IMAGE> <OUTPUT_IMAGE> [BIT_WIDTH]" << std::endl;
        return -1;
    }
    else if (mode == "decrypt" && argc > 5) {
        std::cout << "Usage: icrypt decrypt <TEXT_FILE> <INPUT_IMAGE> [BIT_WIDTH]" << std::endl;
        return -1;
    }
    else if (mode != "encrypt" && mode != "decrypt") {
        std::cout << "Usage: icrypt <encrypt|decrypt> <TEXT_FILE> <INPUT_IMAGE> <OUTPUT_IMAGE> [BIT_WIDTH]" << std::endl;
        return -1;
    }

    // Read in arguments
    std::string txtPth = argv[2];
    std::string inputImPth = argv[3];
    std::string outputImPth;
    if (mode == "encrypt") outputImPth = argv[4];
    int bitWidth = 1;
    if (mode == "encrypt" && argc == 6) bitWidth = std::stoi(argv[5]);
    else if (mode == "decrypt" && argc == 5) bitWidth = std::stoi(argv[4]);

    if (bitWidth != 1 && bitWidth != 2 && bitWidth != 4) {
        std::cout << "Error: Bit width must be 1, 2, or 4" << std::endl;
        return -1;
    }
    if (mode == "encrypt" && outputImPth.find('.') == std::string::npos) {
        std::cout << "Error: Output image path must have an extension" << std::endl;
        return -1;
    }

    std::string decodedText;

    std::ifstream inTxtFile;
    std::ofstream outTxtFile;
    // Read in text file
    if (mode == "encrypt") {
        inTxtFile = std::ifstream(txtPth);
        if (!inTxtFile.is_open()) {
            std::cout << "Could not open file" << std::endl;
            return -1;
        }
        std::string line;
        while (getline(inTxtFile, line)) decodedText += line + "\n";
        decodedText.pop_back(); // Remove last newline character
        inTxtFile.close();
    }
    // Open stream for output text file
    else outTxtFile = std::ofstream(txtPth);

    // Read in input image
    cv::Mat image = imread(inputImPth, cv::IMREAD_UNCHANGED);
    if (image.empty()) {
        std::cout << "Could not open or find the image" << std::endl;
        return -1;
    }

    if (mode == "encrypt") {
        int overflow = decodedText.length() - (image.rows * image.cols);
        if (overflow > 0)
            std::cout << "Warning: The last " << overflow << "character of text will be truncated!" << std::endl;

        // Encode the text into the image
        cv::Mat outputImage = image.clone();
        encodeText(outputImage, decodedText, bitWidth);
        // Write the image
        imwrite(outputImPth, outputImage);
    }
    else {
        // Decode the text from the image
        decodedText = decodeText(image, bitWidth);
        outTxtFile << decodedText;
        outTxtFile.close();
    }

    return 0;
}
