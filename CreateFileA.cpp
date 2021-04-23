#include <iostream>
#include <Windows.h>

class shared_handle {
public:
	shared_handle(HANDLE handle) : m_handle{ handle }, m_counter{ 1 } {
	}

	shared_handle(const shared_handle& value) : m_handle{ value.m_handle }, m_counter{ value.m_counter } {
		++m_counter;
	}

	shared_handle& operator=(const shared_handle& value) {
		m_handle = value.m_handle;
		m_counter = value.m_counter;

		++m_counter;
		return *this;
	}

	~shared_handle() {
		if (--m_counter == 0) {
			CloseHandle(m_handle);
		}
	}

	auto get() {
		return m_handle;
	}

	auto use_count() {
		return m_counter;
	}

private:
	HANDLE m_handle;
	int m_counter;
};

class WinException {
public:
	void PrintErrorMessage() {
		DWORD errorCode = GetLastError();
		wchar_t errorBuff[80];

		FormatMessageW(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			errorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			errorBuff,
			(sizeof(errorBuff) / sizeof(wchar_t)),
			NULL);

		std::wcout << "System Error: " << errorBuff << std::endl;
	}
};

int main() {
	WinException log;

	wchar_t sourceFileName[80];
	wchar_t destFileName[80];

	std::wcout << "Enter source file name: ";
	std::wcin >> sourceFileName;

	std::wcout << "Enter destination file name: ";
	std::wcin >> destFileName;

	DWORD dwBytesRead, dwBytesWritten, dwPos;
	BYTE buff[4096] = {};

	shared_handle fileSource(CreateFileW(sourceFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
	if (fileSource.get() == INVALID_HANDLE_VALUE) {
		std::wcout << "Could not open file: " << sourceFileName << std::endl;
		log.PrintErrorMessage();
		return -1;
	}

	auto readResult = ReadFile(fileSource.get(), buff, sizeof(buff), &dwBytesRead, NULL);

	shared_handle fileDest(CreateFileW(destFileName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL));
	if (fileDest.get() == INVALID_HANDLE_VALUE) {
		std::wcout << "Could not open file:" << destFileName;
		log.PrintErrorMessage();
		return -1;
	}

	WriteFile(fileDest.get(), buff, dwBytesRead, 0, 0);
}