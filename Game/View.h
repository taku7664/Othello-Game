#pragma once

class CView
{
public:
	CView() = default;
	virtual ~CView() = default;

	struct Context
	{
		std::string Label;
		ImVec2 Size;
		UINT ChildFlags = 0;
		UINT WindowFlags = 0;
	};
	
public:
	void Show(Context& context);

public:
	virtual void OnVisible() {}
	virtual void OnHide() {}
	virtual void OnShow( Context& context ) {}

};

