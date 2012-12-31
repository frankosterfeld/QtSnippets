#include "testreader.h"
#include <QIODevice>
#include <QDebug>

TestReader::TestReader( QObject* parent )
    : QObject( parent )
{
}

QByteArray TestReader::receivedData() const
{
    return m_data;
}

void TestReader::setDevice( QIODevice* dev ) {
    m_device = dev;
    connect( m_device, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::UniqueConnection );
    if ( dev->bytesAvailable() > 0 )
        QMetaObject::invokeMethod( this, "readyRead", Qt::QueuedConnection );
}

QIODevice* TestReader::device() const {
    return m_device;
}

void TestReader::readyRead() {
    const qint64 a = m_device->bytesAvailable();

    QByteArray buf( a > 0 ? a : 1024, ' ' );
    const qint64 nr = m_device->read( buf.data(), buf.size() );

    if ( nr > 0 ) {
        m_data += buf.left( nr );
        if ( m_device->bytesAvailable() > 0 )
            QMetaObject::invokeMethod( this, "readyRead", Qt::QueuedConnection );
    } else {
        emit finished();
    }
}

