/******************************************************************************
 *   Copyright (C) 2012 Frank Osterfeld <frank.osterfeld@gmail.com>           *
 *                                                                            *
 * This program is distributed in the hope that it will be useful, but        *
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE. For licensing and distribution        *
 * details, check the accompanying file 'COPYING'.                            *
 *****************************************************************************/

#include "delayingproxydevice.h"
#include "testreader.h"

#include <QtTest/QtTest>
#include <QXmlStreamReader>

class TestDelayingProxyDevice : public QObject
{
    Q_OBJECT

public:
private Q_SLOTS:

    void initTestCase() {
        qsrand( 0 ); // make sure outcome is repeatable

    }
    void testRandom() {
        //ensure that qrand() returns reproducible numbers (see qsrand() above)
        //should this ever fail, better use a fixed sample of "random" data stored in a file and setRandomDataSource()
        const QVector<int> expected = QVector<int>()
            << 84 << 79 << 82 << 24 << 3
            << 33 << 64 << 3 << 20 << 33
            << 22 << 62 << 51 << 36 << 86
            << 87 << 77 << 1 << 74 << 22;
        QVector<int> actual( expected.size() );
        for ( int i = 0; i < actual.size(); ++i )
            actual[i] = qrand() % 100;
        QCOMPARE(actual, expected);
    }


    void testEmptyByteArray() {
        const QByteArray emptyData;

        DelayingProxyDevice dev( emptyData );
        dev.setMediumChunkSize( 5 );
        dev.setRandomizationDelta( 3 );
        const bool opened = dev.open( QIODevice::ReadOnly );
        QVERIFY( opened );

        TestReader reader;
        reader.setDevice( &dev );
        QEventLoop loop;

        QObject::connect( &reader, SIGNAL(finished()), &loop, SLOT(quit()) );
        loop.exec();
        QCOMPARE( reader.receivedData(), emptyData );
    }

    void testByteArray()
    {
        for ( int i = 0; i < 20; ++i ) {
            const QByteArray testData = "foo bar foo bar lal alal a sdf dsf sadf dsaf dsaf dsaf dfsrgtrewg jlkewrfgewroijresfewa";

            DelayingProxyDevice dev( testData );
            dev.setMediumChunkSize( 5 );
            dev.setRandomizationDelta( 3 );
            const bool opened = dev.open( QIODevice::ReadOnly );
            QVERIFY( opened );

            TestReader reader;
            reader.setDevice( &dev );
            QEventLoop loop;
            QObject::connect( &reader, SIGNAL(finished()), &loop, SLOT(quit()) );
            loop.exec();
            QCOMPARE( reader.receivedData(), testData );
        }
    }

    void testFile() {
        const QString fname = QLatin1String(":/bildblog.xml");
        QFile f( fname );
        const bool fopened = f.open( QIODevice::ReadOnly );
        QVERIFY( fopened );

        DelayingProxyDevice dev( &f );
        dev.setMediumChunkSize( 5 );
        dev.setRandomizationDelta( 3 );
        QVERIFY( dev.open( QIODevice::ReadOnly ) );

        TestReader reader;
        reader.setDevice( &dev );
        QEventLoop loop;
        QObject::connect( &reader, SIGNAL(finished()), &loop, SLOT(quit()) );
        loop.exec();
        QFile f2( fname );
        const bool fopened2 = f2.open( QIODevice::ReadOnly );
        QVERIFY( fopened2 );
        const QByteArray expected = f2.readAll();
        QCOMPARE( reader.receivedData(), expected );
    }

    void testXmlStreamReader() {
        const QString fname = QLatin1String(":/bildblog.xml");
        QFile f( fname );
        const bool fopened = f.open( QIODevice::ReadOnly );
        QVERIFY( fopened );

        DelayingProxyDevice dev( &f );
        dev.setMediumChunkSize( 5 );
        dev.setRandomizationDelta( 3 );
        const bool opened = dev.open( QIODevice::ReadOnly );
        QVERIFY( opened );

        QString title, linkrel, linktype, linkhref, contenttype, contentmode, content;

        QXmlStreamReader reader( &dev );
        reader.setNamespaceProcessing( true );
        while ( !reader.atEnd() &&  ( !reader.hasError() || reader.error() == QXmlStreamReader::PrematureEndOfDocumentError ) ) {
            if ( reader.error() == QXmlStreamReader::PrematureEndOfDocumentError ) {
                const bool ready = dev.waitForReadyRead( 1000 );
                QVERIFY(ready);
            }
            reader.readNext();
            if ( reader.isStartElement() ) {
                const QXmlStreamAttributes attrs = reader.attributes();
                if ( reader.name() == QLatin1String("title") ) {
                    title = reader.readElementText();
                } else if ( reader.name() == QLatin1String("link") ) {
                    linkrel = attrs.value( QLatin1String("rel") ).toString();
                    linktype = attrs.value( QLatin1String("type") ).toString();
                    linkhref = attrs.value( QLatin1String("href") ).toString();
                } else if ( reader.name() == QLatin1String("content") ) {
                    contenttype  = attrs.value( QLatin1String("type") ).toString();
                    contentmode = attrs.value( QLatin1String("mode") ).toString();
                }
            }
        }

        QVERIFY( !reader.hasError() );
        QCOMPARE( linkrel, QLatin1String("alternate") );
        QCOMPARE( linktype, QLatin1String("text/html") );
        QCOMPARE( linkhref, QLatin1String("http://www.bildblog.de") );
        QCOMPARE( contenttype, QLatin1String("text/html") );
        QCOMPARE( contentmode, QLatin1String("escaped") );
        QCOMPARE( title, QString() ); //This should be "BILDBlog". Bug in readElementText(), see QTBUG-14661
    }
};

QTEST_MAIN(TestDelayingProxyDevice)

#include "testdelayingproxydevice.moc"

