#ifndef DELAYINGPROXYDEVICE_H
#define DELAYINGPROXYDEVICE_H

#include <QIODevice>
#include <QSharedPointer>

/**
 * Interface for delivering pseudo-random numbers to DelayingProxyDevice.
 * @see DelayingProxyDevice::randomizationDelta()
 */
class RandomDataSource {
public:
    virtual ~RandomDataSource();
    virtual int random() = 0;
};

/**
 * @brief A QIODevice proxy ensuring the data to arrive in small chunks.
 *
 * When processing input data incrementally reading from a device, such as parsing data from a network socket,
 * processing code (I/O, parser, etc.) often exhibits subtle and hard to reproduce bugs when the processed data
 * arrives in small chunks, instead of all at once.
 * To ensure parser code is robust enough to handle any behavior of the I/O, DelayingProxyDevice can be used
 * to test the parser by enforcing small chunk sizes.
 *
 * DelayingProxyDevice is always sequential, regardless of the wrapped source device. Thus, seek() will not work.
 *
 * You can use this class in e.g. a unit test as follows:
 *
 * @code
 * qsrand( 0 ); // make sure we have reproducible "randomization"
 * QFile f( "testfile.xml" );
 * QVERIFY( f.open( QIODevice::ReadOnly ) );
 * DelayingProxyDevice proxy( f.readAll() ); //Alternatively, you can pass &f, but that will make the test indeterministic
 * proxy.setMediumChunkSize( 8 );
 * proxy.setRandomizationDelta( 7 ); //will deliver data in chunks from 1 to 15 bytes
 * QVERIFY( proxy.open( QIODevice::ReadOnly ) );
 * YourParser parser;
 * parser.setDevice( &proxy );
 * @endcode
 */
class DelayingProxyDevice : public QIODevice {
    Q_OBJECT
    Q_PROPERTY(qint64 randomizationDelta READ randomizationDelta WRITE setRandomizationDelta)
    Q_PROPERTY(qint64 mediumChunkSize READ mediumChunkSize WRITE setMediumChunkSize)
public:
    explicit DelayingProxyDevice( const QByteArray& data, QObject* parent=0 );
    explicit DelayingProxyDevice( QIODevice* sourceDevice, QObject* parent=0 );

    ~DelayingProxyDevice();

    qint64 mediumChunkSize() const;
    void setMediumChunkSize( qint64 chunkSize );

    QSharedPointer<RandomDataSource> randomDataSource() const;
    void setRandomDataSource( const QSharedPointer<RandomDataSource>& source );

    /**
     * Allows randomizing the chunk sizes choosing a random value between
     * mediumChunkSize() - randomizationDelta() and mediumChunkSize() + randomizationDelta().
     * By default, qrand() is used to retrieve pseudo-random numbers. To override
     * this (e.g. to use another algorithm or to read reproducible pseudo-random
     * numbers from a file) implement a RandomDataSource and set it via setRandomDataSource().
     *
     * Defaults to 0 (no randomization, the chunk size will always be chunkSize())
     */
    qint64 randomizationDelta() const;
    void setRandomizationDelta( qint64 randomizationDelta );

    bool atEnd() const;
    qint64 bytesAvailable() const;
    qint64 bytesToWrite() const;
    bool canReadLine() const;
    void close();
    bool isSequential() const;
    bool open( OpenMode mode );
    qint64 pos() const;
    bool reset();
    bool seek( qint64 pos );
    bool waitForReadyRead( int msecs );
    bool waitForBytesWritten( int msecs );\
    qint64 size() const;

protected:
    qint64 readData( char* data, qint64 maxlen );
    qint64 writeData( const char* data, qint64 len );

private Q_SLOTS:
    void makeChunkAvailable();
    void sourceReadyRead();

private:
    class Private;
    Private* const d;
};

#endif // DELAYINGPROXYDEVICE_H
