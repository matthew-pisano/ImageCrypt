//
// Created by matthew on 1/14/25.
//


#include <catch2/catch_test_macros.hpp>

#include "base64.h"


TEST_CASE("Test Base64 Encode") {
    SECTION("Test Empty String") {
        REQUIRE( base64Encode("").empty() );
    }

    SECTION("Test Aligned String") {
        REQUIRE( base64Encode("hellow") == "aGVsbG93" );
        REQUIRE( base64Encode("halting problem") == "aGFsdGluZyBwcm9ibGVt" );
        REQUIRE( base64Encode("That's no moon.  It's a space station!!") ==
            "VGhhdCdzIG5vIG1vb24uICBJdCdzIGEgc3BhY2Ugc3RhdGlvbiEh" );
        REQUIRE( base64Encode("\x1B\x54\x32\xFE\x88\x10\x34\x6F\x54") ==
            "G1Qy/ogQNG9U" );
    }

    SECTION("Test Padded String") {
        REQUIRE( base64Encode("hello") == "aGVsbG8=" );
        REQUIRE( base64Encode("haltingproble") == "aGFsdGluZ3Byb2JsZQ==" );
        REQUIRE( base64Encode("That's no moon.  It's a space station!") ==
            "VGhhdCdzIG5vIG1vb24uICBJdCdzIGEgc3BhY2Ugc3RhdGlvbiE=" );
        REQUIRE( base64Encode("\x1B\x54\x32\xFE\x88\x10\x34\x6F\x54\x69") ==
            "G1Qy/ogQNG9UaQ==" );
    }
}