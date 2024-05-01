#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>

#include "CLI11/CLI11.hpp"
#include "image_encode.h"
#include "encodings.h"
#include "base64.h"


std::string readInText(const std::string& txtPth) {
    std::string fileText;
    std::ifstream inTxtFile(txtPth);
    if (!inTxtFile.is_open()) {
        std::cout << "Could not open file" << std::endl;
        exit(-1);
    }
    std::string line;
    while (getline(inTxtFile, line)) fileText += line + "\n";
    fileText.pop_back(); // Remove last newline character
    inTxtFile.close();

    return fileText;
}


void encodeCommand(const std::string& inputText, cv::Mat& image, const std::string& outputImPth, int bitWidth, Encoding *enc, const std::string& key) {
    std::string b64Text = base64Encode(inputText);
    std::string hashEncText = enc->encode(b64Text, key);
    int overflow = (int) hashEncText.length() - (image.rows * image.cols);
    if (overflow > 0)
        std::cout << "Warning: The last " << overflow << " characters of text will be truncated!" << std::endl;

    // Encode the text into the image
    cv::Mat outputImage = image.clone();
    encodeText(outputImage, hashEncText, bitWidth);
    // Write the image
    imwrite(outputImPth, outputImage);
}

void decodeCommand(cv::Mat& image, const std::string& outputTxtPth, int bitWidth, Encoding *enc, const std::string& key) {
    // Decode the text from the image
    std::string hashEncText = decodeText(image, bitWidth);
    std::string b64Text = enc->decode(hashEncText, key);
    std::string plainText = base64Decode(b64Text);
    std::ofstream outTxtFile = std::ofstream(outputTxtPth);
    outTxtFile << plainText;
    outTxtFile.close();
}


int main(int argc, char** argv) {

    CLI::App app{"Image-Based document encoder-decoder", "icrypt"};
    app.require_subcommand(1);

    std::string txtPth;
    std::string inputImPth;
    std::string outputImPth;
    int bitWidth = 1;
    std::string encoding = "plain";
    std::string keyPth;


    app.add_option("-e, --encoding", encoding, "The encoding to use (plain, shiftall, shiftchar)")->default_val("plain");
    app.add_option("-k, --key", keyPth, "The key file to use for encoding/decoding, if applicable")->default_val("");
    app.add_option("-b, --bit-width", bitWidth, "The number of bits to use for encoding within each channel (1, 2, or 4)")->default_val(1);

    CLI::App *encode = app.add_subcommand("encode", "Encode text into an image");
    encode->fallthrough();
    encode->add_option("input-image", inputImPth, "The input image to encode the text into")->required();
    encode->add_option("text-file", txtPth, "The text file to encode")->required();
    encode->add_option("-o,--output-image", outputImPth, "The output image to write the text to")->required();

    CLI::App *decode = app.add_subcommand("decode", "Decode text from an image");
    decode->fallthrough();
    decode->add_option("input-image", inputImPth, "The input image to decode the text from")->required();
    decode->add_option("-o,--output-text", txtPth, "The text file to write the decoded text to")->required();

    try {
        app.parse(argc, argv);
        if (!encode->parsed() && !decode->parsed())
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
    if (encode->parsed() && outputImPth.find('.') == std::string::npos) {
        std::cout << "Error: Output image path must have an extension" << std::endl;
        return -1;
    }

    std::string key;
    if (encoding != "plain" && !keyPth.empty()) key = readInText(keyPth);

    Encoding *enc = encodingFromName(encoding);

    // Read in input image
    cv::Mat image = imread(inputImPth, cv::IMREAD_UNCHANGED);
    if (image.empty()) {
        std::cout << "Could not open or find the image" << std::endl;
        return -1;
    }

    if (encode->parsed()) {
        std::string inputText = readInText(txtPth);
        encodeCommand(inputText, image, outputImPth, bitWidth, enc, key);
    }
    else decodeCommand(image, txtPth, bitWidth, enc, key);

    delete enc;

    return 0;
}
