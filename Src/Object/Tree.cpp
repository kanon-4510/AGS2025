#include<DxLib.h>
#include"../Common/Vector2.h"
#include"../Scene/GameScene.h"
#include"../Manager/SceneManager.h"
#include "../Manager/InputManager.h"
#include"../Application.h"
#include"Player.h"
#include"Tree.h"

Tree::Tree(void)
{
	lv_ = 0;
	isAlive_ = false;
	hp_ = 0;
	water_ = 0;
	dir_ = {};
	modelIdB_ = 0;
	modelIdK_ = 0;
	modelIdA_ = 0;
	modelIdO_ = 0;

	//// ÉJÉvÉZÉãÉRÉâÉCÉ_
	//capsule_ = std::make_unique<Capsule>(transform_);
	//capsule_->SetLocalPosTop({ 00.0f, 130.0f, 1.0f });
	//capsule_->SetLocalPosDown({ 00.0f, 0.0f, 1.0f });
	//capsule_->SetRadius(30.0f);
}
Tree::~Tree(void)
{
}

bool Tree::Init(GameScene* parent)
{
	modelIdB_ = MV1LoadModel((Application::PATH_MODEL + "wood/Baby.mv1").c_str());
	modelIdK_ = MV1LoadModel((Application::PATH_MODEL + "wood/Kid.mv1").c_str());
	modelIdA_ = MV1LoadModel((Application::PATH_MODEL + "wood/Adult.mv1").c_str());
	modelIdO_ = MV1LoadModel((Application::PATH_MODEL + "wood/Old.mv1").c_str());

	scl_ = { 2.0f, 2.0f, 2.0f };						// ëÂÇ´Ç≥ÇÃê›íË
	rot_ = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };		// äpìxÇÃê›íË
	pos_ = { 0.0f, 60.0f, 0.0f };						// à íuÇÃê›íË

	lv_ = 1;
	isAlive_ = true;
	grow_ = Tree::GROW::BABY;
	hp_ = 50;
	water_ = 0;
	gameScene_ = parent;

	//collisionRadius_ = 100.0f;	// è’ìÀîªíËópÇÃãÖëÃîºåa
	//collisionLocalPos_ = { 0.0f, 60.0f, 0.0f };	// è’ìÀîªíËópÇÃãÖëÃíÜêSÇÃí≤êÆç¿ïW

	return true;
}
void Tree::Update(void)
{
	switch (grow_)
	{
	case Tree::GROW::BABY:
		scl_ = { 3.0f, 2.5f, 3.0f };						// ëÂÇ´Ç≥ÇÃê›íË
		rot_ = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };		// äpìxÇÃê›íË
		pos_ = { 0.0f, 100.0f, 0.0f };						// à íuÇÃê›íË
		MV1SetScale(modelIdB_, scl_);		//ÇRÇcÉÇÉfÉãÇÃëÂÇ´Ç≥Çê›íË(à¯êîÇÕÅAx, y, zÇÃî{ó¶)
		MV1SetRotationXYZ(modelIdB_, rot_);	//ÇRÇcÉÇÉfÉãÇÃå¸Ç´(à¯êîÇÕÅAx, y, zÇÃâÒì]ó ÅBíPà ÇÕÉâÉWÉAÉìÅB)
		MV1SetPosition(modelIdB_, pos_);	//ÇRÇcÉÇÉfÉãÇÃà íu(à¯êîÇÕÅAÇRÇcç¿ïW)
		break;
	case Tree::GROW::KID:
		rot_ = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };		// äpìxÇÃê›íË
		scl_ = { 15.0f, 10.0f, 15.0f };
		pos_ = { 0.0f, 450.0f, 0.0f };
		MV1SetScale(modelIdK_, scl_);		//ÇRÇcÉÇÉfÉãÇÃëÂÇ´Ç≥Çê›íË(à¯êîÇÕÅAx, y, zÇÃî{ó¶)
		MV1SetRotationXYZ(modelIdK_, rot_);	//ÇRÇcÉÇÉfÉãÇÃå¸Ç´(à¯êîÇÕÅAx, y, zÇÃâÒì]ó ÅBíPà ÇÕÉâÉWÉAÉìÅB)
		MV1SetPosition(modelIdK_, pos_);	//ÇRÇcÉÇÉfÉãÇÃà íu(à¯êîÇÕÅAÇRÇcç¿ïW)
		break;
	case Tree::GROW::ADULT:
		rot_ = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };		// äpìxÇÃê›íË
		scl_ = { 30.0f, 25.0f, 30.0f };
		pos_ = { 0.0f, 805.0f, 0.0f };
		MV1SetScale(modelIdA_, scl_);		//ÇRÇcÉÇÉfÉãÇÃëÂÇ´Ç≥Çê›íË(à¯êîÇÕÅAx, y, zÇÃî{ó¶)
		MV1SetRotationXYZ(modelIdA_, rot_);	//ÇRÇcÉÇÉfÉãÇÃå¸Ç´(à¯êîÇÕÅAx, y, zÇÃâÒì]ó ÅBíPà ÇÕÉâÉWÉAÉìÅB)
		MV1SetPosition(modelIdA_, pos_);	//ÇRÇcÉÇÉfÉãÇÃà íu(à¯êîÇÕÅAÇRÇcç¿ïW)
		break;
	case Tree::GROW::OLD:
		rot_ = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };		// äpìxÇÃê›íË
		scl_ = { 50.0f, 45.0f, 50.0f };
		pos_ = { 0.0f, 1470.0f, 0.0f };
		MV1SetScale(modelIdO_, scl_);		//ÇRÇcÉÇÉfÉãÇÃëÂÇ´Ç≥Çê›íË(à¯êîÇÕÅAx, y, zÇÃî{ó¶)
		MV1SetRotationXYZ(modelIdO_, rot_);	//ÇRÇcÉÇÉfÉãÇÃå¸Ç´(à¯êîÇÕÅAx, y, zÇÃâÒì]ó ÅBíPà ÇÕÉâÉWÉAÉìÅB)
		MV1SetPosition(modelIdO_, pos_);	//ÇRÇcÉÇÉfÉãÇÃà íu(à¯êîÇÕÅAÇRÇcç¿ïW)
		break;
	}

	if (lv_ >=75 && water_ >= 4)
	{
		lv_ += 1;
		water_ -= 4;
		ChangeGrow();
	}
	else if (lv_ >= 50 && water_ >= 3)
	{
		lv_ += 1;
		water_ -= 3;
		ChangeGrow();
	}
	if (lv_ >= 25 && water_ >= 2)
	{
		lv_ += 1;
		water_ -= 2;
		ChangeGrow();
	}
	if (lv_ >= 1 && water_ >= 1)
	{
		lv_ += 1;
		water_ -= 1;
		ChangeGrow();
	}
	ChangeGrow();

	auto& ins = InputManager::GetInstance();
	if (ins.IsNew(KEY_INPUT_O))
	{
		lv_ += 1;
		ChangeGrow();
	}
	else if (ins.IsNew(KEY_INPUT_P))
	{
		lv_ -= 1;
	}
}
void Tree::Draw(void)
{
	// ÉÇÉfÉãÇÃï`âÊ
	switch (grow_)
	{
	case Tree::GROW::BABY:
		MV1DrawModel(modelIdB_);
		break;
	case Tree::GROW::KID:
		MV1DrawModel(modelIdK_);
		break;
	case Tree::GROW::ADULT:
		MV1DrawModel(modelIdA_);
		break;
	case Tree::GROW::OLD:
		MV1DrawModel(modelIdO_);
		break;
	}

	DrawDebug();
}

void Tree::DrawDebug(void)
{
	int white = 0xffffff;
	int black = 0x000000;
	int red = 0xff0000;
	int green = 0x00ff00;
	int blue = 0x0000ff;
	int yellow = 0xffff00;
	int purpl = 0x800080;

	VECTOR v;

	v = pos_;
	DrawFormatString(20, 230, white, "ñÿÇÃç¿ïWÅF(%0.2f, %0.2f, %0.2f)ñÿÇÃèÛë‘(%d,%d)",
		v.x, v.y, v.z,lv_,grow_
	);
}

int Tree::GetHp(void)
{
	return hp_;
}
int Tree::GetLv(void)
{
	return lv_;
}
void Tree::ChangeGrow(void)
{
	if (lv_ == 75)
	{
		grow_ = Tree::GROW::OLD;
		hp_ = 50;
	}
	else if (lv_ == 50)
	{
		grow_ = Tree::GROW::ADULT;
		hp_ = 50;
	}
	else if (lv_ == 25)
	{
		grow_ = Tree::GROW::KID;
		hp_ = 50;
	}
}

//const Capsule& Tree::GetCapsule(void) const
//{
//	return *capsule_;
//}

//void Tree::SetCollisionPos(const VECTOR collision)
//{
//	spherePos_ = collision;
//}

//VECTOR Tree::GetCollisionPos(void) const
//{
//	return VAdd(collisionLocalPos_,pos_);
//}

//float Tree::GetCollisionRadius(void)
//{
//	return collisionRadius_;
//}

//void Tree::eHit(void)
//{
//	hp_ -= 1;
//}
//void Tree::pHit(void)
//{
//	water_++;
//}