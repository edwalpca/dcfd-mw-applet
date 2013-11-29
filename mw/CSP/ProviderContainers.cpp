/* ****************************************************************************

 * eID Middleware Project.
 * Copyright (C) 2008-2009 FedICT.
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version
 * 3.0 as published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, see
 * http://www.gnu.org/licenses/.

**************************************************************************** */
#include "ProviderContainers.h"
#include "csputil.h"
#include "cspdefines.h"
#include "../common/src/Util.h"

#ifdef UNICODE
#undef UNICODE
#endif

using namespace eIDMW;

CProviderContainers::CProviderContainers()
{
	m_bContainersListed = false;
	m_index = 0;
}

BOOL CProviderContainers::EnumContainers(
	LPBYTE pbData, LPDWORD pcbDataLen, DWORD dwFlags)
{
	BOOL bReturnVal = FALSE;

	if (!m_bContainersListed)
		ListContainers();

	if (dwFlags & CRYPT_FIRST)
		m_index = 0;

	if (m_index >= m_containinerNames.size())
		SetLastError(ERROR_NO_MORE_ITEMS);
	else
	{
		std::string csContainerName = m_containinerNames.at(m_index);
		DWORD dwLen = (DWORD) csContainerName.size() + 1;
		if (pbData == NULL)
		{
			// Normally, we should here return the maximum container
			// name length, but we'll just add 100 bytes to this one.
			*pcbDataLen = dwLen + 100;
			bReturnVal = TRUE;
		}
		else if (*pcbDataLen < dwLen)
			SetLastError(ERROR_MORE_DATA);
		else
		{
			memcpy(pbData, csContainerName.c_str(), dwLen);
			*pcbDataLen = dwLen;
			bReturnVal = TRUE;
			m_index++;
		}
	}

	return bReturnVal;
}

BOOL CProviderContainers::ContainerExists(LPCSTR szContainer)
{
#ifdef FOR_UNIT_TESTS_ONLY
	return TRUE;
#endif

	BOOL bFound = FALSE;

	if (!m_bContainersListed)
		ListContainers();

	std::string strContainer = szContainer;

	for (size_t i = 0; i < m_containinerNames.size() && !bFound; i++)
		bFound = 0 == strContainer.compare(m_containinerNames.at(i));

	return bFound;
}

BOOL CProviderContainers::ListContainersGetPublicKeyInfo(
	const wchar_t *wsName, CERT_PUBLIC_KEY_INFO *pPubKeyInfo)
{
	HCERTSTORE hCertStore;
	PCCERT_CONTEXT pCertContext = NULL;
	BOOL bSucceeded = FALSE;

	hCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL,
		CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_READONLY_FLAG, L"MY");
	if (hCertStore != NULL)
	{
		while (pCertContext = CertEnumCertificatesInStore(hCertStore, pCertContext))
		{
			DWORD cbData;
			BOOL bRet = CertGetCertificateContextProperty (pCertContext,
				CERT_KEY_PROV_INFO_PROP_ID, NULL, &cbData);
			if (bRet)
			{
				CRYPT_KEY_PROV_INFO *pProvInfo = (CRYPT_KEY_PROV_INFO*) malloc(cbData);
				if (pProvInfo)
				{
					memset(pProvInfo, 0, sizeof(CRYPT_KEY_PROV_INFO));
					bRet = CertGetCertificateContextProperty(pCertContext,
						CERT_KEY_PROV_INFO_PROP_ID, pProvInfo, &cbData);
					if (bRet && pProvInfo->pwszProvName != NULL && pProvInfo->pwszContainerName != NULL)
					{
						const wchar_t *test = pProvInfo->pwszProvName;
						int x = 0;
					}
					if (bRet && pProvInfo != NULL)
					{
						if (pPubKeyInfo == NULL)
						{
							if (pProvInfo->pwszProvName != NULL && wcscmp(pProvInfo->pwszProvName, wsName) == 0)
							{
								std::string strContainerName = utilStringNarrow(pProvInfo->pwszContainerName);
								m_containinerNames.push_back(strContainerName);
								bSucceeded = TRUE;
							}
						}
						else
						{
							if (pProvInfo->pwszContainerName != NULL && wcscmp(pProvInfo->pwszContainerName, wsName) == 0)
								bSucceeded = DuplicatePubKeyInfo(pPubKeyInfo, &pCertContext->pCertInfo->SubjectPublicKeyInfo);
						}
						/*
						&& pProvInfo->pwszProvName != NULL && pProvInfo->pwszContainerName != NULL &&
						wcscmp(pProvInfo->pwszProvName, wsName) == 0)
					{
						if (pPubKeyInfo == NULL)
						{
							std::string strContainerName = utilStringNarrow(pProvInfo->pwszContainerName);
							m_containinerNames.push_back(strContainerName);
							bSucceeded = TRUE;
						}
						else
						{
							bSucceeded = DuplicatePubKeyInfo(pPubKeyInfo, &pCertContext->pCertInfo->SubjectPublicKeyInfo);
						}
						*/
					}
				}

				if (pProvInfo)
					free(pProvInfo);
            }
		}

		CertCloseStore (hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
	}

	return bSucceeded;
}

void CProviderContainers::ListContainers()
{
	CAutoMutex oAutoMutex(&m_oProvContMutex);

	std::wstring strCSPName = utilStringWiden(CSP_NAME);
	const wchar_t *wsCSPName = strCSPName.c_str();

	ListContainersGetPublicKeyInfo(wsCSPName, NULL);

	m_bContainersListed = TRUE;
}

#ifdef FOR_UNIT_TESTS_ONLY
BOOL GetUnitTestPublicKeyInfo(const std::string & csContainerName,
	CERT_PUBLIC_KEY_INFO *pPubKeyInfo);
#endif

BOOL CProviderContainers::GetPublicKeyInfo(const std::string &csContainerName,
	CERT_PUBLIC_KEY_INFO *pPubKeyInfo)
{
#ifdef FOR_UNIT_TESTS_ONLY
	return GetUnitTestPublicKeyInfo(csContainerName, pPubKeyInfo);
#endif

	CAutoMutex oAutoMutex(&m_oProvContMutex);

	std::wstring strContainerName = utilStringWiden(csContainerName);
	const wchar_t *wcsContainerName = strContainerName.c_str();

	return 	ListContainersGetPublicKeyInfo(wcsContainerName, pPubKeyInfo);
}

// To allow the unit tests to run on any PC, we hardcode here the public
// key for the Authentication key of the card in "Emulated reader 0".
#ifdef FOR_UNIT_TESTS_ONLY
BOOL GetUnitTestPublicKeyInfo(const std::string & csContainerName,
	CERT_PUBLIC_KEY_INFO *pPubKeyInfo)
{
	const unsigned char tucCert[] =
	{
        0x30,0x82,0x03,0xf6,0x30,0x82,0x02,0xde,0xa0,0x03,0x02,0x01,0x02,0x02,0x10,0x01,
        0x00,0x00,0x00,0x00,0x01,0x15,0xa8,0xd3,0x59,0x66,0x00,0x00,0xce,0xc0,0x03,0x30,
        0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x05,0x05,0x00,0x30,0x37,
        0x31,0x1a,0x30,0x18,0x06,0x03,0x55,0x04,0x06,0x13,0x11,0x42,0x45,0x20,0x20,0x20,
        0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x31,0x19,0x30,0x17,
        0x06,0x03,0x55,0x04,0x03,0x13,0x10,0x5a,0x45,0x54,0x45,0x53,0x20,0x43,0x69,0x74,
        0x69,0x7a,0x65,0x6e,0x20,0x43,0x41,0x30,0x1e,0x17,0x0d,0x30,0x37,0x31,0x30,0x31,
        0x35,0x31,0x32,0x33,0x39,0x32,0x34,0x5a,0x17,0x0d,0x31,0x32,0x31,0x30,0x31,0x35,
        0x31,0x32,0x33,0x39,0x32,0x34,0x5a,0x30,0x6c,0x31,0x0b,0x30,0x09,0x06,0x03,0x55,
        0x04,0x06,0x13,0x02,0x42,0x45,0x31,0x26,0x30,0x24,0x06,0x03,0x55,0x04,0x03,0x13,
        0x1d,0x67,0x6e,0x61,0x6d,0x65,0x30,0x20,0x6e,0x61,0x6d,0x65,0x30,0x20,0x28,0x41,
        0x75,0x74,0x68,0x65,0x6e,0x74,0x69,0x63,0x61,0x74,0x69,0x6f,0x6e,0x29,0x31,0x0e,
        0x30,0x0c,0x06,0x03,0x55,0x04,0x04,0x13,0x05,0x6e,0x61,0x6d,0x65,0x30,0x31,0x0f,
        0x30,0x0d,0x06,0x03,0x55,0x04,0x2a,0x13,0x06,0x67,0x6e,0x61,0x6d,0x65,0x30,0x31,
        0x14,0x30,0x12,0x06,0x03,0x55,0x04,0x05,0x13,0x0b,0x30,0x30,0x30,0x30,0x30,0x30,
        0x30,0x30,0x30,0x30,0x30,0x30,0x81,0x9f,0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,
        0xf7,0x0d,0x01,0x01,0x01,0x05,0x00,0x03,0x81,0x8d,0x00,0x30,0x81,0x89,0x02,0x81,
        0x81,0x00,0xdd,0x15,0xe4,0xaa,0xd6,0x3b,0xea,0xef,0x8a,0x5c,0x1f,0xd9,0x86,0x29,
        0x04,0xbb,0x10,0xf8,0xce,0x9f,0x86,0x7d,0xaf,0x4f,0xba,0xca,0x24,0xaa,0xc1,0x18,
        0x27,0xae,0xd1,0xf6,0x60,0x3e,0x72,0x8c,0x7f,0x97,0xbc,0xe3,0x22,0xc6,0x47,0x4c,
        0xb4,0x7c,0x03,0x6f,0x4a,0x22,0xb3,0x7f,0x7e,0x13,0xd0,0xb4,0x26,0xc9,0x29,0xe7,
        0x82,0x72,0x55,0xaf,0x73,0x19,0x31,0x52,0xb8,0x49,0xa4,0xbe,0xdb,0x82,0x01,0x13,
        0xfd,0x32,0x87,0xb3,0x5f,0x53,0x2d,0x20,0x7d,0x17,0x9d,0x37,0x48,0xf4,0xcb,0xa8,
        0x1f,0x08,0x5d,0xf3,0x32,0x05,0x6a,0x7f,0xa3,0x2e,0xab,0x39,0x2b,0x5f,0xf1,0x43,
        0xd3,0x7e,0x33,0xb7,0x3c,0x43,0x3d,0xd3,0xec,0xb2,0x50,0x06,0x51,0x8b,0xb5,0x72,
        0xf8,0x9d,0x02,0x03,0x01,0x00,0x01,0xa3,0x82,0x01,0x4b,0x30,0x82,0x01,0x47,0x30,
        0x0e,0x06,0x03,0x55,0x1d,0x0f,0x01,0x01,0xff,0x04,0x04,0x03,0x02,0x07,0x80,0x30,
        0x45,0x06,0x03,0x55,0x1d,0x20,0x04,0x3e,0x30,0x3c,0x30,0x3a,0x06,0x07,0x60,0x38,
        0x01,0x01,0x01,0x02,0x02,0x30,0x2f,0x30,0x2d,0x06,0x08,0x2b,0x06,0x01,0x05,0x05,
        0x07,0x02,0x01,0x16,0x21,0x68,0x74,0x74,0x70,0x3a,0x2f,0x2f,0x72,0x65,0x70,0x6f,
        0x73,0x69,0x74,0x6f,0x72,0x79,0x2e,0x65,0x69,0x64,0x2e,0x7a,0x65,0x74,0x65,0x73,
        0x2e,0x62,0x65,0x20,0x20,0x20,0x30,0x3c,0x06,0x03,0x55,0x1d,0x1f,0x04,0x35,0x30,
        0x33,0x30,0x31,0xa0,0x2f,0xa0,0x2d,0x86,0x2b,0x68,0x74,0x74,0x70,0x3a,0x2f,0x2f,
        0x63,0x72,0x6c,0x2e,0x65,0x69,0x64,0x2e,0x7a,0x65,0x74,0x65,0x73,0x2e,0x62,0x65,
        0x2f,0x62,0x65,0x6c,0x67,0x69,0x75,0x6d,0x2f,0x62,0x65,0x6c,0x67,0x69,0x75,0x6d,
        0x2e,0x63,0x72,0x6c,0x30,0x7c,0x06,0x08,0x2b,0x06,0x01,0x05,0x05,0x07,0x01,0x01,
        0x04,0x70,0x30,0x6e,0x30,0x3f,0x06,0x08,0x2b,0x06,0x01,0x05,0x05,0x07,0x30,0x02,
        0x86,0x33,0x68,0x74,0x74,0x70,0x3a,0x2f,0x2f,0x77,0x77,0x77,0x2e,0x5a,0x65,0x74,
        0x65,0x73,0x43,0x61,0x72,0x64,0x73,0x2e,0x62,0x65,0x2f,0x64,0x75,0x6d,0x6d,0x79,
        0x2f,0x63,0x65,0x72,0x74,0x69,0x66,0x63,0x61,0x74,0x65,0x73,0x2f,0x74,0x65,0x73,
        0x74,0x2e,0x63,0x72,0x74,0x30,0x2b,0x06,0x08,0x2b,0x06,0x01,0x05,0x05,0x07,0x30,
        0x01,0x86,0x1f,0x68,0x74,0x74,0x70,0x3a,0x2f,0x2f,0x6f,0x63,0x73,0x70,0x2e,0x5a,
        0x65,0x74,0x65,0x73,0x43,0x61,0x72,0x64,0x73,0x2e,0x62,0x65,0x2f,0x64,0x75,0x6d,
        0x6d,0x79,0x30,0x11,0x06,0x09,0x60,0x86,0x48,0x01,0x86,0xf8,0x42,0x01,0x01,0x04,
        0x04,0x03,0x02,0x05,0xa0,0x30,0x1f,0x06,0x03,0x55,0x1d,0x23,0x04,0x18,0x30,0x16,
        0x80,0x14,0xed,0x2b,0xa5,0x67,0x16,0xb1,0xed,0x70,0xe1,0x5d,0x55,0x7a,0x11,0x12,
        0x16,0x1c,0x97,0x41,0x56,0x96,0x30,0x0d,0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,
        0x01,0x01,0x05,0x05,0x00,0x03,0x82,0x01,0x01,0x00,0x45,0x28,0x20,0xfb,0xa3,0xf1,
        0x04,0x25,0xbb,0x6c,0x4b,0x37,0x08,0xd1,0x54,0x99,0x1c,0x1c,0x28,0x4b,0xde,0x2e,
        0x53,0xf4,0x81,0x11,0xd1,0xb6,0x09,0x56,0xef,0x2c,0x30,0xfa,0x43,0x74,0x89,0xbc,
        0x05,0x06,0xc3,0xff,0x83,0x49,0xc4,0xea,0xc7,0x01,0x3e,0x67,0x75,0x74,0x71,0x59,
        0x0a,0x71,0xfb,0xf3,0xb7,0x10,0xd2,0x79,0x52,0x89,0x5f,0x1b,0xf8,0x58,0x61,0xfb,
        0xb5,0xec,0x09,0xe9,0x29,0x5c,0x1b,0x43,0x29,0x12,0x92,0x88,0xcc,0x70,0x97,0xd9,
        0xe0,0x81,0x61,0xa8,0x42,0xf8,0x88,0x57,0xf4,0x95,0x0a,0x01,0xc2,0xf4,0xa2,0xe7,
        0xfb,0x99,0xee,0xfa,0xbf,0x77,0x03,0xf9,0x6f,0xf4,0xad,0xa2,0xd9,0x01,0xd0,0x3c,
        0xdc,0x67,0xfe,0xa2,0x0b,0x82,0x7e,0x1e,0xcd,0xb2,0x45,0x4a,0x88,0x13,0xd1,0x2c,
        0x3e,0x0b,0xea,0xd0,0x2d,0x66,0xda,0x6b,0xad,0x6b,0x6e,0x5d,0x01,0x54,0x2f,0x56,
        0xdf,0xe4,0x1d,0x49,0x75,0xd5,0x1b,0x75,0xd6,0x84,0xf5,0x90,0xa9,0x68,0xce,0x87,
        0xb8,0xfe,0xb8,0x9f,0x4f,0x17,0xcb,0x62,0xbe,0xce,0x41,0x93,0x7b,0x43,0xcc,0xd5,
        0xff,0x26,0xfd,0xde,0xef,0xda,0xeb,0x0c,0x7f,0x14,0xb4,0xfd,0xe6,0xca,0x3a,0xcb,
        0xbf,0xc1,0xcc,0x71,0x43,0x1b,0x32,0xe8,0x37,0xa1,0x6d,0xe1,0xc3,0xd3,0x00,0xd5,
        0x9a,0xc1,0x38,0x92,0x71,0xb3,0xa5,0xba,0xd8,0xab,0x9f,0xb7,0x75,0xf6,0xd6,0x9a,
        0x95,0x8d,0xbb,0xe5,0x7a,0x15,0x2a,0x91,0x33,0x31,0x05,0x8a,0x4c,0x74,0xd3,0x95,
        0xbe,0x45,0xe6,0xe3,0x0e,0x64,0xc3,0xa4,0x6f,0x45,
	};

	BOOL bSucceeded = TRUE;

	const CERT_CONTEXT *pxCertContext =
		CertCreateCertificateContext(X509_ASN_ENCODING, tucCert, sizeof(tucCert));
	if (pxCertContext)
	{
		DuplicatePubKeyInfo(pPubKeyInfo, &pxCertContext->pCertInfo->SubjectPublicKeyInfo);
		CertFreeCertificateContext(pxCertContext);
	}
	else
		bSucceeded = FALSE;

	return bSucceeded;
}
#endif
