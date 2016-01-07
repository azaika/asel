#define NOMINMAX
#include <Windows.h>
#undef NOMINMAX

#include "../Asel.h"

namespace asel {
	namespace impl {
		PROCESS_INFORMATION& ProcInfoToWinAPI(Process::Info& pi) {
			return *reinterpret_cast<PROCESS_INFORMATION*>(&pi);
		}
	} //::asel::impl

	Process::Process(const FilePath& path, const String& args) {
		SECURITY_ATTRIBUTES procSa = {}, threadSa = {};
		procSa.nLength = sizeof(::SECURITY_ATTRIBUTES);
		threadSa.nLength = sizeof(::SECURITY_ATTRIBUTES);

		::CreateProcess(
			path.c_str(),
			const_cast<wchar_t*>(args.c_str()),
			&procSa,
			&threadSa,
			false,
			0,
			nullptr,
			nullptr,
			nullptr,
			&impl::ProcInfoToWinAPI(*info_)
			);
	}

	bool Process::terminate(int exitCode) {
		return ::TerminateProcess(info_->procHandle, exitCode) != 0;
	}

	uint32 Process::getExitCode() const {
		::DWORD ret;

		::GetExitCodeProcess(info_->procHandle, &ret);
		return ret;
	}

	void Process::releaseInfo(Info* pi) {
		::CloseHandle(pi->procHandle);
		::CloseHandle(pi->threadHandle);
	}
} //::asel