#include "packetizer.h"
#include <QDebug>

#define START_BYTE static_cast<unsigned char>(0xAA)
#define ESCAPE_BYTE static_cast<unsigned char>(0xBB)
#define STOP_BYTE static_cast<unsigned char>(0xCC)

#define ESCAPE_START static_cast<unsigned char>(0x55)
#define ESCAPE_ESCAPE static_cast<unsigned char>(0x44)
#define ESCAPE_STOP static_cast<unsigned char>(0x33)


Packetizer::Packetizer(QObject *identifier,QObject *parent) : QObject(parent)
{
	m_isInPacket = false;
	m_isInEscape = false;
	m_identifer = identifier;
}

QByteArray Packetizer::generatePacket(QByteArray payload)
{
	QByteArray packet;
	packet.append((char)0xAA);
	unsigned char checksum = 0;
	for (int i=0;i<payload.size();i++)
	{
		checksum += payload[i];
	}
	payload.append(checksum);
	for (int j=0;j<payload.size();j++)
	{
		if (payload[j] == (char)0xAA)
		{
			packet.append((char)0xBB);
			packet.append((char)0x55);
		}
		else if (payload[j] == (char)0xBB)
		{
			packet.append((char)0xBB);
			packet.append((char)0x44);
		}
		else if (payload[j] == (char)0xCC)
		{
			packet.append((char)0xBB);
			packet.append((char)0x33);
		}
		else
		{
			packet.append(payload[j]);
		}
	}
	packet.append((char)0xCC);
	return packet;
}

void Packetizer::parseBuffer(QByteArray buffer)
{
	for (int i=0;i<buffer.size();i++)
	{
		if (static_cast<unsigned char>(buffer.at(i)) == START_BYTE)
		{
			if (m_isInPacket)
			{
				//Bad start
				qDebug() << "Bad Start";
			}
			m_isInPacket = true;
			m_isInEscape = false;
			m_currMsg.clear();
		}
		else if (static_cast<unsigned char>(buffer.at(i)) == STOP_BYTE)
		{
			if (!m_isInPacket)
			{
				//Bad stop
				qDebug() << "Bad Stop";
				continue;
			}
			m_isInPacket = false;
			QByteArray toemit = m_currMsg;
			toemit.detach();
			emit newPacket(m_identifer,toemit);
			m_currMsg.clear();
		}
		else if (m_isInPacket)
		{
			if (m_isInEscape)
			{
				if (static_cast<unsigned char>(buffer.at(i)) == ESCAPE_START)
				{
					m_currMsg.append(START_BYTE);
				}
				else if (static_cast<unsigned char>(buffer.at(i)) == ESCAPE_ESCAPE)
				{
					m_currMsg.append(ESCAPE_BYTE);
				}
				else if (static_cast<unsigned char>(buffer.at(i)) == ESCAPE_STOP)
				{
					m_currMsg.append(STOP_BYTE);
				}
				else
				{
					//Bad escape character
					qDebug() << "Bad Escape char";
				}
				m_isInEscape = false;
			}
			else if (static_cast<unsigned char>(buffer.at(i)) == ESCAPE_BYTE)
			{
				m_isInEscape = true;
			}
			else
			{
				m_currMsg.append(buffer.at(i));
			}
		}
		else
		{
			//Out of packet bytes.
			qDebug() << "Out of packet bytes" << QString::number(buffer.at(i),16);
		}
	}
}
