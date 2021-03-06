// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2012-2016 - TortoiseGit
// Copyright (C) 2003-2008, 2013-2015 - TortoiseSVN

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#include "stdafx.h"
#include "PathUtils.h"
#include <memory>

BOOL CPathUtils::MakeSureDirectoryPathExists(LPCTSTR path)
{
	size_t len = _tcslen(path) + 10;
	auto buf = std::make_unique<TCHAR[]>(len);
	auto internalpathbuf = std::make_unique<TCHAR[]>(len);
	TCHAR * pPath = internalpathbuf.get();
	SECURITY_ATTRIBUTES attribs = { 0 };
	attribs.nLength = sizeof(SECURITY_ATTRIBUTES);
	attribs.bInheritHandle = FALSE;

	ConvertToBackslash(internalpathbuf.get(), path, len);
	do
	{
		SecureZeroMemory(buf.get(), (len)*sizeof(TCHAR));
		TCHAR * slashpos = _tcschr(pPath, '\\');
		if (slashpos)
			_tcsncpy_s(buf.get(), len, internalpathbuf.get(), slashpos - internalpathbuf.get());
		else
			_tcsncpy_s(buf.get(), len, internalpathbuf.get(), len);
		CreateDirectory(buf.get(), &attribs);
		pPath = _tcschr(pPath, '\\');
	} while ((pPath++)&&(_tcschr(pPath, '\\')));

	return CreateDirectory(internalpathbuf.get(), &attribs);
}

void CPathUtils::Unescape(char * psz)
{
	char * pszSource = psz;
	char * pszDest = psz;

	static const char szHex[] = "0123456789ABCDEF";

	// Unescape special characters. The number of characters
	// in the *pszDest is assumed to be <= the number of characters
	// in pszSource (they are both the same string anyway)

	while (*pszSource != '\0' && *pszDest != '\0')
	{
		if (*pszSource == '%')
		{
			// The next two chars following '%' should be digits
			if ( *(pszSource + 1) == '\0' ||
				*(pszSource + 2) == '\0' )
			{
				// nothing left to do
				break;
			}

			char nValue = '?';
			const char* pszLow = nullptr;
			const char* pszHigh = nullptr;
			++pszSource;

			*pszSource = (char) toupper(*pszSource);
			pszHigh = strchr(szHex, *pszSource);

			if (pszHigh)
			{
				++pszSource;
				*pszSource = (char) toupper(*pszSource);
				pszLow = strchr(szHex, *pszSource);

				if (pszLow)
				{
					nValue = (char) (((pszHigh - szHex) << 4) +
						(pszLow - szHex));
				}
			}
			else
			{
				pszSource--;
				nValue = *pszSource;
			}
			*pszDest++ = nValue;
		}
		else
			*pszDest++ = *pszSource;

		++pszSource;
	}

	*pszDest = '\0';
}

static const char iri_escape_chars[256] = {
	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,

	/* 128 */
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0
};

const char uri_autoescape_chars[256] = {
	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 0, 0, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 1, 0, 0,

	/* 64 */
	1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 0, 1,
	0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 1, 0,

	/* 128 */
	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,

	/* 192 */
	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
};

static const char uri_char_validity[256] = {
	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 0, 0, 1, 0, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 1, 0, 0,

	/* 64 */
	1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 0, 1,
	0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 1, 0,

	/* 128 */
	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,

	/* 192 */
	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
};


void CPathUtils::ConvertToBackslash(LPTSTR dest, LPCTSTR src, size_t len)
{
	_tcscpy_s(dest, len, src);
	TCHAR * p = dest;
	for (; *p != '\0'; ++p)
		if (*p == '/')
			*p = '\\';
}

CStringA CPathUtils::PathEscape(const CStringA& path)
{
	CStringA ret2;
	int c;
	int i;
	for (i=0; path[i]; ++i)
	{
		c = (unsigned char)path[i];
		if (iri_escape_chars[c])
		{
			// no escaping needed for that char
			ret2 += (unsigned char)path[i];
		}
		else // char needs escaping
			ret2.AppendFormat("%%%02X", (unsigned char)c);
	}
	CStringA ret;
	for (i=0; ret2[i]; ++i)
	{
		c = (unsigned char)ret2[i];
		if (uri_autoescape_chars[c])
		{
			// no escaping needed for that char
			ret += (unsigned char)ret2[i];
		}
		else // char needs escaping
			ret.AppendFormat("%%%02X", (unsigned char)c);
	}

	if ((ret.Left(11).Compare("file:///%5C") == 0) && (ret.Find('%', 12) < 0))
		ret.Replace(("file:///%5C"), ("file://"));
	ret.Replace(("file:////%5C"), ("file://"));

	return ret;
}

#ifdef CSTRING_AVAILABLE
CString CPathUtils::GetFileNameFromPath(CString sPath)
{
	CString ret;
	sPath.Replace(_T("/"), _T("\\"));
	ret = sPath.Mid(sPath.ReverseFind('\\') + 1);
	return ret;
}

CString CPathUtils::GetFileExtFromPath(const CString& sPath)
{
	int dotPos = sPath.ReverseFind('.');
	int slashPos = sPath.ReverseFind('\\');
	if (slashPos < 0)
		slashPos = sPath.ReverseFind('/');
	if (dotPos > slashPos)
		return sPath.Mid(dotPos);
	return CString();
}

CString CPathUtils::GetLongPathname(const CString& path)
{
	if (path.IsEmpty())
		return path;
	TCHAR pathbufcanonicalized[MAX_PATH] = {0}; // MAX_PATH ok.
	DWORD ret = 0;
	CString sRet;
	if (!PathIsURL(path) && PathIsRelative(path))
	{
		ret = GetFullPathName(path, 0, nullptr, nullptr);
		if (ret)
		{
			auto pathbuf = std::make_unique<TCHAR[]>(ret + 1);
			if ((ret = GetFullPathName(path, ret, pathbuf.get(), nullptr)) != 0)
				sRet = CString(pathbuf.get(), ret);
		}
	}
	else if (PathCanonicalize(pathbufcanonicalized, path))
	{
		ret = ::GetLongPathName(pathbufcanonicalized, nullptr, 0);
		if (ret == 0)
			return path;
		auto pathbuf = std::make_unique<TCHAR[]>(ret + 2);
		ret = ::GetLongPathName(pathbufcanonicalized, pathbuf.get(), ret + 1);
		sRet = CString(pathbuf.get(), ret);
	}
	else
	{
		ret = ::GetLongPathName(path, nullptr, 0);
		if (ret == 0)
			return path;
		auto pathbuf = std::make_unique<TCHAR[]>(ret + 2);
		ret = ::GetLongPathName(path, pathbuf.get(), ret + 1);
		sRet = CString(pathbuf.get(), ret);
	}
	if (ret == 0)
		return path;
	return sRet;
}

BOOL CPathUtils::FileCopy(CString srcPath, CString destPath, BOOL force)
{
	srcPath.Replace('/', '\\');
	destPath.Replace('/', '\\');
	CString destFolder = destPath.Left(destPath.ReverseFind('\\'));
	MakeSureDirectoryPathExists(destFolder);
	return (CopyFile(srcPath, destPath, !force));
}

CString CPathUtils::ParsePathInString(const CString& Str)
{
	CString sToken;
	int curPos = 0;
	sToken = Str.Tokenize(_T("'\t\r\n"), curPos);
	while (!sToken.IsEmpty())
	{
		if ((sToken.Find('/')>=0)||(sToken.Find('\\')>=0))
		{
			sToken.Trim(_T("'\""));
			return sToken;
		}
		sToken = Str.Tokenize(_T("'\t\r\n"), curPos);
	}
	sToken.Empty();
	return sToken;
}

CString CPathUtils::GetAppDirectory(HMODULE hMod /* = nullptr */)
{
    CString path;
    DWORD len = 0;
    DWORD bufferlen = MAX_PATH;     // MAX_PATH is not the limit here!
    path.GetBuffer(bufferlen);
    do
    {
        bufferlen += MAX_PATH;      // MAX_PATH is not the limit here!
        path.ReleaseBuffer(0);
        len = GetModuleFileName(hMod, path.GetBuffer(bufferlen+1), bufferlen);
    } while(len == bufferlen);
    path.ReleaseBuffer();
    path = path.Left(path.ReverseFind('\\')+1);
    return GetLongPathname(path);
}

CString CPathUtils::GetAppParentDirectory(HMODULE hMod /* = nullptr */)
{
    CString path = GetAppDirectory(hMod);
    path = path.Left(path.ReverseFind('\\'));
    path = path.Left(path.ReverseFind('\\')+1);
    return path;
}

CString CPathUtils::GetAppDataDirectory()
{
	PWSTR pszPath = nullptr;
	if (SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE, nullptr, &pszPath) != S_OK)
		return CString();

	CString path = pszPath;
	CoTaskMemFree(pszPath);
	path += L"\\TortoiseGit";
	if (!PathIsDirectory(path))
		CreateDirectory(path, nullptr);

	path += _T('\\');
	return path;
}

CString CPathUtils::GetLocalAppDataDirectory()
{
	PWSTR pszPath = nullptr;
	if (SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &pszPath) != S_OK)
		return CString();
	CString path = pszPath;
	CoTaskMemFree(pszPath);
	path += L"\\TortoiseGit";
	if (!PathIsDirectory(path))
		CreateDirectory(path, nullptr);

	path += _T('\\');
	return path;
}

CStringA CPathUtils::PathUnescape(const CStringA& path)
{
	auto urlabuf = std::make_unique<char[]>(path.GetLength() + 1);

	strcpy_s(urlabuf.get(), path.GetLength()+1, path);
	Unescape(urlabuf.get());

	return urlabuf.get();
}

CStringW CPathUtils::PathUnescape(const CStringW& path)
{
	char * buf;
	CStringA patha;
	int len = path.GetLength();
	if (len==0)
		return CStringW();
	buf = patha.GetBuffer(len*4 + 1);
	int lengthIncTerminator = WideCharToMultiByte(CP_UTF8, 0, path, -1, buf, len * 4, nullptr, nullptr);
	patha.ReleaseBuffer(lengthIncTerminator-1);

	patha = PathUnescape(patha);

	WCHAR * bufw;
	len = patha.GetLength();
	bufw = new WCHAR[len*4 + 1];
	SecureZeroMemory(bufw, (len*4 + 1)*sizeof(WCHAR));
	MultiByteToWideChar(CP_UTF8, 0, patha, -1, bufw, len*4);
	CStringW ret = CStringW(bufw);
	delete [] bufw;
	return ret;
}
#ifdef _MFC_VER
CString CPathUtils::GetVersionFromFile(const CString & p_strFilename)
{
	struct TRANSARRAY
	{
		WORD wLanguageID;
		WORD wCharacterSet;
	};

	CString strReturn;
	DWORD dwReserved = 0;
	DWORD dwBufferSize = GetFileVersionInfoSize((LPTSTR)(LPCTSTR)p_strFilename,&dwReserved);

	if (dwBufferSize > 0)
	{
		auto pBuffer = std::make_unique<BYTE[]>(dwBufferSize);

		if (pBuffer)
		{
			UINT        nInfoSize = 0,
						nFixedLength = 0;
			LPSTR       lpVersion = nullptr;
			VOID*       lpFixedPointer;
			TRANSARRAY* lpTransArray;
			CString     strLangProductVersion;

			GetFileVersionInfo((LPTSTR)(LPCTSTR)p_strFilename,
				dwReserved,
				dwBufferSize,
				pBuffer.get());

			// Check the current language
			VerQueryValue(pBuffer.get(),
				_T("\\VarFileInfo\\Translation"),
				&lpFixedPointer,
				&nFixedLength);
			lpTransArray = (TRANSARRAY*) lpFixedPointer;

			strLangProductVersion.Format(_T("\\StringFileInfo\\%04x%04x\\ProductVersion"),
				lpTransArray[0].wLanguageID, lpTransArray[0].wCharacterSet);

			VerQueryValue(pBuffer.get(),
				(LPTSTR)(LPCTSTR)strLangProductVersion,
				(LPVOID *)&lpVersion,
				&nInfoSize);
			if (nInfoSize && lpVersion)
				strReturn = (LPCTSTR)lpVersion;
		}
	}

	return strReturn;
}
#endif
CString CPathUtils::PathPatternEscape(const CString& path)
{
	CString result = path;
	// first remove already escaped patterns to avoid having those
	// escaped twice
	result.Replace(_T("\\["), _T("["));
	result.Replace(_T("\\]"), _T("]"));
	// now escape the patterns (again)
	result.Replace(_T("["), _T("\\["));
	result.Replace(_T("]"), _T("\\]"));
	return result;
}

CString CPathUtils::PathPatternUnEscape(const CString& path)
{
	CString result = path;
	result.Replace(_T("\\["), _T("["));
	result.Replace(_T("\\]"), _T("]"));
	return result;
}

#endif
