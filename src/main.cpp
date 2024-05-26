#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <Magick++.h>

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
        std::cerr << "Could not open file '" << txtPth << "'" << std::endl;
        exit(-1);
    }

    std::string line;
    while (getline(inTxtFile, line)) fileText += line + "\n";
    fileText.pop_back(); // Remove last newline character
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
 * Encodes the given text into the image and returns the modified output image
 * @param inputText The text to encode
 * @param image The image to encode the text into
 * @param bitWidth The number of bits to use for encoding within each channel (1, 2, or 4)
 * @param enc The encoding to use
 * @param key The key to encode with
 * @return The modified output image
 */
cv::Mat encodeCommand(const std::string& inputText, cv::Mat& image, int bitWidth, Encoding* enc, const std::string& key) {
    std::string b64Text = base64Encode(inputText);
    std::string hashEncText = enc->encode(b64Text, key);
    int overflow = (int) hashEncText.length() - (image.rows * image.cols);
    if (overflow > 0)
        std::cerr << "Warning: The last " << overflow << " characters of text will be truncated!" << std::endl;

    // Encode the text into the image
    cv::Mat outputImage = image.clone();
    encodeText(outputImage, hashEncText, bitWidth);

    return outputImage;
}


/**
 * Decodes the text from the image
 * @param image The image to decode the text from
 * @param bitWidth The number of bits to use for decoding within each channel (1, 2, or 4)
 * @param enc The encoding to use
 * @param key The key to decode with
 * @return The decoded text
 */
std::string decodeCommand(cv::Mat& image, int bitWidth, Encoding* enc, const std::string& key) {
    // Decode the text from the image
    std::string hashEncText = decodeText(image, bitWidth);
    std::string b64Text = enc->decode(hashEncText, key);
    std::string plainText = base64Decode(b64Text);

    return plainText;
}


/**
 * Gets the extension of the file at the given path
 * @param path The path to the file
 * @return The extension of the file
 */
std::string getExtension(const std::string& path) {
    size_t dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos) return "";
    return path.substr(dotPos + 1);
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

    Encoding* enc = encodingFromName(encoding);

    if (getExtension(inputImPth) == "gif" || getExtension(outputImPth) == "gif")
        Magick::InitializeMagick(*argv);

    std::vector<cv::Mat> inputImages;
    // Read in input image
    if (getExtension(inputImPth) != "gif") {
        cv::Mat image = imread(inputImPth, cv::IMREAD_UNCHANGED);
        if (image.empty()) {
            std::cerr << "Could not open or find the image '" << inputImPth << "'" << std::endl;
            return -1;
        }

        inputImages.push_back(image);
    }
    else {
        std::list<Magick::Image> inputFrames;
        Magick::readImages(&inputFrames, inputImPth);

        for (Magick::Image &image : inputFrames) {
            cv::Mat frame;

            int image_cols = image.columns();
            int image_rows = image.rows();

            // Check if image has an alpha channel
            if (image.matte()) {
                frame = cv::Mat(image_rows, image_cols, CV_8UC4);
                image.write(0, 0, image_cols, image_rows, "BGRA", Magick::CharPixel, frame.data);
            }
            else {
                frame = cv::Mat(image_rows, image_cols, CV_8UC3);
                image.write(0, 0, image_cols, image_rows, "BGR", Magick::CharPixel, frame.data);
            }

            image.magick("GIF");
            image.depth(8);
            image.type(Magick::TrueColorMatteType);
            image.quality(100);

            // Disable color quantization and dithering
            image.quantizeDither(false);
            image.quantizeColors(256); // Set to a high number to avoid color reduction

            std::cout << "Quantization info: " << image.quantizeInfo()->dither << ", " << image.quantizeInfo()->number_colors << std::endl;
            inputImages.push_back(frame);
        }
    }

    if (encode->parsed()) {
        std::string inputText = !txtPth.empty() ? textFromFile(txtPth) : textFromStdin();
        if (inputText.empty()) {
            std::cerr << "Error: No text to encode!" << std::endl;
            return -1;
        }

        int imageCapacity = inputImages[0].rows * inputImages[0].cols * (bitWidth == 1 ? 0.5 : (bitWidth == 2 ? 1 : 2));
        int stringPos = 0;
        std::string textSlice;
        std::vector<cv::Mat> outputImages;
        // Transform the input images into output images using the input text
        for (cv::Mat& image : inputImages) {
            textSlice = inputText.length() >= stringPos + imageCapacity ? inputText.substr(stringPos, imageCapacity) :
                    inputText.length() > stringPos ? inputText.substr(stringPos) : "";
            // Encode the text into the image, use the original image if no text to encode
            cv::Mat outputImage;
            if (!textSlice.empty())
                outputImage = encodeCommand(textSlice, image, bitWidth, enc, key);
            else {
                outputImage = image.clone();
                if (outputImage.channels() == 3) addAlphaChannel(outputImage);
            }

            outputImages.push_back(outputImage);
            stringPos += imageCapacity;
        }

        if (getExtension(outputImPth) != "gif") {
            imwrite(outputImPth, outputImages[0]);
        } else {
            std::list<Magick::Image> outputFrames;
            for (cv::Mat &image : outputImages) {
                Magick::Image frame(image.cols, image.rows, "BGRA", Magick::CharPixel, image.data);

                frame.magick("GIF");
                frame.depth(8);
                frame.type(Magick::TrueColorMatteType);
                frame.quality(100);

                // Disable color quantization and dithering
                frame.quantizeDither(false);
                frame.quantizeColors(256); // Set to a high number to avoid color reduction

                std::cout << "Quantization info: " << frame.quantizeInfo()->dither << ", " << frame.quantizeInfo()->number_colors << std::endl;

                outputFrames.push_back(frame);
            }
            for (int i = 0; i < 5; i++) {
                cv::Vec4b pixel = outputImages.front().at<cv::Vec4b>(i / outputImages.front().cols, i % outputImages.front().cols);
                std::cout << "Pixel " << i << ": " << (int) pixel[0] << " " << (int) pixel[1] << " " << (int) pixel[2] << " " << (int) pixel[3] << std::endl;
            }
            Magick::writeImages(outputFrames.begin(), outputFrames.end(), outputImPth);
        }
    } else {
        std::string plainText;
        for (int i = 0; i < 5; i++) {
            cv::Vec4b pixel = inputImages[0].at<cv::Vec4b>(i / inputImages[0].cols, i % inputImages[0].cols);
            std::cout << "Pixel " << i << ": " << (int) pixel[0] << " " << (int) pixel[1] << " " << (int) pixel[2] << " " << (int) pixel[3] << std::endl;
        }
        for (cv::Mat& image : inputImages)
            plainText += decodeCommand(image, bitWidth, enc, key);

        if (!txtPth.empty()) {
            std::ofstream outTxtFile = std::ofstream(txtPth);
            outTxtFile << plainText;
            outTxtFile.close();
        } else std::cout << plainText << std::endl;
    }

    delete enc;

    return 0;
}
