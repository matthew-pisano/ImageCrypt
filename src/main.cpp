#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>

#include "CLI11/CLI11.hpp"
#include "image_encode.h"
#include "encodings.h"
#include "base64.h"


/**
 * Reads in the text from a file
 * @param txtPth The path to the text file
 * @return The text content from the file
 */
std::string textFromFile(const std::string& txtPth) {
    std::string fileText;
    std::ifstream inTxtFile(txtPth);
    if (!inTxtFile.is_open()) {
        std::cerr << "Could not open file" << std::endl;
        exit(-1);
    }

    std::string line;
    while (getline(inTxtFile, line)) fileText += line + "\n";
    if (!fileText.empty()) fileText.pop_back(); // Remove last newline character
    inTxtFile.close();

    return fileText;
}


/**
 * Reads in the text from stdin
 * @return The text content from stdin
 */
std::string textFromStdin() {
    std::string text;
    std::string line;
    while (std::getline(std::cin, line)) text += line + "\n";
    text.pop_back(); // Remove last newline character
    return text;
}


/**
 * Encodes the given text into the image
 * @param inputText The text to encode
 * @param image The image to encode the text into
 * @param outputImPth The path to write the output image to
 * @param bitWidth The number of bits to use for encoding within each channel (1, 2, or 4)
 * @param enc The encoding to use
 * @param key The key to encode with
 */
void encodeCommand(const std::string& inputText, const cv::Mat& image, const std::string& outputImPth, const int bitWidth, Encoding* enc, const std::string& key) {
    const std::string b64Text = base64Encode(inputText);
    const std::string hashEncText = enc->encode(b64Text, key);
    const int overflow = static_cast<int>(hashEncText.length()) - (image.rows * image.cols);
    if (overflow > 0)
        std::cerr << "Warning: The last " << overflow << " characters of text will be truncated!" << std::endl;

    // Encode the text into the image
    cv::Mat outputImage = image.clone();
    encodeText(outputImage, hashEncText, bitWidth);
    // Write the image
    imwrite(outputImPth, outputImage);
}


/**
 * Decodes the text from the image
 * @param image The image to decode the text from
 * @param outputTxtPth The path to write the output text to.  If empty, the text will be printed to the console
 * @param bitWidth The number of bits to use for decoding within each channel (1, 2, or 4)
 * @param enc The encoding to use
 * @param key The key to decode with
 */
void decodeCommand(cv::Mat& image, const std::string& outputTxtPth, const int bitWidth, Encoding* enc, const std::string& key) {
    // Decode the text from the image
    std::string hashEncText = decodeText(image, bitWidth);
    std::string b64Text = enc->decode(hashEncText, key);
    std::string plainText = base64Decode(b64Text);

    if (!outputTxtPth.empty()) {
        std::ofstream outTxtFile = std::ofstream(outputTxtPth);
        outTxtFile << plainText;
        outTxtFile.close();
    } else std::cout << plainText << std::endl;
}


int main(const int argc, char** argv) {

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

    CLI::App* encode = app.add_subcommand("encode", "Encode text into an image");
    encode->fallthrough();
    encode->add_option("input-image", inputImPth, "The input image to encode the text into")->required();
    encode->add_option("text-file", txtPth, "The text file to encode.  If omitted, text will be read from stdin")->default_val("");
    encode->add_option("-o,--output-image", outputImPth, "The output image to write the text to")->required();

    CLI::App* decode = app.add_subcommand("decode", "Decode text from an image");
    decode->fallthrough();
    decode->add_option("input-image", inputImPth, "The input image to decode the text from")->required();
    decode->add_option("-o,--output-text", txtPth, "The text file to write the decoded text to.  If omitted, text will be sent to stdout")->default_val("");

    try {
        app.parse(argc, argv);
        if (!encode->parsed() && !decode->parsed())
            std::cout << app.help() << std::endl;
    } catch (const CLI::ParseError& e) { return app.exit(e); }
    catch (const std::runtime_error& e) {
        const CLI::Error cli("Runtime Error", e.what());
        return app.exit(cli);
    }

    if (bitWidth != 1 && bitWidth != 2 && bitWidth != 4) {
        std::cerr << "Error: Bit width must be 1, 2, or 4" << std::endl;
        return -1;
    }
    if (encode->parsed() && outputImPth.find('.') == std::string::npos) {
        std::cerr << "Error: Output image path must have an extension" << std::endl;
        return -1;
    }

    std::string key;
    if (encoding != "plain" && !keyPth.empty()) key = textFromFile(keyPth);
    if (key.empty()) {
        std::cerr << "Warning: Key file is empty!  If you would like a blank key, omit the key file." << std::endl;
        return -1;
    }

    Encoding* enc = encodingFromName(encoding);

    // Read in input image
    cv::Mat image = imread(inputImPth, cv::IMREAD_UNCHANGED);
    if (image.empty()) {
        std::cerr << "Could not open or find the image" << std::endl;
        return -1;
    }

    if (encode->parsed()) {
        const std::string inputText = !txtPth.empty() ? textFromFile(txtPth) : textFromStdin();
        encodeCommand(inputText, image, outputImPth, bitWidth, enc, key);
    } else decodeCommand(image, txtPth, bitWidth, enc, key);

    delete enc;  // Clean up encoding object

    return 0;
}
