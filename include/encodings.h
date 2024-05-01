//
// Created by matthew on 8/20/23.
//

#ifndef ICRYPT_ENCODINGS_H
#define ICRYPT_ENCODINGS_H

#include <string>


/**
 * An abstract encoding specification
 */
struct Encoding {

    virtual ~Encoding() = default;

    /**
     * The name identifier of the encoding
     * @return The encoding name
     */
    virtual std::string name() = 0;

    /**
     * Decodes a std::string using the given key and returns the result
     * @param encoded The original, encoded std::string
     * @param key The key to decode with
     * @return The decoded std::string
     */
    virtual std::string decode(std::string encoded, const std::string& key) = 0;

    /**
     * Encodes a std::string using the given key and returns the result
     * @param raw The original std::string
     * @param key The key to encode with
     * @return The encoded std::string
     */
    virtual std::string encode(std::string raw, const std::string& key) = 0;
};


/**
 * A plaintext encoding
 */
struct PlainEncoding : public Encoding {

    std::string name() override;

    std::string decode(std::string encoded, const std::string& key) override;

    std::string encode(std::string raw, const std::string& key) override;
};


/**
 * A shiftall encoding.  Encoded text is shifted up in characters uniformly based off of the key hash
 */
struct ShiftAllEncoding : public Encoding {

    std::string name() override;

    std::string decode(std::string encoded, const std::string& key) override;

    std::string encode(std::string raw, const std::string& key) override;
};


/**
 * A shiftchar encoding.  Encoded text is shifted up character by character based off of the hash of the key and the character index
 */
struct ShiftCharEncoding : public Encoding {

    std::string name() override;

    std::string decode(std::string encoded, const std::string& key) override;

    std::string encode(std::string raw, const std::string& key) override;
};


/**
 * Gets a pointer to an encoding object based off of the name of the encoding
 * @param name The name of the encoding to get
 * @return A pointer to an encoding instance
 */
Encoding* encodingFromName(const std::string& name);

#endif //ICRYPT_ENCODINGS_H
