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

#ifndef QSTACK_H
#define QSTACK_H

#include <QtCore/qvector.h>

QT_MODULE(Core)

template<class T>
class QStack : public QVector<T>
{
public:
    inline QStack() {}
    inline ~QStack() {}
    inline void push(const T &t) { append(t); }
    T pop();
    T &top();
    const T &top() const;
};

template<class T>
inline T QStack<T>::pop()
{ Q_ASSERT(!this->isEmpty()); T t = this->data()[this->size() -1];
  this->resize(this->size()-1); return t; }

template<class T>
inline T &QStack<T>::top()
{ Q_ASSERT(!this->isEmpty()); this->detach(); return this->data()[this->size()-1]; }

template<class T>
inline const T &QStack<T>::top() const
{ Q_ASSERT(!this->isEmpty()); return this->data()[this->size()-1]; }

#endif // QSTACK_H
