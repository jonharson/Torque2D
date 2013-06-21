//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef _TAML_CSVDOCUMENT_H_
#define _TAML_CSVDOCUMENT_H_

#ifndef _FILESTREAM_H_
	#include "io/filestream.h"
#endif

#ifndef _HASHTABLE_H
	#include "collection/HashTable.h"
#endif

//-----------------------------------------------------------------------------

class CsvElement
{
public:
    CsvElement(void) {}
    CsvElement(const char* name) {}
    ~CsvElement(void) {}
};

class CsvHeader
{
public:
    CsvHeader(void) {}
    ~CsvHeader(void) {}
};

class CsvDocument
{
public:
    CsvDocument(void) {}
    ~CsvDocument(void) {}

	bool LoadFile(const char* _filename);
	bool LoadFile(FileStream &stream);

	CsvElement* nextElement(void);

private:
	CsvHeader mHeader;

	HashMap<const U32, CsvElement*> mElements;
	HashMap<const U32, bool> mProcessedElements;
};

#endif // _TAML_CSVDOCUMENT_H_