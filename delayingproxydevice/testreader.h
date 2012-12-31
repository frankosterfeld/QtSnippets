/******************************************************************************
 *   Copyright (C) 2012 Frank Osterfeld <frank.osterfeld@gmail.com>           *
 *                                                                            *
 * This program is distributed in the hope that it will be useful, but        *
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE. For licensing and distribution        *
 * details, check the accompanying file 'COPYING'.                            *
 *****************************************************************************/

#ifndef TESTREADER_H
#define TESTREADER_H

#include <QObject>

class QIODevice;
class TestReader;

class TestReader : public QObject {
    Q_OBJECT
public:
    explicit TestReader( QObject* parent=0 );

    QByteArray receivedData() const;

    QIODevice* device() const;
    void setDevice( QIODevice* dev );

Q_SIGNALS:
    void finished();

private Q_SLOTS:
    void readyRead();

private:
    QIODevice* m_device;
    QByteArray m_data;
};


#endif // TESTREADER_H
