#include "../Application.h"
#include "SoundManager.h"
#include "Sound.h"

SoundManager* SoundManager::instance_ = nullptr;

void SoundManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new SoundManager();
	}
	instance_->Init();
}

SoundManager& SoundManager::GetInstance(void)
{
	return *instance_;
}

void SoundManager::Init(void)
{
	using RES = Sound;
	using RES_T = RES::TYPE;

	static std::string PATH_EFF = Application::PATH_SOUND;
	Sound res;

	// タイトル
	res = Sound(Sound::TYPE::SOUND_2D, Application::PATH_SOUND + "TitleBGM.mp3");
	res.ChengeMaxVolume(0.8);
	soundMap_.emplace(SRC::TITLE_BGM, res);

	// ゲーム
	res = Sound(Sound::TYPE::SOUND_2D, Application::PATH_SOUND + "GameBGM.mp3");
	res.ChengeMaxVolume(0.8);
	soundMap_.emplace(SRC::GAME_BGM, res);

	// ゲームオーバー
	res = Sound(Sound::TYPE::SOUND_2D, Application::PATH_SOUND + "GameOverBGM.mp3");
	res.ChengeMaxVolume(0.8);
	soundMap_.emplace(SRC::GAMEOVER_BGM, res);

	// ゲームクリア
	res = Sound(Sound::TYPE::SOUND_2D, Application::PATH_SOUND + "GameClearBGM.mp3");
	res.ChengeMaxVolume(0.8);
	soundMap_.emplace(SRC::GAMECLEAR_BGM, res);

}

void SoundManager::Release(void)
{
	for (auto& p : soundMap_)
	{
		p.second.Release();
	}

	soundMap_.clear();
}

void SoundManager::Destroy(void)
{
	Release();
	soundMap_.clear();
	delete instance_;
}

bool SoundManager::Play(SRC src, Sound::TIMES times)
{

	const auto& lPair = soundMap_.find(src);
	if (lPair != soundMap_.end())
	{
		if (!lPair->second.CheckLoad())
		{
			lPair->second.Load();
		}
		return lPair->second.Play(times);
	}
	return false;
}

bool SoundManager::Play(SRC src, Sound::TIMES times, VECTOR pos, float radius)
{
	const auto& lPair = soundMap_.find(src);
	if (lPair != soundMap_.end())
	{
		if (!lPair->second.CheckLoad())
		{
			lPair->second.Load();
		}
		return lPair->second.Play(pos, radius, times);
	}
	return false;
}

void SoundManager::Stop(SRC src)
{
	const auto& lPair = soundMap_.find(src);
	if (lPair != soundMap_.end())
	{
		return lPair->second.Stop();
	}
}

bool SoundManager::CheckMove(SRC src)
{
	const auto& lPair = soundMap_.find(src);
	if (lPair != soundMap_.end())
	{
		return lPair->second.CheckMove();
	}
	return false;
}

void SoundManager::ChengeVolume(SRC src, float per)
{
	const auto& lPair = soundMap_.find(src);
	if (lPair != soundMap_.end())
	{
		return lPair->second.ChengeVolume(per);
	}
}
