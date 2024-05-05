//
// Created by matthew on 8/20/23.
//


#include <stdexcept>
#include <unordered_map>
#include <map>

#include "encodings.h"


Encoding* encodingFromName(const std::string& name) {
    Encoding* encodings[] = {new PlainEncoding(), new ShiftAllEncoding(), new ShiftCharEncoding()};
    std::string availEncodings;

    for (Encoding* enc : encodings) {
        if (name == enc->name())
            return enc;
        else {
            availEncodings += enc->name() + ", ";
            delete enc;
        }
    }

    throw std::runtime_error("Encoding '" + name + "' not found!  Available encodings are: " + availEncodings);
}


// PlainEncoding implementation
std::string PlainEncoding::name() { return "plain"; }

std::string PlainEncoding::decode(std::string encoded, const std::string& key) { return encoded; }

std::string PlainEncoding::encode(std::string raw, const std::string& key) { return raw; }

// ShiftAllEncoding implementation
std::string ShiftAllEncoding::name() { return "shiftall"; }

std::string ShiftAllEncoding::decode(std::string encoded, const std::string& key) {
    const int charMax = 128;
    std::string decoded;
    char keyHash = abs(static_cast<int>(std::hash<std::string>{}(key)) % charMax);

    for (char i : encoded)
        decoded += i - keyHash>0 ? i - keyHash : i - keyHash + charMax;
    return decoded;
}

std::string ShiftAllEncoding::encode(std::string raw, const std::string& key) {
    const int charMax = 128;
    std::string encoded;
    char keyHash = abs(static_cast<int>(std::hash<std::string>{}(key)) % charMax);

    for (char i : raw)
        encoded += i + keyHash<charMax ? i + keyHash : i + keyHash - charMax;
    return encoded;
}

// ShiftCharEncoding implementation
std::string ShiftCharEncoding::name() { return "shiftchar"; }

std::string ShiftCharEncoding::decode(std::string encoded, const std::string& key) {
    std::string decoded;
    ShiftAllEncoding subEncoder = ShiftAllEncoding();

    for (int i = 0; i<encoded.length(); i++) {
        std::string charKey = (!key.empty() ? key.substr(i % key.length(), 1) : "") + std::to_string(i);
        decoded += subEncoder.decode(encoded.substr(i, 1), charKey);
    }

    return decoded;
}

std::string ShiftCharEncoding::encode(std::string raw, const std::string& key) {
    std::string encoded;
    ShiftAllEncoding subEncoder = ShiftAllEncoding();

    for (int i = 0; i<raw.length(); i++) {
        std::string charKey = (!key.empty() ? key.substr(i % key.length(), 1) : "") + std::to_string(i);
        encoded += subEncoder.encode(raw.substr(i, 1), charKey);
    }

    return encoded;
}
