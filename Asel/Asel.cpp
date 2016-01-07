#define NOMINMAX
#include <Windows.h>
#undef NOMINMAX

#include "../Asel.h"

namespace asel {
	namespace impl {
		void releaseHandle(Handle* h) {
			::CloseHandle(*h);
		}
	} //asel::impl

	File connectServer(
		const String& serverName,
		ConnectMode mode,
		bool isRawName
		) {
		Handle pipe;

		pipe = ::CreateFile(
			(isRawName ? serverName : L"\\\\.\\pipe\\" + serverName).c_str(),
			static_cast<::DWORD>(mode),
			0,
			nullptr,
			OPEN_EXISTING,
			0,
			0
			);
		if (pipe == INVALID_HANDLE_VALUE)
			return File();

		return File(pipe);
	}
} //::asel