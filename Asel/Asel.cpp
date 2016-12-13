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

	Optional<FileOpenResult> dialogOpenMultiFile(std::uint32_t bufSize) {
		String pathes;
		pathes.resize(bufSize);

		::OPENFILENAME diagInfo = {};
		diagInfo.lStructSize = sizeof(diagInfo);
		diagInfo.lpstrFilter = TEXT("All files {*.*}\0*.*\0\0");
		diagInfo.lpstrFile = &pathes[0];
		diagInfo.nMaxFile = bufSize;
		diagInfo.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_FILEMUSTEXIST;

		if (::GetOpenFileName(&diagInfo) == 0)
			return none;

		pathes.resize(pathes.indexOf(String(2, '\0')));

		FileOpenResult ret;
		ret.files = pathes.split(L'\0');

		auto& first = ret.files[0];
		if (ret.files.size() == 1) {
			const size_t bsIdx = first.lastIndexOf(L'\\');

			ret.directory = first.substr(0, bsIdx + 1);
			first = first.substr(bsIdx + 1);
		}
		else {
			ret.directory = first + L'\\';
			ret.files.erase(ret.files.begin());
		}

		return std::move(ret);
	}

} //::asel