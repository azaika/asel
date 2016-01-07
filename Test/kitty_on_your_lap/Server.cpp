#include <Siv3D.hpp>
#include "Asel.h"

void Main() {
	const Font font(30);
	const String sendText = L"Kitty on your lap";
	bool hasSent = false;

	asel::Server server(L"aselTestServer");
	if (server) {
		server.start([&](asel::File& pipe) {
			pipe.write(sendText);
		});
	}

	while (System::Update()) {
		if (server) {
			font(L"Server: \n" + server.getName() + L"\n�̍\�z�ɐ���").draw();

			if (hasSent || server.hasConnected()) {
				server.update();
				hasSent = true;

				font(L"�e�L�X�g: " + sendText + L"\n�𑗐M���܂���").draw({ 0, 170 });
			}
			else
				font(L"�N���C�A���g�����̐ڑ��ҋ@��").draw({ 0, 170 });
		}
		else
			font(L"�T�[�o�[�̍\�z�Ɏ��s").draw();
	}
}