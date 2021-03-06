#include <QtCore/QtCore> // Only for the Q_OS_* defines.

#if !defined(DIRWATCHERWIN_H) and defined(Q_OS_WIN32)
#define DIRWATCHERWIN_H

#include <DirWatcher.h>

#include <windows.h>

static const int notifyBufferSize = 2048;

class DirWatcherWin : public DirWatcher
{
public:
    DirWatcherWin();
    ~DirWatcherWin();
    
   void addDir(const QString& path);
   void rmDir(const QString& path);
   const QList<WatcherEvent> waitEvent();
   const QList<WatcherEvent> waitEvent(int timeout);
    
private:   
   void watch(int num);
   
   struct Dir
   {
      Dir(HANDLE file, HANDLE event) : file(file), event(event) {}
      HANDLE file;
      HANDLE event;
   };
   QList<Dir> dirs; ///< The watched dirs.
   
   // Is this data can be shares among some 'ReadDirectoryChangesW'?
   char notifyBuffer[notifyBufferSize];
   DWORD nbBytesNotifyBuffer;
};


#endif
