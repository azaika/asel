#pragma once

#include <Siv3D.hpp>

#include <memory>
#include <utility>
#include <thread>
#include <mutex>

namespace asel {
	using Handle = void*;

	namespace impl {
		void releaseHandle(Handle*);

		using HandlePtr = std::unique_ptr<Handle, decltype(&releaseHandle)>;
	} //::asel::impl

	class File {
	public:
		File() = default;
		File(const File&) = default;
		File(File&&) = default;
		File& operator = (const File&) = default;
		File& operator = (File&&) = default;

		explicit File(Handle h);
		explicit File(const std::shared_ptr<Handle>& h);

		explicit operator bool() {
			return file_ != nullptr;
		}

		/// <summary>
		/// <para>�Ǘ�����t�@�C�����當����ǂݍ��݂܂��B</para>
		/// </summary>
		/// <param name="size">�ǂݍ��ޕ�����</param>
		Optional<String> read(size_t size) const;

		/// <summary>
		/// <para>�Ǘ�����t�@�C���ɕ������������݂܂��B</para>
		/// </summary>
		/// <param name="str">�������ޕ�����</param>
		bool write(const String& str);

		/// <summary>�Ǘ����Ă���t�@�C���𖳌��ɂ��A�j�����܂�</summary>
		void close() {
			file_.reset();
		}

	private:
		std::shared_ptr<Handle> file_ = nullptr;

	};

	class Process {
	public:
		struct Info {
			Handle procHandle = 0, threadHandle = 0;
			uint32 procId = 0, threadId = 0;
		};

		//�v���Z�X���܂����쒆�ł��邱�Ƃ�����
		static constexpr uint32 Running = 0x103ul;

		Process() = default;
		Process(Process&&) = default;
		Process& operator = (Process&&) = default;

		/// <param name="path">���s�t�@�C���̃p�X</param>
		/// <param name="args">�R�}���h���C������</param>
		Process(const FilePath& path, const String& args);

		/// <summary>�Ǘ�����v���Z�X�������I�����܂��B</summary>
		/// <param name="exitCode">�v���Z�X�̏I���R�[�h</param>
		bool terminate(int exitCode = 0);

		/// <summary>�Ǘ�����v���Z�X�̏I���R�[�h���擾���܂��B</summary>
		/// <returns>
		/// <para>�v���Z�X���I�����Ă���ꍇ: �v���Z�X�̏I���R�[�h</para>
		/// <para>�v���Z�X�����쒆�ł���ꍇ: <c>Process::Running</c></para>
		/// </returns>
		uint32 getExitCode() const;

		bool isAlive() const noexcept {
			return info_ != nullptr;
		}
		explicit operator bool() const noexcept {
			return isAlive();
		}

		const Info& getInfo() const {
			return *info_;
		}

	private:
		static void releaseInfo(Info* pi);

		using InfoPtr = std::unique_ptr<Info, decltype(&releaseInfo)>;
		InfoPtr info_ = InfoPtr(nullptr, &releaseInfo);

	};

	enum class PipeAccess : uint32 {
		//�N���C�A���g���������݁A�T�[�o�[�͓ǂݎ��
		Inbound = 1,
		//�T�[�o�[���ǂݍ��݁A�N���C�A���g����������
		Outbound = 2,
		//���R
		Free = 3
	};

	enum class ConnectMode : uint32 {
		//�ǂݎ���p
		Read = 1ul << 31,
		//�������ݐ�p
		Write = 1ul << 30,
		//���R
		Free = Read | Write
	};

	/// <summary>PipeAccess��ConnectMode�ɕϊ����܂��B</summary>
	/// <param name="pa">�ϊ�����PipeAccess</param>
	inline ConnectMode toConnectMode(PipeAccess pa) {
		return (
			pa == PipeAccess::Free ? ConnectMode::Free :
			pa == PipeAccess::Inbound ? ConnectMode::Write :
			ConnectMode::Read
			);
	}

	/// <summary>ConnectMode��PipeAccess�ɕϊ����܂��B</summary>
	/// <param name="pa">�ϊ�����ConnectMode</param>
	inline PipeAccess toPipeAccess(ConnectMode pa) {
		return (
			pa == ConnectMode::Free ? PipeAccess::Free :
			pa == ConnectMode::Write ? PipeAccess::Inbound :
			PipeAccess::Outbound
			);
	}

	/// <summary>�T�[�o�[(�p�C�v)�ɐڑ����܂��B</summary>
	/// <param name="serverName">�T�[�o�[(�p�C�v)�̖��O</param>
	/// <param name="mode">�ǂݏ����A�N�Z�X�͈�</param>
	/// <param name="isRawName"><paramref name="serverName"/>��WinAPI�����̖��O�ɂȂ��Ă��邩�ǂ���</param>
	/// <remarks><c>mode</c>���A�N�Z�X����T�[�o�[�̐ݒ�ƈ�v���Ă���K�v������܂��B</remarks>
	File connectServer(
		const String& serverName,
		ConnectMode mode,
		bool isRawName = false
		);
	/// <summary>�T�[�o�[(�p�C�v)�ɐڑ����܂��B</summary>
	/// <param name="serverName">�T�[�o�[(�p�C�v)�̖��O</param>
	/// <param name="isRawName"><paramref name="serverName"/>��WinAPI�����̖��O�ɂȂ��Ă��邩�ǂ���</param>
	/// <remarks>�A�N�Z�X����T�[�o�[�̃A�N�Z�X�ݒ肪Free�ł���K�v������܂��B</remarks>
	inline File connectServer(
		const String& serverName,
		bool isRawName = false
		) {
		return connectServer(serverName, ConnectMode::Free, isRawName);
	}

	class Server {
	public:
		Server() = default;
		Server(Server&&) = default;
		Server& operator = (Server&&) = default;

		/// <param name="name">��������p�C�v�̖��O</param>
		/// <param name="isRawName"><paramref name="name"/>��WinAPI�����̌`�ɂȂ��Ă��邩�ǂ���</param>
		/// <remarks>�A�N�Z�X���[�h��Free�ɐݒ肳��܂��B</remarks>
		Server(
			const String& name,
			bool isRawName = false
			) :
			Server(
				name,
				PipeAccess::Free,
				isRawName
				) {}
		/// <param name="name">��������p�C�v�̖��O</param>
		/// <param name="mode">�A�N�Z�X���[�h�̎��</param>
		/// <param name="isRawName"><paramref name="name"/>��WinAPI�����̌`�ɂȂ��Ă��邩�ǂ���</param>
		Server(
			const String& name,
			PipeAccess mode,
			bool isRawName = false
			);

		explicit operator bool() const noexcept {
			return pipe_ != nullptr;
		}

		/// <summary>�N���C�A���g�ւ̉������ɌĂяo�����֐���ݒ肵�܂��B</summary>
		/// <param name="f">�T�[�o�[�̉������ɌĂяo�����֐�</param>
		void reaction(const std::function<void(File&)>& f) {
			reaction_ = f;
		}

		/// <summary>�N���C�A���g����̐ڑ��ҋ@���J�n���܂��B</summary>
		void start();
		/// <summary>�N���C�A���g����̐ڑ��ҋ@���J�n���܂��B</summary>
		/// <param name="f">�T�[�o�[�̉������ɌĂяo�����֐�</param>
		void start(const std::function<void(File&)>& f) {
			reaction(f);
			start();
		}

		/// <summary>�N���C�A���g����̐ڑ��ҋ@��Ԃ��~���܂��B</summary>
		void stop() {
			pipe_.reset();
		}

		/// <summary>�T�[�o�[�̏�Ԃ��X�V���A�N���C�A���g����̐ڑ����������ꍇ�͉������܂��B</summary>
		bool update();

		/// <summary>���ݐڑ����Ă���N���C�A���g���狭���ؒf���܂��B</summary>
		bool disconnect();

		/// <summary>�N���C�A���g����̐ڑ������邩�ǂ����𒲂ׂ܂��B</summary>
		bool hasConnected() const {
			bool ret;
			{
				std::lock_guard<std::mutex> lock(mtx_);
				ret = hasConnected_;
			}

			return ret;
		}

		/// <summary>�p�C�v�����擾����B</summary>
		/// <param name="doGetRaw">WinAPI�����̖{���̖��O���擾���邩�ǂ���</param>
		/// <returns><paramref name="doGetRaw"/>��true�̏ꍇ�A\\.\pipe\pipename�̌`�Ńp�C�v�����Ԃ�B</returns>
		String getName(bool doGetRaw = true) const noexcept {
			return (
				doGetRaw || name_.isEmpty
				? name_
				: name_.substr(std::size(L"\\\\.\\pipe\\") - 1)
				);
		}

		PipeAccess getAccessMode() const;

		~Server();

	private:
		//�ێ�����p�C�v��
		String name_;
		//�p�C�v�̃n���h��
		std::shared_ptr<Handle> pipe_;

		std::function<void(File&)> reaction_;

		mutable std::mutex mtx_;
		//�ڑ��҂��X���b�h
		std::thread waitConnect_;
		//�ڑ��҂�Event�pHandle
		//0: �ҋ@�p, 1:�����I���p
		impl::HandlePtr events_[2] = { {nullptr, impl::releaseHandle}, {nullptr, impl::releaseHandle} };
		bool hasConnected_ = false;

	};

} //::asel