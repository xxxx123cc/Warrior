#pragma once



UENUM()//检查Actor是否拥有指定的GameplayTag
enum class EWarriorConfirmType:uint8
{
	Yes,  // 确认
	No    // 否定
};

UENUM()//检查返回值是否有效
enum class EWarriorValidType:uint8
{
	Valid,  // 确认
	InValid   // 否定
};
UENUM()//检查effect是否成功应用
enum class EWarriorSuccessful:uint8
{
	Successful,  // 确认
	Failed   // 否定
};



class WarriorEnumTypes
{
public:
	
};
