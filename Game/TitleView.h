#pragma once
#include "View.h"

class TitleView : public CView
{
public:
	TitleView();
	virtual ~TitleView();

public:
	void OnVisible() override;
	void OnShow(Context& context) override;

private:
	void ShowMakeRoom( ImPopupContext& c );
	void ShowJoinRoom( ImPopupContext& c );
	static void ShowMakeFail( ImPopupContext& c , const std::string& errorMsg);
	static void ShowJoinFail( ImPopupContext& c , const std::string& errorMsg);

};

