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
            font(L"Server: \n" + server.getName() + L"\nの構築に成功").draw();

            if (hasSent || server.hasConnected()) {
                server.update();
                hasSent = true;

                font(L"テキスト: " + sendText + L"\nを送信しました").draw({ 0, 170 });
            }
            else
                font(L"クライアントからの接続待機中").draw({0, 170});
        }
        else
            font(L"サーバーの構築に失敗").draw();
    }
}
