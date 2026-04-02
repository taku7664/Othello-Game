#pragma once
#include "View.h"

class ErrorView : public CView
{
public:
	ErrorView();
	virtual ~ErrorView();

public:
	void OnVisible() override;
	void OnHide() override;
	void OnShow(Context& context) override;

private:

};

