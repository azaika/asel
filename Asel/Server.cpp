#define NOMINMAX
#include <Windows.h>
#undef NOMINMAX

#include "../Asel.h"

namespace asel {
	Server::Server(
		const String& name,
		PipeAccess mode,
		bool isRawName
		) {
		name_ = (isRawName ? name : L"\\\\.\\pipe\\" + name);

		pipe_ = std::shared_ptr<Handle>(
			new Handle(
				::CreateNamedPipe(
					name_.c_str(),
					static_cast<uint32>(mode) | FILE_FLAG_OVERLAPPED,
					PIPE_TYPE_BYTE,
					PIPE_UNLIMITED_INSTANCES,
					2048,
					2048,
					1000,
					nullptr
				)),
			[](Handle* h) {
				if (*h != INVALID_HANDLE_VALUE) {
					::FlushFileBuffers(*h);
					::DisconnectNamedPipe(*h);
					::CloseHandle(*h);
				}
			}
		);

		if (*pipe_ == INVALID_HANDLE_VALUE)
			pipe_.reset();

		for (auto&& e : events_) {
			e = impl::HandlePtr(
				new Handle(
					::CreateEvent(
						nullptr,
						true,
						false,
						nullptr
						)
					),
				impl::releaseHandle
				);
		}
	}

	Server::~Server() {
		if (waitConnect_.joinable()) {
			::SetEvent(*events_[1]);

			waitConnect_.join();
		}
	}

	bool Server::update() {
		if (!reaction_)
			return true;

		bool complete = false;
		{
			std::lock_guard<std::mutex> lock(mtx_);
			complete = hasConnected_;
			hasConnected_ = false;
		}

		if (complete) {
			auto pipe = File(pipe_);
			reaction_(pipe);
		}

		return true;
	}

	void Server::start() {
		OVERLAPPED ol = {};
		ol.hEvent = *events_[0];

		::ConnectNamedPipe(*pipe_, &ol);

		waitConnect_ = std::thread([this](Handle ev1, Handle ev2) {
			const Handle evs[2] = { ev1, ev2 };

			::WaitForMultipleObjects(
				2,
				evs,
				false,
				INFINITE
				);

			std::lock_guard<std::mutex> lock(mtx_);
			hasConnected_ = true;
		}, *events_[0], *events_[1]);
	}
	
	bool Server::disconnect() {
		if (!pipe_)
			return false;

		return 
			::FlushFileBuffers(*pipe_) != 0 &&
			::DisconnectNamedPipe(*pipe_) != 0;
	}

	PipeAccess Server::getAccessMode() const {
		::DWORD ret, tmp;
		
		::GetNamedPipeInfo(*pipe_, &ret, &tmp, &tmp, &tmp);

		return static_cast<PipeAccess>(ret);
	}
}