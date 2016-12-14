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

	Optional<FileOpenResult> dialogOpenMultiFile(uint32 bufSize) {
		String pathes;
		pathes.resize(bufSize);

		::OPENFILENAME dialogInfo = {};
		dialogInfo.lStructSize = sizeof(dialogInfo);
		dialogInfo.lpstrFilter = TEXT("All files {*.*}\0*.*\0\0");
		dialogInfo.lpstrFile = &pathes[0];
		dialogInfo.nMaxFile = bufSize;
		dialogInfo.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_FILEMUSTEXIST;

		if (::GetOpenFileName(&dialogInfo) == 0)
			return none;

		pathes.resize(pathes.indexOf(String(2, '\0')));
		pathes.replace(L'\\', L'/');

		FileOpenResult ret;
		ret.files = pathes.split(L'\0');

		auto& head = ret.files[0];
		if (ret.files.size() == 1) {
			const size_t bsIdx = head.lastIndexOf(L'/');

			ret.directory = head.substr(0, bsIdx + 1);
			head = head.substr(bsIdx + 1);
		}
		else {
			ret.directory = head + L'/';
			ret.files.erase(ret.files.begin());
		}

		return std::move(ret);
	}

} // ::asel