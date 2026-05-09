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
	void ShowMakeRoom( IImPopupWindow& wnd );
	void ShowJoinRoom( IImPopupWindow& wnd );
	static void ShowMakeFail( ImPopupContext& c , const std::string& errorMsg);
	static void ShowJoinFail( ImPopupContext& c , const std::string& errorMsg);

};

