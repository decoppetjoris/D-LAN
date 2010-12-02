#include <priv/ChunkIndex/Chunks.h>
using namespace FM;

#include <priv/Cache/Chunk.h>
#include <priv/Log.h>

void Chunks::add(QSharedPointer<Chunk> chunk)
{
   QMutexLocker locker(&this->mutex);
   this->insert(chunk->getHash(), chunk);
}

void Chunks::rm(QSharedPointer<Chunk> chunk)
{
   QMutexLocker locker(&this->mutex);
   this->remove(chunk->getHash());
}

const QSharedPointer<Chunk> Chunks::value(const Common::Hash& hash) const
{
   QMutexLocker locker(&this->mutex);
   return QHash< Common::Hash, QSharedPointer<Chunk> >::value(hash);
}

bool Chunks::contains(const Common::Hash& hash) const
{
   QMutexLocker locker(&this->mutex);
   return QHash< Common::Hash, QSharedPointer<Chunk> >::contains(hash);
}
