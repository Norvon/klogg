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

#ifndef CONTINUATIONLINEMERGER_H
#define CONTINUATIONLINEMERGER_H

#include <QRegularExpression>
#include <QString>

class QIODevice;

class ContinuationLineMerger {
  public:
    explicit ContinuationLineMerger( QRegularExpression prefixRegex );

    bool appendLine( const QString& line, QIODevice& output );
    bool finish( QIODevice& output );

  private:
    QRegularExpression prefixRegex_;
    bool hasInputLine_ = false;
};

#endif // CONTINUATIONLINEMERGER_H
