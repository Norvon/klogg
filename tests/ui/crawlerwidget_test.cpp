/*
 * Copyright (C) 2016 -- 2019 Anton Filimonov and other contributors
 *
 * This file is part of klogg.
 *
 * klogg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * klogg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with klogg.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <catch2/catch.hpp>

#include <QSignalSpy>
#include <QScrollBar>
#include <QTemporaryFile>
#include <QTest>
#include <QTimer>
#include <QToolButton>
#include <qglobal.h>
#include <qnamespace.h>
#include <qtestmouse.h>

#include "savedsearches.h"
#include "session.h"
#include "test_utils.h"

#include "logdata.h"
#include "logfiltereddata.h"

#include "crawlerwidget.h"

static const qint64 SL_NB_LINES = 100LL;

namespace {
bool generateDataFiles( QTemporaryFile& file )
{
    char newLine[ 90 ];

    if ( file.open() ) {
        for ( int i = 0; i < SL_NB_LINES; i++ ) {
            snprintf( newLine, 89,
                      "LOGDATA \t is a part of glogg, we are going to test it thoroughly, this is "
                      "line %06d",
                      i );
            file.write( newLine, static_cast<qint64>( qstrlen( newLine ) ) );
#ifdef Q_OS_WIN
            file.write( "\r\n", 2 );
#else
            file.write( "\n", 1 );
#endif
        }
        file.flush();
    }

    return true;
}

} // namespace

struct CrawlerWidgetPrivate {
};

template <>
struct CrawlerWidget::access_by<CrawlerWidgetPrivate> {
    std::unique_ptr<CrawlerWidget> crawler;

    bool isLoadingFinished()
    {
        return !crawler->loadingInProgress_;
    }

    LinesCount getLogNbLines()
    {
        return crawler->logData_->getNbLine();
    }

    LinesCount getLogFilteredNbLines()
    {
        return crawler->logFilteredData_->getNbLine();
    }

    LinesCount getMarksCount()
    {
        return crawler->logFilteredData_->getNbMarks();
    }

    void selectAllInMainView()
    {
        crawler->logMainView_->selectAll();
    }

    void selectAllInFilteredView()
    {
        crawler->filteredView_->selectAll();
    }

    QString mainViewSelectedText()
    {
        return crawler->logMainView_->getSelectedText();
    }

    QString filteredViewSelectedText()
    {
        return crawler->filteredView_->getSelectedText();
    }

    void setSearchPattern( const QString& pattern )
    {
        QTest::keyClicks( crawler->searchLineEdit_, pattern );
    }

    void replaceSearchPattern( const QString& pattern )
    {
        crawler->searchLineEdit_->setEditText( pattern );
        crawler->updatePredefinedFiltersWidget();
    }

    void enableCaseSensitiveSearch()
    {
        if ( !crawler->matchCaseButton_->isChecked() ) {
            QTest::mouseClick( crawler->matchCaseButton_, Qt::LeftButton );
            QTest::qWait( 100 );
        }
    }

    void enableInverseMatch()
    {
        if ( !crawler->inverseButton_->isChecked() ) {
            QTest::mouseClick( crawler->inverseButton_, Qt::LeftButton );
            QTest::qWait( 100 );
        }
    }

    void enableBooleanCombinationMode()
    {
        if ( !crawler->booleanButton_->isChecked() ) {
            QTest::mouseClick( crawler->booleanButton_, Qt::LeftButton );
            QTest::qWait( 100 );
        }
    }

    void runSearch()
    {
        QTest::mouseClick( crawler->searchButton_, Qt::LeftButton );

        QTest::qWait( 100 );

        waitUiState( [ & ]() { return crawler->stopButton_->isHidden(); } );
    }

    void markCurrentSearchResults()
    {
        REQUIRE( crawler->markSearchResultsButton_->isEnabled() );

        QTest::mouseClick( crawler->markSearchResultsButton_, Qt::LeftButton );
    }

    void keepNextSearchResults()
    {
        if ( !crawler->keepSearchResultsButton_->isChecked() ) {
            QTest::mouseClick( crawler->keepSearchResultsButton_, Qt::LeftButton );
        }
    }

    int filteredTabCount() const
    {
        return crawler->tabbedFilteredView_->count();
    }

    void switchFilteredTab( int tabIndex )
    {
        crawler->tabbedFilteredView_->setCurrentIndex( tabIndex );
        QTest::qWait( 50 );
    }

    FilteredView* filteredTab( int tabIndex ) const
    {
        return qobject_cast<FilteredView*>( crawler->tabbedFilteredView_->widget( tabIndex ) );
    }

    LinesCount marksCountForFilteredTab( int tabIndex ) const
    {
        auto* tabView = filteredTab( tabIndex );
        REQUIRE( tabView != nullptr );

        return crawler->filteredViewsData_.at( tabView )->getNbMarks();
    }

    void queuePendingAutoMarkForFilteredTab( int tabIndex, const QString& searchText )
    {
        auto* tabView = filteredTab( tabIndex );
        REQUIRE( tabView != nullptr );

        crawler->pendingAutoMarkSearches_[ tabView ] = searchText;
    }

    void completeSearchForFilteredTab( int tabIndex, LinesCount nbMatches )
    {
        auto* tabView = filteredTab( tabIndex );
        REQUIRE( tabView != nullptr );

        auto filteredData = crawler->filteredViewsData_.at( tabView );
        crawler->updateFilteredViewForData( tabView, filteredData.get(), nbMatches, 100,
                                            crawler->searchStartLine_ );
    }

    QToolButton* frequentSearchButton( int valueIndex ) const
    {
        auto currentValueIndex = 0;
        for ( auto itemIndex = 0; itemIndex < crawler->frequentSearchesLayout_->count();
              ++itemIndex ) {
            auto* widget = crawler->frequentSearchesLayout_->itemAt( itemIndex )->widget();
            auto* button = widget ? widget->findChild<QToolButton*>() : nullptr;
            if ( button ) {
                if ( currentValueIndex == valueIndex ) {
                    return button;
                }
                ++currentValueIndex;
            }
        }

        return nullptr;
    }

    QToolButton* frequentSearchButtonByTooltip( const QString& tooltip ) const
    {
        for ( auto itemIndex = 0; itemIndex < crawler->frequentSearchesLayout_->count();
              ++itemIndex ) {
            auto* widget = crawler->frequentSearchesLayout_->itemAt( itemIndex )->widget();
            auto* button = widget ? widget->findChild<QToolButton*>() : nullptr;
            if ( button && button->toolTip() == tooltip ) {
                return button;
            }
        }

        return nullptr;
    }

    int frequentSearchButtonCount() const
    {
        auto count = 0;
        for ( auto itemIndex = 0; itemIndex < crawler->frequentSearchesLayout_->count();
              ++itemIndex ) {
            auto* widget = crawler->frequentSearchesLayout_->itemAt( itemIndex )->widget();
            auto* button = widget ? widget->findChild<QToolButton*>() : nullptr;
            if ( button ) {
                ++count;
            }
        }

        return count;
    }

    QString frequentSearchText( int valueIndex ) const
    {
        const auto* button = frequentSearchButton( valueIndex );
        return button ? button->text() : QString{};
    }

    QString frequentSearchTooltip( int valueIndex ) const
    {
        const auto* button = frequentSearchButton( valueIndex );
        return button ? button->toolTip() : QString{};
    }

    int frequentSearchMaxWidth() const
    {
        return crawler->visibilityBox_->sizeHint().width();
    }

    void clickFrequentSearch( int valueIndex )
    {
        auto* button = frequentSearchButton( valueIndex );
        REQUIRE( button != nullptr );

        QTest::mouseClick( button, Qt::LeftButton );
        waitUiState( [ & ]() { return crawler->stopButton_->isHidden(); } );
    }

    QToolButton* autoMarkedSearchButton( int valueIndex ) const
    {
        auto currentValueIndex = 0;
        for ( auto itemIndex = 0; itemIndex < crawler->autoMarkedSearchesLayout_->count();
              ++itemIndex ) {
            auto* widget = crawler->autoMarkedSearchesLayout_->itemAt( itemIndex )->widget();
            auto* button = widget ? widget->findChild<QToolButton*>() : nullptr;
            if ( button ) {
                if ( currentValueIndex == valueIndex ) {
                    return button;
                }
                ++currentValueIndex;
            }
        }

        return nullptr;
    }

    int autoMarkedSearchButtonCount() const
    {
        auto count = 0;
        for ( auto itemIndex = 0; itemIndex < crawler->autoMarkedSearchesLayout_->count();
              ++itemIndex ) {
            auto* widget = crawler->autoMarkedSearchesLayout_->itemAt( itemIndex )->widget();
            auto* button = widget ? widget->findChild<QToolButton*>() : nullptr;
            if ( button ) {
                ++count;
            }
        }

        return count;
    }

    QString autoMarkedSearchTooltip( int valueIndex ) const
    {
        const auto* button = autoMarkedSearchButton( valueIndex );
        return button ? button->toolTip() : QString{};
    }

    void removeAutoMarkedSearch( int valueIndex )
    {
        const auto* button = autoMarkedSearchButton( valueIndex );
        REQUIRE( button != nullptr );

        crawler->removeAutoMarkedSearch( button->toolTip() );
    }

    void render()
    {
        crawler->grab();
    }

    int filteredVerticalScrollMaximum() const
    {
        return crawler->filteredView_->verticalScrollBar()->maximum();
    }

    LineNumber filteredTopLine() const
    {
        return crawler->filteredView_->getTopLine();
    }

    void scrollFilteredView( int yDelta )
    {
        auto* viewport = crawler->filteredView_->viewport();
        const auto position = viewport->rect().center();
        QWheelEvent wheelEvent( position, viewport->mapToGlobal( position ), QPoint(),
                                QPoint( 0, yDelta ), Qt::NoButton, Qt::NoModifier,
                                Qt::NoScrollPhase, false );
        QApplication::sendEvent( viewport, &wheelEvent );
    }

    void scrollFilteredViewDown()
    {
        scrollFilteredView( -120 );
    }

    void scrollFilteredViewUp()
    {
        scrollFilteredView( 120 );
    }

    void scrollFilteredViewToBottom()
    {
        auto* scrollBar = crawler->filteredView_->verticalScrollBar();
        scrollBar->setValue( scrollBar->maximum() );
    }

    void enableGlobalFollowMode()
    {
        Q_EMIT crawler->followSet( true );
    }
};

using CrawlerWidgetVisitor = CrawlerWidget::access_by<CrawlerWidgetPrivate>;

SCENARIO( "Crawler widget search", "[ui]" )
{
    QTemporaryFile file{ "crawler_test_XXXXXX" };
    REQUIRE( generateDataFiles( file ) );

    Session session;
    session.savedSearches().clear();

    REQUIRE( session.savedSearches().recentSearches().empty() );

    CrawlerWidgetVisitor crawlerVisitor;
    crawlerVisitor.crawler.reset( static_cast<CrawlerWidget*>(
        session.open( file.fileName(), []() { return new CrawlerWidget(); } ) ) );

    waitUiState( [ & ]() { return crawlerVisitor.getLogNbLines().get() == SL_NB_LINES; } );
    waitUiState( [ & ]() { return crawlerVisitor.isLoadingFinished(); } );

    crawlerVisitor.render();

    REQUIRE( crawlerVisitor.getLogNbLines().get() == SL_NB_LINES );

    GIVEN( "loaded log data" )
    {
        THEN( "Has no lines in log view" )
        {
            REQUIRE( crawlerVisitor.getLogFilteredNbLines().get() == 0 );
        }

        WHEN( "search for lines" )
        {
            crawlerVisitor.setSearchPattern( "this is line" );
            crawlerVisitor.runSearch();

            REQUIRE( waitUiState( [ &crawlerVisitor ]() {
                return crawlerVisitor.getLogFilteredNbLines().get() == SL_NB_LINES;
            } ) );

            THEN( "all lines are matched" )
            {
                REQUIRE( crawlerVisitor.getLogFilteredNbLines().get() == SL_NB_LINES );
            }

            AND_WHEN( "mark current search results" )
            {
                crawlerVisitor.markCurrentSearchResults();

                THEN( "all matched lines are marked" )
                {
                    REQUIRE( crawlerVisitor.getMarksCount().get() == SL_NB_LINES );
                    REQUIRE( waitUiState( [ &crawlerVisitor ]() {
                        return crawlerVisitor.autoMarkedSearchButtonCount() == 1;
                    } ) );
                    REQUIRE( crawlerVisitor.autoMarkedSearchTooltip( 0 ) == "this is line" );
                }
            }

            AND_WHEN( "copy all from main view" )
            {
                crawlerVisitor.selectAllInMainView();
                auto text = crawlerVisitor.mainViewSelectedText();
                THEN( "text has same number of lines" )
                {
                    REQUIRE( text.split( QChar::LineFeed ).size() == SL_NB_LINES );
                }
            }

            AND_WHEN( "copy all from filtered view" )
            {
                crawlerVisitor.selectAllInFilteredView();
                auto text = crawlerVisitor.filteredViewSelectedText();
                THEN( "text has same number of lines" )
                {
                    REQUIRE( text.split( QChar::LineFeed ).size() == SL_NB_LINES );
                }
            }

            AND_WHEN( "scroll filtered view" )
            {
                crawlerVisitor.crawler->resize( 1200, 600 );
                crawlerVisitor.crawler->show();
                REQUIRE( QTest::qWaitForWindowExposed( crawlerVisitor.crawler.get() ) );

                const auto initialTopLine = crawlerVisitor.filteredTopLine();
                crawlerVisitor.scrollFilteredViewDown();

                THEN( "filtered view scrolls" )
                {
                    REQUIRE( crawlerVisitor.filteredVerticalScrollMaximum() > 0 );
                    REQUIRE( crawlerVisitor.filteredTopLine() > initialTopLine );
                }
            }

            AND_WHEN( "global follow mode is enabled after filtered view reaches the bottom" )
            {
                crawlerVisitor.crawler->resize( 1200, 600 );
                crawlerVisitor.crawler->show();
                REQUIRE( QTest::qWaitForWindowExposed( crawlerVisitor.crawler.get() ) );

                crawlerVisitor.scrollFilteredViewToBottom();
                crawlerVisitor.enableGlobalFollowMode();
                for ( auto i = 0; i < 5; ++i ) {
                    crawlerVisitor.scrollFilteredViewDown();
                }

                const auto initialTopLine = crawlerVisitor.filteredTopLine();
                crawlerVisitor.scrollFilteredViewUp();

                THEN( "filtered view still scrolls away from the bottom" )
                {
                    REQUIRE( crawlerVisitor.filteredVerticalScrollMaximum() > 0 );
                    REQUIRE( crawlerVisitor.filteredTopLine() < initialTopLine );
                }
            }
        }

        WHEN( "search for 10" )
        {
            crawlerVisitor.setSearchPattern( "10" );

            crawlerVisitor.runSearch();

            waitUiState( [ & ]() { return crawlerVisitor.getLogFilteredNbLines().get() == 1; } );

            THEN( "single line match" )
            {
                REQUIRE( crawlerVisitor.getLogFilteredNbLines().get() == 1 );
            }
        }

        WHEN( "case sensitive search" )
        {
            crawlerVisitor.setSearchPattern( "THIS" );
            crawlerVisitor.enableCaseSensitiveSearch();
            crawlerVisitor.runSearch();

            THEN( "no lines matched" )
            {
                REQUIRE( crawlerVisitor.getLogFilteredNbLines().get() == 0 );
            }
        }

        WHEN( "inverse match search" )
        {
            crawlerVisitor.setSearchPattern( "not match" );
            crawlerVisitor.enableInverseMatch();
            crawlerVisitor.runSearch();

            THEN( "all lines matched" )
            {
                REQUIRE( crawlerVisitor.getLogFilteredNbLines().get() == SL_NB_LINES );
            }
        }

        WHEN( "boolean search" )
        {
            crawlerVisitor.setSearchPattern( "\"glogg\" or \"klogg\"" );
            crawlerVisitor.enableBooleanCombinationMode();
            crawlerVisitor.runSearch();

            THEN( "has lines matched" )
            {
                REQUIRE( crawlerVisitor.getLogFilteredNbLines().get() >= 2 );
            }
        }
    }
}

SCENARIO( "Crawler widget auto marked searches are scoped to kept result tabs", "[ui]" )
{
    QTemporaryFile file{ "crawler_auto_marked_tabs_test_XXXXXX" };
    REQUIRE( generateDataFiles( file ) );

    Session session;
    session.savedSearches().clear();

    CrawlerWidgetVisitor crawlerVisitor;
    crawlerVisitor.crawler.reset( static_cast<CrawlerWidget*>(
        session.open( file.fileName(), []() { return new CrawlerWidget(); } ) ) );

    waitUiState( [ & ]() { return crawlerVisitor.getLogNbLines().get() == SL_NB_LINES; } );
    waitUiState( [ & ]() { return crawlerVisitor.isLoadingFinished(); } );

    GIVEN( "two kept filtered result tabs with auto-marked searches" )
    {
        crawlerVisitor.replaceSearchPattern( "line 000010" );
        crawlerVisitor.runSearch();
        crawlerVisitor.markCurrentSearchResults();

        REQUIRE( crawlerVisitor.filteredTabCount() == 1 );
        REQUIRE( crawlerVisitor.getMarksCount().get() == 1 );
        REQUIRE( crawlerVisitor.autoMarkedSearchButtonCount() == 1 );
        REQUIRE( crawlerVisitor.autoMarkedSearchTooltip( 0 ) == "line 000010" );

        crawlerVisitor.keepNextSearchResults();
        crawlerVisitor.replaceSearchPattern( "line 000011" );
        crawlerVisitor.runSearch();
        crawlerVisitor.markCurrentSearchResults();

        REQUIRE( crawlerVisitor.filteredTabCount() == 2 );
        REQUIRE( crawlerVisitor.getMarksCount().get() == 1 );
        REQUIRE( crawlerVisitor.autoMarkedSearchButtonCount() == 1 );
        REQUIRE( crawlerVisitor.autoMarkedSearchTooltip( 0 ) == "line 000011" );

        WHEN( "removing a marked search from the first tab" )
        {
            crawlerVisitor.switchFilteredTab( 0 );

            REQUIRE( crawlerVisitor.getMarksCount().get() == 1 );
            REQUIRE( crawlerVisitor.autoMarkedSearchButtonCount() == 1 );
            REQUIRE( crawlerVisitor.autoMarkedSearchTooltip( 0 ) == "line 000010" );

            crawlerVisitor.removeAutoMarkedSearch( 0 );

            THEN( "only the first tab marks are removed" )
            {
                REQUIRE( crawlerVisitor.getMarksCount().get() == 0 );
                REQUIRE( crawlerVisitor.autoMarkedSearchButtonCount() == 0 );

                crawlerVisitor.switchFilteredTab( 1 );

                REQUIRE( crawlerVisitor.getMarksCount().get() == 1 );
                REQUIRE( crawlerVisitor.autoMarkedSearchButtonCount() == 1 );
                REQUIRE( crawlerVisitor.autoMarkedSearchTooltip( 0 ) == "line 000011" );
            }
        }
    }
}

SCENARIO( "Crawler widget shared auto-mark owners keep marks until the last owner is removed",
          "[ui]" )
{
    QTemporaryFile file{ "crawler_auto_marked_shared_owner_test_XXXXXX" };
    REQUIRE( generateDataFiles( file ) );

    Session session;
    session.savedSearches().clear();

    CrawlerWidgetVisitor crawlerVisitor;
    crawlerVisitor.crawler.reset( static_cast<CrawlerWidget*>(
        session.open( file.fileName(), []() { return new CrawlerWidget(); } ) ) );

    waitUiState( [ & ]() { return crawlerVisitor.getLogNbLines().get() == SL_NB_LINES; } );
    waitUiState( [ & ]() { return crawlerVisitor.isLoadingFinished(); } );

    GIVEN( "two auto-marked searches matching the same line" )
    {
        crawlerVisitor.replaceSearchPattern( "line 000010" );
        crawlerVisitor.runSearch();
        crawlerVisitor.markCurrentSearchResults();

        REQUIRE( crawlerVisitor.getMarksCount().get() == 1 );
        REQUIRE( crawlerVisitor.autoMarkedSearchButtonCount() == 1 );

        crawlerVisitor.replaceSearchPattern( "000010" );
        crawlerVisitor.runSearch();
        crawlerVisitor.markCurrentSearchResults();

        REQUIRE( crawlerVisitor.getMarksCount().get() == 1 );
        REQUIRE( crawlerVisitor.autoMarkedSearchButtonCount() == 2 );
        REQUIRE( crawlerVisitor.autoMarkedSearchTooltip( 0 ) == "000010" );
        REQUIRE( crawlerVisitor.autoMarkedSearchTooltip( 1 ) == "line 000010" );

        WHEN( "one owning search is removed" )
        {
            crawlerVisitor.removeAutoMarkedSearch( 0 );

            THEN( "the shared mark remains until the last owner is removed" )
            {
                REQUIRE( crawlerVisitor.getMarksCount().get() == 1 );
                REQUIRE( crawlerVisitor.autoMarkedSearchButtonCount() == 1 );
                REQUIRE( crawlerVisitor.autoMarkedSearchTooltip( 0 ) == "line 000010" );

                crawlerVisitor.removeAutoMarkedSearch( 0 );

                REQUIRE( crawlerVisitor.getMarksCount().get() == 0 );
                REQUIRE( crawlerVisitor.autoMarkedSearchButtonCount() == 0 );
            }
        }
    }
}

SCENARIO( "Crawler widget pending auto mark completion is scoped to its result tab", "[ui]" )
{
    QTemporaryFile file{ "crawler_pending_auto_marked_tabs_test_XXXXXX" };
    REQUIRE( generateDataFiles( file ) );

    Session session;
    session.savedSearches().clear();

    CrawlerWidgetVisitor crawlerVisitor;
    crawlerVisitor.crawler.reset( static_cast<CrawlerWidget*>(
        session.open( file.fileName(), []() { return new CrawlerWidget(); } ) ) );

    waitUiState( [ & ]() { return crawlerVisitor.getLogNbLines().get() == SL_NB_LINES; } );
    waitUiState( [ & ]() { return crawlerVisitor.isLoadingFinished(); } );

    GIVEN( "two kept filtered result tabs" )
    {
        crawlerVisitor.replaceSearchPattern( "line 000010" );
        crawlerVisitor.runSearch();

        REQUIRE( crawlerVisitor.filteredTabCount() == 1 );
        REQUIRE( crawlerVisitor.getLogFilteredNbLines().get() == 1 );
        REQUIRE( crawlerVisitor.marksCountForFilteredTab( 0 ).get() == 0 );

        crawlerVisitor.keepNextSearchResults();
        crawlerVisitor.replaceSearchPattern( "line 000011" );
        crawlerVisitor.runSearch();

        REQUIRE( crawlerVisitor.filteredTabCount() == 2 );
        REQUIRE( crawlerVisitor.getLogFilteredNbLines().get() == 1 );
        REQUIRE( crawlerVisitor.marksCountForFilteredTab( 1 ).get() == 0 );
        REQUIRE( crawlerVisitor.autoMarkedSearchButtonCount() == 0 );

        WHEN( "an older tab completes a pending auto-mark search after switching tabs" )
        {
            crawlerVisitor.queuePendingAutoMarkForFilteredTab( 0, "line 000010" );
            crawlerVisitor.completeSearchForFilteredTab( 0, 1_lcount );

            THEN( "the mark and marked-search UI stay attached to the older tab" )
            {
                REQUIRE( crawlerVisitor.marksCountForFilteredTab( 0 ).get() == 1 );
                REQUIRE( crawlerVisitor.marksCountForFilteredTab( 1 ).get() == 0 );
                REQUIRE( crawlerVisitor.getMarksCount().get() == 0 );
                REQUIRE( crawlerVisitor.autoMarkedSearchButtonCount() == 0 );

                crawlerVisitor.switchFilteredTab( 0 );

                REQUIRE( crawlerVisitor.getMarksCount().get() == 1 );
                REQUIRE( crawlerVisitor.autoMarkedSearchButtonCount() == 1 );
                REQUIRE( crawlerVisitor.autoMarkedSearchTooltip( 0 ) == "line 000010" );
            }
        }
    }
}

SCENARIO( "Crawler widget frequent search history shortcuts", "[ui]" )
{
    QTemporaryFile file{ "crawler_frequent_search_test_XXXXXX" };
    REQUIRE( generateDataFiles( file ) );

    Session session;
    auto& savedSearches = session.savedSearches();
    savedSearches.clear();

    const auto longSearch = QStringLiteral(
        "camera_params_setting_done_request_with_a_very_long_middle_section" );
    savedSearches.addRecent( "this is line" );
    savedSearches.addRecent( "this is line" );
    savedSearches.addRecent( "this is line" );
    savedSearches.addRecent( "line 000010" );
    savedSearches.addRecent( "line 000010" );
    savedSearches.addRecent( longSearch );
    savedSearches.addRecent( longSearch );
    for ( auto i = 0; i < 8; ++i ) {
        savedSearches.addRecent( QStringLiteral( "history_%1" ).arg( i ) );
    }

    CrawlerWidgetVisitor crawlerVisitor;
    crawlerVisitor.crawler.reset( static_cast<CrawlerWidget*>(
        session.open( file.fileName(), []() { return new CrawlerWidget(); } ) ) );

    waitUiState( [ & ]() { return crawlerVisitor.getLogNbLines().get() == SL_NB_LINES; } );
    waitUiState( [ & ]() { return crawlerVisitor.isLoadingFinished(); } );

    GIVEN( "loaded log data and search history" )
    {
        REQUIRE( waitUiState( [ &crawlerVisitor ]() {
            return crawlerVisitor.frequentSearchButtonCount() == 10;
        } ) );

        THEN( "frequent history searches are shown as fixed-width shortcuts" )
        {
            REQUIRE( crawlerVisitor.frequentSearchTooltip( 0 ) == "this is line (3)" );
            REQUIRE( crawlerVisitor.frequentSearchText( 0 ) == "this is line" );

            auto* longSearchButton
                = crawlerVisitor.frequentSearchButtonByTooltip( longSearch + " (2)" );
            REQUIRE( longSearchButton != nullptr );
            REQUIRE( longSearchButton->width() == crawlerVisitor.frequentSearchMaxWidth() );
            REQUIRE( longSearchButton->parentWidget()->width()
                     == crawlerVisitor.frequentSearchMaxWidth() );
            REQUIRE( longSearchButton->text().contains( "..." ) );
            REQUIRE( longSearchButton->toolTip() == longSearch + " (2)" );
        }

        WHEN( "clicking a frequent search" )
        {
            crawlerVisitor.clickFrequentSearch( 0 );

            THEN( "search is run and matching lines are marked" )
            {
                REQUIRE( waitUiState( [ &crawlerVisitor ]() {
                    return crawlerVisitor.getMarksCount().get() == SL_NB_LINES;
                } ) );
                REQUIRE( crawlerVisitor.getLogFilteredNbLines().get() == SL_NB_LINES );
                REQUIRE( waitUiState( [ &crawlerVisitor ]() {
                    return crawlerVisitor.autoMarkedSearchButtonCount() == 1;
                } ) );
                REQUIRE( crawlerVisitor.autoMarkedSearchTooltip( 0 ) == "this is line" );

                crawlerVisitor.removeAutoMarkedSearch( 0 );

                REQUIRE( waitUiState(
                    [ &crawlerVisitor ]() { return crawlerVisitor.getMarksCount().get() == 0; } ) );
                REQUIRE( crawlerVisitor.autoMarkedSearchButtonCount() == 0 );
            }
        }
    }
}
