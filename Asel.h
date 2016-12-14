#pragma once

#include <Siv3D.hpp>

#include <memory>
#include <utility>
#include <thread>
#include <mutex>

namespace asel {
	using namespace s3d;

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
		/// <para>管理するファイルから文字を読み込みます。</para>
		/// </summary>
		/// <param name="size">読み込む文字数</param>
		s3d::Optional<s3d::String> read(size_t size) const;

		/// <summary>
		/// <para>管理するファイルに文字を書き込みます。</para>
		/// </summary>
		/// <param name="str">書き込む文字列</param>
		bool write(const s3d::String& str);

		/// <summary>管理しているファイルを無効にし、破棄します</summary>
		void close() {
			file_.reset();
		}

	private:
		std::shared_ptr<Handle> file_ = nullptr;

	};

	class Process {
	public:
		struct Info {
			Handle procHandle = 0;
			Handle threadHandle = 0;
			s3d::uint32 procId = 0;
			s3d::uint32 threadId = 0;
		};

		//プロセスがまだ動作中であることを示す
		static constexpr s3d::uint32 Running = 0x103ul;

		Process() = default;
		Process(Process&&) = default;
		Process& operator = (Process&&) = default;

		/// <param name="cmdLine">実行ファイルのパスを含むコマンドライン引数</param>
		Process(const s3d::String& cmdLine);
		/// <param name="path">実行ファイルのパス</param>
		/// <param name="args">コマンドライン引数</param>
		Process(const s3d::FilePath& path, const s3d::String& args);

		/// <summary>管理するプロセスを強制終了します。</summary>
		/// <param name="exitCode">プロセスの終了コード</param>
		bool terminate(int exitCode = 0);

		/// <summary>管理するプロセスの終了コードを取得します。</summary>
		/// <returns>
		/// <para>プロセスが終了している場合: プロセスの終了コード</para>
		/// <para>プロセスが動作中である場合: <c>Process::Running</c></para>
		/// </returns>
		s3d::uint32 getExitCode() const;

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

	enum class PipeAccess : s3d::uint32 {
		//クライアントが書き込み、サーバーは読み取り
		Inbound = 1,
		//サーバーが読み込み、クライアントが書き込み
		Outbound = 2,
		//自由
		Free = 3
	};

	enum class ConnectMode : s3d::uint32 {
		//読み取り専用
		Read = 1ul << 31,
		//書き込み専用
		Write = 1ul << 30,
		//自由
		Free = Read | Write
	};

	/// <summary>PipeAccessをConnectModeに変換します。</summary>
	/// <param name="pa">変換元のPipeAccess</param>
	inline ConnectMode toConnectMode(PipeAccess pa) {
		return (
			pa == PipeAccess::Free ? ConnectMode::Free :
			pa == PipeAccess::Inbound ? ConnectMode::Write :
			ConnectMode::Read
			);
	}

	/// <summary>ConnectModeをPipeAccessに変換します。</summary>
	/// <param name="pa">変換元のConnectMode</param>
	inline PipeAccess toPipeAccess(ConnectMode pa) {
		return (
			pa == ConnectMode::Free ? PipeAccess::Free :
			pa == ConnectMode::Write ? PipeAccess::Inbound :
			PipeAccess::Outbound
			);
	}

	/// <summary>サーバー(パイプ)に接続します。</summary>
	/// <param name="serverName">サーバー(パイプ)の名前</param>
	/// <param name="mode">読み書きアクセス範囲</param>
	/// <param name="isRawName"><paramref name="serverName"/>がWinAPI準拠の名前になっているかどうか</param>
	/// <remarks><c>mode</c>がアクセスするサーバーの設定と一致している必要があります。</remarks>
	File connectServer(
		const s3d::String& serverName,
		ConnectMode mode,
		bool isRawName = false
		);
	/// <summary>サーバー(パイプ)に接続します。</summary>
	/// <param name="serverName">サーバー(パイプ)の名前</param>
	/// <param name="isRawName"><paramref name="serverName"/>がWinAPI準拠の名前になっているかどうか</param>
	/// <remarks>アクセスするサーバーのアクセス設定がFreeである必要があります。</remarks>
	inline File connectServer(
		const s3d::String& serverName,
		bool isRawName = false
		) {
		return connectServer(serverName, ConnectMode::Free, isRawName);
	}

	class Server {
	public:
		Server() = default;
		Server(Server&&) = default;
		Server& operator = (Server&&) = default;

		/// <param name="name">生成するパイプの名前</param>
		/// <param name="isRawName"><paramref name="name"/>がWinAPI準拠の形になっているかどうか</param>
		/// <remarks>アクセスモードはFreeに設定されます。</remarks>
		Server(
			const s3d::String& name,
			bool isRawName = false
			) :
			Server(
				name,
				PipeAccess::Free,
				isRawName
				) {}
		/// <param name="name">生成するパイプの名前</param>
		/// <param name="mode">アクセスモードの種類</param>
		/// <param name="isRawName"><paramref name="name"/>がWinAPI準拠の形になっているかどうか</param>
		Server(
			const s3d::String& name,
			PipeAccess mode,
			bool isRawName = false
			);

		explicit operator bool() const noexcept {
			return pipe_ != nullptr;
		}

		/// <summary>クライアントへの応答時に呼び出される関数を設定します。</summary>
		/// <param name="f">サーバーの応答時に呼び出される関数</param>
		void reaction(const std::function<void(File&)>& f) {
			reaction_ = f;
		}

		/// <summary>クライアントからの接続待機を開始します。</summary>
		void start();
		/// <summary>クライアントからの接続待機を開始します。</summary>
		/// <param name="f">サーバーの応答時に呼び出される関数</param>
		void start(const std::function<void(File&)>& f) {
			reaction(f);
			start();
		}

		/// <summary>クライアントからの接続待機状態を停止します。</summary>
		void stop() {
			pipe_.reset();
		}

		/// <summary>サーバーの状態を更新し、クライアントからの接続があった場合は応答します。</summary>
		bool update();

		/// <summary>現在接続しているクライアントから強制切断します。</summary>
		bool disconnect();

		/// <summary>クライアントからの接続があるかどうかを調べます。</summary>
		bool hasConnected() const {
			bool ret;
			{
				std::lock_guard<std::mutex> lock(mtx_);
				ret = hasConnected_;
			}

			return ret;
		}

		/// <summary>パイプ名を取得します。</summary>
		/// <param name="doGetRaw">WinAPI準拠の本来の名前を取得するかどうか</param>
		/// <returns><paramref name="doGetRaw"/>がtrueの場合、\\.\pipe\pipenameの形でパイプ名が返る。</returns>
		s3d::String getName(bool doGetRaw = true) const noexcept {
			return (
				doGetRaw || name_.isEmpty
				? name_
				: name_.substr(std::size(L"\\\\.\\pipe\\") - 1)
				);
		}

		PipeAccess getAccessMode() const;

		~Server();

	private:
		//保持するパイプ名
		s3d::String name_;
		//パイプのハンドル
		std::shared_ptr<Handle> pipe_;

		std::function<void(File&)> reaction_;

		mutable std::mutex mtx_;
		//接続待ちスレッド
		std::thread waitConnect_;
		//接続待ちEvent用Handle
		//0: 待機用, 1:強制終了用
		impl::HandlePtr events_[2] = { {nullptr, impl::releaseHandle}, {nullptr, impl::releaseHandle} };
		bool hasConnected_ = false;

	};


	struct FileOpenResult {
		String directory;
		Array<String> files;
	};

	/// <summary>複数のファイルを選択できるダイアログを出します。</summary>
	/// <param name="bufSize">内部的なバッファの大きさ</param>
	Optional<FileOpenResult> dialogOpenMultiFile(std::uint32_t bufSize = 256);

} // ::asel