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

#include "continuationlinemerger.h"

#include <utility>

#include <QByteArray>
#include <QIODevice>

#include "linetypes.h"

namespace {

bool writeAll( QIODevice& output, const QByteArray& data )
{
    qint64 offset = 0;
    while ( offset < data.size() ) {
        const auto written = output.write( data.constData() + offset, data.size() - offset );
        if ( written <= 0 ) {
            return false;
        }
        offset += written;
    }
    return true;
}

} // namespace

ContinuationLineMerger::ContinuationLineMerger( QRegularExpression prefixRegex )
    : prefixRegex_( std::move( prefixRegex ) )
{
}

bool ContinuationLineMerger::appendLine( const QString& line, QIODevice& output )
{
    const auto match = prefixRegex_.match( untabify( QString{ line } ) );
    const bool startsNewEntry = match.hasMatch() && match.lastCapturedIndex() >= 1;

    if ( hasInputLine_ && startsNewEntry && !writeAll( output, QByteArrayLiteral( "\n" ) ) ) {
        return false;
    }

    if ( !writeAll( output, line.toUtf8() ) ) {
        return false;
    }

    hasInputLine_ = true;
    return true;
}

bool ContinuationLineMerger::finish( QIODevice& output )
{
    return !hasInputLine_ || writeAll( output, QByteArrayLiteral( "\n" ) );
}
