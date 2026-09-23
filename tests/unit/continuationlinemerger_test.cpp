/*
 * Copyright (C) 2016 -- 2026 Anton Filimonov and other contributors
 *
 * This file is part of klogg.
 *
 * klogg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <catch2/catch.hpp>

#include <vector>

#include <QBuffer>
#include <QStringList>

#include "continuationlinemerger.h"

namespace {

QByteArray mergeLines( const std::vector<QStringList>& chunks )
{
    QByteArray result;
    QBuffer output{ &result };
    REQUIRE( output.open( QIODevice::WriteOnly ) );

    ContinuationLineMerger merger{ QRegularExpression{ R"(^(\d{2}:\d{2}:\d{2}))" } };
    for ( const auto& chunk : chunks ) {
        for ( const auto& line : chunk ) {
            REQUIRE( merger.appendLine( line, output ) );
        }
    }
    REQUIRE( merger.finish( output ) );

    return result;
}

} // namespace

TEST_CASE( "Continuation lines are merged into the preceding timestamped entry",
           "[continuationlinemerger]" )
{
    const auto result = mergeLines( { { "10:00:00 first", "  detail", "tail" },
                                      { "10:00:01 second", "\tmore" } } );

    REQUIRE( result == "10:00:00 first  detailtail\n10:00:01 second\tmore\n" );
}

TEST_CASE( "Continuation merging is stable across chunks", "[continuationlinemerger]" )
{
    const auto result
        = mergeLines( { { "10:00:00 first" }, { " continuation" }, { "10:00:01 second" } } );

    REQUIRE( result == "10:00:00 first continuation\n10:00:01 second\n" );
}

TEST_CASE( "An empty continuation line does not create a visual line",
           "[continuationlinemerger]" )
{
    const auto result = mergeLines( { { "10:00:00 first", "", " continuation" } } );

    REQUIRE( result == "10:00:00 first continuation\n" );
}
