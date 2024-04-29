#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>

#include "CLI11/CLI11.hpp"
#include "image_encode.h"


std::string readInText(const std::string& txtPth) {
    std::string decodedText;
    std::ifstream inTxtFile(txtPth);
    if (!inTxtFile.is_open()) {
        std::cout << "Could not open file" << std::endl;
        exit(-1);
    }
    std::string line;
    while (getline(inTxtFile, line)) decodedText += line + "\n";
    decodedText.pop_back(); // Remove last newline character
    inTxtFile.close();

    return decodedText;
}


int main(int argc, char** argv) {

    CLI::App app{"Image Text Encryptor Encoder", "icrypt"};

    std::string txtPth;
    std::string inputImPth;
    std::string outputImPth;
    int bitWidth = 1;

    CLI::App *encrypt = app.add_subcommand("encrypt", "Encrypt text into an image");
    encrypt->add_option("-t,--text-file", txtPth, "The text file to encrypt")->required();
    encrypt->add_option("-i,--input-image", inputImPth, "The input image to encrypt the text into")->required();
    encrypt->add_option("-o,--output-image", outputImPth, "The output image to write the encrypted text to")->required();
    encrypt->add_option("-b,--bit-width", bitWidth, "The number of bits to use for encoding within each channel (1, 2, or 4)")->default_val(1);

    CLI::App *decrypt = app.add_subcommand("decrypt", "Decrypt text from an image");
    decrypt->add_option("-i,--input-image", inputImPth, "The input image to decrypt the text from")->required();
    decrypt->add_option("-o,--output-text", txtPth, "The text file to write the decrypted text to")->required();
    decrypt->add_option("-b,--bit-width", bitWidth, "The number of bits used for encoding within each channel (1, 2, or 4)")->default_val(1);

    try {
        app.parse(argc, argv);
        if (!encrypt->parsed() && !decrypt->parsed())
            std::cout << app.help() << std::endl;
    } catch (const CLI::ParseError &e) { return app.exit(e); }
    catch (const std::runtime_error &e) {
        const CLI::Error cli("Runtime Error", e.what());
        return app.exit(cli);
    }

    if (bitWidth != 1 && bitWidth != 2 && bitWidth != 4) {
        std::cout << "Error: Bit width must be 1, 2, or 4" << std::endl;
        return -1;
    }
    if (encrypt->parsed() && outputImPth.find('.') == std::string::npos) {
        std::cout << "Error: Output image path must have an extension" << std::endl;
        return -1;
    }

    std::string decodedText;

    std::ofstream outTxtFile;
    // Read in text file
    if (encrypt->parsed()) decodedText = readInText(txtPth);
    // Open stream for output text file
    else outTxtFile = std::ofstream(txtPth);

    // Read in input image
    cv::Mat image = imread(inputImPth, cv::IMREAD_UNCHANGED);
    if (image.empty()) {
        std::cout << "Could not open or find the image" << std::endl;
        return -1;
    }

    if (encrypt->parsed()) {
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
