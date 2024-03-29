/****************************************************************************
**
** Copyright (C) 1992-2005 Trolltech AS. All rights reserved.
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsharedmemory_p.h"

#if !defined(QT_QWS_NO_SHM)

#include <sys/shm.h>



QSharedMemory::QSharedMemory()
    : shmBase(0), shmSize(0), character(0),  shmId(-1), key(-1)
{
}


QSharedMemory::~QSharedMemory()
{
    detach();
}


bool QSharedMemory::create(int size)
{
    if (shmId != -1)
        detach();
    shmId = shmget(IPC_PRIVATE, size, IPC_CREAT|0600);

    if (shmId == -1) {
#ifdef QT_SHM_DEBUG
        perror("QWSBackingStore::create allocating shared memory");
        qWarning("Error allocating shared memory of size %d", datasize);
#endif
        return false;
    }
    shmBase = shmat(shmId,0,0);
    shmctl(shmId, IPC_RMID, 0);
    if (shmBase == (void*)-1) {
#ifdef QT_SHM_DEBUG
        perror("QWSBackingStore::create attaching to shared memory");
        qWarning("Error attaching to shared memory");
#endif
        shmBase = 0;
        return false;
    }
    return true;
}

bool QSharedMemory::attach(int id)
{
    if (shmId == id)
        return id != -1;
    if (shmId != -1)
        detach();

    shmBase = shmat(id,0,0);
    if (shmBase == (void*)-1) {
#ifdef QT_SHM_DEBUG
        perror("QWSBackingStore::attach attaching to shared memory");
        qWarning("Error attaching to shared memory 0x%x of size %d",
                 id, s.width() * s.height());
#endif
        shmBase = 0;
        return false;
    }
    shmId = id;
    return true;
}


void QSharedMemory::detach ()
{
    if (!shmBase)
        return;
    shmdt (shmBase);
    shmBase = 0;
    shmSize = 0;
    shmId = -1;
}

void QSharedMemory::setPermissions (mode_t mode)
{
  struct shmid_ds shm;
  shmctl (shmId, IPC_STAT, &shm);
  shm.shm_perm.mode = mode;
  shmctl (shmId, IPC_SET, &shm);
}

int QSharedMemory::size () const
{
    struct shmid_ds shm;
    shmctl (shmId, IPC_STAT, &shm);
    return shm.shm_segsz;
}


// old API



QSharedMemory::QSharedMemory (int size, const QString &filename, char c)
{
  shmSize = size;
  shmFile = filename;
  shmBase = 0;
  shmId = -1;
  character = c;
  key = ftok (shmFile.toLatin1().constData(), c);
}



bool QSharedMemory::create ()
{
  shmId = shmget (key, shmSize, IPC_CREAT | 0666);
  if (shmId == -1)
    return false;
  else
    return true;
}

void QSharedMemory::destroy ()
{
  struct shmid_ds shm;
  shmctl (shmId, IPC_RMID, &shm);
}

bool QSharedMemory::attach ()
{
  if (shmId == -1)
    shmId = shmget (key, shmSize, 0);

  shmBase = shmat (shmId, 0, 0);

  return (long)shmBase != -1 && shmBase != 0;
}


#endif
