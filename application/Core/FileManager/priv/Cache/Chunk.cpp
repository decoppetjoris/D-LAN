#include <priv/Cache/Chunk.h>
using namespace FM;

#include <Common/Settings.h>

#include <IDataReader.h>
#include <IDataWriter.h>
#include <Exceptions.h>
#include <priv/Log.h>
#include <priv/Cache/File.h>
#include <priv/Cache/SharedDirectory.h>
#include <priv/Cache/DataReader.h>
#include <priv/Cache/DataWriter.h>

Chunk::Chunk(File* file, int num, quint32 knownBytes) :
   CHUNK_SIZE(SETTINGS.get<quint32>("chunk_size")),
   BUFFER_SIZE(SETTINGS.get<quint32>("buffer_size")),
   mutex(QMutex::Recursive), file(file), num(num), knownBytes(knownBytes)
{
   QMutexLocker locker(&this->mutex);
   L_DEBU(QString("New chunk[%1] : %2. File : %3").arg(num).arg(hash.toStr()).arg(this->file->getFullPath()));
}

Chunk::Chunk(File* file, int num, quint32 knownBytes, const Common::Hash& hash) :
   CHUNK_SIZE(SETTINGS.get<quint32>("chunk_size")),
   BUFFER_SIZE(SETTINGS.get<quint32>("buffer_size")),
   mutex(QMutex::Recursive), file(file), num(num), knownBytes(knownBytes), hash(hash)
{
   QMutexLocker locker(&this->mutex);
   L_DEBU(QString("New chunk[%1] : %2. File : %3").arg(num).arg(hash.toStr()).arg(this->file->getFullPath()));
}

Chunk::~Chunk()
{
   QMutexLocker locker(&this->mutex);
   L_DEBU(QString("Chunk Deleted[%1] : %2. File : %3").arg(num).
      arg(this->hash.toStr()).
      arg(this->file ? this->file->getFullPath() : "<file deleted>")
   );
}

void Chunk::removeItsFile()
{
   QMutexLocker locker(&this->mutex);
   if (this->file && !this->file->isComplete())
      delete this->file;
}

Chunk* Chunk::restoreFromFileCache(const Protos::FileCache::Hashes_Chunk& chunk)
{
   this->knownBytes = chunk.known_bytes();

   if (chunk.has_hash())
      this->hash = chunk.hash().hash().data();
   return this;
}

void Chunk::populateHashesChunk(Protos::FileCache::Hashes_Chunk& chunk) const
{
   chunk.set_known_bytes(this->knownBytes);
   if (!this->hash.isNull())
      chunk.mutable_hash()->set_hash(this->hash.getData(), Common::Hash::HASH_SIZE);
}

void Chunk::populateEntry(Protos::Common::Entry* entry) const
{
   QMutexLocker locker(&this->mutex);
   if (this->file)
      this->file->populateEntry(entry);
}

QSharedPointer<IDataReader> Chunk::getDataReader()
{
   QMutexLocker locker(&this->mutex);
   return QSharedPointer<IDataReader>(new DataReader(*this));
}

QSharedPointer<IDataWriter> Chunk::getDataWriter()
{
   QMutexLocker locker(&this->mutex);
   return QSharedPointer<IDataWriter>(new DataWriter(*this));
}

void Chunk::newDataWriterCreated()
{
   QMutexLocker locker(&this->mutex);
   if (this->file)
      this->file->newDataWriterCreated();
}

void Chunk::newDataReaderCreated()
{
   QMutexLocker locker(&this->mutex);
   if (this->file)
      this->file->newDataReaderCreated();
}

void Chunk::dataWriterDeleted()
{
   QMutexLocker locker(&this->mutex);
   if (this->file)
      this->file->dataWriterDeleted();
}

void Chunk::dataReaderDeleted()
{
   QMutexLocker locker(&this->mutex);
   if (this->file)
      this->file->dataReaderDeleted();
}

void Chunk::fileDeleted()
{
   QMutexLocker locker(&this->mutex);
   this->file = 0;
}

/**
  * @exception IOErrorException
  * @exception ChunkDeletedException
  * @exception ChunkNotCompletedException
  */
int Chunk::read(char* buffer, int offset)
{
   QMutexLocker locker(&this->mutex);
   if (!this->file)
      throw ChunkDeletedException();

   if (this->knownBytes == 0)
      throw ChunkNotCompletedException();

   if (offset >= this->knownBytes)
      return 0;

   int bytesRemaining = this->getChunkSize() - offset;
   return this->file->read(buffer, offset + static_cast<qint64>(this->num) * CHUNK_SIZE, bytesRemaining >= this->BUFFER_SIZE ? this->BUFFER_SIZE : bytesRemaining);
}

/**
  * Write the given buffer after 'knownBytes'.
  * @exception IOErrorException
  * @exception ChunkDeletedException
  * @exception TryToWriteBeyondTheEndOfChunkException
  */
bool Chunk::write(const char* buffer, int nbBytes)
{
   //L_WARN(QString("Chunk::write, nbBytes = %1").arg(nbBytes));
   QMutexLocker locker(&this->mutex);
   if (!this->file)
      throw ChunkDeletedException();

   if (this->knownBytes + nbBytes > this->getChunkSize())
      throw TryToWriteBeyondTheEndOfChunkException();

   this->knownBytes += this->file->write(buffer, nbBytes, this->knownBytes + static_cast<qint64>(this->num) * CHUNK_SIZE);

   if (this->knownBytes > this->getChunkSize()) // Should never be true.
   {
      L_ERRO("Chunk::write : this->knownBytes > getChunkSize");
      this->knownBytes = this->getChunkSize();
   }
   bool complete = this->knownBytes == this->getChunkSize();

   if (complete)
      this->file->chunkComplete(this);

   return complete;
}

/*void Chunk::sendContentToSocket(QAbstractSocket& socket)
{
}

void Chunk::getContentFromSocket(QAbstractSocket& socket)
{
}*/

int Chunk::getNum() const
{
   return this->num;
}

int Chunk::getNbTotalChunk() const
{
   QMutexLocker locker(&this->mutex);

   if (this->file)
      return this->file->getNbChunks();

   return 0;
}

bool Chunk::hasHash() const
{
   return !this->hash.isNull();
}

Common::Hash Chunk::getHash() const
{
   QMutexLocker locker(&this->mutex);
   return this->hash;
}

void Chunk::setHash(const Common::Hash& hash)
{
   L_DEBU(QString("Chunk[%1] setHash(..) : %2").arg(this->num).arg(hash.toStr()));

   if (!this->hash.isNull() && this->hash != hash)
      L_WARN(QString("Chunk::setHash : Hash chunk changed from %1 to %2 for the file %3").arg(this->hash.toStr()).arg(hash.toStr()).arg(this->file->getFullPath()));

   this->hash = hash;
}

int Chunk::getKnownBytes() const
{
   QMutexLocker locker(&this->mutex);
   return this->knownBytes;
}

void Chunk::setKnownBytes(int bytes)
{
   this->knownBytes = bytes;
}

int Chunk::getChunkSize() const
{
   if (!this->file)
      return 0;

   //L_WARN(QString("this->file->getNbChunks() = %1").arg(this->file->getNbChunks()));
   if (this->num < this->file->getNbChunks() - 1)
      return this->CHUNK_SIZE;

   int size = this->file->getSize() % this->CHUNK_SIZE;
   if (!size)
      return this->CHUNK_SIZE;
   else
      return size;
}

bool Chunk::isComplete() const
{
   QMutexLocker locker(&this->mutex);
   return this->knownBytes >= this->getChunkSize(); // Should be '==' but we are never 100% sure ;).
}

bool Chunk::isOwnedBy(File* file) const
{
   return this->file == file;
}

QString Chunk::toStr() const
{
   return QString("num = [%1], hash = %2, knownBytes = %3, size = %4").arg(this->num).arg(this->getHash().toStr()).arg(this->getKnownBytes()).arg(this->getChunkSize());
}
