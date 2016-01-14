#include <Siv3D.hpp>
#include "Asel.h"

void Main() {
	const Font font(60);
	Array<asel::Process> procs;
    
    //ボタンの背景
	auto launchBox = font
		.region(L"Launch!", 1.0)
		.scaled(1.1)
		.setCenter(Window::Center());

	while (System::Update()) {
		if (launchBox.leftClicked)
			procs.emplace_back(L"C:\\Windows\\System32\\notepad.exe");

		launchBox.draw(Color(Palette::Orange, 128));
		font.drawCenter(L"Launch!", Window::Center());
	}

	for (auto&& p : procs)
		p.terminate();
}