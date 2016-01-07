#include <Siv3D.hpp>
#include "Asel.h"

void Main() {
	const Font font(30);
	String readStr = L"サーバーへの接続に失敗";
	auto client = asel::connectServer(L"aselTestServer");

	if (client) {
		auto str = client.read(17);
		if (str)
			readStr = *str;

		client.close();
	}

	while (System::Update()) {
		font(readStr).draw();
	}
}