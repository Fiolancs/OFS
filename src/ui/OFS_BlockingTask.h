#pragma once 
#include "OFS_Util.h"

#include <SDL3/SDL_thread.h>

#include <memory>

struct BlockingTaskData
{
	const char* TaskDescription = "";
	void* User = 0;
	int Progress = 0; int MaxProgress = 0;
	SDL_ThreadFunction TaskThreadFunc = nullptr;
	bool DimBackground = true;
};

class OFS_BlockingTask
{
	float RunningTimer = 0.f;
public:
	bool Running = false;
	std::unique_ptr<BlockingTaskData> currentTask = nullptr;

	void DoTask(std::unique_ptr<BlockingTaskData>&& task) noexcept
	{
		FUN_ASSERT(currentTask == nullptr, "there's already a task");
		currentTask = std::move(task);
	}

	void ShowBlockingTask() noexcept;
};
