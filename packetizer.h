#ifndef PACKETIZER_H
#define PACKETIZER_H

#include <QByteArray>
#include <QObject>

//Packetize data and return a QByteArray
class Packetizer : public QObject
{
	Q_OBJECT
public:
	Packetizer(QObject *identifer,QObject *parent = 0);
	static QByteArray generatePacket(QByteArray payload);

signals:
	void newPacket(QObject *ident,QByteArray packet);

public slots:
	void parseBuffer(QByteArray buffer);

private:
	QByteArray m_currMsg;
	bool m_isInPacket;
	bool m_isInEscape;
	QObject *m_identifer;
};

#endif //PACKETIZER_H
