/******************************************************************************
 *   Copyright (C) 2012 Frank Osterfeld <frank.osterfeld@gmail.com>           *
 *                                                                            *
 * This program is distributed in the hope that it will be useful, but        *
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE. For licensing and distribution        *
 * details, check the accompanying file 'COPYING'.                            *
 *****************************************************************************/

#include "delayingproxydevice.h"

#include <QBuffer>
#include <QPointer>
#include <QTimer>


class DelayingProxyDevice::Private {
    DelayingProxyDevice* const q;
public:
    explicit Private( DelayingProxyDevice* qq )
        : q( qq )
        , bytesAvailable( 0 )
        , chunkSize( 8 )
        , randomizationDelta( 0 )
    {
        makeAvailableTimer.setInterval( 10 );
        connect( &makeAvailableTimer, SIGNAL(timeout()), q, SLOT(makeChunkAvailable()) );
    }

    inline int rand() const {
        return random ? random->random() : qrand();
    }

    QSharedPointer<RandomDataSource> random;
    qint64 bytesAvailable;
    qint64 chunkSize;
    qint64 randomizationDelta;
    QPointer<QIODevice> sourceDevice;
    QTimer makeAvailableTimer;

    bool moreInSource() const {
        return bytesAvailable < sourceDevice->bytesAvailable();
    }

    bool allAvailable() const {
        return bytesAvailable == sourceDevice->bytesAvailable();
    }
    qint64 calcChunkSize() const;
    void makeChunkAvailableImpl();
};

qint64 DelayingProxyDevice::Private::calcChunkSize() const {
    Q_ASSERT( chunkSize > 0 );
    Q_ASSERT( chunkSize > randomizationDelta );
    if ( randomizationDelta > 0 ) {
        const qint64 delta = rand() % ( 2 * randomizationDelta + 1 ) - randomizationDelta;
        return qMax( static_cast<qint64>( 1 ), chunkSize + delta );
    } else {
        return chunkSize;
    }
}

DelayingProxyDevice::DelayingProxyDevice( const QByteArray& data, QObject* parent )
    : QIODevice( parent )
    , d( new Private( this ) )
{
    QBuffer* buf = new QBuffer( this );
    buf->setData( data );
    d->sourceDevice = buf;
    const bool opened = buf->open( QIODevice::ReadOnly );
    Q_ASSERT( opened );
    Q_UNUSED( opened )
    connect( d->sourceDevice, SIGNAL(readyRead()), this, SLOT(sourceReadyRead()) );
}

DelayingProxyDevice::DelayingProxyDevice( QIODevice* sourceDevice, QObject* parent )
    : QIODevice( parent )
    , d( new Private( this ) )
{
    d->sourceDevice = sourceDevice;
    connect( d->sourceDevice, SIGNAL(readyRead()), this, SLOT(sourceReadyRead()) );
}

DelayingProxyDevice::~DelayingProxyDevice() {
    delete d;
}

qint64 DelayingProxyDevice::mediumChunkSize() const {
    return d->chunkSize;
}

void DelayingProxyDevice::setMediumChunkSize( qint64 siz ) {
    d->chunkSize = siz;
}

qint64 DelayingProxyDevice::randomizationDelta() const {
    return d->randomizationDelta;
}

void DelayingProxyDevice::setRandomizationDelta( qint64 rf ) {
    d->randomizationDelta = rf;
}

qint64 DelayingProxyDevice::readData( char* data, qint64 maxlen ) {
    Q_ASSERT( d->sourceDevice );
    const qint64 ml = qMin( d->bytesAvailable, maxlen );
    const qint64 nr = d->sourceDevice->read( data, ml );
    if ( nr >= 0 ) {
        d->bytesAvailable -= nr;
        if ( d->bytesAvailable == 0 && d->moreInSource() )
            d->makeAvailableTimer.start();
    }
    else
        setErrorString( d->sourceDevice->errorString() );
    return nr;
}

qint64 DelayingProxyDevice::writeData( const char* data, qint64 len )
{
    Q_ASSERT( d->sourceDevice );
    const qint64 written = d->sourceDevice->write( data, len );
    if ( written < 0 ) {
        setErrorString( d->sourceDevice->errorString() );
    }
    return written;
}

void DelayingProxyDevice::Private::makeChunkAvailableImpl() {
    const qint64 siz = calcChunkSize();
    Q_ASSERT( siz > 0 );
    bytesAvailable = qMin( bytesAvailable + siz, sourceDevice->bytesAvailable() );
}

void DelayingProxyDevice::makeChunkAvailable() {
    d->makeChunkAvailableImpl();
    emit readyRead();
}

bool DelayingProxyDevice::open( OpenMode m ) {
    Q_ASSERT( d->sourceDevice );
    setOpenMode( m );
    if ( d->sourceDevice->bytesAvailable() > 0 || !d->sourceDevice->isSequential() )
       d->makeAvailableTimer.start();
    return true;
}

bool DelayingProxyDevice::isSequential() const {
    return true;
}

bool DelayingProxyDevice::seek( qint64 ) {
    return false;
}

bool DelayingProxyDevice::atEnd() const {
    return d->sourceDevice->atEnd();
}

qint64 DelayingProxyDevice::bytesToWrite() const {
    return d->sourceDevice->bytesToWrite();
}

qint64 DelayingProxyDevice::bytesAvailable() const {
    return d->bytesAvailable;
}

bool DelayingProxyDevice::canReadLine() const {
    return d->sourceDevice->canReadLine();
}

void DelayingProxyDevice::close() {
    d->sourceDevice->close();
}

qint64 DelayingProxyDevice::pos() const {
    return d->sourceDevice->pos();
}

qint64 DelayingProxyDevice::size() const {
    return isOpen() ? d->bytesAvailable : -1;
}

bool DelayingProxyDevice::reset() {
    return d->sourceDevice->reset();
}

bool DelayingProxyDevice::waitForReadyRead( int msecs ) {
    if ( d->bytesAvailable > 0 )
        return true;
    if ( !d->allAvailable() ) {
        d->makeChunkAvailableImpl();
        return true;
    } else {
        const bool ready = d->sourceDevice->waitForReadyRead( msecs );
        if ( ready )
            d->makeChunkAvailableImpl();
        return ready;
    }
}

bool DelayingProxyDevice::waitForBytesWritten( int msecs ) {
    return d->sourceDevice->waitForBytesWritten( msecs );
}

void DelayingProxyDevice::sourceReadyRead() {
    d->makeAvailableTimer.start();
}

QSharedPointer<RandomDataSource> DelayingProxyDevice::randomDataSource() const
{
    return d->random;
}

void DelayingProxyDevice::setRandomDataSource( const QSharedPointer<RandomDataSource>& source )
{
    d->random = source;
}
