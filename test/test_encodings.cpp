//
// Created by matthew on 1/15/25.
//


#include <catch2/catch_test_macros.hpp>

#include "encodings.h"


TEST_CASE("Test Get Encoding from Name") {
    REQUIRE( encodingFromName("plain")->name() == "plain" );
    REQUIRE( encodingFromName("shiftall")->name() == "shiftall" );
    REQUIRE( encodingFromName("shiftchar")->name() == "shiftchar" );

    REQUIRE_THROWS_AS( encodingFromName("notanencoding"), std::runtime_error );
}


TEST_CASE("Test Plain") {
    Encoding* enc = encodingFromName("plain");

    SECTION("Test Encoding") {
        REQUIRE( enc->encode("", "42").empty() );
        REQUIRE( enc->encode("hello there", "42") == "hello there" );
    }

    SECTION("Test Decoding") {
        REQUIRE( enc->decode("", "42").empty() );
        REQUIRE( enc->decode("hello there", "42") == "hello there" );
        REQUIRE( enc->decode("hello there", "bad") == "hello there" );
    }
}


TEST_CASE("Test ShiftAll") {
    Encoding* enc = encodingFromName("shiftall");

    SECTION("Test Encoding") {
        REQUIRE( enc->encode("", "42").empty() );
        REQUIRE( enc->encode("hello there", "42") == "\x19\x16\x1D\x1D Q%\x19\x16#\x16" );
    }

    SECTION("Test Decoding") {
        REQUIRE( enc->decode("", "42").empty() );
        REQUIRE( enc->decode("\x19\x16\x1D\x1D Q%\x19\x16#\x16", "42") == "hello there" );
        REQUIRE_FALSE( enc->decode("\x19\x16\x1D\x1D Q%\x19\x16#\x16", "bad") == "hello there" );
    }
}


TEST_CASE("Test ShiftChar") {
    Encoding* enc = encodingFromName("shiftchar");

    SECTION("Test Encoding") {
        REQUIRE( enc->encode("", "42").empty() );
        REQUIRE( enc->encode("hello there", "42") == "\024(\035>>\vU\031k=L" );
    }

    SECTION("Test Decoding") {
        REQUIRE( enc->decode("", "42").empty() );
        REQUIRE( enc->decode("\024(\035>>\vU\031k=L", "42") == "hello there" );
        REQUIRE_FALSE( enc->decode("\024(\035>>\vU\031k=L", "bad") == "hello there" );
    }
}