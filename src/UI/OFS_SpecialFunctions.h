#pragma once
#include "state/SpecialFunctionsState.h"

#include "Funscript/Funscript.h"
#include "state/OFS_StateHandle.h"

#include <memory>


class FunctionBase {
protected:
	inline Funscript& ctx() noexcept;
public:
	virtual ~FunctionBase() noexcept {}
	virtual void DrawUI() noexcept = 0;
};

class FunctionRangeExtender : public FunctionBase 
{
	int32_t rangeExtend = 0;
	bool createUndoState = true;
	UnsubscribeFn eventUnsub;
public:
	FunctionRangeExtender() noexcept;
	virtual ~FunctionRangeExtender() noexcept;
	void SelectionChanged(const FunscriptSelectionChangedEvent* ev) noexcept;
	virtual void DrawUI() noexcept override;
};

class RamerDouglasPeucker : public FunctionBase
{
	float epsilon = 0.0f;
	float averageDistance = 0.f;
	bool createUndoState = true;
	UnsubscribeFn eventUnsub;
public:
	RamerDouglasPeucker() noexcept;
	virtual ~RamerDouglasPeucker() noexcept;
	void SelectionChanged(const FunscriptSelectionChangedEvent* ev) noexcept;
	virtual void DrawUI() noexcept override;
};

class SpecialFunctionsWindow {
private:
	FunctionBase* function = nullptr;
	OFS::StateHandle stateHandle = OFS::StateManager::INVALID_ID;
public:
	static constexpr const char* WindowId = "###SPECIAL_FUNCTIONS";
	SpecialFunctionsWindow() noexcept;
	void SetFunction(SpecialFunctionType function) noexcept;
	void ShowFunctionsWindow(bool* open) noexcept;
};