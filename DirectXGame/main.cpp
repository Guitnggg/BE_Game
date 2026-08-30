#include "GameScene.h"

#include <KamataEngine.h>
#include <Windows.h>

// エンジンの生存期間とメインループだけを担当する。
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	KamataEngine::Initialize(L"BE1_10秒ストップチャレンジ");
	{
		GameScene gameScene;
		gameScene.Initialize();

		while (!KamataEngine::Update()) {
			gameScene.Update();
			auto* const dxCommon = KamataEngine::DirectXCommon::GetInstance();
			dxCommon->PreDraw();
			gameScene.Draw();
			dxCommon->PostDraw();
		}
	}
	KamataEngine::Finalize();
	return 0;
}
