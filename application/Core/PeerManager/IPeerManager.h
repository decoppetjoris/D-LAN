/**
  * Aybabtu - A decentralized LAN file sharing software.
  * Copyright (C) 2010-2011 Greg Burri <greg.burri@gmail.com>
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  */
  
#ifndef PEERMANAGER_IPEERMANAGER_H
#define PEERMANAGER_IPEERMANAGER_H

#include <QString>
#include <QtNetwork>
#include <QSharedPointer>

#include <Core/FileManager/IChunk.h>

#include <Core/PeerManager/ISocket.h>
#include <Common/Hash.h>
#include <Protos/common.pb.h>

namespace PM
{
   class IPeer;

   class IPeerManager : public QObject
   {
      Q_OBJECT
   public:
      virtual ~IPeerManager() {}

      /**
        * Return our ID. This ID is taken from the settings in the home folder of the current user.
        * If there is no ID in the settings, this one is randomly generated and saved in the settings.
        */
      virtual Common::Hash getID() = 0;

      /**
        * As the ID, the nick is saved in the file settings in the home folder of the current user.
        */
      virtual void setNick(const QString& nick) = 0;
      virtual QString getNick() = 0;

      /**
        * Return all alive peers. A peer is never deleted but can become inactive.
        * @remarks This list doesn't include us.
        */
      virtual QList<IPeer*> getPeers() = 0;

      /**
        * Return the IPeer* coresponding to ID.
        * Return 0 if the peer doesn't exist.
        */
      virtual IPeer* getPeer(const Common::Hash& ID) = 0;

      /**
        * The method must be call frequently to tell that a peer (ID) is still alive.
        * @see The protobuf message 'Protos.Core.IMAlive' in "Protos/core_protocol.proto".
        */
      virtual void updatePeer(const Common::Hash& ID, const QHostAddress& IP, quint16 port, const QString& nick, const quint64& sharingAmount) = 0;

      /**
        * @param tcpSocket PeerManager will care about deleting the socket.
        */
      virtual void newConnection(QTcpSocket* tcpSocket) = 0;

   signals:
      /**
        * When a remote peer want a chunk, this signal is emitted.
        * The chunk will be sent using the socket object. Once the data is finished to send the method 'ISocket::finished()' must be called.
        */
      void getChunk(QSharedPointer<FM::IChunk> chunk, int offset, QSharedPointer<PM::ISocket> socket);
   };
}
#endif
