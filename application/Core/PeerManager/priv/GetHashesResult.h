#ifndef PEERMANAGER_GET_HASHES_RESULT_H
#define PEERMANAGER_GET_HASHES_RESULT_H

#include <QObject>
#include <QSharedPointer>

#include <google/protobuf/message.h>

#include <Protos/common.pb.h>
#include <Protos/core_protocol.pb.h>

#include <Common/Network.h>

#include <IGetHashesResult.h>
#include <priv/Socket.h>

namespace PM
{
   class GetHashesResult : public IGetHashesResult
   {
      Q_OBJECT
   public:
      GetHashesResult(const Protos::Common::Entry& file, QSharedPointer<Socket> socket);
      ~GetHashesResult();
      void start();

   private slots:
      void newMessage(Common::Network::CoreMessageType type, const google::protobuf::Message& message);

   private:
      const Protos::Common::Entry file;
      QSharedPointer<Socket> socket;
   };
}

#endif