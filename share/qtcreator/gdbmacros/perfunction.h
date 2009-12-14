/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://www.qtsoftware.com/contact.
**
**************************************************************************/

#ifndef PERFUNCTION_H
#define PERFUNCTION_H

#include <qglobal.h>
#include <QByteArray>

namespace Performance {
    namespace Internal {
        enum ActivationFlags {
            AF_None        = 0x0000,
            AF_PaintDebug  = 0x0001,
        };

        // encode
        // TODO: make this ARCH/bit/endian resistant
        static QByteArray marshallMessage(quint32 code1, quint32 code2, const QByteArray &data)
        {
            // create the message holder
            QByteArray message;
            quint32 messageSize = 3 * sizeof(quint32) + data.size();
            quint32 offset = 0;
            message.resize(messageSize);

            // copy payload (size, code1, code2, data)
            memcpy(message.data() + offset, &messageSize, sizeof(quint32));
            offset += sizeof(quint32);
            memcpy(message.data() + offset, &code1, sizeof(quint32));
            offset += sizeof(quint32);
            memcpy(message.data() + offset, &code2, sizeof(quint32));
            offset += sizeof(quint32);
            memcpy(message.data() + offset, data.data(), data.size());
            offset += data.size();

            return message;
        }

        // decode (optimize this)
        static quint32 messageLength(const QByteArray &data)
        {
            if (data.size() < (int)sizeof(quint32))
                return 0;
            quint32 length = 0;
            memcpy(&length, data.data(), sizeof(quint32));
            return length;
        }

        static bool demarshallMessage(const QByteArray &marshalled, quint32 *code1, quint32 *code2, QByteArray *data)
        {
            // safety checks
            quint32 messageSize = messageLength(marshalled);
            if (!messageSize || marshalled.length() < (int)messageSize) {
                qWarning("demarshallMessage: message is not complete %d %d", marshalled.length(), messageSize);
                return false;
            }
#if 0
            else if (marshalled.length() > (int)messageSize)
                qWarning("demarshallMessage: data is too long for the message. check logic!");
            else
                qWarning("demarshallMessage: ok %d", messageSize);
#endif

            // read data
            quint32 offset = sizeof(quint32);
            if (code1)
                memcpy(code1, marshalled.data() + offset, sizeof(quint32));
            offset += sizeof(quint32);
            if (code2)
                memcpy(code2, marshalled.data() + offset, sizeof(quint32));
            offset += sizeof(quint32);
            if (data) {
                data->resize(messageSize - offset);
                memcpy(data->data(), marshalled.data() + offset, messageSize - offset);
            }
            return true;
        }
    }
}

#endif // PERFUNCTION_H
