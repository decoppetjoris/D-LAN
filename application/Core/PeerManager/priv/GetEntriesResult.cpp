#include <priv/GetEntriesResult.h>
using namespace PM;

#include <priv/Log.h>

GetEntriesResult::GetEntriesResult(const Protos::Core::GetEntries& dir, Socket* socket)
   : dir(dir), socket(socket)
{
}

GetEntriesResult::~GetEntriesResult()
{
   this->socket->finished();
}

void GetEntriesResult::start()
{
   connect(this->socket, SIGNAL(newMessage(quint32, const google::protobuf::Message&)), this, SLOT(newMessage(quint32, const google::protobuf::Message&)));
   socket->send(0x31, this->dir);
}

void GetEntriesResult::newMessage(quint32 type, const google::protobuf::Message& message)
{
   if (type != 0x32)
      return;

   const Protos::Core::GetEntriesResult& entries = dynamic_cast<const Protos::Core::GetEntriesResult&>(message);
   emit result(entries);
   disconnect(this->socket, SIGNAL(newMessage(quint32, const google::protobuf::Message&)), this, SLOT(newMessage(quint32, const google::protobuf::Message&)));
}