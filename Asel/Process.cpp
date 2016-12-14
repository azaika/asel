#define NOMINMAX
#include <Windows.h>
#undef NOMINMAX

#include "../Asel.h"

namespace asel {
	Process::Process(const s3d::String& cmdLine) {
		info_ = InfoPtr(new Info, &releaseInfo);

		::STARTUPINFO si = {};
		si.cb = sizeof(::STARTUPINFO);

		if (
			::CreateProcess(
				cmdLine.c_str(),
				nullptr,
				nullptr,
				nullptr,
				false,
				0,
				nullptr,
				cmdLine.substr(0, cmdLine.lastIndexOf(L'/')).replace(L'/', L'\\').c_str(),
				&si,
				reinterpret_cast<::PROCESS_INFORMATION*>(info_.get())
				) == 0
			)
			info_.reset();
	}
	Process::Process(const FilePath& path, const String& args) {
		info_ = InfoPtr(new Info, &releaseInfo);

		::STARTUPINFO si = {};
		si.cb = sizeof(::STARTUPINFO);

		if (
			::CreateProcess(
				path.c_str(),
				const_cast<wchar_t*>(args.c_str()),
				nullptr,
				nullptr,
				false,
				0,
				nullptr,
				path.substr(0, path.lastIndexOf(L'/')).replace(L'/', L'\\').c_str(),
				&si,
				reinterpret_cast<::PROCESS_INFORMATION*>(info_.get())
				)
			)
			info_.reset();
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