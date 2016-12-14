#define NOMINMAX
#include <Windows.h>
#undef NOMINMAX

#include "../Asel.h"

namespace asel {
	File::File(Handle h) {
		file_ = std::shared_ptr<Handle>(
			new Handle(h),
			[](Handle* h) { ::CloseHandle(*h); }
		);
	}
	File::File(const std::shared_ptr<Handle>& h) {
		file_ = h;
	}

	Optional<String> File::read(size_t size) const {
		String res;
		res.resize(size);
		::DWORD readSize;

		const bool isSuccess = ::ReadFile(
			*file_,
			const_cast<wchar*>(res.data()),
			static_cast<::DWORD>(size * sizeof(wchar)),
			&readSize,
			nullptr
			) != 0;

		res.resize(readSize / sizeof(wchar));

		if (isSuccess)
			return res;
		else
			return none;
	}

	bool File::write(const String& str) {
		::DWORD writtenSize;

		return ::WriteFile(
			*file_,
			str.c_str(),
			static_cast<::DWORD>(str.length * sizeof(wchar)),
			&writtenSize,
			nullptr
			) != 0;
	}
} //::asel