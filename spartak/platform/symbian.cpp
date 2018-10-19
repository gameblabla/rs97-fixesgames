/*
Spartak Chess based on stockfish engine.
Copyright (C) 2010 djdron

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef _SYMBIAN

#include "../game.h"
#include "../ui/dialog.h"

#include "../io.h"

#include <eikstart.h>
#include <eikedwin.h>
#include <aknapp.h>
#include <aknappui.h>
#include <akndoc.h>
#include <pathinfo.h>
#include <avkon.rsg>
#include <utf.h>
#include <aknsoundsystem.h>

#include <spartak-chess.rsg>
#include "../build/symbian/spartak-chess.hrh"

static eGame* game = NULL;

enum { MAX_PATH_LEN = 1024 };
#define SAFE_CALL(p) if(p) p
template<class T> void SAFE_DELETE(T*& p) { if(p) { delete p; p = NULL; } }

static const char* FileNameToCStr(const TFileName& n)
{
	static char buf[MAX_PATH_LEN];
	TPtr8 ptr((TUint8*)buf, MAX_PATH_LEN);
	CnvUtfConverter::ConvertFromUnicodeToUtf8(ptr, n);
	ptr.SetLength(n.Length());
	ptr.ZeroTerminate();
	return buf;
}

void Init()
{
	TFileName appPath;
	CEikonEnv::Static()->FsSession().PrivatePath(appPath);
	appPath.Insert(0, CEikonEnv::Static()->EikAppUi()->Application()->AppFullName().Left(2));
	const char* p = FileNameToCStr(appPath);
	xIo::ResourcePath(p);
//	xIo::ResourcePath("e:\\spartak\\");
//	xLog::Open();
//	LOG(p);
    game = new eGame;
}
void Done()
{
	SAFE_DELETE(game);
//	xLog::Close();
}

class TDCControl : public CCoeControl
{
public:
	void ConstructL(const TRect& aRect);
	TDCControl() : iTimer(NULL), bitmap(NULL), frame(0)	{}
	virtual ~TDCControl();

public:
	static TInt TimerCallBack(TAny* aInstance);

private:
	void OnTimer();
	void Draw(bool horizontal) const;
	void HandleResourceChange(TInt aType);
	TInt CountComponentControls() const { return 0; }
	CCoeControl* ComponentControl(TInt aIndex) const { return NULL; }
	void Draw(const TRect& aRect) const;
	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);
protected:
	CIdle* iTimer;
	CFbsBitmap* bitmap;
	mutable int frame;
};

static inline dword BGRX(byte r, byte g, byte b)
{
	return (r << 16)|(g << 8)|b;
}

void TDCControl::ConstructL(const TRect&)
{
	CreateWindowL();
	SetExtentToWholeScreen();
	ActivateL();
	Init();

	bitmap = new CFbsBitmap;
	bitmap->Create(TSize(320, 320), EColor16MU);
	iTimer = CIdle::NewL(CActive::EPriorityIdle);
	iTimer->Start(TCallBack(TDCControl::TimerCallBack, this));
}
TDCControl::~TDCControl()
{
	SAFE_CALL(iTimer)->Cancel();
	SAFE_DELETE(iTimer);
	SAFE_DELETE(bitmap);
	Done();
}
void TDCControl::Draw(const TRect&) const
{
	CWindowGc& gc = SystemGc();
	gc.SetBrushColor(0);
	TRect r = Rect();
	gc.Clear(r);
	if(bitmap)
	{
		bool h = r.Width() > r.Height();
		Draw(h);
		TRect rb(0, 0, h ? 320 : 240, h ? 240 : 320);
		int dx = r.Width() - rb.Width();
		int dy = r.Height() - rb.Height();
		if(dx < 0)	dx = 0;
		if(dy < 0)	dy = 0;
		gc.BitBlt(TPoint(dx/2, dy/2), bitmap, rb);
	}
}
void TDCControl::Draw(bool horizontal) const
{
	if(!game->Desktop().Update())
		return;
	eRGBA* data = game->Desktop().Buffer();
	bitmap->LockHeap();
	dword* tex = (dword*)bitmap->DataAddress();

	if(horizontal)
	{
		for(int i = 0; i < 320*240; ++i)
		{
			eRGBA c(*data++);
			*tex++ = BGRX(c.r, c.g, c.b);
		}
	}
	else
	{
		for(int j = 0; j < 320; j++)
		{
			eRGBA* d = data + 320*239 + j;
			for(int i = 0; i < 240; ++i)
			{
				eRGBA c(*d);
				*tex++ = BGRX(c.r, c.g, c.b);
				d -= 320;
			}
			tex += 320-240;
		}
	}
	bitmap->UnlockHeap();
}
void TDCControl::OnTimer()
{
	++frame;
	if(!(frame%100))
		User::ResetInactivityTime();
	User::AfterHighRes(15000);
	if(game->Update())
		DrawDeferred();
	else
		CEikonEnv::Static()->EikAppUi()->HandleCommandL(EEikCmdExit);
}
void TDCControl::HandleResourceChange(TInt aType)
{
	switch(aType)
	{
	case KEikDynamicLayoutVariantSwitch:
		SetExtentToWholeScreen();
		break;
	}
}
TInt TDCControl::TimerCallBack( TAny* aInstance )
{
	((TDCControl*)aInstance)->OnTimer();
	return 1;
}
static char TranslateKey(const TKeyEvent& aKeyEvent)
{
	bool rotate = true;
    switch(aKeyEvent.iScanCode)
    {
    case '5':
    case EStdKeyDevice3:		return 'a';
    case '4':
    case EStdKeyLeftArrow:		return rotate ? 'd' : 'l';
    case '6':
    case EStdKeyRightArrow:		return rotate ? 'u' : 'r';
    case '2':
    case EStdKeyUpArrow:		return rotate ? 'l' : 'u';
    case '8':
    case EStdKeyDownArrow:      return rotate ? 'r' : 'd';
    case EStdKeyYes:			return 'f';
    }
    return 0;
}
TKeyResponse TDCControl::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
{
	if(aType != EEventKeyDown)
		return EKeyWasNotConsumed;
	char ch = TranslateKey(aKeyEvent);
	if(!ch)
		return EKeyWasNotConsumed;
	game->Command(ch);
	return EKeyWasConsumed;
}

class TAppUi : public CAknAppUi
{
public:
	void ConstructL()
	{
		BaseConstructL();
		gl_control = new (ELeave) TDCControl;
		gl_control->SetMopParent(this);
		gl_control->ConstructL(ClientRect());
		AddToStackL( gl_control );
		SetKeyBlockMode(ENoKeyBlock);
		CAknKeySoundSystem* ks = KeySounds();
		if(ks)
		{
			ks->PushContextL(R_AVKON_SILENT_SKEY_LIST);
			ks->BringToForeground();
			ks->LockContext();
		}
	}
	virtual ~TAppUi()
	{
		if(gl_control)
		{
			RemoveFromStack(gl_control);
			delete gl_control;
		}
		CAknKeySoundSystem* ks = KeySounds();
		if(ks)
		{
			ks->ReleaseContext();
			ks->PopContext();
		}
	}

private:
	void HandleCommandL(TInt aCommand);
	virtual TKeyResponse HandleKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{
		return EKeyWasNotConsumed;
	}

private:
	TDCControl* gl_control;
};

void TAppUi::HandleCommandL(TInt aCommand)
{
	switch(aCommand)
	{
	case EEikCmdExit:
	case EAknSoftkeyExit:
		Exit();
		break;
//	case EReset:
//		gl_control->Reset();
//		break;
	default:
		break;
	}
}

class TDoc : public CAknDocument
{
public:
	TDoc(CEikApplication& aApp) : CAknDocument(aApp)	{}
	virtual ~TDoc() {}

private:
	CEikAppUi* CreateAppUiL() { return new (ELeave) TAppUi; }
};

class TApp : public CAknApplication
{
private:
	CApaDocument* CreateDocumentL()	{ return new (ELeave) TDoc(*this); }
	TUid AppDllUid() const { const TUid KUid = { 0xA89FB850 }; return KUid; }
};

LOCAL_C CApaApplication* NewApplication()
{
	return new TApp;
}

GLDEF_C TInt E32Main()
{
	return EikStart::RunApplication(NewApplication);
}

#if __GCCE__

extern "C"
{

extern unsigned int __aeabi_uidivmod(unsigned numerator, unsigned denominator);

int __aeabi_idiv(int numerator, int denominator)
{
	int neg_result = (numerator ^ denominator) & 0x80000000;
	int result = __aeabi_uidivmod((numerator < 0) ? -numerator : numerator,
			(denominator < 0) ? -denominator : denominator);
	return neg_result ? -result : result;
}
unsigned __aeabi_uidiv(unsigned numerator, unsigned denominator)
{
	return __aeabi_uidivmod(numerator, denominator);
}

}
//extern "C"

#endif//__GCCE__

#endif//_SYMBIAN
