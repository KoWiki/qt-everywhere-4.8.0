/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qunicodetables_p.h"
#include "qlibrary.h"
#include "qtextcodec.h"

#include "qharfbuzz_p.h"

QT_USE_NAMESPACE

extern "C" {

HB_GraphemeClass HB_GetGraphemeClass(HB_UChar32 ch)
{
    const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch);
    return (HB_GraphemeClass) prop->graphemeBreak;
}

HB_WordClass HB_GetWordClass(HB_UChar32 ch)
{
    const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch);
    return (HB_WordClass) prop->wordBreak;
}

HB_SentenceClass HB_GetSentenceClass(HB_UChar32 ch)
{
    const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch);
    return (HB_SentenceClass) prop->sentenceBreak;
}

HB_LineBreakClass HB_GetLineBreakClass(HB_UChar32 ch)
{
    return (HB_LineBreakClass)QUnicodeTables::lineBreakClass(ch);
}


void HB_GetGraphemeAndLineBreakClass(HB_UChar32 ch, HB_GraphemeClass *grapheme, HB_LineBreakClass *lineBreak)
{
    const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch);
    *grapheme = (HB_GraphemeClass) prop->graphemeBreak;
    *lineBreak = (HB_LineBreakClass) prop->line_break_class;
}

void HB_GetUnicodeCharProperties(HB_UChar32 ch, HB_CharCategory *category, int *combiningClass)
{
    const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch);
    *category = (HB_CharCategory)prop->category;
    *combiningClass = prop->combiningClass;
}

HB_CharCategory HB_GetUnicodeCharCategory(HB_UChar32 ch)
{
    return (HB_CharCategory)QChar::category(ch);
}

int HB_GetUnicodeCharCombiningClass(HB_UChar32 ch)
{
    return QChar::combiningClass(ch);
}

HB_UChar16 HB_GetMirroredChar(HB_UChar16 ch)
{
    return QChar::mirroredChar(ch);
}

void *HB_Library_Resolve(const char *library, int version, const char *symbol)
{
#ifdef QT_NO_LIBRARY
    return 0;
#else
    return QLibrary::resolve(QLatin1String(library), version, symbol);
#endif
}

} // extern "C"

QT_BEGIN_NAMESPACE

HB_Bool qShapeItem(HB_ShaperItem *item)
{
    return HB_ShapeItem(item);
}

HB_Face qHBNewFace(void *font, HB_GetFontTableFunc tableFunc)
{
    return HB_NewFace(font, tableFunc);
}

void qHBFreeFace(HB_Face face)
{
    HB_FreeFace(face);
}

void qGetCharAttributes(const HB_UChar16 *string, hb_uint32 stringLength,
                        const HB_ScriptItem *items, hb_uint32 numItems,
                        HB_CharAttributes *attributes)
{
    HB_GetCharAttributes(string, stringLength, items, numItems, attributes);
}

int qSubstituteWithVerticalVariants(HB_FaceRec_* hbFace, quint32 *glyphs, const unsigned length)
{
    if(!hbFace) {
        return 0xffff;
    }

    HB_Buffer buffer;
    hb_buffer_new(&buffer);
    for (unsigned i = 0; i < length; ++i) {
        hb_buffer_add_glyph(buffer, glyphs[i], 0, i);
    }

    HB_UShort scriptIndex;
    HB_UShort featureIndex;
    if (HB_GSUB_Select_Script(hbFace->gsub, HB_MAKE_TAG('D', 'F', 'L', 'T'), &scriptIndex) != HB_Err_Ok ){
        if (HB_GSUB_Select_Script(hbFace->gsub, HB_MAKE_TAG('h', 'a', 'n', 'i'), &scriptIndex) != HB_Err_Ok ){
            int error = HB_GSUB_Select_Script(hbFace->gsub, HB_MAKE_TAG('k', 'a', 'n', 'a'), &scriptIndex);
            if (error != HB_Err_Ok)
                return error;
        }
    }

    if (HB_GSUB_Select_Feature(hbFace->gsub, HB_MAKE_TAG('v', 'e', 'r', 't'), scriptIndex, 0xffff, &featureIndex) != HB_Err_Ok) {
        int error = HB_GSUB_Select_Feature(hbFace->gsub, HB_MAKE_TAG('v', 'r', 't', '2'), scriptIndex, 0xffff, &featureIndex) != HB_Err_Ok;
        if (error != HB_Err_Ok)
            return error;
    }

    HB_GSUB_Add_Feature(hbFace->gsub, featureIndex, 1);

    int error = HB_GSUB_Apply_String(hbFace->gsub, buffer);
    if (!error){
        for (unsigned i = 0; i < length; ++i) {
            glyphs[i] = buffer->out_string[i].gindex;
        }
    }
    HB_GSUB_Clear_Features(hbFace->gsub);
    return error;
}

bool qHasVerticalGlyphs(HB_FaceRec_* hbFace)
{
    if(!hbFace)
        return false;

    HB_UShort scriptIndex;
    HB_UShort featureIndex;

    if (HB_GSUB_Select_Script(hbFace->gsub, HB_MAKE_TAG('D', 'F', 'L', 'T'), &scriptIndex) == HB_Err_Ok
        || HB_GSUB_Select_Script(hbFace->gsub, HB_MAKE_TAG('h', 'a', 'n', 'i'), &scriptIndex) == HB_Err_Ok
        || HB_GSUB_Select_Script(hbFace->gsub, HB_MAKE_TAG('k', 'a', 'n', 'a'), &scriptIndex) == HB_Err_Ok )
    {
        return HB_GSUB_Select_Feature(hbFace->gsub, HB_MAKE_TAG('v', 'e', 'r', 't'), scriptIndex, 0xffff, &featureIndex) == HB_Err_Ok
                || HB_GSUB_Select_Feature(hbFace->gsub, HB_MAKE_TAG('v', 'r', 't', '2'), scriptIndex, 0xffff, &featureIndex) == HB_Err_Ok;
    }
    return false;
}


QT_END_NAMESPACE
